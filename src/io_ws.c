// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"

#include "io_ws.h"
#include "io_queue.h"
#include "pal_ws.h"
#include "pal_mt.h"
#include "pal_time.h"
#include "prx_config.h"
#include "util_string.h"

//
// Receive and send in up to 4k frames
//
#define DEFAULT_FRAME_SIZE 0x1000  

//
// Status of connection
//
typedef enum io_ws_connection_status
{
    io_ws_connection_status_connected,
    io_ws_connection_status_disconnected,
    io_ws_connection_status_closing,
    io_ws_connection_status_closed
}
io_ws_connection_status_t;

//
// Struct for inbound and outbound streams
//
typedef struct io_ws_stream
{
    io_stream_t itf;                                   // Stream interface
    io_queue_t* queue;                      // Queue of buffers for stream
    io_queue_buffer_t* current;      // Buffer to write to or read from...
    size_t readable;                // Amount of data readable from stream
}
io_ws_stream_t;

//
// Represents a websocket based, scheduler bound connection
//
struct io_ws_connection
{
    io_url_t* address;
    STRING_HANDLE user_header_key;
    STRING_HANDLE pwd_header_key;
    ticks_t expiry;    // When the credential used to authenticate expires
    pal_wsclient_t* wsclient;               // Pal Websocket client handle
    prx_scheduler_t* scheduler;
    io_ws_stream_t outbound;                                // Send stream
    io_ws_stream_t inbound;                              // Receive stream
    io_stream_handler_t receiver_cb;
    void* receiver_ctx;
    ticks_t last_activity;
    ticks_t last_success;                     // Last successful connected
    int32_t last_error;
    io_ws_connection_reconnect_t reconnect_cb;   // Call when disconnected
    void* reconnect_ctx;
    int32_t back_off_in_seconds;       // Delay until next connect attempt
    io_ws_connection_status_t status;
    log_t log;
};


//
// Scheduler task to disconnect econnection
//
static void io_ws_connection_disconnect(
    io_ws_connection_t* connection
)
{
    dbg_assert_is_task(connection->scheduler);

    // Move fragments not delivered back to ready so they are sent again
    io_queue_rollback(connection->outbound.queue);

    if (connection->status == io_ws_connection_status_connected)
    {
        pal_wsclient_can_send(connection->wsclient, false);
        pal_wsclient_can_recv(connection->wsclient, false);

        // Wait for disconnect to complete, then reconnect
        pal_wsclient_disconnect(connection->wsclient);
    }
}

//
// Reads from connection
//
static int32_t io_ws_connection_stream_reader(
    void *context,
    void *data,
    size_t count,
    size_t* read
)
{
    int32_t result;
    io_ws_connection_t* connection = (io_ws_connection_t*)context;
    char* buf = (char*)data;
    size_t was_read;

    dbg_assert_ptr(read);
    dbg_assert_ptr(data);
    dbg_assert_ptr(connection);

    result = er_ok;
    *read = 0;
    while (count > 0)
    {
        if (!connection->inbound.current)
        {
            // Replenish from in progress list if null
            connection->inbound.current = io_queue_pop_inprogress(
                connection->inbound.queue);
            if (!connection->inbound.current)
            {
                // End of stream...
                result = er_reading;
                break;
            }
        }

        // Read from frame...
        result = io_stream_read(
            io_queue_buffer_as_stream(connection->inbound.current), 
            &buf[*read], count, &was_read);
        if (result != er_ok)
        {
            break;
        }

        if (connection->inbound.current->length ==
                connection->inbound.current->read_offset)
        {
            // If fully read, release...
            io_queue_buffer_release(connection->inbound.current);
            connection->inbound.current = NULL;
        }

        connection->inbound.readable -= was_read;
        count -= was_read;
        *read += was_read;
    }

    if (result != er_ok)
    {
        log_error(connection->log, "Failed to read from frame stream");
    }

    return result;
}

//
// Available in connection to read
//
static size_t io_ws_connection_stream_readable(
    void *context
)
{
    dbg_assert_ptr(context);
    return ((io_ws_connection_t*)context)->inbound.readable;
}

