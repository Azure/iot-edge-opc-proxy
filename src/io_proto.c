// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// #define DBG_MEM

#include "util_mem.h"
#include "io_ref.h"
#include "prx_buffer.h"
#define ENABLE_GLOBAL
#include "io_proto.h"
#undef ENABLE_GLOBAL
#include "io_types.h"

#define VERSION  ((MODULE_MAJ_VER << 6) | (MODULE_MIN_VER + 5))

//
// Protocol factory creates protocol messages from pool memory
//
struct io_message_factory
{
    prx_buffer_factory_t* messages;    // Pool of fixed size message objects
    prx_buffer_factory_t* buffers;     // Dynamic buffers for payload allocs
};


#if defined(DBG_MEM)
#define dbg_assert_msg(m) \
    do { if (m->buffer) \
        prx_buffer_get_size(m->owner->buffers, m->buffer); \
    prx_buffer_get_size(m->owner->messages, m); } while(0)
#else
#define dbg_assert_msg(m) \
    dbg_assert_ptr(m);
#endif

//
// Create protocol message factory (actually a type safe fixed buffer pool)
//
int32_t io_message_factory_create(
    size_t pool_size,
    size_t low_watermark,
    size_t high_watermark,
    prx_buffer_pool_cb_t cb,
    void* context,
    io_message_factory_t** created
)
{
    int result;
    io_message_factory_t* factory;
    prx_pool_config_t config;
    
    factory = mem_zalloc_type(io_message_factory_t);
    if (!factory)
        return er_out_of_memory;
    do
    {
        memset(&config, 0, sizeof(config));
        config.initial_count = pool_size;
        config.max_count = cb ? pool_size : 0;
        config.low_watermark = low_watermark;
        config.high_watermark = high_watermark;
        config.context = context;
        config.cb = cb;

        result = prx_fixed_pool_create("messages",
            sizeof(io_message_t), &config, &factory->messages);
        if (result != er_ok)
            break;

        result = prx_dynamic_pool_create("buffers",
            0, NULL, &factory->buffers);
        if (result != er_ok)
            break;

        *created = factory;
        return er_ok;
    }
    while (0);

    io_message_factory_free(factory);
    return result;
}

//
// Free protocol message factory
//
void io_message_factory_free(
    io_message_factory_t* factory
)
{
    if (!factory)
        return;

    if (factory->buffers)
        prx_buffer_factory_free(factory->buffers);
    if (factory->messages)
        prx_buffer_factory_free(factory->messages);

    mem_free_type(io_message_factory_t, factory);
}

//
// Dynamic buffer pool allocator
//
static int32_t io_message_allocator(
    io_codec_ctx_t* ctx,
    size_t size,
    void** mem,
    size_t* mem_size
)
{
    int32_t result;
    io_message_t* message = (io_message_t*)ctx->user_context;
    dbg_assert_ptr(message);
    dbg_assert_ptr(mem_size);

    result = io_message_allocate_buffer(message, size, mem);
    if (result == er_ok)
        *mem_size = size;
    else
        *mem_size = 0;
    return result;
}

//
// Encode a ping request
//
static int32_t io_encode_ping_request(
    io_codec_ctx_t* ctx,
    io_ping_request_t* request
)
{
    int32_t result;
    __io_encode_type_begin(ctx, request, 1);
    __io_encode_object(ctx, prx_socket_address, request, address);
    __io_encode_type_end(ctx);
    return result;
}

//
// Decode a ping request
//
static int32_t io_decode_ping_request(
    io_codec_ctx_t* ctx,
    io_ping_request_t* request
)
{
    int32_t result;
    __io_decode_type_begin(ctx, request, 1);
    __io_decode_object(ctx, prx_socket_address, request, address);
    __io_decode_type_end(ctx);
    return result;
}

//
// Encode a ping response
//
static int32_t io_encode_ping_response(
    io_codec_ctx_t* ctx,
    io_ping_response_t* response
)
{
    int32_t result;

    __io_encode_type_begin(ctx, response, 3);
    __io_encode_object(ctx, prx_socket_address, response, address);
    result = io_encode_bin(ctx, "physical_address",
        response->physical_address, sizeof(response->physical_address));
    if (result != er_ok)
        return result;
    __io_encode_value(ctx, uint32, response, time_ms);
    __io_encode_type_end(ctx);
    return result;
}

