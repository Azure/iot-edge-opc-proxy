// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "io_types.h"

//
// Encode address info
//
int32_t io_encode_prx_addrinfo(
    io_codec_ctx_t *ctx,
    const prx_addrinfo_t* prx_ai
)
{
    int32_t result;

    dbg_assert_ptr(ctx);
    dbg_assert_ptr(prx_ai);

    __io_encode_type_begin(ctx, prx_ai, 2);
    __io_encode_object(ctx, prx_socket_address, prx_ai, address);
    result = io_encode_string(ctx, "name", prx_ai->name ? prx_ai->name : "");
    if (result != er_ok)
        return result;
    __io_encode_type_end(ctx);
    return result;
}

//
// Decode address info
//
int32_t io_decode_prx_addrinfo(
    io_codec_ctx_t *ctx,
    prx_addrinfo_t* prx_ai
)
{
    int32_t result;
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(prx_ai);

    __io_decode_type_begin(ctx, prx_ai, 2);
    __io_decode_object(ctx, prx_socket_address, prx_ai, address);
    result = io_decode_string_default(ctx, "name", &prx_ai->name);
    if (result != er_ok)
        return result;
    __io_decode_type_end(ctx);

    prx_ai->reserved = 0;
    return result;
}

//
// Encode interface address info
//
int32_t io_encode_prx_ifaddrinfo(
    io_codec_ctx_t *ctx,
    const prx_ifaddrinfo_t* prx_ifa
)
{
    int32_t result;

    dbg_assert_ptr(ctx);
    dbg_assert_ptr(prx_ifa);

    __io_encode_type_begin(ctx, prx_ifa, 6);
    __io_encode_object(ctx, prx_socket_address, prx_ifa, address);
    __io_encode_value(ctx, uint8, prx_ifa, prefix);
    __io_encode_value(ctx, uint8, prx_ifa, flags);
    result = io_encode_string(ctx, "name", prx_ifa->name);
    if (result != er_ok)
        return result;
    __io_encode_value(ctx, int32, prx_ifa, index);
    __io_encode_object(ctx, prx_socket_address, prx_ifa, broadcast_addr);
    __io_encode_type_end(ctx);
    return result;
}

//
// Decode interface address info
//
int32_t io_decode_prx_ifaddrinfo(
    io_codec_ctx_t *ctx,
    prx_ifaddrinfo_t* prx_ifa
)
{
    int32_t result;

    dbg_assert_ptr(ctx);
    dbg_assert_ptr(prx_ifa);

    __io_decode_type_begin(ctx, prx_ifa, 6);
    __io_decode_object(ctx, prx_socket_address, prx_ifa, address);
    __io_decode_value(ctx, uint8, prx_ifa, prefix);
    __io_decode_value(ctx, uint8, prx_ifa, flags);
    result = io_decode_string_fixed(
        ctx, "name", prx_ifa->name, sizeof(prx_ifa->name));
    if (result != er_ok)
        return result;
    __io_decode_value(ctx, int32, prx_ifa, index);
    __io_decode_object(ctx, prx_socket_address, prx_ifa, broadcast_addr);
    __io_decode_type_end(ctx);

    prx_ifa->reserved = 0;
    return result;
}

//
// Encode ip socket address
//
static int32_t io_encode_prx_socket_address_inet(
    io_codec_ctx_t *ctx,
    const prx_socket_address_inet_t *prx_sa
)
{
    int32_t result;
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(prx_sa);

    __io_encode_type_begin(ctx, 
        prx_sa, prx_sa->family == prx_address_family_inet ? 4 : 5);
    __io_encode_value(ctx, int32, prx_sa, family);
    __io_encode_value(ctx, uint32, prx_sa, flow);
    __io_encode_value(ctx, uint16, prx_sa, port);
    if (prx_sa->family == prx_address_family_inet)
    {
        result = io_encode_bin(ctx, "addr", 
            prx_sa->un.in4.un.u8, sizeof(prx_sa->un.in4.un.u8));
        if (result != er_ok)
            return result;
    }
    else
    {
        result = io_encode_bin(ctx, "addr", 
            prx_sa->un.in6.un.u8, sizeof(prx_sa->un.in6.un.u8));
        if (result != er_ok)
            return result;
        result = io_encode_uint32(ctx, "scope_id", 
            prx_sa->un.in6.scope_id);
        if (result != er_ok)
            return result;
    }
    __io_encode_type_end(ctx);
    return result;
}