//
// Resets stream
//
static int32_t io_ws_connection_stream_reset(
    void *context
)
{
    io_ws_connection_t* connection = (io_ws_connection_t*)context;
    io_queue_buffer_t* buffer;
    while (true)
    {
        buffer = io_queue_pop_ready(connection->inbound.queue);
        if (!buffer)
            break;
        io_queue_buffer_release(buffer);
    }
  
    buffer = connection->outbound.current;
    connection->outbound.current = NULL;
    if (buffer)
        io_queue_buffer_release(buffer);

    return er_ok;
}

//
// Writes to connection
//
static int32_t io_ws_connection_stream_writer(
    void *context,
    const void *data,
    size_t count
)
{
    int32_t result;
    io_ws_connection_t* connection = (io_ws_connection_t*)context;
    size_t written = 0, avail = 0, to_write;
    char* buf = (char*)data;

    dbg_assert_ptr(data);
    dbg_assert_ptr(connection);

    result = er_ok;

    // Writes as much as fits into a frame
    while (count > 0)
    {
        if (connection->outbound.current)
        {
            avail = connection->outbound.current->length - 
                connection->outbound.current->write_offset;
            if (!avail)
            {
                // Current buffer full, mark as fragment and add to ready list
                connection->outbound.current->flags = 
                    pal_wsclient_buffer_type_binary_fragment;
                io_queue_buffer_set_ready(connection->outbound.current);
                connection->outbound.current = NULL;
            }
        }

        if (!connection->outbound.current)
        {
            // Create new buffer to write to
            result = io_queue_create_buffer(connection->outbound.queue, 
                NULL, DEFAULT_FRAME_SIZE, &connection->outbound.current);
            if (result != er_ok)
            {
                written = 0;
                break;
            }
            avail = DEFAULT_FRAME_SIZE;
        }

        to_write = count > avail ? avail : count;
        result = io_stream_write(
            io_queue_buffer_as_stream(connection->outbound.current), buf, to_write);
        if (result != er_ok)
            break;

        count -= to_write;
        buf += to_write;
        written += to_write;
    }

    if (result != er_ok)
    {
        log_error(connection->log, "Failed to write to frame stream");
    }
    return result;
}

//
// Available in connection to write
//
static size_t io_ws_connection_stream_writeable(
    void *context
)
{
    (void)context;
    return (size_t)-1;  // Assume infinite amount of buffers
}

//
// Free websocket client instance, called when closing and disconnect received
//
static void io_ws_connection_free(
    io_ws_connection_t* connection
)
{
    bool connected;
    dbg_assert_ptr(connection);
    dbg_assert_is_task(connection->scheduler);

    connected = connection->status == io_ws_connection_status_connected;
    connection->status = io_ws_connection_status_closing;

    if (connection->scheduler)
        prx_scheduler_clear(connection->scheduler, NULL, connection);

    if (connected)
    {
        pal_wsclient_can_send(connection->wsclient, false);
        pal_wsclient_can_recv(connection->wsclient, false);

        pal_wsclient_disconnect(connection->wsclient);

        // Wait for disconnect to complete
        log_trace(connection->log,
            "connection %p closing... waiting for disconnect!", connection);
        return;
    }

    if (connection->wsclient)
    {
        pal_wsclient_close(connection->wsclient);

        // Wait for close to complete
        log_trace(connection->log,
            "connection %p closing... waiting for close!", connection);
        return;
    }

    connection->status = io_ws_connection_status_closed;
    
    if (connection->scheduler)
        prx_scheduler_release(connection->scheduler, connection);

    if (connection->inbound.current)
        io_queue_buffer_release(connection->inbound.current);
    if (connection->inbound.queue)
        io_queue_free(connection->inbound.queue);

    if (connection->outbound.current)
        io_queue_buffer_release(connection->outbound.current);
    if (connection->outbound.queue)
        io_queue_free(connection->outbound.queue);

    if (connection->address)
        io_url_free(connection->address);
    if (connection->user_header_key)
        STRING_delete(connection->user_header_key);
    if (connection->pwd_header_key)
        STRING_delete(connection->pwd_header_key);
    
    log_trace(connection->log, "connection %p freed!", connection);
    mem_free_type(io_ws_connection_t, connection);
}

