// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST io_codec
#include "util_ut.h"

//
// 1. Required mocks
//
#include "util_log.h"
#include "azure_c_shared_utility/strings.h"

#include "parson.h"

MOCKABLE_FUNCTION(, JSON_Value*, json_parse_string_with_comments, const char*, string);
MOCKABLE_FUNCTION(, size_t, json_serialization_size, const JSON_Value*, value);
MOCKABLE_FUNCTION(, size_t, json_serialization_size_pretty, const JSON_Value*, value);
MOCKABLE_FUNCTION(, JSON_Status, json_serialize_to_buffer, const JSON_Value*, value, char*, buf, size_t, buf_size_in_bytes);
MOCKABLE_FUNCTION(, JSON_Status, json_serialize_to_buffer_pretty, const JSON_Value*, value, char*, buf, size_t, buf_size_in_bytes);
MOCKABLE_FUNCTION(, JSON_Value*, json_object_get_value, const JSON_Object*, object, const char*, name);
MOCKABLE_FUNCTION(, JSON_Status, json_object_set_value, JSON_Object*, object, const char*, name, JSON_Value*, value);
MOCKABLE_FUNCTION(, JSON_Status, json_object_set_string, JSON_Object*, object, const char*, name, const char*, string);
MOCKABLE_FUNCTION(, JSON_Status, json_object_set_number, JSON_Object*, object, const char*, name, double, number);
MOCKABLE_FUNCTION(, JSON_Status, json_object_set_boolean, JSON_Object*, object, const char*, name, int, boolean);
MOCKABLE_FUNCTION(, JSON_Status, json_object_set_null, JSON_Object*, object, const char*, name);
MOCKABLE_FUNCTION(, JSON_Value*, json_array_get_value, const JSON_Array*, array, size_t, index);
MOCKABLE_FUNCTION(, size_t, json_array_get_count, const JSON_Array*, array);
MOCKABLE_FUNCTION(, JSON_Status, json_array_append_string, JSON_Array*, array, const char*, string);
MOCKABLE_FUNCTION(, JSON_Status, json_array_append_number, JSON_Array*, array, double, number);
MOCKABLE_FUNCTION(, JSON_Status, json_array_append_boolean, JSON_Array*, array, int, boolean);
MOCKABLE_FUNCTION(, JSON_Status, json_array_append_value, JSON_Array*, array, JSON_Value*, value);
MOCKABLE_FUNCTION(, JSON_Status, json_array_append_null, JSON_Array*, array);
MOCKABLE_FUNCTION(, JSON_Value*, json_value_init_object);
MOCKABLE_FUNCTION(, JSON_Value*, json_value_init_array);
MOCKABLE_FUNCTION(, JSON_Value*, json_value_init_null);
MOCKABLE_FUNCTION(, JSON_Value_Type, json_value_get_type, const JSON_Value*, value);
MOCKABLE_FUNCTION(, JSON_Object*,json_value_get_object, const JSON_Value*, value);
MOCKABLE_FUNCTION(, JSON_Array*,json_value_get_array, const JSON_Value*, value);
MOCKABLE_FUNCTION(, const char*, json_value_get_string, const JSON_Value*, value);
MOCKABLE_FUNCTION(, double, json_value_get_number, const JSON_Value*, value);
MOCKABLE_FUNCTION(, int, json_value_get_boolean, const JSON_Value*, value);
MOCKABLE_FUNCTION(, void, json_value_free, JSON_Value*, value);

#include "cmp.h"

