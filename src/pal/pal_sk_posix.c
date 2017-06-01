// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os.h"
#include "util_mem.h"
#include "pal.h"
#include "pal_sk.h"
#include "pal_net.h"
#include "pal_ev.h"
#include "pal_mt.h"
#include "pal_types.h"
#include "util_string.h"

// #define ASYNC_CONNECT 1

#define MAX_SOCKET_ADDRESS_BYTES 127

static uintptr_t event_port = 0;

//
// State of socket for open and close calls
//
typedef enum pal_socket_state
{
    pal_socket_state_closed,
    pal_socket_state_opening,
    pal_socket_state_open,
    pal_socket_state_closing
}
pal_socket_state_t;

//
// Handles event port events
//
typedef int32_t(*pal_handler_t)(
    pal_socket_t* sock,
    int32_t result
    );

//
// Represents a socket and handles socket specific requests
//
struct pal_socket
{
    pal_socket_client_itf_t itf;                 // Client interface
    fd_t sock_fd;                   // Real underlying socket handle
    uintptr_t event_handle;

    pal_handler_t read_cb;                    // Handles read events 
    pal_handler_t write_cb;                  // Handles write events
    pal_socket_state_t state;                 // State of the socket

    prx_socket_address_t peer;                // Cached peer address
    void* context;

    prx_addrinfo_t* prx_ai;  // For async connect save the ai result
    size_t prx_ai_count;       // Size of the resolved addresses
    size_t prx_ai_cur;             // Current address to connect
    log_t log;
};

//
// Close event handle and clear file descriptor 
//
void pal_socket_clear(
    pal_socket_t* sock
)
{
    if (sock->event_handle != 0)
    {
        log_trace(sock->log, "Clearing socket (fd:%d)", sock->sock_fd);
        // Event handle closes file descripter
        pal_event_close(sock->event_handle, true);
        sock->event_handle = 0;
    }
    else if (sock->sock_fd != _invalid_fd)
    {
        close(sock->sock_fd);
    }
    sock->sock_fd = _invalid_fd;
}

//
// Called when close completes - terminal state for socket
//
static void pal_socket_complete_close(
    pal_socket_t* sock
)
{
    size_t size;
    void* context;
    if (sock->state != pal_socket_state_closing)
        return;

    context = sock->context;
    sock->context = NULL;
    sock->state = pal_socket_state_closed;
    sock->sock_fd = _invalid_fd;

    size = sizeof(pal_socket_t*);
    sock->itf.cb(sock->itf.context, pal_socket_event_closed,
        (uint8_t**)&sock, &size, NULL, NULL, er_ok, &context);
}

//
// Called when open completes - terminal state for begin open
//
static void pal_socket_open_complete(
    pal_socket_t* sock,
    int32_t result
)
{
    size_t size;
    dbg_assert_ptr(sock);

    if (result == er_ok)
        sock->state = pal_socket_state_open;
    else
        sock->state = pal_socket_state_closed;

    // Complete open
    size = sizeof(pal_socket_t*);
    sock->itf.cb(sock->itf.context, pal_socket_event_opened,
        (uint8_t**)&sock, &size, NULL, NULL, result, &sock->context);
    sock->context = NULL;

    if (!sock->prx_ai)
        return;

    pal_freeaddrinfo(sock->prx_ai);
    sock->prx_ai = NULL;
    sock->prx_ai_count = 0;
    sock->prx_ai_cur = 0;
}

//
// Called when socket becomes writable during opening state 
//
static int32_t pal_socket_connect_finish(
    pal_socket_t* sock,
    int32_t result
)
{
    int error;
    uint8_t sa_in[MAX_SOCKET_ADDRESS_BYTES];
    socklen_t len = sizeof(error);
    dbg_assert_ptr(sock);

    while (result == er_ok)
    {
        // Check error on socket
        len = sizeof(error);
        if (0 != getsockopt(sock->sock_fd, SOL_SOCKET, SO_ERROR,
            (sockbuf_t*)&error, &len))
        {
            log_error(sock->log, "Could not get error (fd:%d)", sock->sock_fd);
            result = er_fatal;
            break;
        }

        if (error != 0)
        {
            // Connection failed
            log_error(sock->log, "Socket connecting resulted in error %d (fd:%d)", 
                error, sock->sock_fd);
            result = pal_os_last_net_error_as_prx_error();
            break;
        }

        // Reset timeout on socket
#if defined(TCP_USER_TIMEOUT)
        (void)setsockopt (sock->sock_fd, SOL_TCP, TCP_USER_TIMEOUT, 
            (char*)&error, sizeof(error));
#endif
        // Get peer address
        len = sizeof(sa_in);
        error = getpeername(sock->sock_fd, (struct sockaddr*)sa_in, &len);
        if (error != 0)
        {
            result = pal_os_last_net_error_as_prx_error();
            break;
        }
        result = pal_os_to_prx_socket_address(
            (const struct sockaddr*)sa_in, len, &sock->peer);
           
        break;
    }

    if (result != er_ok)
        pal_socket_clear(sock);
    return result;
}

//
// Dummy callback
//
static int32_t pal_socket_dummy_cb(
    pal_socket_t* sock,
    int32_t result
)
{
    (void)sock;
    return result;
}