//
// Clear back_off timer 
//
static void io_ws_connection_clear_failures(
    io_ws_connection_t* connection
)
{
    if (connection->back_off_in_seconds)
    {
        log_trace(connection->log, "Clearing failures on connection %p...",
            connection);
    }
    connection->last_error = er_ok;
    connection->last_success = connection->last_activity = ticks_get();
    connection->back_off_in_seconds = 0;
}

//
// Scheduler task that creates and connects the connection
//
static void io_ws_connection_reconnect(
    io_ws_connection_t* connection
);

//
// Scheduler task that connects all unconnected layers
//
static void io_ws_connection_reset(
    io_ws_connection_t* connection
)
{
    dbg_assert_ptr(connection);
    dbg_assert_is_task(connection->scheduler);

    // Clear all connection tasks
    prx_scheduler_clear(connection->scheduler, NULL, connection);

    if (connection->reconnect_cb && !connection->reconnect_cb(
        connection->reconnect_ctx, connection->last_error))
        return;

    log_info(connection->log, "Reconnecting in %d seconds...",
        connection->back_off_in_seconds);

    __do_later(connection, io_ws_connection_reconnect,
        connection->back_off_in_seconds * 1000);

    if (!connection->back_off_in_seconds)
        connection->back_off_in_seconds = 1;
    connection->back_off_in_seconds *= 2;
    if (connection->back_off_in_seconds > 1 * 60 * 60)
        connection->back_off_in_seconds = 1 * 60 * 60;

    connection->status = io_ws_connection_status_disconnected;
}

//
// Scheduler task that delivers all received buffers or errors on scheduler
//
static void io_ws_connection_deliver_stream(
    io_ws_connection_t* connection
)
{
    int32_t result;
    io_queue_buffer_t* buffer;
    dbg_assert_ptr(connection);
    dbg_assert_is_task(connection->scheduler);

    connection->inbound.readable = 0;
    while (true)
    {
        buffer = io_queue_pop_ready(connection->inbound.queue);
        if (!buffer)
        {
            dbg_assert(connection->status != 
                io_ws_connection_status_connected,
                "Must not be connected here");
            log_info(connection->log, 
                "No buffer while streaming (state: %d)...", 
                connection->status);
            return;
        }

        connection->inbound.readable += buffer->length;
        io_queue_buffer_set_inprogress(buffer);

        if (buffer->flags != pal_wsclient_buffer_type_binary_msg)
        {
            continue;
        }

        // Hit end of stream, notify receiver of new stream
        result = connection->receiver_cb(connection->receiver_ctx,
            &connection->inbound.itf);

        if (connection->inbound.current)
            io_queue_buffer_release(connection->inbound.current);

        connection->inbound.current = NULL;
        connection->inbound.readable = 0;
        if (result != er_ok)
            break;

        io_ws_connection_clear_failures(connection);
        return;  // Success!
    }

    // If error, disconnect
    log_error(connection->log,
        "Receiver indicated error %s!  Disconnecting...",
        prx_err_string(result));

    connection->last_error = result;
    __do_next(connection, io_ws_connection_disconnect);
}

//
// Scheduler task that delivers all send results 
//
static void io_ws_connection_deliver_send_results(
    io_ws_connection_t* connection
)
{
    int32_t result;
    io_queue_buffer_t* buffer;
    io_ws_connection_send_complete_t cb;
    void* ctx;

    dbg_assert_ptr(connection);

    while (true)
    {
        buffer = io_queue_pop_done(connection->outbound.queue);
        if (!buffer)
            break;

        cb = (io_ws_connection_send_complete_t)buffer->cb_ptr;
        buffer->cb_ptr = NULL;
        ctx = buffer->ctx;
        buffer->ctx = NULL;
        result = buffer->code;
        buffer->code = er_ok;

        io_queue_buffer_release(buffer);

      //  if (result == er_ok)
      //      io_ws_connection_clear_failures(connection);

        // Inform client of sent completion
        if (cb) cb(ctx, result);
    }
}

