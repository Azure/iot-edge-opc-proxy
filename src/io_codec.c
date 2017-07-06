// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "io_codec.h"
#include "util_string.h"

#include "cmp.h"
#include "parson.h"

#include "azure_c_shared_utility/base64.h"

//
// Initialize a context object from a stream
//
static int32_t json_init_ctx(
    io_codec_ctx_t* ctx,
    io_stream_t* stream,
    bool init_from_stream,
    log_t log
)
{
    int32_t result;
    size_t buf_len = 0, avail, read;
    char* buffer = NULL, *tmp;
    while(true)
    {
        result = er_ok;
        if (!init_from_stream)
        {
            ctx->default_allocator = NULL;
            ctx->user_context = NULL;
            ctx->index = 0;
            ctx->log = log;
            ctx->context = json_value_init_object();
            break;
        }

        avail = io_stream_readable(stream);
        if (!avail)
        {
            if (buf_len == 0)
            {
                result = er_reading;
                break;
            }

            buffer[buf_len] = 0;

            ctx->default_allocator = NULL;
            ctx->user_context = NULL;
            ctx->index = 0;
            ctx->log = log;
            ctx->context = json_parse_string_with_comments(buffer);
            if (!ctx->context)
            {
                result = er_invalid_format;
                break;
            }
            // Done
            break;
        }

        if (avail > 0x1000)
            avail = 0x1000; 
        
        tmp = (char*)mem_realloc(buffer, buf_len + avail + 1);
        if (!tmp)
        {
            result = er_out_of_memory;
            break;
        }
        buffer = tmp;
        result = io_stream_read(
            stream, &buffer[buf_len], avail, &read);
        if (result != er_ok)
            break;
      
        // dbg_assert(read == avail, "Not all read, but success");
        buf_len += read;
    }
    if (buffer)
        mem_free(buffer);
    return result;
}

//
// Finish the context, serialize out to stream
//
static int32_t json_fini_ctx(
    io_codec_ctx_t* ctx,
    io_stream_t* stream,
    bool flush
)
{
    int32_t result;
    char* buffer = NULL;
    size_t buf_size;
    JSON_Value* val = (JSON_Value*)ctx->context;
    dbg_assert_ptr(val);
    do
    {
        result = er_ok;
        if (!flush)
            break;

        buf_size = json_serialization_size_pretty(val);
        if (!buf_size)
            break;

        buffer = (char*)mem_alloc(buf_size);
        if (!buffer)
        {
            result = er_out_of_memory;
            break;
        }

        if (JSONSuccess != json_serialize_to_buffer_pretty(val, buffer, buf_size))
        {
            result = er_writing;
            break;
        }
        
        result = io_stream_reset(stream);
        if (result != er_ok)
            break;
        result = io_stream_write(stream, buffer, buf_size-1);  // omit '\0'
        break;
    } 
    while (0);
    if (buffer)
        mem_free(buffer);
    json_value_free(val);
    return result;
}

//
// Append a decimal value to an array
//
static int32_t json_append_number(
    io_codec_ctx_t* ctx,
    double value
)
{
    JSON_Array* arr;

    arr = json_value_get_array((JSON_Value*)ctx->context);
    if (JSONSuccess != json_array_append_number(arr, value))
        return er_out_of_memory;
    return er_ok;
}

//
// Sets a new number property
//
static int32_t json_set_number(
    io_codec_ctx_t* ctx,
    const char* name,
    double value
)
{
    JSON_Object* obj;

    obj = json_value_get_object((JSON_Value*)ctx->context);
    if (JSONSuccess != json_object_set_number(obj, name, value))
        return er_out_of_memory;
    return er_ok;
}

//
// Writes a number value 
//
static int32_t json_write_number(
    io_codec_ctx_t* ctx,
    const char* name,
    double value
)
{
    if (!name)
        return json_append_number(ctx, value);
    else
        return json_set_number(ctx, name, value);
}

//
// Writes a signed integer
//
static int32_t json_write_integer(
    io_codec_ctx_t* ctx,
    const char* name,
    int64_t value
)
{
    return json_write_number(ctx, name, (double)value);
}

//
// Writes an unsigned integer
//
static int32_t json_write_uinteger(
    io_codec_ctx_t* ctx,
    const char* name,
    uint64_t value
)
{
    return json_write_number(ctx, name, (double)value);
}

//
// Appends a json boolean value to an array
//
static int32_t json_append_bool(
    io_codec_ctx_t* ctx,
    bool value
)
{
    JSON_Array* arr;

    arr = json_value_get_array((JSON_Value*)ctx->context);
    if (JSONSuccess != json_array_append_boolean(arr, value))
        return er_out_of_memory;
    return er_ok;
}

//
// Sets a new boolean property
//
static int32_t json_set_bool(
    io_codec_ctx_t* ctx,
    const char* name,
    bool value
)
{
    JSON_Object* obj;

    obj = json_value_get_object((JSON_Value*)ctx->context);
    if (JSONSuccess != json_object_set_boolean(obj, name, value ? 1 : 0))
        return er_out_of_memory;
    return er_ok;
}
//
// Writes a json boolean value
//
static int32_t json_write_bool(
    io_codec_ctx_t* ctx,
    const char* name,
    bool value
)
{
    if (!name)
        return json_append_bool(ctx, value);
    else
        return json_set_bool(ctx, name, value);
}