MOCKABLE_FUNCTION(, const char*, cmp_strerror, cmp_ctx_t*, ctx);
MOCKABLE_FUNCTION(, bool, cmp_write_integer, cmp_ctx_t*, ctx, int64_t, d);
MOCKABLE_FUNCTION(, bool, cmp_write_uinteger, cmp_ctx_t*, ctx, uint64_t, u);
MOCKABLE_FUNCTION(, bool, cmp_write_decimal, cmp_ctx_t*, ctx, double, d);
MOCKABLE_FUNCTION(, bool, cmp_write_nil, cmp_ctx_t*, ctx);
MOCKABLE_FUNCTION(, bool, cmp_write_bool, cmp_ctx_t*, ctx, bool, b);
MOCKABLE_FUNCTION(, bool, cmp_write_str, cmp_ctx_t*, ctx, const char*, data, uint32_t, size);
MOCKABLE_FUNCTION(, bool, cmp_write_bin, cmp_ctx_t*, ctx, const void*, data, uint32_t, size);
MOCKABLE_FUNCTION(, bool, cmp_write_array, cmp_ctx_t*, ctx, uint32_t, size);
MOCKABLE_FUNCTION(, bool, cmp_read_uinteger, cmp_ctx_t*, ctx, uint64_t*, u);
MOCKABLE_FUNCTION(, bool, cmp_read_integer, cmp_ctx_t*, ctx, int64_t*, u);
MOCKABLE_FUNCTION(, bool, cmp_read_decimal, cmp_ctx_t*, ctx, double*, d);
MOCKABLE_FUNCTION(, bool, cmp_read_nil, cmp_ctx_t*, ctx);
MOCKABLE_FUNCTION(, bool, cmp_read_bool, cmp_ctx_t*, ctx, bool*, b);
MOCKABLE_FUNCTION(, bool, cmp_read_str_size, cmp_ctx_t*, ctx, uint32_t*, size);
MOCKABLE_FUNCTION(, bool, cmp_read_bin_size, cmp_ctx_t*, ctx, uint32_t*, size);
MOCKABLE_FUNCTION(, bool, cmp_read_object, cmp_ctx_t*, ctx, cmp_object_t*, obj);
MOCKABLE_FUNCTION(, bool, cmp_read_array, cmp_ctx_t*, ctx, uint32_t*, size);
MOCKABLE_FUNCTION(, bool, cmp_skip_object, cmp_ctx_t*, ctx, cmp_object_t*, obj);

//
// 2. Include unit under test
//
#undef ENABLE_MOCKS
#include UNIT_H
#define ENABLE_MOCKS
#include UNIT_C

//
// 3. Setup test suite
//
BEGIN_DECLARE_TEST_SUITE()
END_DECLARE_TEST_SUITE()
// -or- DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
DECLARE_TEST_SETUP()