//
// Pal callback task when disconnected
//
static void io_ws_connection_on_disconnected(
    io_ws_connection_t* connection
)
{
    dbg_assert_is_task(connection->scheduler);
    /**/ if (connection->status == io_ws_connection_status_closing)
    {
        if (connection->wsclient)
        {
            // Continue freeing connection
            log_debug(connection->log,
                "connection %p disconnected... continue to close!",
                connection);
            __do_next(connection, io_ws_connection_free);
        }
    }
    else if (connection->status == io_ws_connection_status_connected)
    {
        log_debug(connection->log, "connection %p disconnected... reset!", 
            connection);
        connection->status = io_ws_connection_status_disconnected;
        connection->last_error = er_closed;
        __do_next(connection, io_ws_connection_reset);
    }
    else if (connection->status != io_ws_connection_status_disconnected)
    {
        dbg_assert(0, "bad state %d for connection %p", 
            connection->status, connection);
    }
}

//
// Pal callback when connected
//
static void io_ws_connection_on_connected(
    io_ws_connection_t* connection
)
{
    connection->status = io_ws_connection_status_connected;
    connection->last_activity = ticks_get();
    // Start sending and receiving 
    pal_wsclient_can_send(connection->wsclient, true);
    pal_wsclient_can_recv(connection->wsclient, true);
}

//
// Pal callback to begin receive operation
//
static void io_ws_connection_on_begin_receive(
    io_ws_connection_t* connection,
    uint8_t** buf,
    size_t* length
)
{
    int32_t result;
    io_queue_buffer_t* buffer = NULL;

    dbg_assert_ptr(connection);
    dbg_assert_ptr(buf);
    dbg_assert_ptr(length);

    result = io_queue_create_buffer(
        connection->inbound.queue, NULL, DEFAULT_FRAME_SIZE, &buffer);
    if (result != er_ok)
    {
        *buf = NULL;
        *length = 0;
    }
    else
    {
        *buf = io_queue_buffer_to_ptr(buffer);
        *length = DEFAULT_FRAME_SIZE;
    }
}

//
// Pal callback to complete receiving operation
//
static void io_ws_connection_on_end_receive(
    io_ws_connection_t* connection,
    uint8_t** buf,
    size_t* length,
    pal_wsclient_buffer_type_t* type,
    int32_t result
)
{
    io_queue_buffer_t* buffer;

    dbg_assert_ptr(connection);
    dbg_assert_ptr(buf);
    dbg_assert_ptr(length);

    buffer = io_queue_buffer_from_ptr(*buf);
    if (!buffer)
        return;

    if (result != er_ok)
    {
        io_queue_buffer_release(buffer);

        if (result != er_aborted)
        {
            log_error(connection->log, "Error during end receive (%s).",
                prx_err_string(result));

            // Disconnect and reset
            connection->last_error = result;
            __do_next(connection, io_ws_connection_disconnect);
        }
    }
    else
    {
        dbg_assert_ptr(type);

        buffer->flags = *type;
        buffer->length = *length;
        io_queue_buffer_set_ready(buffer); // Ready to deliver

        if (buffer->flags == pal_wsclient_buffer_type_binary_msg)
        {
            // Hit last fragment of message, notify client of stream...
            __do_next(connection, io_ws_connection_deliver_stream);
        }
    }
}

//
// Pal callback to request a buffer to be send
//
static void io_ws_connection_on_begin_send(
    io_ws_connection_t* connection,
    uint8_t** buf,
    size_t* length,
    pal_wsclient_buffer_type_t* type
)
{
    io_queue_buffer_t* buffer = NULL;

    dbg_assert_ptr(connection);
    dbg_assert_ptr(buf);
    dbg_assert_ptr(length);
    dbg_assert_ptr(type);

    buffer = io_queue_pop_ready(connection->outbound.queue);
    if (!buffer)
    {
        // Nothing to send...
        *buf = NULL;
        *length = 0;
        *type = pal_wsclient_buffer_type_unknown;
    }
    else
    {
        *buf = io_queue_buffer_to_ptr(buffer);
        *length = buffer->write_offset;
        *type = (pal_wsclient_buffer_type_t)buffer->flags;
    }
}