//
// Append a string value to an array
//
static int32_t json_append_string(
    io_codec_ctx_t* ctx,
    const char* value
)
{
    JSON_Array* arr;
    
    arr = json_value_get_array((JSON_Value*)ctx->context);
    if (JSONSuccess != json_array_append_string(arr, value))
        return er_out_of_memory;
    return er_ok;
}

//
// Sets a string value
//
static int32_t json_set_string(
    io_codec_ctx_t* ctx,
    const char* name,
    const char* value
)
{
    JSON_Object* obj;
    
    obj = json_value_get_object((JSON_Value*)ctx->context);
    if (JSONSuccess != json_object_set_string(obj, name, value))
        return er_out_of_memory;
    return er_ok;
}

//
// Writes a string value
//
static int32_t json_write_string(
    io_codec_ctx_t* ctx,
    const char* name,
    const char* value
)
{
    if (!name)
        return json_append_string(ctx, value);
    else
        return json_set_string(ctx, name, value);
}

//
// Writes base64 encoded binary data value
//
static int32_t json_write_bin(
    io_codec_ctx_t* ctx,
    const char* name,
    const void* value,
    size_t size
)
{
    int32_t result;
    STRING_HANDLE base64;

    if (!size)
        return json_write_string(ctx, name, "");

    base64 = Base64_Encode_Bytes((const unsigned char*)value, size);
    if (!base64)
        return er_out_of_memory;

    result = json_write_string(ctx, name, STRING_c_str(base64));
    STRING_delete(base64);
    return result;
}

//
// Writes a new json array
//
static int32_t json_write_array(
    io_codec_ctx_t* ctx,
    const char* name,
    size_t count,
    io_codec_ctx_t* array
)
{
    JSON_Status status;
    JSON_Value* value;
    JSON_Object* obj = json_value_get_object((JSON_Value*)ctx->context);
    (void)count;

    value = json_value_init_array();
    if (!value)
        return er_out_of_memory;

    status = json_object_set_value(obj, name, value);
    if (status != JSONSuccess)
    {
        json_value_free(value);
        return er_out_of_memory;
    }

    array->context = value;
    array->codec = ctx->codec;
    array->default_allocator = ctx->default_allocator;
    array->user_context = ctx->user_context;
    array->log = ctx->log;
    array->index = 0;
    return er_ok;
}

//
// Writes a new json object
//
static int32_t json_write_object(
    io_codec_ctx_t* ctx,
    const char* name,
    bool is_null,
    io_codec_ctx_t* object
)
{
    JSON_Status status;
    JSON_Value* value;
    JSON_Object* obj = NULL;
    JSON_Array* arr = NULL;
    JSON_Value_Type type;

    status = JSONSuccess;
    value = (JSON_Value*)ctx->context;

    type = json_value_get_type(value);
    /**/ if (type == JSONArray)
    {
        arr = json_value_get_array(value);
        if (!arr)
            return er_invalid_format;
        value = is_null ? json_value_init_null() : json_value_init_object();
        if (!value)
            return er_out_of_memory;
        status = json_array_append_value(arr, value);
    }
    else if (type != JSONObject)
    {
        log_error(ctx->log,
            "Cannnot write an object into anything other than obj or arr");
        return er_arg;
    }
    else if (name)
    {
        obj = json_value_get_object(value);
        if (!obj)
            return er_invalid_format;
        value = is_null ? json_value_init_null() : json_value_init_object();
        if (!value)
            return er_out_of_memory;
        status = json_object_set_value(obj, name, value);
    }
    else
    {
        status = JSONSuccess; // this is the first object
    }

    if (status != JSONSuccess)
    {
        json_value_free(value);
        return er_out_of_memory;
    }

    object->context = value;
    object->codec = ctx->codec;
    object->default_allocator = ctx->default_allocator;
    object->user_context = ctx->user_context;
    object->log = ctx->log;
    object->index = 0;
    return er_ok;
}

//
// Start writing a type
//
static int32_t json_write_type_begin(
    io_codec_ctx_t* ctx,
    size_t members
)
{
    (void)ctx;
    (void)members;
    return er_ok;
}

//
// No op
//
static int32_t json_write_type_end(
    io_codec_ctx_t* ctx
)
{
    (void)ctx;
    return er_ok;
}

//
// Returns next number in current array
//
static int32_t json_next_number(
    io_codec_ctx_t* ctx,
    double* value
)
{
    JSON_Value* val;
    JSON_Array* arr;

    arr = json_value_get_array((JSON_Value*)ctx->context);
    val = json_array_get_value(arr, ctx->index++);
    if (!val || JSONNumber != json_value_get_type(val))
        return er_invalid_format;
    *value = json_value_get_number(val);
    return er_ok;
}

