// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "xio_sk.h"
#include "pal.h"
#include "pal_sk.h"
#include "io_queue.h"
#include "prx_buffer.h"
#include "prx_sched.h"
#include "util_string.h"

#include "azure_c_shared_utility/xio.h"

//
// Receive in 64 k increments
//
#define DEFAULT_RECEIVE_BUFFER_SIZE 0x10000

//
// Socket interface for xio
//
typedef struct xio_socket
{
    pal_socket_t* sock;
    prx_scheduler_t* scheduler;

    ON_BYTES_RECEIVED on_bytes_received;
    void* on_bytes_received_context;
    ON_IO_OPEN_COMPLETE on_io_open_complete;
    void* on_io_open_complete_context;
    ON_IO_ERROR on_io_error;
    void* on_io_error_context;
    ON_IO_CLOSE_COMPLETE on_io_close_complete;
    void* on_io_close_complete_context;

    bool recv_enabled;
    io_queue_t* inbound;     // Inbound buffer queue with received buffers
    io_queue_t* outbound;               // Outbound queue for send buffers
    int32_t last_error;  // Last error received in connect/disconnect path
    bool closed;
    bool destroy;
    log_t log;
}
xio_socket_t;

//
// Free xio context on scheduler thread when fully disconnected.
//
void xio_socket_free(
    xio_socket_t* sk
)
{
    dbg_assert_ptr(sk);
    
    sk->destroy = true;
    if (!sk->closed)
    {
        pal_socket_close(sk->sock);
        return; // Called again when closed..
    }

    sk->on_io_close_complete = NULL;
    sk->on_io_error = NULL;

    if (sk->sock)
        pal_socket_free(sk->sock);

    if (sk->outbound)
        io_queue_free(sk->outbound);

    if (sk->inbound)
        io_queue_free(sk->inbound);

    if (sk->scheduler)
        prx_scheduler_release(sk->scheduler, sk);

    mem_free_type(xio_socket_t, sk);
}

//
// Scheduler task that delivers the result of the open operation
//
static void xio_socket_deliver_open_result(
    xio_socket_t* sk
)
{
    /**/ if (sk->on_io_open_complete)
    {
        sk->recv_enabled = true;
        sk->on_io_open_complete(sk->on_io_open_complete_context,
            sk->last_error != er_ok ? IO_OPEN_ERROR : IO_OPEN_OK);
        
        // Start receiving and sending...
        pal_socket_can_recv(sk->sock, true);
        pal_socket_can_send(sk->sock, true);
    }
    else if (sk->on_io_error && sk->last_error != er_ok)
    {
        sk->on_io_error(sk->on_io_error_context);
    }
}

//
// Scheduler task that delivers all received buffers or errors on scheduler
//
static void xio_socket_deliver_inbound_results(
    xio_socket_t* sk
)
{
    io_queue_buffer_t* buffer;
    size_t size;
    const uint8_t* buf;

    dbg_assert_ptr(sk);
    if (sk->closed)
        return;

    while (true)
    {
        buffer = io_queue_pop_done(sk->inbound);
        if (!buffer)
            break;

        size = buffer->length;
        buf = (const uint8_t*)io_queue_buffer_to_ptr(buffer);

        // Deliver read buffer
        /**/ if (buffer->code != er_ok && sk->on_io_error)
        {
            sk->on_io_error(
                sk->on_io_error_context);
        }
        else if (size > 0 && sk->on_bytes_received)
        {
            sk->on_bytes_received(
                sk->on_bytes_received_context, buf, size);
        }

        io_queue_buffer_release(buffer);
    }
}

//
// Scheduler task that delivers all send results 
//
static void xio_socket_deliver_outbound_results(
    xio_socket_t* sk
)
{
    io_queue_buffer_t* buffer;
    dbg_assert_ptr(sk);

    while (true)
    {
        buffer = io_queue_pop_done(sk->outbound);
        if (!buffer)
            break;

        if (buffer->cb_ptr)
        {
            ((ON_SEND_COMPLETE)buffer->cb_ptr)(buffer->ctx,
                buffer->code != er_ok ? IO_SEND_ERROR : IO_SEND_OK);
        }
        io_queue_buffer_release(buffer);
    }
}

