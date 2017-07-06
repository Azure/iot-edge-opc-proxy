// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "io_transport.h"
#include "io_mqtt.h"
#include "io_ws.h"
#include "prx_buffer.h"
#include "prx_config.h"
#include "prx_log.h"
#include "prx_sched.h"
#include "prx_ns.h"
#include "util_misc.h"
#include "util_stream.h"
#include "util_string.h"

//
// Client type and version
//
#define API_VERSION "2016-11-14"

//
// Connection base
//
typedef struct io_iot_hub_connection
{
    io_connection_t funcs;            // Must be first to cast if needed
    io_connection_cb_t handler_cb;       // connection event handler ...
    io_codec_id_t codec_id;
    void* handler_cb_ctx;                             // ... and context
    io_message_factory_t* message_pool;               // Message factory
}
io_iot_hub_connection_t;

//
// Azure IoT Hub device to cloud connection based on method endpoints
//
typedef struct io_iot_hub_umqtt_connection
{
    io_iot_hub_connection_t base;           // Common connection members
    io_mqtt_connection_t* mqtt_connection; // Underlying mqtt connection
    STRING_HANDLE event_uri;             // Preallocated event topic uri
#define LOG_PUBLISH_INTERVAL    (2 * 1000)
    io_stream_t* log_stream;         // to publish log stream content to
    prx_scheduler_t* scheduler;  // plus Scheduler to pump log telemetry
    io_mqtt_subscription_t* subscription;   // Receiver subscription ...
    prx_buffer_factory_t* buffer_pool;       // plus dynamic buffer pool
#define ALIVE_PUBLISH_INTERVAL (10 * 1000)
    bool alive;                       // Whether the connection is alive
    log_t log;
}
io_iot_hub_umqtt_connection_t;

//
// Azure IoT Hub stream connection based on raw websockets
//
typedef struct io_iot_hub_ws_connection
{
    io_iot_hub_connection_t base;           // Connection base structure
    io_ws_connection_t* ws_connection;          // Underlying connection
    io_message_t* message;          // Current message sent in scheduler
    log_t log;
}
io_iot_hub_ws_connection_t;

//
// Deinit connection base
//
static void io_iot_hub_connection_base_deinit(
    io_iot_hub_connection_t* connection
)
{
    dbg_assert_ptr(connection);
    if (connection->message_pool)
        io_message_factory_free(connection->message_pool);
}

//
// Reconnect handler
//
static bool io_iot_hub_connection_base_reconnect_handler(
    void* context,
    int32_t last_error
)
{
    int32_t result;
    io_iot_hub_connection_t* connection;

    connection = (io_iot_hub_connection_t*)context;
    if (!connection->handler_cb)
        return false;
    result = connection->handler_cb(connection->handler_cb_ctx, 
        io_connection_reconnecting, NULL, last_error);
    return result == er_ok;
}

//
// Initialize connection base
//
static int32_t io_iot_hub_connection_base_init(
    io_iot_hub_connection_t* connection,
    io_codec_id_t codec_id,
    io_connection_cb_t handler_cb,
    void* handler_cb_ctx
)
{
    dbg_assert_ptr(connection);

    connection->handler_cb = handler_cb;
    connection->handler_cb_ctx = handler_cb_ctx;
    connection->codec_id = codec_id;

    return io_message_factory_create(
        100, 0, 0, NULL, connection, &connection->message_pool);
}

//
// Called when the connection interface is freed
//
static void io_iot_hub_umqtt_connection_on_free(
    io_iot_hub_umqtt_connection_t* connection
)
{
    dbg_assert_ptr(connection);
    dbg_assert(!connection->mqtt_connection && !connection->subscription &&
        !connection->scheduler, "Should be closed");

    if (connection->log_stream)
        io_stream_close(connection->log_stream);
    if (connection->event_uri)
        STRING_delete(connection->event_uri);
    if (connection->buffer_pool)
        prx_buffer_factory_free(connection->buffer_pool);

    io_iot_hub_connection_base_deinit(&connection->base);
    log_info(connection->log, "mqtt transport connection closed.");
    mem_free_type(io_iot_hub_umqtt_connection_t, connection);
}