//
// Gets a number property value
//
static int32_t json_get_number(
    io_codec_ctx_t* ctx,
    const char* name,
    double* value
)
{
    JSON_Value* val;
    JSON_Object* obj;

    obj = json_value_get_object((JSON_Value*)ctx->context);
    val = json_object_get_value(obj, name);
    if (!val || JSONNumber != json_value_get_type(val))
        return er_invalid_format;
    *value = json_value_get_number(val);
    return er_ok;
}

//
// Reads a number value 
//
static int32_t json_read_number(
    io_codec_ctx_t* ctx,
    const char* name,
    double* value
)
{
    if (!name)
        return json_next_number(ctx, value);
    else
        return json_get_number(ctx, name, value);
}

//
// Reads a signed integner
//
static int32_t json_read_integer(
    io_codec_ctx_t* ctx,
    const char* name,
    int64_t* value
)
{
    double number;
    int32_t result;
    result = json_read_number(ctx, name, &number);
    if (result != er_ok)
        return result;
    *value = (int64_t)number;
    return er_ok;
}

//
// Reads an unsigned integer
//
static int32_t json_read_uinteger(
    io_codec_ctx_t* ctx,
    const char* name,
    uint64_t* value
)
{
    double number;
    int32_t result;
    result = json_read_number(ctx, name, &number);
    if (result != er_ok)
        return result;
    *value = (uint64_t)number;
    return er_ok;
}

//
// Returns next boolean value in current array
//
static int32_t json_next_bool(
    io_codec_ctx_t* ctx,
    bool* value
)
{
    JSON_Value* val;
    JSON_Array* arr;

    arr = json_value_get_array((JSON_Value*)ctx->context);
    val = json_array_get_value(arr, ctx->index++);
    if (!val || JSONBoolean != json_value_get_type(val))
        return er_invalid_format;
    *value = !!json_value_get_boolean(val);
    return er_ok;
}

//
// Returns a boolean property value
//
static int32_t json_get_bool(
    io_codec_ctx_t* ctx,
    const char* name,
    bool* value
)
{
    JSON_Value* val;
    JSON_Object* obj;
    
    obj = json_value_get_object((JSON_Value*)ctx->context);
    val = json_object_get_value(obj, name);
    if (!val || JSONBoolean != json_value_get_type(val))
        return er_invalid_format;
    *value = !!json_value_get_boolean(val);
    return er_ok;
}

//
// Reads a boolean value
//
static int32_t json_read_bool(
    io_codec_ctx_t* ctx,
    const char* name,
    bool* value
)
{
    if (!name)
        return json_next_bool(ctx, value);
    else
        return json_get_bool(ctx, name, value);
}

//
// Reads a string value from json message
//
static int32_t json_read_string(
    io_codec_ctx_t* ctx,
    const char* name,
    io_decode_allocator_t allocator,
    char** value,
    size_t* size
)
{
    int32_t result;
    JSON_Value* val;
    const char* string_value;

    val = (JSON_Value*)ctx->context;
    if (!name)
        val = json_array_get_value(json_value_get_array(val), ctx->index++);
    else
        val = json_object_get_value(json_value_get_object(val), name);
    if (!val || JSONString != json_value_get_type(val))
        return er_invalid_format;

    string_value = json_value_get_string(val);
    dbg_assert_ptr(string_value);

    result = allocator(ctx, strlen(string_value) + 1, (void**)value, size);
    if (result != er_ok)
        return result;
    strcpy(*value, string_value);
    return result;
}

//
// Decodes base64 encoded binary data from json
//
static int32_t json_read_bin(
    io_codec_ctx_t* ctx,
    const char* name,
    io_decode_allocator_t allocator,
    void** value,
    size_t* size
)
{
    int32_t result;
    JSON_Value* val;
    const char* string_value;
    size_t len;
    BUFFER_HANDLE decoded;
    
    val = (JSON_Value*)ctx->context;
    if (!name)
        val = json_array_get_value(json_value_get_array(val), ctx->index++);
    else
        val = json_object_get_value(json_value_get_object(val), name);
    if (!val || JSONString != json_value_get_type(val))
        return er_invalid_format;

    string_value = json_value_get_string(val);
    dbg_assert_ptr(string_value);
    if (!strlen(string_value))
    {
        *value = NULL;
        *size = 0;
        return er_ok;
    }

    decoded = Base64_Decoder(string_value);
    if (!decoded)
        return er_out_of_memory;
    do
    {
        if (0 != BUFFER_size(decoded, &len))
        {
            result = er_out_of_memory;
            break;
        }

        result = allocator(ctx, len, value, size);
        if (result != er_ok)
            break;

        memcpy(*value, BUFFER_u_char(decoded), *size);
        result = er_ok;
        break;
    } 
    while (0);
    BUFFER_delete(decoded);
    return result;
}

//
// No op
//
static int32_t json_read_type_begin(
    io_codec_ctx_t* ctx
)
{
    (void)ctx;
    return er_ok;
}