//
// Scheduler task that delivers close result
//
static void xio_socket_deliver_close_result(
    xio_socket_t* sk
)
{
    io_queue_buffer_t* buffer;

    xio_socket_deliver_inbound_results(sk);
    xio_socket_deliver_outbound_results(sk);

    // Roll back all in progress buffers
    io_queue_rollback(sk->outbound);

    while (true)
    {
        buffer = io_queue_pop_ready(sk->outbound);
        if (!buffer)
            break;
        if (buffer->cb_ptr)
            ((ON_SEND_COMPLETE)buffer->cb_ptr)(buffer->ctx, IO_SEND_CANCELLED);
        io_queue_buffer_release(buffer);
    }

    if (sk->on_io_error && sk->last_error != er_ok)
    {
        sk->on_io_error(sk->on_io_error_context);
        sk->on_io_error = NULL;
    }

    if (sk->on_io_close_complete)
    {
        sk->on_io_close_complete(sk->on_io_close_complete_context);
        sk->on_io_close_complete = NULL;
    }

    sk->closed = true;

    // Free xio if destroy was called as well...
    if (sk->destroy)
    {
        __do_next(sk, xio_socket_free);
    }
}

//
// Start closing
//
static void xio_socket_on_begin_close(
    xio_socket_t* sk
)
{
    dbg_assert_ptr(sk);
    if (!sk->sock)
        return;
    pal_socket_close(sk->sock);
    sk->sock = NULL;
}

//
// Pal callback to begin receive operation
//
static void xio_socket_on_begin_receive(
    xio_socket_t* sk,
    uint8_t** buf,
    size_t* length
)
{
    int32_t result;
    io_queue_buffer_t* buffer = NULL;

    dbg_assert_ptr(sk);
    dbg_assert_ptr(buf);
    dbg_assert_ptr(length);

    result = io_queue_create_buffer(sk->inbound, NULL,
        DEFAULT_RECEIVE_BUFFER_SIZE, &buffer);
    if (result != er_ok)
    {
        *buf = NULL;
        *length = 0;
    }
    else
    {
        *buf = io_queue_buffer_to_ptr(buffer);
        *length = DEFAULT_RECEIVE_BUFFER_SIZE;
    }
}

//
// Pal callback to complete receiving operation
//
static void xio_socket_on_end_receive(
    xio_socket_t* sk,
    uint8_t** buf,
    size_t* length,
    int32_t* flags,
    int32_t result
)
{
    io_queue_buffer_t* buffer;

    dbg_assert_ptr(sk);
    dbg_assert_ptr(buf);
    dbg_assert_ptr(length);

    (void)flags;  // Flags are irrelevant to us

    buffer = io_queue_buffer_from_ptr(*buf);
    dbg_assert_ptr(buffer);

    if (result != er_ok && result != er_aborted && result != er_retry)
    {
        log_error(sk->log, "Error during pal receive (%s).",
            prx_err_string(result));
    }

    if (result == er_retry)
    {
        // Short cut, just release
        io_queue_buffer_release(buffer);
    }
    else
    {
        // Mark done and deliver on scheduler thread
        buffer->code = result;
        buffer->length = buffer->write_offset = *length;
        buffer->read_offset = 0;
        io_queue_buffer_set_done(buffer);
        __do_next(sk, xio_socket_deliver_inbound_results);
    }
}

//
// Pal callback to request a buffer to be send
//
static void xio_socket_on_begin_send(
    xio_socket_t* sk,
    uint8_t** buf,
    size_t* length,
    int32_t* flags
)
{
    io_queue_buffer_t* buffer = NULL;

    dbg_assert_ptr(sk);
    dbg_assert_ptr(buf);
    dbg_assert_ptr(length);
    dbg_assert_ptr(flags);

    buffer = io_queue_pop_inprogress(sk->outbound);
    if (!buffer)
        buffer = io_queue_pop_ready(sk->outbound);
    if (!buffer)
    {
        *buf = NULL;
        *length = 0;
        *flags = 0;
    }
    else
    {
        *buf = io_queue_buffer_to_ptr(buffer);
        *length = buffer->write_offset;
        *flags = 0;
    }
}

//
// Pal callback to complete send operation
//
static void xio_socket_on_end_send(
    xio_socket_t* sk,
    uint8_t** buf,
    size_t* length,
    int32_t result
)
{
    io_queue_buffer_t* buffer;

    dbg_assert_ptr(sk);
    dbg_assert_ptr(length);
    dbg_assert_ptr(*buf);

    if (result != er_ok && result != er_aborted && result != er_retry)
    {
        log_error(sk->log, "Error during pal socket send (%s).",
            prx_err_string(result));
    }

    buffer = io_queue_buffer_from_ptr(*buf);
    dbg_assert_ptr(buffer);

    if (result == er_retry)
    {
        // Retry - put into in progress
        io_queue_buffer_set_inprogress(buffer);
        return;
    }

    dbg_assert(buffer->write_offset == *length || result != er_ok, 
        "Not all sent (%zu != %zu)", buffer->write_offset, *length);

    if (buffer->cb_ptr && !sk->closed)
    {
        // Set done and deliver
        buffer->code = result;
        buffer->read_offset = *length;
        
        io_queue_buffer_set_done(buffer);
        __do_next(sk, xio_socket_deliver_outbound_results);
    }
    else
    {
        // Short cut, just release
        io_queue_buffer_release(buffer);
    }
}