//
// Called when the connection interface closes the connection
//
static void io_iot_hub_umqtt_connection_on_close(
    io_iot_hub_umqtt_connection_t* connection
)
{
    dbg_assert_ptr(connection);
    dbg_assert_is_task(connection->scheduler);

    connection->alive = false;

    if (connection->subscription)
        io_mqtt_subscription_release(connection->subscription);
    connection->subscription = NULL;
    if (connection->mqtt_connection)
        io_mqtt_connection_close(connection->mqtt_connection);
    connection->mqtt_connection = NULL;

    if (connection->scheduler)
        prx_scheduler_release(connection->scheduler, connection);
    connection->scheduler = NULL;

    // TODO: Decouple, subscribe and wait for close to complete...
    // For now, close returns all messages, and then closes on the scheduler thread...
    // Same with retries and errors...

    (void)connection->base.handler_cb(connection->base.handler_cb_ctx, 
        io_connection_closed, NULL, er_ok);
    connection->base.handler_cb = NULL;
}

//
// Release message
//
static void io_iot_hub_umqtt_connection_on_send_complete(
    void* context,
    int32_t result
)
{
    dbg_assert_ptr(context);
    (void)result;
    io_message_release((io_message_t*)context);
}

//
// Push log stream content periodically
//
static void io_iot_hub_umqtt_connection_send_log_telemetry(
    io_iot_hub_umqtt_connection_t* connection
)
{
    int32_t result;
    size_t read, readable;
    uint8_t* buffer = NULL;

    // Read into transfer buffer - ensure we leave enough time between batches
#define MAX_CONTINUOUS_LOG_PUBLISH 3
    for (int i = 0; i < MAX_CONTINUOUS_LOG_PUBLISH; i++)
    {
        readable = io_stream_readable(connection->log_stream);
        if (!readable)
            break;

        if (buffer)
            prx_buffer_release(connection->buffer_pool, buffer);
        buffer = (uint8_t*)prx_buffer_new(connection->buffer_pool, readable);
        if (!buffer)
            break;

        result = io_stream_read(connection->log_stream, buffer, 
            prx_buffer_get_size(connection->buffer_pool, buffer), &read);
        if (result != er_ok || read == 0)
            break;

        result = io_mqtt_connection_publish(connection->mqtt_connection,
            STRING_c_str(connection->event_uri), NULL, buffer, read, NULL, NULL);
        if (result != er_ok)
            break;
    } 
    while (0);

    if (buffer)
        prx_buffer_release(connection->buffer_pool, buffer);

    // Reschedule...
    __do_later(connection, io_iot_hub_umqtt_connection_send_log_telemetry, 
        LOG_PUBLISH_INTERVAL);
}

//
// Push alive property periodically
//
static void io_iot_hub_umqtt_connection_send_alive_property(
    io_iot_hub_umqtt_connection_t* connection
)
{
    int32_t result;
    const char* publish;

    publish = !connection->alive ? "{ \"alive\": 0 }" :
        "{"
         "\"module\": \"" MODULE_VERSION "\","
         "\"alive\": 1 "
        "}";
    result = io_mqtt_connection_publish(connection->mqtt_connection,
        "$iothub/twin/PATCH/properties/reported/?$rid=1", NULL, 
        (const uint8_t*)publish, strlen(publish) + 1, NULL, NULL);
    if (result != er_ok)
    {
        log_error(connection->log, "Failed to publish %s (%s)", 
            publish, prx_err_string(result));
    }
    // Reschedule...
    __do_later(connection, io_iot_hub_umqtt_connection_send_alive_property,
        ALIVE_PUBLISH_INTERVAL);
}

