// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _io_codec_h_
#define _io_codec_h_

#include "common.h"
#include "util_log.h"
#include "util_dbg.h"
#include "io_stream.h"
#include "azure_c_shared_utility/strings.h"

//
// Document codec interface
//
typedef struct io_codec io_codec_t;

//
// Known codecs
//
typedef enum io_codec_id
{
    io_codec_auto = 0,
    io_codec_mpack,
    io_codec_json,

    //...

    io_codec_unknown = io_codec_auto
}
io_codec_id_t;

typedef struct io_codec_ctx io_codec_ctx_t;

//
// User allocator - allows user to pass copy memory to decoder
//
typedef int32_t (*io_decode_allocator_t)(
    io_codec_ctx_t* ctx,
    size_t desired,
    void** mem,
    size_t* mem_len
    );

//
// Codec context - on the stack during code/decode
//
struct io_codec_ctx
{
    io_codec_t* codec;                        // The codec interface
    void* context;          // The real context object for the codec
    size_t index;         // An index, e.g. into array or properties
    void* user_context;  // user context associated with the context
    io_decode_allocator_t default_allocator;    // Default allocator
    log_t log;
};

//
// Initializes a codec context object on the stack
//
typedef int32_t (*io_codec_init_ctx_t)(
    io_codec_ctx_t* ctx,
    io_stream_t* stream,
    bool init_from_stream, 
    log_t log
    );

//
// Encodes a signed integer
//
typedef int32_t (*io_encode_int64_t)(
    io_codec_ctx_t* ctx,
    const char* name,
    int64_t value
    );

//
// Encodes an unsigned integer
//
typedef int32_t (*io_encode_uint64_t)(
    io_codec_ctx_t* ctx,
    const char* name,
    uint64_t value
    );

//
// Encodes a decimal value 
//
typedef int32_t (*io_encode_double_t)(
    io_codec_ctx_t* ctx,
    const char* name,
    double value
    );

//
// Encodes a boolean value
//
typedef int32_t (*io_encode_bool_t)(
    io_codec_ctx_t* ctx,
    const char* name,
    bool value
    );

//
// Encodes a string (utf-8)
//
typedef int32_t (*io_encode_string_t)(
    io_codec_ctx_t* ctx,
    const char* name,
    const char* value
    );

//
// Encodes binary data
//
typedef int32_t (*io_encode_bin_t)(
    io_codec_ctx_t* ctx,
    const char* name,
    const void* value,
    size_t size
    );

//
// Encodes a custom type with n properties
//
typedef int32_t (*io_encode_type_begin_t)(
    io_codec_ctx_t* ctx,
    size_t members
    );

//
// Finish encoding custom type
//
typedef int32_t(*io_encode_type_end_t)(
    io_codec_ctx_t* ctx
    );

//
// Encodes an object, object lives on stack
//
typedef int32_t (*io_encode_object_t)(
    io_codec_ctx_t* ctx,
    const char* name,
    bool is_null,
    io_codec_ctx_t* object
    );

//
// Encodes an array, array lives on stack
//
typedef int32_t(*io_encode_array_t)(
    io_codec_ctx_t* ctx,
    const char* name,
    size_t length,
    io_codec_ctx_t* array
    );

//
// Decodes a signed integner
//
typedef int32_t (*io_decode_int64_t)(
    io_codec_ctx_t* ctx,
    const char* name,
    int64_t* value
    );

//
// Decodes an unsigned integer
//
typedef int32_t (*io_decode_uint64_t)(
    io_codec_ctx_t* ctx,
    const char* name,
    uint64_t* value
    );

//
// Decodes a decimal value 
//
typedef int32_t (*io_decode_double_t)(
    io_codec_ctx_t* ctx,
    const char* name,
    double* value
    );

//
// Decodes a boolean value
//
typedef int32_t (*io_decode_bool_t)(
    io_codec_ctx_t* ctx,
    const char* name,
    bool* value
    );

//
// Decodes a string (utf-8)
//
typedef int32_t (*io_decode_string_t)(
    io_codec_ctx_t* ctx,
    const char* name,
    io_decode_allocator_t alloc,
    char** value,
    size_t* size
    );

//
// Decodes binary data
//
typedef int32_t (*io_decode_bin_t)(
    io_codec_ctx_t* ctx,
    const char* name,
    io_decode_allocator_t alloc,
    void** value,
    size_t* size
    );

