// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os.h"
#include "util_mem.h"
#include "util_string.h"
#include "pal.h"
#include "pal_sk.h"
#include "pal_net.h"
#include "pal_mt.h"
#include "pal_types.h"
#include "pal_err.h"
#include "prx_sched.h"

static LPFN_CONNECTEX _ConnectEx = NULL;
static LPFN_ACCEPTEX _AcceptEx = NULL;
static LPFN_GETACCEPTEXSOCKADDRS _GetAcceptExSockAddrs = NULL;
static HMODULE _ntdll = NULL;
static prx_scheduler_t* _scheduler;

//
// Io Completion port operation context
//
typedef struct pal_socket_async pal_socket_async_t;

//
// Begin operation - returns !er_waiting if completed synchronously
//
typedef int32_t (*pal_socket_async_begin_t)(
    pal_socket_async_t* async_op
    );

//
// Complete operation
//
typedef void (*pal_socket_async_complete_t)(
    pal_socket_async_t* async_op
    );

//
// Completion port context
//
struct pal_socket_async
{
    OVERLAPPED ov;         // Must be first to cast from OVERLAPPED*
    bool enabled;
    bool pending;
    pal_socket_t* sock;
    pal_socket_async_begin_t begin;               // Begin operation
    pal_socket_async_complete_t complete;      // Complete operation 
    uint8_t* buffer;
    size_t buf_len;                          // Buffer length in/out
    int32_t result;                        // Result set or returned
    DWORD flags;                            // Flags set or returned
    SOCKADDR_STORAGE addr_buf[2];  // Socket address buf for this op
    socklen_t addr_len;       // length of address in address buffer
    void* context;
};

//
// Represents a async winsock socket using io completion ports
//
struct pal_socket
{
    pal_socket_client_itf_t itf;                 // Client interface
    SOCKET sock_fd;                 // Real underlying socket handle
    bool closing;                   // Whether the socket is closing

    prx_addrinfo_t* prx_ai;  // For async connect save the ai result
    size_t prx_ai_count;           // Size of the resolved addresses
    size_t prx_ai_cur;                 // Current address to connect
    char* ai_name;                 // For unix path, store pipe name

    pal_socket_async_t open_op;           // Async connect operation

    pal_socket_async_t send_op;              // Async send operation
    pal_socket_async_t recv_op;    // Async recv or accept operation

    prx_socket_address_t local;              // Cached local address
    prx_socket_address_t peer;                // Cached peer address
    prx_scheduler_t* scheduler;  // Scheduler to synchronize overlap
    log_t log;
};

//
// Get error code for os error
//
static int32_t pal_socket_from_os_error(
    DWORD error
)
{
    char* message = NULL;
    /**/ if (error == ERROR_SUCCESS)
        return er_ok;
    else if (error != STATUS_CANCELLED)
    {
        FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS |
            FORMAT_MESSAGE_MAX_WIDTH_MASK,
            _ntdll, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (char*)&message, 0, NULL);
        if (message)
        {
            string_trim_back(message, "\r\n\t ");
            log_info(NULL, "%s (0x%x)", message, error);
            LocalFree(message);
        }
        else
        {
            log_info(NULL, "Unknown socket error 0x%x.", error);
        }
    }
    else 
    {
        log_debug(NULL, "Operation Cancelled");
    }
    return pal_os_to_prx_error(error);
}

// 
// Close underlying socket handle
//
static void pal_socket_close_handle(
    pal_socket_t* sock
)
{
    if (sock->itf.props.family == prx_address_family_unix)
    {
        // Then the fd is a pipe handle, use CloseHandle
        if ((HANDLE)sock->sock_fd != INVALID_HANDLE_VALUE)
        {
            CloseHandle((HANDLE)sock->sock_fd);
            sock->sock_fd = (SOCKET)INVALID_HANDLE_VALUE;
        }
    }
    else
    {
        // otherwise use closesocket
        if (sock->sock_fd != INVALID_SOCKET)
        {
            closesocket(sock->sock_fd);
            sock->sock_fd = INVALID_SOCKET;
        }
    }
}

//
// Create pipe name for address path to mock unix domain sockets
//
static int32_t pal_socket_make_pipe_name(
    const char* path,
    char** name
)
{
    char* pipe_name;

    if (path[0] && path[0] == '\\' &&
        path[1] && path[1] == '\\')
    {
        // already windows compatible pipe path
        return string_clone(path, name);
    }

    // Skipping leading slashes
    while (*path && *path == '/')
        path++;
    if (!path[0])
        return er_arg;

    pipe_name = (char*) mem_alloc(strlen(path) + 10); // + '\\.\pipe\', and \0
    if (!pipe_name)
        return er_out_of_memory;
    strcpy(&pipe_name[0], "\\\\.\\pipe\\");
    strcpy(&pipe_name[9], path);

    *name = pipe_name;
    return er_ok;
}

//
// Try close socket
//
static void pal_socket_try_close(
    pal_socket_t* sock
);

//
// Init async op
//
static int32_t pal_socket_async_op_init(
    pal_socket_async_t* async_op
)
{
    memset(&async_op->ov, 0, sizeof(OVERLAPPED));
    async_op->buffer = NULL;
    async_op->buf_len = 0;
    async_op->addr_len = 0;
    async_op->flags = 0;
    async_op->context = NULL;

    return er_nomore;  // Reset and no more looping please
}

//
// Close begin callback
//
static int32_t pal_socket_async_close_begin(
    pal_socket_async_t* async_op
)
{
    dbg_assert_ptr(async_op);
    dbg_assert_ptr(async_op->sock);

    __do_next(async_op->sock, pal_socket_try_close);

    return er_waiting;  // Waiting for close to complete...
}

//
// Close complete callback
//
static bool pal_socket_async_has_close_completed(
    pal_socket_async_t* async_op
)
{
    DWORD error;
    dbg_assert_ptr(async_op);
    dbg_assert_ptr(async_op->sock);

    // This will dead end anything in progress into close complete
    async_op->begin = pal_socket_async_close_begin;

    if (!HasOverlappedIoCompleted(&async_op->ov))
    {
        // Cancel any pending io
        if (CancelIoEx((HANDLE)async_op->sock->sock_fd, &async_op->ov))
            return false;

        error = GetLastError();
        memset(&async_op->ov, 0, sizeof(OVERLAPPED));
    }
    
    if (async_op->buffer)
        return false;  // Wait for buffer to complete
    
    prx_scheduler_clear(async_op->sock->scheduler, NULL, async_op);
    return true;
}

//
// Check whether all operations are complete while socket is closing
//
static void pal_socket_try_close(
    pal_socket_t* sock
)
{
    size_t size;
    dbg_assert_ptr(sock);

    if (sock->sock_fd == INVALID_SOCKET)
        return;

    sock->closing = true;
    if (pal_socket_async_has_close_completed(&sock->open_op) &&
        pal_socket_async_has_close_completed(&sock->send_op) &&
        pal_socket_async_has_close_completed(&sock->recv_op))
    {
        prx_scheduler_clear(sock->scheduler, NULL, sock);
        pal_socket_close_handle(sock);

        // No more operations are pending, close
        size = sizeof(pal_socket_t*);
        sock->itf.cb(sock->itf.context, pal_socket_event_closed, 
            (uint8_t**)&sock, &size, NULL, NULL, er_ok, NULL);
    }
    else
    {
        // Try again in 1 second
        __do_later(sock, pal_socket_try_close, 1000);
    }
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

    // Complete open
    size = sizeof(pal_socket_t*);
    sock->itf.cb(sock->itf.context, pal_socket_event_opened, 
        (uint8_t**)&sock, &size, NULL, NULL, result, NULL);

    if (!sock->prx_ai)
        return;

    pal_freeaddrinfo(sock->prx_ai);
    sock->prx_ai = NULL;
    sock->prx_ai_count = 0;
    sock->prx_ai_cur = 0;
}