//
// Decode a ping response
//
static int32_t io_decode_ping_response(
    io_codec_ctx_t* ctx,
    io_ping_response_t* response
)
{
    int32_t result;
    size_t len;

    __io_decode_type_begin(ctx, response, 3);
    __io_decode_object(ctx, prx_socket_address, response, address);
    len = sizeof(response->physical_address);
    result = io_decode_bin_fixed(
        ctx, "physical_address", response->physical_address, &len);
    if (result != er_ok)
        return result;
    __io_decode_value(ctx, uint32, response, time_ms);
    __io_decode_type_end(ctx);
    return result;
}

//
// Encode a resolve request
//
static int32_t io_encode_resolve_request(
    io_codec_ctx_t* ctx,
    io_resolve_request_t* request
)
{
    int32_t result;
    __io_encode_type_begin(ctx, request, 4);
    __io_encode_value(ctx, int32, request, family);
    __io_encode_value(ctx, uint32, request, flags);
    __io_encode_value(ctx, uint16, request, port);
    result = io_encode_string(ctx, "host", request->host);
    if (result != er_ok)
        return result;
    __io_encode_type_end(ctx);
    return result;
}

//
// Decode a resolve request
//
static int32_t io_decode_resolve_request(
    io_codec_ctx_t* ctx,
    io_resolve_request_t* request
)
{
    int32_t result;
    __io_decode_type_begin(ctx, request, 4);
    __io_decode_value(ctx, int32, request, family);
    __io_decode_value(ctx, uint32, request, flags);
    __io_decode_value(ctx, uint16, request, port);
    result = io_decode_string_fixed(ctx, "host",
        request->host, sizeof(request->host));
    if (result != er_ok)
        return result;
    __io_decode_type_end(ctx);
    request->reserved = 0;
    return result;
}

//
// Encode a resolve response
//
static int32_t io_encode_resolve_response(
    io_codec_ctx_t* ctx,
    io_resolve_response_t* response
)
{
    int32_t result;
    io_codec_ctx_t arr;

    __io_encode_type_begin(ctx, response, 1);

    result = io_encode_array(ctx, "results", (size_t)response->result_count, &arr);
    if (result != er_ok)
        return result;

    for (prx_size_t i = 0; i < response->result_count; i++)
    {
        io_codec_ctx_t obj;
        result = io_encode_object(&arr, NULL, false, &obj);
        if (result != er_ok)
            break;
        result = io_encode_prx_addrinfo(&obj, &response->results[i]);
        if (result != er_ok)
            break;
    }
    if (result != er_ok)
        return result;

    __io_encode_type_end(ctx);
    return result;
}

//
// Decode a resolve response
//
static int32_t io_decode_resolve_response(
    io_codec_ctx_t* ctx,
    io_resolve_response_t* response
)
{
    int32_t result = er_ok;
    io_codec_ctx_t arr;
    size_t size;

    __io_decode_type_begin(ctx, response, 1);

    result = io_decode_array(ctx, "results", &size, &arr);
    if (result != er_ok)
        return result;

    response->result_count = (prx_size_t)size;
    if (!size)
        response->results = NULL;
    else
    {
        size = (size + 1) * sizeof(prx_addrinfo_t);
        if (ctx->default_allocator) // Use default allocator if set
            result = ctx->default_allocator(ctx, size, (void**)&response->results, &size);
        else
        {
            // Otherwise allocate with malloc - correponds to prx_client_freeaddrinfo
            response->results = (prx_addrinfo_t*)mem_zalloc(size);
            if (!response->results)
                result = er_out_of_memory;
        }
        if (result != er_ok)
            return result;

        for (prx_size_t i = 0; i < response->result_count; i++)
        {
            io_codec_ctx_t obj;
            result = io_decode_object(&arr, NULL, NULL, &obj);
            if (result != er_ok)
                break;
            result = io_decode_prx_addrinfo(&obj, &response->results[i]);
            if (result != er_ok)
                break;
        }

        if (result != er_ok)
        {
            // Free memory...
            if (ctx->default_allocator)
                ctx->default_allocator(ctx, 0, (void**)&response->results, &size);
            else
                mem_free(response->results);
            response->results = NULL;
            return result;
        }
    }
    __io_decode_type_end(ctx);
    return result;
}