//
// Decodes an object, object lives on stack
//
typedef int32_t (*io_decode_object_t)(
    io_codec_ctx_t* ctx,
    const char* name,
    bool* is_null,
    io_codec_ctx_t* object
    );

//
// Begin decoding custom type, e.g. array header in mpack
//
typedef int32_t (*io_decode_type_begin_t)(
    io_codec_ctx_t* ctx
    );

//
// End custom type decoding, and e.g. skips unread properties
//
typedef int32_t (*io_decode_type_end_t)(
    io_codec_ctx_t* ctx
    );

//
// Decodes an array, array lives on stack
//
typedef int32_t (*io_decode_array_t)(
    io_codec_ctx_t* ctx,
    const char* name,
    size_t* length,
    io_codec_ctx_t* array
    );

//
// Finalizes a codec context object
//
typedef int32_t (*io_codec_fini_ctx_t)(
    io_codec_ctx_t* ctx,
    io_stream_t* stream,
    bool flush
);

//
// Codec interface
//
struct io_codec
{
    io_codec_id_t id;

    io_encode_int64_t enc_int;
    io_encode_uint64_t enc_uint;
    io_encode_double_t enc_decimal;
    io_encode_bool_t enc_bool;
    io_encode_string_t enc_string;
    io_encode_bin_t enc_bin;
    io_encode_object_t enc_object;
    io_encode_type_begin_t enc_tbegin;
    io_encode_type_end_t enc_tend;
    io_encode_array_t enc_array;

    io_decode_int64_t dec_int;
    io_decode_uint64_t dec_uint;
    io_decode_double_t dec_double;
    io_decode_bool_t dec_bool;
    io_decode_string_t dec_string;
    io_decode_bin_t dec_bin;
    io_decode_object_t dec_object;
    io_decode_type_begin_t dec_tbegin;
    io_decode_type_end_t dec_tend;
    io_decode_array_t dec_array;

    io_codec_init_ctx_t init_ctx;
    io_codec_fini_ctx_t fini_ctx;
};


//
// init a codec context object on the stack
//
decl_inline_5(int32_t, io_codec_ctx_init,
    io_codec_t*, codec,
    io_codec_ctx_t*, ctx,
    io_stream_t*, stream,
    bool, init_from_stream,
    log_t, log
)
{
    dbg_assert_ptr(ctx);
    ctx->codec = codec;
    dbg_assert_ptr(ctx->codec);
    dbg_assert_ptr(ctx->codec->init_ctx);
    return ctx->codec->init_ctx(ctx, stream, init_from_stream, log);
}

//
// Return codec id of the codec in the context
//
decl_inline_1(io_codec_id_t, io_codec_ctx_get_codec_id,
    io_codec_ctx_t*, ctx
)
{
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(ctx->codec);
    return ctx->codec->id;
}

//
// finlize a codec context object
//
decl_inline_3(int32_t, io_codec_ctx_fini,
    io_codec_ctx_t*, ctx,
    io_stream_t*, stream,
    bool, flush
)
{
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(ctx->codec);
    dbg_assert_ptr(ctx->codec->fini_ctx);
    return ctx->codec->fini_ctx(ctx, stream, flush);
}

//
// Encodes signed integer
//
decl_inline_3(int32_t, io_encode_int64,
    io_codec_ctx_t*, ctx,
    const char*, name,
    int64_t, value
)
{
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(ctx->codec);
    dbg_assert_ptr(ctx->codec->enc_int);
    return ctx->codec->enc_int(ctx, name, value);
}

#define io_encode_int32 io_encode_int64
#define io_encode_int16 io_encode_int64
#define io_encode_int8 io_encode_int64

//
// Encodes unsigned integer
//
decl_inline_3(int32_t, io_encode_uint64,
    io_codec_ctx_t*, ctx,
    const char*, name,
    uint64_t, value
)
{
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(ctx->codec);
    dbg_assert_ptr(ctx->codec->enc_uint);
    return ctx->codec->enc_uint(ctx, name, value);
}

#define io_encode_uint32 io_encode_uint64
#define io_encode_uint16 io_encode_uint64
#define io_encode_uint8 io_encode_uint64

