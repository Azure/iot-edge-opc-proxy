// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "util_misc.h"
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
    result = io_decode_string_default(ctx, "name", (char**)&prx_ai->name);
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
// Encode file info
//
int32_t io_encode_prx_file_info(
    io_codec_ctx_t *ctx,
    const prx_file_info_t* prx_fi
)
{
    int32_t result;

    dbg_assert_ptr(ctx);
    dbg_assert_ptr(prx_fi);

    __io_encode_type_begin(ctx, prx_fi, 6);
    __io_encode_value(ctx, uint64, prx_fi, inode_number);
    __io_encode_value(ctx, uint64, prx_fi, device_id);
    __io_encode_value(ctx, int32, prx_fi, type);
    __io_encode_value(ctx, uint64, prx_fi, total_size);
    __io_encode_value(ctx, uint64, prx_fi, last_atime);
    __io_encode_value(ctx, uint64, prx_fi, last_mtime);
    __io_encode_type_end(ctx);
    return result;
}

//
// Decode file info
//
int32_t io_decode_prx_file_info(
    io_codec_ctx_t *ctx,
    prx_file_info_t* prx_fi
)
{
    int32_t result;

    dbg_assert_ptr(ctx);
    dbg_assert_ptr(prx_fi);

    __io_decode_type_begin(ctx, prx_fi, 6);
    __io_decode_value(ctx, uint64, prx_fi, inode_number);
    __io_decode_value(ctx, uint64, prx_fi, device_id);
    __io_decode_value(ctx, int32, prx_fi, type);
    __io_decode_value(ctx, uint64, prx_fi, total_size);
    __io_decode_value(ctx, uint64, prx_fi, last_atime);
    __io_decode_value(ctx, uint64, prx_fi, last_mtime);
    __io_decode_type_end(ctx);
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
        prx_sa, prx_sa->family == prx_address_family_inet ? 3 : 5);
    __io_encode_value(ctx, int32, prx_sa, family);
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
        __io_encode_value(ctx, uint32, prx_sa, flow);

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
    __io_decode_value(ctx, uint16, prx_sa, port);
    if (prx_sa->family == prx_address_family_inet)
    {
        size = sizeof(prx_sa->un.in4.un.u8);
        result = io_decode_bin_fixed(ctx, "addr",
            prx_sa->un.in4.un.u8, &size);
        if (result != er_ok)
            return result;

        prx_sa->flow = 0;
    }
    else
    {
        __io_decode_value(ctx, uint32, prx_sa, flow);

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
    prx_sa->_padding = 0;
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

    __io_encode_type_begin(ctx, prx_sa, 5);
    __io_encode_value(ctx, int32, prx_sa, family);
    __io_encode_value(ctx, uint16, prx_sa, port);
    __io_encode_value(ctx, uint16, prx_sa, flags);
    __io_encode_value(ctx, int32, prx_sa, itf_index);
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
    __io_decode_value(ctx, uint16, prx_sa, port);
    __io_decode_value(ctx, uint16, prx_sa, flags);
    __io_decode_value(ctx, int32, prx_sa, itf_index);
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
    dbg_assert(0, "Unexpected family %d during encoding", prx_sa->un.family);
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
    io_codec_ctx_t arr;

    dbg_assert_ptr(ctx);
    dbg_assert_ptr(prx_sp);

    __io_encode_type_begin(ctx, prx_sp, 7);
    __io_encode_value(ctx, int32, prx_sp, family);
    __io_encode_value(ctx, int32, prx_sp, sock_type);
    __io_encode_value(ctx, int32, prx_sp, proto_type);
    __io_encode_value(ctx, uint32, prx_sp, flags);
    __io_encode_value(ctx, uint64, prx_sp, timeout);
    __io_encode_object(ctx, prx_socket_address, prx_sp, address);

    result = io_encode_array(ctx, "options", prx_sp->options_len, &arr);
    if (result != er_ok)
        return result;
    for (size_t i = 0; i < prx_sp->options_len; i++)
    {
        io_codec_ctx_t obj;
        result = io_encode_object(&arr, NULL, false, &obj);
        if (result != er_ok)
            break;
        result = io_encode_prx_property(&obj, &prx_sp->options[i]);
        if (result != er_ok)
            break;
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
    io_codec_ctx_t arr;
    size_t allocated;
    
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(prx_sp);

    __io_decode_type_begin(ctx, prx_sp, 7);
    __io_decode_value(ctx, int32, prx_sp, family);
    __io_decode_value(ctx, int32, prx_sp, sock_type);
    __io_decode_value(ctx, int32, prx_sp, proto_type);
    __io_decode_value(ctx, uint32, prx_sp, flags);
    __io_decode_value(ctx, uint64, prx_sp, timeout);
    __io_decode_object(ctx, prx_socket_address, prx_sp, address);

    prx_sp->options = NULL;
    result = io_decode_array(ctx, "options", &prx_sp->options_len, &arr);
    if (result != er_ok)
        return result;
    if (prx_sp->options_len > 0)
    {
        if (!ctx->default_allocator)
        {
            prx_sp->options = (prx_property_t*)mem_alloc(prx_sp->options_len *
                sizeof(prx_property_t));
            if (!prx_sp->options)
                return er_out_of_memory;
        }
        else
        {
            allocated = 0;
            result = ctx->default_allocator(ctx, prx_sp->options_len *
                sizeof(prx_property_t), (void**)&prx_sp->options, &allocated);
            if (result != er_ok)
                return result;
        }
    }
    for (size_t i = 0; i < prx_sp->options_len; i++)
    {
        io_codec_ctx_t obj;
        result = io_decode_object(&arr, NULL, NULL, &obj);
        if (result != er_ok)
            break;
        result = io_decode_prx_property(&obj, &prx_sp->options[i]);
        if (result != er_ok)
            break;
    }
    if (result != er_ok)
        return result;
    __io_decode_type_end(ctx);
    return result;
}

//
// Encode ip4 socket address
//
static int32_t io_encode_prx_socket_address_inet4(
    io_codec_ctx_t *ctx,
    const prx_socket_address_inet_t *prx_sa
)
{
    int32_t result;
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(prx_sa);

    __io_encode_type_begin(ctx, prx_sa, 3);
    __io_encode_value(ctx, int32, prx_sa, family);
    __io_encode_value(ctx, uint32, prx_sa, flow);
    __io_encode_value(ctx, uint16, prx_sa, port);

    result = io_encode_bin(ctx, "addr",
        prx_sa->un.in4.un.u8, sizeof(prx_sa->un.in4.un.u8));
    if (result != er_ok)
        return result;
    __io_encode_type_end(ctx);
    return result;
}

//
// Encode ip4 socket address
//
static int32_t io_decode_prx_socket_address_inet4(
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

    size = sizeof(prx_sa->un.in4.un.u8);
    result = io_decode_bin_fixed(ctx, "addr",
        prx_sa->un.in4.un.u8, &size);
    if (result != er_ok)
        return result;
    __io_decode_type_end(ctx);
    return result;
}

//
// Encode ip6 socket address
//
static int32_t io_encode_prx_socket_address_inet6(
    io_codec_ctx_t *ctx,
    const prx_socket_address_inet_t *prx_sa
)
{
    int32_t result;
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(prx_sa);

    __io_encode_type_begin(ctx, prx_sa, 5);
    __io_encode_value(ctx, int32, prx_sa, family);
    __io_encode_value(ctx, uint32, prx_sa, flow);
    __io_encode_value(ctx, uint16, prx_sa, port);

    result = io_encode_bin(ctx, "addr",
        prx_sa->un.in6.un.u8, sizeof(prx_sa->un.in6.un.u8));
    if (result != er_ok)
        return result;
    result = io_encode_uint32(ctx, "scope_id",
        prx_sa->un.in6.scope_id);
    if (result != er_ok)
        return result;
    __io_encode_type_end(ctx);
    return result;
}

//
// Encode multicast option
//
int32_t io_encode_prx_multicast_option(
    io_codec_ctx_t *ctx,
    const prx_multicast_option_t *prx_mo
)
{
    int32_t result;
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(prx_mo);

    __io_encode_type_begin(ctx, prx_mo, 3); 
    __io_encode_value(ctx, int32, prx_mo, family);
    __io_encode_value(ctx, int32, prx_mo, itf_index);
    switch (prx_mo->family)
    {
    case prx_address_family_inet:
        result = io_encode_bin(ctx, "addr", prx_mo->addr.in4.un.u8,
            sizeof(prx_mo->addr.in4.un.u8));
        break;
    case prx_address_family_inet6:
        result = io_encode_bin(ctx, "addr", prx_mo->addr.in6.un.u8,
            sizeof(prx_mo->addr.in6.un.u8));
        break;
    default:
        dbg_assert(0, "Unexpected family %d during encoding", prx_mo->family);
        return er_invalid_format;
    }
    if (result != er_ok)
        return result;
    __io_encode_type_end(ctx);
    return result;
}

//
// Decode multicast option
//
int32_t io_decode_prx_multicast_option(
    io_codec_ctx_t *ctx,
    prx_multicast_option_t *prx_mo
)
{
    int32_t result;
    size_t size;
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(prx_mo);

    __io_decode_type_begin(ctx, prx_mo, 3);
    __io_decode_value(ctx, int32, prx_mo, family);
    __io_decode_value(ctx, int32, prx_mo, itf_index);
    switch (prx_mo->family)
    {
    case prx_address_family_inet:
        size = sizeof(prx_mo->addr.in4.un.u8);
        result = io_decode_bin_fixed(ctx, "addr", prx_mo->addr.in4.un.u8, &size);
        if (result != er_ok)
            return result;
        break;
    case prx_address_family_inet6:
        size = sizeof(prx_mo->addr.in6.un.u8);
        result = io_decode_bin_fixed(ctx, "addr", prx_mo->addr.in6.un.u8, &size);
        if (result != er_ok)
            return result;
        prx_mo->addr.in6.scope_id = prx_mo->itf_index;
        break;
    default:
        dbg_assert(0, "Unexpected family %d during encoding", prx_mo->family);
        return er_invalid_format;
    }
    __io_encode_type_end(ctx);
    return result;
}

//
// Encode a socket option value
//
int32_t io_encode_prx_property(
    io_codec_ctx_t* ctx,
    const prx_property_t* prx_prop
)
{
    int32_t result;
    io_codec_ctx_t obj;
    __io_encode_type_begin(ctx, prx_prop, 2);
    __io_encode_value(ctx, int32, prx_prop, type);
    /**/ if (prx_prop->type == prx_so_ip_multicast_join ||
             prx_prop->type == prx_so_ip_multicast_leave ||
             prx_prop->type >= prx_property_type_file_info)
    {
        result = io_encode_object(ctx, "property", false, &obj);
        if (result != er_ok)
            return result;
        switch (prx_prop->type)
        {
        case prx_so_ip_multicast_join:
        case prx_so_ip_multicast_leave:
            result = io_encode_prx_multicast_option(&obj, &prx_prop->property.mcast);
            break;
        case prx_property_type_addrinfo:
            result = io_encode_prx_addrinfo(&obj, &prx_prop->property.addr_info);
            break;
        case prx_property_type_ifaddrinfo:
            result = io_encode_prx_ifaddrinfo(&obj, &prx_prop->property.itf_info);
            break;
        case prx_property_type_file_info:
            result = io_encode_prx_file_info(&obj, &prx_prop->property.file_info);
            break;
        default:
            result = er_writing;
            break;
        }
    }
    else if (prx_prop->type >= prx_record_type_default &&
             prx_prop->type < __prx_record_max)
    {
        result = io_encode_bin(ctx, "property", prx_prop->property.bin.value, 
            prx_prop->property.bin.size);
    }
    else if (prx_prop->type > prx_so_unknown &&
             prx_prop->type < __prx_so_max)
    {
        result = io_encode_uint64(ctx, "property", prx_prop->property.value);
    }
    else
    {
        result = er_writing;
    }
    if (result != er_ok)
        return result;
    __io_encode_type_end(ctx);
    return result;
}

//
// Decode a property value
//
int32_t io_decode_prx_property(
    io_codec_ctx_t* ctx,
    prx_property_t* prx_prop
)
{
    int32_t result;
    bool is_null;
    io_codec_ctx_t obj;
    __io_decode_type_begin(ctx, prx_prop, 2);
    __io_decode_value(ctx, int32, prx_prop, type);
    /**/ if (prx_prop->type == prx_so_ip_multicast_join ||
             prx_prop->type == prx_so_ip_multicast_leave ||
             prx_prop->type >= prx_property_type_file_info)
    {
        result = io_decode_object(ctx, "property", &is_null, &obj);
        if (result != er_ok)
            return result;
        if (is_null)
            return er_invalid_format;
        switch (prx_prop->type)
        {
        case prx_so_ip_multicast_join:
        case prx_so_ip_multicast_leave:
            result = io_decode_prx_multicast_option(&obj, &prx_prop->property.mcast);
            break;
        case prx_property_type_addrinfo:
            result = io_decode_prx_addrinfo(&obj, &prx_prop->property.addr_info);
            break;
        case prx_property_type_ifaddrinfo:
            result = io_decode_prx_ifaddrinfo(&obj, &prx_prop->property.itf_info);
            break;
        case prx_property_type_file_info:
            result = io_decode_prx_file_info(&obj, &prx_prop->property.file_info);
            break;
        default:
            result = er_invalid_format;
            break;
        }
    }
    else if (prx_prop->type >= prx_record_type_default && 
             prx_prop->type < __prx_record_max)
    {
        result = io_decode_bin_default(ctx, "property", 
            (void**)&prx_prop->property.bin.value, &prx_prop->property.bin.size);
    }
    else if (prx_prop->type > prx_so_unknown &&
             prx_prop->type < __prx_so_max)
    {
        result = io_decode_uint64(ctx, "property", &prx_prop->property.value);
    }
    else
    {
        result = er_invalid_format;
    }
    if (result != er_ok)
        return result;
    __io_decode_type_end(ctx);
    return result;
}