//
// Called by event port when event occurred on registered socket
//
static int32_t pal_socket_event_callback(
    void* context,
    pal_event_type event_type,
    int32_t error_code
)
{
    int error;
    socklen_t len = sizeof(error);
    pal_socket_t* sock = (pal_socket_t*)context;

    dbg_assert_ptr(sock);
    switch (event_type)
    {
    case pal_event_type_read:
        // Ready to read, perform read operation
        return sock->read_cb(sock, error_code);
    case pal_event_type_write:
        // Ready to write, perform write
        return sock->write_cb(sock, error_code);
    case pal_event_type_close:
        // Remote side disconnected
        return sock->read_cb(sock, er_closed);
    case pal_event_type_error:
        // Socket error, indicate through recv
        if (0 != getsockopt(sock->sock_fd, SOL_SOCKET, SO_ERROR, (sockbuf_t*)&error, &len))
            return er_fatal;
        return sock->read_cb(sock, pal_os_to_prx_net_error(error));
    case pal_event_type_destroy:
        // Complete close if closing
        sock->sock_fd = _invalid_fd;
        pal_socket_complete_close(sock);
        return er_ok;
    case pal_event_type_unknown:
    default:
        dbg_assert(0, "Unknown event type %d", event_type);
        return er_bad_state;
    }
}

//
// On failure try next address
//
static void pal_socket_try_next(
    pal_socket_t *sock
);

//
// Called when socket became writable after connect call
//
static int32_t pal_socket_on_connected(
    pal_socket_t* sock,
    int32_t result
)
{
    dbg_assert_ptr(sock);

    if (sock->state != pal_socket_state_opening)
        return er_bad_state;
#if defined(ASYNC_CONNECT)
    if (result == er_closed)
        return er_ok;  // TODO: Need to check socket error here...
#endif
    // Complete connection
    result = pal_socket_connect_finish(sock, result);
    if (result == er_ok)
    {
        pal_socket_open_complete(sock, result);
        // Success!
        log_trace(sock->log, "Socket connected asynchronously!");
    }
    else
    {
        log_error(sock->log,
            "Failed to connect socket, continue... (%s)",
            prx_err_string(result));

        // Continue with next address
        pal_socket_try_next(sock);
    }
    return er_ok;
}

// 
// accept a new connected socket to a remote address
//
static int32_t pal_socket_on_accept(
    pal_socket_t* sock,
    int32_t result
)
{
    pal_socket_t* accepted;
    uint8_t sa_in[MAX_SOCKET_ADDRESS_BYTES];
    socklen_t sa_len = sizeof(sa_in);
    uint8_t* buffer;
    size_t buf_len;
    void* context;

    if (!event_port)
        return er_bad_state;
    if (sock->state != pal_socket_state_open)
        return er_closed;

    // Call receive and get a client socket option to create new socket with
    sock->itf.cb(sock->itf.context, pal_socket_event_begin_accept,
        &buffer, &buf_len, NULL, NULL, er_ok, &context);
    if (!buffer || buf_len != sizeof(pal_socket_client_itf_t))
    {
        // Done accepting
        pal_socket_can_recv(sock, false);
        return er_aborted;
    }
    do
    {
        if (result != er_ok)
            break;
        
        // Create new socket object to accept with
        result = pal_socket_create((pal_socket_client_itf_t*)buffer, &accepted);
        if (result != er_ok)
        {
            log_error(sock->log, "Failed to create Socket object. (%s)",
                prx_err_string(result));
            break;
        }

        // Get fd
        accepted->sock_fd = accept(sock->sock_fd, (struct sockaddr*)sa_in, &sa_len);
        if (accepted->sock_fd == _invalid_fd)
        {
            result = pal_os_last_net_error_as_prx_error();
            break;
        }

        result = pal_os_to_prx_socket_address((const struct sockaddr*)sa_in, 
            sa_len, &accepted->peer);
        if (result != er_ok)
        {
            log_error(NULL, "accept received invalid socket address (%s)",
                prx_err_string(result));
            return er_retry;
        }

        // Register with event port, will make socket nonblocking
        result = pal_event_port_register(event_port, accepted->sock_fd, 
            pal_socket_event_callback, accepted, &accepted->event_handle);
        if (result != er_ok)
            break;
        result = pal_event_select(accepted->event_handle, pal_event_type_close);
        if (result != er_ok)
            break;
    } 
    while (0);

    buf_len = sizeof(pal_socket_t*);
    sock->itf.cb(sock->itf.context, pal_socket_event_end_accept,
        (uint8_t**)&accepted, &buf_len, NULL, NULL, result, &context);

    if (result != er_ok)
    {
        pal_socket_clear(accepted);
        pal_socket_free(accepted);
    }
    else
    {
        // Open accepted socket
        accepted->context = context;
        pal_socket_open_complete(accepted, result);
    }
    return result;
}

