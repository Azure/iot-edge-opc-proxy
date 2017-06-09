// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _util_string_h_
#define _util_string_h_

#include "common.h"
#include "azure_c_shared_utility/strings.h"

//
// Safe version, can take non 0 terminated string
//
decl_internal_2(STRING_HANDLE, STRING_safe_construct_n,
    const char*, buffer,
    size_t, len
);

//
// Construct trimmed string
//
decl_internal_2(STRING_HANDLE, STRING_construct_trim,
    const char*, val,
    const char*, trim_chars
);

//
// Construct a random ascii string
//
decl_internal_1(STRING_HANDLE, STRING_construct_random,
    size_t, len
);

//
// Construct a random uuid string
//
decl_internal_0(STRING_HANDLE, STRING_construct_uuid,
    void
);

//
// Construct a utf8 conforming string from a raw buffer
//
decl_internal_2(STRING_HANDLE, STRING_construct_utf8,
    const uint8_t*, buf,
    size_t, buf_len
);

//
// Construct a base64 conforming string from a raw buffer
//
decl_internal_2(STRING_HANDLE, STRING_construct_base64,
    const uint8_t*, buf,
    size_t, buf_len
);

//
// Construct a base16 encoded string from a raw buffer
//
decl_internal_2(STRING_HANDLE, STRING_construct_base16,
    const uint8_t*, buf,
    size_t, buf_len
);

//
// Concat a buffer up to len chars
//
decl_internal_3(int32_t, STRING_concat_n,
    STRING_HANDLE, string,
    const char*, buffer,
    size_t, len
);

//
// Concat an integer value
//
decl_internal_3(int32_t, STRING_concat_int,
    STRING_HANDLE, string,
    int32_t, value,
    int32_t, radix
);

//
// Update the value of a string
//
decl_internal_2(int32_t, STRING_update,
    STRING_HANDLE, string,
    const char*, val
);

//
// Comnpare STRING to c string
//
decl_internal_2(int32_t, STRING_compare_nocase,
    STRING_HANDLE, string,
    STRING_HANDLE, compare_to
);

//
// Compare STRING to c string case sensitive
//
decl_internal_2(int32_t, STRING_compare_c_str,
    STRING_HANDLE, string,
    const char*, compare_to
);

//
// Comnpare STRING to c string
//
decl_internal_2(int32_t, STRING_compare_c_str_nocase,
    STRING_HANDLE, string,
    const char*, compare_to
);

//
// Find STRING in STRING 
//
decl_internal_2(const char*, STRING_find,
    STRING_HANDLE, string,
    STRING_HANDLE, to_find
);

//
// Find STRING in STRING case insensitive
//
decl_internal_2(const char*, STRING_find_nocase,
    STRING_HANDLE, string,
    STRING_HANDLE, to_find
);

//
// Find c string in STRING 
//
decl_internal_2(const char*, STRING_find_c_str,
    STRING_HANDLE, string,
    const char*, to_find
);

//
// Find c string in STRING  case insensitive
//
decl_internal_2(const char*, STRING_find_c_str_nocase,
    STRING_HANDLE, string,
    const char*, to_find
);

//
// Create a copy of the string
//
decl_internal_2(int32_t, STRING_clone_c_str,
    STRING_HANDLE, string,
    char**, copy
);

//
// Create a copy of the string
//
decl_internal_2(int32_t, string_clone,
    const char*, string,
    char**, copy
);

//
// Copy int32_t to string
//
decl_internal_4(int32_t, string_from_int,
    int32_t, value,
    int32_t, radix,
    char*, string,
    size_t, len
);

//
// Parse int32_t from string
//
decl_internal_3(int32_t, string_to_int,
    const char*, string,
    int32_t, radix,
    int32_t*, value
);

//
// Generate string from uuid
//
#define UUID_PRINTABLE_STRING_LENGTH 37
decl_public_3(int32_t, string_from_uuid,
    const uint8_t*, uuid,
    char*, string,
    size_t, len  // must be >= UUID_PRINTABLE_STRING_LENGTH
);

//
// Parse uuid
//
decl_public_2(int32_t, string_to_uuid,
    const char*, string,
    uint8_t*, uuid
);

//
// Trim scheme
//
decl_internal_2(const char*, string_trim_scheme,
    const char*, host_name,
    size_t*, name_len
);

//
// Returns new length after trimming the back
//
decl_internal_3(size_t, string_trim_back_len,
    const char*, val,
    size_t, len,
    const char*, trim_chars
);

//
// Compare up to len chars to 'to' case insensitive
//
decl_internal_3(bool, string_is_equal_nocase,
    const char*, val,
    size_t, len,
    const char*, to
);

//
// Case sensitive compare
//
decl_internal_3(bool, string_is_equal,
    const char*, val,
    size_t, len,
    const char*, to
);

//
// Compare to strings, case sensitive
//
decl_internal_2(int32_t, string_compare,
    const char*, val,
    const char*, to
);

//
// Compare to strings, case insensitive
//
decl_internal_2(int32_t, string_compare_nocase,
    const char*, val,
    const char*, to
);

//
// Check whether string starts with a string
//
decl_internal_2(bool, string_starts_with_nocase,
    const char*, val,
    const char*, find
);

//
// Remove the string from the value string
//
decl_internal_2(void, string_remove_nocase,
    char*, value,
    const char*, to_remove
);

//
// Find first occurrence in string
//
decl_internal_2(const char*, string_find,
    const char*, value,
    const char*, to_find
);

//
// Find first occurrence in string, case insensitive
//
decl_internal_2(const char*, string_find_nocase,
    const char*, value,
    const char*, to_find
);

// 
// Removes all characters in trim_chars from the start
//
decl_internal_2(const char*, string_trim_front,
    const char*, val,
    const char*, trim_chars
);

// 
// Removes all characters in trim_chars in the back
//
decl_internal_2(void, string_trim_back,
    char*, val,
    const char*, trim_chars
);

// 
// Removes all characters in trim_chars from the start and end
//
decl_internal_2(const char*, string_trim,
    char*, val,
    const char*, trim_chars
);

//
// Utility to parse a connection string
//
typedef int32_t(*fn_parse_cb_t) (
    void* ctx,
    const char* key,
    size_t key_len,
    const char* val,
    size_t val_len
);

decl_internal_4(int32_t, string_key_value_parser,
    const char*, connection_string,
    fn_parse_cb_t, visitor,
    char, delim,
    void*, ctx
);

//
// Parse a service name into its components inline
//
decl_internal_4(int32_t, string_parse_service_full_name,
    char*, full_name,
    char**, service_name,
    char**, service_type,
    char**, domain
);

//
// Parse a range list in the form of 5;6;4-9;10
//
decl_internal_3(int32_t, string_parse_range_list,
    const char*, range_string,
    int32_t**, range_tuples,
    size_t*, range_tuple_count
);

//
// Make a valid service name from its components
//
decl_internal_5(int32_t, string_copy_service_full_name,
    const char*, service_name,
    const char*, service_type,
    const char*, domain,
    char*, full_name,
    size_t, full_size
);

//
// Decode base64 into bytes 
//
decl_internal_3(int32_t, string_base64_to_byte_array,
    const char*, val,
    uint8_t**, buffer,
    size_t*, len
);

//
// Decode base16 into bytes - memallocs if *buffer=*len=0
//
decl_internal_3(int32_t, string_base16_to_byte_array,
    const char*, val,
    uint8_t**, buffer,
    size_t*, len
);

#endif // _util_string_h_