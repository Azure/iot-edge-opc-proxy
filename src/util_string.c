// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os.h"

#include "util_mem.h"
#include "util_string.h"
#include "pal_rand.h"

#include <ctype.h>
#include <string.h>
#if !defined(_WIN32)
#include <strings.h>
#endif
#include <stdio.h>

#include "azure_c_shared_utility/base64.h"

//
// Parse a string and pass key and value to visitor
//
int32_t string_key_value_parser(
    const char * cstr,
    fn_parse_cb_t visitor,
    char delim,
    void* ctx
)
{
    int32_t result = er_ok;

    const char* key;
    size_t key_len;
    const char* val;
    size_t val_len;

    chk_arg_fault_return(cstr);
    chk_arg_fault_return(visitor);

    key = cstr;
    while(true)
    {
        while (*cstr != '\0'  && *cstr != '=')
            cstr++;
        key_len = (size_t)(cstr - key);
        if (*cstr == '\0' || cstr[1] == '\0')
        {
            result = visitor(ctx, key, key_len, cstr, 0);
            break;
        }

        val = ++cstr;
        while (*cstr != '\0' && *cstr != delim)
            cstr++;
        val_len = (size_t)(cstr - val);

        result = visitor(ctx, key, key_len, val, val_len);
        if (result != er_ok)
            break;

        if (*cstr == '\0' || cstr[1] == '\0')
            break;
        key = ++cstr;
    }

    return result;
}

//
// Create a copy of the STRING_HANDLE value
//
int32_t STRING_clone_c_str(
    STRING_HANDLE string,
    char** copy
)
{
    size_t len;
    chk_arg_fault_return(copy);
    chk_arg_fault_return(string);
    len = STRING_length(string);
    *copy = (char*)mem_alloc(len + 1);
    if (!*copy)
        return er_out_of_memory;
    if (0 == STRING_copy(string, *copy))
    {
        (*copy)[len] = 0;
        return er_ok;
    }
    mem_free(*copy);
    *copy = NULL;
    return er_fatal;
}

//
// Create a copy of the string
//
int32_t string_clone(
     const char* string,
     char** copy
)
{
    size_t len;
    chk_arg_fault_return(copy);
    chk_arg_fault_return(string);
    len = strlen(string);
    *copy = (char*)mem_alloc(len + 1);
    if (!*copy)
        return er_out_of_memory;
    strcpy(*copy, string);
    (*copy)[len] = 0;
    return er_ok;
}

//
// Check whether string starts with a string
//
bool string_starts_with_nocase(
    const char* val,
    const char* find
)
{
    size_t find_len;
    if (!val || !find)
        return false;
    find_len = strlen(find);
    if (find_len > strlen(val))
        return false;
    return string_is_equal_nocase(val, find_len, find);
}

//
// Remove the string from the value string
//
void string_remove_nocase(
    char* value,
    const char* find
)
{
    char* next = value;
    if (!value || !find)
        return;
    size_t val_len = strlen(value);
    size_t find_len = strlen(find);
    if (!val_len || !find_len)
        return;

    for (size_t remain = val_len; find_len <= remain; remain--)
    {
        if (string_is_equal_nocase(next, find_len, find))
        {
            val_len -= find_len;
            memmove(next, next + find_len, val_len);
            continue;
        }
        next++;
    }
    value[val_len] = 0;
}

//
// Find first occurrence in string
//
const char* string_find(
    const char* value,
    const char* find
)
{
    const char* next = value;
    if (!value || !find)
        return NULL;
    size_t val_len = strlen(value);
    size_t find_len = strlen(find);
    if (!val_len || !find_len)
        return NULL;

    while (find_len <= val_len)
    {
        if (string_is_equal(next, find_len, find))
            return next;

        next++;
        val_len--;
    }
    return NULL;
}