// 
// Socket ready to call recvfrom on
//
static int32_t pal_socket_on_recvfrom(
    pal_socket_t* sock,
    int32_t result
)
{
    prx_socket_address_t sa;
    uint8_t sa_in[MAX_SOCKET_ADDRESS_BYTES];
    socklen_t sa_len = sizeof(sa_in);
    int os_flags = 0;
    int32_t flags;
    uint8_t* buffer;
    size_t buf_len;
    void* context;

    if (!event_port)
        return er_bad_state;
    if (sock->state != pal_socket_state_open)
        return er_closed;
    
    sock->itf.cb(sock->itf.context, pal_socket_event_begin_recv,
        &buffer, &buf_len, NULL, NULL, er_ok, &context);
    if (!buffer)
    {
        // Done receiving
        pal_socket_can_recv(sock, false);
        return er_aborted;
    }
    do
    {
        if (result != er_ok)
            break;
        
        // TODO: Use recvmsg to retrieve flags
        result = recvfrom(sock->sock_fd, (char*)buffer, (int)buf_len, os_flags, 
            (struct sockaddr*)sa_in, &sa_len);
        if (result < 0)
        {
            result = pal_os_last_net_error_as_prx_error();
            if (result != er_retry)
            {
                log_error(sock->log, "Recvfrom error %s.",
                    prx_err_string(result));
            }
            break;
        }
        else
        {
            buf_len = result;
            result = er_ok;
        }

        result = pal_os_to_prx_message_flags(os_flags, &flags);
        if (result != er_ok)
            break;
        result = pal_os_to_prx_socket_address(
            (const struct sockaddr*)sa_in, sa_len, &sa);
        if (result != er_ok)
            break;

        log_debug(sock->log, "Recvfrom: %zu bytes...", buf_len);
        break;
    } while (0);

    if (result != er_ok)
        buf_len = 0;

    sock->itf.cb(sock->itf.context, pal_socket_event_end_recv,
        &buffer, &buf_len, &sa, &flags, result, &context);
    return result;
}

// 
// recv from a connected socket
//
static int32_t pal_socket_on_recv(
    pal_socket_t* sock,
    int32_t result
)
{
    int os_flags = 0;
    int32_t flags;
    uint8_t* buffer;
    size_t buf_len;
    void* context;

    if (!event_port)
        return er_bad_state;
    // If still opening, finish connect
    /**/ if (sock->state == pal_socket_state_opening)
        return pal_socket_on_connected(sock, result);
    else if (sock->state != pal_socket_state_open)
        return er_closed;
    else if (result == er_closed)
        sock->state = pal_socket_state_closing;
        
    sock->itf.cb(sock->itf.context, pal_socket_event_begin_recv,
        &buffer, &buf_len, NULL, NULL, er_ok, &context);
    if (!buffer)
    {
        // Done receiving
        pal_socket_can_recv(sock, false);
        return er_aborted;
    }
    do
    {
        if (result != er_ok)
            break;

        result = recv(sock->sock_fd, (char*)buffer, (int)buf_len, os_flags);
        if (result < 0)
        {
            result = pal_os_last_net_error_as_prx_error();
            if (result != er_retry)
            {
                log_error(sock->log, "Recv error %s.",
                    prx_err_string(result));
            }
            break;
        }
        else
        {
            buf_len = result;
            result = er_ok;
        }

        result = pal_os_to_prx_message_flags(os_flags, &flags);
        if (result != er_ok)
            break;
        log_debug(sock->log, "Recv: %zu bytes...", buf_len);
        break;
    } while (0);

    if (result != er_ok)
        buf_len = 0;

    sock->itf.cb(sock->itf.context, pal_socket_event_end_recv,
        &buffer, &buf_len, NULL, &flags, result, &context);
    return result;
}

// 
// send over a connected socket to target address
//
static int32_t pal_socket_on_send(
    pal_socket_t* sock,
    int32_t result
)
{
    int os_flags = 0;
    int32_t flags;
    uint8_t* buffer;
    size_t buf_len;
    void* context;

    if (!event_port)
        return er_bad_state;
    /**/ if (sock->state == pal_socket_state_opening)
    {
        // If still opening, finish connect
        result = pal_socket_on_connected(sock, result);
        if (result != er_ok)
            return result;
    }
    else if (sock->state != pal_socket_state_open)
        return er_closed;

    sock->itf.cb(sock->itf.context, pal_socket_event_begin_send,
        &buffer, &buf_len, NULL, &flags, er_ok, &context);
    if (!buffer)
    {
        // Done sending
        pal_socket_can_send(sock, false);
        return er_aborted;
    }
    do
    {
        if (result != er_ok)
            break;

        result = pal_os_from_prx_message_flags(flags, &os_flags);
        if (result != er_ok)
        {
            log_error(sock->log, "Send received bad flags %x (%s)",
                flags, prx_err_string(result));
            break;
        }

        result = send(sock->sock_fd, (const sockbuf_t*)buffer, 
            (socksize_t)buf_len, os_flags);
        if (result < 0)
        {
            result = pal_os_last_net_error_as_prx_error();
            if (result != er_retry)
            {
                log_error(sock->log, "Send error %s.", 
                prx_err_string(result));
            }
            break;
        }
        else
        {
            buf_len = result;
            result = er_ok;
        }
        log_debug(sock->log, "Send: %zu bytes ...", buf_len);
        break;
    } while (0);

    if (result != er_ok)
        buf_len = 0;

    sock->itf.cb(sock->itf.context, pal_socket_event_end_send,
        &buffer, &buf_len, NULL, NULL, result, &context);
    return result;
}