//
// Send message - converts protocol message to transport message
//
static int32_t io_iot_hub_umqtt_connection_on_send(
    io_iot_hub_umqtt_connection_t* connection,
    io_message_t* message
)
{
    int32_t result;
    io_codec_ctx_t ctx, obj;
    io_dynamic_buffer_stream_t stream;
    io_stream_t* codec_stream;
    STRING_HANDLE response_uri = NULL;

    codec_stream = io_dynamic_buffer_stream_init(&stream, 
        connection->buffer_pool, 0x400);
    if (!codec_stream)
    {
        log_error(connection->log, "Failed to create dynamic buffer codec stream");
        return er_out_of_memory;
    }
    do
    {
        // Post to respoinse uri if this message has correlation id, otherwise to event
        if (message->correlation_id)
        {
            // For simplicity always return error 200, and formulate the uri manually...
            response_uri = STRING_construct("$iothub/methods/res/200/?$rid=");
            if (!response_uri)
            {
                result = er_out_of_memory;
                break;
            }

            result = STRING_concat_int(response_uri, message->correlation_id, 16);
            if (result != er_ok)
            {
                log_error(connection->log, "Failed to add request id to uri (%s)",
                    prx_err_string(result));
                break;
            }
        }

        result = io_codec_ctx_init(io_codec_by_id(connection->base.codec_id),
            &ctx, codec_stream, false, connection->log);
        if (result != er_ok)
        {
            log_error(connection->log, "Failed to initialize codec from stream (%s)",
                prx_err_string(result));
            break;
        }

        // Convert protocol message 
        result = io_encode_object(&ctx, NULL, false, &obj);
        if (result != er_ok)
        {
            log_error(connection->log, "Failed to encode protocol object (%s)",
                prx_err_string(result));
            break;
        }

        result = io_encode_message(&obj, message);
        if (result != er_ok)
        {
            log_error(connection->log, "Failed to encode protocol message (%s)",
                prx_err_string(result));
            break;
        }

        // Note: dynamic stream has no close function, so no cleanup needed
        result = io_codec_ctx_fini(&ctx, codec_stream, true);
        if (result != er_ok)
        {
            log_error(connection->log, "Failed to finalize codec to stream (%s)",
                prx_err_string(result));
            break;
        }

        result = io_message_clone(message, &message); // We will always get a callback
        if (result != er_ok)
            break;

        if (message->type == io_message_type_data ||
            message->type == io_message_type_poll)
        {
            log_trace(connection->log, "OUT: [%s#%u]",
                io_message_type_as_string(message->type), message->seq_id);
        }
        else
        {
            log_info(connection->log, "OUT: [%s#%u]",
                io_message_type_as_string(message->type), message->seq_id);
        }

        // Buffer now contains encoded response, send through transport
        result = io_mqtt_connection_publish(connection->mqtt_connection,
            response_uri ? STRING_c_str(response_uri) : STRING_c_str(connection->event_uri), 
            NULL, stream.out, stream.out_len, io_iot_hub_umqtt_connection_on_send_complete, 
            message);

        //
        // This is called from proxy server scheduler.  Since it is the same scheduler
        // as used by mqtt transport, it does not need to be decoupled.  However, should  
        // we decide to run the server on a different scheduler, e.g. thread pool, this
        // needs to posted to mqtt scheduler. (TODO)
        //
        if (result != er_ok)
        {
            log_error(connection->log, "Failed to send buffer over umqtt (%s)",
                prx_err_string(result));
        }
        break;
    } 
    while (0);

    if (response_uri)
        STRING_delete(response_uri);
    if (stream.out)
        prx_buffer_release(connection->buffer_pool, stream.out);
    return result;
}