//
// Find first occurrence in string, case insensitive
//
const char* string_find_nocase(
    const char* value,
    const char* find
)
{
    const char* next = value;
    if (!value || !find)
        return NULL;
    size_t val_len = strlen(value);
    size_t find_len = strlen(find);
    if (!val_len || !find_len)
        return NULL;

    while (find_len <= val_len)
    {
        if (string_is_equal_nocase(next, find_len, find))
            return next;

        next++;
        val_len--;
    }
    return NULL;
}

//
// Trim scheme
//
const char* string_trim_scheme(
    const char* host_name,
    size_t* name_len
)
{
    size_t string_len;
    const char* delim = "://";
    if (!host_name)
        return NULL;

    if (!name_len)
    {
        string_len = strlen(host_name);
        name_len = &string_len;
    }

    if (*name_len < sizeof(delim))
        return host_name;

    for (size_t i = 0; i < *name_len - sizeof(delim); i++)
    {
        if (host_name[i]   == delim[0] && 
            host_name[i+1] == delim[1] && 
            host_name[i+2] == delim[2])
        {
            i += 3;
            *name_len -= i;
            return &host_name[i];
        }
    }
    return host_name;
}

//
// Returns new length after trimming the back
//
size_t string_trim_back_len(
    const char* val,
    size_t len,
    const char* trim_chars
)
{
    const char* trim;
    if (!val)
        return 0;
    if (!trim_chars)
        return len;
    while(len > 0)
    {
        trim = trim_chars;
        while (*trim && val[len - 1] != *trim)
            trim++;
        if (!*trim)
            break; // do not trim further
        --len;
    }
    return len;
}

//
// Compare to strings, case sensitive
//
int32_t string_compare(
    const char* val,
    const char* to
)
{
    if (!val)
        return -1;
    if (!to)
        return 1;
    return strcmp(val, to);
}

//
// Compare to strings, case insensitive
//
int32_t string_compare_nocase(
    const char* val,
    const char* to
)
{
    if (!val)
        return -1;
    if (!to)
        return 1;
    return 
#ifdef _WIN32
        _stricmp(val, to)
#else
        strcasecmp(val, to)
#endif
        ;
}

//
// Compares a non 0 terminated buffer to a 0 terminated string.
//
bool string_is_equal_nocase(
    const char* val,
    size_t len,
    const char* to
)
{
    if (!val && !to)
        return true;
    if (!val || !to)
        return false;
    return len == strlen(to) &&
#ifdef _WIN32
        0 == _strnicmp(val, to, len)
#else
        0 == strncasecmp(val, to, len)
#endif
        ;
}

//
// Copy int32_t to string
//
int32_t string_from_int(
    int32_t value,
    int32_t radix,
    char* string,
    size_t len
)
{
    chk_arg_fault_return(string);
    if (len == 0)
        return er_arg;
    if (radix != 10 && radix != 16)
        return er_arg;
    return
#ifdef _WIN32
        _itoa_s(value, string, len, radix) == 0
#else
        snprintf(string, len, (radix == 10) ? "%d" : "%x", value) > 0
#endif
        ? er_ok : er_arg;
}

//
// Parse int32_t from string
//
int32_t string_to_int(
    const char* string,
    int32_t radix,
    int32_t* value
)
{
    char* end_ptr;
    chk_arg_fault_return(string);
    chk_arg_fault_return(value);
    if (radix != 10 && radix != 16)
        return er_arg;
    
    *value = strtol(string, &end_ptr, radix);
    if (!*end_ptr)
        return er_ok;
    return er_arg;
}

//
// Case sensitive compare
//
bool string_is_equal(
    const char* val,
    size_t len,
    const char* to
)
{
    if (!val && !to)
        return true;
    if (!val || !to)
        return false;
    return len == strlen(to) && 0 == strncmp(val, to, len);
}

//
// Compare STRING to c string case sensitive
//
int32_t STRING_compare_c_str(
    STRING_HANDLE string,
    const char* compare_to
)
{
    chk_arg_fault_return(string);
    return string_compare(STRING_c_str(string), compare_to);
}