//
// Pal event callback 
//
static void xio_socket_event_handler(
    void* context,
    pal_socket_event_t ev,
    uint8_t** buffer,
    size_t* size,
    prx_socket_address_t* addr,  
    int32_t* flags,           
    int32_t error,
    void** op_context
)
{
    xio_socket_t* sk = (xio_socket_t*)context;

    dbg_assert(!addr, "no address expected.");
    (void)op_context;
    (void)addr;

    switch (ev)
    {
    case pal_socket_event_opened:
        sk->last_error = error;
        __do_next(sk, xio_socket_deliver_open_result);
        break;
    case pal_socket_event_begin_recv:
        dbg_assert(error == er_ok, "no error expected.");
        xio_socket_on_begin_receive(sk, buffer, size);
        break;
    case pal_socket_event_end_recv:
        xio_socket_on_end_receive(sk, buffer, size, flags, error);
        break;
    case pal_socket_event_begin_send:
        dbg_assert(error == er_ok, "no error expected.");
        xio_socket_on_begin_send(sk, buffer, size, flags);
        break;
    case pal_socket_event_end_send:
        xio_socket_on_end_send(sk, buffer, size, error);
        break;
    case pal_socket_event_closed:
        sk->last_error = error;
        __do_next(sk, xio_socket_deliver_close_result);
        break;
    case pal_socket_event_begin_accept:
    case pal_socket_event_end_accept:
    default:
        dbg_assert(0, "Should not be here!");
    }
}

//
// Send buffer and indicate completion
//
static int32_t xio_socket_send(
    CONCRETE_IO_HANDLE handle,
    const void* buf,
    size_t size,
    ON_SEND_COMPLETE on_send_complete,
    void* callback_context
)
{
    int32_t result;
    io_queue_buffer_t* buffer;
    xio_socket_t* sk = (xio_socket_t*)handle;

    chk_arg_fault_return(handle);
    chk_arg_fault_return(buf);
    chk_arg_fault_return(size);

    result = io_queue_create_buffer(sk->outbound, buf, size, &buffer);
    if (result != er_ok)
        return result;

    buffer->cb_ptr = (void(*)(void*, int32_t))on_send_complete;
    buffer->ctx = callback_context;

    io_queue_buffer_set_ready(buffer);

    pal_socket_can_send(sk->sock, true);
    
    if (sk->recv_enabled)
    {
        pal_socket_can_recv(sk->sock, true);
    }
    return er_ok;
}

//
// Enable or disable receive
//
static int32_t xio_socket_recv(
    CONCRETE_IO_HANDLE handle,
    bool enabled
)
{
    xio_socket_t* sk = (xio_socket_t*)handle;
    sk->recv_enabled = enabled;
    return pal_socket_can_recv(sk->sock, sk->recv_enabled);
}

//
// Open interface
//
static int32_t xio_socket_open(
    CONCRETE_IO_HANDLE handle,
    ON_IO_OPEN_COMPLETE on_io_open_complete,
    void* on_io_open_complete_context,
    ON_BYTES_RECEIVED on_bytes_received,
    void* on_bytes_received_context,
    ON_IO_ERROR on_io_error,
    void* on_io_error_context
)
{
    int result;
    xio_socket_t* sk = (xio_socket_t*)handle;
    chk_arg_fault_return(handle);

    if (!sk->scheduler)
    {
        result = prx_scheduler_create(NULL, &sk->scheduler);
        if (result != er_ok)
            return result;
    }

    sk->on_io_close_complete = NULL;

    sk->on_bytes_received = on_bytes_received;
    sk->on_bytes_received_context = on_bytes_received_context;
    sk->on_io_open_complete = on_io_open_complete;
    sk->on_io_open_complete_context = on_io_open_complete_context;
    sk->on_io_error = on_io_error;
    sk->on_io_error_context = on_io_error_context;

    sk->last_error = er_ok;

    return pal_socket_open(sk->sock);
}