// 
// Send as much as possible to address
//
static int32_t pal_socket_on_sendto(
    pal_socket_t* sock,
    int32_t result
)
{
    int os_flags = 0;
    int32_t flags;
    uint8_t* buffer;
    prx_socket_address_t sa;
    uint8_t sa_in[MAX_SOCKET_ADDRESS_BYTES];
    socklen_t sa_len = sizeof(sa_in);
    size_t buf_len;
    void* context;

    if (!event_port)
        return er_bad_state;
    if (sock->state != pal_socket_state_open)
        return er_closed;

    sock->itf.cb(sock->itf.context, pal_socket_event_begin_send,
        &buffer, &buf_len, &sa, &flags, er_ok, &context);
    if (!buffer)
    {
        // Done sending
        pal_socket_can_send(sock, false);
        return er_aborted;
    }
    do
    {
        if (result != er_ok)
            break;

        result = pal_os_from_prx_socket_address(&sa, (struct sockaddr*)sa_in, &sa_len);
        if (result != er_ok)
        {
            log_error(sock->log, "Sendto received bad address (%s)",
                prx_err_string(result));
            break;
        }

        result = pal_os_from_prx_message_flags(flags, &os_flags);
        if (result != er_ok)
        {
            log_error(sock->log, "Sendto received bad flags %x (%s)",
                flags, prx_err_string(result));
            break;
        }

        result = sendto(sock->sock_fd, (const sockbuf_t*)buffer, 
            (socksize_t)buf_len, os_flags, (const struct sockaddr*)sa_in, sa_len);
        if (result < 0)
        {
            result = pal_os_last_net_error_as_prx_error();
            if (result != er_retry)
            {
                log_error(sock->log, "Sendto error %s.", 
                    prx_err_string(result));
            }
            break;
        }
        else
        {
            buf_len = result;
            result = er_ok;
        }
        log_debug(sock->log, "Sendto: %zu bytes ...", buf_len);
        break;
    } while (0);

    if (result != er_ok)
        buf_len = 0;

    sock->itf.cb(sock->itf.context, pal_socket_event_end_send,
        &buffer, &buf_len, NULL, NULL, result, &context);
    return result;
}

//
// Connect socket
//
static int32_t pal_socket_connect(
    pal_socket_t* sock
)
{
    int32_t result;
    int timeout = 5 * 1000;  // Wait 5 seconds then timeout connect
    uint8_t sa_in[MAX_SOCKET_ADDRESS_BYTES];
    socklen_t sa_len = sizeof(sa_in);
    int error;
    dbg_assert_ptr(sock);

    // Connect socket
    do
    {
        result = pal_os_from_prx_socket_address(&sock->itf.props.address, 
            (struct sockaddr*)sa_in, &sa_len);
        if (result != er_ok)
        {
            log_error(sock->log, "Bad address for connect call (%s)",
                prx_err_string(result));
            break;
        }

        log_trace(sock->log, "Socket connecting ... (fd:%d)", sock->sock_fd);
#if defined(TCP_USER_TIMEOUT)
        (void)setsockopt (sock->sock_fd, SOL_TCP, TCP_USER_TIMEOUT,
            (char*)&timeout, sizeof(timeout));
#endif
#if defined(ASYNC_CONNECT)
        // Wait for close, error, or writeable
        result = pal_event_select(sock->event_handle, 
            pal_event_type_close);
        if (result != er_ok)
            break;
#endif
        error = connect(sock->sock_fd, (const struct sockaddr*)sa_in, sa_len);
        if (error < 0)
        {
            error = errno;
            if (error == EINPROGRESS)
            {
                result = pal_event_select(sock->event_handle, 
                    pal_event_type_write);
                if (result != er_ok)
                    break;
                //
                // Wait for callback, indicate open loop to break out...
                //
                return er_waiting;
            }
            result = pal_os_to_prx_net_error(error);
            log_error(sock->log, "Failed connecting socket (%s)",
                prx_err_string(result));
        }
        else
        {
            log_trace(sock->log, "Socket connected synchronously!");
            result = er_ok;
        }
    } 
    while (0);

    // Finish connect
    return pal_socket_connect_finish(sock, result);
}

//
// Begin bind operation
//
static int32_t pal_socket_bind(
    pal_socket_t* sock
)
{
    int32_t result;
    uint8_t sa_in[MAX_SOCKET_ADDRESS_BYTES];
    socklen_t sa_len = sizeof(sa_in);
    int error;
    dbg_assert_ptr(sock);

    // Passive or listener
    do
    {
        result = pal_os_from_prx_socket_address(&sock->itf.props.address,
            (struct sockaddr*)sa_in, &sa_len);
        if (result != er_ok)
        {
            log_error(sock->log, "Bad address for bind call (%s)",
                prx_err_string(result));
            break;
        }
 
#if defined(ASYNC_CONNECT)
         // Wait for close, error
        result = pal_event_select(sock->event_handle, 
            pal_event_type_close);
        if (result != er_ok)
            break;
#endif
        error = bind(sock->sock_fd, (const struct sockaddr*)sa_in, sa_len);
        if (error < 0)
        {
            result = pal_os_last_net_error_as_prx_error();
            log_error(sock->log, "Failed binding socket (%s)",
                prx_err_string(result));
            break;
        }
        else
        {
            log_trace(sock->log, "Socket bound synchronously!");
            result = er_ok;
        }

        if (sock->itf.props.sock_type == prx_socket_type_dgram ||
            sock->itf.props.sock_type == prx_socket_type_raw)
            break;

        dbg_assert(0 != (sock->itf.props.flags & prx_socket_flag_passive),
            "should be passive");

        // Start listen immediately
        error = listen(sock->sock_fd, -1);
        if (error < 0)
        {
            result = pal_os_last_net_error_as_prx_error();
            log_error(sock->log, "Failed to set socket to listen (%s)",
                prx_err_string(result));
            break;
        }

        log_trace(sock->log, "Socket listening...");
        result = er_ok;
    } while (0);
    return result;
}