//
// Comnpare STRING to c string
//
int32_t STRING_compare_c_str_nocase(
    STRING_HANDLE string,
    const char* compare_to
)
{
    chk_arg_fault_return(string);
    return string_compare_nocase(STRING_c_str(string), compare_to);
}

//
// Comnpare STRING to c string
//
int32_t STRING_compare_nocase(
    STRING_HANDLE string,
    STRING_HANDLE compare_to
)
{
    chk_arg_fault_return(compare_to);
    return STRING_compare_c_str_nocase(string, STRING_c_str(compare_to));
}

// 
// Removes all characters in trim_chars from the start
//
const char* string_trim_front(
    const char* val,
    const char* trim_chars
)
{
    const char* trim;
    if (!val || !trim_chars)
        return val;
    while (*val)
    {
        trim = trim_chars;
        while (*trim && *val != *trim)
            trim++;
        if (!*trim)
            break; // Not found any char in trim_chars
        val++;
    }
    return val;
}

// 
// Removes all characters in trim_chars in the back
//
void string_trim_back(
    char* val,
    const char* trim_chars
)
{
    size_t len;
    if (!val || !trim_chars)
        return;
    len = strlen(val);
    if (!len)
        return;

    len = string_trim_back_len(val, len, trim_chars);
    val[len] = 0;
}

// 
// Removes all characters in trim_chars from the start and end
//
const char* string_trim(
    char* val,
    const char* trim_chars
)
{
    val = (char*)string_trim_front(val, trim_chars);
    string_trim_back(val, trim_chars);
    return val;
}

//
// Construct trimmed string
//
STRING_HANDLE STRING_construct_trim(
    const char* val,
    const char* trim_chars
)
{
    STRING_HANDLE result;
    if (!val)
        return NULL;
    if (!trim_chars || !*trim_chars)
        return STRING_construct(val);
    result = STRING_construct(string_trim_front(val, trim_chars));
    if (!result)
        return NULL;
    string_trim_back((char*)STRING_c_str(result), trim_chars);
    return result;
}

//
// Construct a 0 terminated string from the passed buffer
//
STRING_HANDLE STRING_safe_construct_n(
    const char* buffer,
    size_t len
)
{
    STRING_HANDLE result;
    char* copy;

    if (!buffer)
        return NULL;

    //
    // STRING_construct_n uses strlen on the buffer argument,
    // assuming it is a 0 terminated string, while we need to
    // form strings from plain non terminated memory.
    //

    copy = (char*)crt_alloc(len + 1);
    if (!copy)
        return NULL;

    memcpy(copy, buffer, len);
    copy[len] = 0;
    
    result = STRING_new_with_memory(copy);
    if (result)
        return result;
    crt_free(copy);
    return NULL;
}

//
// Concat a buffer up to len chars
//
int32_t STRING_concat_n(
    STRING_HANDLE string,
    const char* buffer,
    size_t len
)
{
    int32_t result = er_ok;
    STRING_HANDLE tmp;

    chk_arg_fault_return(string);
    chk_arg_fault_return(buffer);

    tmp = STRING_safe_construct_n(buffer, len);
    if (!tmp)
        return er_out_of_memory;

    if (0 != STRING_concat_with_STRING(string, tmp))
        result = er_out_of_memory;
    
    STRING_delete(tmp);
    return result;
}

//
// Concat an integer value
//
int32_t STRING_concat_int(
    STRING_HANDLE string,
    int32_t value,
    int32_t radix
)
{
    int32_t result;
    char buffer[32];
    chk_arg_fault_return(string);
    result = string_from_int(value, radix, buffer, sizeof(buffer));
    if (result == er_ok)
        result = STRING_concat(string, buffer) == 0 ? er_ok : er_out_of_memory;
    return result;
}

//
// Find STRING in STRING 
//
const char* STRING_find(
    STRING_HANDLE string,
    STRING_HANDLE to_find
)
{
    if (!to_find)
        return NULL;
    return STRING_find_c_str(string, STRING_c_str(to_find));
}

