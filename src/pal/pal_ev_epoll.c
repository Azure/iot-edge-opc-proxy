// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "pal_ev.h"
#include "pal_net.h"
#include "pal_mt.h"
#include "pal.h"
#include "os.h"
#include "util_misc.h"
#include "azure_c_shared_utility/doublylinkedlist.h"
#include "azure_c_shared_utility/threadapi.h"

#if !defined(UNIT_TEST)
#include <sys/epoll.h>
#endif

//
// A port based on epoll
//
typedef struct pal_epoll_port
{
    lock_t lock;
    int epoll_fd;
    THREAD_HANDLE thread;  // Main polling loop
    bool running;
}
pal_epoll_port_t;

//
// Poll event contains context and callback 
//
typedef struct pal_epoll_event
{
    uint32_t events;
    fd_t sock_fd;
    pal_epoll_port_t* port;
    pal_event_port_handler_t cb;
    void* context;
}
pal_epoll_event_t;

//
// Frees the event
//
static void pal_epoll_event_free(
    pal_epoll_event_t* ev_data
)
{
    if (ev_data->sock_fd != _invalid_fd)
    {
        close(ev_data->sock_fd);
    }
    ev_data->cb(ev_data->context, pal_event_type_destroy, 0);
    mem_free_type(pal_epoll_event_t, ev_data);
}

//
// Wait for events
//
static int32_t pal_epoll_event_loop(
    pal_epoll_port_t* pal_port
)
{
    int32_t result = er_ok;
    struct epoll_event evt;
    pal_epoll_event_t* ev_data;

    dbg_assert_ptr(pal_port);

    lock_enter(pal_port->lock);
    while (pal_port->running)
    {
        // Wait forever for one event, result should be 1.
        lock_exit(pal_port->lock);
        result = epoll_wait(pal_port->epoll_fd, &evt, 1, -1);

        //
        // We should never see 0 events. Given an infinite timeout, epoll_wait
        // will never return 0 events even if there are no file descriptors 
        // registered with the epoll fd. In that case, the wait will block 
        // until a file descriptor is added and an event occurs on the added 
        // file descriptor.
        //

        if (!pal_port->running)
        {
            lock_enter(pal_port->lock);
            break;
        }
        
        if (result < 1)
        {
            result = pal_os_last_net_error_as_prx_error();

            lock_enter(pal_port->lock);
            if (result != er_ok && result != er_aborted)
                break;
            result = er_ok;
            continue;
        }

        ev_data = (pal_epoll_event_t*)evt.data.ptr;
        dbg_assert_ptr(ev_data);
        
        // Dispatch events
        /**/ if (0 != (evt.events & EPOLLHUP))
            ev_data->cb(ev_data->context, pal_event_type_close, 0);
        else if (0 != (evt.events & EPOLLERR))
            ev_data->cb(ev_data->context, pal_event_type_error, 0);
        else 
        {
            if (0 != (evt.events & EPOLLIN))
            {
                while (er_ok == ev_data->cb(
                    ev_data->context, pal_event_type_read, 0))
                {
                    // Read as much as possible
                }
            }
            if (0 != (evt.events & EPOLLOUT))
            {
                while (er_ok == ev_data->cb(
                    ev_data->context, pal_event_type_write, 0))
                {
                    // Write as much as possible
                }
            }
        }
            
        lock_enter(pal_port->lock);
    }
    lock_exit(pal_port->lock);
    return result;
}

//
// Worker thread for the socket layer
//
static int32_t pal_epoll_worker_thread(
    void* context
)
{
    int32_t result = er_closed;
    pal_epoll_port_t* pal_port = (pal_epoll_port_t*)context;

    while (pal_port->running)
    {
        result = pal_epoll_event_loop(pal_port);
        if (result != er_ok && pal_port->running)
        {
            log_error(NULL, "Error occurred in polling thread: %s.",
                prx_error_string(result));
            ThreadAPI_Sleep(100);
        }
    }

    pal_port->running = false;
    return result;
}

//
// Register event callback
//
int32_t pal_event_port_register(
    uintptr_t port,
    intptr_t sock_fd,
    pal_event_port_handler_t cb,
    void* context,
    uintptr_t* event_handle
)
{
    int32_t result;
    struct epoll_event evt;
    pal_epoll_event_t* ev_data;
    pal_epoll_port_t* pal_port = (pal_epoll_port_t*)port;

    if (!pal_port || !cb || !event_handle)
        return er_fault;
    if ((fd_t)sock_fd == _invalid_fd)
        return er_arg;

    ev_data = mem_zalloc_type(pal_epoll_event_t);
    if (!ev_data)
        return er_out_of_memory;
    do
    {
        ev_data->cb = cb;
        ev_data->context = context;
        ev_data->port = pal_port;
        ev_data->sock_fd = (fd_t)sock_fd;
        evt.data.ptr = ev_data;
        evt.events = EPOLLET;

        _fd_nonblock(ev_data->sock_fd, result);

        if (0 != epoll_ctl(pal_port->epoll_fd, EPOLL_CTL_ADD, ev_data->sock_fd, &evt))
        {
            result = pal_os_last_net_error_as_prx_error();
            break;
        }

        *event_handle = (uintptr_t)ev_data;
        log_debug(NULL, "Added event port for %x.", (int)sock_fd);
        return er_ok;
    } 
    while (0);

    mem_free_type(pal_epoll_event_t, ev_data);
    return result;
}

