// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST prx_config
#include "util_ut.h"

//
// 1. Required mocks
//
#include "io_codec.h"

//
// 2. Include unit under test
//
#undef ENABLE_MOCKS
#include UNIT_H
#define ENABLE_MOCKS
#include UNIT_C


//
// Simple string compare
//
static bool string_is_equal_nocase__hook(
    const char* val,
    size_t len,
    const char* to
)
{
    (void)len;
    return 0 == strcmp(val, to);
}

//
// 3. Setup test suite
//
BEGIN_DECLARE_TEST_SUITE()
REGISTER_UMOCK_ALIAS_TYPE(STRING_HANDLE, void*)
REGISTER_GLOBAL_MOCK_HOOK(string_is_equal_nocase, string_is_equal_nocase__hook);
END_DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
DECLARE_TEST_SETUP()
//
// Test prx_config_create happy path
//
TEST_FUNCTION(prx_config_create__success)
{
    prx_config_t* config_valid;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange
    STRICT_EXPECTED_CALL(h_realloc(sizeof(prx_config_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);

    // act
    result = prx_config_create(&config_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

//
// Test prx_config_create passing as created argument an invalid prx_config_t** value
//
TEST_FUNCTION(prx_config_create__arg_created_null)
{
    int32_t result;

    // arrange

    // act
    result = prx_config_create(NULL);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

//
// Test prx_config_create unhappy path
//
TEST_FUNCTION(prx_config_create__neg)
{
    prx_config_t* config_valid;
    int32_t result;

    // arrange
    STRICT_EXPECTED_CALL(h_realloc(sizeof(prx_config_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn(NULL);

    // act
    result = prx_config_create(&config_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_out_of_memory, result);
}

//
// Test prx_config_free happy path
//
TEST_FUNCTION(prx_config_free__success)
{
    prx_config_t config_valid;

    for (size_t i = 0; i < _countof(config_valid.values); i++)
        config_valid.values[i] = (STRING_HANDLE)(0x8000 | i);

    // arrange
    for (size_t i = 0; i < _countof(config_valid.values); i++)
    {
        STRICT_EXPECTED_CALL(STRING_delete((STRING_HANDLE)(0x8000 | i)));
    }
    STRICT_EXPECTED_CALL(h_free(&config_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act
    prx_config_free(&config_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
}

//
// Test prx_config_free passing as config argument an invalid prx_config_t* value
//
TEST_FUNCTION(prx_config_free__arg_config_null)
{
    // arrange

    // act
    prx_config_free(NULL);

    // assert
    // ...
    ASSERT_EXPECTED_CALLS();
}

#if 0

//
// Test prx_config_get_partition_count happy path
//
TEST_FUNCTION(prx_config_get_partition_count__success)
{
    static const char* k_string_c_valid = "5";
    const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x23423;
    prx_config_t config_valid;
    int32_t result;

    memset(&config_valid, 0, sizeof(config_valid));
    config_valid.entries[prx_config_entry_partition_count] = k_string_valid;

    // arrange
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn(k_string_c_valid);

    // act
    result = prx_config_get_partition_count(&config_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, 5, result);
}

//
// Test prx_config_get_partition_count passing as config argument an invalid prx_config_t* value
//
TEST_FUNCTION(prx_config_get_partition_count__arg_config_invalid)
{
    prx_config_t config_valid;
    int32_t result;

    memset(&config_valid, 0, sizeof(config_valid));

    // arrange

    // act
    result = prx_config_get_partition_count(&config_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, 0, result);
}

//
// Test prx_config_get_partition_count passing as config argument an invalid prx_config_t* value
//
TEST_FUNCTION(prx_config_get_partition_count__arg_config_null)
{
    int32_t result;

    // arrange

    // act
    result = prx_config_get_partition_count(NULL);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, 0, result);
}

//
// Test prx_config_set_partition_count happy path
//
TEST_FUNCTION(prx_config_set_partition_count__success)
{
    static const char* k_string_c_valid = "32";
    const STRING_HANDLE k_string1_valid = (STRING_HANDLE)0x23423;
    const STRING_HANDLE k_string2_valid = (STRING_HANDLE)0x23423;
    prx_config_t config_valid;
    int32_t result;

    memset(&config_valid, 0, sizeof(config_valid));
    config_valid.entries[prx_config_entry_partition_count] = k_string1_valid;

    // arrange
    STRICT_EXPECTED_CALL(string_from_int(32, 10, IGNORED_PTR_ARG, 32))
        .CopyOutArgumentBuffer_string(k_string_c_valid, strlen(k_string_c_valid)+1)
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(STRING_delete(k_string1_valid));
    STRICT_EXPECTED_CALL(STRING_construct(k_string_c_valid))
        .SetReturn(k_string2_valid);

    // act
    result = prx_config_set_partition_count(&config_valid, 32);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

//
// Test prx_config_set_partition_count happy path
//
TEST_FUNCTION(prx_config_set_partition_count__success_null)
{
    const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x1;
    prx_config_t config_valid;
    int32_t result;

    memset(&config_valid, 0, sizeof(config_valid));
    config_valid.entries[prx_config_entry_partition_count] = k_string_valid;

    // arrange
    STRICT_EXPECTED_CALL(STRING_delete(k_string_valid));

    // act
    result = prx_config_set_partition_count(&config_valid, 0);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(void_ptr, NULL, config_valid.entries[prx_config_entry_partition_count]);
}

//
// Test prx_config_set_partition_count passing as config argument an invalid prx_config_t* value
//
TEST_FUNCTION(prx_config_set_partition_count__arg_config_null)
{
    static const char* k_string_c_valid = "16";
    int32_t result;

    // arrange
    STRICT_EXPECTED_CALL(string_from_int(16, 10, IGNORED_PTR_ARG, 32))
        .CopyOutArgumentBuffer_string(k_string_c_valid, strlen(k_string_c_valid) + 1)
        .SetReturn(er_ok);

    // act
    result = prx_config_set_partition_count(NULL, 16);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

//
// Test prx_config_set_partition_count unhappy path
//
TEST_FUNCTION(prx_config_set_partition_count__neg)
{
    static const char* k_string_c_valid = "6";
    prx_config_t config_valid;
    int32_t result;

    memset(&config_valid, 0, sizeof(config_valid));

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(string_from_int(6, 10, IGNORED_PTR_ARG, 32))
        .CopyOutArgumentBuffer_string(k_string_c_valid, strlen(k_string_c_valid) + 1)
        .SetReturn(er_ok)
        .SetFailReturn(er_invalid_format);
    STRICT_EXPECTED_CALL(STRING_construct(k_string_c_valid))
        .SetReturn((STRING_HANDLE)2)
        .SetFailReturn(NULL);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_config_set_partition_count(&config_valid, 6);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_invalid_format, er_out_of_memory);
}

//
// Test prx_config_get_entity happy path
//
TEST_FUNCTION(prx_config_get_entity__success)
{
    static const char* k_string_c_valid = "queue1";
    const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x2af3;
    prx_config_t config_valid;
    const char* result;

    memset(&config_valid, 0, sizeof(config_valid));
    config_valid.entries[prx_config_entry_entity] = k_string_valid;

    // arrange
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn(k_string_c_valid);

    // act
    result = prx_config_get_entity(&config_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, k_string_c_valid, result);
}

//
// Test prx_config_get_entity passing as config argument an invalid prx_config_t* value
//
TEST_FUNCTION(prx_config_get_entity__arg_config_invalid)
{
    prx_config_t config_valid;
    const char* result;

    memset(&config_valid, 0, sizeof(config_valid));

    // arrange

    // act
    result = prx_config_get_entity(&config_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

//
// Test prx_config_get_entity passing as config argument an invalid prx_config_t* value
//
TEST_FUNCTION(prx_config_get_entity__arg_config_null)
{
    const char* result;

    // arrange

    // act
    result = prx_config_get_entity(NULL);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

//
// Test prx_config_set_entity happy path
//
TEST_FUNCTION(prx_config_set_entity__success)
{
    static const char* k_string_c_valid = "queue3";
    const STRING_HANDLE k_string1_valid = (STRING_HANDLE)0x23423;
    const STRING_HANDLE k_string2_valid = (STRING_HANDLE)0x23423;
    prx_config_t config_valid;
    int32_t result;

    memset(&config_valid, 0, sizeof(config_valid));
    config_valid.entries[prx_config_entry_entity] = k_string1_valid;

    // arrange
    STRICT_EXPECTED_CALL(STRING_delete(k_string1_valid));
    STRICT_EXPECTED_CALL(STRING_construct(k_string_c_valid))
        .SetReturn(k_string2_valid);

    // act
    result = prx_config_set_entity(&config_valid, k_string_c_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

//
// Test prx_config_set_entity happy path
//
TEST_FUNCTION(prx_config_set_entity__success_null)
{
    const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x1;
    prx_config_t config_valid;
    int32_t result;

    memset(&config_valid, 0, sizeof(config_valid));
    config_valid.entries[prx_config_entry_entity] = k_string_valid;

    // arrange
    STRICT_EXPECTED_CALL(STRING_delete(k_string_valid));

    // act
    result = prx_config_set_entity(&config_valid, NULL);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(void_ptr, NULL, config_valid.entries[prx_config_entry_entity]);
}

//
// Test prx_config_set_entity passing as config argument an invalid prx_config_t* value
//
TEST_FUNCTION(prx_config_set_entity__arg_config_null)
{
    static const char* k_string_c_valid = "queue3";
    int32_t result;

    // arrange

    // act
    result = prx_config_set_entity(NULL, k_string_c_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

//
// Test prx_config_set_entity unhappy path
//
TEST_FUNCTION(prx_config_set_entity__neg)
{
    static const char* k_string_c_valid = "queue54";
    prx_config_t config_valid;
    int32_t result;

    memset(&config_valid, 0, sizeof(config_valid));

    // arrange
    STRICT_EXPECTED_CALL(STRING_construct(k_string_c_valid))
        .SetReturn(NULL);

    // act
    result = prx_config_set_entity(&config_valid, k_string_c_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_out_of_memory, result);
}

//
// Test io_encode_config happy path
//
TEST_FUNCTION(io_encode_config__success)
{
    static const STRING_HANDLE k_string1_valid = (STRING_HANDLE)0x654;
    static const STRING_HANDLE k_string2_valid = (STRING_HANDLE)0x345;
    static io_codec_ctx_t* k_ctx_valid = (io_codec_ctx_t*)0x1234124;
    prx_config_t config_valid;
    prx_config_t* config_ptr_valid = &config_valid;
    int32_t result;

    memset(&config_valid, 0, sizeof(config_valid));
    config_valid.entries[0] = k_string2_valid;

    // arrange
    STRICT_EXPECTED_CALL(io_encode_type_begin(k_ctx_valid, 1))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(STRING_new())
        .SetReturn(k_string1_valid);
    STRICT_EXPECTED_CALL(STRING_length(k_string1_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat(k_string1_valid, "HostName="))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(k_string1_valid, k_string2_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(io_encode_STRING_HANDLE(k_ctx_valid, "connection-string", k_string1_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(STRING_delete(k_string1_valid));
    STRICT_EXPECTED_CALL(io_encode_type_end(k_ctx_valid))
        .SetReturn(er_ok);

    // act
    result = io_encode_config(k_ctx_valid, &config_ptr_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

//
// Test io_encode_config unhappy path
//
TEST_FUNCTION(io_encode_config__neg)
{
    static const STRING_HANDLE k_string1_valid = (STRING_HANDLE)0x654;
    static const STRING_HANDLE k_string2_valid = (STRING_HANDLE)0x345;
    static io_codec_ctx_t* k_ctx_valid = (io_codec_ctx_t*)0x1234124;
    prx_config_t config_valid;
    prx_config_t* config_ptr_valid = &config_valid;
    int32_t result;

    memset(&config_valid, 0, sizeof(config_valid));
    config_valid.entries[0] = k_string2_valid;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(io_encode_type_begin(k_ctx_valid, 1))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL(STRING_new())
        .SetReturn(k_string1_valid)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(STRING_length(k_string1_valid))
        .SetReturn(0)
        .SetFailReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat(k_string1_valid, "HostName="))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(k_string1_valid, k_string2_valid))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(io_encode_STRING_HANDLE(k_ctx_valid, "connection-string", k_string1_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL(STRING_delete(k_string1_valid));
    STRICT_EXPECTED_CALL(io_encode_type_end(k_ctx_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_invalid_format);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_config(k_ctx_valid, &config_ptr_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result,
        er_writing, er_out_of_memory, er_ok, er_out_of_memory, er_out_of_memory,
        er_writing, er_ok,            er_invalid_format);
}

//
// Test io_decode_config happy path
//
TEST_FUNCTION(io_decode_config__success)
{
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x654;
    static const STRING_HANDLE k_hostname_string_valid = (STRING_HANDLE)0x345;
    static const STRING_HANDLE k_device_id_string_valid = (STRING_HANDLE)0x222;
    static const char* k_component = "HostName=cannotfail.ever.com;DeviceId=andever";
    static io_codec_ctx_t* k_ctx_valid = (io_codec_ctx_t*)0x1234124;
    prx_config_t* config_ptr_valid;
    int32_t result;

    REGISTER_GLOBAL_MOCK_HOOK(string_key_value_parser, string_key_value_parser__hook_config_with_hostname_and_device_id);
    REGISTER_GLOBAL_MOCK_RETURNS(string_key_value_parser, er_ok, er_invalid_format);

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange
    STRICT_EXPECTED_CALL(io_decode_type_begin(k_ctx_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_decode_STRING_HANDLE(k_ctx_valid, "connection-string", IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_string(&k_string_valid, sizeof(k_string_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn(k_component);
    STRICT_EXPECTED_CALL(h_realloc(sizeof(prx_config_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(string_key_value_parser(k_component, IGNORED_PTR_ARG, ';', (void*)UT_MEM))
        .IgnoreArgument(2);
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_safe_construct_n("Hub.Suffix", 2))
        .SetReturn(k_hostname_string_valid);
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_safe_construct_n("DeviceId", 2))
        .SetReturn(k_device_id_string_valid);
    STRICT_EXPECTED_CALL(STRING_c_str(k_hostname_string_valid))
        .SetReturn("Hub.Suffix");
    STRICT_EXPECTED_CALL(STRING_c_str(k_device_id_string_valid))
        .SetReturn("DeviceId");
    STRICT_EXPECTED_CALL(STRING_delete(k_string_valid));
    STRICT_EXPECTED_CALL(io_decode_type_end(k_ctx_valid))
        .SetReturn(er_ok);

    // act
    result = io_decode_config(k_ctx_valid, &config_ptr_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

//
// Test io_decode_config unhappy path
//
TEST_FUNCTION(io_decode_config__neg)
{
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x654;
    static const STRING_HANDLE k_hostname_string_valid = (STRING_HANDLE)0x345;
    static const STRING_HANDLE k_device_id_string_valid = (STRING_HANDLE)0x222;
    static const char* k_component = "HostName=cannotfail.ever.com;DeviceId=andever";
    static io_codec_ctx_t* k_ctx_valid = (io_codec_ctx_t*)0x1234124;
    prx_config_t* config_ptr_valid;
    int32_t result;

    REGISTER_GLOBAL_MOCK_HOOK(string_key_value_parser, string_key_value_parser__hook_config_with_hostname_and_device_id);
    REGISTER_GLOBAL_MOCK_RETURNS(string_key_value_parser, er_ok, er_invalid_format);

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(io_decode_type_begin(k_ctx_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_invalid_format);
    STRICT_EXPECTED_CALL(io_decode_STRING_HANDLE(k_ctx_valid, "connection-string", IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_string(&k_string_valid, sizeof(k_string_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_invalid_format);
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn(k_component)
        .SetFailReturn(k_component);
    STRICT_EXPECTED_CALL(h_realloc(sizeof(prx_config_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(string_key_value_parser(k_component, IGNORED_PTR_ARG, ';', (void*)UT_MEM))
        .IgnoreArgument(2)
        .SetFailReturn(er_invalid_format);
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_safe_construct_n("Hub.Suffix", 2))
        .SetReturn(k_hostname_string_valid)
        .SetFailReturn(NULL);
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_safe_construct_n("DeviceId", 2))
        .SetReturn(k_device_id_string_valid)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(STRING_c_str(k_hostname_string_valid))
        .SetReturn("Hub.Suffix")
        .SetFailReturn("Hub.Suffix");
    STRICT_EXPECTED_CALL(STRING_c_str(k_device_id_string_valid))
        .SetReturn("DeviceId")
        .SetFailReturn("DeviceId");
    STRICT_EXPECTED_CALL(STRING_delete(k_string_valid));
    STRICT_EXPECTED_CALL(io_decode_type_end(k_ctx_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_invalid_format);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_config(k_ctx_valid, &config_ptr_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result,
        er_invalid_format, er_invalid_format, er_ok, er_out_of_memory, er_invalid_format);
}

#endif

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

