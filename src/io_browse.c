// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "util_misc.h"
#include "io_types.h"
#include "io_browse.h"

//
// Encode browse request
//
int32_t io_encode_browse_request(
    io_codec_ctx_t* ctx,
    const io_browse_request_t* prx_br
)
{
    int32_t result;

    dbg_assert_ptr(ctx);
    dbg_assert_ptr(prx_br);

    __io_encode_type_begin(ctx, prx_br, 5);
    __io_encode_object(ctx, ref, prx_br, handle);
    __io_encode_value(ctx, uint8, prx_br, version);

    if (prx_br->version > PRX_BROWSE_VERSION)
        return er_invalid_format;

    __io_encode_value(ctx, int32, prx_br, type);
    __io_encode_value(ctx, uint32, prx_br, flags);
    __io_encode_object(ctx, prx_socket_address, prx_br, item);
    __io_encode_type_end(ctx);
    return result;
}

//
// Decode browse request
//
int32_t io_decode_browse_request(
    io_codec_ctx_t* ctx,
    io_browse_request_t* prx_br
)
{
    int32_t result;

    dbg_assert_ptr(ctx);
    dbg_assert_ptr(prx_br);

    __io_decode_type_begin(ctx, prx_br, 5);
    __io_decode_object(ctx, ref, prx_br, handle);
    __io_decode_value(ctx, uint8, prx_br, version);
    __io_decode_value(ctx, int32, prx_br, type);
    __io_decode_value(ctx, uint32, prx_br, flags);
    __io_decode_object(ctx, prx_socket_address, prx_br, item);
    __io_decode_type_end(ctx);
    return result;
}

//
// Encode browse response
//
int32_t io_encode_browse_response(
    io_codec_ctx_t* ctx,
    const io_browse_response_t* prx_br
)
{
    int32_t result;
    io_codec_ctx_t arr;

    dbg_assert_ptr(ctx);
    dbg_assert_ptr(prx_br);

    __io_encode_type_begin(ctx, prx_br, 5);
    __io_encode_object(ctx, ref, prx_br, handle);
    __io_encode_value(ctx, uint32, prx_br, flags);
    __io_encode_value(ctx, int32, prx_br, error_code);
    __io_encode_object(ctx, prx_socket_address, prx_br, item);

    result = io_encode_array(ctx, "props", prx_br->props_size, &arr);
    if (result != er_ok)
        return result;

    for (size_t i = 0; i < prx_br->props_size; i++)
    {
        io_codec_ctx_t obj;
        result = io_encode_object(&arr, NULL, false, &obj);
        if (result != er_ok)
            break;
        result = io_encode_prx_property(&obj, &prx_br->props[i]);
        if (result != er_ok)
            break;
    }
    if (result != er_ok)
        return result;

    __io_encode_type_end(ctx);
    return result;
}

//
// Decode browse response
//
int32_t io_decode_browse_response(
    io_codec_ctx_t* ctx,
    io_browse_response_t* prx_br
)
{
    int32_t result;
    io_codec_ctx_t arr;
    size_t size;

    dbg_assert_ptr(ctx);
    dbg_assert_ptr(prx_br);

    __io_decode_type_begin(ctx, prx_br, 5);
    __io_decode_object(ctx, ref, prx_br, handle);
    __io_decode_value(ctx, uint32, prx_br, flags);
    __io_decode_value(ctx, int32, prx_br, error_code);
    __io_decode_object(ctx, prx_socket_address, prx_br, item);

    result = io_decode_array(ctx, "props", &prx_br->props_size, &arr);
    if (result != er_ok)
        return result;
    if (!prx_br->props_size)
        prx_br->props = NULL;
    else
    {
        size = (prx_br->props_size + 1) * sizeof(prx_property_t);
        if (ctx->default_allocator) // Use default allocator if set
            result = ctx->default_allocator(ctx, size, (void**)&prx_br->props, &size);
        else
        {
            // Otherwise allocate with malloc - correponds to prx_client_freeaddrinfo
            prx_br->props = (prx_property_t*)mem_zalloc(size);
            if (!prx_br->props)
                result = er_out_of_memory;
        }
        if (result != er_ok)
            return result;

        for (size_t i = 0; i < prx_br->props_size; i++)
        {
            io_codec_ctx_t obj;
            result = io_decode_object(&arr, NULL, NULL, &obj);
            if (result != er_ok)
                break;
            result = io_decode_prx_property(&obj, &prx_br->props[i]);
            if (result != er_ok)
                break;
        }

        if (result != er_ok)
        {
            // Free memory...
            if (ctx->default_allocator)
                ctx->default_allocator(ctx, 0, (void**)&prx_br->props, &size);
            else
                mem_free(prx_br->props);
            prx_br->props = NULL;
            return result;
        }
    }
    __io_decode_type_end(ctx);
    return result;
}