//
// Close interface
//
static int32_t xio_socket_close(
    CONCRETE_IO_HANDLE handle,
    ON_IO_CLOSE_COMPLETE on_io_close_complete,
    void* on_io_close_complete_context
)
{
    xio_socket_t* sk = (xio_socket_t*)handle;
    chk_arg_fault_return(handle);

    dbg_assert(!sk->destroy, "Closing destroyed xio");

    sk->on_bytes_received = NULL;
    sk->on_io_open_complete = NULL;
    sk->on_io_error = NULL;
    sk->last_error = er_ok;

    if (!sk->on_io_close_complete)
    {
        sk->on_io_close_complete = on_io_close_complete;
        sk->on_io_close_complete_context = on_io_close_complete_context;
        __do_next(sk, xio_socket_on_begin_close);
    }
    else
    {
        if (on_io_close_complete)
            on_io_close_complete(on_io_close_complete_context);
    }
    return er_ok;
}

//
// Destroy io handle
//
static void xio_socket_destroy(
    CONCRETE_IO_HANDLE handle
)
{
    xio_socket_t* sk = (xio_socket_t*)handle;
    if (!sk)
        return;
    if (!sk->scheduler)
    {
        xio_socket_free(sk);
        return;
    }
 
    // Stop any queued actions to ensure controlled free
    if (sk->scheduler)
        prx_scheduler_clear(sk->scheduler, NULL, sk);

    // Detach any callbacks from buffers
    io_queue_abort(sk->outbound);

    // Kick off free
    __do_next(sk, xio_socket_free);
}

//
// Create sock interface with given parameters
//
CONCRETE_IO_HANDLE xio_socket_create(
    void* io_create_parameters
)
{
    int32_t result;
    xio_sk_config_t* config;
    pal_socket_client_itf_t client;
    xio_socket_t* sk;

    config = (xio_sk_config_t*)io_create_parameters;
    if (!config || !config->hostname)
        return NULL;
    if (config->port == 0 || config->port >= 0xffff)
        return NULL;
    sk = mem_zalloc_type(xio_socket_t);
    if (!sk)
        return NULL;
    do
    {
        sk->log = log_get("xio_sk");

        memset(&client.props, 0, sizeof(client.props));
        client.props.family = prx_address_family_unspec;
        client.props.sock_type = prx_socket_type_stream;
        client.props.proto_type = prx_protocol_type_tcp;

        strncpy(client.props.address.un.proxy.host_fix, config->hostname, 
            sizeof(client.props.address.un.proxy.host_fix));
        client.props.address.un.proxy.family = prx_address_family_proxy;
        client.props.address.un.proxy.port = (int16_t)config->port;

        result = io_queue_create("socket-outbound", &sk->outbound);
        if (result != er_ok)
            break;
        result = io_queue_create("socket-inbound", &sk->inbound);
        if (result != er_ok)
            break;

        client.context = sk;
        client.cb = xio_socket_event_handler;
        result = pal_socket_create(&client, &sk->sock);
        if (result != er_ok)
            break;
        return (CONCRETE_IO_HANDLE)sk;

    } while (0);

    xio_socket_free(sk);
    return NULL;
}

//
// Reads data from the socket interface
//
void xio_socket_dowork(
    CONCRETE_IO_HANDLE handle
)
{
    (void)handle;
    return;
}

//
// Retrieve option
//
static OPTIONHANDLER_HANDLE xio_socket_retrieve_option(
    CONCRETE_IO_HANDLE handle
)
{
    (void)handle;
    return NULL;
}

//
// Io control
//
int xio_socket_setoption(
    CONCRETE_IO_HANDLE handle,
    const char* option_name, 
    const void* buffer
)
{
    int result;
    xio_socket_t* sk = (xio_socket_t*)handle;

    chk_arg_fault_return(handle);
    /**/ if (0 == string_compare(option_name, xio_opt_scheduler))
        result = prx_scheduler_create((prx_scheduler_t*)buffer, &sk->scheduler);
    else if (0 == string_compare(option_name, xio_opt_flow_ctrl))
        result = xio_socket_recv(handle, *((uint32_t*)buffer) != 0);
    else
        result = er_not_supported;

    return result;
}

//
// Interface definition table
//
static const IO_INTERFACE_DESCRIPTION xio_socket_description =
{
    xio_socket_retrieve_option,
    xio_socket_create,
    xio_socket_destroy,
    xio_socket_open,
    xio_socket_close,
    xio_socket_send,
    xio_socket_dowork,
    xio_socket_setoption
};

const IO_INTERFACE_DESCRIPTION* socketio_get_interface_description(
    void
)
{
    return &xio_socket_description;
}
