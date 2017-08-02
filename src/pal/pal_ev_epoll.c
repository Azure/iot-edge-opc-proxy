// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os.h"
#include "util_mem.h"
#include "pal_mt.h"
#include "pal_ev.h"
#include "pal_net.h"
#include "pal.h"
#include "util_misc.h"
#include "azure_c_shared_utility/doublylinkedlist.h"
#include "azure_c_shared_utility/threadapi.h"

#if !defined(UNIT_TEST)
#include <sys/epoll.h>
#endif

//
// A port based on epoll. A socketpair is used for controling
// the epoll thread(s) and adding/removing/destroying events.
//
typedef struct pal_epoll_port
{
    int epoll_fd;
    fd_t control_fd[2];         // Socket pair for control
    THREAD_HANDLE thread;             // Main polling loop
    bool running;
    atomic_t num_events;
    tid_t self;
    pal_timeout_handler_t cb;
    void* context;
    log_t log;
}
pal_epoll_port_t;

//
// Poll event contains context and callback
//
typedef struct pal_epoll_event
{
    fd_t sock_fd;
    uint32_t events;
    lock_t lock;
    bool close_fd;
    pal_epoll_port_t* port;
    pal_event_port_handler_t cb;
    void* context;
}
pal_epoll_event_t;

//
// Control messages
//
typedef struct pal_epoll_ctl
{
    int op;
    struct epoll_event evt;
}
pal_epoll_ctl_t;

//
// Frees the event
//
static void pal_epoll_event_free(
    pal_epoll_event_t* ev_data
)
{
    if (ev_data->lock)
        lock_enter(ev_data->lock);

    if (ev_data->close_fd && ev_data->sock_fd != _invalid_fd)
        close(ev_data->sock_fd);
    if (ev_data->lock)
    {
        lock_exit(ev_data->lock);
        lock_free(ev_data->lock);
    }
    ev_data->cb(ev_data->context, pal_event_type_destroy, 0);
    mem_free_type(pal_epoll_event_t, ev_data);
}