// 
// Try opening socket based on address in properties
//
static int32_t pal_socket_try_open(
    pal_socket_t *sock
)
{
    int32_t result;
    int os_af, os_socktype, os_prototype;
    dbg_assert_ptr(sock);
    do
    {
        // Create socket and register with event port
        result = pal_os_from_prx_address_family(
            sock->itf.props.family, &os_af);
        if (result != er_ok)
            break;
        result = pal_os_from_prx_socket_type(
            sock->itf.props.sock_type, &os_socktype);
        if (result != er_ok)
            break;
        result = pal_os_from_prx_protocol_type(
            sock->itf.props.proto_type, &os_prototype);
        if (result != er_ok)
            break;

        sock->sock_fd = socket(os_af, os_socktype, os_prototype);
        if (sock->sock_fd == _invalid_fd)
        {
            result = pal_os_last_net_error_as_prx_error();
            break;
        }
        log_trace(sock->log, "Socket created ... (fd:%d)", sock->sock_fd);

#if defined(ASYNC_CONNECT)
        // This will make socket nonblocking
        result = pal_event_port_register(event_port, sock->sock_fd, 
            pal_socket_event_callback, sock, &sock->event_handle);
        if (result != er_ok)
            break;
#endif

        if ((sock->itf.props.sock_type == prx_socket_type_seqpacket ||
             sock->itf.props.sock_type == prx_socket_type_rdm ||
             sock->itf.props.sock_type == prx_socket_type_stream) &&
            !(sock->itf.props.flags & prx_socket_flag_passive))
        {
            // If stream socket client connect
            result = pal_socket_connect(sock);
        }
        else
        {
            // Otherwise, bind and optionally start listening...
            result = pal_socket_bind(sock);
        }
    } 
    while (0);

    // Failed synchronously, close socket...
    if (result != er_ok && result != er_waiting)
    {
        log_error(sock->log, "Failed opening socket (%s)!",
            prx_err_string(result));

        pal_socket_clear(sock);
    }
    return result;
}

//
// Open socket based on next address in cached list or fail
//
static void pal_socket_try_next(
    pal_socket_t *sock
)
{
    int32_t result;
    dbg_assert_ptr(sock);
    dbg_assert(sock->sock_fd == _invalid_fd && sock->state == pal_socket_state_opening,
        "Should be opening.");
    while (true)
    {
        if (sock->prx_ai_cur >= sock->prx_ai_count)
        {
            log_error(sock->log, "No other candidate addresses to open...");
            result = er_connecting;
            break;
        }

        // Update address in properties
        memcpy(&sock->itf.props.address, &sock->prx_ai[sock->prx_ai_cur].address,
            sizeof(prx_socket_address_t));
        sock->itf.props.family = sock->prx_ai[sock->prx_ai_cur].address.un.family;

        // Try the address
        result = pal_socket_try_open(sock);
        if (result == er_waiting)
            return; // Wait for callback

        if (result != er_ok)
        {
            sock->prx_ai_cur++;
            continue;  // Try next
        }
        // Success!
        log_trace(sock->log, "Socket %d opened synchronously!", sock->sock_fd);
        
#if !defined(ASYNC_CONNECT)
        // Make sure any event is not completing open
        sock->state = pal_socket_state_open;

        // This will make socket nonblocking
        result = pal_event_port_register(event_port, sock->sock_fd, 
            pal_socket_event_callback, sock, &sock->event_handle);
        if (result != er_ok)
            break;

        // Wait for close, error, or writeable
        result = pal_event_select(sock->event_handle, 
            pal_event_type_close);
        if (result != er_ok)
            break;
#endif
        break;
    }

    // Complete open which frees address list
    pal_socket_open_complete(sock, result);
}

//
// Resolve proxy address first and try to open each returned address
//
static int32_t pal_socket_open_by_name(
    pal_socket_t *sock
)
{
    int32_t result;
    char port[MAX_PORT_LENGTH];
    const char* server = NULL;
    uint32_t flags = 0;

    dbg_assert_ptr(sock);
    do
    {
        dbg_assert(sock->itf.props.address.un.family == prx_address_family_proxy,
            "Bad address family");
        if (strlen(sock->itf.props.address.un.proxy.host) != 0)
            server = sock->itf.props.address.un.proxy.host;

        result = string_from_int(
            sock->itf.props.address.un.ip.port, 10, port, sizeof(port));
        if (result != er_ok)
            break;

        log_info(sock->log, "Resolving %s:%s...",
            server ? server : "<null>", port);
        if (sock->itf.props.flags & prx_socket_flag_passive)
            flags |= prx_ai_passive;
        result = pal_getaddrinfo(
            server, port, sock->itf.props.family, flags, &sock->prx_ai, &sock->prx_ai_count);
        if (result == er_ok && !sock->prx_ai_count)
            result = er_connecting;
        if (result != er_ok)
        {
            log_error(sock->log, "pal_getaddrinfo for %s:%s failed (%s).",
                server ? server : "<null>", port, prx_err_string(result));
            break;
        }

        // Now we have a list of addresses, try to open one by one...
        pal_socket_try_next(sock);
        result = er_ok;
        break;
    } while (0);
    return result;
}