//
// No op
//
static int32_t json_read_type_end(
    io_codec_ctx_t* ctx
)
{
    (void)ctx;
    return er_ok;
}

//
// Decodes a json object
//
static int32_t json_read_object(
    io_codec_ctx_t* ctx,
    const char* name,
    bool* is_null,
    io_codec_ctx_t* object
)
{
    JSON_Value* val;
    JSON_Value_Type type;
    
    val = (JSON_Value*)ctx->context;
    if (!name)
    {
        type = json_value_get_type(val);
        /**/ if (type == JSONArray)
            val = json_array_get_value(json_value_get_array(val), ctx->index++);
        else if (type != JSONObject) // this is the first object
            return er_arg;
    }
    else
    {
        val = json_object_get_value(json_value_get_object(val), name);
    }
    if (!val)
        return er_invalid_format;

    type = json_value_get_type(val);
    /**/ if (type == JSONNull)
        *is_null = true;
    else if (type == JSONObject)
        *is_null = false;
    else
        return er_arg;

    object->log = ctx->log;
    object->codec = ctx->codec;
    object->default_allocator = ctx->default_allocator;
    object->user_context = ctx->user_context;
    object->context = val;
    object->index = 0;
    return er_ok;
}

//
// Decodes a json array
//
static int32_t json_read_array(
    io_codec_ctx_t* ctx,
    const char* name,
    size_t* size,
    io_codec_ctx_t* array
)
{
    JSON_Value* val;
    JSON_Array* arr;
    
    val = (JSON_Value*)ctx->context;
    if (!name)
        val = json_array_get_value(json_value_get_array(val), ctx->index++);
    else
        val = json_object_get_value(json_value_get_object(val), name);
    if (!val || JSONArray != json_value_get_type(val))
        return er_invalid_format;

    arr = json_value_get_array(val);
    dbg_assert_ptr(arr);
    *size = json_array_get_count(arr);

    array->log = ctx->log;
    array->codec = ctx->codec;
    array->default_allocator = ctx->default_allocator;
    array->user_context = ctx->user_context;
    array->context = val;
    array->index = 0;
    return er_ok;
}

//
// Get the json codec
//
static io_codec_t* json_codec(
    void
)
{
    static io_codec_t codec = {
        io_codec_json,

        json_write_integer,
        json_write_uinteger,
        json_write_number,
        json_write_bool,
        json_write_string,
        json_write_bin,
        json_write_object,
        json_write_type_begin,
        json_write_type_end,
        json_write_array,

        json_read_integer,
        json_read_uinteger,
        json_read_number,
        json_read_bool,
        json_read_string,
        json_read_bin,
        json_read_object,
        json_read_type_begin,
        json_read_type_end,
        json_read_array,

        json_init_ctx,
        json_fini_ctx
    };
    return &codec;
}

#define cmp_out_of_memory_error 0xff

//
// Returns the error occurred on the context
//
static int32_t cmp_ctx_get_err(
    cmp_ctx_t *ctx
)
{
    if (ctx->error == 0) /* ERROR_NONE */
        return er_ok;

    log_error(NULL, "Message Pack error '%s' occurred.", cmp_strerror(ctx));

    switch (ctx->error)
    {
    case 1   /* STR_DATA_LENGTH_TOO_LONG_ERROR */  : return er_arg;
    case 2   /* BIN_DATA_LENGTH_TOO_LONG_ERROR */  : return er_arg;
    case 3   /* ARRAY_LENGTH_TOO_LONG_ERROR */     : return er_arg;
    case 4   /* MAP_LENGTH_TOO_LONG_ERROR */       : return er_arg;
    case 5   /* INPUT_VALUE_TOO_LARGE_ERROR */     : return er_arg;
    case 6   /* FIXED_VALUE_WRITING_ERROR */       : return er_writing;
    case 7   /* TYPE_MARKER_READING_ERROR */       : return er_invalid_format;
    case 8   /* TYPE_MARKER_WRITING_ERROR */       : return er_writing;
    case 9   /* DATA_READING_ERROR */              : return er_invalid_format;
    case 10  /* DATA_WRITING_ERROR */              : return er_writing;
    case 11  /* EXT_TYPE_READING_ERROR */          : return er_invalid_format;
    case 12  /* EXT_TYPE_WRITING_ERROR */          : return er_writing;
    case 13  /* INVALID_TYPE_ERROR */              : return er_arg;
    case 14  /* LENGTH_READING_ERROR */            : return er_invalid_format;
    case 15  /* LENGTH_WRITING_ERROR */            : return er_writing;
    case 16  /* SKIP_DEPTH_LIMIT_EXCEEDED_ERROR */ : return er_fault;
    case 17                                        : return er_fault;
    case cmp_out_of_memory_error                   : return er_out_of_memory;
    }
    return er_unknown;
}