//
// Pal callback to complete send operation
//
static void io_ws_connection_on_end_send(
    io_ws_connection_t* connection,
    uint8_t** buf,
    size_t* length,
    int32_t result
)
{
    io_queue_buffer_t* buffer;
    dbg_assert_ptr(connection);
    dbg_assert_ptr(length);

    (void)length;
    buffer = io_queue_buffer_from_ptr(*buf);
    if (!buffer)
        return;

    buffer->code = result;
    io_queue_buffer_set_inprogress(buffer);

    if (result == er_ok && 
        buffer->flags == pal_wsclient_buffer_type_binary_msg)
    {
        // If last buffer of message, then set all so far to done and deliver result
        while (true)
        {
            buffer = io_queue_pop_inprogress(connection->outbound.queue);
            if (!buffer)
                break;
            io_queue_buffer_set_done(buffer);
        }
        __do_next(connection, io_ws_connection_deliver_send_results);
    }

    else if (result != er_ok && result != er_aborted)
    {
        log_error(connection->log, "Error during end send (%s).",
            prx_err_string(result));

        // Disconnect and reset
        connection->last_error = result;
        __do_next(connection, io_ws_connection_disconnect);
    }
}

//
// Pal event callback 
//
static void io_ws_connection_event_handler(
    void* context,
    pal_wsclient_event_t ev,
    uint8_t** buffer,
    size_t* size,
    pal_wsclient_buffer_type_t* type,
    int32_t error
)
{
    io_ws_connection_t* connection = (io_ws_connection_t*)context;
    switch (ev)
    {
    case pal_wsclient_event_connected:
        dbg_assert(!buffer && !size && !type, "no buffer expected.");
        if (error == er_ok)
        {
            __do_next(connection, io_ws_connection_on_connected);
            break;
        }
        log_error(connection->log, "... failed to connect. (%s) (ws-conn:%p)",
            prx_err_string(error), connection);
        connection->status = io_ws_connection_status_connected;
        // Fall through
    case pal_wsclient_event_disconnected:
        dbg_assert(!buffer && !size && !type, "no buffer expected.");
        log_trace(connection->log, "... disconnected. (%s) (ws-conn:%p)",
            prx_err_string(error), connection);
        connection->last_error = error;
          __do_next(connection, io_ws_connection_on_disconnected);
        break;
    case pal_wsclient_event_closed:
        dbg_assert(error == er_ok, "no error expected.");
        dbg_assert(!buffer && !size && !type, "no buffer expected.");
        dbg_assert_ptr(connection->wsclient);
        connection->wsclient = NULL;
        log_trace(connection->log, "... destroyed. (ws-conn:%p)",
            connection);
        __do_next(connection, io_ws_connection_free);
        break;
    case pal_wsclient_event_begin_recv:
        dbg_assert(error == er_ok, "no error expected.");
        dbg_assert(!type, "no type expected.");
        io_ws_connection_on_begin_receive(connection, buffer, size);
        break;
    case pal_wsclient_event_end_recv:
        io_ws_connection_on_end_receive(connection, buffer, size, type, error);
        break;
    case pal_wsclient_event_begin_send:
        dbg_assert(error == er_ok, "no error expected.");
        io_ws_connection_on_begin_send(connection, buffer, size, type);
        break;
    case pal_wsclient_event_end_send:
        dbg_assert(!type, "no type expected.");
        io_ws_connection_on_end_send(connection, buffer, size, error);
        break;
    default:
        dbg_assert(0, "Should not be here!");
    }
}

//
// Renew token once token expired
//
static void io_ws_connection_renew_token(
    io_ws_connection_t* connection
)
{
    ticks_t now;
    uint32_t ttl;

    dbg_assert_is_task(connection->scheduler);
    dbg_assert(connection->expiry != 0, "Not expiring, yet should reset.");

    // If connection has expired, reset connection
    now = ticks_get();
    if (connection->expiry < now)
    {
        log_info(connection->log, "Need to refresh credentials, disconnect...");
        connection->last_error = er_ok;
        __do_next(connection, io_ws_connection_disconnect);
    }
    else
    {
        ttl = (uint32_t)(connection->expiry - now);
        __do_later(connection, io_ws_connection_renew_token, ttl);
    }
}