//
// Encode a link request
//
static int32_t io_encode_link_request(
    io_codec_ctx_t* ctx,
    io_link_request_t* request
)
{
    int32_t result;
    __io_encode_type_begin(ctx, request, 2);
    __io_encode_value(ctx, uint8, request, version);
    __io_encode_object(ctx, prx_socket_properties, request, props);
    __io_encode_type_end(ctx);
    return result;
}

//
// Decode a link request
//
static int32_t io_decode_link_request(
    io_codec_ctx_t* ctx,
    io_link_request_t* request
)
{
    int32_t result;
    __io_decode_type_begin(ctx, request, 2);
    __io_decode_value(ctx, uint8, request, version);
    __io_decode_object(ctx, prx_socket_properties, request, props);
    __io_decode_type_end(ctx);
    return result;
}

//
// Encode a link response
//
static int32_t io_encode_link_response(
    io_codec_ctx_t* ctx,
    io_link_response_t* response
)
{
    int32_t result;
    __io_encode_type_begin(ctx, response, 3);
    __io_encode_object(ctx, ref, response, link_id);
    __io_encode_object(ctx, prx_socket_address, response, local_address);
    __io_encode_object(ctx, prx_socket_address, response, peer_address);
    __io_encode_type_end(ctx);
    return result;
}

//
// Decode a link response
//
static int32_t io_decode_link_response(
    io_codec_ctx_t* ctx,
    io_link_response_t* response
)
{
    int32_t result;
    __io_decode_type_begin(ctx, response, 3);
    __io_decode_object(ctx, ref, response, link_id);
    __io_decode_object(ctx, prx_socket_address, response, local_address);
    __io_decode_object(ctx, prx_socket_address, response, peer_address);
    __io_decode_type_end(ctx);
    return result;
}

//
// Encode a setopt request
//
static int32_t io_encode_setopt_request(
    io_codec_ctx_t* ctx,
    io_setopt_request_t* request
)
{
    int32_t result;
    __io_encode_type_begin(ctx, request, 1);
    __io_encode_object(ctx, prx_socket_option_value, request, so_val);
    __io_encode_type_end(ctx);
    return result;
}

//
// Decode a setopt request
//
static int32_t io_decode_setopt_request(
    io_codec_ctx_t* ctx,
    io_setopt_request_t* request
)
{
    int32_t result;
    __io_decode_type_begin(ctx, request, 1);
    __io_decode_object(ctx, prx_socket_option_value, request, so_val);
    __io_decode_type_end(ctx);
    return result;
}

//
// Encode a getopt request
//
static int32_t io_encode_getopt_request(
    io_codec_ctx_t* ctx,
    io_getopt_request_t* request
)
{
    int32_t result;
    __io_encode_type_begin(ctx, request, 1);
    __io_encode_value(ctx, int32, request, so_opt);
    __io_encode_type_end(ctx);
    return result;
}

//
// Decode a getopt request
//
static int32_t io_decode_getopt_request(
    io_codec_ctx_t* ctx,
    io_getopt_request_t* request
)
{
    int32_t result;
    __io_decode_type_begin(ctx, request, 1);
    __io_decode_value(ctx, int32, request, so_opt);
    __io_decode_type_end(ctx);
    return result;
}

//
// Encode a getopt response
//
static int32_t io_encode_getopt_response(
    io_codec_ctx_t* ctx,
    io_getopt_response_t* response
)
{
    int32_t result;
    __io_encode_type_begin(ctx, response, 1);
    __io_encode_object(ctx, prx_socket_option_value, response, so_val);
    __io_encode_type_end(ctx);
    return result;
}