// 
// Receive message - converts buffer to protocol message
//
static void io_iot_hub_umqtt_connection_on_receive(
    void* context,
    io_mqtt_properties_t* properties,
    const uint8_t* body,
    size_t body_len
)
{
    int32_t result;
    io_message_t* message = NULL;
    io_iot_hub_umqtt_connection_t* connection;
    io_codec_ctx_t ctx, obj;
    char request_id[UUID_PRINTABLE_STRING_LENGTH];
    io_fixed_buffer_stream_t stream;
    io_stream_t* codec_stream;
    bool is_null;

    connection = (io_iot_hub_umqtt_connection_t*)context;
    dbg_assert_ptr(connection);

    codec_stream = io_fixed_buffer_stream_init(&stream, body, body_len, NULL, 0);
    dbg_assert_ptr(codec_stream);
    // Convert body into protocol message
    result = io_codec_ctx_init(io_codec_by_id(connection->base.codec_id),
        &ctx, codec_stream, true, connection->log);
    if (result != er_ok)
    {
        log_error(connection->log, "Failed to initialize codec from stream (%s)",
            prx_err_string(result));
        return;
    }
    do
    {
        // Create new protocol message from message pool
        result = io_message_create_empty(connection->base.message_pool, &message);
        if (result != er_ok)
        {
            log_error(connection->log, "Failed creating protocol message (%s)",
                prx_err_string(result));
            break;
        }

        // Decode buffer into protocol message
        result = io_decode_object(&ctx, NULL, &is_null, &obj);
        if (result != er_ok)
        {
            log_error(connection->log, "Failed to decode object (%s)",
                prx_err_string(result));
            break;
        }

        if (is_null) // Must never be null...
            result = er_invalid_format;
        else
            result = io_decode_message(&obj, message);
        if (result != er_ok)
        {
            log_error(connection->log, "Failed to decode message (%s)",
                prx_err_string(result));
            break;
        }

        // Get request id from properties
        result = io_mqtt_properties_get(
            properties, "$rid", request_id, sizeof(request_id));
        if (result != er_ok)
        {
            log_error(connection->log, "Failed to get $rid from properties (%s)",
                prx_err_string(result));
            break;
        }

        result = string_to_int(request_id, 16, &message->correlation_id);
        if (result != er_ok)
        {
            log_error(connection->log, "Failed to parse $rid value %s (%s)",
                request_id, prx_err_string(result));
            break;
        }

        if (message->type == io_message_type_data ||
            message->type == io_message_type_poll)
        {
            log_trace(connection->log, "IN: [%s#%u]", 
                io_message_type_as_string(message->type), message->seq_id);
        }
        else
        {
            log_info(connection->log, "IN: [%s#%u]", 
                io_message_type_as_string(message->type), message->seq_id);
        }

        result = connection->base.handler_cb(connection->base.handler_cb_ctx, 
            io_connection_received, message, result);
        //
        // This is called from mqtt scheduler thread.  Since it is the same scheduler
        // as used by server, it does not need to be decoupled.  However, should we 
        // decide to run the server on a different scheduler, e.g. thread pool, this
        // needs to post to server scheduler instead.  (TODO)
        //
        break;
    } 
    while (0);

    (void)io_codec_ctx_fini(&ctx, codec_stream, false);

    if (message)
        io_message_release(message);
}