//
// Scheduler task that creates and connects the connection
//
static void io_ws_connection_reconnect(
    io_ws_connection_t* connection
)
{
    int32_t result;
    STRING_HANDLE token = NULL;
    const char* username = NULL, *password = NULL;
    int64_t ttl = 0;
    bool secure;

    dbg_assert_ptr(connection);
    dbg_assert_is_task(connection->scheduler);
    do
    {
        if (connection->wsclient)
        {
            log_debug(connection->log, "Connecting existing client %p (ws-conn:%p)",
                connection->wsclient, connection);
            result = er_ok;
            break;
        }

        if (__prx_config_get_int(prx_config_key_connect_flag, 0) & 0x2)
            secure = 0 != STRING_compare_c_str_nocase(connection->address->scheme, "ws");
        else
            secure = true;
        result = pal_wsclient_create("proxy", STRING_c_str(connection->address->host_name), 
            connection->address->port, STRING_c_str(connection->address->path), secure,
            io_ws_connection_event_handler, connection, &connection->wsclient);
        if (result != er_ok)
            break;

        // Add authorization 
        if (connection->address->token_provider)
        {
            result = io_token_provider_new_token(connection->address->token_provider,
                &token, &ttl);
            if (result != er_ok)
            {
                log_error(connection->log, "Failed to make token for connection (%s)!",
                    prx_err_string(result));

                // TODO: Notify transport since this might not be recoverable 
                // in case of one time token
                break;
            }
            connection->expiry = ttl ? ticks_get() + ttl : 0;
            username = io_token_provider_get_property(
                connection->address->token_provider, io_token_property_policy);
            password = STRING_c_str(token);
        }

        if (connection->address->user_name)
            username = STRING_c_str(connection->address->user_name);
        if (connection->address->password)
            password = STRING_c_str(connection->address->password);

        if (username)
        {
            if (!connection->user_header_key)
            {
                log_error(connection->log, "Missing user header key for user name!");
                result = er_arg;
                break;
            }
            result = pal_wsclient_add_header(connection->wsclient,
                STRING_c_str(connection->user_header_key), username);
            if (result != er_ok)
                break;
        }

        if (password)
        {
            if (!connection->pwd_header_key)
            {
                log_error(connection->log, "Missing password header key for password!");
                result = er_arg;
                break;
            }
            result = pal_wsclient_add_header(connection->wsclient,
                STRING_c_str(connection->pwd_header_key), password);
            if (result != er_ok)
                break;
        }
    } 
    while (0);

    if (token)
        STRING_delete(token);

    // Connect client
    do
    {
        if (result != er_ok)
            break;
 
        result = pal_wsclient_connect(connection->wsclient);
        if (result != er_ok)
            break;

        pal_wsclient_can_recv(connection->wsclient, true);
        connection->last_activity = ticks_get();

        if (ttl > 0)
        {
            // Wait until we expire
            __do_later(connection, io_ws_connection_renew_token, (uint32_t)ttl);
        }
        return;
    } 
    while (0);

    // Attempt to reconnect
    connection->last_error = result;
    __do_next(connection, io_ws_connection_reset);
}

