// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"

#include "common.h"
#include "io_queue.h"
#include "prx_sched.h"
#include "util_string.h"
#include "pal_ws.h"
#include "pal_mt.h"
#include "xio_sk.h"

#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/wsio.h"

//
// Receive in 16 k increments
//
#define DEFAULT_RECEIVE_BUFFER_SIZE 0x4000

//
// Represents a web socket io handle instance
//
typedef struct xio_wsclient_t
{
    pal_wsclient_t* websocket;
    prx_scheduler_t* scheduler;

    ON_BYTES_RECEIVED on_bytes_received;
    void* on_bytes_received_context;
    ON_IO_OPEN_COMPLETE on_io_open_complete;
    void* on_io_open_complete_context;
    ON_IO_ERROR on_io_error;
    void* on_io_error_context;
    ON_IO_CLOSE_COMPLETE on_io_close_complete;
    void* on_io_close_complete_context;
    IO_DOWORK dowork_cb;
    CONCRETE_IO_HANDLE dowork_ctx;

    bool recv_enabled;
    io_queue_t* inbound;     // Inbound buffer queue with received buffers
    io_queue_t* outbound;               // Outbound queue for send buffers
    int32_t last_error;  // Last error received in connect/disconnect path
    bool closed;
    bool destroy;
    log_t log;
}
xio_wsclient_t;


//
// Free xio context on scheduler thread when fully disconnected.
//
static void xio_wsclient_free(
    xio_wsclient_t* ws
)
{
    dbg_assert_ptr(ws);

    ws->destroy = true;
    if (!ws->closed)
        return; // Called again when closed..

    ws->on_io_close_complete = NULL;
    ws->on_io_error = NULL;

    if (ws->websocket)
    {
        pal_wsclient_close(ws->websocket);
        return; // Called again when destroyed..
    }

    if (ws->inbound)
        io_queue_free(ws->inbound);

    if (ws->outbound)
        io_queue_free(ws->outbound);

    if (ws->scheduler)
        prx_scheduler_release(ws->scheduler, ws);

    mem_free_type(xio_wsclient_t, ws);
}

//
// Scheduler task that delivers the result of the open operation
//
static void xio_wsclient_deliver_open_result(
    xio_wsclient_t* ws
)
{
    /**/ if (ws->on_io_open_complete)
    {
        ws->recv_enabled = true;
        ws->on_io_open_complete(ws->on_io_open_complete_context,
            ws->last_error != er_ok ? IO_OPEN_ERROR : IO_OPEN_OK);
    }
    else if (ws->on_io_error && ws->last_error != er_ok)
    {
        ws->on_io_error(ws->on_io_error_context);
    }
}

//
// Scheduler task that delivers all received buffers or errors on scheduler
//
static void xio_wsclient_deliver_inbound_results(
    xio_wsclient_t* ws
)
{
    io_queue_buffer_t* buffer;
    size_t size;
    const uint8_t* buf;

    dbg_assert_ptr(ws);
    if (ws->closed)
        return;

    while (true)
    {
        buffer = io_queue_pop_done(ws->inbound);
        if (!buffer)
            break;

        size = buffer->length;
        buf = (const uint8_t*)io_queue_buffer_to_ptr(buffer);

        /**/ if (ws->on_io_error && buffer->code != er_ok)
        {
            ws->on_io_error(
                ws->on_io_error_context);
        }
        else if (size > 0)
        {
            if (ws->on_bytes_received)
            {
                ws->on_bytes_received(
                    ws->on_bytes_received_context, buf, size);
            }

            if (ws->dowork_cb)
                ws->dowork_cb(ws->dowork_ctx);
        }

        io_queue_buffer_release(buffer);
    }
}

//
// Scheduler task that delivers all send results
//
static void xio_wsclient_deliver_outbound_results(
    xio_wsclient_t* ws
)
{
    io_queue_buffer_t* buffer;
    dbg_assert_ptr(ws);

    while (true)
    {
        buffer = io_queue_pop_done(ws->outbound);
        if (!buffer)
            break;

        if (buffer->cb_ptr)
        {
            ((ON_SEND_COMPLETE)buffer->cb_ptr)(buffer->ctx,
                buffer->code != er_ok ? IO_SEND_ERROR : IO_SEND_OK);
        }

        if (ws->dowork_cb)
            ws->dowork_cb(ws->dowork_ctx);

        io_queue_buffer_release(buffer);
    }
}