//
// Open a iothub mqtt methods connection from connection string
//
static int32_t io_iot_hub_umqtt_server_transport_create_connection(
    void* context,
    prx_ns_entry_t* entry,
    io_codec_id_t codec,
    io_connection_cb_t handler_cb,
    void* handler_cb_ctx,
    prx_scheduler_t* scheduler,
    io_connection_t** created
)
{
    int32_t result;
    io_iot_hub_umqtt_connection_t* connection;
    io_url_t* url = NULL;
    STRING_HANDLE path = NULL;
    io_cs_t* cs = NULL;
    
    chk_arg_fault_return(scheduler);
    chk_arg_fault_return(handler_cb);
    chk_arg_fault_return(entry);
    chk_arg_fault_return(created);

    dbg_assert_ptr(context);
    (void)context;

    connection = mem_zalloc_type(io_iot_hub_umqtt_connection_t);
    if (!connection)
        return er_out_of_memory;
    do
    {
        connection->log = log_get("tp_mqtt");
        result = io_iot_hub_connection_base_init(
            &connection->base, codec, handler_cb, handler_cb_ctx);
        if (result != er_ok)
            break;

        result = prx_ns_entry_get_cs(entry, &cs);
        if (result != er_ok)
            break;

        connection->event_uri = STRING_construct("devices/");
        if (!connection->event_uri ||
            0 != STRING_concat(connection->event_uri, io_cs_get_device_id(cs)) ||
            0 != STRING_concat(connection->event_uri, "/messages/events/"))
        {
            result = er_out_of_memory;
            break;
        }

        result = prx_scheduler_create(scheduler, &connection->scheduler);
        if (result != er_ok)
            break;

        // Create protocol factory and send message pools
        result = prx_dynamic_pool_create("umqtt.connection.send",
            0, NULL, &connection->buffer_pool);
        if (result != er_ok)
            break;

        //
        // Create special user name:
        // {HostName}/{DeviceId}/api-version={v}&DeviceClientType={Type}%2F{Version}
        //
        path = STRING_construct(io_cs_get_host_name(cs));
        if (!path ||
            0 != STRING_concat(path, "/") ||
            0 != STRING_concat(path, io_cs_get_device_id(cs)) ||
            0 != STRING_concat(path,
                "/api-version=" API_VERSION 
                "&DeviceClientType=" MODULE_NAME "%2F" MODULE_VERSION))
        {
            result = er_out_of_memory;
            break;
        }

        // NULL scheme ensures best scheme is chosen
        result = io_url_create(
            __prx_config_get_int(prx_config_key_connect_flag, 0) & 0x1 ? "wss" : NULL,
            io_cs_get_host_name(cs), 0, "/$iothub/websocket", STRING_c_str(path), NULL, 
            &url);
        if (result != er_ok)
            break;
        // Add token provider to address 
        result = io_cs_create_token_provider(cs, &url->token_provider);
        if (result != er_ok)
            break;

        // Create connection
        result = io_mqtt_connection_create(url, io_cs_get_device_id(cs), scheduler, 
            &connection->mqtt_connection);
        if (result != er_ok)
            break;
        io_url_free(url);
        url = NULL;

        // Subscribe to method
        result = io_mqtt_connection_subscribe(connection->mqtt_connection,
            "$iothub/methods/POST/#", io_iot_hub_umqtt_connection_on_receive,
            connection, &connection->subscription);
        if (result != er_ok)
            break;

        // Start connection
        result = io_mqtt_connection_connect(connection->mqtt_connection, 
            io_iot_hub_connection_base_reconnect_handler, &connection->base);
        if (result != er_ok)
            break;

        connection->alive = true;
        __do_next(connection, io_iot_hub_umqtt_connection_send_alive_property);

        if (__prx_config_get_int(prx_config_key_log_telemetry, 0))
        {
            connection->log_stream = log_stream_get();
            if (!connection->log_stream)
                log_error(connection->log, "Failed to create telemetry log stream (%s).",
                    prx_err_string(result));
            else
            {
                // Start sending initial logs
                __do_next(connection, io_iot_hub_umqtt_connection_send_log_telemetry);
            }
        }


        STRING_delete(path);
        path = NULL;
        
        connection->base.funcs.context =
            connection;
        connection->base.funcs.on_send = (io_connection_send_t)
            io_iot_hub_umqtt_connection_on_send;
        connection->base.funcs.on_close = (io_connection_close_t)
            io_iot_hub_umqtt_connection_on_close;
        connection->base.funcs.on_free = (io_connection_free_t)
            io_iot_hub_umqtt_connection_on_free;

        io_cs_free(cs);
        *created = &connection->base.funcs;
        return er_ok;

    } while (0);

    if (path)
        STRING_delete(path);
    if (url)
        io_url_free(url);
    if (cs)
        io_cs_free(cs);

    if (connection->subscription)
        io_mqtt_subscription_release(connection->subscription);
    connection->subscription = NULL;
    if (connection->mqtt_connection)
        io_mqtt_connection_close(connection->mqtt_connection);
    connection->mqtt_connection = NULL;

    io_iot_hub_umqtt_connection_on_free(connection);
    return result;
}