//
// Sets the error occurred in the context
//
static bool cmp_ctx_set_err(
    cmp_ctx_t *ctx,
    int32_t error
)
{
    switch (error)
    {
    case er_ok:                                     ctx->error = 0;  
        return true;
    case er_arg:                                    ctx->error = 1;  break;
    case er_writing:                                ctx->error = 10; break;
    case er_reading:                                ctx->error = 9;  break;
    case er_invalid_format:                         ctx->error = 7;  break;
    case er_out_of_memory:     ctx->error = cmp_out_of_memory_error; break;
    case er_fault:                                  ctx->error = 16; break;
    default:                                        ctx->error = 99; break;
    }
    return false;
}

//
// Codec reader
//
static bool cmp_stream_read(
    cmp_ctx_t *ctx,
    void *data,
    size_t limit
)
{
    int32_t result;
    size_t read = 0;
    io_stream_t* stream = (io_stream_t*)ctx->buf;
    result = io_stream_read(stream, data, limit, &read);
    if (result == er_ok && read != limit)
        result = er_reading;
    return cmp_ctx_set_err(ctx, result);
}

//
// Codec writer
//
static size_t cmp_stream_write(
    cmp_ctx_t *ctx,
    const void *data,
    size_t count
)
{
    io_stream_t* stream = (io_stream_t*)ctx->buf;
    return cmp_ctx_set_err(ctx, io_stream_write(stream, data, count)) ? 
        count : 0;
}

//
// Check each write
//
#define mpack_write_complete(ctx) \
    dbg_assert(ctx->index != 0, "Unexpected extra item during encoding"); \
    ctx->index-- \

//
// Writes a message pack signed integer
//
static int32_t mpack_write_integer(
    io_codec_ctx_t* ctx,
    const char* name,
    int64_t value
)
{
    cmp_ctx_t* cmp_ctx = (cmp_ctx_t*)ctx->context;
    (void)name;
    if (!cmp_write_integer(cmp_ctx, value))
        return cmp_ctx_get_err(cmp_ctx);
    mpack_write_complete(ctx);
    return er_ok;
}

//
// Writes a message pack unsigned integer
//
static int32_t mpack_write_uinteger(
    io_codec_ctx_t* ctx,
    const char* name,
    uint64_t value
)
{
    cmp_ctx_t* cmp_ctx = (cmp_ctx_t*)ctx->context;
    (void)name;
    if (!cmp_write_uinteger(cmp_ctx, value))
        return cmp_ctx_get_err(cmp_ctx);
    mpack_write_complete(ctx);
    return er_ok;
}

//
// Writes a message pack decimal value 
//
static int32_t mpack_write_decimal(
    io_codec_ctx_t* ctx,
    const char* name,
    double value
)
{
    cmp_ctx_t* cmp_ctx = (cmp_ctx_t*)ctx->context;
    (void)name;
    if (!cmp_write_decimal(cmp_ctx, value))
        return cmp_ctx_get_err(cmp_ctx);
    mpack_write_complete(ctx);
    return er_ok;
}

//
// Writes a message pack boolean value
//
static int32_t mpack_write_bool(
    io_codec_ctx_t* ctx,
    const char* name,
    bool value
)
{
    cmp_ctx_t* cmp_ctx = (cmp_ctx_t*)ctx->context;
    (void)name;
    if (!cmp_write_bool(cmp_ctx, value))
        return cmp_ctx_get_err(cmp_ctx);
    mpack_write_complete(ctx);
    return er_ok;
}

//
// Writes a message pack string (utf-8)
//
static int32_t mpack_write_string(
    io_codec_ctx_t* ctx,
    const char* name,
    const char* value
)
{
    cmp_ctx_t* cmp_ctx = (cmp_ctx_t*)ctx->context;
    (void)name;
    if (!cmp_write_str(cmp_ctx, value, (uint32_t)strlen(value)))
        return cmp_ctx_get_err(cmp_ctx);
    mpack_write_complete(ctx);
    return er_ok;
}

//
// Writes message pack binary data
//
static int32_t mpack_write_bin(
    io_codec_ctx_t* ctx,
    const char* name,
    const void* value,
    size_t size
)
{
    cmp_ctx_t* cmp_ctx = (cmp_ctx_t*)ctx->context;
    (void)name;
    if (!cmp_write_bin(cmp_ctx, value, (uint32_t)size))
        return cmp_ctx_get_err(cmp_ctx);
    mpack_write_complete(ctx);
    return er_ok;
}

//
// Does nothing since message pack does not support hierarchies
//
static int32_t mpack_write_object(
    io_codec_ctx_t* ctx,
    const char* name,
    bool is_null,
    io_codec_ctx_t* object
)
{
    cmp_ctx_t* cmp_ctx = (cmp_ctx_t*)ctx->context;

    if (is_null && !cmp_write_nil(cmp_ctx))
        return cmp_ctx_get_err(cmp_ctx);

    (void)name;
    object->codec = ctx->codec;
    object->context = ctx->context;
    object->default_allocator = ctx->default_allocator;
    object->user_context = ctx->user_context;
    object->log = ctx->log;
    mpack_write_complete(ctx);
    return er_ok;
}

