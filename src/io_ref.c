// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#define ENABLE_GLOBAL
#include "io_ref.h"
#undef ENABLE_GLOBAL
#include "pal_net.h"
#include "pal_rand.h"
#include "util_string.h"

// 
// Assign new ref
//
int32_t io_ref_new(
    io_ref_t* ref
)
{
    chk_arg_fault_return(ref);
    return pal_rand_fill(ref->un.u8, sizeof(ref->un.u8));
}

// 
// Clear ref
//
void io_ref_clear(
    io_ref_t* ref
)
{
    if (!ref)
        return;
    ref->un.u64[0] = 0;
    ref->un.u64[1] = 0;
}

//
// Copy ref to string buffer
//
int32_t io_ref_to_string(
    io_ref_t* ref,
    char* string,
    size_t len
)
{
    chk_arg_fault_return(ref);
    return string_from_uuid(ref->un.u8, string, len);
}

//
// Convert ref from ref string
//
int32_t io_ref_from_string(
    const char* string,
    io_ref_t* ref
)
{
    int32_t result;
    prx_socket_address_t sa;
    chk_arg_fault_return(string);
    chk_arg_fault_return(ref);

    // Trim any curly braces for convinience
    if (string[0] == '{')
    {
        char copy[128];
        strcpy(copy, string + 1);
        string_trim_back(copy, "}");
        return io_ref_from_string(copy, ref);
    }

    result = string_to_uuid(string, ref->un.u8);
    if (result == er_ok)
        return result;
    
    result = pal_pton(string, &sa);
    if (result == er_ok)
    {
        result = io_ref_from_prx_socket_address(&sa, ref);
        if (result == er_ok)
            return result;
    }

    log_error(NULL, "%s is not an ref string (%s).",
        string, prx_err_string(result));
    return result;
}

//
// Convert ref to ref string
//
STRING_HANDLE io_ref_to_STRING(
    io_ref_t* ref
)
{
    STRING_HANDLE result;
    result = STRING_new();
    if (!result)
        return NULL;
    if (er_ok == io_ref_append_to_STRING(ref, result))
        return result;
    STRING_delete(result);
    return NULL;
}

//
// Append to string handle
//
int32_t io_ref_append_to_STRING(
    io_ref_t* ref,
    STRING_HANDLE string
)
{
    int32_t result;
    char tmp[UUID_PRINTABLE_STRING_LENGTH];
    tmp[0] = 0;
    
    chk_arg_fault_return(ref);
    chk_arg_fault_return(string);

    result = io_ref_to_string(ref, tmp, sizeof(tmp));
    if (result != er_ok)
        return result;
    result = STRING_concat(string, tmp);
    if (result != 0)
        return er_out_of_memory;
    return er_ok;
}

//
// Returns whether 2 refes are equal
//
bool io_ref_equals(
    io_ref_t* ref1,
    io_ref_t* ref2
)
{
    if (!ref1 && !ref2)
        return true;
    if (!ref1 || !ref2)
        return false;
    return
        ref1->un.u64[0] == ref2->un.u64[0] &&
        ref1->un.u64[1] == ref2->un.u64[1];
}

//
// Copyies one ref to another
//
void io_ref_copy(
    io_ref_t* src,
    io_ref_t* dst
)
{
    if (!src || !dst)
        return;

    dst->un.u64[0] = src->un.u64[0];
    dst->un.u64[1] = src->un.u64[1];
}

//
// Swaps two refes
//
void io_ref_swap(
    io_ref_t* ref1,
    io_ref_t* ref2
)
{
    uint64_t tmp;

    if (!ref1 || !ref2)
        return;

    tmp = ref1->un.u64[0];
    ref1->un.u64[0] = ref2->un.u64[0];
    ref2->un.u64[0] = tmp;

    tmp = ref1->un.u64[1];
    ref1->un.u64[1] = ref2->un.u64[1];
    ref2->un.u64[1] = tmp;
}

//
// Hash ref
//
uint32_t io_ref_hash(
    io_ref_t *ref
)
{
    return 
        ((ref->un.u32[0] ^      ( ref->un.u32[1] << 16 | 
         ref->un.u32[2] << 24)) | ref->un.u32[3]);
}

//
// Convert ref from socket ref
//
int32_t io_ref_from_prx_socket_address(
    prx_socket_address_t* sa,
    io_ref_t* ref
)
{
    chk_arg_fault_return(ref);
    chk_arg_fault_return(sa);

    dbg_assert(sa->un.family == prx_address_family_inet6, "");

    ref->un.u64[0] = sa->un.ip.un.in6.un.u64[0];
    ref->un.u64[1] = sa->un.ip.un.in6.un.u64[1];

    return er_ok;
}

//
// Convert ref to socket ref
//
int32_t io_ref_to_prx_socket_address(
    io_ref_t* ref,
    prx_socket_address_t* sa
)
{
    chk_arg_fault_return(ref);
    chk_arg_fault_return(sa);

    sa->un.family = prx_address_family_inet6;

    sa->un.ip.un.in6.un.u64[0] = ref->un.u64[0];
    sa->un.ip.un.in6.un.u64[1] = ref->un.u64[1];

    return er_ok;
}

//
// Encode the ref
//
int32_t io_encode_ref(
    io_codec_ctx_t *ctx,
    io_ref_t* ref
)
{
    int32_t result;
    STRING_HANDLE str;

    dbg_assert_ptr(ctx);
    dbg_assert_ptr(ref);

    __io_encode_type_begin(ctx, ref, 1);
    if (io_codec_json == io_codec_ctx_get_codec_id(ctx))
    {
        str = io_ref_to_STRING(ref);
        if (!str)
            return er_out_of_memory;
        result = io_encode_STRING_HANDLE(ctx, "id", str);
        STRING_delete(str);
        if (result != er_ok)
            return result;
    }
    else
    {
        result = io_encode_bin(ctx, "id", ref->un.u8, sizeof(ref->un.u8));
        if (result != er_ok)
            return result;
    }
    __io_encode_type_end(ctx);
    return result;
}

//
// Decode the ref
//
int32_t io_decode_ref(
    io_codec_ctx_t *ctx,
    io_ref_t* ref
)
{
    int32_t result;
    size_t size;
    STRING_HANDLE str;

    dbg_assert_ptr(ctx);
    dbg_assert_ptr(ref);

    __io_decode_type_begin(ctx, ref, 1);
    if (io_codec_json == io_codec_ctx_get_codec_id(ctx))
    {
        result = io_decode_STRING_HANDLE(ctx, "id", &str);
        if (result != er_ok)
            return result;
        result = io_ref_from_string(STRING_c_str(str), ref);
        STRING_delete(str);
        if (result != er_ok)
            return result;
    }
    else
    {
        size = sizeof(ref->un.u8);
        result = io_decode_bin_fixed(ctx, "id", ref->un.u8, &size);
        if (result != er_ok)
            return result;
    }
    __io_decode_type_end(ctx);
    return result;
}