//
// Decode a getopt response
//
static int32_t io_decode_getopt_response(
    io_codec_ctx_t* ctx,
    io_getopt_response_t* response
)
{
    int32_t result;
    __io_decode_type_begin(ctx, response, 1);
    __io_decode_object(ctx, prx_socket_option_value, response, so_val);
    __io_decode_type_end(ctx);
    return result;
}

//
// Encode a open request
//
static int32_t io_encode_open_request(
    io_codec_ctx_t* ctx,
    io_open_request_t* request
)
{
    int32_t result;
    __io_encode_type_begin(ctx, request, 3);
    __io_encode_object(ctx, ref, request, stream_id);
    result = io_encode_string(
        ctx, "connection-string", request->connection_string);
    if (result != er_ok)
        return result;
    __io_encode_value(ctx, bool, request, polled);
    __io_encode_type_end(ctx);
    return result;
}

//
// Decode a open request
//
static int32_t io_decode_open_request(
    io_codec_ctx_t* ctx,
    io_open_request_t* request
)
{
    int32_t result;
    __io_decode_type_begin(ctx, request, 3);
    __io_decode_object(ctx, ref, request, stream_id);
    result = io_decode_string_default(
        ctx, "connection-string", (char**)&request->connection_string);
    if (result != er_ok)
        return result;
    __io_decode_value(ctx, bool, request, polled);
    __io_decode_type_end(ctx);
    return result;
}

//
// Encode a poll request
//
static int32_t io_encode_poll_message(
    io_codec_ctx_t* ctx,
    io_poll_message_t* message
)
{
    int32_t result;
    __io_encode_type_begin(ctx, message, 1);
    __io_encode_value(ctx, uint64, message, timeout);
    __io_encode_type_end(ctx);
    return result;
}

//
// Decode a poll request
//
static int32_t io_decode_poll_message(
    io_codec_ctx_t* ctx,
    io_poll_message_t* message
)
{
    int32_t result;
    __io_decode_type_begin(ctx, message, 1);
    __io_decode_value(ctx, uint64, message, timeout);
    __io_decode_type_end(ctx);
    return result;
}

//
// Encode a data message
//
static int32_t io_encode_data_message(
    io_codec_ctx_t* ctx,
    io_data_message_t* message
)
{
    int32_t result;

    __io_encode_type_begin(ctx, message, 2);
    __io_encode_object(ctx, prx_socket_address, message, source_address);
    result = io_encode_bin(ctx, "buffer",
        message->buffer, (uint32_t)message->buffer_length);
    if (result != er_ok)
        return result;
    __io_encode_type_end(ctx);
    return result;
}

//
// Decode a data message
//
static int32_t io_decode_data_message(
    io_codec_ctx_t* ctx,
    io_data_message_t* message
)
{
    int32_t result;
    size_t read;
    read = (size_t)message->buffer_length;
    if (message->buffer && read == 0)
        return er_arg;

    __io_decode_type_begin(ctx, message, 2);
    __io_decode_object(ctx, prx_socket_address, message, source_address);
    result = io_decode_bin_default(ctx, "buffer", (void**)&message->buffer, &read);
    if (result != er_ok)
        return result;
    message->buffer_length = (prx_size_t)read;
    __io_decode_type_end(ctx);
    return result;
}

//
// Encode a close response
//
static int32_t io_encode_close_response(
    io_codec_ctx_t* ctx,
    io_close_response_t* response
)
{
    int32_t result;
    __io_encode_type_begin(ctx, response, 4);
    __io_encode_value(ctx, uint64, response, time_open);
    __io_encode_value(ctx, uint64, response, bytes_received);
    __io_encode_value(ctx, uint64, response, bytes_sent);
    __io_encode_value(ctx, int32, response, error_code);
    __io_encode_type_end(ctx);
    return result;
}

//
// Decode a close response
//
static int32_t io_decode_close_response(
    io_codec_ctx_t* ctx,
    io_close_response_t* response
)
{
    int32_t result;
    __io_decode_type_begin(ctx, response, 4);
    __io_decode_value(ctx, uint64, response, time_open);
    __io_decode_value(ctx, uint64, response, bytes_received);
    __io_decode_value(ctx, uint64, response, bytes_sent);
    __io_decode_value(ctx, int32, response, error_code);
    __io_decode_type_end(ctx);
    return result;
}