//
// Open address without resolving name
//
static int32_t pal_socket_open_by_addr(
    pal_socket_t *sock
)
{
    int32_t result;
    dbg_assert_ptr(sock);
    dbg_assert(sock->itf.props.address.un.family != prx_address_family_proxy,
        "Bad address family");

    // Begin open
    result = pal_socket_try_open(sock);
    if (result == er_waiting)
        return er_ok; // Wait for callback

    // Complete open 
    pal_socket_open_complete(sock, result);
    return result;
}

//
// Open a new socket based on properties passed during create
//
int32_t pal_socket_open(
    pal_socket_t *sock
)
{
    chk_arg_fault_return(sock);

    dbg_assert(!sock->prx_ai_cur && !sock->prx_ai_count && !sock->prx_ai,
        "Should not have an address list");
    dbg_assert(sock->sock_fd == _invalid_fd && sock->state == pal_socket_state_closed, 
        "Should not be open.");

    sock->state = pal_socket_state_opening;
    if (sock->itf.props.address.un.family == prx_address_family_proxy)
        return pal_socket_open_by_name(sock);

    return pal_socket_open_by_addr(sock);
}

//
// Create a new sock to track
//
int32_t pal_socket_create(
    pal_socket_client_itf_t* client_itf,
    pal_socket_t** created
)
{
    pal_socket_t* sock;
    chk_arg_fault_return(client_itf);
    chk_arg_fault_return(client_itf->cb);
    chk_arg_fault_return(created);

    sock = mem_zalloc_type(pal_socket_t);
    if (!sock)
        return er_out_of_memory;
 
    sock->state = pal_socket_state_closed;
    sock->log = log_get("pal_sk");
    sock->sock_fd = _invalid_fd;
    memcpy(&sock->itf, client_itf, sizeof(pal_socket_client_itf_t));

    // Set function pointers based on type of socket
    if (sock->itf.props.family != prx_address_family_unix &&
        (sock->itf.props.sock_type == prx_socket_type_dgram ||
         sock->itf.props.sock_type == prx_socket_type_raw))
    {
        // Non connection oriented sockets recvfrom and sendto..
        sock->write_cb = pal_socket_on_sendto;
        sock->read_cb = pal_socket_on_recvfrom;
    }
    else if (sock->itf.props.flags & prx_socket_flag_passive)
    {
        // Listen socket, can only recv new sockets
        sock->write_cb = pal_socket_dummy_cb;
        sock->read_cb = pal_socket_on_accept;
    }
    else
    {
        // Stream socket can send and receive - no address
        sock->write_cb = pal_socket_on_send;
        sock->read_cb = pal_socket_on_recv;
    }

    *created = sock;
    return er_ok;
}

//
// Create an opened / connected pair of local sockets 
//
int32_t pal_socket_pair(
    pal_socket_client_itf_t* itf1,
    pal_socket_t** created1,
    pal_socket_client_itf_t* itf2,
    pal_socket_t** created2
)
{
    int32_t result;
    int os_socktype, os_prototype;
    int fds[2];
    pal_socket_t* sock1 = NULL, *sock2 = NULL;

    chk_arg_fault_return(itf1);
    chk_arg_fault_return(itf1->cb);
    chk_arg_fault_return(created1);
    chk_arg_fault_return(itf2);
    chk_arg_fault_return(itf2->cb);
    chk_arg_fault_return(created2);

    if (itf1->props.sock_type != itf2->props.sock_type ||
        itf1->props.proto_type != itf2->props.proto_type)
        return er_arg;
    do
    {
        // Create socket pair and register both with event port
        result = pal_os_from_prx_socket_type(itf1->props.sock_type,
            &os_socktype);
        if (result != er_ok)
            break;
        result = pal_os_from_prx_protocol_type(itf1->props.proto_type, 
            &os_prototype);
        if (result != er_ok)
            break;
        if (0 != socketpair(AF_UNIX, os_socktype, os_prototype, fds))
        {
            result = pal_os_last_net_error_as_prx_error();
            break;
        }

         // Make sure that sockets are created as pipe sockets - adjust address family
        itf1->props.family = itf2->props.family = prx_address_family_unix;

        result = pal_socket_create(itf1, &sock1);
        if (result != er_ok)
            break;
        sock1->sock_fd = fds[0];
        fds[0] = _invalid_fd;
        result = pal_socket_create(itf2, &sock2);
        if (result != er_ok)
            break;
        sock2->sock_fd = fds[1];
        fds[1] = _invalid_fd;

        // Make sockets nonblocking and register with event port
        result = pal_event_port_register(event_port, sock1->sock_fd,
            pal_socket_event_callback, sock1, &sock1->event_handle);
        if (result != er_ok)
            break;
        result = pal_event_port_register(event_port, sock2->sock_fd,
            pal_socket_event_callback, sock2, &sock2->event_handle);
        if (result != er_ok)
            break;

        // Wait for close, error, or writeable
        result = pal_event_select(sock1->event_handle, pal_event_type_close);
        if (result != er_ok)
            break;
        result = pal_event_select(sock2->event_handle, pal_event_type_close);
        if (result != er_ok)
            break;

        *created1 = sock1;
        *created2 = sock2;
        break;
    } 
    while (0);

    // To avoid leaking resources complete open with error notifying callback
    if (sock1)
        pal_socket_open_complete(sock1, result);
    if (sock2)
        pal_socket_open_complete(sock2, result);

    if (result != er_ok)
    {
        if (fds[0] != _invalid_fd)
            close(fds[0]);
        if (fds[1] != _invalid_fd)
            close(fds[1]);
        if (sock1)
        {
            pal_socket_clear(sock1);
            pal_socket_free(sock1);
        }
        if (sock2)
        {
            pal_socket_clear(sock2);
            pal_socket_free(sock2);
        }
    }
    return result;
}