//
// Encodes decimal value 
//
decl_inline_3(int32_t, io_encode_double,
    io_codec_ctx_t*, ctx,
    const char*, name,
    double, value
)
{
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(ctx->codec);
    dbg_assert_ptr(ctx->codec->enc_decimal);
    return ctx->codec->enc_decimal(ctx, name, value);
}

//
// Encodes boolean value
//
decl_inline_3(int32_t, io_encode_bool,
    io_codec_ctx_t*, ctx,
    const char*, name,
    bool, value
)
{
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(ctx->codec);
    dbg_assert_ptr(ctx->codec->enc_bool);
    return ctx->codec->enc_bool(ctx, name, value);
}

//
// Encodes string (utf-8)
//
decl_inline_3(int32_t, io_encode_string,
    io_codec_ctx_t*, ctx,
    const char*, name,
    const char*, value
)
{
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(ctx->codec);
    dbg_assert_ptr(ctx->codec->enc_string);
    return ctx->codec->enc_string(ctx, name, value);
}

//
// Encodes binary data
//
decl_inline_4(int32_t, io_encode_bin,
    io_codec_ctx_t*, ctx,
    const char*, name,
    const void*, value,
    size_t, size
)
{
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(ctx->codec);
    dbg_assert_ptr(ctx->codec->enc_bin);
    return ctx->codec->enc_bin(ctx, name, value, size);
}

//
// Begins encoding type
//
decl_inline_2(int32_t, io_encode_type_begin,
    io_codec_ctx_t*, ctx,
    size_t, members
)
{
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(ctx->codec);
    dbg_assert_ptr(ctx->codec->enc_tbegin);
    return ctx->codec->enc_tbegin(ctx, members);
}

//
// Begins encoding type
//
decl_inline_1(int32_t, io_encode_type_end,
    io_codec_ctx_t*, ctx
)
{
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(ctx->codec);
    dbg_assert_ptr(ctx->codec->enc_tend);
    return ctx->codec->enc_tend(ctx);
}

//
// Encodes an object
//
decl_inline_4(int32_t, io_encode_object,
    io_codec_ctx_t*, ctx,
    const char*, name,
    bool, is_null,
    io_codec_ctx_t*, object
)
{
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(ctx->codec);
    dbg_assert_ptr(ctx->codec->enc_object);
    return ctx->codec->enc_object(ctx, name, is_null, object);
}

//
// Encodes an array
//
decl_inline_4(int32_t, io_encode_array,
    io_codec_ctx_t*, ctx,
    const char*, name,
    size_t, size,
    io_codec_ctx_t*, array
)
{
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(ctx->codec);
    dbg_assert_ptr(ctx->codec->enc_array);
    return ctx->codec->enc_array(ctx, name, size, array);
}

//
// Encodes string (utf-8)
//
decl_inline_3(int32_t, io_encode_STRING_HANDLE,
    io_codec_ctx_t*, ctx,
    const char*, name,
    STRING_HANDLE, value
)
{
    return io_encode_string(ctx, name, value ? STRING_c_str(value) : NULL);
}

//
// decodes a signed integner
//
decl_inline_3(int32_t, io_decode_int64,
    io_codec_ctx_t*, ctx,
    const char*, name,
    int64_t*, value
)
{
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(ctx->codec);
    dbg_assert_ptr(ctx->codec->dec_int);
    return ctx->codec->dec_int(ctx, name, value);
}

//
// decodes an unsigned integer
//
decl_inline_3(int32_t, io_decode_uint64,
    io_codec_ctx_t*, ctx,
    const char*, name,
    uint64_t*, value
)
{
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(ctx->codec);
    dbg_assert_ptr(ctx->codec->dec_uint);
    return ctx->codec->dec_uint(ctx, name, value);
}

//
// decodes uint32
//
decl_inline_3(int32_t, io_decode_uint32,
    io_codec_ctx_t*, ctx,
    const char*, name,
    uint32_t*, val
)
{
    int32_t result;
    uint64_t u64;
    result = io_decode_uint64(ctx, name, &u64);
    if (result != er_ok)
        return result;
    *val = (uint32_t)u64;
    return (uint64_t)*val == u64 ? er_ok : er_invalid_format;
}

//
// decodes uint16
//
decl_inline_3(int32_t, io_decode_uint16,
    io_codec_ctx_t*, ctx,
    const char*, name,
    uint16_t*, val
)
{
    int32_t result;
    uint64_t u64;
    result = io_decode_uint64(ctx, name, &u64);
    if (result != er_ok)
        return result;
    *val = (uint16_t)u64;
    return (uint64_t)*val == u64 ? er_ok : er_invalid_format;
}