//
// Write array header for array encoding method
//
static int32_t mpack_write_type_begin(
    io_codec_ctx_t* ctx,
    size_t members
)
{
    cmp_ctx_t* cmp_ctx = (cmp_ctx_t*)ctx->context;
    if (!cmp_write_array(cmp_ctx, (uint32_t)members + 1))
        return cmp_ctx_get_err(cmp_ctx);
    if (!cmp_write_nil(cmp_ctx))  // member place holder as per msgpack-cli
        return cmp_ctx_get_err(cmp_ctx);
    ctx->index = members;
    return er_ok;
}

//
// No op
//
static int32_t mpack_write_type_end(
    io_codec_ctx_t* ctx
)
{
    (void)ctx;
    dbg_assert(ctx->index == 0, "Missed writing expected property");
    return er_ok;
}

//
// Writes a message pack array marker
//
static int32_t mpack_write_array(
    io_codec_ctx_t* ctx,
    const char* name,
    size_t size,
    io_codec_ctx_t* array
)
{
    cmp_ctx_t* cmp_ctx = (cmp_ctx_t*)ctx->context;
    (void)name;
    if (!cmp_write_array(cmp_ctx, (uint32_t)size))
        return cmp_ctx_get_err(cmp_ctx);
    array->codec = ctx->codec;
    array->context = ctx->context;
    array->default_allocator = ctx->default_allocator;
    array->user_context = ctx->user_context;
    array->log = ctx->log;
    array->index = size;
    mpack_write_complete(ctx);
    return er_ok;
}

//
// Check each read
//
#define mpack_read_begin(ctx) \
    (void)name; if (ctx->index == 0) { \
        dbg_assert(0, "Expected another item of name %s", name); \
        log_error(ctx->log, "Missing item %s during decoding!", name); \
        return er_invalid_format; \
    } \
    ctx->index--

//
// Reads a message pack encoded integer
//
static int32_t mpack_read_integer(
    io_codec_ctx_t* ctx,
    const char* name,
    int64_t* value
)
{
    cmp_ctx_t* cmp_ctx = (cmp_ctx_t*)ctx->context;
    mpack_read_begin(ctx);
    if (!cmp_read_integer(cmp_ctx, value))
        return cmp_ctx_get_err(cmp_ctx);
    return er_ok;
}

//
// Reads a message pack encoded unsigned integer
//
static int32_t mpack_read_uinteger(
    io_codec_ctx_t* ctx,
    const char* name,
    uint64_t* value
)
{
    cmp_ctx_t* cmp_ctx = (cmp_ctx_t*)ctx->context;
    mpack_read_begin(ctx);
    if (!cmp_read_uinteger(cmp_ctx, value))
        return cmp_ctx_get_err(cmp_ctx);
    return er_ok;
}

//
// Reads a message pack encoded decimal value 
//
static int32_t mpack_read_decimal(
    io_codec_ctx_t* ctx,
    const char* name,
    double* value
)
{
    cmp_ctx_t* cmp_ctx = (cmp_ctx_t*)ctx->context;
    mpack_read_begin(ctx);
    if (!cmp_read_decimal(cmp_ctx, value))
        return cmp_ctx_get_err(cmp_ctx);
    return er_ok;
}

//
// Reads a message pack encoded boolean value
//
static int32_t mpack_read_bool(
    io_codec_ctx_t* ctx,
    const char* name,
    bool* value
)
{
    cmp_ctx_t* cmp_ctx = (cmp_ctx_t*)ctx->context;
    mpack_read_begin(ctx);
    if (!cmp_read_bool(cmp_ctx, value))
        return cmp_ctx_get_err(cmp_ctx);
    return er_ok;
}

//
// Reads a message pack encoded string (utf-8)
//
static int32_t mpack_read_string(
    io_codec_ctx_t* ctx,
    const char* name,
    io_decode_allocator_t allocator,
    char** value,
    size_t* size
)
{
    int32_t result;
    uint32_t len32;
    cmp_ctx_t* cmp_ctx = (cmp_ctx_t*)ctx->context;
    mpack_read_begin(ctx);
    if (!cmp_read_str_size(cmp_ctx, &len32))
        return cmp_ctx_get_err(cmp_ctx);
    result = allocator(ctx, len32 + 1, (void**)value, size);
    if (result != er_ok)
        return result;
    do
    {
        if (*size != (len32 + 1))
        {
            result = er_out_of_memory;
            break;
        }

        if (!cmp_ctx->read(cmp_ctx, *value, len32))
        {
            result = er_reading;
            break;
        }

        ((char*)(*value))[len32] = '\0';
        return er_ok;
    } 
    while (0);
    allocator(ctx, 0, (void**)value, size);
    return result;
}