// 
// Stop streaming, unregister socket from event port
//
void pal_socket_close(
    pal_socket_t *sock
)
{
    if (!sock)
        return;

    sock->state = pal_socket_state_closing;
    // Close the event handle
    if (sock->event_handle != 0)
        pal_event_close(sock->event_handle, true);
    else
        pal_socket_complete_close(sock);
}

//
// Get socket properties
//
int32_t pal_socket_get_properties(
    pal_socket_t *sock,
    prx_socket_properties_t* props
)
{
    chk_arg_fault_return(sock);
    chk_arg_fault_return(props);
    memcpy(props, &sock->itf.props, sizeof(prx_socket_properties_t));
    return er_ok;
}

//
// Get peer address
//
int32_t pal_socket_getpeername(
    pal_socket_t* sock,
    prx_socket_address_t* socket_address
)
{
    chk_arg_fault_return(sock);
    chk_arg_fault_return(socket_address);
    memcpy(socket_address, &sock->peer, sizeof(prx_socket_address_t));
    return er_ok;
}

//
// Get local address
//
int32_t pal_socket_getsockname(
    pal_socket_t* sock,
    prx_socket_address_t* socket_address
)
{
    int32_t result;
    uint8_t sa_in[MAX_SOCKET_ADDRESS_BYTES];
    socklen_t sa_len = sizeof(sa_in);

    chk_arg_fault_return(sock);
    chk_arg_fault_return(socket_address);

    result = getsockname(sock->sock_fd, (struct sockaddr*)sa_in, &sa_len);
    if (result != 0)
        return pal_os_last_net_error_as_prx_error();

    result = pal_os_to_prx_socket_address(
        (const struct sockaddr*)sa_in, sa_len, socket_address);
    if (result != er_ok)
        return result;
    return er_ok;
}

//
// Notify socket that caller is ready to send
//
int32_t pal_socket_can_send(
    pal_socket_t* sock,
    bool ready
)
{
    chk_arg_fault_return(sock);
    if (ready)
        return pal_event_select(sock->event_handle, pal_event_type_write);
    else
        return pal_event_clear(sock->event_handle, pal_event_type_write);
}

//
// Notify socket that caller is ready to recv
//
int32_t pal_socket_can_recv(
    pal_socket_t* sock,
    bool ready
)
{
    chk_arg_fault_return(sock);
    if (ready)
        return pal_event_select(sock->event_handle, pal_event_type_read);
    else
        return pal_event_clear(sock->event_handle, pal_event_type_read);
}

//
// Get socket option
//
int32_t pal_socket_getsockopt(
    pal_socket_t* sock,
    prx_socket_option_t socket_option,
    uint64_t* value
)
{
    int32_t result;
    int error;
    int32_t opt_lvl, opt_name;
    socklen_t opt_len;
    int32_t opt_val;
    int avail;

    chk_arg_fault_return(sock);
    chk_arg_fault_return(value);

    if (socket_option == prx_so_shutdown)
        return er_not_supported;

    if (socket_option == prx_so_available)
    {
        error = ioctl(sock->sock_fd, FIONREAD, &avail);
        if (error == -1)
            return pal_os_last_net_error_as_prx_error();
        *value = (size_t)avail;
        return er_ok;
    }

    if (socket_option == prx_so_linger)
    {
        struct linger opt;
        opt_len = sizeof(opt);
        error = getsockopt(sock->sock_fd, SOL_SOCKET, SO_LINGER, (char*)&opt, &opt_len);
        if (error != 0)
            return pal_os_last_net_error_as_prx_error();
        *value = opt.l_onoff ? opt.l_linger : 0;
        return er_ok;
    }

    result = pal_os_from_prx_socket_option(socket_option, &opt_lvl, &opt_name);
    if (result != er_ok)
        return result;

    opt_len = sizeof(int32_t);
    error = getsockopt(sock->sock_fd, opt_lvl, opt_name, (char*)&opt_val, &opt_len);
    if (error != 0)
        return pal_os_last_net_error_as_prx_error();

    dbg_assert(opt_len <= (socklen_t)sizeof(int32_t), "invalid len returned");
    if (socket_option == prx_so_error)
        *value = pal_os_to_prx_net_error(opt_val);
    else
        *value = opt_val;
    return er_ok;
}