//
// Called when ConnectEx completes in any mode
//
static int32_t pal_socket_connect_complete(
    pal_socket_async_t* async_op
)
{
    int error;
    int32_t result = async_op->result;

    dbg_assert_ptr(async_op);
    dbg_assert_ptr(async_op->sock);

    while(result == er_ok)
    {
        // Update the connect context.
        error = setsockopt(async_op->sock->sock_fd, 
            SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);
        if (error != 0)
        {
            result = pal_os_last_net_error_as_prx_error();
            break;
        }

        async_op->addr_len = sizeof(async_op->addr_buf);
        error = getsockname(async_op->sock->sock_fd,
            (struct sockaddr*)async_op->addr_buf, &async_op->addr_len);
        if (error != 0)
        {
            result = pal_os_last_net_error_as_prx_error();
            break;
        }
        result = pal_os_to_prx_socket_address(
            (const struct sockaddr*)async_op->addr_buf,
            async_op->addr_len, &async_op->sock->local);
        if (result != er_ok)
            break;

        async_op->addr_len = sizeof(async_op->addr_buf);
        error = getpeername(async_op->sock->sock_fd,
            (struct sockaddr*)async_op->addr_buf, &async_op->addr_len);
        if (error != 0)
        {
            result = pal_os_last_net_error_as_prx_error();
            break;
        }
        result = pal_os_to_prx_socket_address(
            (const struct sockaddr*)async_op->addr_buf,
            async_op->addr_len, &async_op->sock->peer);
        break;
    }

    async_op->addr_len = 0;

    if (result != er_ok)
        pal_socket_close_handle(async_op->sock);
    return result;
}

//
// Called when CreateFile completes
//
static int32_t pal_socket_createfile_complete(
    pal_socket_async_t* async_op
)
{
    int32_t result = async_op->result;

    dbg_assert_ptr(async_op);
    dbg_assert_ptr(async_op->sock);

    while (result == er_ok)
    {
        // Todo: Get remote computer name, etc...
    }

    async_op->addr_len = 0;

    if (result != er_ok)
        pal_socket_close_handle(async_op->sock);
    return result;
}

//
// Kicks off the operation on the io completion port if none is pending
//
static void pal_socket_async_begin(
    pal_socket_async_t* async_op
);

//
// Completes operation and kicks off new one
//
static void pal_socket_async_complete(
    pal_socket_async_t* async_op
)
{
    dbg_assert_ptr(async_op);
    dbg_assert_is_task(async_op->sock->scheduler);

    // No more pending 
    dbg_assert(async_op->pending, "Op should be pending when completing...");
    async_op->pending = false;

    if (async_op->sock->closing)
        return;
    if (async_op->result != er_ok)
        return;
    if (!async_op->enabled)
        return;

    // Try scheduling another round
    pal_socket_async_begin(async_op);
}

//
// Io completion port operation callback when operation completed
//
static void CALLBACK pal_socket_async_complete_from_OVERLAPPED(
    DWORD error,
    DWORD bytes,
    LPOVERLAPPED ov
)
{
    int32_t result;
    pal_socket_async_t* async_op = (pal_socket_async_t*)ov;

    dbg_assert_ptr(async_op);

    async_op->result = pal_socket_from_os_error(error);
    async_op->buf_len = (size_t)bytes;

    while (true)
    {
        // Complete operation
        dbg_assert(async_op->pending, "Op should be pending");
        dbg_assert_ptr(async_op->complete);
        async_op->complete(async_op);

        if (!async_op->enabled)
            break;
        if (async_op->result != er_ok)
            break;

        // Start next operation - still pending...
        dbg_assert_ptr(async_op->begin);
        result = async_op->begin(async_op);
        if (result == er_waiting)
            return; // Complete pending on callback

        if (result == er_nomore)
        {
            break; // no more buffers or user input. Do not complete but end pending...
        }
    }
    __do_next_s(async_op->sock->scheduler, pal_socket_async_complete, async_op);
}

//
// Kicks off the operation on the io completion port if none is pending
//
static void pal_socket_async_begin(
    pal_socket_async_t* async_op
)
{
    int32_t result;
    dbg_assert_ptr(async_op);
    dbg_assert_is_task(async_op->sock->scheduler);
    
    // Kick off operation, but only once if not already in progress
    if (async_op->pending)
        return;

    async_op->pending = true;
    while (async_op->enabled)
    {
        dbg_assert_ptr(async_op->begin);
        result = async_op->begin(async_op);
        if (result == er_waiting)
            return; // Complete pending on callback
        if (result == er_nomore)
        {
            // Disable until user requeues another one of this task.
            async_op->result = er_nomore;
            break; // no more pending, but do not complete
        }

        dbg_assert_ptr(async_op->complete);
        async_op->complete(async_op);
        if (result != er_ok)
            break;
    } 
    pal_socket_async_complete(async_op);
}

//
// Enables op operation loop 
//
static void pal_socket_async_enable(
    pal_socket_async_t* async_op
)
{
    dbg_assert_ptr(async_op);
    dbg_assert_ptr(async_op->sock);
    dbg_assert_is_task(async_op->sock->scheduler);

    async_op->enabled = true;

    pal_socket_async_begin(async_op);
}

//
// Disables op operation loop 
//
static void pal_socket_async_disable(
    pal_socket_async_t* async_op
)
{
    dbg_assert_ptr(async_op);
    dbg_assert_ptr(async_op->sock);
    dbg_assert_is_task(async_op->sock->scheduler);

    async_op->enabled = false;
}

//
// Continue open session, opens next address in list or completes
//
static void pal_socket_open_next_begin(
    pal_socket_t *sock
);

//
// Called when ConnectEx completes asynchronously
//
static void pal_socket_async_connect_complete(
    pal_socket_async_t* async_op
)
{
    int32_t result;
    dbg_assert_ptr(async_op);
    dbg_assert_ptr(async_op->sock);
    dbg_assert_is_task(async_op->sock->scheduler);
    
    // Complete connection
    result = pal_socket_connect_complete(async_op);
    if (result == er_ok)
    {
        pal_socket_open_complete(async_op->sock, er_ok);

        // Success!
        log_trace(async_op->sock->log, "Socket connected asynchronously!");
    } 
    else
    {
        log_error(async_op->sock->log, 
            "Failed to connect socket, continue... (%s)",
            prx_err_string(result));

        // Continue with next address
        async_op->sock->prx_ai_cur++;
        __do_next(async_op->sock, pal_socket_open_next_begin);
    }

    pal_socket_async_op_init(async_op);
}

//
// Called when AcceptEx completes
//
static void pal_socket_async_accept_complete(
    pal_socket_async_t* async_op
)
{
    int32_t result;
    pal_socket_t* accepted = (pal_socket_t*)async_op->buffer;
    struct sockaddr* sa_local, *sa_peer;
    socklen_t sa_llen, sa_plen;

    dbg_assert_ptr(async_op);
    dbg_assert(async_op->buf_len == sizeof(pal_socket_t*), "Unexpected size");
    dbg_assert_ptr(async_op->sock);
    dbg_assert_is_task(async_op->sock->scheduler);

    result = async_op->result;
    do
    {
        if (result != er_ok)
        {
            log_error(async_op->sock->log,
                "Failed accept (%s)", prx_err_string(result));
            break;
        }

        // Parse addresses
        _GetAcceptExSockAddrs(async_op->addr_buf, 0,
            sizeof(async_op->addr_buf[0]),
            sizeof(async_op->addr_buf[1]),
            &sa_local, &sa_llen, &sa_peer, &sa_plen);

        result = pal_os_to_prx_socket_address(sa_local, sa_llen, &accepted->local);
        if (result != er_ok)
        {
            log_error(async_op->sock->log, "Accept received bad local address (%s)",
                prx_err_string(result));
            break;
        }

        result = pal_os_to_prx_socket_address(sa_peer, sa_plen, &accepted->peer);
        if (result != er_ok)
        {
            log_error(async_op->sock->log, "Accept received bad peer address (%s)",
                prx_err_string(result));
            break;
        }

        // Update props
        memcpy(&accepted->itf.props.address, &accepted->peer, 
            sizeof(prx_socket_address_t));
        accepted->itf.props.family = accepted->peer.un.family;

        // Update the accept context.
        if (0 != setsockopt(accepted->sock_fd, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
            (char*)&accepted->sock_fd, sizeof(SOCKET)))
        {
            result = pal_os_last_net_error_as_prx_error();
            break;
        }
    } 
    while (0);

    // Complete accept with the new socket as the argument.
    async_op->sock->itf.cb(async_op->sock->itf.context, pal_socket_event_end_accept,
        (uint8_t**)&accepted, &async_op->buf_len, NULL, NULL, result, &async_op->context);

    if (result != er_ok)
    {
        pal_socket_close_handle(accepted);
        pal_socket_free(accepted);
    }
    else
    {
        // Open accepted socket
        pal_socket_open_complete(accepted, er_ok);
    }
    pal_socket_async_op_init(async_op);
}