//
// Reads message pack encoded binary data
//
static int32_t mpack_read_bin(
    io_codec_ctx_t* ctx,
    const char* name,
    io_decode_allocator_t allocator,
    void** value,
    size_t* size
)
{
    int32_t result;
    uint32_t len32;
    cmp_ctx_t* cmp_ctx = (cmp_ctx_t*)ctx->context;
    mpack_read_begin(ctx);
    if (!cmp_read_bin_size(cmp_ctx, &len32))
        return cmp_ctx_get_err(cmp_ctx);
    if (!len32)
    {
        *value = NULL;
        *size = 0;
        return er_ok;
    }

    result = allocator(ctx, len32, value, size);
    if (result != er_ok)
        return result;
    do
    {
        if (*size != len32)
        {
            result = er_out_of_memory;
            break;
        }

        if (!cmp_ctx->read(cmp_ctx, *value, len32))
        {
            result = er_reading;
            break;
        }
        return er_ok;
    } 
    while (0);
    allocator(ctx, 0, (void**)value, size);
    return result;
}

//
// Starts new object or returns if object is null
//
static int32_t mpack_read_object(
    io_codec_ctx_t* ctx,
    const char* name,
    bool* is_null,
    io_codec_ctx_t* object
)
{
    cmp_ctx_t* cmp_ctx = (cmp_ctx_t*)ctx->context;
    cmp_object_t obj;
    mpack_read_begin(ctx);
    if (!cmp_read_object(cmp_ctx, &obj))
        return cmp_ctx_get_err(cmp_ctx);
    switch (obj.type)
    {
    case CMP_TYPE_NIL:
        // object is null
        *is_null = true;
        object->index = 0;
        break;
    case CMP_TYPE_FIXARRAY:
    case CMP_TYPE_ARRAY16:
    case CMP_TYPE_ARRAY32:
        // object is array encoded object
        *is_null = false;
        object->index = obj.as.array_size;

        mpack_read_begin(object);
        if (!cmp_read_nil(cmp_ctx)) // Read member holder
            return cmp_ctx_get_err(cmp_ctx);
        break;
    case CMP_TYPE_FIXMAP:
    case CMP_TYPE_MAP16:
    case CMP_TYPE_MAP32:
        log_error(ctx->log,
            "Map encoded msgpack objects are not supported!");
        return er_invalid_format;
    default:
        log_error(ctx->log,
            "Unexpected type (%d) when reading object", obj.type);
        return er_invalid_format;
    }
    object->codec = ctx->codec;
    object->context = ctx->context;
    object->default_allocator = ctx->default_allocator;
    object->user_context = ctx->user_context;
    object->log = ctx->log;
    return er_ok;
}

//
// No op
//
static int32_t mpack_read_type_begin(
    io_codec_ctx_t* ctx
)
{
    (void)ctx;
    return er_ok;
}

//
// No op
//
static int32_t mpack_read_type_end(
    io_codec_ctx_t* ctx
)
{
    dbg_assert(ctx->index == 0, "Missing properties");
    (void)ctx;
    return er_ok;
}

//
// Reads a message pack array marker
//
static int32_t mpack_read_array(
    io_codec_ctx_t* ctx,
    const char* name,
    size_t* size,
    io_codec_ctx_t* array
)
{
    cmp_ctx_t* cmp_ctx = (cmp_ctx_t*)ctx->context;
    uint32_t arr_size;
    mpack_read_begin(ctx);
    if (!cmp_read_array(cmp_ctx, &arr_size))
        return cmp_ctx_get_err(cmp_ctx);
    array->codec = ctx->codec;
    array->context = ctx->context;
    array->default_allocator = ctx->default_allocator;
    array->user_context = ctx->user_context;
    array->log = ctx->log;
    array->index = *size = (size_t)arr_size;
    return er_ok;
}

//
// Initialize a context object from a stream
//
static int32_t mpack_init_ctx(
    io_codec_ctx_t* ctx,
    io_stream_t* stream,
    bool init_from_stream,
    log_t log
)
{
    cmp_ctx_t* cmp_ctx;
    (void)init_from_stream;
    cmp_ctx = mem_zalloc_type(cmp_ctx_t);
    if (!cmp_ctx)
        return er_out_of_memory;

    cmp_ctx->buf = stream;
    cmp_ctx->read = cmp_stream_read;
    cmp_ctx->skip = NULL; // Use read instead
    cmp_ctx->write = cmp_stream_write;

    ctx->context = cmp_ctx;
    ctx->default_allocator = NULL;
    ctx->user_context = NULL;
    ctx->log = log;
    ctx->index = 1;
    return er_ok;
}

//
// Finialize a context object to stream
//
static int32_t mpack_fini_ctx(
    io_codec_ctx_t* ctx,
    io_stream_t* stream,
    bool flush
)
{
    (void)stream;
    (void)flush;
    mem_free_type(cmp_ctx_t, ctx->context);
    return er_ok;
}

//
// Get the mpack codec
//
static io_codec_t* mpack_codec(
    void
)
{
    static io_codec_t codec = {
        io_codec_mpack,
        
        mpack_write_integer,
        mpack_write_uinteger,
        mpack_write_decimal,
        mpack_write_bool,
        mpack_write_string,
        mpack_write_bin,
        mpack_write_object,
        mpack_write_type_begin,
        mpack_write_type_end,
        mpack_write_array,

        mpack_read_integer,
        mpack_read_uinteger,
        mpack_read_decimal,
        mpack_read_bool,
        mpack_read_string,
        mpack_read_bin,
        mpack_read_object,
        mpack_read_type_begin,
        mpack_read_type_end,
        mpack_read_array,

        mpack_init_ctx,
        mpack_fini_ctx
    };
    return &codec;
}