//
// Set socket option
//
int32_t pal_socket_setsockopt(
    pal_socket_t* sock,
    prx_socket_option_t socket_option,
    uint64_t value
)
{
    int32_t result;
    int error;
    int32_t opt_lvl, opt_name;
    int32_t opt_val;

    chk_arg_fault_return(sock);

    /**/ if (socket_option == prx_so_available)
        return er_not_supported;
    else if (socket_option == prx_so_shutdown)
    {
        if (value != prx_shutdown_op_read)
            pal_socket_can_send(sock, false);
        else if (value != prx_shutdown_op_write)
            pal_socket_can_recv(sock, false);

        result = pal_os_from_prx_shutdown_op((prx_shutdown_op_t)value, &opt_val);
        if (result != er_ok)
            return result;
        error = shutdown(sock->sock_fd, opt_val);
    }
    else if (socket_option == prx_so_linger)
    {
        struct linger opt;
        opt.l_onoff = !!value;
        opt.l_linger = (unsigned short)value;
        error = setsockopt(sock->sock_fd, SOL_SOCKET, SO_LINGER,
            (char*)&opt, sizeof(opt));
    }
    else if (socket_option == prx_so_nonblocking)
        return er_ok;
    else if (socket_option == prx_so_acceptconn)
        return er_not_supported;
    else
    {
        opt_val = (int32_t)value;
        result = pal_os_from_prx_socket_option(socket_option, &opt_lvl, &opt_name);
        if (result != er_ok)
            return result;

        error = setsockopt(
            sock->sock_fd, opt_lvl, opt_name, (const char*)&opt_val, 
            (socklen_t)sizeof(opt_val));
    }
    return error == 0 ? er_ok : pal_os_last_net_error_as_prx_error();
}

//
// Leave multicast group
//
int32_t pal_socket_leave_multicast_group(
    pal_socket_t* sock,
    prx_multicast_option_t* option
)
{
    int error;
    struct ipv6_mreq opt6;
    struct ip_mreqn opt;

    chk_arg_fault_return(sock);
    chk_arg_fault_return(option);

    switch (option->family)
    {
    case prx_address_family_inet:
        opt.imr_multiaddr.s_addr = option->addr.in4.un.addr;

        opt.imr_address.s_addr = INADDR_ANY;
        opt.imr_ifindex = option->itf_index;

        error = setsockopt(
            sock->sock_fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&opt, sizeof(opt));
        break;
    case prx_address_family_inet6:
        memcpy(opt6.ipv6mr_multiaddr.s6_addr, option->addr.in6.un.u8,
            sizeof(option->addr.in6.un.u8));
        opt6.ipv6mr_interface = option->itf_index;
        error = setsockopt(
            sock->sock_fd, IPPROTO_IPV6, IPV6_DROP_MEMBERSHIP, (char*)&opt6, sizeof(opt6));
        break;
    default:
        return er_not_supported;
    }
    return error == 0 ? er_ok : pal_os_last_net_error_as_prx_error();
}

//
// Join multicast group
//
int32_t pal_socket_join_multicast_group(
    pal_socket_t* sock,
    prx_multicast_option_t* option
)
{
    int error;
    struct ipv6_mreq opt6;
    struct ip_mreqn opt;

    chk_arg_fault_return(sock);
    chk_arg_fault_return(option);

    switch (option->family)
    {
    case prx_address_family_inet:
        opt.imr_multiaddr.s_addr = option->addr.in4.un.addr;

        opt.imr_address.s_addr = INADDR_ANY;
        opt.imr_ifindex = option->itf_index;

        error = setsockopt(
            sock->sock_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&opt, sizeof(opt));
        break;
    case prx_address_family_inet6:
        memcpy(opt6.ipv6mr_multiaddr.s6_addr, option->addr.in6.un.u8,
            sizeof(option->addr.in6.un.u8));
        opt6.ipv6mr_interface = option->itf_index;
#if !defined(IPV6_ADD_MEMBERSHIP) && defined(IPV6_JOIN_GROUP)
#define IPV6_ADD_MEMBERSHIP IPV6_JOIN_GROUP
#endif
        error = setsockopt(
            sock->sock_fd, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, (char*)&opt6, sizeof(opt6));
        break;
    default:
        return er_not_supported;
    }
    return error == 0 ? er_ok : pal_os_last_net_error_as_prx_error();
}

//
// Free the socket
//
void pal_socket_free(
    pal_socket_t *sock
)
{
    if (!sock)
        return;
    dbg_assert(sock->sock_fd == _invalid_fd, "socket still open");
    mem_free_type(pal_socket_t, sock);
}

#include "azure_c_shared_utility/tlsio_openssl.h"

//
// Return default tls implentation
//
const IO_INTERFACE_DESCRIPTION* platform_get_default_tlsio(void)
{
    return tlsio_openssl_get_interface_description();
}

//
// Initialize socket layer
//
int32_t pal_socket_init(
    uint32_t* caps
)
{
    int32_t result;
    
    chk_arg_fault_return(caps);

    result = pal_event_port_create(&event_port);
    if (result != er_ok)
    {
        log_error(NULL, "FATAL: Failed creating event port.");
    }
    else
    {
        (void)tlsio_openssl_init();

        (*caps) |= (pal_cap_sockets | pal_cap_ev);
    }
    return result;
}

//
// Free the socket layer
//
void pal_socket_deinit(
    void
)
{
    if (event_port)
    {
        pal_event_port_close(event_port);

        tlsio_openssl_deinit();
    }
    event_port = 0;
}