//
// Encode the message
//
static int32_t io_encode_message_content(
    io_codec_ctx_t* ctx,
    io_message_t* msg
)
{
    int32_t result;
    io_codec_ctx_t obj;

#define __io_encode_content(ctx, type, s) do { \
    result = io_encode_object(ctx, "content", false, &obj); \
    if (result != er_ok) \
        return result; \
    result = io_encode_##type (&obj, &s->content.type ); \
    if (result != er_ok) \
        return result; \
    } while(0)


#define __io_encode_null(ctx) do { \
    result = io_encode_object(ctx, "content", true, &obj); \
    if (result != er_ok) \
        return result; \
    } while(0)
    
    dbg_assert_msg(msg);
    if (msg->is_response)
    {
        /**/ if (msg->type == io_message_type_data)
            __io_encode_content(ctx, data_message, msg);
        else if (msg->type == io_message_type_close)
            __io_encode_content(ctx, close_response, msg);
        else if (msg->type == io_message_type_ping)
            __io_encode_content(ctx, ping_response, msg);
        else if (msg->type == io_message_type_poll)
            __io_encode_null(ctx);
        else if (msg->type == io_message_type_link)
            __io_encode_content(ctx, link_response, msg);
        else if (msg->type == io_message_type_open)
            __io_encode_null(ctx);
        else if (msg->type == io_message_type_setopt)
            __io_encode_null(ctx);
        else if (msg->type == io_message_type_getopt)
            __io_encode_content(ctx, getopt_response, msg);
        else if (msg->type == io_message_type_resolve)
            __io_encode_content(ctx, resolve_response, msg);
        else 
            result = er_not_supported;
    }
    else  // encode request
    {
        /**/ if (msg->type == io_message_type_data)
            __io_encode_content(ctx, data_message, msg); 
        else if (msg->type == io_message_type_close)
            __io_encode_null(ctx);
        else if (msg->type == io_message_type_ping)
            __io_encode_content(ctx, ping_request, msg);
        else if (msg->type == io_message_type_poll)
            __io_encode_content(ctx, poll_message, msg);
        else if (msg->type == io_message_type_link)
            __io_encode_content(ctx, link_request, msg);
        else if (msg->type == io_message_type_open)
            __io_encode_content(ctx, open_request, msg);
        else if (msg->type == io_message_type_setopt)
            __io_encode_content(ctx, setopt_request, msg);
        else if (msg->type == io_message_type_getopt)
            __io_encode_content(ctx, getopt_request, msg);
        else if (msg->type == io_message_type_resolve)
            __io_encode_content(ctx, resolve_request, msg);
        else
            result = er_not_supported;
    }
    dbg_assert_msg(msg);
    return result;
}

//
// Decode the message content
//
static int32_t io_decode_message_content(
    io_codec_ctx_t* ctx,
    io_message_t* msg
)
{
    int32_t result;
    io_codec_ctx_t obj;
    bool is_null;

#define __io_decode_content(ctx, type, s) do { \
    result = io_decode_object(ctx, "content", &is_null, &obj); \
    if (result != er_ok) \
        return result; \
    if (is_null) \
        return er_invalid_format; \
    result = io_decode_##type (&obj, &s->content.type ); \
    if (result != er_ok) \
        return result; \
    } while(0)

