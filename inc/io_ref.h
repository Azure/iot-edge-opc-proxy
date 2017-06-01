// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _io_ref_h_
#define _io_ref_h_

#include "common.h"
#include "prx_types.h"
#include "azure_c_shared_utility/strings.h"
#include "io_codec.h"

//
// 128 bit guid acting as a remoting reference
//
typedef struct io_ref
{
    union
    {
        uint64_t u64[2];
        uint32_t u32[4];
        uint8_t  u8[16];
    }
    un;
}
io_ref_t;

// 
// reference constants for comparison and copy
//
decl_g(io_ref_t, io_ref_null,  {  0ULL,  0ULL });
decl_g(io_ref_t, io_ref_bcast, { ~0ULL, ~0ULL });

// 
// Assign new reference
//
decl_internal_1(int32_t, io_ref_new,
    io_ref_t*, ref
);

// 
// Clear reference
//
decl_internal_1(void, io_ref_clear,
    io_ref_t*, ref
);

//
// Convert reference from string
//
decl_internal_2(int32_t, io_ref_from_string,
    const char*, string,
    io_ref_t*, ref
);

//
// Convert reference from socket address
//
decl_internal_2(int32_t, io_ref_from_prx_socket_address,
    const prx_socket_address_t*, sa,
    io_ref_t*, ref
);

//
// Convert reference to socket address
//
decl_internal_2(int32_t, io_ref_to_prx_socket_address,
    const io_ref_t*, ref,
    prx_socket_address_t*, sa
);

//
// Copy reference to string buffer
//
decl_internal_3(int32_t, io_ref_to_string,
    const io_ref_t*, ref,
    char*, buffer,
    size_t, buf_len
);

//
// Convert reference to reference string
//
decl_internal_1(STRING_HANDLE, io_ref_to_STRING,
    const io_ref_t*, ref
);

//
// Append to string handle
//
decl_internal_2(int32_t, io_ref_append_to_STRING,
    const io_ref_t*, ref,
    STRING_HANDLE, string
);

//
// Copyies one reference to another
//
decl_internal_2(void, io_ref_copy,
    const io_ref_t*, src,
    io_ref_t*, dst
);

//
// Swaps two references
//
decl_internal_2(void, io_ref_swap,
    io_ref_t*, ref1,
    io_ref_t*, ref2
);

//
// Returns whether 2 references are equal
//
decl_internal_2(bool, io_ref_equals,
    const io_ref_t*, ref1,
    const io_ref_t*, ref2
);

//
// Hash reference
//
decl_internal_1(uint32_t, io_ref_hash,
    const io_ref_t*, ref
);

//
// Encode the reference
//
decl_internal_2(int32_t, io_encode_ref,
    io_codec_ctx_t*, ctx,
    const io_ref_t*, ref
);

//
// Decode the reference
//
decl_internal_2(int32_t, io_decode_ref,
    io_codec_ctx_t*, ctx,
    io_ref_t*, ref
);

#if defined(IO_REF_LONG)
#define IO_REF_FMT "%08x%08x%08x%08x"
#define IO_REF_PRINT(a) (a)->un.u32[0], (a)->un.u32[1], (a)->un.u32[2], (a)->un.u32[3]
#else
#define IO_REF_FMT "%08x"
#define IO_REF_PRINT(a) (a)->un.u32[3]
#endif

#endif // _io_ref_h_