//
// Called when the connection is freed
//
static void io_iot_hub_ws_connection_on_free(
    io_iot_hub_ws_connection_t* connection
)
{
    dbg_assert_ptr(connection);
    dbg_assert(!connection->ws_connection, "Should be closed");

    io_iot_hub_connection_base_deinit(&connection->base);
    log_trace(connection->log, "Websocket transport connection freed.");
    mem_free_type(io_iot_hub_ws_connection_t, connection);
}

//
// Called when the connection interface closes the connection
//
static void io_iot_hub_ws_connection_on_close(
    io_iot_hub_ws_connection_t* connection
)
{
    dbg_assert_ptr(connection);

    if (connection->ws_connection)
        io_ws_connection_close(connection->ws_connection);
    connection->ws_connection = NULL;

    // TODO: Decouple, subscribe and wait for close to complete...
    // For now, close returns all messages, and then closes on the scheduler thread...
    // Same with retries and errors...

    (void)connection->base.handler_cb(connection->base.handler_cb_ctx, 
        io_connection_closed, NULL, er_ok);
    connection->base.handler_cb = NULL;
}

// 
// Writes protocol message to send stream
//
static int32_t io_iot_hub_ws_connection_send_handler(
    void* context,
    io_stream_t* stream
)
{
    int32_t result;
    io_codec_ctx_t ctx, obj;
    io_iot_hub_ws_connection_t* connection;

    connection = (io_iot_hub_ws_connection_t*)context;
    dbg_assert_ptr(connection);
    dbg_assert_ptr(connection->message);
    dbg_assert_ptr(stream);
    do
    {
        result = io_codec_ctx_init(io_codec_by_id(connection->base.codec_id),
            &ctx, stream, false, connection->log);
        if (result != er_ok)
        {
            log_error(connection->log, "Failed to initialize codec from stream (%s)",
                prx_err_string(result));
            break;
        }

        // Convert protocol message 
        result = io_encode_object(&ctx, NULL, false, &obj);
        if (result == er_ok)
            result = io_encode_message(&obj, connection->message);
        if (result != er_ok)
        {
            log_error(connection->log, "Failed to encode protocol message object (%s)",
                prx_err_string(result));
            break;
        }

        // Note: dynamic stream has no close function, so no cleanup needed
        result = io_codec_ctx_fini(&ctx, stream, true);
        if (result != er_ok)
        {
            log_error(connection->log, "Failed to finalize codec to stream (%s)",
                prx_err_string(result));
            break;
        }
    } 
    while (0);

    if (result != er_ok)
        io_stream_reset(stream);  // Reset stream as to not to send anything

    return result;
}

//
// Message sending completed
//
static void io_iot_hub_ws_connection_on_send_complete(
    void* context,
    int32_t result
)
{
    dbg_assert_ptr(context);
    (void)result;
    io_message_release((io_message_t*)context);
}

//
// Send message - converts protocol message to transport message
//
static int32_t io_iot_hub_ws_connection_on_send(
    io_iot_hub_ws_connection_t* connection,
    io_message_t* message
)
{
    int32_t result;

    result = io_message_clone(message, &connection->message);
    if (result != er_ok)
        return result;

    if (message->type == io_message_type_data ||
        message->type == io_message_type_poll)
    {
        log_debug(connection->log, "OUT (ws): [%s#%u]",
            io_message_type_as_string(message->type), message->seq_id);
    }
    else
    {
        log_info(connection->log, "OUT (ws): [%s#%u]",
            io_message_type_as_string(message->type), message->seq_id);
    }

    result = io_ws_connection_send(connection->ws_connection, 
        io_iot_hub_ws_connection_send_handler, connection, 
        io_iot_hub_ws_connection_on_send_complete, connection->message);

    connection->message = NULL; // We will always get a send complete callback...

    if (result != er_ok)
    {
        log_error(connection->log, "Failed to send message over websocket (%s)",
            prx_err_string(result));
    }
    return result;
}

