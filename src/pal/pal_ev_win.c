// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os.h"
#include "util_mem.h"
#include "pal.h"
#include "pal_ev.h"
#include "pal_mt.h"
#include "pal_net.h"
#include "pal_err.h"

#include "azure_c_shared_utility/doublylinkedlist.h"

//
// No need for port, we run on global thread pool
//
#define EVENT_PORT_HANDLE 0x12345678

//
// Event data info for registered socket
//
typedef struct pal_event_data
{
    DLIST_ENTRY link;
    SOCKET sock;                           // The registered socket handle
    HANDLE net_event;       
    HANDLE wait_object;
    long events;                    // Currently enabled events for socket
    pal_event_port_handler_t cb;              // Registered event callback
    void* context;                              // ...and callback context
    lock_t read_lock;           // synchronize read event callback from tp
    lock_t write_lock;                 // synchronize write event callback
}
pal_event_data_t;

//
// Convert a socket event type to network event type
//
static long pal_socket_to_fd_event_type(
    uint32_t events
)
{
    switch (events)
    {
    case pal_event_type_read:
        return (FD_READ | FD_ACCEPT | FD_CLOSE);
    case pal_event_type_write:
        return FD_WRITE;
    case pal_event_type_close:
    case pal_event_type_error:
    default:
        return 0;
    }
}

//
// Callback called on fire/timeout for registered wait
//
static void CALLBACK pal_wait_for_socket_events_callback(
    void* context,
    BOOLEAN timeout
)
{
    WSANETWORKEVENTS events;
    pal_event_data_t* ev_data = (pal_event_data_t*)context;

    (void)timeout;
    dbg_assert(!timeout, "INFINITE timeout timed out!");

    if (ev_data->events != 0 &&
        SOCKET_ERROR != WSAEnumNetworkEvents(ev_data->sock, ev_data->net_event, &events))
    {
        if (0 != (events.lNetworkEvents & FD_ACCEPT))
        {
            lock_enter(ev_data->read_lock);

            ev_data->cb(ev_data->context,
                pal_event_type_read, events.iErrorCode[FD_ACCEPT_BIT]);

            lock_exit(ev_data->read_lock);
        }
        
        if (0 != (events.lNetworkEvents & FD_READ))
        {
            lock_enter(ev_data->read_lock);
            if (events.iErrorCode[FD_READ_BIT] != 0)
            {
                ev_data->cb(ev_data->context,
                    pal_event_type_read, events.iErrorCode[FD_READ_BIT]);
            }
            else
            {
                while (er_ok == ev_data->cb(
                    ev_data->context, pal_event_type_read, er_ok) &&
                    (ev_data->events & FD_READ))
                {
                    // Read as much as possible
                }
            }
            lock_exit(ev_data->read_lock);
        }

        if (0 != (events.lNetworkEvents & FD_WRITE))
        {
            lock_enter(ev_data->write_lock);
            if (events.iErrorCode[FD_WRITE_BIT] != 0)
            {
                ev_data->cb(ev_data->context,
                    pal_event_type_write, events.iErrorCode[FD_WRITE_BIT]);
            }
            else
            {
                while (er_ok == ev_data->cb(
                    ev_data->context, pal_event_type_write, er_ok) &&
                    (ev_data->events & FD_WRITE))
                {
                    // Write as much as possible
                }
            }
            lock_exit(ev_data->write_lock);
        }

        if (0 != (events.lNetworkEvents & FD_CLOSE))
        {
            lock_enter(ev_data->read_lock);
            while (er_ok == ev_data->cb(ev_data->context, 
                pal_event_type_read, er_ok) &&
                (ev_data->events & FD_READ))
            {
                // Read as much as possible
            }
            lock_exit(ev_data->read_lock);

            ev_data->cb(ev_data->context,
                pal_event_type_close, events.iErrorCode[FD_CLOSE_BIT]);
        }
    }
    else
    {
        log_error(NULL, "Failure enumerating network events (%s).",
            prx_err_string(pal_os_last_net_error_as_prx_error()));
    }
}

//
// Create event port
//
int32_t pal_event_port_create(
    pal_timeout_handler_t timeout_handler,
    void* context,
    uintptr_t* port
)
{
    chk_arg_fault_return(port);
    if (timeout_handler)
        return er_not_supported;
    (void)context;
    *port = EVENT_PORT_HANDLE;
    return er_ok;
}

