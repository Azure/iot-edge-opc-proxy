// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os.h"
#include "util_mem.h"
#include "pal_ev.h"
#include "pal_net.h"
#include "pal_err.h"
#include "pal_mt.h"
#include "pal.h"

#include "azure_c_shared_utility/doublylinkedlist.h"
#include "azure_c_shared_utility/threadapi.h"

//
// Poll Port is a fake event port for poll events for both
// Linux and Windows.  A socketpair is used for controling
// the poll thread and adding/removing/destroying events.
//
typedef struct pal_poll_port
{
    lock_t lock;
    DLIST_ENTRY event_data_list;      // Registered events
    atomic_t event_data_list_count;
    struct pollfd* poll_buffer;  // Buffer to pass to poll
    fd_t control_fd[2];         // Socket pair for control
    THREAD_HANDLE thread;             // Main polling loop
    bool running;
    pal_timeout_handler_t cb;
    void* context;
    log_t log;
}
pal_poll_port_t;

//
// Poll events
//
typedef struct pal_poll_event
{
    DLIST_ENTRY link;
    struct pollfd poll_struct;
    bool close_fd;
    pal_event_port_handler_t cb;
    pal_poll_port_t* port;
    void* context;
}
pal_poll_event_t;

//
// Frees the event
//
static void pal_poll_event_free(
    pal_poll_event_t* ev_data
)
{
    if (ev_data->close_fd && ev_data->poll_struct.fd != _invalid_fd)
    {
        closesocket(ev_data->poll_struct.fd);
    }
    ev_data->cb(ev_data->context, pal_event_type_destroy, 0);
    mem_free_type(pal_poll_event_t, ev_data);
}

//
// Wait for events
//
static int32_t pal_poll_event_loop(
    pal_poll_port_t* pal_port
)
{
    pal_poll_event_t* next;
    struct pollfd* poll_copy = NULL;
    size_t index, poll_len;
    DLIST_ENTRY events;
    char control_char;
    int32_t result;
    int poll_result;
    int32_t timeout;
    dbg_assert_ptr(pal_port);

    result = er_ok;
    lock_enter(pal_port->lock);
    while (pal_port->running)
    {
        DList_InitializeListHead(&events);
        DList_AppendTailList(&events, &pal_port->event_data_list);
        DList_RemoveEntryList(&pal_port->event_data_list);
        DList_InitializeListHead(&pal_port->event_data_list);
        while (!DList_IsListEmpty(&events))
        {
            next = containingRecord(DList_RemoveHeadList(&events),
                pal_poll_event_t, link);
            if (next->port)
            {
                // lock_exit to prevent deadlock in callback
                lock_exit(pal_port->lock);
                if (next->poll_struct.revents)
                {
                    if (0 != (next->poll_struct.revents & POLLRDNORM))
                        next->cb(next->context, pal_event_type_read, 0);
                    if (0 != (next->poll_struct.revents & POLLWRNORM))
                        next->cb(next->context, pal_event_type_write, 0);
                    if (0 != (next->poll_struct.revents & POLLERR))
                        next->cb(next->context, pal_event_type_error, 0);
                    if (0 != (next->poll_struct.revents & POLLHUP))
                        next->cb(next->context, pal_event_type_close, 0);
                    next->poll_struct.revents = 0;
                }
                lock_enter(pal_port->lock);
            }

            if (!next->port) // The event was closed in the callback...
            {
                // Remove entry
                atomic_dec(pal_port->event_data_list_count);
                log_trace(pal_port->log, "Removing event port for %x.",
                    (uint32_t)next->poll_struct.fd);
                lock_exit(pal_port->lock);
                pal_poll_event_free(next);
                lock_enter(pal_port->lock);
            }
            else
            {
                // Add back to event data list
                DList_InsertTailList(&pal_port->event_data_list,
                    &next->link);
            }
        }

        if (!pal_port->running)
            break;

        // No events, make a copy of the poll array
        poll_len = pal_port->event_data_list_count + 1;
        poll_copy = (struct pollfd*)mem_realloc(
            pal_port->poll_buffer, poll_len * sizeof(struct pollfd));
        if (!poll_copy)
        {
            result = er_out_of_memory;
            break;
        }
        pal_port->poll_buffer = poll_copy;

        poll_copy->events = POLLRDNORM;
        poll_copy->fd = pal_port->control_fd[1];
        poll_copy->revents = 0;
        poll_copy++;

        for (PDLIST_ENTRY p = pal_port->event_data_list.Flink;
            p != &pal_port->event_data_list; p = p->Flink)
        {
            next = containingRecord(p, pal_poll_event_t, link);
            memcpy(poll_copy++, &next->poll_struct, sizeof(struct pollfd));
        }

#define POLL_TIMEOUT_MAX (10 * 60 * 1000)
        if (!pal_port->cb)
            timeout = POLL_TIMEOUT_MAX;
        else
            timeout = pal_port->cb(pal_port->context, pal_port->event_data_list_count == 0);

        // Wait for the first event, do not hold the lock while waiting
        lock_exit(pal_port->lock);
        poll_result = poll(pal_port->poll_buffer, (unsigned long)poll_len, timeout);
        lock_enter(pal_port->lock);

        if (poll_result < 0)
        {
            result = pal_os_last_net_error_as_prx_error();
            log_error(pal_port->log, "Error returned during polling (%s).",
                prx_err_string(result));
            if (result != er_ok && result != er_arg)
                break;
            result = er_ok;
        }

        // Clear control char in control socket pair
        if (pal_port->poll_buffer[0].revents != 0)
        {
            log_debug(pal_port->log, "Clearing event control channel.");
            recv(pal_port->control_fd[1], &control_char, 1, 0);
        }

        // Copy back revents
        index = 1;
        for (PDLIST_ENTRY p = pal_port->event_data_list.Flink;
            p != &pal_port->event_data_list && index < poll_len;
            p = p->Flink, index++)
        {
            next = containingRecord(p, pal_poll_event_t, link);
            dbg_assert(next->poll_struct.revents == 0,
                "Poll struct and port out of sync");
            dbg_assert(next->poll_struct.fd == pal_port->poll_buffer[index].fd,
                "fd out of sync");

            next->poll_struct.revents = pal_port->poll_buffer[index].revents;
            log_debug(pal_port->log, "Event %x on poll socket %x.",
                next->poll_struct.revents, (int)next->poll_struct.fd);
        }
    }
    lock_exit(pal_port->lock);
    log_debug(pal_port->log, "Exit event port polling loop (%s).",
        prx_err_string(result));
    return result;
}