//
// Find STRING in STRING case insensitive
//
const char* STRING_find_nocase(
    STRING_HANDLE string,
    STRING_HANDLE to_find
)
{
    if (!to_find)
        return NULL;
    return STRING_find_c_str_nocase(string, STRING_c_str(to_find));
}

//
// Find c string in STRING 
//
const char* STRING_find_c_str(
    STRING_HANDLE string,
    const char* to_find
)
{
    if (!string)
        return NULL;
    return string_find(STRING_c_str(string), to_find);
}

//
// Find c string in STRING  case insensitive
//
const char* STRING_find_c_str_nocase(
    STRING_HANDLE string,
    const char* to_find
)
{
    if (!string)
        return NULL;
    return string_find_nocase(STRING_c_str(string), to_find);
}

//
// Update the value of a string
//
int32_t STRING_update(
    STRING_HANDLE string,
    const char* val
)
{
    chk_arg_fault_return(string);
    chk_arg_fault_return(val);

    if (0 == STRING_compare_c_str(string, val))
        return er_ok;

    if (0 != STRING_empty(string) || 0 != STRING_concat(string, val))
        return er_out_of_memory;

    return er_ok;
}

//
// Construct a utf8 conforming string from a raw char* buffer
//
STRING_HANDLE STRING_construct_utf8(
    const uint8_t* buf,
    size_t buf_len
)
{
    char* string_buf;
    uint8_t* s;
    size_t alloc;
    
    if (!buf || buf_len <= 0)
        return NULL;
    alloc = buf_len;
    for (size_t i = 0; i < buf_len; i++)
    {
        if (buf[i] > 0x7f) 
            alloc++;
    }
    s = (uint8_t*)crt_alloc(alloc + 1);
    string_buf = (char*)s;
    if (!string_buf)
        return NULL;
    for (size_t i = 0; i < buf_len; i++)
    {
        if (buf[i] <= 0x7f)
            *s++ = buf[i];
        else
        {
            // 110xxxxx
            *s++ = (uint8_t)((0xff << 6) | (buf[i] >> 6));
            // 10xxxxxx
            *s++ = (uint8_t)(0x80 | (buf[i] & 0x3f));
        }
    }
    *s = 0;
    return STRING_new_with_memory(string_buf);
}

//
// Helper to encode a buffer as base16
//
void string_from_buf(
    const uint8_t* buf, // must be at least half the size of len
    char* string,
    size_t len,
    bool swap
)
{
    uint8_t c;

#define _byte_to_string(i) \
        c = (buf[(i)] & 0xf0) >> 4; \
        *string++ = (c < 10) ? ('0' + c) : ('a' + c) - 10; \
        c = (buf[(i)] & 0x0f); \
        *string++ = (c < 10) ? ('0' + c) : ('a' + c) - 10

    len /= 2;
    if (swap)
        for (size_t i = len; i > 0; i--)
        {
            _byte_to_string(i - 1);
        }
    else
        for (size_t i = 0; i < len; i++)
        {
            _byte_to_string(i);
        }
}

//
// Helper to encode a buffer as base16
//
int32_t string_to_buf(
    const char* string,
    uint8_t* buf,  
    size_t len,        // must be at least half the size of string len
    bool swap
)
{
    char c;

#define _string_to_byte(i) \
        c = *string++; \
        /**/ if (c >= '0' && c <= '9') \
            buf[i] = (c - '0') << 4; \
        else if (c >= 'a' && c <= 'f') \
            buf[i] = (10 + (c - 'a')) << 4; \
        else if (c >= 'A' && c <= 'F') \
            buf[i] = (10 + (c - 'A')) << 4; \
        else \
            return er_invalid_format; \
        c = *string++; \
        /**/ if (c >= '0' && c <= '9') \
            buf[i] |= (c - '0'); \
        else if (c >= 'a' && c <= 'f') \
            buf[i] |= (10 + (c - 'a')); \
        else if (c >= 'A' && c <= 'F') \
            buf[i] |= (10 + (c - 'A')); \
        else \
            return er_invalid_format

    if (swap)
        for (size_t i = len; i > 0; i--)
        {
            _string_to_byte(i - 1);
        }
    else
        for (size_t i = 0; i < len; i++)
        {
            _string_to_byte(i);
        }
    return er_ok;
}