// 
// Reads protocol message from receive stream
//
static int32_t io_iot_hub_ws_connection_receive_handler(
    void* context,
    io_stream_t* stream
)
{
    int32_t result;
    io_message_t* message = NULL;
    io_codec_ctx_t ctx, obj;
    io_iot_hub_ws_connection_t* connection;
    bool is_null;

    connection = (io_iot_hub_ws_connection_t*)context;
    dbg_assert_ptr(connection);
    dbg_assert_ptr(stream);

    // Convert body into protocol message
    result = io_codec_ctx_init(io_codec_by_id(connection->base.codec_id), 
        &ctx, stream, true, connection->log);
    if (result != er_ok)
    {
        log_error(connection->log, "Failed to initialize codec from stream (%s)",
            prx_err_string(result));
        return result;
    }
    do
    {
        // Get new empty protocol message from message pool
        result = io_message_create_empty(connection->base.message_pool, &message);
        if (result != er_ok)
        {
            log_error(connection->log, "Failed creating protocol message (%s)",
                prx_err_string(result));
            break;
        }

        // Decode mpack encoded stream buffers into protocol message
        result = io_decode_object(&ctx, NULL, &is_null, &obj);
        if (result == er_ok)
        {
            if (is_null) // Must never be null...
                result = er_invalid_format;
            else
                result = io_decode_message(&obj, message);
        }
        if (result != er_ok)
        {
            log_error(connection->log, "Failed to decode protocol message object (%s)",
                prx_err_string(result));
            break;
        }

        if (message->type == io_message_type_data ||
            message->type == io_message_type_poll)
        {
            log_debug(connection->log, "IN (ws): [%s#%u]",
                io_message_type_as_string(message->type), message->seq_id);
        }
        else
        {
            log_info(connection->log, "IN (ws): [%s#%u]",
                io_message_type_as_string(message->type), message->seq_id);
        }

        result = connection->base.handler_cb(connection->base.handler_cb_ctx, 
            io_connection_received, message, result);
        //
        // This is called from ws scheduler thread.  Since it is the same scheduler
        // as used by server, it does not need to be decoupled.  However, should we 
        // decide to run the server on a different scheduler, e.g. thread pool, this
        // needs to post to server scheduler instead.  (TODO)
        //
        break;
    } 
    while (0);

    (void)io_codec_ctx_fini(&ctx, stream, false);

    if (message)
        io_message_release(message);
    return result;
}