//
// Worker thread for the socket layer
//
static int32_t pal_poll_worker_thread(
    void* context
)
{
    int32_t result = er_closed;
    pal_poll_port_t* pal_port = (pal_poll_port_t*)context;

    while (pal_port->running)
    {
        result = pal_poll_event_loop(pal_port);
        if (result != er_ok)
        {
            log_error(pal_port->log, "Error occurred in polling thread: %s.",
                prx_err_string(result));
            ThreadAPI_Sleep(100);
        }
    }

    pal_port->running = false;
    return result;
}

//
// Signal control socket - assert done under port lock
//
static int32_t pal_poll_signal(
    pal_poll_port_t* pal_port
)
{
    int32_t result = er_ok;
    char control_char = 0xc;
    dbg_assert_ptr(pal_port);
    dbg_assert(pal_port->control_fd[0] != _invalid_fd, "No control fd");
    if (1 != send(pal_port->control_fd[0], &control_char, 1, 0))
    {
        result = pal_os_last_error_as_prx_error();
        log_error(pal_port->log, "Failed sending control signal (%s).",
            prx_err_string(result));
    }
    return result;
}

//
// Register event callback
//
int32_t pal_event_port_register(
    uintptr_t port,
    intptr_t sock,
    pal_event_port_handler_t cb,
    void* context,
    uintptr_t* event_handle
)
{
    int32_t result;
    pal_poll_event_t* ev_data;
    pal_poll_port_t* pal_port = (pal_poll_port_t*)port;

    chk_arg_fault_return(pal_port);
    chk_arg_fault_return(cb);
    chk_arg_fault_return(event_handle);
    if (sock == _invalid_fd)
        return er_arg;

    ev_data = mem_zalloc_type(pal_poll_event_t);
    if (!ev_data)
        return er_out_of_memory;

    ev_data->poll_struct.fd = (fd_t)sock;
    ev_data->cb = cb;
    ev_data->context = context;
    ev_data->port = pal_port;

    result = 1;
    _fd_nonblock(ev_data->poll_struct.fd);

    lock_enter(pal_port->lock);
    DList_InsertTailList(&pal_port->event_data_list, &ev_data->link);
    atomic_inc(pal_port->event_data_list_count);
    (void)pal_poll_signal(pal_port);
    lock_exit(pal_port->lock);

    *event_handle = (uintptr_t)ev_data;

    log_debug(pal_port->log, "Added event port for %x.",
        (int32_t)sock);
    return er_ok;
}