//
// Convert base16 string to byte buffer
//
int32_t string_base16_to_byte_array(
    const char* val,
    uint8_t** buffer,
    size_t* len
)
{
    int32_t result;
    size_t min_len;
    chk_arg_fault_return(buffer);
    chk_arg_fault_return(len);
    chk_arg_fault_return(val);
    min_len = (strlen(val) / 2);
    if (!min_len)
        return er_arg;
    if (*len && *len < min_len)
        return er_fault;
    if (!*buffer || !*len)
        *buffer = (uint8_t*)mem_alloc(min_len);
    *len = min_len;
    if (!*buffer)
        return er_out_of_memory;
    result = string_to_buf(val, *buffer, *len, false);
    if (result == er_ok)
        return er_ok;
    mem_free(*buffer);
    return result;
}

//
// Convert base64 string to byte buffer
//
int32_t string_base64_to_byte_array(
    const char* val,
    uint8_t** buffer,
    size_t* len
)
{
    int32_t result;
    BUFFER_HANDLE handle;
    chk_arg_fault_return(buffer);
    chk_arg_fault_return(len);
    chk_arg_fault_return(val);

    handle = Base64_Decoder(val);
    if (!handle)
        return er_out_of_memory;
    do
    {
        *len = BUFFER_length(handle);
        *buffer = (uint8_t*)mem_alloc(*len);
        if (!*buffer)
        {
            result = er_out_of_memory;
            break;
        }
        memcpy(*buffer, BUFFER_u_char(handle), *len);
        result = er_ok;
        break;
    } 
    while (0);
    BUFFER_delete(handle);
    return result;
}

//
// Parse a uuid into a byte buffer
//
int32_t string_to_uuid(
    const char* string,
    uint8_t* uuid
)
{
    int32_t result;
    chk_arg_fault_return(string);
    chk_arg_fault_return(uuid);
    if (strlen(string) + 1 < UUID_PRINTABLE_STRING_LENGTH)
        return er_arg;

    // 8-4-4-4-12
    result = string_to_buf(string, uuid, 4, true);
    if (result != er_ok)
        return result;
    string += 8;  uuid += 4;
    if (*string++ != '-')
        return er_invalid_format;

    for (int i = 0; i < 3; i++)
    {
    result = string_to_buf(string, uuid, 2, i != 2);
    if (result != er_ok)
        return result;
    string += 4;  uuid += 2;
    if (*string++ != '-')
        return er_invalid_format;
    }

    result = string_to_buf(string, uuid, 6, false);
    if (result != er_ok)
        return result;
    return er_ok;
}

//
// Generate a uuid formatted string from a 16 bit byte buffer
//
int32_t string_from_uuid(
    const uint8_t* uuid,
    char* string,
    size_t len            // must be >= UUID_PRINTABLE_STRING_LENGTH
)
{
    chk_arg_fault_return(string);
    chk_arg_fault_return(uuid);
    if (len < UUID_PRINTABLE_STRING_LENGTH)
        return er_arg;
    // 8-4-4-4-12
    string_from_buf(uuid, string, 8, true);
    string += 8;  uuid += 4;
    *string++ = '-';
    for (int i = 0; i < 3; i++)
    {
    string_from_buf(uuid, string, 4, i != 2);
    string += 4;  uuid += 2;
    *string++ = '-';
    }
    string_from_buf(uuid, string, 12, false);
    string += 12; 
    *string++ = '\0';
    return er_ok;
}