//
// Called when ConnectNamedPipe completes
//
static void pal_socket_async_connectpipe_complete(
    pal_socket_async_t* async_op
)
{
    int32_t result;
    pal_socket_t* accepted = (pal_socket_t*)async_op->buffer;

    dbg_assert_ptr(async_op);
    dbg_assert(async_op->buf_len == sizeof(pal_socket_t*), "Unexpected size");
    dbg_assert_ptr(async_op->sock);
    dbg_assert_is_task(async_op->sock->scheduler);

    result = async_op->result;
    if (result != er_ok)
    {
        log_error(async_op->sock->log,
            "Failed connecting pipe client (%s)", prx_err_string(result));
    }

    // Complete ConnectNamedPipe with the new socket as the argument.
    async_op->sock->itf.cb(async_op->sock->itf.context, pal_socket_event_end_accept,
        (uint8_t**)&accepted, &async_op->buf_len, NULL, NULL, result, &async_op->context);

    if (result != er_ok)
    {
        pal_socket_close_handle(accepted);
        pal_socket_free(accepted);
    }
    else
    {
        // Open accepted socket
        pal_socket_open_complete(accepted, er_ok);
    }
    pal_socket_async_op_init(async_op);
}

//
// Called when WSASend completes
//
static void pal_socket_async_send_complete(
    pal_socket_async_t* async_op
)
{
    dbg_assert_ptr(async_op);
    dbg_assert_ptr(async_op->sock);
    dbg_assert_is_task(async_op->sock->scheduler);

    // Complete send
    dbg_assert_ptr(async_op->buffer);
    async_op->sock->itf.cb(async_op->sock->itf.context, pal_socket_event_end_send,
        &async_op->buffer, &async_op->buf_len, NULL, NULL, async_op->result,
        &async_op->context);
    pal_socket_async_op_init(async_op);
}

//
// Called when WSARecv completes
//
static void pal_socket_async_recv_complete(
    pal_socket_async_t* async_op
)
{
    int32_t flags = 0;
    int32_t result = async_op->result;

    dbg_assert_ptr(async_op);
    dbg_assert_ptr(async_op->sock);
    dbg_assert(!async_op->addr_len, "Expected no adddress on WSARecv");
    
    if (result == er_ok)
    {
        result = pal_os_to_prx_message_flags(async_op->flags, &flags);
        if (result != er_ok)
        {
            log_error(async_op->sock->log, "Recv received bad flags %x (%s)",
                async_op->flags, prx_err_string(result));
        }
    }

    dbg_assert_ptr(async_op->buffer);
    async_op->sock->itf.cb(async_op->sock->itf.context, pal_socket_event_end_recv,
        &async_op->buffer, &async_op->buf_len, NULL, &flags, result, &async_op->context);
    pal_socket_async_op_init(async_op);
}

//
// Called when WSASendTo completes
//
static void pal_socket_async_sendto_complete(
    pal_socket_async_t* async_op
)
{
    dbg_assert_ptr(async_op);
    dbg_assert_ptr(async_op->sock);

    // Complete sendto
    async_op->sock->itf.cb(async_op->sock->itf.context, pal_socket_event_end_send,
        &async_op->buffer, &async_op->buf_len, NULL, NULL, async_op->result, 
        &async_op->context);
    pal_socket_async_op_init(async_op);
}

//
// Called when WSARecvFrom completes
//
static void pal_socket_async_recvfrom_complete(
    pal_socket_async_t* async_op
)
{
    int32_t flags = 0;
    int32_t result = async_op->result;
    prx_socket_address_t addr, *addr_ptr = NULL;

    dbg_assert_ptr(async_op);
    dbg_assert_ptr(async_op->sock);
    do
    {
        if (result != er_ok)
            break;

        // Process received flags and address... 
        result = pal_os_to_prx_message_flags(async_op->flags, &flags);
        if (result != er_ok)
        {
            log_error(async_op->sock->log, "Recvfrom received bad flags %x (%s)",
                async_op->flags, prx_err_string(result));
            break;
        }

        result = pal_os_to_prx_socket_address(
            (struct sockaddr*)async_op->addr_buf, async_op->addr_len, &addr);
        if (result != er_ok)
        {
            log_error(async_op->sock->log, "Recvfrom received bad address (%s)",
                prx_err_string(result));
            break;
        }
        addr_ptr = &addr;
        break;
    } 
    while (0);

    async_op->sock->itf.cb(async_op->sock->itf.context, pal_socket_event_end_recv,
        &async_op->buffer, &async_op->buf_len, addr_ptr, &flags, result, &async_op->context);
    pal_socket_async_op_init(async_op);
}

//
// Called when WriteFile completes
//
static void pal_socket_async_writefile_complete(
    pal_socket_async_t* async_op
)
{
    dbg_assert_ptr(async_op);
    dbg_assert_ptr(async_op->sock);

    // Complete write
    async_op->sock->itf.cb(async_op->sock->itf.context, pal_socket_event_end_send,
        &async_op->buffer, &async_op->buf_len, NULL, NULL, async_op->result,
        &async_op->context);
    pal_socket_async_op_init(async_op);
}

//
// Called when ReadFile completes
//
static void pal_socket_async_readfile_complete(
    pal_socket_async_t* async_op
)
{
    int32_t flags = 0;
    int32_t result = async_op->result;

    dbg_assert_ptr(async_op);
    dbg_assert_ptr(async_op->sock);

    // Complete read
    async_op->sock->itf.cb(async_op->sock->itf.context, pal_socket_event_end_recv,
        &async_op->buffer, &async_op->buf_len, NULL, &flags, result,
        &async_op->context);
    pal_socket_async_op_init(async_op);
}