#ifdef io_codec_ctx_init //
// Test io_codec_ctx_init happy path
//
TEST_FUNCTION(io_codec_ctx_init__success)
{
    static const io_codec_t* k_codec_valid;
    static const io_codec_ctx_t* k_ctx_valid;
    static const io_stream_t* k_stream_valid;
    static const bool k_init_from_stream_valid;
    static const zlog_t k_log_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_codec_ctx_init(k_codec_valid, k_ctx_valid, k_stream_valid, k_init_from_stream_valid, k_log_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_codec_ctx_init passing as codec argument an invalid io_codec_t* value
//
TEST_FUNCTION(io_codec_ctx_init__arg_codec_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_codec_ctx_init();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_codec_ctx_init passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_codec_ctx_init__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_codec_ctx_init();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_codec_ctx_init passing as stream argument an invalid io_stream_t* value
//
TEST_FUNCTION(io_codec_ctx_init__arg_stream_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_codec_ctx_init();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_codec_ctx_init passing as init_from_stream argument an invalid bool value
//
TEST_FUNCTION(io_codec_ctx_init__arg_init_from_stream_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_codec_ctx_init();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_codec_ctx_init passing as log argument an invalid zlog_t value
//
TEST_FUNCTION(io_codec_ctx_init__arg_log_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_codec_ctx_init();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_codec_ctx_init unhappy path
//
TEST_FUNCTION(io_codec_ctx_init__neg)
{
    static const io_codec_t* k_codec_valid;
    static const io_codec_ctx_t* k_ctx_valid;
    static const io_stream_t* k_stream_valid;
    static const bool k_init_from_stream_valid;
    static const zlog_t k_log_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_codec_ctx_init(k_codec_valid, k_ctx_valid, k_stream_valid, k_init_from_stream_valid, k_log_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_codec_ctx_init

#ifdef io_codec_ctx_get_codec_id //
// Test io_codec_ctx_get_codec_id happy path
//
TEST_FUNCTION(io_codec_ctx_get_codec_id__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    io_codec_id_t result;

    // arrange
    // ...

    // act
    result = io_codec_ctx_get_codec_id(k_ctx_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(io_codec_id_t, er_ok, result);
    // ...
}

//
// Test io_codec_ctx_get_codec_id passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_codec_ctx_get_codec_id__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_codec_ctx_get_codec_id();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_codec_ctx_get_codec_id unhappy path
//
TEST_FUNCTION(io_codec_ctx_get_codec_id__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    io_codec_id_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_codec_ctx_get_codec_id(k_ctx_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(io_codec_id_t, result, er_ok);
}

#endif // io_codec_ctx_get_codec_id

#ifdef io_codec_ctx_fini //
// Test io_codec_ctx_fini happy path
//
TEST_FUNCTION(io_codec_ctx_fini__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const io_stream_t* k_stream_valid;
    static const bool k_flush_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_codec_ctx_fini(k_ctx_valid, k_stream_valid, k_flush_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_codec_ctx_fini passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_codec_ctx_fini__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_codec_ctx_fini();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_codec_ctx_fini passing as stream argument an invalid io_stream_t* value
//
TEST_FUNCTION(io_codec_ctx_fini__arg_stream_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_codec_ctx_fini();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_codec_ctx_fini passing as flush argument an invalid bool value
//
TEST_FUNCTION(io_codec_ctx_fini__arg_flush_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_codec_ctx_fini();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_codec_ctx_fini unhappy path
//
TEST_FUNCTION(io_codec_ctx_fini__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const io_stream_t* k_stream_valid;
    static const bool k_flush_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_codec_ctx_fini(k_ctx_valid, k_stream_valid, k_flush_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_codec_ctx_fini

#ifdef io_encode_int64 //
// Test io_encode_int64 happy path
//
TEST_FUNCTION(io_encode_int64__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const int64_t k_value_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_encode_int64(k_ctx_valid, k_name_valid, k_value_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_encode_int64 passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_encode_int64__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_encode_int64();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_encode_int64 passing as name argument an invalid const char* value
//
TEST_FUNCTION(io_encode_int64__arg_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_encode_int64();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_encode_int64 passing as value argument an invalid int64_t value
//
TEST_FUNCTION(io_encode_int64__arg_value_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_encode_int64();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_encode_int64 unhappy path
//
TEST_FUNCTION(io_encode_int64__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const int64_t k_value_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_int64(k_ctx_valid, k_name_valid, k_value_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_encode_int64

#ifdef io_encode_uint64 //
// Test io_encode_uint64 happy path
//
TEST_FUNCTION(io_encode_uint64__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const uint64_t k_value_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_encode_uint64(k_ctx_valid, k_name_valid, k_value_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_encode_uint64 passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_encode_uint64__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_encode_uint64();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_encode_uint64 passing as name argument an invalid const char* value
//
TEST_FUNCTION(io_encode_uint64__arg_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_encode_uint64();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_encode_uint64 passing as value argument an invalid uint64_t value
//
TEST_FUNCTION(io_encode_uint64__arg_value_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_encode_uint64();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_encode_uint64 unhappy path
//
TEST_FUNCTION(io_encode_uint64__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const uint64_t k_value_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_uint64(k_ctx_valid, k_name_valid, k_value_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_encode_uint64

#ifdef io_encode_double //
// Test io_encode_double happy path
//
TEST_FUNCTION(io_encode_double__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const double k_value_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_encode_double(k_ctx_valid, k_name_valid, k_value_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_encode_double passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_encode_double__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_encode_double();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_encode_double passing as name argument an invalid const char* value
//
TEST_FUNCTION(io_encode_double__arg_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_encode_double();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_encode_double passing as value argument an invalid double value
//
TEST_FUNCTION(io_encode_double__arg_value_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_encode_double();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_encode_double unhappy path
//
TEST_FUNCTION(io_encode_double__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const double k_value_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_double(k_ctx_valid, k_name_valid, k_value_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_encode_double

#ifdef io_encode_bool //
// Test io_encode_bool happy path
//
TEST_FUNCTION(io_encode_bool__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const bool k_value_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_encode_bool(k_ctx_valid, k_name_valid, k_value_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_encode_bool passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_encode_bool__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_encode_bool();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_encode_bool passing as name argument an invalid const char* value
//
TEST_FUNCTION(io_encode_bool__arg_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_encode_bool();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_encode_bool passing as value argument an invalid bool value
//
TEST_FUNCTION(io_encode_bool__arg_value_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_encode_bool();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_encode_bool unhappy path
//
TEST_FUNCTION(io_encode_bool__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const bool k_value_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_bool(k_ctx_valid, k_name_valid, k_value_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_encode_bool

#ifdef io_encode_string //
// Test io_encode_string happy path
//
TEST_FUNCTION(io_encode_string__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const const char* k_value_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_encode_string(k_ctx_valid, k_name_valid, k_value_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_encode_string passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_encode_string__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_encode_string();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_encode_string passing as name argument an invalid const char* value
//
TEST_FUNCTION(io_encode_string__arg_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_encode_string();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_encode_string passing as value argument an invalid const char* value
//
TEST_FUNCTION(io_encode_string__arg_value_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_encode_string();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_encode_string unhappy path
//
TEST_FUNCTION(io_encode_string__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const const char* k_value_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_string(k_ctx_valid, k_name_valid, k_value_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_encode_string

#ifdef io_encode_bin //
// Test io_encode_bin happy path
//
TEST_FUNCTION(io_encode_bin__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const const void* k_value_valid;
    static const size_t k_size_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_encode_bin(k_ctx_valid, k_name_valid, k_value_valid, k_size_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_encode_bin passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_encode_bin__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_encode_bin();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_encode_bin passing as name argument an invalid const char* value
//
TEST_FUNCTION(io_encode_bin__arg_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_encode_bin();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_encode_bin passing as value argument an invalid const void* value
//
TEST_FUNCTION(io_encode_bin__arg_value_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_encode_bin();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_encode_bin passing as size argument an invalid size_t value
//
TEST_FUNCTION(io_encode_bin__arg_size_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_encode_bin();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_encode_bin unhappy path
//
TEST_FUNCTION(io_encode_bin__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const const void* k_value_valid;
    static const size_t k_size_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_bin(k_ctx_valid, k_name_valid, k_value_valid, k_size_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_encode_bin

#ifdef io_encode_type_begin //
// Test io_encode_type_begin happy path
//
TEST_FUNCTION(io_encode_type_begin__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const size_t k_members_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_encode_type_begin(k_ctx_valid, k_members_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_encode_type_begin passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_encode_type_begin__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_encode_type_begin();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_encode_type_begin passing as members argument an invalid size_t value
//
TEST_FUNCTION(io_encode_type_begin__arg_members_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_encode_type_begin();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_encode_type_begin unhappy path
//
TEST_FUNCTION(io_encode_type_begin__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const size_t k_members_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_type_begin(k_ctx_valid, k_members_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_encode_type_begin

#ifdef io_encode_type_end //
// Test io_encode_type_end happy path
//
TEST_FUNCTION(io_encode_type_end__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_encode_type_end(k_ctx_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_encode_type_end passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_encode_type_end__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_encode_type_end();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_encode_type_end unhappy path
//
TEST_FUNCTION(io_encode_type_end__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_type_end(k_ctx_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_encode_type_end

#ifdef io_encode_object //
// Test io_encode_object happy path
//
TEST_FUNCTION(io_encode_object__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const bool k_is_null_valid;
    static const io_codec_ctx_t* k_object_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_encode_object(k_ctx_valid, k_name_valid, k_is_null_valid, k_object_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_encode_object passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_encode_object__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_encode_object();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_encode_object passing as name argument an invalid const char* value
//
TEST_FUNCTION(io_encode_object__arg_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_encode_object();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_encode_object passing as is_null argument an invalid bool value
//
TEST_FUNCTION(io_encode_object__arg_is_null_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_encode_object();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_encode_object passing as object argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_encode_object__arg_object_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_encode_object();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_encode_object unhappy path
//
TEST_FUNCTION(io_encode_object__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const bool k_is_null_valid;
    static const io_codec_ctx_t* k_object_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_object(k_ctx_valid, k_name_valid, k_is_null_valid, k_object_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_encode_object

#ifdef io_encode_array //
// Test io_encode_array happy path
//
TEST_FUNCTION(io_encode_array__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const size_t k_size_valid;
    static const io_codec_ctx_t* k_array_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_encode_array(k_ctx_valid, k_name_valid, k_size_valid, k_array_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_encode_array passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_encode_array__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_encode_array();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_encode_array passing as name argument an invalid const char* value
//
TEST_FUNCTION(io_encode_array__arg_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_encode_array();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_encode_array passing as size argument an invalid size_t value
//
TEST_FUNCTION(io_encode_array__arg_size_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_encode_array();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_encode_array passing as array argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_encode_array__arg_array_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_encode_array();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_encode_array unhappy path
//
TEST_FUNCTION(io_encode_array__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const size_t k_size_valid;
    static const io_codec_ctx_t* k_array_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_array(k_ctx_valid, k_name_valid, k_size_valid, k_array_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_encode_array

#ifdef io_encode_STRING_HANDLE //
// Test io_encode_STRING_HANDLE happy path
//
TEST_FUNCTION(io_encode_STRING_HANDLE__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const STRING_HANDLE k_value_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_encode_STRING_HANDLE(k_ctx_valid, k_name_valid, k_value_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_encode_STRING_HANDLE passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_encode_STRING_HANDLE__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_encode_STRING_HANDLE();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_encode_STRING_HANDLE passing as name argument an invalid const char* value
//
TEST_FUNCTION(io_encode_STRING_HANDLE__arg_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_encode_STRING_HANDLE();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_encode_STRING_HANDLE passing as value argument an invalid STRING_HANDLE value
//
TEST_FUNCTION(io_encode_STRING_HANDLE__arg_value_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_encode_STRING_HANDLE();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_encode_STRING_HANDLE unhappy path
//
TEST_FUNCTION(io_encode_STRING_HANDLE__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const STRING_HANDLE k_value_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_STRING_HANDLE(k_ctx_valid, k_name_valid, k_value_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_encode_STRING_HANDLE

#ifdef io_decode_int64 //
// Test io_decode_int64 happy path
//
TEST_FUNCTION(io_decode_int64__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const int64_t* k_value_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_decode_int64(k_ctx_valid, k_name_valid, k_value_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_decode_int64 passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_decode_int64__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_int64();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_int64 passing as name argument an invalid const char* value
//
TEST_FUNCTION(io_decode_int64__arg_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_int64();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_int64 passing as value argument an invalid int64_t* value
//
TEST_FUNCTION(io_decode_int64__arg_value_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_int64();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_int64 unhappy path
//
TEST_FUNCTION(io_decode_int64__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const int64_t* k_value_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_int64(k_ctx_valid, k_name_valid, k_value_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_decode_int64

#ifdef io_decode_uint64 //
// Test io_decode_uint64 happy path
//
TEST_FUNCTION(io_decode_uint64__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const uint64_t* k_value_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_decode_uint64(k_ctx_valid, k_name_valid, k_value_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_decode_uint64 passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_decode_uint64__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_uint64();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_uint64 passing as name argument an invalid const char* value
//
TEST_FUNCTION(io_decode_uint64__arg_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_uint64();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_uint64 passing as value argument an invalid uint64_t* value
//
TEST_FUNCTION(io_decode_uint64__arg_value_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_uint64();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_uint64 unhappy path
//
TEST_FUNCTION(io_decode_uint64__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const uint64_t* k_value_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_uint64(k_ctx_valid, k_name_valid, k_value_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_decode_uint64

#ifdef io_decode_uint32 //
// Test io_decode_uint32 happy path
//
TEST_FUNCTION(io_decode_uint32__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const uint32_t* k_val_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_decode_uint32(k_ctx_valid, k_name_valid, k_val_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_decode_uint32 passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_decode_uint32__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_uint32();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_uint32 passing as name argument an invalid const char* value
//
TEST_FUNCTION(io_decode_uint32__arg_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_uint32();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_uint32 passing as val argument an invalid uint32_t* value
//
TEST_FUNCTION(io_decode_uint32__arg_val_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_uint32();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_uint32 unhappy path
//
TEST_FUNCTION(io_decode_uint32__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const uint32_t* k_val_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_uint32(k_ctx_valid, k_name_valid, k_val_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_decode_uint32

#ifdef io_decode_uint16 //
// Test io_decode_uint16 happy path
//
TEST_FUNCTION(io_decode_uint16__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const uint16_t* k_val_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_decode_uint16(k_ctx_valid, k_name_valid, k_val_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_decode_uint16 passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_decode_uint16__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_uint16();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_uint16 passing as name argument an invalid const char* value
//
TEST_FUNCTION(io_decode_uint16__arg_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_uint16();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_uint16 passing as val argument an invalid uint16_t* value
//
TEST_FUNCTION(io_decode_uint16__arg_val_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_uint16();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_uint16 unhappy path
//
TEST_FUNCTION(io_decode_uint16__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const uint16_t* k_val_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_uint16(k_ctx_valid, k_name_valid, k_val_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_decode_uint16

#ifdef io_decode_uint8 //
// Test io_decode_uint8 happy path
//
TEST_FUNCTION(io_decode_uint8__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const uint8_t* k_val_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_decode_uint8(k_ctx_valid, k_name_valid, k_val_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_decode_uint8 passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_decode_uint8__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_uint8();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_uint8 passing as name argument an invalid const char* value
//
TEST_FUNCTION(io_decode_uint8__arg_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_uint8();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_uint8 passing as val argument an invalid uint8_t* value
//
TEST_FUNCTION(io_decode_uint8__arg_val_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_uint8();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_uint8 unhappy path
//
TEST_FUNCTION(io_decode_uint8__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const uint8_t* k_val_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_uint8(k_ctx_valid, k_name_valid, k_val_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_decode_uint8

#ifdef io_decode_int32 //
// Test io_decode_int32 happy path
//
TEST_FUNCTION(io_decode_int32__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const int32_t* k_val_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_decode_int32(k_ctx_valid, k_name_valid, k_val_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_decode_int32 passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_decode_int32__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_int32();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_int32 passing as name argument an invalid const char* value
//
TEST_FUNCTION(io_decode_int32__arg_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_int32();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_int32 passing as val argument an invalid int32_t* value
//
TEST_FUNCTION(io_decode_int32__arg_val_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_int32();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_int32 unhappy path
//
TEST_FUNCTION(io_decode_int32__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const int32_t* k_val_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_int32(k_ctx_valid, k_name_valid, k_val_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_decode_int32

#ifdef io_decode_int16 //
// Test io_decode_int16 happy path
//
TEST_FUNCTION(io_decode_int16__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const int16_t* k_val_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_decode_int16(k_ctx_valid, k_name_valid, k_val_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_decode_int16 passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_decode_int16__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_int16();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_int16 passing as name argument an invalid const char* value
//
TEST_FUNCTION(io_decode_int16__arg_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_int16();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_int16 passing as val argument an invalid int16_t* value
//
TEST_FUNCTION(io_decode_int16__arg_val_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_int16();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_int16 unhappy path
//
TEST_FUNCTION(io_decode_int16__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const int16_t* k_val_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_int16(k_ctx_valid, k_name_valid, k_val_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_decode_int16

#ifdef io_decode_int8 //
// Test io_decode_int8 happy path
//
TEST_FUNCTION(io_decode_int8__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const int8_t* k_val_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_decode_int8(k_ctx_valid, k_name_valid, k_val_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_decode_int8 passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_decode_int8__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_int8();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_int8 passing as name argument an invalid const char* value
//
TEST_FUNCTION(io_decode_int8__arg_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_int8();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_int8 passing as val argument an invalid int8_t* value
//
TEST_FUNCTION(io_decode_int8__arg_val_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_int8();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_int8 unhappy path
//
TEST_FUNCTION(io_decode_int8__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const int8_t* k_val_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_int8(k_ctx_valid, k_name_valid, k_val_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_decode_int8

#ifdef io_decode_double //
// Test io_decode_double happy path
//
TEST_FUNCTION(io_decode_double__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const double* k_value_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_decode_double(k_ctx_valid, k_name_valid, k_value_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_decode_double passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_decode_double__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_double();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_double passing as name argument an invalid const char* value
//
TEST_FUNCTION(io_decode_double__arg_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_double();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_double passing as value argument an invalid double* value
//
TEST_FUNCTION(io_decode_double__arg_value_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_double();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_double unhappy path
//
TEST_FUNCTION(io_decode_double__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const double* k_value_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_double(k_ctx_valid, k_name_valid, k_value_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_decode_double

#ifdef io_decode_bool //
// Test io_decode_bool happy path
//
TEST_FUNCTION(io_decode_bool__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const bool* k_value_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_decode_bool(k_ctx_valid, k_name_valid, k_value_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_decode_bool passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_decode_bool__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_bool();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_bool passing as name argument an invalid const char* value
//
TEST_FUNCTION(io_decode_bool__arg_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_bool();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_bool passing as value argument an invalid bool* value
//
TEST_FUNCTION(io_decode_bool__arg_value_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_bool();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_bool unhappy path
//
TEST_FUNCTION(io_decode_bool__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const bool* k_value_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_bool(k_ctx_valid, k_name_valid, k_value_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_decode_bool

#ifdef io_decode_string //
// Test io_decode_string happy path
//
TEST_FUNCTION(io_decode_string__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const io_decode_allocator_t k_alloc_valid;
    static const char** k_value_valid;
    static const size_t* k_size_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_decode_string(k_ctx_valid, k_name_valid, k_alloc_valid, k_value_valid, k_size_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_decode_string passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_decode_string__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_string();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_string passing as name argument an invalid const char* value
//
TEST_FUNCTION(io_decode_string__arg_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_string();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_string passing as alloc argument an invalid io_decode_allocator_t value
//
TEST_FUNCTION(io_decode_string__arg_alloc_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_string();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_string passing as value argument an invalid char** value
//
TEST_FUNCTION(io_decode_string__arg_value_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_string();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_string passing as size argument an invalid size_t* value
//
TEST_FUNCTION(io_decode_string__arg_size_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_string();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_string unhappy path
//
TEST_FUNCTION(io_decode_string__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const io_decode_allocator_t k_alloc_valid;
    static const char** k_value_valid;
    static const size_t* k_size_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_string(k_ctx_valid, k_name_valid, k_alloc_valid, k_value_valid, k_size_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_decode_string

#ifdef io_decode_string_default //
// Test io_decode_string_default happy path
//
TEST_FUNCTION(io_decode_string_default__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const char** k_string_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_decode_string_default(k_ctx_valid, k_name_valid, k_string_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_decode_string_default passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_decode_string_default__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_string_default();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_string_default passing as name argument an invalid const char* value
//
TEST_FUNCTION(io_decode_string_default__arg_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_string_default();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_string_default passing as string argument an invalid char** value
//
TEST_FUNCTION(io_decode_string_default__arg_string_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_string_default();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_string_default unhappy path
//
TEST_FUNCTION(io_decode_string_default__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const char** k_string_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_string_default(k_ctx_valid, k_name_valid, k_string_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_decode_string_default;

#ifdef io_decode_string_fixed //
// Test io_decode_string_fixed happy path
//
TEST_FUNCTION(io_decode_string_fixed__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const char* k_string_valid;
    static const size_t k_len_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_decode_string_fixed(k_ctx_valid, k_name_valid, k_string_valid, k_len_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_decode_string_fixed passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_decode_string_fixed__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_string_fixed();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_string_fixed passing as name argument an invalid const char* value
//
TEST_FUNCTION(io_decode_string_fixed__arg_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_string_fixed();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_string_fixed passing as string argument an invalid char* value
//
TEST_FUNCTION(io_decode_string_fixed__arg_string_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_string_fixed();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_string_fixed passing as len argument an invalid size_t value
//
TEST_FUNCTION(io_decode_string_fixed__arg_len_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_string_fixed();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_string_fixed unhappy path
//
TEST_FUNCTION(io_decode_string_fixed__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const char* k_string_valid;
    static const size_t k_len_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_string_fixed(k_ctx_valid, k_name_valid, k_string_valid, k_len_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_decode_string_fixed;

#ifdef io_decode_STRING_HANDLE //
// Test io_decode_STRING_HANDLE happy path
//
TEST_FUNCTION(io_decode_STRING_HANDLE__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const STRING_HANDLE* k_string_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_decode_STRING_HANDLE(k_ctx_valid, k_name_valid, k_string_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_decode_STRING_HANDLE passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_decode_STRING_HANDLE__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_STRING_HANDLE();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_STRING_HANDLE passing as name argument an invalid const char* value
//
TEST_FUNCTION(io_decode_STRING_HANDLE__arg_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_STRING_HANDLE();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_STRING_HANDLE passing as string argument an invalid STRING_HANDLE* value
//
TEST_FUNCTION(io_decode_STRING_HANDLE__arg_string_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_STRING_HANDLE();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_STRING_HANDLE unhappy path
//
TEST_FUNCTION(io_decode_STRING_HANDLE__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const STRING_HANDLE* k_string_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_STRING_HANDLE(k_ctx_valid, k_name_valid, k_string_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_decode_STRING_HANDLE;

#ifdef io_decode_bin //
// Test io_decode_bin happy path
//
TEST_FUNCTION(io_decode_bin__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const io_decode_allocator_t k_alloc_valid;
    static const void** k_value_valid;
    static const size_t* k_size_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_decode_bin(k_ctx_valid, k_name_valid, k_alloc_valid, k_value_valid, k_size_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_decode_bin passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_decode_bin__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_bin();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_bin passing as name argument an invalid const char* value
//
TEST_FUNCTION(io_decode_bin__arg_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_bin();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_bin passing as alloc argument an invalid io_decode_allocator_t value
//
TEST_FUNCTION(io_decode_bin__arg_alloc_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_bin();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_bin passing as value argument an invalid void** value
//
TEST_FUNCTION(io_decode_bin__arg_value_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_bin();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_bin passing as size argument an invalid size_t* value
//
TEST_FUNCTION(io_decode_bin__arg_size_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_bin();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_bin unhappy path
//
TEST_FUNCTION(io_decode_bin__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const io_decode_allocator_t k_alloc_valid;
    static const void** k_value_valid;
    static const size_t* k_size_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_bin(k_ctx_valid, k_name_valid, k_alloc_valid, k_value_valid, k_size_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_decode_bin

#ifdef io_decode_bin_default //
// Test io_decode_bin_default happy path
//
TEST_FUNCTION(io_decode_bin_default__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const void** k_value_valid;
    static const size_t* k_size_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_decode_bin_default(k_ctx_valid, k_name_valid, k_value_valid, k_size_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_decode_bin_default passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_decode_bin_default__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_bin_default();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_bin_default passing as name argument an invalid const char* value
//
TEST_FUNCTION(io_decode_bin_default__arg_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_bin_default();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_bin_default passing as value argument an invalid void** value
//
TEST_FUNCTION(io_decode_bin_default__arg_value_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_bin_default();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_bin_default passing as size argument an invalid size_t* value
//
TEST_FUNCTION(io_decode_bin_default__arg_size_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_bin_default();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_bin_default unhappy path
//
TEST_FUNCTION(io_decode_bin_default__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const void** k_value_valid;
    static const size_t* k_size_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_bin_default(k_ctx_valid, k_name_valid, k_value_valid, k_size_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_decode_bin_default;

#ifdef io_decode_bin_fixed //
// Test io_decode_bin_fixed happy path
//
TEST_FUNCTION(io_decode_bin_fixed__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const void* k_value_valid;
    static const size_t* k_size_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_decode_bin_fixed(k_ctx_valid, k_name_valid, k_value_valid, k_size_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_decode_bin_fixed passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_decode_bin_fixed__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_bin_fixed();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_bin_fixed passing as name argument an invalid const char* value
//
TEST_FUNCTION(io_decode_bin_fixed__arg_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_bin_fixed();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_bin_fixed passing as value argument an invalid void* value
//
TEST_FUNCTION(io_decode_bin_fixed__arg_value_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_bin_fixed();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_bin_fixed passing as size argument an invalid size_t* value
//
TEST_FUNCTION(io_decode_bin_fixed__arg_size_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_bin_fixed();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_bin_fixed unhappy path
//
TEST_FUNCTION(io_decode_bin_fixed__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const void* k_value_valid;
    static const size_t* k_size_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_bin_fixed(k_ctx_valid, k_name_valid, k_value_valid, k_size_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_decode_bin_fixed;

#ifdef io_decode_type_begin //
// Test io_decode_type_begin happy path
//
TEST_FUNCTION(io_decode_type_begin__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_decode_type_begin(k_ctx_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_decode_type_begin passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_decode_type_begin__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_type_begin();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_type_begin unhappy path
//
TEST_FUNCTION(io_decode_type_begin__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_type_begin(k_ctx_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_decode_type_begin

#ifdef io_decode_type_end //
// Test io_decode_type_end happy path
//
TEST_FUNCTION(io_decode_type_end__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_decode_type_end(k_ctx_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_decode_type_end passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_decode_type_end__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_type_end();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_type_end unhappy path
//
TEST_FUNCTION(io_decode_type_end__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_type_end(k_ctx_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_decode_type_end

#ifdef io_decode_object //
// Test io_decode_object happy path
//
TEST_FUNCTION(io_decode_object__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const bool* k_is_null_valid;
    static const io_codec_ctx_t* k_object_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_decode_object(k_ctx_valid, k_name_valid, k_is_null_valid, k_object_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_decode_object passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_decode_object__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_object();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_object passing as name argument an invalid const char* value
//
TEST_FUNCTION(io_decode_object__arg_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_object();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_object passing as is_null argument an invalid bool* value
//
TEST_FUNCTION(io_decode_object__arg_is_null_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_object();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_object passing as object argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_decode_object__arg_object_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_object();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_object unhappy path
//
TEST_FUNCTION(io_decode_object__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const bool* k_is_null_valid;
    static const io_codec_ctx_t* k_object_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_object(k_ctx_valid, k_name_valid, k_is_null_valid, k_object_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_decode_object

#ifdef io_decode_array //
// Test io_decode_array happy path
//
TEST_FUNCTION(io_decode_array__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const size_t* k_size_valid;
    static const io_codec_ctx_t* k_array_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_decode_array(k_ctx_valid, k_name_valid, k_size_valid, k_array_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_decode_array passing as ctx argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_decode_array__arg_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_array();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_array passing as name argument an invalid const char* value
//
TEST_FUNCTION(io_decode_array__arg_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_array();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_array passing as size argument an invalid size_t* value
//
TEST_FUNCTION(io_decode_array__arg_size_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_array();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_array passing as array argument an invalid io_codec_ctx_t* value
//
TEST_FUNCTION(io_decode_array__arg_array_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_decode_array();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_decode_array unhappy path
//
TEST_FUNCTION(io_decode_array__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const char* k_name_valid;
    static const size_t* k_size_valid;
    static const io_codec_ctx_t* k_array_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_array(k_ctx_valid, k_name_valid, k_size_valid, k_array_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_decode_array

#ifdef io_codec_by_id //
// Test io_codec_by_id happy path
//
TEST_FUNCTION(io_codec_by_id__success)
{
    static const io_codec_id_t k_id_valid;
    io_codec_t* result;

    // arrange
    // ...

    // act
    result = io_codec_by_id(k_id_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(io_codec_t*, er_ok, result);
    // ...
}

//
// Test io_codec_by_id passing as id argument an invalid io_codec_id_t value
//
TEST_FUNCTION(io_codec_by_id__arg_id_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_codec_by_id();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_codec_by_id unhappy path
//
TEST_FUNCTION(io_codec_by_id__neg)
{
    static const io_codec_id_t k_id_valid;
    io_codec_t* result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_codec_by_id(k_id_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(io_codec_t*, result, er_ok);
}

#endif // io_codec_by_id;

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