//
// Construct a random uuid string
//
STRING_HANDLE STRING_construct_uuid(
    void
)
{
    int32_t result;
    STRING_HANDLE string;
    uint8_t uuid[16];
    char* buffer;

    buffer = (char*)crt_alloc(UUID_PRINTABLE_STRING_LENGTH);
    if (!buffer)
        return NULL;
    do
    {
        result = pal_rand_fill(uuid, sizeof(uuid));
        if (result != er_ok)
            break;
        result = string_from_uuid(
            uuid, buffer, UUID_PRINTABLE_STRING_LENGTH);
        if (result != er_ok)
            break;
        string = STRING_new_with_memory(buffer);
        if (!string)
            break;

        return string;
    } while (0);
    crt_free(buffer);
    return NULL;
}

//
// Construct a base64 conforming string from a raw char* buffer
//
STRING_HANDLE STRING_construct_base64(
    const uint8_t* buf,
    size_t buf_len
)
{
    return Base64_Encode_Bytes((const unsigned char*)buf, buf_len);
}

//
// Construct a base16 conforming string from a raw char* buffer
//
STRING_HANDLE STRING_construct_base16(
    const uint8_t* buf,
    size_t len
)
{
    char* buffer;
    if (!len || !buf)
        return NULL;
    len *= 2;
    buffer = (char*)crt_alloc(len + 1);
    if (!buffer)
        return NULL;
    string_from_buf(buf, buffer, len, false);
    buffer[len] = 0;
    return STRING_new_with_memory(buffer);
}

// 
// Construct a random ascii string 
// 
STRING_HANDLE STRING_construct_random(
    size_t len
)
{
    int32_t result;
    char* buffer;
size_t buf_len;

if (!len)
return NULL;

buf_len = (len + 1) / 2;
buffer = (char*)crt_alloc(len + 2 + buf_len);
if (buffer)
{
    result = pal_rand_fill(
        (unsigned char*)&buffer[len + 2], buf_len);
    if (result == er_ok)
    {
        // Format the buffer as base16 and terminate 
        string_from_buf((unsigned char*)&buffer[len + 2],
            buffer, (len + 1), false);
        buffer[len] = 0;
        return STRING_new_with_memory(buffer);
    }
    crt_free(buffer);
}
return NULL;
}

//
// Make a valid service name from its components
//
int32_t string_copy_service_full_name(
    const char* service_name,
    const char* service_type,
    const char* domain,
    char* full_name,
    size_t full_size
)
{
    bool trailing_dot;

    chk_arg_fault_return(full_name);
    if (service_name && domain && !service_type)
        return er_arg;
    if (full_size <= 1)
        return er_arg;
    if ((service_name || service_type) && !domain)
        domain = "local";

    trailing_dot = false; // Did we see a trailing dot?
    if (service_name && *service_name)
    {
        // Service name is assumed to be free form, so it
        // might end with a . and contain spaces, etc. 
        while (*service_name)
        {
            if (!--full_size) return er_fault;
            *full_name++ = *service_name++;
        }

        if (!--full_size) return er_fault;
        *full_name++ = '.';
        trailing_dot = true;
    }
    if (service_type && *service_type)
    {
        while (*service_type)
        {
            if (!--full_size) return er_fault;
            trailing_dot = *service_type == '.';
            *full_name++ = *service_type++;
        }
        if (!trailing_dot)
        {
            if (!--full_size) return er_fault;
            *full_name++ = '.';
            trailing_dot = true;
        }
    }
    if (domain && *domain)
    {
        while (*domain)
        {
            if (!--full_size) return er_fault;
            trailing_dot = *domain == '.';
            *full_name++ = *domain++;
        }
        // No need to add a trailing dot
    }
    if (trailing_dot)
        full_name--;
    *full_name = 0;
    return er_ok;
}