#define __io_decode_null(ctx) do { \
    result = io_decode_object(ctx, "content", &is_null, &obj); \
    if (result != er_ok) \
        return result; \
    if (!is_null) \
        return er_invalid_format; \
    } while(0)

    // Ensure all types are allocated from buffer pool
    dbg_assert_msg(msg);
    ctx->user_context = msg;
    ctx->default_allocator = io_message_allocator;

    is_null = false;
    if (msg->is_response)
    {
        /**/ if (msg->type == io_message_type_data)
            __io_decode_content(ctx, data_message, msg); 
        else if (msg->type == io_message_type_close)
            __io_decode_content(ctx, close_response, msg);
        else if (msg->type == io_message_type_ping)
            __io_decode_content(ctx, ping_response, msg);
        else if (msg->type == io_message_type_poll)
            __io_decode_null(ctx);
        else if (msg->type == io_message_type_link)
            __io_decode_content(ctx, link_response, msg);
        else if (msg->type == io_message_type_open)
            __io_decode_null(ctx);
        else if (msg->type == io_message_type_setopt)
            __io_decode_null(ctx);
        else if (msg->type == io_message_type_getopt)
            __io_decode_content(ctx, getopt_response, msg);
        else if (msg->type == io_message_type_resolve)
            __io_decode_content(ctx, resolve_response, msg);
        else
            result = er_not_supported;
    }
    else  // decode request
    {
        /**/ if (msg->type == io_message_type_data)
            __io_decode_content(ctx, data_message, msg); 
        else if (msg->type == io_message_type_close)
            __io_decode_null(ctx);
        else if (msg->type == io_message_type_ping)
            __io_decode_content(ctx, ping_request, msg);
        else if (msg->type == io_message_type_poll)
            __io_decode_content(ctx, poll_message, msg);
        else if (msg->type == io_message_type_link)
            __io_decode_content(ctx, link_request, msg);
        else if (msg->type == io_message_type_open)
            __io_decode_content(ctx, open_request, msg);
        else if (msg->type == io_message_type_setopt)
            __io_decode_content(ctx, setopt_request, msg);
        else if (msg->type == io_message_type_getopt)
            __io_decode_content(ctx, getopt_request, msg);
        else if (msg->type == io_message_type_resolve)
            __io_decode_content(ctx, resolve_request, msg);
        else
            result = er_not_supported;
    }

    dbg_assert_msg(msg);
    return result;
}

//
// Returns the name for the type
//
const char* io_message_type_as_string(
    uint32_t type
)
{
    /**/ if (type == io_message_type_ping)
        return "PING";
    else if (type == io_message_type_resolve)
        return "RESOLVE";
    else if (type == io_message_type_link)
        return "LINK";
    else if (type == io_message_type_setopt)
        return "SETOPT";
    else if (type == io_message_type_getopt)
        return "GETOPT";
    else if (type == io_message_type_open)
        return "OPEN";
    else if (type == io_message_type_data)
        return "DATA";
    else if (type == io_message_type_close)
        return "CLOSE";
    else if (type == io_message_type_poll)
        return "POLL";

    // ... more socket server messages

    else
        return "UNKNOWN";
}

//
// Create empty protocol message
//
int32_t io_message_create_empty(
    io_message_factory_t* pool,
    io_message_t** created
)
{
    io_message_t* message;

    chk_arg_fault_return(pool);
    chk_arg_fault_return(created);

    message = (io_message_t*)prx_buffer_alloc(pool->messages, NULL);
    if (!message)
        return er_out_of_memory;

    memset(message, 0, sizeof(io_message_t));
    message->owner = pool;
    *created = message;
    return er_ok;
}

//
// Create protocol message
//
int32_t io_message_create(
    io_message_factory_t* pool,
    int32_t type,
    io_ref_t* source,
    io_ref_t* target,
    io_message_t** created
)
{
    int32_t result;
    io_message_t* message;

    chk_arg_fault_return(source);
    chk_arg_fault_return(target);
    chk_arg_fault_return(created);

    result = io_message_create_empty(pool, &message);
    if (result != er_ok)
        return result;

    message->type = type;
    io_ref_copy(source, &message->source_id);
    io_ref_copy(target, &message->target_id);
    dbg_assert_msg(message);
    *created = message;
    return er_ok;
}

//
// Clone protocol message
//
int32_t io_message_clone(
    io_message_t* original,
    io_message_t** created
)
{
    io_message_t* message;
    chk_arg_fault_return(original);
    chk_arg_fault_return(created);

    dbg_assert_msg(original);
    message = (io_message_t*)prx_buffer_alloc(
        original->owner->messages, original);
    if (!message)
        return er_out_of_memory;
    dbg_assert_msg(message);
    if (original->buffer)
    {
        // clone contained buffer
        message->buffer = prx_buffer_alloc(
            original->owner->buffers, original->buffer);
        if (!message->buffer)
        {
            prx_buffer_release(original->owner->messages, message);
            return er_out_of_memory;
        }
    }
    message->owner = original->owner;
    dbg_assert_msg(message);
    *created = message;
    return er_ok;
}