//
// Encode ip socket address
//
static int32_t io_decode_prx_socket_address_inet(
    io_codec_ctx_t *ctx,
    prx_socket_address_inet_t *prx_sa
)
{
    int32_t result;
    size_t size;
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(prx_sa);

    // header and family was already decoded
    __io_decode_value(ctx, uint32, prx_sa, flow);
    __io_decode_value(ctx, uint16, prx_sa, port);
    if (prx_sa->family == prx_address_family_inet)
    {
        size = sizeof(prx_sa->un.in4.un.u8);
        result = io_decode_bin_fixed(ctx, "addr", 
            prx_sa->un.in4.un.u8, &size);
        if (result != er_ok)
            return result;
    }
    else
    {
        size = sizeof(prx_sa->un.in6.un.u8);
        result = io_decode_bin_fixed(ctx, "addr", 
            prx_sa->un.in6.un.u8, &size);
        if (result != er_ok)
            return result;
        result = io_decode_uint32(ctx, "scope_id",
            &prx_sa->un.in6.scope_id);
        if (result != er_ok)
            return result;
    }
    __io_decode_type_end(ctx);
    return result;
}

//
// Encode proxy socket address
//
static int32_t io_encode_prx_socket_address_proxy(
    io_codec_ctx_t *ctx,
    const prx_socket_address_proxy_t *prx_sa
)
{
    int32_t result;
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(prx_sa);

    __io_encode_type_begin(ctx, prx_sa, 4);
    __io_encode_value(ctx, int32, prx_sa, family);
    __io_encode_value(ctx, uint32, prx_sa, flow);
    __io_encode_value(ctx, uint16, prx_sa, port);
    result = io_encode_string(
        ctx, "host", prx_sa->host);
    if (result != er_ok)
        return result;
    __io_encode_type_end(ctx);
    return result;
}

//
// Decode proxy socket address
//
static int32_t io_decode_prx_socket_address_proxy(
    io_codec_ctx_t *ctx,
    prx_socket_address_proxy_t *prx_sa
)
{
    int32_t result;
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(prx_sa);

    // header and family was already decoded
    __io_decode_value(ctx, uint32, prx_sa, flow);
    __io_decode_value(ctx, uint16, prx_sa, port);
    result = io_decode_string_fixed(
        ctx, "host", prx_sa->host, sizeof(prx_sa->host));
    if (result != er_ok)
        return result;
    __io_decode_type_end(ctx);
    return result;
}

//
// Encode unix socket address
//
static int32_t io_encode_prx_socket_address_unix(
    io_codec_ctx_t *ctx,
    const prx_socket_address_unix_t *prx_sa
)
{
    int32_t result;
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(prx_sa);

    __io_encode_type_begin(ctx, prx_sa, 2);
    __io_encode_value(ctx, int32, prx_sa, family);
    result = io_encode_string(
        ctx, "path", prx_sa->path);
    if (result != er_ok)
        return result;
    __io_encode_type_end(ctx);
    return result;
}

//
// Decode unix socket address
//
static int32_t io_decode_prx_socket_address_unix(
    io_codec_ctx_t *ctx,
    prx_socket_address_unix_t *prx_sa
)
{
    int32_t result;
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(prx_sa);

    // header and family was already decoded
    result = io_decode_string_fixed(
        ctx, "path", prx_sa->path, sizeof(prx_sa->path));
    if (result != er_ok)
        return result;
    __io_decode_type_end(ctx);
    return result;
}

//
// Encode socket address
//
int32_t io_encode_prx_socket_address(
    io_codec_ctx_t *ctx,
    const prx_socket_address_t *prx_sa
)
{
    int32_t result;
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(prx_sa);

    switch (prx_sa->un.family)
    {
    case prx_address_family_unspec:
        __io_encode_type_begin(ctx, prx_sa, 1);
        result = io_encode_int32(ctx, "family", prx_sa->un.family);
        if (result != er_ok)
            return result;
        __io_encode_type_end(ctx);
        return er_ok;
    case prx_address_family_inet:
    case prx_address_family_inet6:
        return io_encode_prx_socket_address_inet(ctx, &prx_sa->un.ip);
    case prx_address_family_proxy:
        return io_encode_prx_socket_address_proxy(ctx, &prx_sa->un.proxy);
    case prx_address_family_unix:
        return io_encode_prx_socket_address_unix(ctx, &prx_sa->un.ux);
    }
    return er_arg;
}

