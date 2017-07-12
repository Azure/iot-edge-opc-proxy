// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "util_stream.h"
#include "util_string.h"
#include "io_browse.h"
#include "io_types.h"
#include "io_proto.h"


// ! Keep in sync with proto.cs ids
enum
{
    browse_request_type = 1,
    browse_response_type,
};

//
// Decodes a protocol message from stream, then encodes it again to same stream
//
static int32_t io_type_stream_decode_encode(
    io_codec_id_t codec_id,
    int32_t type,
    io_stream_t* stream
)
{
    int32_t result;
    bool is_null = false;
    bool call_fini = false;
    io_codec_ctx_t ctx, obj;

    io_browse_request_t browse_request;
    io_browse_response_t browse_response;

    dbg_assert_ptr(stream);
    do
    {
        result = io_codec_ctx_init(io_codec_by_id(codec_id), &ctx, stream, true, NULL);
        if (result != er_ok)
            break;
        call_fini = true;

        // Decode message object
        result = io_decode_object(&ctx, NULL, &is_null, &obj);
        if (result != er_ok)
            break;

        switch (type)
        {
        case browse_request_type:
            // browse request
            result = io_decode_browse_request(&obj, &browse_request);
            break;
        case browse_response_type:
            // browse response
            result = io_decode_browse_response(&obj, &browse_response);
            break;
        default:
            result = er_fatal;
            break;
        }
        if (result != er_ok)
            break;

        result = io_codec_ctx_fini(&ctx, stream, false);
        call_fini = false;
        if (result != er_ok)
            break;

        result = io_stream_reset(stream);
        if (result != er_ok)
            break;

        result = io_codec_ctx_init(io_codec_by_id(codec_id), &ctx, stream, false, NULL);
        if (result != er_ok)
            break;
        call_fini = true;

        ctx.index = 1;
        result = io_encode_object(&ctx, NULL, is_null, &obj);
        if (result != er_ok)
            break;

        switch (type)
        {
        case browse_request_type:
            // browse request
            result = io_encode_browse_request(&obj, &browse_request);
            break;
        case browse_response_type:
            // browse response
            result = io_encode_browse_response(&obj, &browse_response);
            break;
        default:
            result = er_fatal;
            break;
        }
        if (result != er_ok)
            break;

        result = io_codec_ctx_fini(&ctx, stream, true);
        call_fini = false;
        if (result != er_ok)
            break;
    } 
    while (0);

    // Flush
    if (call_fini)
        (void)io_codec_ctx_fini(&ctx, stream, true);

    return result;
}

//
// Decodes a protocol message from stream, then encodes it again to same stream
//
static int32_t io_message_stream_decode_encode(
    io_codec_id_t codec_id,
    int num_messages,
    io_stream_t* stream
)
{
    int32_t result;
    bool is_null = false;
    bool call_fini = false;
    io_codec_ctx_t ctx, obj;
    io_message_factory_t* factory = NULL;
    io_message_t* message[10];
    memset(message, 0, sizeof(message));

    dbg_assert_ptr(stream);
    do
    {
        result = io_message_factory_create("proto", 2, 0, 0, 0, NULL, NULL, &factory);
        if (result != er_ok)
            break;

        for (int i = 0; i < num_messages; i++)
        {
            result = io_codec_ctx_init(io_codec_by_id(codec_id), &ctx, stream, true, NULL);
            if (result != er_ok)
                break;
            call_fini = true;

            result = io_message_create_empty(factory, &message[i]);
            if (result != er_ok)
                break;

            // Decode message object
            result = io_decode_object(&ctx, NULL, &is_null, &obj);
            if (result != er_ok)
                break;
            result = io_decode_message(&obj, message[i]);
            if (result != er_ok)
                break;

            result = io_codec_ctx_fini(&ctx, stream, false);
            call_fini = false;
            if (result != er_ok)
                break;
        }

        if (result != er_ok)
            break;
        result = io_stream_reset(stream);
        if (result != er_ok)
            break;

        // Reencode message object
        for (int i = 0; i < num_messages; i++)
        {
            result = io_codec_ctx_init(io_codec_by_id(codec_id), &ctx, stream, false, NULL);
            if (result != er_ok)
                break;
            call_fini = true;

            ctx.index = 1;
            result = io_encode_object(&ctx, NULL, is_null, &obj);
            if (result != er_ok)
                break;
            result = io_encode_message(&obj, message[i]);
            if (result != er_ok)
                break;

            result = io_codec_ctx_fini(&ctx, stream, true);
            call_fini = false;
            if (result != er_ok)
                break;
        }
    } 
    while (0);

    // Flush
    if (call_fini)
        (void)io_codec_ctx_fini(&ctx, stream, true);

    for (int i = 0; i < num_messages; i++)
    {
        if (message[i])
            io_message_release(message[i]);
    }

    if (factory)
        io_message_factory_free(factory);

    return result;
}

//
// Decodes a type with a specific id
//
_ext__ int32_t io_type_decode_encode(
    io_codec_id_t codec_id,
    int32_t type,
    uint8_t* in,
    int32_t in_len,
    uint8_t* out,
    int32_t out_len
)
{
    int32_t result;
    io_fixed_buffer_stream_t mem;
    io_stream_t* stream;

    stream = io_fixed_buffer_stream_init(&mem, in, in_len, out, out_len);
    if (!stream)
        return er_fault;

    result = io_type_stream_decode_encode(codec_id, type, stream);
    io_stream_close(stream);

    log_info(NULL, "D/E of buffer returned %s.", prx_err_string(result));
    return result;
}

//
// Decodes a protocol message input buffer, then encodes it to output buffer
//
_ext__ int32_t io_message_decode_encode(
    io_codec_id_t codec_id,
    int num_messages,
    uint8_t* in,
    int32_t in_len,
    uint8_t* out,
    int32_t out_len
)
{
    int32_t result;
    io_fixed_buffer_stream_t mem;
    io_stream_t* stream;

    stream = io_fixed_buffer_stream_init(&mem, in, in_len, out, out_len);
    if (!stream)
        return er_fault;

    result = io_message_stream_decode_encode(codec_id, num_messages, stream);
    io_stream_close(stream);

    log_info(NULL, "D/E of buffer returned %s.", prx_err_string(result));
    return result;
}

//
// Decodes a protocol message from a file, then encodes it to a random file
//
_ext__ int32_t io_message_file_decode_encode(
    io_codec_id_t codec_id,
    const char* input_file,
    const char* out_file
)
{
    int32_t result;
    io_stream_t* stream;
    io_file_stream_t fs;

    stream = io_file_stream_init(&fs, input_file, out_file);
    if (!stream)
        return er_arg;
    result = io_message_stream_decode_encode(codec_id, 1, stream);
    io_stream_close(stream);

    log_info(NULL, "D/E of %s returned %s.", input_file, prx_err_string(result));
    return result;
}

//
// Entry point for fuzz harness
//
int32_t fuzz(
    const char* option,
    const char* input_file,
    const char* out_file
)
{
    static io_codec_id_t codec_id;

    /**/ if (0 == string_compare(option, "json"))
        codec_id = io_codec_json;
    else if (0 == string_compare(option, "mpack"))
        codec_id = io_codec_mpack;
    else
        return er_arg;
    return io_message_file_decode_encode(codec_id, input_file, out_file);
}