//
// Open a iot hub websocket stream
//
static int32_t io_iot_hub_ws_server_transport_create_connection(
    void* context,
    prx_ns_entry_t* entry,
    io_codec_id_t codec,
    io_connection_cb_t handler_cb,
    void* handler_cb_ctx,
    prx_scheduler_t* scheduler,
    io_connection_t** created
)
{
    int32_t result;
    io_url_t* url = NULL;
    STRING_HANDLE path = NULL, client_id = NULL;
    const char* user_header, *pwd_header;
    io_ref_t id;
    io_iot_hub_ws_connection_t* connection;
    io_cs_t* cs = NULL;

    chk_arg_fault_return(scheduler);
    chk_arg_fault_return(handler_cb);
    chk_arg_fault_return(entry);
    chk_arg_fault_return(created);

    dbg_assert_ptr(context);
    (void)context;

    connection = mem_zalloc_type(io_iot_hub_ws_connection_t);
    if (!connection)
        return er_out_of_memory;
    do
    {
        connection->log = log_get("tp_ws");

        result = io_iot_hub_connection_base_init(
            &connection->base, codec, handler_cb, handler_cb_ctx);
        if (result != er_ok)
            break;

        result = prx_ns_entry_get_cs(entry, &cs);
        if (result != er_ok)
            break;

        // Create client id
        result = prx_ns_entry_get_addr(entry, &id);
        if (result != er_ok)
            break;
        client_id = io_ref_to_STRING(&id);
        if (!client_id)
        {
            result = er_out_of_memory;
            break;
        }

        user_header = "x-id";
        pwd_header = "x-authz";

        /**/ if (io_cs_get_endpoint_name(cs))
        {
            // Connection string refers to a service bus relay connection 
            // wss://{{HostName}}:443/$hc/{{EndpointName}}?
            //                  sb-hc-action=connect&sb-hc-id={client_id}
            path = STRING_construct("/$hc/");
            if (!path ||
                0 != STRING_concat(path, io_cs_get_endpoint_name(cs)) ||
                0 != STRING_concat(path,
                    "?sb-hc-action=connect"
                    "&sb-hc-id=") ||
                0 != STRING_concat_with_STRING(path, client_id))
            {
                result = er_out_of_memory;
                break;
            }

            // Open connection with token in ServiceBusAuthorization header field
            // and client id in x-id header field
            user_header = "x-Id";
            pwd_header = "ServiceBusAuthorization";
            
            result = io_url_create("wss", io_cs_get_host_name(cs), 443,
                STRING_c_str(path), STRING_c_str(client_id), NULL, &url);
        }
        else if (io_cs_get_endpoint(cs))
        {
            // Connection string refers to a plain https endpoint
            result = io_url_parse(io_cs_get_endpoint(cs), &url);
        }
        else if (io_cs_get_entity(cs))
        {
            // Connection string refers to a remote entity on the host
            path = STRING_construct("/");
            if (!path || 0 != STRING_concat(path, io_cs_get_entity(cs)))
            {
                result = er_out_of_memory;
                break;
            }

            result = io_url_create("wss", io_cs_get_host_name(cs), 443,
                STRING_c_str(path), STRING_c_str(client_id), NULL, &url);
        }
        else
        {
            // Connection string not supported
            result = er_not_supported;
        }

        if (result != er_ok)
        {
            log_error(connection->log, "Invalid websocket connection string (%s)!", 
                prx_err_string(result));
            break;
        }

        result = io_cs_create_token_provider(cs, &url->token_provider);
        if (result != er_ok)
            break;

        result = io_ws_connection_create(url, user_header, pwd_header,
            scheduler, io_iot_hub_ws_connection_receive_handler, connection,
            &connection->ws_connection);
        if (result != er_ok)
            break;
        result = io_ws_connection_connect(connection->ws_connection,
            io_iot_hub_connection_base_reconnect_handler, &connection->base);
        if (result != er_ok)
            break;

        connection->base.funcs.context =
            connection;
        connection->base.funcs.on_send = (io_connection_send_t)
            io_iot_hub_ws_connection_on_send;
        connection->base.funcs.on_close = (io_connection_close_t)
            io_iot_hub_ws_connection_on_close;
        connection->base.funcs.on_free = (io_connection_free_t)
            io_iot_hub_ws_connection_on_free;

        STRING_delete(client_id);
        STRING_delete(path);
        io_url_free(url);
        io_cs_free(cs);
        *created = &connection->base.funcs;
        return er_ok;

    } while (0);

    if (url)
        io_url_free(url);
    if (cs)
        io_cs_free(cs);
    if (path)
        STRING_delete(path);
    if (client_id)
        STRING_delete(client_id);

    if (connection->ws_connection)
        io_ws_connection_close(connection->ws_connection);
    connection->ws_connection = NULL;
    io_iot_hub_ws_connection_on_free(connection);
    return result;
}

//
// mqtt server transport (methods)
//
io_transport_t* io_iot_hub_mqtt_server_transport(
    void
)
{
    static io_transport_t transport = {
        io_iot_hub_umqtt_server_transport_create_connection,
        &transport
    };
    return &transport;
}

//
// Websocket server transport (stream)
//
io_transport_t* io_iot_hub_ws_server_transport(
    void
)
{
    static io_transport_t transport = {
        io_iot_hub_ws_server_transport_create_connection,
        &transport
    };
    return &transport;
}