//
// Allocate memory from the message object for dynamic payload
//
int32_t io_message_allocate_buffer(
    io_message_t* message,
    size_t size,
    void** mem
)
{
    int32_t result;
    size_t current_size;

    chk_arg_fault_return(message);
    chk_arg_fault_return(mem);
    dbg_assert_msg(message);
    if (!size) // No need to free, buffer is freed when message released.
    {
        *mem = NULL;
        return er_ok;
    }
    if (!message->buffer)
    {
        message->buffer = prx_buffer_new(message->owner->buffers, size);
        if (!message->buffer)
            return er_out_of_memory;
        current_size = 0;
    }
    else // otherwise grow existing buffer from pool
    {
        current_size = prx_buffer_get_size(
            message->owner->buffers, message->buffer);
        result = prx_buffer_set_size(
            message->owner->buffers, &message->buffer, current_size + size);
        if (result != er_ok)
            return result;
    }
    *mem = &((uint8_t*)message->buffer)[current_size];
    dbg_assert_msg(message);
    return er_ok;
}

//
// Release protocol message
//
void io_message_release(
    io_message_t* message
)
{
    if (!message)
        return;
    dbg_assert_ptr(message->owner);
    dbg_assert_msg(message);

    if (message->buffer)
        prx_buffer_release(message->owner->buffers, message->buffer);

    prx_buffer_release(message->owner->messages, message);
}

//
// Encode the message
//
int32_t io_encode_message(
    io_codec_ctx_t* ctx,
    io_message_t* msg
)
{
    int32_t result;

    dbg_assert_ptr(ctx);
    dbg_assert_msg(msg);

    __io_encode_type_begin(ctx, msg, 8);
    result = io_encode_uint8(ctx, "version", VERSION);
    if (result != er_ok)
        return result;

    __io_encode_object(ctx, ref, msg, source_id);
    __io_encode_object(ctx, ref, msg, proxy_id);
    __io_encode_object(ctx, ref, msg, target_id);

    __io_encode_value(ctx, int32, msg, error_code);
    __io_encode_value(ctx, bool, msg, is_response);
    __io_encode_value(ctx, uint32, msg, type);

    result = io_encode_message_content(ctx, msg);
    if (result != er_ok)
        return result;

    __io_encode_type_end(ctx);
    dbg_assert_msg(msg);
    return result;
}

//
// Decode the message
//
int32_t io_decode_message(
    io_codec_ctx_t* ctx,
    io_message_t* msg
)
{
    int32_t result;
    uint8_t version;

    dbg_assert_ptr(ctx);
    dbg_assert_msg(msg);

    __io_decode_type_begin(ctx, msg, 8);
    result = io_decode_uint8(ctx, "version", &version);
    if (result != er_ok)
        return result;
    if (version != VERSION)
    {
        log_error(NULL, "Received message with unknown version %d.", version);
        return er_invalid_format;
    }

    __io_decode_object(ctx, ref, msg, source_id);
    __io_decode_object(ctx, ref, msg, proxy_id);
    __io_decode_object(ctx, ref, msg, target_id);

    __io_decode_value(ctx, int32, msg, error_code);
    __io_decode_value(ctx, bool, msg, is_response);
    __io_decode_value(ctx, uint32, msg, type);

    result = io_decode_message_content(ctx, msg);
    if (result != er_ok)
        return result;

    __io_decode_type_end(ctx);

    dbg_assert_msg(msg);
    return result;
}

//
// Converts message to response 
//
void io_message_as_response(
    io_message_t* message
)
{
    dbg_assert_msg(message);
    io_ref_swap(&message->source_id, &message->target_id);
    memset(&message->content, 0, sizeof(message->content));
    message->is_response = true;
}