//
// Parse a range list in the form of 5;6;4-9;10
//
int32_t string_parse_range_list(
    const char* range_string,
    int32_t** tuples,
    size_t* len
)
{
    const char* ptr, *in;
    int32_t* range_list = NULL;
    int32_t low, high;
    size_t index;

    chk_arg_fault_return(range_string);
    if ((len && !tuples) || (tuples && !len))
        return er_fault;
    //
    // First round validate and count, then allocate and assign
    // This is not efficient, but makes the code shorter. This
    // is only called twice, so it should be fine.
    //
    while(true) 
    {
        // Validate range list and count items for alloc
        index = 0;
        in = ptr = range_string;
        while (true)
        {
            low = high = strtol(in, (char**)&ptr, 10);
            if (*ptr == '-')
            {
                in = ptr + 1;
                high = strtol(in, (char**)&ptr, 10);
                if (ptr == in)
                    return er_invalid_format;
            }
            if (range_list)
            {
                // Enforce order for reverse ranges (e.g. 9-3)
                *range_list++ = high > low ? low : high;
                *range_list++ = high > low ? high : low;
            }
            if (ptr != in)
                index++; // Found a valid item
            if (!*ptr)
                break;  // reached end
            else if (*ptr == ' ')
            { 
                // Skip trailing while space
                while (*ptr == ' ') 
                    ptr++;
                // But inside its not allowed...
                if (!*ptr)
                    break;
                return er_invalid_format;
            }
            else if (*ptr != ';' || ptr == in)
                return er_invalid_format;
            in = ptr + 1; // Skip ;
        }

        // if called to validate or if we filled the list we are done
        if ((!tuples && !len) || range_list)
            break;

        if (index == 0)
        {
            *tuples = NULL;
            *len = 0;
            return er_ok;
        }
        // Allocate range list tuples
        range_list = (int32_t*)mem_alloc(2 * index * sizeof(int32_t));
        if (!range_list)
            return er_out_of_memory;
        *tuples = range_list;
        *len = index;
    }
    return er_ok;
}

//
// Break a service name into its components
//
int32_t string_parse_service_full_name(
    char* full_name,
    char** service_name,
    char** service_type,
    char** domain
)
{
    chk_arg_fault_return(full_name);
    chk_arg_fault_return(service_name);
    chk_arg_fault_return(service_type);
    chk_arg_fault_return(domain);

    *service_name = NULL;
    *service_type = NULL;
    *domain = NULL;

    // Remove trailing .
    string_trim_back(full_name, ".");
    if (!*full_name)
        return er_ok;

    // Check if this is just a service type - no name
    if (*full_name == '_')
        *service_type = full_name;
    else
    {
        *service_name = full_name;
        // Find the first "." which is followed by "_"
        while (*++full_name)
        {
            if (full_name[0] == '.')
            {
                if (full_name[1] &&
                    full_name[1] == '_')
                {
                    // Found service type - break
                    *full_name++ = 0;
                    *service_type = full_name;
                    break;
                }

                if (!*domain)  // Save away possible domain
                    *domain = full_name;
            }
        }

        // No service type found - assume this is a domain.
        if (!*service_type)
        {
       //
       // The following would parse host.domain, but this 
       // is not a valid service full name, which must 
       // always have a service type.  
       // 
       //     full_name = *domain;
       //     if (full_name)
       //     {
       //         // *domain points to first label
       //         *full_name++ = 0;
       //         *domain = full_name;
       //         return er_ok;
       //     }
       //
            *domain = *service_name;
            *service_name = NULL;
            return er_ok;
        }
    }
    //
    // full_name now points to service type - find the 
    // first "." that is not followed by "_" and use 
    // the string that follows as domain. If we cannot 
    // find one, assume domain is "local"
    //
    while (*++full_name)
    {
        if (full_name[0] == '.' &&
            full_name[1] &&
            full_name[1] != '_')
        {
            *full_name++ = 0;
            *domain = full_name;
            return er_ok;
        }
    }
    *domain = "local";
    return er_ok;
}

