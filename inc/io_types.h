// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _io_types_h_
#define _io_types_h_

#include "common.h"
#include "prx_types.h"
#include "io_codec.h"

//
// Encode address info
//
decl_public_2(int32_t, io_encode_prx_addrinfo,
    io_codec_ctx_t*, ctx,
    const prx_addrinfo_t*, prx_ai
);

//
// Decode address info
//
decl_public_2(int32_t, io_decode_prx_addrinfo,
    io_codec_ctx_t*, ctx,
    prx_addrinfo_t*, prx_ai
);

//
// Encode interface address info
//
decl_public_2(int32_t, io_encode_prx_ifaddrinfo,
    io_codec_ctx_t*, ctx,
    const prx_ifaddrinfo_t*, prx_ifa
);

//
// Decode interface address info
//
decl_public_2(int32_t, io_decode_prx_ifaddrinfo,
    io_codec_ctx_t*, ctx,
    prx_ifaddrinfo_t*, prx_ifa
);

//
// Encode socket address
//
decl_public_2(int32_t, io_encode_prx_socket_address,
    io_codec_ctx_t*, ctx,
    const prx_socket_address_t*, prx_address
);

//
// Decode socket address
//
decl_public_2(int32_t, io_decode_prx_socket_address,
    io_codec_ctx_t*, ctx,
    prx_socket_address_t*, prx_address
);

//
// Encode socket properties
//
decl_public_2(int32_t, io_encode_prx_socket_properties,
    io_codec_ctx_t*, ctx,
    const prx_socket_properties_t*, prx_sp
);

//
// Decode socket properties
//
decl_public_2(int32_t, io_decode_prx_socket_properties,
    io_codec_ctx_t*, ctx,
    prx_socket_properties_t*, prx_sp
);

//
// Encode a socket option value
//
decl_public_2(int32_t, io_encode_prx_socket_option_value,
    io_codec_ctx_t*, ctx,
    const prx_socket_option_value_t*, prx_so_val
);

//
// Decode a socket option value
//
decl_public_2(int32_t, io_decode_prx_socket_option_value,
    io_codec_ctx_t*, ctx,
    prx_socket_option_value_t*, prx_so_val
);


#endif // _io_types_h_