//
// Free the event port and vector
//
void pal_event_port_close(
    uintptr_t port
)
{
    (void)port;
    dbg_assert(port == EVENT_PORT_HANDLE, "Wrong port");
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
    pal_event_data_t* ev_data;

    (void)port;
    dbg_assert(port == EVENT_PORT_HANDLE, "Wrong port");
    chk_arg_fault_return(event_handle);
    chk_arg_fault_return(cb);
    if (sock == _invalid_fd)
        return er_arg;

    ev_data = mem_zalloc_type(pal_event_data_t);
    if (!ev_data)
        return er_out_of_memory;
    do
    {
        ev_data->sock = (SOCKET)sock;
        ev_data->context = context;
        ev_data->cb = cb;
        ev_data->wait_object = INVALID_HANDLE_VALUE;

        result = lock_create(&ev_data->read_lock);
        if (result != er_ok)
            break;
        result = lock_create(&ev_data->write_lock);
        if (result != er_ok)
            break;

        ev_data->net_event = WSACreateEvent();
        if (ev_data->net_event == INVALID_HANDLE_VALUE)
        {
            result = er_out_of_memory;
            break;
        }

        if (!RegisterWaitForSingleObject(&ev_data->wait_object, 
            ev_data->net_event, pal_wait_for_socket_events_callback, ev_data, 
            INFINITE, WT_EXECUTEDEFAULT))
        {
            result = pal_os_last_error_as_prx_error();
            break;
        }

        log_debug(NULL, "Added event for %x.", (int32_t)sock);
        *event_handle = (uintptr_t)ev_data;
        return er_ok;

    } while (0);

    pal_event_close((uintptr_t)ev_data, false);
    return result;
}

//
// Register interest in a certain type of event
//
int32_t pal_event_select(
    uintptr_t event_handle,
    pal_event_type_t event_type
)
{
    pal_event_data_t* ev_data = (pal_event_data_t*)event_handle;
    long plat_event;
    chk_arg_fault_return(ev_data);
    plat_event = pal_socket_to_fd_event_type(event_type);
    if (!plat_event)
        return er_ok;
    if (plat_event != (ev_data->events & plat_event))
    {
        ev_data->events |= plat_event;

        if (SOCKET_ERROR == WSAEventSelect(ev_data->sock, ev_data->net_event, ev_data->events))
        {
            return pal_os_last_net_error_as_prx_error();
        }

        if (0 != (plat_event & FD_WRITE))
        {
            //
            // FD_WRITE can be assumed set, until the first error. When we 
            // enable write just try to submit as much data as possible
            //
            while (er_ok == ev_data->cb(
                ev_data->context, pal_event_type_write, er_ok) &&
                (ev_data->events & FD_WRITE))
            {
                // Write as much as possible
            }
        }
    }
    return er_ok;
}

//
// Clear interest in event
//
int32_t pal_event_clear(
    uintptr_t event_handle,
    pal_event_type_t event_type
)
{
    pal_event_data_t* ev_data = (pal_event_data_t*)event_handle;
    long plat_event = pal_socket_to_fd_event_type(event_type);
    chk_arg_fault_return(ev_data);
    if (!plat_event)
        return er_ok;
    if (plat_event == (ev_data->events & plat_event))
    {
        ev_data->events &= ~plat_event;

        if (SOCKET_ERROR == WSAEventSelect(ev_data->sock, ev_data->net_event, ev_data->events))
        {
            return pal_os_last_net_error_as_prx_error();
        }
    }
    return er_ok;
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
    HANDLE complete;
    pal_event_data_t* ev_data = (pal_event_data_t*)event_handle;
    if (!ev_data)
        return;

    if (ev_data->net_event != INVALID_HANDLE_VALUE)
    {
        ev_data->events = 0;
        if (ev_data->sock != INVALID_SOCKET &&
            SOCKET_ERROR == WSAEventSelect(ev_data->sock, ev_data->net_event, ev_data->events))
        {
            result = pal_os_last_net_error_as_prx_error();
            log_error(NULL, "Failed to deselect event (%s). ", prx_err_string(result));
        }
    }

    if (ev_data->wait_object != INVALID_HANDLE_VALUE)
    {
        complete = WSACreateEvent();
        (void)UnregisterWaitEx(ev_data->wait_object, complete);

        if (complete != INVALID_HANDLE_VALUE)
        {
            if (WSA_WAIT_EVENT_0 != WSAWaitForMultipleEvents(
                1, &complete, TRUE, 60000, FALSE))
            {
                log_error(NULL, "Error: Timeout UnregisterWaitEx completion notification.");
            }
            WSACloseEvent(complete);
        }
        ev_data->wait_object = complete = INVALID_HANDLE_VALUE;
    }

    if (ev_data->net_event != INVALID_HANDLE_VALUE)
    {
        WSACloseEvent(ev_data->net_event);
        ev_data->net_event = INVALID_HANDLE_VALUE;
    }

    if (close_fd)
    {
        closesocket(ev_data->sock);
        ev_data->sock = _invalid_fd;
    }

    if (ev_data->cb)
    {
        ev_data->cb(ev_data->context, pal_event_type_destroy, er_ok);
        ev_data->cb = NULL;
    }

    if (ev_data->read_lock)
        lock_free(ev_data->read_lock);
    if (ev_data->write_lock)
        lock_free(ev_data->write_lock);

    mem_free_type(pal_event_data_t, ev_data);
}
