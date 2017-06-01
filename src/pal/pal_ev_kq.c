// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os.h"
#include "util_mem.h"
#include "pal_ev.h"
#include "pal_net.h"
#include "pal_mt.h"
#include "pal.h"
#include "util_misc.h"
#include "azure_c_shared_utility/doublylinkedlist.h"
#include "azure_c_shared_utility/threadapi.h"

#if !defined(UNIT_TEST)
#if defined(__linux__)
#include <kqueue/sys/event.h>
#else
#include <sys/event.h>
#endif
#endif

//
// The port based on kqueue
//
typedef struct pal_kqueue_port
{
    lock_t lock;
    int32_t kqueue_fd;
    THREAD_HANDLE thread;  // Main event loop
    bool running;
}
pal_kqueue_port_t;

//
// kequeue event contains context and callback 
//
typedef struct pal_kqueue_event
{
    fd_t sock_fd;
    bool close_fd;
    pal_kqueue_port_t* port;
    pal_event_port_handler_t cb;
    void* context;
}
pal_kqueue_event_t;

//
// Frees the event
//
static void pal_kqueue_event_free(
    pal_kqueue_event_t* ev_data
)
{
    if (ev_data->close_fd && ev_data->sock_fd != _invalid_fd)
    {
        close(ev_data->sock_fd);
    }
    ev_data->cb(ev_data->context, pal_event_type_destroy, 0);
    mem_free_type(pal_kqueue_event_t, ev_data);
}

//
// Wait for events
//
static int32_t pal_kqueue_event_loop(
    pal_kqueue_port_t* pal_port
)
{
    int32_t result = er_ok;
    struct kevent evt;
    pal_kqueue_event_t* ev_data;

    dbg_assert_ptr(pal_port);

    lock_enter(pal_port->lock);
    while (pal_port->running)
    {
        // Wait forever for one event, result should be 1.
        lock_exit(pal_port->lock);
        result = kevent(pal_port->kqueue_fd, NULL, 0, &evt, 1, NULL);

        //
        // We should never see 0 events. Given an infinite timeout, kqueue_wait
        // will never return 0 events even if there are no file descriptors 
        // registered with the kqueue fd. In that case, the wait will block 
        // until a file descriptor is added and an event occurs on the added 
        // file descriptor.
        //
        if (result < 1)
        {
            result = pal_os_last_net_error_as_prx_error();
            if (result != er_ok && result != er_arg)
                break;

            result = er_ok;
            lock_enter(pal_port->lock);
            continue;
        }

        ev_data = (pal_kqueue_event_t*)evt.udata;
        dbg_assert_ptr(ev_data);
        if (ev_data->sock_fd == _invalid_fd)
        {
            lock_enter(pal_port->lock);
            continue;
        }

        // Dispatch events
        if (evt.filter == EVFILT_READ)
            ev_data->cb(ev_data->context, pal_event_type_read, 0);
        
        else if (evt.filter == EVFILT_WRITE)
        {
            if (0 != (evt.flags & EV_EOF))
                ev_data->cb(ev_data->context, pal_event_type_read, 0);

            ev_data->cb(ev_data->context, pal_event_type_write, 0);
        }

        if (0 != (evt.flags & EV_ERROR))
            ev_data->cb(ev_data->context, pal_event_type_error, 0);

        lock_enter(pal_port->lock);
    }
    lock_exit(pal_port->lock);
    return result;
}