//
// Convert socket event type to epoll event type 
//
static uint32_t pal_event_type_to_epoll_event(
    pal_event_type event_type
)
{
    switch (event_type)
    {
    case pal_event_type_close:
        return EPOLLIN | EPOLLOUT;
    case pal_event_type_read:
        return EPOLLIN;
    case pal_event_type_write:
        return EPOLLOUT;
    case pal_event_type_error:
        return EPOLLERR;
    case pal_event_type_destroy:
    case pal_event_type_unknown:
    default:
        dbg_assert(0, "Unknown event type");
        break;
    }
    return 0;
}

//
// Register interest in a certain type of event
//
int32_t pal_event_select(
    uintptr_t event_handle,
    pal_event_type event_type
)
{
    int32_t result = er_ok;
    struct epoll_event evt;
    pal_epoll_event_t* ev_data;
    uint32_t plat_event;

    ev_data = (pal_epoll_event_t*)event_handle;
    if (!ev_data)
        return er_fault;

    plat_event = pal_event_type_to_epoll_event(event_type);
    if (!plat_event)
        return er_ok;
    if (plat_event == (ev_data->events & plat_event))
        return er_ok; // Already set

    lock_enter(ev_data->port->lock);
    evt.events = (ev_data->events | EPOLLET) | plat_event;
    evt.data.ptr = (void*)ev_data;
    if (0 != epoll_ctl(ev_data->port->epoll_fd, EPOLL_CTL_MOD, ev_data->sock_fd, &evt))
        result = pal_os_last_net_error_as_prx_error();
    else
        ev_data->events |= plat_event;
    lock_exit(ev_data->port->lock);
    return result;
}

//
// Clear interest in event
//
int32_t pal_event_clear(
    uintptr_t event_handle,
    pal_event_type event_type
)
{
    int32_t result = er_ok;
    struct epoll_event evt;
    pal_epoll_event_t* ev_data;
    uint32_t plat_event;

    ev_data = (pal_epoll_event_t*)event_handle;
    if (!ev_data)
        return er_fault;

    plat_event = pal_event_type_to_epoll_event(event_type);
    if (!plat_event)
        return er_ok;
    if (plat_event != (ev_data->events & plat_event))
        return er_ok; // Already cleared

    lock_enter(ev_data->port->lock);
    evt.events = (ev_data->events | EPOLLET) & ~plat_event;
    evt.data.ptr = (void*)ev_data;
    if (0 != epoll_ctl(ev_data->port->epoll_fd, EPOLL_CTL_MOD, ev_data->sock_fd, &evt))
        result = pal_os_last_net_error_as_prx_error();
    else
        ev_data->events &= ~plat_event;
    lock_exit(ev_data->port->lock);
    return result;
}

//
// Closes the event
//
void pal_event_close(
    uintptr_t event_handle
)
{
    struct epoll_event evt;
    pal_epoll_event_t* ev_data;
    pal_epoll_port_t* pal_port;

    if (!event_handle)
        return;

    ev_data = (pal_epoll_event_t*)event_handle;
    pal_port = ev_data->port;

    lock_enter(pal_port->lock);

    ev_data->events = 0;
    evt.events = 0;
    evt.data.ptr = (void*)ev_data;
    if (0 != epoll_ctl(ev_data->port->epoll_fd, EPOLL_CTL_DEL, ev_data->sock_fd, &evt))
    {
        log_error(NULL, "Failed to delete event data from epoll port");
    }

    lock_exit(pal_port->lock);
    pal_epoll_event_free(ev_data);
}

//
// Create vector to track events
//
int32_t pal_event_port_create(
    uintptr_t* port
)
{
    int32_t result;
    pal_epoll_port_t* pal_port;
    if (!port)
        return er_fault;

    pal_port = mem_zalloc_type(pal_epoll_port_t);
    if (!pal_port)
        return er_out_of_memory;
    do
    {
        pal_port->epoll_fd = -1;
        result = lock_create(&pal_port->lock);
        if (result != er_ok)
            break;

        pal_port->epoll_fd = epoll_create1(EPOLL_CLOEXEC);
        if (pal_port->epoll_fd == -1)
        {
            result = er_out_of_memory;
            break;
        }

        // Start event loop thread
        pal_port->running = true;
        if (THREADAPI_OK != ThreadAPI_Create(
            &pal_port->thread, pal_epoll_worker_thread, pal_port))
        {
            result = er_fatal;
            break;
        }

        *port = (uintptr_t)pal_port;
        return er_ok;
    } while (0);

    pal_event_port_close((uintptr_t)pal_port);
    return result;
}

//
// Free the event port and vector
//
void pal_event_port_close(
    uintptr_t port
)
{
    int32_t result;
    int fd = -1;
    struct epoll_event evt;
    pal_epoll_port_t* pal_port = (pal_epoll_port_t*)port;
    if (pal_port)
    {
        pal_port->running = false;

        if (pal_port->epoll_fd != -1)
        {
            fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (fd != -1)
            {
                evt.events = EPOLLOUT | EPOLLONESHOT;
                evt.data.ptr = NULL;
                (void)epoll_ctl(pal_port->epoll_fd, EPOLL_CTL_ADD, fd, &evt);
            }
        }

        if (pal_port->thread)
            ThreadAPI_Join(pal_port->thread, &result);

        if (pal_port->epoll_fd != -1)
        {
            lock_enter(pal_port->lock);
            if (fd != -1)
            {
                // Unregister the control fd
                evt.events = 0;
                evt.data.ptr = NULL;
                (void)epoll_ctl(pal_port->epoll_fd, EPOLL_CTL_DEL, fd, &evt);
            }
            (void)close(pal_port->epoll_fd);
 
            pal_port->epoll_fd = -1;
            lock_exit(pal_port->lock);
        }

        if (fd != -1)
            (void)close(fd);

        if (pal_port->lock)
            lock_free(pal_port->lock);

        mem_free_type(pal_epoll_port_t, pal_port);
    }
}