//
// mem_alloc based allocator
//
static int32_t mem_allocator(
    io_codec_ctx_t* ctx,
    size_t size,
    void** mem,
    size_t* mem_size
)
{
    (void)ctx;
    if (size > 0)
    {
        *mem = mem_alloc(size);
        if (!*mem)
            return er_out_of_memory;
    }
    else
    {
        mem_free(*mem);
        *mem = NULL;
    }
    *mem_size = size;
    return er_ok;
}

//
// crt_alloc based allocator
//
static int32_t crt_allocator(
    io_codec_ctx_t* ctx,
    size_t size,
    void** mem,
    size_t* mem_size
)
{
    (void)ctx;
    if (size > 0)
    {
        *mem = crt_alloc(size);
        if (!*mem)
            return er_out_of_memory;
    }
    else
    {
        crt_free(*mem);
        *mem = NULL;
    }
    *mem_size = size;
    return er_ok;
}

//
// fixed buffer validator
//
static int32_t fixed_validator(
    io_codec_ctx_t* ctx,
    size_t size,
    void** mem,
    size_t* mem_size
)
{
    (void)mem;
    (void)ctx;
    if (*mem_size >= size)
    {
        *mem_size = size;
        return er_ok;
    }
    return er_out_of_memory;
}

//
// Read buffer
//
int32_t io_decode_bin_malloc(
    io_codec_ctx_t* ctx,
    const char* name,
    void** value,
    size_t* size
)
{
    int32_t result;
    void* alloc = NULL;
    result = io_decode_bin(ctx, name, mem_allocator, &alloc, size);
    if (result != er_ok)
    {
        if (alloc)
            mem_free(alloc);
        *size = 0;
        return result;
    }
    *value = alloc;
    return er_ok;
}

//
// Read buffer
//
int32_t io_decode_bin_default(
    io_codec_ctx_t* ctx,
    const char* name,
    void** value,
    size_t* size
)
{
    int32_t result;
    void* alloc = NULL;

    if (!ctx->default_allocator)
        return io_decode_bin_malloc(ctx, name, value, size);

    result = io_decode_bin(ctx, name, ctx->default_allocator, &alloc, size);
    if (result != er_ok)
    {
        if (alloc)
            ctx->default_allocator(ctx, 0, &alloc, size);
        *size = 0;
        return result;
    }
    *value = alloc;
    return er_ok;
}

//
// Read fixed string
//
int32_t io_decode_bin_fixed(
    io_codec_ctx_t* ctx,
    const char* name,
    void* val,
    size_t* len
)
{
    return io_decode_bin(ctx, name, fixed_validator, &val, len);
}

//
// Read string
//
int32_t io_decode_string_malloc(
    io_codec_ctx_t* ctx,
    const char* name,
    char** string
)
{
    size_t size;
    int32_t result;
    char* alloc = NULL;
    result = io_decode_string(ctx, name, crt_allocator, &alloc, &size);
    if (result != er_ok)
    {
        if (alloc)
            crt_free(alloc);
        return result;
    }
    *string = alloc;
    return er_ok;
}

//
// Read string
//
int32_t io_decode_STRING_HANDLE(
    io_codec_ctx_t* ctx,
    const char* name,
    STRING_HANDLE* decoded
)
{
    char* string_mem;
    int32_t result;
    STRING_HANDLE string;

    result = io_decode_string_malloc(ctx, name, &string_mem);
    if (result != er_ok)
        return result;

    string = STRING_new_with_memory(string_mem);
    if (!string)
    {
        crt_free(string_mem);
        return er_out_of_memory;
    }
    
    *decoded = string;
    return er_ok;
}

//
// Read string
//
int32_t io_decode_string_default(
    io_codec_ctx_t* ctx,
    const char* name,
    char** string
)
{
    size_t size;
    int32_t result;
    char* alloc = NULL;

    if (!ctx->default_allocator)
        return io_decode_string_malloc(ctx, name, string);

    result = io_decode_string(ctx, name, ctx->default_allocator, &alloc, &size);
    if (result != er_ok)
    {
        if (alloc)
            ctx->default_allocator(ctx, 0, (void**)&alloc, &size);
        return result;
    }
    *string = alloc;
    return er_ok;
}

//
// Read fixed string
//
int32_t io_decode_string_fixed(
    io_codec_ctx_t* ctx,
    const char* name,
    char* string,
    size_t len
)
{
    return io_decode_string(ctx, name, fixed_validator, &string, &len);
}

//
// Get the codec by id
//
io_codec_t* io_codec_by_id(
    io_codec_id_t id
)
{
    switch (id)
    {
    case io_codec_mpack:
        return mpack_codec();
    case io_codec_json:
        return json_codec();
    
        //...

    default:
        dbg_assert(0, "Unsupported codec id %d", id);
        break;
    }
    return NULL;
}