//
// Worker thread for the socket layer
//
static int32_t pal_kqueue_worker_thread(
    void* context
)
{
    int32_t result = er_closed;
    pal_kqueue_port_t* pal_port = (pal_kqueue_port_t*)context;

    while (pal_port->running)
    {
        result = pal_kqueue_event_loop(pal_port);
        if (result != er_ok)
        {
            log_error(NULL, "Error occurred in polling thread: %s.",
                prx_err_string(result));
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
    pal_kqueue_event_t* ev_data;
    pal_kqueue_port_t* pal_port = (pal_kqueue_port_t*)port;

    if (!pal_port || !cb || !event_handle)
        return er_fault;
    if ((fd_t)sock_fd == _invalid_fd)
        return er_arg;

    ev_data = mem_zalloc_type(pal_kqueue_event_t);
    if (!ev_data)
        return er_out_of_memory;

    ev_data->cb = cb;
    ev_data->context = context;
    ev_data->port = pal_port;
    ev_data->sock_fd = (fd_t)sock_fd;

    _fd_nonblock(ev_data->sock_fd, result);

    *event_handle = (uintptr_t)ev_data;
    log_debug(NULL, "Added event port for %x.", (int)sock_fd);
    return er_ok;
}

//
// Convert socket event type to kqueue event type 
//
static uint32_t pal_event_type_to_kqueue_event(
    pal_event_type event_type
)
{
    switch (event_type)
    {
    case pal_event_type_read:
        return EVFILT_READ;
    case pal_event_type_write:
        return EVFILT_WRITE;
    case pal_event_type_error:
    case pal_event_type_destroy:
    case pal_event_type_close:
        break;
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
    int32_t result;
    struct kevent evt;
    pal_kqueue_event_t* ev_data;
    uint32_t plat_event;

    ev_data = (pal_kqueue_event_t*)event_handle;
    if (!ev_data)
        return er_fault;

    plat_event = pal_event_type_to_kqueue_event(event_type);
    if (!plat_event)
        return er_ok;

    lock_enter(ev_data->port->lock);

    EV_SET(&evt, (uint64_t)ev_data->sock_fd,
        plat_event, EV_ADD | EV_CLEAR | EV_RECEIPT, 0, 0, ev_data);
    if (0 != kevent(ev_data->port->kqueue_fd, &evt, 1, NULL, 0, NULL))
        result = pal_os_last_net_error_as_prx_error();
    else
        result = er_ok;
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
    struct kevent evt;
    pal_kqueue_event_t* ev_data;
    uint32_t plat_event;

    ev_data = (pal_kqueue_event_t*)event_handle;
    if (!ev_data)
        return er_fault;

    plat_event = pal_event_type_to_kqueue_event(event_type);
    if (!plat_event)
        return er_ok;

    lock_enter(ev_data->port->lock);

    EV_SET(&evt, (uint64_t)ev_data->sock_fd,
        plat_event, EV_DELETE | EV_RECEIPT, 0, 0, ev_data);
    if (0 != kevent(ev_data->port->kqueue_fd, &evt, 1, NULL, 0, NULL))
        result = pal_os_last_net_error_as_prx_error();

    lock_exit(ev_data->port->lock);
    return result;
}

//
// Closes the event
//
void pal_event_close(
    uintptr_t event_handle,
    bool close_fd
)
{
    struct kevent events[2];
    pal_kqueue_event_t* ev_data;
    pal_kqueue_port_t* pal_port;

    if (!event_handle)
        return;

    ev_data = (pal_kqueue_event_t*)event_handle;
    ev_data->close_fd = close_fd;
    pal_port = ev_data->port;

    lock_enter(pal_port->lock);
    EV_SET(&events[0], (uint64_t)ev_data->sock_fd,
        EVFILT_WRITE, EV_DELETE | EV_RECEIPT, 0, 0, ev_data);
    EV_SET(&events[1], (uint64_t)ev_data->sock_fd,
        EVFILT_READ, EV_DELETE | EV_RECEIPT, 0, 0, ev_data);

    if (0 != kevent(pal_port->kqueue_fd, events, 2, NULL, 0, NULL))
    {
        log_error(NULL, "Failed to delete event data from kqueue port");
    }

    lock_exit(pal_port->lock);
    pal_kqueue_event_free(ev_data);
}

//
// Create vector to track events
//
int32_t pal_event_port_create(
    uintptr_t* port
)
{
    int32_t result;
    pal_kqueue_port_t* pal_port;
    if (!port)
        return er_fault;

    pal_port = mem_zalloc_type(pal_kqueue_port_t);
    if (!pal_port)
        return er_out_of_memory;
    do
    {
        pal_port->kqueue_fd = -1;
        result = lock_create(&pal_port->lock);
        if (result != er_ok)
            break;

        pal_port->kqueue_fd = kqueue();
        if (pal_port->kqueue_fd == -1)
        {
            result = er_out_of_memory;
            break;
        }

        // Start event loop thread
        pal_port->running = true;
        if (THREADAPI_OK != ThreadAPI_Create(
            &pal_port->thread, pal_kqueue_worker_thread, pal_port))
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
    pal_kqueue_port_t* pal_port = (pal_kqueue_port_t*)port;
    if (pal_port)
    {
        pal_port->running = false;
        if (pal_port->thread)
            ThreadAPI_Join(pal_port->thread, &result);

        if (pal_port->kqueue_fd != -1)
        {
            lock_enter(pal_port->lock);
            (void)close(pal_port->kqueue_fd);
            pal_port->kqueue_fd = -1;
            lock_exit(pal_port->lock);
        }

        if (pal_port->lock)
            lock_free(pal_port->lock);

        mem_free_type(pal_kqueue_port_t, pal_port);
    }
}