//
// decodes uint8
//
decl_inline_3(int32_t, io_decode_uint8,
    io_codec_ctx_t*, ctx,
    const char*, name,
    uint8_t*, val
)
{
    int32_t result;
    uint64_t u64;
    result = io_decode_uint64(ctx, name, &u64);
    if (result != er_ok)
        return result;
    *val = (uint8_t)u64;
    return (uint64_t)*val == u64 ? er_ok : er_invalid_format;
}

//
// decodes int32
//
decl_inline_3(int32_t, io_decode_int32,
    io_codec_ctx_t*, ctx,
    const char*, name,
    int32_t*, val
)
{
    int32_t result;
    int64_t i64;
    result = io_decode_int64(ctx, name, &i64);
    if (result != er_ok)
        return result;
    *val = (int32_t)i64;
    return (int64_t)*val == i64 ? er_ok : er_invalid_format;
}

//
// decodes int16
//
decl_inline_3(int32_t, io_decode_int16,
    io_codec_ctx_t*, ctx,
    const char*, name,
    int16_t*, val
)
{
    int32_t result;
    int64_t i64;
    result = io_decode_int64(ctx, name, &i64);
    if (result != er_ok)
        return result;
    *val = (int16_t)i64;
    return (int64_t)*val == i64 ? er_ok : er_invalid_format;
}

//
// read int8
//
decl_inline_3(int32_t, io_decode_int8,
    io_codec_ctx_t*, ctx,
    const char*, name,
    int8_t*, val
)
{
    int32_t result;
    int64_t i64;
    result = io_decode_int64(ctx, name, &i64);
    if (result != er_ok)
        return result;
    *val = (int8_t)i64;
    return (int64_t)*val == i64 ? er_ok : er_invalid_format;
}

//
// decodes a decimal value 
//
decl_inline_3(int32_t, io_decode_double,
    io_codec_ctx_t*, ctx,
    const char*, name,
    double*, value
)
{
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(ctx->codec);
    dbg_assert_ptr(ctx->codec->dec_double);
    return ctx->codec->dec_double(ctx, name, value);
}

//
// decodes a boolean value
//
decl_inline_3(int32_t, io_decode_bool,
    io_codec_ctx_t*, ctx,
    const char*, name,
    bool*, value
)
{
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(ctx->codec);
    dbg_assert_ptr(ctx->codec->dec_bool);
    return ctx->codec->dec_bool(ctx, name, value);
}

//
// decodes a string (utf-8)
//
decl_inline_5(int32_t, io_decode_string,
    io_codec_ctx_t*, ctx,
    const char*, name,
    io_decode_allocator_t, alloc,
    char**, value,
    size_t*, size
)
{
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(ctx->codec);
    dbg_assert_ptr(ctx->codec->dec_string);
    if (!alloc)
        alloc = ctx->default_allocator;
    return ctx->codec->dec_string(ctx, name, alloc, value, size);
}

//
// Decode and copy into mem_alloc'ed string
//
decl_internal_3(int32_t, io_decode_string_default,
    io_codec_ctx_t*, ctx,
    const char*, name,
    char**, string
);

//
// Read fixed string
//
decl_internal_4(int32_t, io_decode_string_fixed,
    io_codec_ctx_t*, ctx,
    const char*, name,
    char*, string,
    size_t, len
);

//
// Decode and return a string handle
//
decl_internal_3(int32_t, io_decode_STRING_HANDLE,
    io_codec_ctx_t*, ctx,
    const char*, name,
    STRING_HANDLE*, string
);

//
// decodes binary data
//
decl_inline_5(int32_t, io_decode_bin,
    io_codec_ctx_t*, ctx,
    const char*, name,
    io_decode_allocator_t, alloc,
    void**, value,
    size_t*, size
)
{
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(ctx->codec);
    dbg_assert_ptr(ctx->codec->dec_bin);
    if (!alloc)
        alloc = ctx->default_allocator;
    return ctx->codec->dec_bin(ctx, name, alloc, value, size);
}

//
// Decode and copy into mem_alloc'ed buffer
//
decl_internal_4(int32_t, io_decode_bin_default,
    io_codec_ctx_t*, ctx,
    const char*, name,
    void**, value,
    size_t*, size
);