//
// Worker thread for the epoll fd
//
static int32_t pal_epoll_worker_thread(
    void* context
)
{
    int32_t result = er_ok;
    pal_epoll_port_t* pal_port = (pal_epoll_port_t*)context;
    struct epoll_event evt;
    pal_epoll_ctl_t ctl;
    pal_epoll_event_t* ev_data;

    dbg_assert_ptr(pal_port);
    pal_port->self = tid_self();

    while (pal_port->running)
    {
        // Wait forever for one event, result should be 1.
        result = epoll_wait(pal_port->epoll_fd, &evt, 1, !pal_port->cb ? -1 :
            pal_port->cb(pal_port->context, pal_port->num_events == 0));
        if (result == 0)
            continue;
        if (result < 1)
        {
            result = pal_os_last_net_error_as_prx_error();
            if (result != er_ok && result != er_aborted)
            {
                log_error(pal_port->log, "Error in polling thread: %s.",
                    prx_err_string(result));
                ThreadAPI_Sleep(100);
            }
            continue;
        }

        ev_data = (pal_epoll_event_t*)evt.data.ptr;
        if (!ev_data)
        {
            // Run epoll control loop
            while (0 != (evt.events & EPOLLIN))
            {
                ctl.evt.data.ptr = NULL;
                result = recv(pal_port->control_fd[1], (sockbuf_t*)&ctl,
                    sizeof(pal_epoll_ctl_t), 0);
                if (result != sizeof(pal_epoll_ctl_t))
                    break; // Done - no more control events for us.

                ev_data = (pal_epoll_event_t*)ctl.evt.data.ptr;

                dbg_assert_ptr(ev_data);
                dbg_assert(ctl.op == EPOLL_CTL_DEL || ctl.op == EPOLL_CTL_MOD,
                    "Unexpected control op %d", ctl.op);

                result = epoll_ctl(pal_port->epoll_fd, ctl.op, ev_data->sock_fd,
                    &ctl.evt);
                if (result != 0)
                {
                    result = pal_os_last_net_error_as_prx_error();
                    log_error(pal_port->log, "Failed to epoll_ctl(%d) (%s)",
                        ctl.op, prx_err_string(result));
                }

                // In case of delete, free event data and resources
                if (ctl.op == EPOLL_CTL_DEL)
                {
                    pal_epoll_event_free(ev_data);
                }
            }
        }
        else
        {
            // Dispatch events
            /**/ if (0 != (evt.events & EPOLLHUP))
                ev_data->cb(ev_data->context, pal_event_type_close, 0);
            else if (0 != (evt.events & EPOLLERR))
                ev_data->cb(ev_data->context, pal_event_type_error, 0);
            else
            {
                // Read/Write as much as possible, but do so interleaved
                while (0 != (evt.events & (EPOLLIN | EPOLLOUT)))
                {
                    if (0 != (evt.events & EPOLLIN) && er_ok != ev_data->cb(
                        ev_data->context, pal_event_type_read, 0))
                        evt.events &= ~EPOLLIN;
                    if (0 != (evt.events & EPOLLOUT) && er_ok != ev_data->cb(
                        ev_data->context, pal_event_type_write, 0))
                        evt.events &= ~EPOLLOUT;
                }

                if (0 != (evt.events & EPOLLRDHUP))
                    ev_data->cb(ev_data->context, pal_event_type_close, 0);
            }
        }
    }

    pal_port->self = (tid_t)0;
    pal_port->running = false;
    return 0;
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

        result = lock_create(&ev_data->lock);
        if (result != er_ok)
            break;

        evt.data.ptr = ev_data;
        evt.events = EPOLLET;

        _fd_nonblock(ev_data->sock_fd);

        atomic_inc(pal_port->num_events);
        if (0 != epoll_ctl(pal_port->epoll_fd, EPOLL_CTL_ADD,
            ev_data->sock_fd, &evt))
        {
            result = pal_os_last_net_error_as_prx_error();
            break;
        }

        *event_handle = (uintptr_t)ev_data;
        log_debug(pal_port->log, "Added event port for %x.",
            (int)sock_fd);
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
    pal_event_type_t event_type
)
{
    switch (event_type)
    {
    case pal_event_type_close:
        return EPOLLRDHUP;
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
    pal_event_type_t event_type
)
{
    int32_t result;
    pal_epoll_ctl_t ctl;
    pal_epoll_event_t* ev_data = (pal_epoll_event_t*)event_handle;
    uint32_t plat_event = pal_event_type_to_epoll_event(event_type);

    chk_arg_fault_return(ev_data);
    if (!plat_event)
        return er_ok;

    lock_enter(ev_data->lock);
    memset(&ctl, 0, sizeof(pal_epoll_ctl_t));
    ctl.evt.events = (ev_data->events | EPOLLET) | plat_event;
    ctl.evt.data.ptr = (void*)ev_data;
    ctl.op = EPOLL_CTL_MOD;
    result = er_ok;
    if (tid_equal(ev_data->port->self, tid_self()))
    {
        if (0 != epoll_ctl(ev_data->port->epoll_fd, ctl.op, ev_data->sock_fd, &ctl.evt))
        {
            result = pal_os_last_net_error_as_prx_error();
            log_error(ev_data->port->log, "Failed to epoll_ctl(%d) (%s)",
                ctl.op, prx_err_string(result));
        }
    }
    else if (0 > send(ev_data->port->control_fd[0], (const sockbuf_t*)&ctl,
        sizeof(pal_epoll_ctl_t), 0))
    {
        result = pal_os_last_net_error_as_prx_error();
        log_error(ev_data->port->log, "Failed to send select (%s)",
            prx_err_string(result));
    }

    if (result == er_ok)
        ev_data->events |= plat_event;
    lock_exit(ev_data->lock);
    return result;
}

//
// Clear interest in event
//
int32_t pal_event_clear(
    uintptr_t event_handle,
    pal_event_type_t event_type
)
{
    int32_t result;
    pal_epoll_ctl_t ctl;
    pal_epoll_event_t* ev_data = (pal_epoll_event_t*)event_handle;
    uint32_t plat_event = pal_event_type_to_epoll_event(event_type);

    chk_arg_fault_return(ev_data);
    if (!plat_event)
        return er_ok;

    lock_enter(ev_data->lock);
    memset(&ctl, 0, sizeof(pal_epoll_ctl_t));
    ctl.evt.events = (ev_data->events | EPOLLET) & ~plat_event;
    ctl.evt.data.ptr = (void*)ev_data;
    ctl.op = EPOLL_CTL_MOD;
    result = er_ok;
    if (tid_equal(ev_data->port->self, tid_self()))
    {
        if (0 != epoll_ctl(ev_data->port->epoll_fd, ctl.op, ev_data->sock_fd, &ctl.evt))
        {
            result = pal_os_last_net_error_as_prx_error();
            log_error(ev_data->port->log, "Failed to epoll_ctl(%d) (%s)",
                ctl.op, prx_err_string(result));
        }
    }
    else if (0 > send(ev_data->port->control_fd[0], (const sockbuf_t*)&ctl,
        sizeof(pal_epoll_ctl_t), 0))
    {
        result = pal_os_last_net_error_as_prx_error();
        log_error(ev_data->port->log, "Failed to send clear (%s)",
            prx_err_string(result));
    }

    if (result == er_ok)
        ev_data->events &= ~plat_event;
    lock_exit(ev_data->lock);
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
    int32_t result;
    pal_epoll_ctl_t ctl;
    pal_epoll_event_t* ev_data = (pal_epoll_event_t*)event_handle;
    if (!ev_data)
        return;

    lock_enter(ev_data->lock);
    ev_data->close_fd = close_fd;
    ev_data->events = 0;

    atomic_dec(ev_data->port->num_events);
    if (ev_data->port->thread)
    {
        memset(&ctl, 0, sizeof(pal_epoll_ctl_t));
        ctl.evt.data.ptr = (void*)ev_data;
        ctl.op = EPOLL_CTL_DEL;

        if (tid_equal(ev_data->port->self, tid_self()))
        {
            if (0 != epoll_ctl(ev_data->port->epoll_fd, ctl.op, ev_data->sock_fd, &ctl.evt))
            {
                result = pal_os_last_net_error_as_prx_error();
                log_error(ev_data->port->log, "Failed to epoll_ctl(%d) (%s)",
                    ctl.op, prx_err_string(result));
            }
        }
        else if (0 > send(ev_data->port->control_fd[0], (const sockbuf_t*)&ctl,
            sizeof(pal_epoll_ctl_t), 0))
        {
            result = pal_os_last_net_error_as_prx_error();
            log_error(ev_data->port->log, "Failed to send DEL to epoll port (%s).",
                prx_err_string(result));
        }
        else
        {
            lock_exit(ev_data->lock);
            return;  // Wait for epoll thread to unregister and free event
        }
    }

    lock_exit(ev_data->lock);
    pal_epoll_event_free(ev_data);
}

//
// Create vector to track events
//
int32_t pal_event_port_create(
    pal_timeout_handler_t timeout_handler,
    void* context,
    uintptr_t* port
)
{
    int32_t result;
    struct epoll_event evt;
    pal_epoll_port_t* pal_port;
    chk_arg_fault_return(port);

    pal_port = mem_zalloc_type(pal_epoll_port_t);
    if (!pal_port)
        return er_out_of_memory;
    do
    {
        pal_port->log = log_get("pal.ev");
        pal_port->cb = timeout_handler;
        pal_port->context = context;
        pal_port->epoll_fd = epoll_create1(EPOLL_CLOEXEC);
        if (pal_port->epoll_fd == -1)
        {
            result = er_out_of_memory;
            break;
        }

        // Add control sockets
        if (-1 == socketpair(AF_UNIX, SOCK_DGRAM, 0, pal_port->control_fd))
        {
            result = pal_os_last_net_error_as_prx_error();
            log_error(pal_port->log, "Failed to make control sockets (%s)",
                prx_err_string(result));
            break;
        }

        _fd_nonblock(pal_port->control_fd[0]);
        _fd_nonblock(pal_port->control_fd[1]);

        // Register control socket with epoll port
        evt.data.ptr = NULL;
        evt.events = EPOLLET | EPOLLIN | EPOLLERR | EPOLLOUT;
        if (0 != epoll_ctl(pal_port->epoll_fd, EPOLL_CTL_ADD,
            pal_port->control_fd[1], &evt))
        {
            result = pal_os_last_net_error_as_prx_error();
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
    }
    while (0);

    pal_event_port_close((uintptr_t)pal_port);
    return result;
}

//
// Stop event port
//
void pal_event_port_stop(
    uintptr_t port
)
{
    int32_t result;
    char control_char = 0;
    pal_epoll_port_t* pal_port = (pal_epoll_port_t*)port;
    if (!pal_port || !pal_port->running)
        return;

    pal_port->running = false;
    if (pal_port->control_fd[1] != _invalid_fd)
        (void)send(pal_port->control_fd[0], &control_char, 1, 0);
    if (pal_port->thread)
        ThreadAPI_Join(pal_port->thread, &result);
    pal_port->thread = NULL;
}

//
// Stop and free the event port
//
void pal_event_port_close(
    uintptr_t port
)
{
    pal_epoll_port_t* pal_port = (pal_epoll_port_t*)port;
    if (!pal_port)
        return;

    pal_event_port_stop(port);

    if (pal_port->control_fd[0] != _invalid_fd)
        (void)close(pal_port->control_fd[0]);
    if (pal_port->control_fd[1] != _invalid_fd)
        (void)close(pal_port->control_fd[1]);
    if (pal_port->epoll_fd != _invalid_fd)
        (void)close(pal_port->epoll_fd);

    mem_free_type(pal_epoll_port_t, pal_port);
}