//
// Create new socket fd that is bound to iocpt
//
static int32_t pal_socket_properties_to_fd(
    prx_socket_properties_t* props,
    SOCKET* sock_fd
)
{
    int32_t result;
    int os_af, os_type, os_proto;
    dbg_assert_ptr(props);
    dbg_assert_ptr(sock_fd);

    dbg_assert(props->family != prx_address_family_unix, "Only winsock expected");

    result = pal_os_from_prx_address_family(props->family, &os_af);
    if (result != er_ok)
        return result;
    result = pal_os_from_prx_socket_type(props->sock_type, &os_type);
    if (result != er_ok)
        return result;
    result = pal_os_from_prx_protocol_type(props->proto_type, &os_proto);
    if (result != er_ok)
        return result;

    *sock_fd = WSASocket(os_af, os_type, os_proto, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (*sock_fd == INVALID_SOCKET)
        return pal_os_last_net_error_as_prx_error();

    if (!BindIoCompletionCallback((HANDLE)*sock_fd, 
        pal_socket_async_complete_from_OVERLAPPED, 0))
    {
        closesocket(*sock_fd);
        *sock_fd = INVALID_SOCKET;
        return pal_os_last_error_as_prx_error();
    }
    return er_ok;
}

//
// Begin accept operation
//
static int32_t pal_socket_async_accept_begin(
    pal_socket_async_t* async_op
)
{
    int32_t result;
    DWORD received = 0;
    pal_socket_t* accepted;
    pal_socket_client_itf_t* accepted_itf;

    dbg_assert_ptr(async_op);
    dbg_assert_ptr(async_op->sock);
    dbg_assert_is_task(async_op->sock->scheduler);

    // Call receive and get a client socket option
    async_op->sock->itf.cb(async_op->sock->itf.context, pal_socket_event_begin_accept, 
        &async_op->buffer, &async_op->buf_len, NULL, NULL, er_ok, &async_op->context);
    if (!async_op->buffer || async_op->buf_len != sizeof(pal_socket_client_itf_t))
    {
        // Done accepting
        return er_nomore;
    }
    do
    {
        async_op->pending = true;

        accepted_itf = (pal_socket_client_itf_t*)async_op->buffer;
        if (accepted_itf->props.family == prx_address_family_unix)
        {
            result = er_not_supported;
            break;
        }

        // Create new socket object and handle to accept with
        accepted_itf->props.flags &= ~prx_socket_flag_passive;
        result = pal_socket_create(accepted_itf, &accepted);
        if (result != er_ok)
        {
            log_error(async_op->sock->log, "Failed to create Socket object. (%s)",
                prx_err_string(result));
            break;
        }

        result = pal_socket_properties_to_fd(&async_op->sock->itf.props, &accepted->sock_fd);
        if (result != er_ok)
        {
            log_error(async_op->sock->log, "Failed to create Socket handle. (%s)",
                prx_err_string(result));
            break;
        }

        async_op->buffer = (uint8_t*)accepted; // Now complete accept
        async_op->buf_len = sizeof(accepted);

        if (!_AcceptEx(async_op->sock->sock_fd, accepted->sock_fd, async_op->addr_buf,
            0, sizeof(async_op->addr_buf[0]), sizeof(async_op->addr_buf[1]),
            &received, &async_op->ov))
        {
            result = pal_os_last_net_error_as_prx_error();
            if (result == er_waiting)
                return result;
        }
        else
        {
            // Wait for callback
            return er_waiting;
        }
    } 
    while (0);
    
    // Complete now
    async_op->buf_len = received;
    async_op->result = result;
    return result;
}

//
// Begin ConnectNamedPipe operation
//
static int32_t pal_socket_async_connectpipe_begin(
    pal_socket_async_t* async_op
)
{
    int32_t result;
    DWORD received = 0;
    DWORD os_type;
    pal_socket_t* accepted;
    pal_socket_client_itf_t* accepted_itf;

    dbg_assert_ptr(async_op);
    dbg_assert_ptr(async_op->sock);
    dbg_assert_ptr(async_op->sock->ai_name);

    // Call receive and get a client socket interface
    async_op->sock->itf.cb(async_op->sock->itf.context, pal_socket_event_begin_accept,
        &async_op->buffer, &async_op->buf_len, NULL, NULL, er_ok, &async_op->context);
    if (!async_op->buffer || async_op->buf_len != sizeof(pal_socket_client_itf_t))
    {
        // Done accepting
        return er_nomore;
    }
    do
    {
        async_op->pending = true;

        accepted_itf = (pal_socket_client_itf_t*)async_op->buffer;
        if (accepted_itf->props.family != prx_address_family_unix)
        {
            result = er_not_supported;
            break;
        }

        // Create socket that represents named pipe
        accepted_itf->props.flags &= ~prx_socket_flag_passive;
        result = pal_socket_create(accepted_itf, &accepted);
        if (result != er_ok)
        {
            log_error(async_op->sock->log, "Failed to create named pipe socket object. (%s)",
                prx_err_string(result));
            break;
        }

        // Create named pipe handle
        if (accepted_itf->props.sock_type == prx_socket_type_dgram ||
            accepted_itf->props.sock_type == prx_socket_type_raw)
            os_type = PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE;
        else
            os_type = PIPE_TYPE_BYTE | PIPE_READMODE_BYTE;
        
        accepted->sock_fd = (SOCKET)CreateNamedPipeA(async_op->sock->ai_name,
            PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, os_type, PIPE_UNLIMITED_INSTANCES, 
            0x10000, 0x10000, 0, NULL);
        if ((HANDLE)accepted->sock_fd == INVALID_HANDLE_VALUE)
        {
            result = pal_os_last_error_as_prx_error();
            log_error(async_op->sock->log, "Failed to create named pipe handle. (%s)",
                prx_err_string(result));
            break;
        }

        if (!BindIoCompletionCallback((HANDLE)accepted->sock_fd, 
            pal_socket_async_complete_from_OVERLAPPED, 0))
        {
            result = pal_os_last_error_as_prx_error();
            log_error(async_op->sock->log, "Failed to bind named pipe handle to IOCP. (%s)",
                prx_err_string(result));
            break;
        }

        async_op->buffer = (uint8_t*)accepted; // Now complete connecting
        async_op->buf_len = sizeof(accepted);
        if (!ConnectNamedPipe((HANDLE)accepted->sock_fd, &async_op->ov))
        {
            result = pal_os_last_net_error_as_prx_error();
            if (result == er_waiting)
                return result;
        }
        else
        {
            // Wait for callback
            return er_waiting;
        }
    } 
    while (0);
    
    // Complete now
    async_op->buf_len = received;
    async_op->result = result;
    return result;
}

//
// Begin send operation
//
static int32_t pal_socket_async_send_begin(
    pal_socket_async_t* async_op
)
{
    int32_t result;
    DWORD sent = 0;
    int error, os_flags;
    WSABUF buf;
    int32_t flags;

    dbg_assert_ptr(async_op);
    dbg_assert_ptr(async_op->sock);

    async_op->sock->itf.cb(async_op->sock->itf.context, pal_socket_event_begin_send, 
        &async_op->buffer, &async_op->buf_len, NULL, &flags, er_ok, &async_op->context);
    if (!async_op->buffer)
    {
        // Done sending
        return er_nomore;
    }
    do
    {
        async_op->pending = true;
        result = pal_os_from_prx_message_flags(flags, &os_flags);
        if (result != er_ok)
        {
            log_error(async_op->sock->log, "Send received bad flags %d", os_flags);
            break;
        }

        buf.buf = (char*)async_op->buffer;
        buf.len = (u_long)async_op->buf_len;
        error = WSASend(async_op->sock->sock_fd, &buf, 1, &sent, (DWORD)os_flags,
            &async_op->ov, NULL);
        if (error != 0)
        {
            result = pal_os_last_net_error_as_prx_error();
            if (result == er_waiting)
                return result;
        }
        else
        {
            // Wait for callback
            return er_waiting;
        }
    } 
    while (0);

    // Complete now
    async_op->buf_len = sent;
    async_op->result = result;
    return result;
}

//
// Begin recv operation
//
static int32_t pal_socket_async_recv_begin(
    pal_socket_async_t* async_op
)
{
    int32_t result;
    DWORD received = 0;
    int error;
    WSABUF buf;

    dbg_assert_ptr(async_op);
    dbg_assert_ptr(async_op->sock);

    async_op->sock->itf.cb(async_op->sock->itf.context, pal_socket_event_begin_recv, 
        &async_op->buffer, &async_op->buf_len, NULL, NULL, er_ok, &async_op->context);
    if (!async_op->buffer)
    {
        // Done recv
        return er_nomore;
    }
    do
    {
        async_op->pending = true;
        buf.buf = (char*)async_op->buffer;
        buf.len = (u_long)async_op->buf_len;
        error = WSARecv(async_op->sock->sock_fd, &buf, 1, &received,
            &async_op->flags, &async_op->ov, NULL);
        if (error != 0)
        {
            result = pal_os_last_net_error_as_prx_error();
            if (result == er_waiting)
                return result;
        }
        else
        {
            // Wait for callback
            return er_waiting;
        }
    }
    while (0);

    // Complete now
    async_op->buf_len = received;
    async_op->result = result;
    return result;
}

//
// Begin sendto operation
//
static int32_t pal_socket_async_sendto_begin(
    pal_socket_async_t* async_op
)
{
    int32_t result;
    DWORD sent = 0;
    int error;
    WSABUF buf;
    int32_t flags;
    prx_socket_address_t addr;

    dbg_assert_ptr(async_op);
    dbg_assert_ptr(async_op->sock);

    async_op->sock->itf.cb(async_op->sock->itf.context, pal_socket_event_begin_send,
        &async_op->buffer, &async_op->buf_len, &addr, &flags, er_ok, &async_op->context);
    if (!async_op->buffer)
    {
        // Done sending
        return er_nomore;
    }
    do
    {
        async_op->pending = true;
        async_op->addr_len = sizeof(async_op->addr_buf);
        result = pal_os_from_prx_socket_address(&addr,
            (struct sockaddr*)async_op->addr_buf, &async_op->addr_len);
        if (result != er_ok)
        {
            log_error(async_op->sock->log, "Sendto received bad address (%s)",
                prx_err_string(result));
            break;
        }

        result = pal_os_from_prx_message_flags(flags, (int*)&async_op->flags);
        if (result != er_ok)
        {
            log_error(async_op->sock->log, "Sendto received bad flags %x (%s)",
                flags, prx_err_string(result));
            break;
        }

        buf.buf = (char*)async_op->buffer;
        buf.len = (u_long)async_op->buf_len;

        error = WSASendTo(async_op->sock->sock_fd, &buf, 1, &sent, async_op->flags,
            (struct sockaddr*)async_op->addr_buf, async_op->addr_len, &async_op->ov, 
            NULL);
        if (error != 0)
        {
            result = pal_os_last_net_error_as_prx_error();
            if (result == er_waiting)
                return result;
        }
        else
        {
            // Wait for callback
            return er_waiting;
        }
    }
    while (0);

    // Complete now
    async_op->buf_len = sent;
    async_op->result = result;
    return result;
}

//
// Begin recvfrom operation
//
static int32_t pal_socket_async_recvfrom_begin(
    pal_socket_async_t* async_op
)
{
    int32_t result;
    DWORD received = 0;
    int error;
    WSABUF buf;

    dbg_assert_ptr(async_op);
    dbg_assert_ptr(async_op->sock);

    async_op->sock->itf.cb(async_op->sock->itf.context, pal_socket_event_begin_recv, 
        &async_op->buffer, &async_op->buf_len, NULL, NULL, er_ok, &async_op->context);
    if (!async_op->buffer)
    {
        // Done receiving
        return er_nomore;
    }
    do
    {
        async_op->pending = true;
        buf.buf = (char*)async_op->buffer;
        buf.len = (u_long)async_op->buf_len;

        async_op->addr_len = sizeof(async_op->addr_buf);
        error = WSARecvFrom(async_op->sock->sock_fd, &buf, 1, &received,
            &async_op->flags, (struct sockaddr*)async_op->addr_buf,
            &async_op->addr_len, &async_op->ov, NULL);
        if (error != 0)
        {
            result = pal_os_last_net_error_as_prx_error();
            if (result == er_waiting)
                return result;
        }
        else
        {
            // Wait for callback
            return er_waiting;
        }
    }
    while (0);

    // Complete now
    async_op->buf_len = received;
    async_op->result = result;
    return result;
}

//
// Begin WriteFile operation
//
static int32_t pal_socket_async_writefile_begin(
    pal_socket_async_t* async_op
)
{
    int32_t result;
    DWORD sent = 0;
    int32_t flags;

    dbg_assert_ptr(async_op);
    dbg_assert_ptr(async_op->sock);

    async_op->sock->itf.cb(async_op->sock->itf.context, pal_socket_event_begin_send,
        &async_op->buffer, &async_op->buf_len, NULL, &flags, er_ok, &async_op->context);
    if (!async_op->buffer)
    {
        // Done sending
        return er_nomore;
    }
    do
    {
        async_op->pending = true;
        dbg_assert(!flags, "Not supported");

        if (!WriteFile((HANDLE)async_op->sock->sock_fd, async_op->buffer, 
            (DWORD)async_op->buf_len, &sent, &async_op->ov))
        {
            result = pal_os_last_error_as_prx_error();
            if (result == er_waiting)
                return result;
        }
        else
        {
            // Wait for callback
            return er_waiting;
        }
    } 
    while (0);

    // Complete now
    async_op->buf_len = sent;
    async_op->result = result;
    return result;
}

//
// Begin ReadFile operation
//
static int32_t pal_socket_async_readfile_begin(
    pal_socket_async_t* async_op
)
{
    int32_t result;
    DWORD received = 0;

    dbg_assert_ptr(async_op);
    dbg_assert_ptr(async_op->sock);

    async_op->sock->itf.cb(async_op->sock->itf.context, pal_socket_event_begin_recv,
        &async_op->buffer, &async_op->buf_len, NULL, NULL, er_ok, &async_op->context);
    if (!async_op->buffer)
    {
        // Done receiving
        return er_nomore;
    }
    do
    {
        async_op->pending = true;
        if (!ReadFile((HANDLE)async_op->sock->sock_fd, async_op->buffer,
            (DWORD)async_op->buf_len, &received, &async_op->ov))
        {
            result = pal_os_last_error_as_prx_error();
            if (result == er_waiting)
                return result;
        }
        else
        {
            // Wait for callback
            return er_waiting;
        }
    } 
    while (0);

    // Complete now
    async_op->buf_len = received;
    async_op->result = result;
    return result;
}


//
// Begin connect operation
//
static int32_t pal_socket_async_connect_begin(
    pal_socket_async_t* async_op
)
{
    int32_t result;
    DWORD error, tmp;

    dbg_assert_ptr(async_op);
    dbg_assert_ptr(async_op->sock);
    dbg_assert_is_task(async_op->sock->scheduler);

    // Connect socket
    async_op->pending = true;
    do
    {
        // Bind to in_addr_any = 0 - mandatory for ConnectEx
        memset(&async_op->addr_buf[1], 0, sizeof(async_op->addr_buf[1]));
        async_op->addr_buf[1].ss_family = async_op->addr_buf[0].ss_family;
        error = bind(async_op->sock->sock_fd,
            (const struct sockaddr*)&async_op->addr_buf[1],
            (int)async_op->addr_len);
        if (error != 0)
        {
            result = pal_os_last_net_error_as_prx_error();
            log_error(async_op->sock->log, "Failed binding socket for connect (%s)",
                prx_err_string(result));
            break;
        }
        
        if (!_ConnectEx(async_op->sock->sock_fd,
            (const struct sockaddr*)async_op->addr_buf, (int)async_op->addr_len,
            NULL, 0, &tmp, &async_op->ov))
        {
            result = pal_os_last_net_error_as_prx_error();
            if (result == er_waiting)
                return result;
            log_error(async_op->sock->log, "Failed connecting socket (%s)",
                prx_err_string(result));
        }
        else
        {
          //  log_trace(async_op->sock->log, "Socket connected synchronously!");
          //  result = er_ok;
            return er_waiting;
        }
    } 
    while (0);

    // Finish connect
    async_op->buf_len = 0;
    async_op->result = result;
    result = pal_socket_connect_complete(async_op);
    return result;
}

//
// Begin CreateFile/WaitPipe operation
//
static int32_t pal_socket_async_createfile_begin(
    pal_socket_async_t* async_op
)
{
    int32_t result;
    HANDLE h_file;

    dbg_assert_ptr(async_op);
    dbg_assert_ptr(async_op->sock);
    dbg_assert_is_task(async_op->sock->scheduler);

    // Connect socket
    async_op->pending = true;
    do
    {
        dbg_assert(async_op->addr_len == sizeof(struct sockaddr_un), "Address size");
        dbg_assert_ptr(async_op->sock->ai_name);

        h_file = CreateFileA(async_op->sock->ai_name, GENERIC_READ | GENERIC_WRITE, 
            0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
        if (h_file == INVALID_HANDLE_VALUE)
        {
            result = pal_os_last_error_as_prx_error();
            if (result == er_busy)
            {
                // There is no overlapped connect - the server needs to have its named 
                // pipe loop started and enough instances available.  Then we should  
                // be fine, otherwise, for now, fail fast...

                // TODO: Consider retry for timeout time on the scheduler.
            }

            log_error(async_op->sock->log, "Failed to create named pipe handle. (%s)",
                prx_err_string(result));
            break;
        }

        if (!BindIoCompletionCallback(h_file, pal_socket_async_complete_from_OVERLAPPED, 0))
        {
            CloseHandle(h_file);

            result = pal_os_last_error_as_prx_error();
            log_error(async_op->sock->log, "Failed to bind named pipe handle to IOCP. (%s)",
                prx_err_string(result));
            break;
        }

        async_op->sock->sock_fd = (SOCKET)h_file;
        result = er_ok;
    } 
    while (0);

    // Finish connect
    async_op->buf_len = 0;
    async_op->result = result;
    result = pal_socket_createfile_complete(async_op);
    return result;
}

//
// Begin bind operation
//
static int32_t pal_socket_bind(
    pal_socket_t* sock
)
{
    int32_t result;
    DWORD error;
    dbg_assert_ptr(sock);

    // Passive or listener, can only do synchronous
    do
    {
        error = bind(sock->sock_fd,
            (const struct sockaddr*)sock->open_op.addr_buf,
            (int)sock->open_op.addr_len);

        if (error != 0)
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
        if (error != 0)
        {
            result = pal_os_last_net_error_as_prx_error();
            log_error(sock->log, "Failed to set socket to listen (%s)",
                prx_err_string(result));
            break;
        }
 
        log_trace(sock->log, "Socket listening...");
        result = er_ok;
    }
    while(0);
    return result;
}

// 
// Try opening socket based on address in connect op
//
static int32_t pal_socket_open_begin(
    pal_socket_t *sock
)
{
    int32_t result;
    dbg_assert_ptr(sock);

    // Decide which path to take to open the socket...
    if (sock->itf.props.family == prx_address_family_unix)
    {
        // Create a name that is compatible with windows named pipes
        result = pal_socket_make_pipe_name(sock->itf.props.address.un.ux.path, 
            &sock->ai_name);
        if (result != er_ok)
        {
            log_error(sock->log, "Failed making pipe name for path %.108s (%s)!",
                sock->itf.props.address.un.ux.path, prx_err_string(result));
            return result;
        }

        if (sock->itf.props.flags & prx_socket_flag_passive)
        {
            // If server end (passive), wait for read to be enabled
            result = er_ok;
        }
        else
        {
            // If client end (!passive) - create file to connect to pipe
            result = pal_socket_async_createfile_begin(&sock->open_op);
        }
    }
    else
    {
        // Create new iocp socket and attempt to open based on properties
        result = pal_socket_properties_to_fd(&sock->itf.props, &sock->sock_fd);
        if (result != er_ok)
        {
            log_error(sock->log, "Failed creating iocp socket (%s)!",
                prx_err_string(result));
            return result;
        }

        if ((sock->itf.props.sock_type == prx_socket_type_seqpacket ||
             sock->itf.props.sock_type == prx_socket_type_rdm ||
             sock->itf.props.sock_type == prx_socket_type_stream) &&
           !(sock->itf.props.flags & prx_socket_flag_passive))
        {
            // If stream socket, and not passive, then use ConnectEx
            result = pal_socket_async_connect_begin(&sock->open_op);
        }
        else
        {
            // Otherwise, bind and optionally start listening...
            result = pal_socket_bind(sock);
        }
    }

    // Failed synchronously, close socket...
    if (result != er_ok && 
        result != er_waiting)
        pal_socket_close_handle(sock);
    return result;
}

//
// Open socket based on next address in cached list or fail
//
static void pal_socket_open_next_begin(
    pal_socket_t *sock
)
{
    int32_t result;
    dbg_assert_ptr(sock);
    dbg_assert_is_task(sock->scheduler);

    while (true)
    {
        if (sock->prx_ai_cur >= sock->prx_ai_count)
        {
            log_error(sock->log, "No other candidate addresses to open...");
            result = er_connecting;
            break;
        }

        // Setup async operation structure for open operation
        sock->open_op.addr_len = sizeof(sock->open_op.addr_buf);
        result = pal_os_from_prx_socket_address(&sock->prx_ai[sock->prx_ai_cur].address,
            (struct sockaddr*)sock->open_op.addr_buf, &sock->open_op.addr_len);
        if (result != er_ok)
            break;

        // Update address family in properties
        sock->itf.props.family = sock->prx_ai[sock->prx_ai_cur].address.un.family;

        // Start to open
        result = pal_socket_open_begin(sock);
        if (result == er_waiting)
            return; // Wait for callback

        if (result != er_ok)
        {
            sock->prx_ai_cur++;
            continue;  // Try next
        }
        // Success!
        log_debug(sock->log, "Socket opened synchronously!");
        break;
    }

    // Complete open which frees address list
    pal_socket_open_complete(sock, result);
}

//
// Resolve proxy address first and try to open each returned address
//
static void pal_socket_open_by_name_begin(
    pal_socket_t *sock
)
{
    int32_t result;
    char port[MAX_PORT_LENGTH];
    const char* server = NULL;
    uint32_t flags = 0;

    dbg_assert_ptr(sock);
    dbg_assert_is_task(sock->scheduler);

    do
    {
        dbg_assert(sock->itf.props.address.un.family == prx_address_family_proxy,
            "Bad address family");
        server = prx_socket_address_proxy_get_host(&sock->itf.props.address.un.proxy);
        if (server && !strlen(server))
            server = NULL;
        result = string_from_int(
            sock->itf.props.address.un.ip.port, 10, port, sizeof(port));
        if (result != er_ok)
            break;

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
        __do_next(sock, pal_socket_open_next_begin);
        return;
    } 
    while (0);

    // Complete open 
    pal_socket_open_complete(sock, result);
}

//
// Open address without resolving name
//
static void pal_socket_open_by_addr_begin(
    pal_socket_t *sock
)
{
    int32_t result;
    dbg_assert_ptr(sock);
    dbg_assert_is_task(sock->scheduler);
    do
    {
        dbg_assert(
            sock->itf.props.address.un.family != prx_address_family_proxy &&
            sock->itf.props.address.un.family != prx_address_family_unix,
            "Bad address family");

        // Setup async operation structure for open operation
        sock->open_op.addr_len = sizeof(sock->open_op.addr_buf);
        result = pal_os_from_prx_socket_address(&sock->itf.props.address,
            (struct sockaddr*)sock->open_op.addr_buf, &sock->open_op.addr_len);
        if (result != er_ok)
            break;

        // Update address family in properties
        sock->itf.props.family = sock->itf.props.address.un.family;

        // Begin open
        result = pal_socket_open_begin(sock);
        if (result == er_waiting)
            return; // Wait for callback
    } 
    while (0);

    // Complete open 
    pal_socket_open_complete(sock, result);
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
    dbg_assert(sock->sock_fd == INVALID_SOCKET, "Socket open");

    sock->open_op.enabled = true;

    /**/ if (sock->itf.props.address.un.family == prx_address_family_proxy)
        __do_next(sock, pal_socket_open_by_name_begin);
    else
        __do_next(sock, pal_socket_open_by_addr_begin);
    return er_ok;
}

//
// Enables send operation loop 
//
int32_t pal_socket_can_send(
    pal_socket_t* sock,
    bool ready
)
{
    chk_arg_fault_return(sock);
    if (sock->sock_fd == INVALID_SOCKET)
        return er_closed;

    if (ready)
        __do_next_s(sock->scheduler, pal_socket_async_enable, &sock->send_op);
    else
        __do_next_s(sock->scheduler, pal_socket_async_disable, &sock->send_op);
    return er_ok;
}

//
// Enables receive operation loop 
//
int32_t pal_socket_can_recv(
    pal_socket_t* sock,
    bool ready
)
{
    chk_arg_fault_return(sock);
    if (sock->sock_fd == INVALID_SOCKET)
        return er_closed;

    if (ready)
        __do_next_s(sock->scheduler, pal_socket_async_enable, &sock->recv_op);
    else
        __do_next_s(sock->scheduler, pal_socket_async_disable, &sock->recv_op);
    return er_ok;
}

//
// Create a new sock to track
//
int32_t pal_socket_create(
    pal_socket_client_itf_t* itf,
    pal_socket_t** created
)
{
    int32_t result;
    pal_socket_t* sock;

    chk_arg_fault_return(itf);
    chk_arg_fault_return(created);
    chk_arg_fault_return(itf->cb);

    sock = mem_zalloc_type(pal_socket_t);
    if (!sock)
        return er_out_of_memory;
    do
    {
        sock->log = log_get("pal_sk");
        sock->sock_fd = INVALID_SOCKET;
        result = prx_scheduler_create(_scheduler, &sock->scheduler);
        if (result != er_ok)
            break;

        memcpy(&sock->itf, itf, sizeof(pal_socket_client_itf_t));

        // Set function pointers based on type of socket
        sock->send_op.sock =
            sock;
        sock->recv_op.sock =
            sock;
        sock->open_op.sock =
            sock;

        if (sock->itf.props.family == prx_address_family_unix)
        {
            // Named pipe to mimic unix domain socket behavior
            
            sock->open_op.begin =
                pal_socket_async_op_init;
            sock->open_op.complete =
                pal_socket_async_connect_complete;

            if (sock->itf.props.flags & prx_socket_flag_passive)
            {
                // Server
                sock->send_op.begin =
                    pal_socket_async_op_init;
                sock->send_op.complete =
                    NULL;
                sock->recv_op.begin =
                    pal_socket_async_connectpipe_begin;
                sock->recv_op.complete =
                    pal_socket_async_connectpipe_complete;
            }
            else
            {
                // Client end points
                sock->send_op.begin =
                    pal_socket_async_writefile_begin;
                sock->send_op.complete =
                    pal_socket_async_writefile_complete;
                sock->recv_op.begin =
                    pal_socket_async_readfile_begin;
                sock->recv_op.complete =
                    pal_socket_async_readfile_complete;
            }
        }
        else 
        {
            // Regular socket

            sock->open_op.begin =
                pal_socket_async_op_init;
            sock->open_op.complete =
                pal_socket_async_connect_complete;

            if (sock->itf.props.sock_type == prx_socket_type_dgram ||
                sock->itf.props.sock_type == prx_socket_type_raw)
            {
                // Non connection oriented sockets recvfrom and sendto..
                sock->send_op.begin =
                    pal_socket_async_sendto_begin;
                sock->send_op.complete =
                    pal_socket_async_sendto_complete;
                sock->recv_op.begin =
                    pal_socket_async_recvfrom_begin;
                sock->recv_op.complete =
                    pal_socket_async_recvfrom_complete;
            }
            else if (sock->itf.props.flags & prx_socket_flag_passive)
            {
                // Listen socket, can only recv new sockets
                sock->send_op.begin =
                    pal_socket_async_op_init;
                sock->send_op.complete =
                    NULL;
                sock->recv_op.begin =
                    pal_socket_async_accept_begin;
                sock->recv_op.complete =
                    pal_socket_async_accept_complete;
            }
            else
            {
                // Stream socket can send and receive - no address
                sock->send_op.begin =
                    pal_socket_async_send_begin;
                sock->send_op.complete =
                    pal_socket_async_send_complete;
                sock->recv_op.begin =
                    pal_socket_async_recv_begin;
                sock->recv_op.complete =
                    pal_socket_async_recv_complete;
            }
        }

        *created = sock;
        return er_ok;
    } while (0);

    pal_socket_free(sock);
    return result;
}

//
// Create an opened / connected pair of asynchronous pipes
//
int32_t pal_socket_pair(
    pal_socket_client_itf_t* itf1,
    pal_socket_t** created1,
    pal_socket_client_itf_t* itf2,
    pal_socket_t** created2
)
{
    int32_t result;
    HANDLE fds[2];
    char pipe_name[32];
    OVERLAPPED ov;
    DWORD tmp;
    STRING_HANDLE rand;
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
    
    fds[0] = INVALID_HANDLE_VALUE;
    fds[1] = INVALID_HANDLE_VALUE;
    memset(&ov, 0, sizeof(ov));
    ov.hEvent = INVALID_HANDLE_VALUE;

    do
    {
        // Create random pipe name
        rand = STRING_construct_random(10);
        if (!rand)
            return er_out_of_memory;
        strcpy(&pipe_name[0], "\\\\.\\pipe\\");
        strcpy(&pipe_name[9], STRING_c_str(rand));
        STRING_delete(rand);

        // Create Event to wait on in overlapped connect
        ov.hEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
        if (ov.hEvent == INVALID_HANDLE_VALUE)
        {
            result = pal_os_last_error_as_prx_error();
            break;
        }

        // Create named pipe handle and connect it asynchronously
        if (itf1->props.sock_type == prx_socket_type_dgram ||
            itf1->props.sock_type == prx_socket_type_raw)
            tmp = PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE;
        else
            tmp = PIPE_TYPE_BYTE | PIPE_READMODE_BYTE;
        fds[0] = CreateNamedPipeA(pipe_name, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, 
            tmp, 1, 0x10000, 0x10000, 0, NULL);
        if (fds[0] == INVALID_HANDLE_VALUE)
        {
            result = pal_os_last_error_as_prx_error();
            log_error(NULL, "Failed to create named server pipe handle. (%s)",
                prx_err_string(result));
            break;
        }

        if (!ConnectNamedPipe(fds[0], &ov))
        {
            result = pal_os_last_net_error_as_prx_error();
            if (result != er_waiting)
            {
                log_error(NULL, "Failed to connect named server pipe handle. (%s)",
                    prx_err_string(result));
                break;
            }
        }

        fds[1] = CreateFileA(pipe_name, GENERIC_READ | GENERIC_WRITE, 0, NULL, 
            OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
        if (fds[1] == INVALID_HANDLE_VALUE)
        {
            result = pal_os_last_error_as_prx_error();
            log_error(NULL, "Failed to create/connect named client pipe handle. (%s)",
                prx_err_string(result));
            break;
        }

        // Wait for connect
        tmp = WaitForSingleObject(ov.hEvent, 5000);
        if (WAIT_OBJECT_0 != tmp)
        {
            // Failed to connect...
            if (tmp == WAIT_FAILED)
                result = pal_os_last_error_as_prx_error();
            else
                result = er_timeout;
            log_error(NULL, "Failed connecting both pipe handles to pair... (%s)",
                prx_err_string(result));
            break;
        }

        CloseHandle(ov.hEvent);
        ov.hEvent = INVALID_HANDLE_VALUE;

        // Make sure that sockets are created as pipe sockets - adjust address family
        itf1->props.family = itf2->props.family = prx_address_family_unix;

        // Create socket objects to wrap both pipe handles in them
        result = pal_socket_create(itf1, &sock1);
        if (result != er_ok)
            break;
        sock1->sock_fd = (SOCKET)fds[0];
        fds[0] = INVALID_HANDLE_VALUE;
        result = pal_socket_create(itf2, &sock2);
        if (result != er_ok)
            break;
        sock2->sock_fd = (SOCKET)fds[1];
        fds[1] = INVALID_HANDLE_VALUE;

        // Bind both handles to completion port
        if (!BindIoCompletionCallback(
                (HANDLE)sock1->sock_fd, pal_socket_async_complete_from_OVERLAPPED, 0) ||
            !BindIoCompletionCallback(
                (HANDLE)sock2->sock_fd, pal_socket_async_complete_from_OVERLAPPED, 0))
        {
            result = pal_os_last_error_as_prx_error();
            log_error(NULL, "Failed to bind named pipe handle to IOCP. (%s)",
                prx_err_string(result));
            break;
        }

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
        if (ov.hEvent != INVALID_HANDLE_VALUE)
            CloseHandle(ov.hEvent);
        if (fds[0] != INVALID_HANDLE_VALUE)
            CloseHandle(fds[0]);
        if (fds[1] != INVALID_HANDLE_VALUE)
            CloseHandle(fds[1]);
        if (sock1)
            pal_socket_free(sock1);
        if (sock2)
            pal_socket_free(sock2);
    }

    return er_ok;
}

// 
// Close and disconnect socket
//
void pal_socket_close(
    pal_socket_t *sock
)
{
    if (!sock)
        return;
    // Close socket and cancel io in progress
    __do_next(sock, pal_socket_try_close);
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
    int32_t opt_lvl, opt_name;
    socklen_t opt_len;
    int32_t opt_val;
    u_long ulavail;
    DWORD dwavail;

    chk_arg_fault_return(sock);

    if (socket_option == prx_so_shutdown)
        return er_not_supported;

    if (socket_option == prx_so_available)
    {
        if (sock->itf.props.family == prx_address_family_unix)
        {
            dwavail = 0;
            if (!PeekNamedPipe((HANDLE)sock->sock_fd, NULL, 0, NULL, &dwavail, NULL))
                return pal_os_last_error_as_prx_error();
            *value = dwavail;
        }
        else
        {
            ulavail = 0;
            result = ioctlsocket(sock->sock_fd, FIONREAD, &ulavail);
            if (result == SOCKET_ERROR)
                return pal_os_last_net_error_as_prx_error();
            *value = ulavail;
        }
        return er_ok;
    }
    
    if (sock->itf.props.family == prx_address_family_unix)
        return er_not_supported;

    if (socket_option == prx_so_linger)
    {
        struct linger opt;
        opt_len = sizeof(opt);
        result = getsockopt(sock->sock_fd, SOL_SOCKET, SO_LINGER, 
            (char*)&opt, &opt_len);
        if (result != 0)
            return pal_os_last_net_error_as_prx_error();
        *value = opt.l_onoff ? opt.l_linger : 0;
        return er_ok;
    }

    result = pal_os_from_prx_socket_option(socket_option, &opt_lvl, &opt_name);
    if (result != er_ok)
        return result;

    opt_len = sizeof(int32_t);
    result = getsockopt(sock->sock_fd, opt_lvl, opt_name, 
        (char*)&opt_val, &opt_len);
    if (result != 0)
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
    int32_t opt_lvl, opt_name;
    int32_t opt_val;

    /**/ if (socket_option == prx_so_available)
        return er_not_supported;
    else if (socket_option == prx_so_shutdown)
    {
        if (value != prx_shutdown_op_read)
            sock->send_op.begin = pal_socket_async_op_init;
        if (value != prx_shutdown_op_write)
            sock->recv_op.begin = pal_socket_async_op_init;

        result = pal_os_from_prx_shutdown_op((prx_shutdown_op_t)value, &opt_val);
        if (result != er_ok)
            return result;
        if (sock->itf.props.family == prx_address_family_unix)
        {
            (void)DisconnectNamedPipe((HANDLE)sock->sock_fd);
        }
        else
        {
            result = shutdown(sock->sock_fd, opt_val);
        }
    }
    else if (sock->itf.props.family == prx_address_family_unix)
        return er_not_supported;
    else if (socket_option == prx_so_linger)
    {
        struct linger opt;
        opt.l_onoff = !!value;
        opt.l_linger = (unsigned short)value;
        result = setsockopt(sock->sock_fd, SOL_SOCKET, SO_LINGER,
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

        result = setsockopt(sock->sock_fd, opt_lvl, opt_name, 
            (const char*)&opt_val, (socklen_t)sizeof(opt_val));
    }
    return result == 0 ? er_ok : pal_os_last_net_error_as_prx_error();
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
    chk_arg_fault_return(sock);
    chk_arg_fault_return(socket_address);
    memcpy(socket_address, &sock->local, sizeof(prx_socket_address_t));
    return er_ok;
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
// Leave multicast group
//
int32_t pal_socket_leave_multicast_group(
    pal_socket_t* sock,
    const prx_multicast_option_t* option
)
{
    int32_t result;
    struct ipv6_mreq opt6;
    struct ip_mreq opt;

    chk_arg_fault_return(sock);
    chk_arg_fault_return(option);

    if (sock->itf.props.family == prx_address_family_unix)
        return er_not_supported;

    switch (option->family)
    {
    case prx_address_family_inet:
        opt.imr_multiaddr.s_addr = option->addr.in4.un.addr;
        opt.imr_interface.s_addr = option->itf_index;
        result = setsockopt(
            sock->sock_fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&opt, sizeof(opt));
        break;
    case prx_address_family_inet6:
        memcpy(opt6.ipv6mr_multiaddr.s6_addr, option->addr.in6.un.u8,
            sizeof(option->addr.in6.un.u8));
        opt6.ipv6mr_interface = option->itf_index;
        result = setsockopt(
            sock->sock_fd, IPPROTO_IPV6, IPV6_DROP_MEMBERSHIP, (char*)&opt6, sizeof(opt6));
        break;
    default:
        return er_not_supported;
    }
    return result == 0 ? er_ok : pal_os_last_net_error_as_prx_error();
}

//
// Join multicast group
//
int32_t pal_socket_join_multicast_group(
    pal_socket_t* sock,
    const prx_multicast_option_t* option
)
{
    int32_t result;
    struct ipv6_mreq opt6;
    struct ip_mreq opt;

    chk_arg_fault_return(sock);
    chk_arg_fault_return(option);

    if (sock->itf.props.family == prx_address_family_unix)
        return er_not_supported;

    switch (option->family)
    {
    case prx_address_family_inet:
        opt.imr_multiaddr.s_addr = option->addr.in4.un.addr;
        opt.imr_interface.s_addr = option->itf_index;
        result = setsockopt(
            sock->sock_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&opt, sizeof(opt));
        break;
    case prx_address_family_inet6:
        memcpy(opt6.ipv6mr_multiaddr.s6_addr, option->addr.in6.un.u8,
            sizeof(option->addr.in6.un.u8));
        opt6.ipv6mr_interface = option->itf_index;
        result = setsockopt(
            sock->sock_fd, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, (char*)&opt6, sizeof(opt6));
        break;
    default:
        return er_not_supported;
    }
    return result == 0 ? er_ok : pal_os_last_net_error_as_prx_error();
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
    dbg_assert(sock->sock_fd == INVALID_SOCKET, "socket still open");

    if (sock->ai_name)
        mem_free(sock->ai_name);
    if (sock->scheduler)
        prx_scheduler_release(sock->scheduler, sock);

    mem_free_type(pal_socket_t, sock);
}

//
// Socket pair abstraction to support controling event loops
//
int socketpair(
    int domain, 
    int type, 
    int protocol,
    fd_t socks[2]
)
{
    SOCKET listener;
    SOCKADDR_STORAGE sa_buf;
    struct sockaddr_in* sa_in = (struct sockaddr_in*)&sa_buf;
    struct sockaddr* sa = (struct sockaddr*)&sa_buf;
    int error, tmp = 1, len;

    dbg_assert_ptr(socks);

    // We only expect the once used for control sockets here...
    dbg_assert(domain == AF_UNIX, "Unexpected domain");
    dbg_assert(type == SOCK_STREAM, "Unexpected stream");
    dbg_assert(protocol == 0, "Unexpected proto");
    (void)domain;
    (void)type;
    (void)protocol;

    listener = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP,
        NULL, 0, 0);
    if (listener == INVALID_SOCKET)
        return -1;

    socks[0] = socks[1] = INVALID_SOCKET;
    do
    {
        len = (int)sizeof(tmp);
        if (SOCKET_ERROR == setsockopt(listener, SOL_SOCKET,
            SO_REUSEADDR, (char*)&tmp, len))
        {
            log_error(NULL, "Failed to set up listener.");
            break;
        }

        // Start listening
        memset(sa_in, 0, sizeof(struct sockaddr_in));
        sa_in->sin_family = AF_INET;
        sa_in->sin_addr.s_addr = htonl(INADDR_LOOPBACK);

        len = (int)sizeof(sa_buf);
        if (SOCKET_ERROR == bind(
                listener, sa, sizeof(struct sockaddr_in)) ||
            SOCKET_ERROR == getsockname(
                listener, sa, &len) ||
            SOCKET_ERROR == listen(
                listener, 1))
        {
            log_error(NULL, "Failed to set up listener for pipe.");
            break;
        }

        // Connect
        socks[0] = (fd_t)WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 
            NULL, 0, 0);
        if (socks[0] == INVALID_SOCKET)
            break;
        if (SOCKET_ERROR == connect(socks[0], sa, len))
        {
            log_error(NULL, "Failed to connect client end of pipe.");
            break;
        }

        // Accept connection
        socks[1] = (fd_t)accept(listener, NULL, NULL);
        if (socks[1] == -1)
        {
            log_error(NULL, "Failed to set up server end of pipe.");
            break;
        }
        closesocket(listener);
        return 0;
    } 
    while (0);

    error = WSAGetLastError();
    if (listener != INVALID_SOCKET)
        closesocket(listener);
    if (socks[0] != INVALID_SOCKET)
        closesocket(socks[0]);
    if (socks[1] != INVALID_SOCKET)
        closesocket(socks[1]);
    WSASetLastError(error);
    return -1;
}

#if defined(USE_OPENSSL)
#include "azure_c_shared_utility/tlsio_openssl.h"
#else
#include "azure_c_shared_utility/tlsio_schannel.h"
#endif

//
// Return default tls implentation
//
const IO_INTERFACE_DESCRIPTION* platform_get_default_tlsio(void)
{
#if defined(USE_OPENSSL)
    return tlsio_openssl_get_interface_description();
#else
    return tlsio_schannel_get_interface_description();
#endif
}

//
// Initialize the winsock layer and retrieve function pointers
//
int32_t pal_socket_init(
    uint32_t* caps
)
{
    int32_t result;
    int error;
    WSADATA wsd;
    SOCKET s;
    DWORD cb;

    GUID guid_connectex = WSAID_CONNECTEX;
    GUID guid_acceptex = WSAID_ACCEPTEX;
    GUID guid_getacceptexsockaddrs = WSAID_GETACCEPTEXSOCKADDRS;

    chk_arg_fault_return(caps);

    if (_scheduler)
        return er_bad_state;
    result = prx_scheduler_create(NULL, &_scheduler);
    if (result != er_ok)
        return result;
#if defined(USE_OPENSSL)
    (void)tlsio_openssl_init();
#endif
    // To format NTSTATUS errors
    _ntdll = LoadLibraryA("NTDLL.DLL");
    error = WSAStartup(MAKEWORD(2, 2), &wsd);
    if (error != 0)
        return pal_socket_from_os_error(error);
    do
    {
        // Retrieve winsock function pointers
        s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
        if (s == INVALID_SOCKET)
        {
            error = SOCKET_ERROR;
            break;
        }

        error = WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER,
            &guid_connectex, sizeof(guid_connectex),
            &_ConnectEx, sizeof(_ConnectEx), &cb, NULL, NULL);
        if (error != 0)
            break;
        error = WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER,
            &guid_acceptex, sizeof(guid_acceptex),
            &_AcceptEx, sizeof(_AcceptEx), &cb, NULL, NULL);
        if (error != 0)
            break;
        error = WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER,
            &guid_getacceptexsockaddrs, sizeof(guid_getacceptexsockaddrs),
            &_GetAcceptExSockAddrs, sizeof(_GetAcceptExSockAddrs),
            &cb, NULL, NULL);
        if (error != 0)
            break;
    } 
    while (0);

    if (s != INVALID_SOCKET)
        closesocket(s);
    if (error != 0)
    {
        log_error(NULL, "Couldn't get WSA function pointers.");
        pal_socket_deinit();
        return pal_os_last_net_error_as_prx_error();
    }

    (*caps) |= (pal_cap_sockets | pal_cap_ev);
    return er_ok;
}

//
// Deinit socket layer
//
void pal_socket_deinit(
    void
)
{
    int error;
    error = WSACleanup();
    if (error != 0)
        (void)pal_socket_from_os_error(error);
#if defined(USE_OPENSSL)
    tlsio_openssl_deinit();
#endif
    if (_scheduler)
    {
        prx_scheduler_release(_scheduler, NULL);
        prx_scheduler_at_exit(_scheduler);
        _scheduler = NULL;
    }
    if (_ntdll)
        (void)FreeLibrary(_ntdll);
    _ntdll = NULL;
}