//
// Scheduler task that delivers close result
//
static void xio_wsclient_deliver_close_result(
    xio_wsclient_t* ws
)
{
    io_queue_buffer_t* buffer;

    xio_wsclient_deliver_inbound_results(ws);
    xio_wsclient_deliver_outbound_results(ws);

    // Roll back all in progress buffers
    io_queue_rollback(ws->outbound);

    while (true)
    {
        buffer = io_queue_pop_ready(ws->outbound);
        if (!buffer)
            break;
        if (buffer->cb_ptr)
            ((ON_SEND_COMPLETE)buffer->cb_ptr)(buffer->ctx, IO_SEND_CANCELLED);
        io_queue_buffer_release(buffer);
    }

    if (ws->on_io_error && ws->last_error != er_ok)
    {
        ws->on_io_error(ws->on_io_error_context);
        ws->on_io_error = NULL;
    }

    if (ws->on_io_close_complete)
    {
        ws->on_io_close_complete(ws->on_io_close_complete_context);
        ws->on_io_close_complete = NULL;
    }

    ws->closed = true;

    // Free xio if destroy was called as well...
    if (ws->destroy)
    {
        __do_next(ws, xio_wsclient_free);
    }
}

//
// Pal callback to begin receive operation
//
static void xio_wsclient_on_begin_receive(
    xio_wsclient_t* ws,
    uint8_t** buf,
    size_t* length
)
{
    int32_t result;
    io_queue_buffer_t* buffer = NULL;

    dbg_assert_ptr(ws);
    dbg_assert_ptr(buf);
    dbg_assert_ptr(length);

    result = io_queue_create_buffer(ws->inbound, NULL,
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
static void xio_wsclient_on_end_receive(
    xio_wsclient_t* ws,
    uint8_t** buf,
    size_t* length,
    pal_wsclient_buffer_type_t* type,
    int32_t result
)
{
    io_queue_buffer_t* buffer;

    dbg_assert_ptr(ws);
    dbg_assert_ptr(buf);
    dbg_assert_ptr(length);

    (void)type;  // Type of buffer is irrelevant to xio

    buffer = io_queue_buffer_from_ptr(*buf);
    dbg_assert_ptr(buffer);

    if (result != er_ok && result != er_aborted && result != er_retry)
    {
        log_error(ws->log, "Error during pal receive (%s).",
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
        buffer->length = *length;
        io_queue_buffer_set_done(buffer);
        __do_next(ws, xio_wsclient_deliver_inbound_results);
    }
}

//
// Pal callback to request a buffer to be send
//
static void xio_wsclient_on_begin_send(
    xio_wsclient_t* ws,
    uint8_t** buf,
    size_t* length,
    pal_wsclient_buffer_type_t* type
)
{
    io_queue_buffer_t* buffer = NULL;

    dbg_assert_ptr(ws);
    dbg_assert_ptr(buf);
    dbg_assert_ptr(length);
    dbg_assert_ptr(type);

    buffer = io_queue_pop_inprogress(ws->outbound);
    if (!buffer)
        buffer = io_queue_pop_ready(ws->outbound);
    if (!buffer)
    {
        *buf = NULL;
        *length = 0;
        *type = pal_wsclient_buffer_type_unknown;
    }
    else
    {
        *buf = io_queue_buffer_to_ptr(buffer);
        *length = buffer->write_offset;
        *type = pal_wsclient_buffer_type_binary_msg;
    }
}

//
// Pal callback to complete send operation
//
static void xio_wsclient_on_end_send(
    xio_wsclient_t* ws,
    uint8_t** buf,
    size_t* length,
    int32_t result
)
{
    io_queue_buffer_t* buffer;

    dbg_assert_ptr(ws);
    dbg_assert_ptr(length);
    dbg_assert_ptr(*buf);

    if (result != er_ok && result != er_aborted && result != er_retry)
    {
        log_error(ws->log, "Error during pal wsclient send (%s).",
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

    dbg_assert(buffer->length == *length || result != er_ok, "Not all sent");

    if (buffer->cb_ptr && !ws->closed)
    {
        // Mark done and deliver
        buffer->code = result;
        buffer->read_offset = *length;

        io_queue_buffer_set_done(buffer);
        __do_next(ws, xio_wsclient_deliver_outbound_results);
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
static void xio_wsclient_event_handler(
    void* context,
    pal_wsclient_event_t ev,
    uint8_t** buffer,
    size_t* size,
    pal_wsclient_buffer_type_t* type,
    int32_t error
)
{
    xio_wsclient_t* ws = (xio_wsclient_t*)context;

    switch (ev)
    {
    case pal_wsclient_event_connected:
        dbg_assert(!buffer && !size && !type, "no buffer expected.");
        ws->last_error = error;
        __do_next(ws, xio_wsclient_deliver_open_result);
        break;
    case pal_wsclient_event_begin_recv:
        dbg_assert(error == er_ok, "no error expected.");
        dbg_assert(!type, "no type expected.");
        xio_wsclient_on_begin_receive(ws, buffer, size);
        break;
    case pal_wsclient_event_end_recv:
        xio_wsclient_on_end_receive(ws, buffer, size, type, error);
        break;
    case pal_wsclient_event_begin_send:
        dbg_assert(error == er_ok, "no error expected.");
        xio_wsclient_on_begin_send(ws, buffer, size, type);
        break;
    case pal_wsclient_event_end_send:
        dbg_assert(!type, "no type expected.");
        xio_wsclient_on_end_send(ws, buffer, size, error);
        break;
    case pal_wsclient_event_disconnected:
        dbg_assert(!buffer && !size && !type, "no buffer expected.");
        ws->last_error = error;
        __do_next(ws, xio_wsclient_deliver_close_result);
        break;
    case pal_wsclient_event_closed:
        dbg_assert(error == er_ok, "no error expected.");
        dbg_assert(!buffer && !size && !type, "no buffer expected.");
        ws->websocket = NULL;
        __do_next(ws, xio_wsclient_free);
        break;
    default:
        dbg_assert(0, "Should not be here!");
    }
}

//
// Sends buffer by enqueing into the outbound queue
//
static int32_t xio_wsclient_send(
    CONCRETE_IO_HANDLE handle,
    const void* buf,
    size_t size,
    ON_SEND_COMPLETE on_send_complete,
    void* callback_context
)
{
    int32_t result;
    io_queue_buffer_t* buffer;
    xio_wsclient_t* ws = (xio_wsclient_t*)handle;

    chk_arg_fault_return(handle);
    chk_arg_fault_return(buf);
    chk_arg_fault_return(size);

    result = io_queue_create_buffer(ws->outbound, buf, size, &buffer);
    if (result != er_ok)
        return result;

    buffer->cb_ptr = (void(*)(void*, int32_t))on_send_complete;
    buffer->ctx = callback_context;

    io_queue_buffer_set_ready(buffer);

    pal_wsclient_can_send(ws->websocket, true);
    if (ws->recv_enabled)
    {
        pal_wsclient_can_recv(ws->websocket, true);
    }
    return er_ok;
}

//
// Enable or disable receive
//
static int32_t xio_wsclient_recv(
    CONCRETE_IO_HANDLE handle,
    bool enabled
)
{
    xio_wsclient_t* ws = (xio_wsclient_t*)handle;
    ws->recv_enabled = enabled;
    return pal_wsclient_can_recv(ws->websocket, enabled);
}

//
// Open websocket io
//
static int32_t xio_wsclient_open(
    CONCRETE_IO_HANDLE handle,
    ON_IO_OPEN_COMPLETE on_io_open_complete,
    void* on_io_open_complete_context,
    ON_BYTES_RECEIVED on_bytes_received,
    void* on_bytes_received_context,
    ON_IO_ERROR on_io_error,
    void* on_io_error_context
)
{
    int32_t result;
    xio_wsclient_t* ws = (xio_wsclient_t*)handle;
    chk_arg_fault_return(handle);

    if (!ws->scheduler)
    {
        result = prx_scheduler_create(NULL, &ws->scheduler);
        if (result != er_ok)
            return result;
    }

    ws->on_io_close_complete = NULL;

    ws->on_bytes_received = on_bytes_received;
    ws->on_bytes_received_context = on_bytes_received_context;
    ws->on_io_open_complete = on_io_open_complete;
    ws->on_io_open_complete_context = on_io_open_complete_context;
    ws->on_io_error = on_io_error;
    ws->on_io_error_context = on_io_error_context;

    ws->last_error = er_ok;

    pal_wsclient_can_recv(ws->websocket, true);
    return pal_wsclient_connect(ws->websocket);
}

//
// Close websocket io
//
static int32_t xio_wsclient_close(
    CONCRETE_IO_HANDLE handle,
    ON_IO_CLOSE_COMPLETE on_io_close_complete,
    void* on_io_close_complete_context
)
{
    xio_wsclient_t* ws = (xio_wsclient_t*)handle;
    chk_arg_fault_return(handle);

    ws->on_bytes_received = NULL;
    ws->on_io_open_complete = NULL;
    ws->on_io_error = NULL;
    ws->dowork_cb = NULL;

    ws->on_io_close_complete = on_io_close_complete;
    ws->on_io_close_complete_context = on_io_close_complete_context;
    ws->last_error = er_ok;

    return pal_wsclient_disconnect(ws->websocket);
}

//
// Destroy io handle
//
static void xio_wsclient_destroy(
    CONCRETE_IO_HANDLE handle
)
{
    xio_wsclient_t* ws = (xio_wsclient_t*)handle;
    if (!ws)
        return;
    if (!ws->scheduler)
    {
        xio_wsclient_free(ws);
        return;
    }

    // Stop any queued actions to ensure controlled free
    prx_scheduler_clear(ws->scheduler, NULL, ws);

    // Detach any callbacks from buffers
    io_queue_abort(ws->outbound);

    // Kick off free
    __do_next(ws, xio_wsclient_free);

    // Controlled deliver close
    __do_next(ws, xio_wsclient_deliver_close_result);
}

//
// Create io handle
//
static CONCRETE_IO_HANDLE xio_wsclient_create(
    void* io_create_parameters
)
{
    int32_t result;
    WSIO_CONFIG* ws_io_config = (WSIO_CONFIG*)io_create_parameters;
    xio_wsclient_t* ws;

    if (!ws_io_config || !ws_io_config->hostname ||
        !ws_io_config->protocol || !ws_io_config->resource_name)
        return NULL;

    ws = mem_zalloc_type(xio_wsclient_t);
    if (!ws)
        return NULL;
    do
    {
        ws->log = log_get("xio_ws");

        result = io_queue_create("WS-inbound", &ws->inbound);
        if (result != er_ok)
            break;
        result = io_queue_create("WS-outbound", &ws->outbound);
        if (result != er_ok)
            break;

        result = pal_wsclient_create(ws_io_config->protocol,
            ws_io_config->hostname, (uint16_t)ws_io_config->port,
            ws_io_config->resource_name, true,
            xio_wsclient_event_handler, ws, &ws->websocket);
        if (result != er_ok)
            break;

        return ws;
    }
    while (0);
    xio_wsclient_destroy(ws);
    return NULL;
}

//
// No op
//
static void xio_wsclient_dowork(
    CONCRETE_IO_HANDLE handle
)
{
    (void)handle;
    return;
}

//
// No op
//
static OPTIONHANDLER_HANDLE xio_wsclient_retrieve_option(
    CONCRETE_IO_HANDLE handle
)
{
    (void)handle;
    return NULL;
}

//
// Set option - attaches the scheduler to the io handle
//
static int32_t xio_wsclient_setoption(
    CONCRETE_IO_HANDLE handle,
    const char* option_name,
    const void* buffer
)
{
    xio_wsclient_t* ws = (xio_wsclient_t*)handle;
    chk_arg_fault_return(handle);

    /**/ if (0 == string_compare(option_name, xio_opt_scheduler))
        return prx_scheduler_create((prx_scheduler_t*)buffer, &ws->scheduler);
    else if (0 == string_compare(option_name, xio_opt_flow_ctrl))
        return xio_wsclient_recv(handle, *((uint32_t*)buffer) != 0);
    else if (0 == string_compare(option_name, xio_opt_dowork_cb))
        ws->dowork_cb = (IO_DOWORK)buffer;
    else if (0 == string_compare(option_name, xio_opt_dowork_ctx))
        ws->dowork_ctx = (CONCRETE_IO_HANDLE)buffer;
    else
        return er_not_supported;

    return er_ok;
}

//
// Interface description
//
static const IO_INTERFACE_DESCRIPTION xio_wsclient_io_interface_description =
{
    xio_wsclient_retrieve_option,
    xio_wsclient_create,
    xio_wsclient_destroy,
    xio_wsclient_open,
    xio_wsclient_close,
    xio_wsclient_send,
    xio_wsclient_dowork,
    xio_wsclient_setoption
};

//
// Io interface for win32 websockets
//
const IO_INTERFACE_DESCRIPTION* wsio_get_interface_description(void)
{
    return &xio_wsclient_io_interface_description;
}