//
// Read fixed binary
//
decl_internal_4(int32_t, io_decode_bin_fixed,
    io_codec_ctx_t*, ctx,
    const char*, name,
    void*, value,
    size_t*, size
);

//
// Begins decoding type
//
decl_inline_1(int32_t, io_decode_type_begin,
    io_codec_ctx_t*, ctx
)
{
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(ctx->codec);
    dbg_assert_ptr(ctx->codec->dec_tbegin);
    return ctx->codec->dec_tbegin(ctx);
}

//
// Ends decoding type
//
decl_inline_1(int32_t, io_decode_type_end,
    io_codec_ctx_t*, ctx
)
{
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(ctx->codec);
    dbg_assert_ptr(ctx->codec->dec_tend);
    return ctx->codec->dec_tend(ctx);
}

//
// Decodes an object
//
decl_inline_4(int32_t, io_decode_object,
    io_codec_ctx_t*, ctx,
    const char*, name,
    bool*, is_null,
    io_codec_ctx_t*, object
)
{
    bool tmp;
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(ctx->codec);
    dbg_assert_ptr(ctx->codec->dec_object);
    return ctx->codec->dec_object(ctx, name, is_null ? is_null : &tmp, object);
}

//
// Decodes an array
//
decl_inline_4(int32_t, io_decode_array,
    io_codec_ctx_t*, ctx,
    const char*, name,
    size_t*, size,
    io_codec_ctx_t*, array
)
{
    dbg_assert_ptr(ctx);
    dbg_assert_ptr(ctx->codec);
    dbg_assert_ptr(ctx->codec->dec_array);
    return ctx->codec->dec_array(ctx, name, size, array);
}

//
// Get codec by id
//
decl_internal_1(io_codec_t*, io_codec_by_id,
    io_codec_id_t, id
);

//
// Utilty macros
//

typedef double double_t;
#ifndef __cplusplus
typedef _Bool _Bool_t;
#endif
typedef bool bool_t;
typedef struct STRING_TAG* STRING_HANDLE_t;

//
// Utility macro to encode a primitive struct member
//
#define __io_encode_value(ctx, type, s, m)  { \
        result = io_encode_##type (ctx, #m, ( type##_t)s->m); \
        if (result != er_ok) \
            return result; \
    }

//
// Utility macro to decode a primitive struct member
//
#define __io_decode_value(ctx, type, s, m)  { \
        result = io_decode_##type (ctx, #m, ( type##_t*)&s->m); \
        if (result != er_ok) \
            return result; \
    }

//
// Utility macro to encode a pointer object struct member
//
#define __io_encode_object(ctx, type, s, m)  { \
        io_codec_ctx_t obj; \
        result = io_encode_object(ctx, #m, false, &obj); \
        if (result != er_ok) \
            return result; \
        result = io_encode_##type (&obj, &s->m); \
        if (result != er_ok) \
            return result; \
    }

//
// Utility macro to decode a pointer object struct member
//
#define __io_decode_object(ctx, type, s, m)  { \
        io_codec_ctx_t obj; \
        bool is_null; \
        result = io_decode_object(ctx, #m, &is_null, &obj); \
        if (result != er_ok) \
            return result; \
        if (!is_null) { \
            result = io_decode_##type(&obj, &s->m); \
            if (result != er_ok) \
                return result; \
        } \
    }

//
// Utility macro to begin custom type encoding
//
#define __io_encode_type_begin(ctx, p, n)  { \
        (void)p; \
        result = io_encode_type_begin(ctx, n); \
        if (result != er_ok) \
            return result; \
    }

//
// Utility macro to finish encoding type
//
#define __io_encode_type_end(ctx)  { \
        result = io_encode_type_end(ctx); \
        if (result != er_ok) \
            return result; \
    }

//
// Utility macro to begin custom type decoding
//
#define __io_decode_type_begin(ctx, p, n) { \
        (void)p, n; \
        result = io_decode_type_begin(ctx); \
        if (result != er_ok) \
            return result; \
    } 

//
// Utility macro to finish decoding type
//
#define __io_decode_type_end(ctx)  { \
        result = io_decode_type_end(ctx); \
        if (result != er_ok) \
            return result; \
    }

#endif  // _io_codec_h_