//
// Convert a poll event type to socket event type
//
static uint32_t pal_event_type_to_poll_event(
    pal_event_type_t event_type
)
{
    switch (event_type)
    {
    case pal_event_type_read:
        return POLLRDNORM;
    case pal_event_type_write:
        return POLLWRNORM;
    case pal_event_type_close:
#if defined(WIN32)
    case pal_event_type_error:
        // Windows does not support select error/hangup
        return 0;
#else
        return POLLHUP;
    case pal_event_type_error:
        return POLLERR;
#endif
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
    pal_poll_event_t* ev_data;
    uint32_t plat_event;

    ev_data = (pal_poll_event_t*)event_handle;
    chk_arg_fault_return(ev_data);
    plat_event = pal_event_type_to_poll_event(event_type);
    if (!plat_event)
        return er_ok;
    if (plat_event == (ev_data->poll_struct.events & plat_event))
        return er_ok; // Already set

    lock_enter(ev_data->port->lock);
    ev_data->poll_struct.events |= plat_event;
    result = pal_poll_signal(ev_data->port);
    lock_exit(ev_data->port->lock);
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
    pal_poll_event_t* ev_data;
    uint32_t plat_event;

    ev_data = (pal_poll_event_t*)event_handle;
    chk_arg_fault_return(ev_data);
    plat_event = pal_event_type_to_poll_event(event_type);
    if (!plat_event)
        return er_ok;
    if (plat_event != (ev_data->poll_struct.events & plat_event))
        return er_ok; // Already cleared

    lock_enter(ev_data->port->lock);
    ev_data->poll_struct.events &= ~plat_event;
    result = pal_poll_signal(ev_data->port);
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
    pal_poll_event_t* ev_data;
    pal_poll_port_t* pal_port;

    ev_data = (pal_poll_event_t*)event_handle;
    if (!ev_data || !ev_data->port)
        return;

    pal_port = ev_data->port;
    lock_enter(pal_port->lock);
    ev_data->port = NULL;
    ev_data->close_fd = close_fd;
    ev_data->poll_struct.events = 0;
    if (pal_port->thread)
    {
        (void)pal_poll_signal(pal_port);
        lock_exit(pal_port->lock);
        return;
    }
    lock_exit(pal_port->lock);
    pal_poll_event_free(ev_data);
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
    pal_poll_port_t* pal_port;
    chk_arg_fault_return(port);

    pal_port = mem_zalloc_type(pal_poll_port_t);
    if (!pal_port)
        return er_out_of_memory;
    do
    {
        DList_InitializeListHead(&pal_port->event_data_list);
        pal_port->log = log_get("pal.ev");
        pal_port->cb = timeout_handler;
        pal_port->context = context;
        pal_port->event_data_list_count = 0;
        pal_port->control_fd[0] = _invalid_fd;
        pal_port->control_fd[1] = _invalid_fd;

        result = lock_create(&pal_port->lock);
        if (result != er_ok)
            break;

        // Add control sockets
        if (-1 == socketpair(AF_UNIX, SOCK_STREAM, 0, pal_port->control_fd))
        {
            result = pal_os_last_error_as_prx_error();
            log_error(pal_port->log, "Failed to make control sockets (%s)",
                prx_err_string(result));
            break;
        }

        _fd_nonblock(pal_port->control_fd[0]);
        _fd_nonblock(pal_port->control_fd[1]);

        result = pal_poll_signal(pal_port);
        if (result != er_ok)
            break;

        // Start event loop thread
        pal_port->running = true;
        if (THREADAPI_OK != ThreadAPI_Create(
            &pal_port->thread, pal_poll_worker_thread, pal_port))
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
    pal_poll_port_t* pal_port = (pal_poll_port_t*)port;
    if (!pal_port)
        return;

    pal_port->running = false;
    if (pal_port->control_fd[0] != _invalid_fd)
        pal_poll_signal(pal_port);
    if (pal_port->thread)
        ThreadAPI_Join(pal_port->thread, &result);
    pal_port->thread = NULL;
}

//
// Free the event port
//
void pal_event_port_close(
    uintptr_t port
)
{
    pal_poll_port_t* pal_port = (pal_poll_port_t*)port;
    if (!pal_port)
        return;

    pal_event_port_stop(port);

    if (pal_port->control_fd[0] != _invalid_fd)
        closesocket(pal_port->control_fd[0]);
    if (pal_port->control_fd[1] != _invalid_fd)
        closesocket(pal_port->control_fd[1]);
    if (pal_port->poll_buffer)
        mem_free(pal_port->poll_buffer);

    dbg_assert(DList_IsListEmpty(&pal_port->event_data_list),
        "Leaking events registered when closing event port!");
    if (pal_port->lock)
        lock_free(pal_port->lock);

    mem_free_type(pal_poll_port_t, pal_port);
}