//
// Decode socket address
//
int32_t io_decode_prx_socket_address(
    io_codec_ctx_t *ctx,
    prx_socket_address_t *prx_sa
)
{
    int32_t result;

    dbg_assert_ptr(ctx);
    dbg_assert_ptr(prx_sa);

    __io_decode_type_begin(ctx, prx_sa, 1); // at least one property
    result = io_decode_int32(ctx, "family", (int32_t*)&prx_sa->un.family);
    if (result != er_ok)
        return result;
    switch (prx_sa->un.family)
    {
    case prx_address_family_unspec:
        __io_decode_type_end(ctx);
        return er_ok;
    case prx_address_family_inet:
    case prx_address_family_inet6:
        return io_decode_prx_socket_address_inet(ctx, &prx_sa->un.ip);
    case prx_address_family_proxy:
        return io_decode_prx_socket_address_proxy(ctx, &prx_sa->un.proxy);
    case prx_address_family_unix:
        return io_decode_prx_socket_address_unix(ctx, &prx_sa->un.ux);
    }
    dbg_assert(0, "Unknown address family decoded (%d)", prx_sa->un.family);
    return er_invalid_format;
}

//
// Encode socket properties
//
int32_t io_encode_prx_socket_properties(
    io_codec_ctx_t *ctx,
    const prx_socket_properties_t *prx_sp
)
{
    int32_t result;
    size_t size = 0;
    io_codec_ctx_t arr;

    dbg_assert_ptr(ctx);
    dbg_assert_ptr(prx_sp);

    __io_encode_type_begin(ctx, prx_sp, 6);
    __io_encode_value(ctx, int32, prx_sp, family);
    __io_encode_value(ctx, int32, prx_sp, sock_type);
    __io_encode_value(ctx, int32, prx_sp, proto_type);
    __io_encode_value(ctx, uint32, prx_sp, flags);
    __io_encode_object(ctx, prx_socket_address, prx_sp, address);

    for (size_t i = 0; i < _countof(prx_sp->options); i++)
    {
        if (prx_sp->options[i].option != prx_so_unknown)
            size++;
    }

    result = io_encode_array(ctx, "options", size, &arr);
    if (result != er_ok)
        return result;
    for (size_t i = 0; i < _countof(prx_sp->options); i++)
    {
        if (prx_sp->options[i].option != prx_so_unknown)
        {
            io_codec_ctx_t obj;
            result = io_encode_object(&arr, NULL, false, &obj);
            if (result != er_ok)
                break;
            result = io_encode_prx_socket_option_value(&obj, &prx_sp->options[i]);
            if (result != er_ok)
                break;
        }
    }
    if (result != er_ok)
        return result;
    __io_encode_type_end(ctx);
    return result;
}

//
// Decode socket properties
//
int32_t io_decode_prx_socket_properties(
    io_codec_ctx_t *ctx,
    prx_socket_properties_t *prx_sp
)
{
    int32_t result;
    size_t size;
    prx_socket_option_value_t option;
    io_codec_ctx_t arr;
    
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(prx_sp);

    __io_decode_type_begin(ctx, prx_sp, 6);
    __io_decode_value(ctx, int32, prx_sp, family);
    __io_decode_value(ctx, int32, prx_sp, sock_type);
    __io_decode_value(ctx, int32, prx_sp, proto_type);
    __io_decode_value(ctx, uint32, prx_sp, flags);
    __io_decode_object(ctx, prx_socket_address, prx_sp, address);

    result = io_decode_array(ctx, "options", &size, &arr);
    if (result != er_ok)
        return result;

    memset(prx_sp->options, 0, sizeof(prx_sp->options));
    for (size_t i = 0; i < size; i++)
    {
        io_codec_ctx_t obj;
        result = io_decode_object(&arr, NULL, NULL, &obj);
        if (result != er_ok)
            break;
        option.option = prx_so_unknown;
        result = io_decode_prx_socket_option_value(&obj, &option);
        if (result != er_ok)
            break;

        if (option.option > prx_so_unknown && option.option < __prx_so_max)
        {
            prx_sp->options[option.option].option = option.option;
            prx_sp->options[option.option].value = option.value;
        }
    }
    if (result != er_ok)
        return result;
    __io_decode_type_end(ctx);
    return result;
}

//
// Encode a socket option value
//
int32_t io_encode_prx_socket_option_value(
    io_codec_ctx_t* ctx,
    const prx_socket_option_value_t* prx_so_val
)
{
    int32_t result;
    __io_encode_type_begin(ctx, prx_so_val, 2);
    __io_encode_value(ctx, int32, prx_so_val, option);
    __io_encode_value(ctx, uint64, prx_so_val, value);
    __io_encode_type_end(ctx);
    return result;
}

//
// Decode a socket option value
//
int32_t io_decode_prx_socket_option_value(
    io_codec_ctx_t* ctx,
    prx_socket_option_value_t* prx_so_val
)
{
    int32_t result;
    __io_decode_type_begin(ctx, prx_so_val, 2);
    __io_decode_value(ctx, int32, prx_so_val, option);
    __io_decode_value(ctx, uint64, prx_so_val, value);
    __io_decode_type_end(ctx);
    return result;
}