//
// Create websocket connection
//
int32_t io_ws_connection_create(
    io_url_t* address,
    const char* user_header_key,
    const char* pwd_header_key,
    prx_scheduler_t* scheduler,
    io_stream_handler_t receiver_cb,
    void* receiver_ctx,
    io_ws_connection_t** created
)
{
    int32_t result;
    io_ws_connection_t* connection;

    chk_arg_fault_return(address);
    chk_arg_fault_return(created);

    connection = mem_zalloc_type(io_ws_connection_t);
    if (!connection)
        return er_out_of_memory;
    do
    {
        connection->log = log_get("io_ws");
        connection->receiver_cb = receiver_cb;
        connection->receiver_ctx = receiver_ctx;
        connection->status = io_ws_connection_status_disconnected;
        
        result = io_url_clone(address, &connection->address);
        if (result != er_ok)
            break;

        if (user_header_key)
            connection->user_header_key = STRING_construct(user_header_key);
        if (pwd_header_key)
            connection->pwd_header_key = STRING_construct(pwd_header_key);
        if ((user_header_key && !connection->user_header_key) ||
            (pwd_header_key && !connection->pwd_header_key))
        {
            result = er_out_of_memory;
            break;
        }

        result = prx_scheduler_create(scheduler, &connection->scheduler);
        if (result != er_ok)
            break;

        result = io_queue_create(
            "WS-inbound", &connection->inbound.queue);
        if (result != er_ok)
            break;
        connection->inbound.itf.context =
            connection;
        connection->inbound.itf.reader =
            io_ws_connection_stream_reader;
        connection->inbound.itf.readable =
            io_ws_connection_stream_readable;
        connection->inbound.itf.reset =
            io_ws_connection_stream_reset;

        result = io_queue_create(
            "WS-outbound", &connection->outbound.queue);
        if (result != er_ok)
            break;
        connection->outbound.itf.context =
            connection;
        connection->outbound.itf.writer =
            io_ws_connection_stream_writer;
        connection->outbound.itf.writeable =
            io_ws_connection_stream_writeable;

        *created = connection;
        return er_ok;
    } while (0);

    io_ws_connection_free(connection);
    return result;
}

//
// Open connection -- must be called from scheduler
//
int32_t io_ws_connection_connect(
    io_ws_connection_t* connection,
    io_ws_connection_reconnect_t reconnect_cb,
    void* reconnect_ctx
)
{
    chk_arg_fault_return(connection);
    dbg_assert_is_task(connection->scheduler);

    connection->reconnect_cb = reconnect_cb;
    connection->reconnect_ctx = reconnect_ctx;

    __do_next(connection, io_ws_connection_reconnect);
    return er_ok;
}

//
// Get a stream to send one full message with -- must be called from scheduler
//
int32_t io_ws_connection_send(
    io_ws_connection_t* connection,
    io_stream_handler_t sender_cb,
    void* sender_ctx,
    io_ws_connection_send_complete_t complete_cb,
    void* complete_ctx

)
{
    int32_t result;
    io_queue_buffer_t* buffer;
    chk_arg_fault_return(connection);
    chk_arg_fault_return(sender_cb);
    dbg_assert_is_task(connection->scheduler);

    // Let sender write to the outbound frame queue
    result = sender_cb(sender_ctx, &connection->outbound.itf);
    // Flush all written frames...
    if (result != er_ok || !connection->outbound.current) // Nothing to flush...
    {
        if (complete_cb)
            complete_cb(complete_ctx, result);
        return result; 
    }

    // Flush current message into ready and mark as end of stream
    buffer = connection->outbound.current;
    connection->outbound.current = NULL;
    buffer->flags = pal_wsclient_buffer_type_binary_msg;
    buffer->cb_ptr = complete_cb;
    buffer->ctx = complete_ctx;
    io_queue_buffer_set_ready(buffer);

    if (!connection->wsclient)
        return er_ok; // Wait for connect, which will send/receive...

    // Restart sending and receiving...
    result = pal_wsclient_can_send(connection->wsclient, true);
    if (result == er_ok)
        pal_wsclient_can_recv(connection->wsclient, true);
    return result;
}

//
// Close connection -- must be called from scheduler
//
int32_t io_ws_connection_close(
    io_ws_connection_t* connection
)
{
    dbg_assert_is_task(connection->scheduler);
    chk_arg_fault_return(connection);

    dbg_assert(connection->status != io_ws_connection_status_closing && 
        connection->status != io_ws_connection_status_closed,
        "Double close of %p ...", connection); 
    connection->reconnect_cb = NULL;

    // Abort all outbound buffers
    if (connection->outbound.queue)
        io_queue_abort(connection->outbound.queue);

    io_ws_connection_free(connection);
    return er_ok;
}
