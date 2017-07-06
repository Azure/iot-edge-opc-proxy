// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST io_cs
#include "util_ut.h"

//
// 1. Required mocks
//
#include "io_codec.h"
#include "io_token.h"

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
// Parser mock hook that returns default cs content
//
static int32_t string_key_value_parser__hook_1(
    const char* connection_string,
    fn_parse_cb_t visitor,
    char delim,
    void* ctx
)
{
    int32_t result;
    (void)connection_string;
    (void)delim;
    result = visitor(ctx, "HostName", 8, "Hub.Suffix", 14);
    if (result != er_ok)
        return result;
    result = visitor(ctx, "DeviceId", 8, "DeviceId", 8);
    if (result != er_ok)
        return result;
    result = visitor(ctx, "SharedAccessKeyName", 19, "SharedAccessKeyName", 19);
    if (result != er_ok)
        return result;
    result = visitor(ctx, "SharedAccessKey", 15, "SharedAccessKey", 15);
    if (result != er_ok)
        return result;
    return result;
}

// 
// Test io_cs_create_from_string happy path 
// 
TEST_FUNCTION(io_cs_create_from_string__success_1)
{
    static const STRING_HANDLE k_hostname_string_valid = (STRING_HANDLE)1;
    static const STRING_HANDLE k_device_id_string_valid = (STRING_HANDLE)2;
    static const char* k_connection_string_valid = "some_connection_string";
    io_cs_t* cs_valid;
    int32_t result;

    REGISTER_GLOBAL_MOCK_HOOK(string_key_value_parser, string_key_value_parser__hook_1);
    REGISTER_GLOBAL_MOCK_RETURNS(string_key_value_parser, er_ok, er_invalid_format);

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_cs_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(string_key_value_parser(k_connection_string_valid, IGNORED_PTR_ARG, ';', (void*)UT_MEM))
        .IgnoreArgument(2);
    
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_safe_construct_n("Hub.Suffix", 14))
        .SetReturn(k_hostname_string_valid);
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_safe_construct_n("DeviceId", 8))
        .SetReturn(k_device_id_string_valid);
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_safe_construct_n("SharedAccessKeyName", 19))
        .SetReturn((STRING_HANDLE)3);
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_safe_construct_n("SharedAccessKey", 15))
        .SetReturn((STRING_HANDLE)4);
    STRICT_EXPECTED_CALL(pal_caps()).SetReturn((uint32_t)~pal_cap_cred);
    STRICT_EXPECTED_CALL(STRING_c_str(k_hostname_string_valid))
        .SetReturn("Hub.Suffix");
    STRICT_EXPECTED_CALL(STRING_c_str(k_device_id_string_valid))
        .SetReturn("DeviceId");

    // act 
    result = io_cs_create_from_string(k_connection_string_valid, &cs_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(void_ptr, (void*)cs_valid, (void*)UT_MEM);
}

//
// Parser hook, returns all possible components
//
static int32_t string_key_value_parser__hook_2(
    const char* connection_string,
    fn_parse_cb_t visitor,
    char delim,
    void* ctx
)
{
    int32_t result;
    (void)connection_string;
    (void)delim;
    result = visitor(ctx, "HostName", 8, "Hub.Suffix", 14);
    if (result != er_ok)
        return result;
    result = visitor(ctx, "Endpoint", 8, "mqtts://Endpoint/", 17);
    if (result != er_ok)
        return result;
    result = visitor(ctx, "DeviceId", 8, "DeviceId", 8);
    if (result != er_ok)
        return result;
    result = visitor(ctx, "EntityPath", 10, "EntityPath", 10);
    if (result != er_ok)
        return result;
    result = visitor(ctx, "SharedAccessKeyName", 19, "SharedAccessKeyName", 19);
    if (result != er_ok)
        return result;
    result = visitor(ctx, "SharedAccessKey", 15, "SharedAccessKey", 15);
    if (result != er_ok)
        return result;
    result = visitor(ctx, "ConsumerGroup", 13, "ConsumerGroup", 13);
    if (result != er_ok)
        return result;
    result = visitor(ctx, "Partitions", 10, "32", 2);
    if (result != er_ok)
        return result;
    return result;
}

// 
// Test io_cs_create_from_string happy path 
// 
TEST_FUNCTION(io_cs_create_from_string__success_2)
{
    static const STRING_HANDLE k_hostname_string_valid = (STRING_HANDLE)9999;
    static const STRING_HANDLE k_device_id_string_valid = (STRING_HANDLE)8888;
    static const char* k_connection_string_valid = "some_connection_string";
    static const char* k_endpoint = "mqtts://Endpoint/";
    static const size_t k_endpoint_len = 17;
    io_cs_t* cs_valid;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    REGISTER_GLOBAL_MOCK_HOOK(string_key_value_parser, string_key_value_parser__hook_2);
    REGISTER_GLOBAL_MOCK_RETURNS(string_key_value_parser, er_ok, er_invalid_format);

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_cs_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(string_key_value_parser(k_connection_string_valid, IGNORED_PTR_ARG, ';', (void*)UT_MEM))
        .IgnoreArgument(2)
        .SetFailReturn(er_invalid_format);
    

    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_safe_construct_n("Hub.Suffix", 14))
        .SetReturn(k_hostname_string_valid);

    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(STRING_safe_construct_n(k_endpoint, k_endpoint_len))
        .SetReturn((STRING_HANDLE)1);

    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_safe_construct_n("DeviceId", 8))
        .SetReturn(k_device_id_string_valid);

    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_safe_construct_n("EntityPath", 10))
        .SetReturn((STRING_HANDLE)2);

    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_safe_construct_n("SharedAccessKeyName", 19))
        .SetReturn((STRING_HANDLE)3);

    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_safe_construct_n("SharedAccessKey", 15))
        .SetReturn((STRING_HANDLE)4);

    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_safe_construct_n("ConsumerGroup", 13))
        .SetReturn((STRING_HANDLE)5);

    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_safe_construct_n("32", 2))
        .SetReturn((STRING_HANDLE)5);

    STRICT_EXPECTED_CALL(pal_caps()).SetReturn((uint32_t)~pal_cap_cred);
    STRICT_EXPECTED_CALL(STRING_c_str(k_hostname_string_valid))
        .SetReturn("Hub.Suffix");
    STRICT_EXPECTED_CALL(STRING_c_str(k_device_id_string_valid))
        .SetReturn("DeviceId");

    // act 
    result = io_cs_create_from_string(k_connection_string_valid, &cs_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(void_ptr, (void*)cs_valid, (void*)UT_MEM);
}

// 
// Test io_cs_create_from_string passing as connection_string argument an invalid const char* value 
// 
TEST_FUNCTION(io_cs_create_from_string__arg_connection_string_null)
{
    io_cs_t* cs_valid;
    int32_t result;

    // arrange 

    // act 
    result = io_cs_create_from_string(NULL, &cs_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

//
// Parser mock hook that returns one item passed as cs arg
//
static int32_t string_key_value_parser__hook_connection_string_with_2_components(
    const char* connection_string,
    fn_parse_cb_t visitor,
    char delim,
    void* ctx
)
{
    int32_t result;
    (void)delim;
    result = visitor(ctx, connection_string, strlen(connection_string), connection_string, strlen(connection_string));
    if (result != er_ok)
        return result;
    result = visitor(ctx, connection_string, strlen(connection_string), connection_string, strlen(connection_string));
    if (result != er_ok)
        return result;
    return result;
}

// 
// Test io_cs_create_from_string when connection string has 2 device id components
// 
TEST_FUNCTION(io_cs_create_from_string__arg_connection_string_with_2_components_1)
{
    static const STRING_HANDLE k_string_handle_valid = (STRING_HANDLE)0x62423;
    static const char* k_component = "DeviceId";
    io_cs_t* cs_valid;
    int32_t result;

    REGISTER_GLOBAL_MOCK_HOOK(string_key_value_parser, string_key_value_parser__hook_connection_string_with_2_components);
    REGISTER_GLOBAL_MOCK_RETURNS(string_key_value_parser, er_ok, er_invalid_format);

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_cs_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(string_key_value_parser(k_component, IGNORED_PTR_ARG, ';', (void*)UT_MEM))
        .IgnoreArgument(2)
        .SetFailReturn(er_invalid_format);
    

    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_safe_construct_n(k_component, 8))
        .SetReturn(k_string_handle_valid);
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(STRING_delete(k_string_handle_valid));
    STRICT_EXPECTED_CALL(h_free((void*)UT_MEM, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    result = io_cs_create_from_string(k_component, &cs_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_invalid_format, result);
}

// 
// Test io_cs_create_from_string when connection string has 2 endpoint components
// 
TEST_FUNCTION(io_cs_create_from_string__arg_connection_string_with_2_components_2)
{
    static const STRING_HANDLE k_string_handle_valid = (STRING_HANDLE)0x3824;
    static const char* k_component = "Endpoint";
    static size_t k_component_len = 8;
    io_cs_t* cs_valid;
    int32_t result;

    REGISTER_GLOBAL_MOCK_HOOK(string_key_value_parser, string_key_value_parser__hook_connection_string_with_2_components);
    REGISTER_GLOBAL_MOCK_RETURNS(string_key_value_parser, er_ok, er_invalid_format);

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_cs_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(string_key_value_parser(k_component, IGNORED_PTR_ARG, ';', (void*)UT_MEM))
        .IgnoreArgument(2)
        .SetFailReturn(er_invalid_format);
    

    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_safe_construct_n(k_component, k_component_len))
        .SetReturn(k_string_handle_valid);
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(STRING_delete(k_string_handle_valid));
    STRICT_EXPECTED_CALL(h_free((void*)UT_MEM, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    result = io_cs_create_from_string(k_component, &cs_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_invalid_format, result);
}

//
// Parser mock hook that returns one item passed as cs arg
//
static int32_t string_key_value_parser__hook_connection_string_with_1_component(
    const char* connection_string,
    fn_parse_cb_t visitor,
    char delim,
    void* ctx
)
{
    (void)delim;
    return visitor(ctx, connection_string, strlen(connection_string), connection_string, strlen(connection_string));
}

// 
// Test io_cs_create_from_string when connection string contains no host name
// 
TEST_FUNCTION(io_cs_create_from_string__arg_connection_string_no_hostname)
{
    static const STRING_HANDLE k_string_handle_valid = (STRING_HANDLE)0x62423;
    static const char* k_component = "DeviceId";
    io_cs_t* cs_valid;
    int32_t result;

    REGISTER_GLOBAL_MOCK_HOOK(string_key_value_parser, string_key_value_parser__hook_connection_string_with_1_component);
    REGISTER_GLOBAL_MOCK_RETURNS(string_key_value_parser, er_ok, er_invalid_format);

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_cs_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(string_key_value_parser(k_component, IGNORED_PTR_ARG, ';', (void*)UT_MEM))
        .IgnoreArgument(2)
        .SetFailReturn(er_invalid_format);

    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_safe_construct_n(k_component, 8))
        .SetReturn(k_string_handle_valid);

    STRICT_EXPECTED_CALL(STRING_delete(k_string_handle_valid));
    STRICT_EXPECTED_CALL(h_free((void*)UT_MEM, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    result = io_cs_create_from_string(k_component, &cs_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_invalid_format, result);
}

// 
// Test io_cs_create_from_string when connection string contains a bad host name
// 
TEST_FUNCTION(io_cs_create_from_string__arg_connection_string_bad_hostname_1)
{
    static const STRING_HANDLE k_hostname_string_valid = (STRING_HANDLE)0x62423;
    static const char* k_component = "HostName";
    io_cs_t* cs_valid;
    int32_t result;

    REGISTER_GLOBAL_MOCK_HOOK(string_key_value_parser, string_key_value_parser__hook_connection_string_with_1_component);
    REGISTER_GLOBAL_MOCK_RETURNS(string_key_value_parser, er_ok, er_invalid_format);

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_cs_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(string_key_value_parser(k_component, IGNORED_PTR_ARG, ';', (void*)UT_MEM))
        .IgnoreArgument(2)
        .SetFailReturn(er_invalid_format);
    

    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_safe_construct_n(k_component, 8))
        .SetReturn(k_hostname_string_valid);

    STRICT_EXPECTED_CALL(STRING_c_str(k_hostname_string_valid))
        .SetReturn(k_component);

    STRICT_EXPECTED_CALL(STRING_delete(k_hostname_string_valid));
    STRICT_EXPECTED_CALL(h_free((void*)UT_MEM, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    result = io_cs_create_from_string(k_component, &cs_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_invalid_format, result);
}

// 
// Test io_cs_create_from_string when connection string contains a bad host name
// 
TEST_FUNCTION(io_cs_create_from_string__arg_connection_string_bad_hostname_2)
{
    static const STRING_HANDLE k_hostname_string_valid = (STRING_HANDLE)0x62423;
    static const char* k_component = "HostName";
    io_cs_t* cs_valid;
    int32_t result;

    REGISTER_GLOBAL_MOCK_HOOK(string_key_value_parser, string_key_value_parser__hook_connection_string_with_1_component);
    REGISTER_GLOBAL_MOCK_RETURNS(string_key_value_parser, er_ok, er_invalid_format);

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_cs_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(string_key_value_parser(k_component, IGNORED_PTR_ARG, ';', (void*)UT_MEM))
        .IgnoreArgument(2)
        .SetFailReturn(er_invalid_format);
    

    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_safe_construct_n(k_component, 8))
        .SetReturn(k_hostname_string_valid);

    STRICT_EXPECTED_CALL(STRING_c_str(k_hostname_string_valid))
        .SetReturn("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.test.com");

    STRICT_EXPECTED_CALL(STRING_delete(k_hostname_string_valid));
    STRICT_EXPECTED_CALL(h_free((void*)UT_MEM, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    result = io_cs_create_from_string(k_component, &cs_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_invalid_format, result);
}

// 
// Test io_cs_create_from_string when connection string contains a bad host name
// 
TEST_FUNCTION(io_cs_create_from_string__arg_connection_string_bad_hostname_3)
{
    static const STRING_HANDLE k_hostname_string_valid = (STRING_HANDLE)0x62423;
    static const char* k_component = "HostName";
    io_cs_t* cs_valid;
    int32_t result;

    REGISTER_GLOBAL_MOCK_HOOK(string_key_value_parser, string_key_value_parser__hook_connection_string_with_1_component);
    REGISTER_GLOBAL_MOCK_RETURNS(string_key_value_parser, er_ok, er_invalid_format);

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_cs_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(string_key_value_parser(k_component, IGNORED_PTR_ARG, ';', (void*)UT_MEM))
        .IgnoreArgument(2);
    

        EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_safe_construct_n(k_component, 8))
        .SetReturn(k_hostname_string_valid);
    STRICT_EXPECTED_CALL(STRING_c_str(k_hostname_string_valid))
        .SetReturn("-test.test.com");
    STRICT_EXPECTED_CALL(STRING_delete(k_hostname_string_valid));
    STRICT_EXPECTED_CALL(h_free((void*)UT_MEM, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    result = io_cs_create_from_string(k_component, &cs_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_invalid_format, result);
}

// 
// Test io_cs_create_from_string when connection string contains a bad host name
// 
TEST_FUNCTION(io_cs_create_from_string__arg_connection_string_bad_hostname_4)
{
    static const STRING_HANDLE k_hostname_string_valid = (STRING_HANDLE)0x62423;
    static const char* k_component = "HostName";
    io_cs_t* cs_valid;
    int32_t result;

    REGISTER_GLOBAL_MOCK_HOOK(string_key_value_parser, string_key_value_parser__hook_connection_string_with_1_component);
    REGISTER_GLOBAL_MOCK_RETURNS(string_key_value_parser, er_ok, er_invalid_format);

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_cs_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(string_key_value_parser(k_component, IGNORED_PTR_ARG, ';', (void*)UT_MEM))
        .IgnoreArgument(2);
    

    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_safe_construct_n(k_component, 8))
        .SetReturn(k_hostname_string_valid);
    STRICT_EXPECTED_CALL(STRING_c_str(k_hostname_string_valid))
        .SetReturn("");
    STRICT_EXPECTED_CALL(STRING_delete(k_hostname_string_valid));
    STRICT_EXPECTED_CALL(h_free((void*)UT_MEM, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    result = io_cs_create_from_string(k_component, &cs_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_invalid_format, result);
}

//
// Parser mock hook that returns a device id
//
static int32_t string_key_value_parser__hook_connection_string_with_hostname_and_device_id(
    const char* connection_string,
    fn_parse_cb_t visitor,
    char delim,
    void* ctx
)
{
    int32_t result;
    (void)delim;
    (void)connection_string;
    result = visitor(ctx, "HostName", 8, "Hub.Suffix", 2);
    if (result != er_ok)
        return result;
    result = visitor(ctx, "DeviceId", 8, "DeviceId", 2);
    if (result != er_ok)
        return result;
    return result;
}

// 
// Test io_cs_create_from_string when connection string contains device id with bad char
// 
TEST_FUNCTION(io_cs_create_from_string__arg_connection_string_bad_device_id_1)
{
    static const STRING_HANDLE k_hostname_string_valid = (STRING_HANDLE)9999;
    static const STRING_HANDLE k_device_id_string_valid = (STRING_HANDLE)8888;
    static const char* k_connection_string_valid = "some_connection_string";
    io_cs_t* cs_valid;
    int32_t result;

    REGISTER_GLOBAL_MOCK_HOOK(string_key_value_parser, string_key_value_parser__hook_connection_string_with_hostname_and_device_id);
    REGISTER_GLOBAL_MOCK_RETURNS(string_key_value_parser, er_ok, er_invalid_format);

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_cs_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(string_key_value_parser(k_connection_string_valid, IGNORED_PTR_ARG, ';', (void*)UT_MEM))
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
        .SetReturn("bad&bada");

    STRICT_EXPECTED_CALL(STRING_delete(k_hostname_string_valid));
    STRICT_EXPECTED_CALL(STRING_delete(k_device_id_string_valid));
    STRICT_EXPECTED_CALL(h_free((void*)UT_MEM, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    result = io_cs_create_from_string(k_connection_string_valid, &cs_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_invalid_format, result);
}

// 
// Test io_cs_create_from_string when connection string contains too long device id
// 
TEST_FUNCTION(io_cs_create_from_string__arg_connection_string_bad_device_id_2)
{
    static const STRING_HANDLE k_hostname_string_valid = (STRING_HANDLE)9999;
    static const STRING_HANDLE k_device_id_string_valid = (STRING_HANDLE)8888;
    static const char* k_connection_string_valid = "some_connection_string";
    io_cs_t* cs_valid;
    int32_t result;

    REGISTER_GLOBAL_MOCK_HOOK(string_key_value_parser, string_key_value_parser__hook_connection_string_with_hostname_and_device_id);
    REGISTER_GLOBAL_MOCK_RETURNS(string_key_value_parser, er_ok, er_invalid_format);

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_cs_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(string_key_value_parser(k_connection_string_valid, IGNORED_PTR_ARG, ';', (void*)UT_MEM))
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
        .SetReturn("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

    STRICT_EXPECTED_CALL(STRING_delete(k_hostname_string_valid));
    STRICT_EXPECTED_CALL(STRING_delete(k_device_id_string_valid));
    STRICT_EXPECTED_CALL(h_free((void*)UT_MEM, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    result = io_cs_create_from_string(k_connection_string_valid, &cs_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_invalid_format, result);
}

// 
// Test io_cs_create_from_string when connection string contains empty device id
// 
TEST_FUNCTION(io_cs_create_from_string__arg_connection_string_bad_device_id_3)
{
    static const STRING_HANDLE k_hostname_string_valid = (STRING_HANDLE)9999;
    static const STRING_HANDLE k_device_id_string_valid = (STRING_HANDLE)8888;
    static const char* k_connection_string_valid = "some_connection_string";
    io_cs_t* cs_valid;
    int32_t result;

    REGISTER_GLOBAL_MOCK_HOOK(string_key_value_parser, string_key_value_parser__hook_connection_string_with_hostname_and_device_id);
    REGISTER_GLOBAL_MOCK_RETURNS(string_key_value_parser, er_ok, er_invalid_format);

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_cs_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(string_key_value_parser(k_connection_string_valid, IGNORED_PTR_ARG, ';', (void*)UT_MEM))
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
        .SetReturn("");

    STRICT_EXPECTED_CALL(STRING_delete(k_hostname_string_valid));
    STRICT_EXPECTED_CALL(STRING_delete(k_device_id_string_valid));
    STRICT_EXPECTED_CALL(h_free((void*)UT_MEM, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    result = io_cs_create_from_string(k_connection_string_valid, &cs_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_invalid_format, result);
}
// 
// Test io_cs_create_from_string passing as created argument an invalid io_cs_t** value 
// 
TEST_FUNCTION(io_cs_create_from_string__arg_created_null)
{
    static const char* k_connection_string_valid = "some_connection_string";
    int32_t result;

    // arrange 

    // act 
    result = io_cs_create_from_string(k_connection_string_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_cs_create_from_string unhappy path 
// 
TEST_FUNCTION(io_cs_create_from_string__neg)
{
    static const STRING_HANDLE k_hostname_string_valid = (STRING_HANDLE)9999;
    static const STRING_HANDLE k_device_id_string_valid = (STRING_HANDLE)8888;
    static const char* k_connection_string_valid = "some_connection_string";
    static const char* k_endpoint = "mqtts://Endpoint/";
    static const size_t k_endpoint_len = 17;
    io_cs_t* cs_valid;
    int32_t result;

    REGISTER_GLOBAL_MOCK_HOOK(string_key_value_parser, string_key_value_parser__hook_2);
    REGISTER_GLOBAL_MOCK_RETURNS(string_key_value_parser, er_ok, er_invalid_format);

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_cs_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(string_key_value_parser(k_connection_string_valid, IGNORED_PTR_ARG, ';', (void*)UT_MEM))
        .IgnoreArgument(2)
        .SetFailReturn(er_invalid_format);

    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_safe_construct_n("Hub.Suffix", 14))
        .SetReturn(k_hostname_string_valid)
        .SetFailReturn(NULL);

    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_safe_construct_n(k_endpoint, k_endpoint_len))
        .SetReturn((STRING_HANDLE)1)
        .SetFailReturn(NULL);

    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_safe_construct_n("DeviceId", 8))
        .SetReturn(k_device_id_string_valid)
        .SetFailReturn(NULL);

    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_safe_construct_n("EntityPath", 10))
        .SetReturn((STRING_HANDLE)2)
        .SetFailReturn(NULL);

    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_safe_construct_n("SharedAccessKeyName", 19))
        .SetReturn((STRING_HANDLE)3)
        .SetFailReturn(NULL);

    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_safe_construct_n("SharedAccessKey", 15))
        .SetReturn((STRING_HANDLE)4)
        .SetFailReturn(NULL);

    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_safe_construct_n("ConsumerGroup", 13))
        .SetReturn((STRING_HANDLE)5)
        .SetFailReturn(NULL);

    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    EXPECTED_CALL(string_is_equal_nocase(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_safe_construct_n("32", 2))
        .SetReturn((STRING_HANDLE)5)
        .SetFailReturn(NULL);

    // act 
        UMOCK_C_NEGATIVE_TESTS_ACT();
        memset(UT_MEM, 0, sizeof(UT_MEM));
    result = io_cs_create_from_string(k_connection_string_valid, &cs_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, 
        er_out_of_memory,  er_invalid_format, er_invalid_format, er_out_of_memory,  er_invalid_format, 
        er_invalid_format, er_invalid_format, er_invalid_format, er_invalid_format, er_invalid_format, 
        er_invalid_format, er_out_of_memory,  er_invalid_format, er_invalid_format, er_out_of_memory,  
        er_invalid_format, er_invalid_format, er_invalid_format, er_invalid_format, er_invalid_format,
        er_invalid_format, er_invalid_format, er_invalid_format, er_out_of_memory,  er_invalid_format, 
        er_invalid_format, er_invalid_format, er_out_of_memory,  er_invalid_format, er_invalid_format, 
        er_invalid_format, er_invalid_format, er_out_of_memory,  er_invalid_format, er_invalid_format, 
        er_invalid_format, er_invalid_format, er_invalid_format, er_out_of_memory,  er_invalid_format, 
        er_invalid_format, er_invalid_format, er_invalid_format, er_invalid_format, er_invalid_format, 
        er_out_of_memory,  er_invalid_format);
}

// 
// Test io_cs_create_from_raw_file passing as file_name argument an invalid const char* value 
// 
TEST_FUNCTION(io_cs_create_from_raw_file__arg_file_name_null)
{
    io_cs_t* cs_valid;
    int32_t result;

    // arrange 

    // act 
    result = io_cs_create_from_raw_file(NULL, &cs_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_cs_create_from_raw_file passing as created argument an invalid const char* value 
// 
TEST_FUNCTION(io_cs_create_from_raw_file__arg_created_null)
{
    static const char* k_file_name_valid = "cs.txt";
    int32_t result;

    // arrange 

    // act 
    result = io_cs_create_from_raw_file(k_file_name_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_cs_create_from_raw_file unhappy path 
// 
TEST_FUNCTION(io_cs_create_from_raw_file__neg_1)
{
    static const char* k_file_name_valid = "cs.txt";
    io_cs_t* cs_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(pal_get_real_path(k_file_name_valid, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .SetReturn(er_not_found);

    // act 
    result = io_cs_create_from_raw_file(k_file_name_valid, &cs_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_not_found, result);
}

// 
// Test io_cs_create_from_raw_file unhappy path 
// 
TEST_FUNCTION(io_cs_create_from_raw_file__neg_2)
{
    static const char* k_file_name_valid = "cs.txt";
    static const char* k_real_name_valid = "/cs.txt";
    io_cs_t* cs_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(pal_get_real_path(k_file_name_valid, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_path(&k_real_name_valid, sizeof(&k_real_name_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_file_exists(k_real_name_valid))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(pal_free_path(k_real_name_valid));

    // act 
    result = io_cs_create_from_raw_file(k_file_name_valid, &cs_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_not_found, result);
}

// 
// Test io_cs_create_from_raw_file unhappy path 
// 
TEST_FUNCTION(io_cs_create_from_raw_file__neg_3)
{
    static const char* k_file_name_valid = "cs.txt";
    static const char* k_real_name_valid = "/cs.txt";
    io_cs_t* cs_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(pal_get_real_path(k_file_name_valid, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_path(&k_real_name_valid, sizeof(&k_real_name_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_file_exists(k_real_name_valid))
        .SetReturn(true);
    STRICT_EXPECTED_CALL(io_file_stream_init(IGNORED_PTR_ARG, k_real_name_valid, NULL))
        .IgnoreArgument(1)
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(pal_free_path(k_real_name_valid));

    // act 
    result = io_cs_create_from_raw_file(k_file_name_valid, &cs_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_reading, result);
}

// 
// Test io_cs_create_from_raw_file unhappy path 
// 
TEST_FUNCTION(io_cs_create_from_raw_file__neg_4)
{
    static const char* k_file_name_valid = "cs.txt";
    static const char* k_real_name_valid = "/cs.txt";
    io_stream_t* k_stream_valid = (io_stream_t*)0x1343;
    io_cs_t* cs_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(pal_get_real_path(k_file_name_valid, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_path(&k_real_name_valid, sizeof(&k_real_name_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_file_exists(k_real_name_valid))
        .SetReturn(true);
    STRICT_EXPECTED_CALL(io_file_stream_init(IGNORED_PTR_ARG, k_real_name_valid, NULL))
        .IgnoreArgument(1)
        .SetReturn(k_stream_valid);
    STRICT_EXPECTED_CALL(io_stream_readable(k_stream_valid))
        .SetReturn(4000);
    STRICT_EXPECTED_CALL(io_stream_close(k_stream_valid));
    STRICT_EXPECTED_CALL(pal_free_path(k_real_name_valid));

    // act 
    result = io_cs_create_from_raw_file(k_file_name_valid, &cs_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_reading, result);
}

// 
// Test io_cs_create_from_raw_file unhappy path 
// 
TEST_FUNCTION(io_cs_create_from_raw_file__neg_5)
{
    static const char* k_file_name_valid = "cs.txt";
    static const char* k_real_name_valid = "/cs.txt";
    io_stream_t* k_stream_valid = (io_stream_t*)0x1343;
    io_cs_t* cs_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(pal_get_real_path(k_file_name_valid, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_path(&k_real_name_valid, sizeof(&k_real_name_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_file_exists(k_real_name_valid))
        .SetReturn(true);
    STRICT_EXPECTED_CALL(io_file_stream_init(IGNORED_PTR_ARG, k_real_name_valid, NULL))
        .IgnoreArgument(1)
        .SetReturn(k_stream_valid);
    STRICT_EXPECTED_CALL(io_stream_readable(k_stream_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(io_stream_close(k_stream_valid));
    STRICT_EXPECTED_CALL(pal_free_path(k_real_name_valid));

    // act 
    result = io_cs_create_from_raw_file(k_file_name_valid, &cs_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_reading, result);
}

// 
// Test io_cs_create_from_raw_file unhappy path 
// 
TEST_FUNCTION(io_cs_create_from_raw_file__neg_6)
{
    static const char* k_file_name_valid = "cs.txt";
    static const char* k_real_name_valid = "/cs.txt";
    io_stream_t* k_stream_valid = (io_stream_t*)0x1343;
    io_cs_t* cs_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(pal_get_real_path(k_file_name_valid, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_path(&k_real_name_valid, sizeof(&k_real_name_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_file_exists(k_real_name_valid))
        .SetReturn(true);
    STRICT_EXPECTED_CALL(io_file_stream_init(IGNORED_PTR_ARG, k_real_name_valid, NULL))
        .IgnoreArgument(1)
        .SetReturn(k_stream_valid);
    STRICT_EXPECTED_CALL(io_stream_readable(k_stream_valid))
        .SetReturn(400);
    STRICT_EXPECTED_CALL(h_realloc(400 + 1, NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(io_stream_close(k_stream_valid));
    STRICT_EXPECTED_CALL(pal_free_path(k_real_name_valid));

    // act 
    result = io_cs_create_from_raw_file(k_file_name_valid, &cs_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_out_of_memory, result);
}

// 
// Test io_cs_create_from_raw_file unhappy path 
// 
TEST_FUNCTION(io_cs_create_from_raw_file__neg_7)
{
    static const char* k_file_name_valid = "cs.txt";
    static const char* k_real_name_valid = "/cs.txt";
    io_stream_t* k_stream_valid = (io_stream_t*)0x1343;
    io_cs_t* cs_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(pal_get_real_path(k_file_name_valid, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_path(&k_real_name_valid, sizeof(&k_real_name_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_file_exists(k_real_name_valid))
        .SetReturn(true);
    STRICT_EXPECTED_CALL(io_file_stream_init(IGNORED_PTR_ARG, k_real_name_valid, NULL))
        .IgnoreArgument(1)
        .SetReturn(k_stream_valid);
    STRICT_EXPECTED_CALL(io_stream_readable(k_stream_valid))
        .SetReturn(400);
    STRICT_EXPECTED_CALL(h_realloc(400 + 1, NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(io_stream_read(k_stream_valid, UT_MEM, 400, IGNORED_PTR_ARG))
        .IgnoreArgument(4)
        .SetReturn(er_reading);
    STRICT_EXPECTED_CALL(io_stream_close(k_stream_valid));
    STRICT_EXPECTED_CALL(h_free((void*)UT_MEM, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);
    STRICT_EXPECTED_CALL(pal_free_path(k_real_name_valid));

    // act 
    result = io_cs_create_from_raw_file(k_file_name_valid, &cs_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_reading, result);
}

// 
// Test io_cs_create happy path 
// 
TEST_FUNCTION(io_cs_create__success)
{
    static const STRING_HANDLE k_hostname_string_valid = (STRING_HANDLE)9999;
    static const char* k_host_name_valid = "host.hub";
    static const char* k_device_id_valid = "device";
    static const char* k_shared_access_key_name_valid = "sakn";
    static const char* k_shared_access_key_valid = "sak";
    io_cs_t* cs_valid;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_cs_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(STRING_construct(k_host_name_valid))
        .SetReturn(k_hostname_string_valid);
    STRICT_EXPECTED_CALL(STRING_construct(k_device_id_valid))
        .SetReturn((STRING_HANDLE)io_cs_entry_device_id);
    STRICT_EXPECTED_CALL(STRING_construct(k_shared_access_key_name_valid))
        .SetReturn((STRING_HANDLE)io_cs_entry_shared_access_key_name);
    STRICT_EXPECTED_CALL(STRING_construct(k_shared_access_key_valid))
        .SetReturn((STRING_HANDLE)io_cs_entry_shared_access_key);
    STRICT_EXPECTED_CALL(pal_caps()).SetReturn((uint32_t)~pal_cap_cred);
    STRICT_EXPECTED_CALL(STRING_c_str(k_hostname_string_valid))
        .SetReturn(k_host_name_valid);
    STRICT_EXPECTED_CALL(STRING_c_str((STRING_HANDLE)io_cs_entry_device_id))
        .SetReturn(k_device_id_valid);

    // act 
    result = io_cs_create(k_host_name_valid, k_device_id_valid, 
        k_shared_access_key_name_valid, k_shared_access_key_valid, &cs_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_cs_create passing as host_name argument an invalid const char* value 
// 
TEST_FUNCTION(io_cs_create__arg_host_name_null)
{
    static const char* k_device_id_valid = "device";
    static const char* k_shared_access_key_name_valid = "sakn";
    static const char* k_shared_access_key_valid = "sak";
    io_cs_t* cs_valid;
    int32_t result;

    // arrange 

    // act 
    result = io_cs_create(NULL, k_device_id_valid,
        k_shared_access_key_name_valid, k_shared_access_key_valid, &cs_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_cs_create passing as device_id argument an invalid const char* value 
// 
TEST_FUNCTION(io_cs_create__arg_device_id_null)
{
    static const STRING_HANDLE k_hostname_string_valid = (STRING_HANDLE)9999;
    static const char* k_host_name_valid = "host.hub";
    static const char* k_shared_access_key_name_valid = "sakn";
    static const char* k_shared_access_key_valid = "sak";
    io_cs_t* cs_valid;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_cs_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(STRING_construct(k_host_name_valid))
        .SetReturn(k_hostname_string_valid);
    STRICT_EXPECTED_CALL(STRING_construct(k_shared_access_key_name_valid))
        .SetReturn((STRING_HANDLE)io_cs_entry_shared_access_key_name);
    STRICT_EXPECTED_CALL(STRING_construct(k_shared_access_key_valid))
        .SetReturn((STRING_HANDLE)io_cs_entry_shared_access_key);
    STRICT_EXPECTED_CALL(pal_caps()).SetReturn((uint32_t)~pal_cap_cred);
    STRICT_EXPECTED_CALL(STRING_c_str(k_hostname_string_valid))
        .SetReturn(k_host_name_valid);

    // act 
    result = io_cs_create(k_host_name_valid, NULL,
        k_shared_access_key_name_valid, k_shared_access_key_valid, &cs_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_cs_create passing as shared_access_key_name argument an invalid const char* value 
// 
TEST_FUNCTION(io_cs_create__arg_shared_access_key_name_null)
{
    static const STRING_HANDLE k_hostname_string_valid = (STRING_HANDLE)9999;
    static const char* k_host_name_valid = "host.hub";
    static const char* k_device_id_valid = "device";
    static const char* k_shared_access_key_valid = "sak";
    io_cs_t* cs_valid;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_cs_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(STRING_construct(k_host_name_valid))
        .SetReturn(k_hostname_string_valid);
    STRICT_EXPECTED_CALL(STRING_construct(k_device_id_valid))
        .SetReturn((STRING_HANDLE)io_cs_entry_device_id);
    STRICT_EXPECTED_CALL(STRING_construct(k_shared_access_key_valid))
        .SetReturn((STRING_HANDLE)io_cs_entry_shared_access_key);
    STRICT_EXPECTED_CALL(pal_caps()).SetReturn((uint32_t)~pal_cap_cred);
    STRICT_EXPECTED_CALL(STRING_c_str(k_hostname_string_valid))
        .SetReturn(k_host_name_valid);
    STRICT_EXPECTED_CALL(STRING_c_str((STRING_HANDLE)io_cs_entry_device_id))
        .SetReturn(k_device_id_valid);

    // act 
    result = io_cs_create(k_host_name_valid, k_device_id_valid, NULL, k_shared_access_key_valid, &cs_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_cs_create passing as shared_access_key argument an invalid const char* value 
// 
TEST_FUNCTION(io_cs_create__arg_shared_access_key_null)
{
    static const STRING_HANDLE k_hostname_string_valid = (STRING_HANDLE)9999;
    static const char* k_host_name_valid = "host.hub";
    static const char* k_device_id_valid = "device";
    static const char* k_shared_access_key_name_valid = "sakn";
    io_cs_t* cs_valid;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_cs_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(STRING_construct(k_host_name_valid))
        .SetReturn(k_hostname_string_valid);
    STRICT_EXPECTED_CALL(STRING_construct(k_device_id_valid))
        .SetReturn((STRING_HANDLE)io_cs_entry_device_id);
    STRICT_EXPECTED_CALL(STRING_construct(k_shared_access_key_name_valid))
        .SetReturn((STRING_HANDLE)io_cs_entry_shared_access_key_name);
    STRICT_EXPECTED_CALL(STRING_c_str(k_hostname_string_valid))
        .SetReturn(k_host_name_valid);
    STRICT_EXPECTED_CALL(STRING_c_str((STRING_HANDLE)io_cs_entry_device_id))
        .SetReturn(k_device_id_valid);

    // act 
    result = io_cs_create(k_host_name_valid, k_device_id_valid, k_shared_access_key_name_valid, NULL, &cs_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_cs_create passing as created argument an invalid io_cs_t** value 
// 
TEST_FUNCTION(io_cs_create__arg_created_null)
{
    static const char* k_host_name_valid = "host.hub";
    static const char* k_device_id_valid = "device";
    static const char* k_shared_access_key_name_valid = "sakn";
    static const char* k_shared_access_key_valid = "sak";
    int32_t result;

    // arrange 

    // act 
    result = io_cs_create(k_host_name_valid, k_device_id_valid,
        k_shared_access_key_name_valid, k_shared_access_key_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_cs_create unhappy path 
// 
TEST_FUNCTION(io_cs_create__neg)
{
    static const STRING_HANDLE k_hostname_string_valid = (STRING_HANDLE)0x42242;
    static const char* k_host_name_valid = "host.hub";
    static const char* k_device_id_valid = "device";
    static const char* k_shared_access_key_name_valid = "sakn";
    static const char* k_shared_access_key_valid = "sak";
    io_cs_t* cs_valid;
    int32_t result;

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_cs_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(STRING_construct(k_host_name_valid))
        .SetReturn(k_hostname_string_valid)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(STRING_construct(k_device_id_valid))
        .SetReturn((STRING_HANDLE)io_cs_entry_device_id)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(STRING_construct(k_shared_access_key_name_valid))
        .SetReturn((STRING_HANDLE)io_cs_entry_shared_access_key_name)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(STRING_construct(k_shared_access_key_valid))
        .SetReturn((STRING_HANDLE)io_cs_entry_shared_access_key)
        .SetFailReturn(NULL);

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    memset(UT_MEM, 0, sizeof(UT_MEM));
    result = io_cs_create(k_host_name_valid, k_device_id_valid,
        k_shared_access_key_name_valid, k_shared_access_key_valid, &cs_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_out_of_memory);
}

// 
// Test io_cs_clone happy path 
// 
TEST_FUNCTION(io_cs_clone__success_1)
{
    io_cs_t orig_valid;
    io_cs_t* clone_valid;
    int32_t result;

    for (size_t i = 0; i < _countof(orig_valid.entries); i++)
        orig_valid.entries[i] = (STRING_HANDLE)(0x8000 | i);

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_cs_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM)
        .SetFailReturn(NULL);
    for (size_t i = 0; i < _countof(orig_valid.entries); i++)
    {
        STRICT_EXPECTED_CALL(STRING_clone((STRING_HANDLE)(0x8000 | i)))
            .SetReturn((STRING_HANDLE)(0x1000 | i))
            .SetFailReturn(NULL);
    }

    // act 
    result = io_cs_clone(&orig_valid, &clone_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(void_ptr, (void*)UT_MEM, (void*)clone_valid);
}

// 
// Test io_cs_clone happy path 
// 
TEST_FUNCTION(io_cs_clone__success_2)
{
    io_cs_t orig_valid;
    io_cs_t* clone_valid;
    int32_t result;

    memset(&orig_valid, 0, sizeof(orig_valid));

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_cs_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM)
        .SetFailReturn(NULL);

    // act 
    result = io_cs_clone(&orig_valid, &clone_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(void_ptr, (void*)UT_MEM, (void*)clone_valid);
}

// 
// Test io_cs_clone passing as orig argument an invalid io_cs_t* value 
// 
TEST_FUNCTION(io_cs_clone__arg_orig_null)
{
    io_cs_t* clone_valid;
    int32_t result;

    // arrange 

    // act 
    result = io_cs_clone(NULL, &clone_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_cs_clone passing as cloned argument an invalid io_cs_t** value 
// 
TEST_FUNCTION(io_cs_clone__arg_cloned_null)
{
    io_cs_t orig_valid;
    int32_t result;

    for (size_t i = 0; i < _countof(orig_valid.entries); i++)
        orig_valid.entries[i] = (STRING_HANDLE)(0x2000 | i);

    // arrange 

    // act 
    result = io_cs_clone(&orig_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_cs_clone unhappy path 
// 
TEST_FUNCTION(io_cs_clone__neg)
{
    io_cs_t orig_valid;
    io_cs_t* clone_valid;
    int32_t result;

    for (size_t i = 0; i < _countof(orig_valid.entries); i++)
        orig_valid.entries[i] = (STRING_HANDLE)(0x8000 | i);

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_cs_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM)
        .SetFailReturn(NULL);
    for (size_t i = 0; i < _countof(orig_valid.entries); i++)
    {
        STRICT_EXPECTED_CALL(STRING_clone((STRING_HANDLE)(0x8000 | i)))
            .SetReturn((STRING_HANDLE)(0x1000 | i))
            .SetFailReturn(NULL);
    }
    STRICT_EXPECTED_CALL(h_free((void*)UT_MEM, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
        UMOCK_C_NEGATIVE_TESTS_ACT();
        result = io_cs_clone(&orig_valid, &clone_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, 
        er_out_of_memory, er_out_of_memory, er_out_of_memory, er_out_of_memory, er_out_of_memory,
        er_out_of_memory, er_out_of_memory, er_out_of_memory, er_out_of_memory, er_out_of_memory,
        er_out_of_memory, er_out_of_memory, er_out_of_memory, er_ok);
}

// 
// Test io_cs_append_to_STRING happy path 
// 
TEST_FUNCTION(io_cs_append_to_STRING__success)
{
    const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x23423;
    io_cs_t connection_string_valid;
    int32_t result;

    for (size_t i = 0; i < _countof(connection_string_valid.entries); i++)
        connection_string_valid.entries[i] = (STRING_HANDLE)(0x80000 | i);

    // arrange 
    STRICT_EXPECTED_CALL(STRING_length(k_string_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, "HostName="))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(k_string_valid, 
        (STRING_HANDLE)(0x80000 | io_cs_entry_host_name)))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_length(k_string_valid))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, ";"))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, "DeviceId="))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(k_string_valid, 
        (STRING_HANDLE)(0x80000 | io_cs_entry_device_id)))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_length(k_string_valid))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, ";"))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, "Endpoint="))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(k_string_valid, 
        (STRING_HANDLE)(0x80000 | io_cs_entry_endpoint)))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_length(k_string_valid))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, ";"))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, "EntityPath="))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(k_string_valid, 
        (STRING_HANDLE)(0x80000 | io_cs_entry_entity)))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_length(k_string_valid))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, ";"))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, "SharedAccessKeyName="))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(k_string_valid, 
        (STRING_HANDLE)(0x80000 | io_cs_entry_shared_access_key_name)))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_length(k_string_valid))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, ";"))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, "SharedAccessKey="))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(k_string_valid, 
        (STRING_HANDLE)(0x80000 | io_cs_entry_shared_access_key)))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_length(k_string_valid))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, ";"))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, "ConsumerGroup="))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(k_string_valid, 
        (STRING_HANDLE)(0x80000 | io_cs_entry_consumer_group)))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_length(k_string_valid))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, ";"))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, "Partitions="))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(k_string_valid, 
        (STRING_HANDLE)(0x80000 | io_cs_entry_partition_count)))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_length(k_string_valid))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, ";"))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, "EndpointName="))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(k_string_valid,
        (STRING_HANDLE)(0x80000 | io_cs_entry_endpoint_name)))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_length(k_string_valid))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, ";"))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, "SharedAccessToken="))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(k_string_valid,
        (STRING_HANDLE)(0x80000 | io_cs_entry_shared_access_token)))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_length(k_string_valid))
        .SetReturn(1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, ";"))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, "SharedAccessKeyHandle="))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(k_string_valid,
        (STRING_HANDLE)(0x80000 | io_cs_entry_shared_access_key_handle)))
        .SetReturn(0);

    // act 
    result = io_cs_append_to_STRING(&connection_string_valid, k_string_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_cs_append_to_STRING passing as connection_string argument an invalid io_cs_t* value 
// 
TEST_FUNCTION(io_cs_append_to_STRING__arg_connection_string_null)
{
    const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x23423;
    int32_t result;

    // arrange 

    // act 
    result = io_cs_append_to_STRING(NULL, k_string_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_cs_append_to_STRING passing as string argument an invalid STRING_HANDLE value 
// 
TEST_FUNCTION(io_cs_append_to_STRING__arg_string_null)
{
    io_cs_t connection_string_valid;
    int32_t result;

    // arrange 

    // act 
    result = io_cs_append_to_STRING(&connection_string_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_cs_append_to_STRING unhappy path 
// 
TEST_FUNCTION(io_cs_append_to_STRING__neg)
{
    const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x23423;
    io_cs_t connection_string_valid;
    int32_t result;

    for (size_t i = 0; i < _countof(connection_string_valid.entries); i++)
        connection_string_valid.entries[i] = (STRING_HANDLE)(0x80000 | i);

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(STRING_length(k_string_valid))
        .SetReturn(0)
        .SetFailReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, "HostName="))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(k_string_valid, 
        (STRING_HANDLE)(0x80000 | io_cs_entry_host_name)))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_length(k_string_valid))
        .SetReturn(1)
        .SetFailReturn(1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, ";"))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, "DeviceId="))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(k_string_valid, 
        (STRING_HANDLE)(0x80000 | io_cs_entry_device_id)))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_length(k_string_valid))
        .SetReturn(1)
        .SetFailReturn(1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, ";"))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, "Endpoint="))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(k_string_valid, 
        (STRING_HANDLE)(0x80000 | io_cs_entry_endpoint)))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_length(k_string_valid))
        .SetReturn(1)
        .SetFailReturn(1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, ";"))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, "EntityPath="))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(k_string_valid, 
        (STRING_HANDLE)(0x80000 | io_cs_entry_entity)))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_length(k_string_valid))
        .SetReturn(1)
        .SetFailReturn(1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, ";"))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, "SharedAccessKeyName="))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(k_string_valid, 
        (STRING_HANDLE)(0x80000 | io_cs_entry_shared_access_key_name)))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_length(k_string_valid))
        .SetReturn(1)
        .SetFailReturn(1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, ";"))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, "SharedAccessKey="))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(k_string_valid, 
        (STRING_HANDLE)(0x80000 | io_cs_entry_shared_access_key)))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_length(k_string_valid))
        .SetReturn(1)
        .SetFailReturn(1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, ";"))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, "ConsumerGroup="))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(k_string_valid, 
        (STRING_HANDLE)(0x80000 | io_cs_entry_consumer_group)))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_length(k_string_valid))
        .SetReturn(1)
        .SetFailReturn(1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, ";"))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, "Partitions="))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(k_string_valid, 
        (STRING_HANDLE)(0x80000 | io_cs_entry_partition_count)))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_length(k_string_valid))
        .SetReturn(1)
        .SetFailReturn(1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, ";"))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, "EndpointName="))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(k_string_valid,
        (STRING_HANDLE)(0x80000 | io_cs_entry_endpoint_name)))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_length(k_string_valid))
        .SetReturn(1)
        .SetFailReturn(1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, ";"))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, "SharedAccessToken="))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(k_string_valid,
        (STRING_HANDLE)(0x80000 | io_cs_entry_shared_access_token)))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_length(k_string_valid))
        .SetReturn(1)
        .SetFailReturn(1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, ";"))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, "SharedAccessKeyHandle="))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(k_string_valid,
        (STRING_HANDLE)(0x80000 | io_cs_entry_shared_access_key_handle)))
        .SetReturn(0)
        .SetFailReturn(-1);

    // act 
        UMOCK_C_NEGATIVE_TESTS_ACT();
        result = io_cs_append_to_STRING(&connection_string_valid, k_string_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, 
        er_ok,              er_out_of_memory,    er_out_of_memory, er_ok,            er_out_of_memory,
        er_out_of_memory, er_out_of_memory,    er_ok,            er_out_of_memory, er_out_of_memory,
        er_out_of_memory, er_ok,            er_out_of_memory, er_out_of_memory, er_out_of_memory,
        er_ok,              er_out_of_memory,    er_out_of_memory, er_out_of_memory, er_ok,
        er_out_of_memory, er_out_of_memory, er_out_of_memory, er_ok,            er_out_of_memory,
        er_out_of_memory, er_out_of_memory, er_ok,            er_out_of_memory, er_out_of_memory,
        er_out_of_memory, er_ok,            er_out_of_memory, er_out_of_memory, er_out_of_memory,
        er_ok, er_out_of_memory, er_out_of_memory, er_out_of_memory, er_ok, er_out_of_memory);
}

// 
// Test io_cs_to_STRING happy path 
// 
TEST_FUNCTION(io_cs_to_STRING__success)
{
    const STRING_HANDLE k_string1_valid = (STRING_HANDLE)0x23423;
    const STRING_HANDLE k_string2_valid = (STRING_HANDLE)0x22343;
    io_cs_t connection_string_valid;
    STRING_HANDLE result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[0] = k_string2_valid;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_new())
        .SetReturn(k_string1_valid);
    STRICT_EXPECTED_CALL(STRING_length(k_string1_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat(k_string1_valid, "HostName="))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(k_string1_valid, k_string2_valid))
        .SetReturn(0);

    // act 
    result = io_cs_to_STRING(&connection_string_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void_ptr, (void*)k_string1_valid, (void*)result);
}

// 
// Test io_cs_to_STRING passing as connection_string argument an invalid io_cs_t* value 
// 
TEST_FUNCTION(io_cs_to_STRING__arg_connection_string_null)
{
    STRING_HANDLE result;

    // arrange 

    // act 
    result = io_cs_to_STRING(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void_ptr, NULL, (void*)result);
}

// 
// Test io_cs_to_STRING unhappy path 
// 
TEST_FUNCTION(io_cs_to_STRING__neg)
{
    const STRING_HANDLE k_string1_valid = (STRING_HANDLE)0x23423;
    const STRING_HANDLE k_string2_valid = (STRING_HANDLE)0x22343;
    io_cs_t connection_string_valid;
    STRING_HANDLE result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[0] = k_string2_valid;

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
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
    STRICT_EXPECTED_CALL(STRING_delete(k_string1_valid));

    // act 
        UMOCK_C_NEGATIVE_TESTS_ACT();
        result = io_cs_to_STRING(&connection_string_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, result, NULL, k_string1_valid, NULL, NULL, k_string1_valid);
}

// 
// Test io_cs_get_host_name happy path 
// 
TEST_FUNCTION(io_cs_get_host_name__success)
{
    static const char* k_string_c_valid = "some_host.hub";
    const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x23423;
    io_cs_t connection_string_valid;
    const char* result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[io_cs_entry_host_name] = k_string_valid;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn(k_string_c_valid);

    // act 
    result = io_cs_get_host_name(&connection_string_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, k_string_c_valid, result);
}

// 
// Test io_cs_get_host_name passing as connection_string argument an invalid io_cs_t* value 
// 
TEST_FUNCTION(io_cs_get_host_name__arg_connection_string_invalid)
{
    io_cs_t connection_string_valid;
    const char* result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));

    // arrange 

    // act 
    result = io_cs_get_host_name(&connection_string_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test io_cs_get_host_name passing as connection_string argument an invalid io_cs_t* value 
// 
TEST_FUNCTION(io_cs_get_host_name__arg_connection_string_null)
{
    const char* result;

    // arrange 

    // act 
    result = io_cs_get_host_name(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test io_cs_set_host_name happy path 
// 
TEST_FUNCTION(io_cs_set_host_name__success)
{
    static const char* k_string_c_valid = "hub.some-host.de";
    const STRING_HANDLE k_string1_valid = (STRING_HANDLE)0x23423;
    const STRING_HANDLE k_string2_valid = (STRING_HANDLE)0x23423;
    io_cs_t connection_string_valid;
    int32_t result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[io_cs_entry_host_name] = k_string1_valid;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_delete(k_string1_valid));
    STRICT_EXPECTED_CALL(STRING_construct_trim(k_string_c_valid, "/\\ "))
        .SetReturn(k_string2_valid);

    // act 
    result = io_cs_set_host_name(&connection_string_valid, k_string_c_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_cs_set_host_name passing as connection_string argument an invalid io_cs_t* value 
// 
TEST_FUNCTION(io_cs_set_host_name__arg_connection_string_null)
{
    static const char* k_string_c_valid = "hub.somehost.de";
    int32_t result;

    // arrange 

    // act 
    result = io_cs_set_host_name(NULL, k_string_c_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_cs_set_host_name passing as host_name argument an invalid const char* value 
// 
TEST_FUNCTION(io_cs_set_host_name__arg_host_name_null)
{
    io_cs_t connection_string_valid;
    int32_t result;

    // arrange 

    // act 
    result = io_cs_set_host_name(&connection_string_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_cs_set_host_name passing as host_name argument an invalid const char* value 
// 
TEST_FUNCTION(io_cs_set_host_name__arg_host_name_invalid_1)
{
    static const char* k_string_c_invalid = "hub";
    io_cs_t connection_string_valid;
    int32_t result;

    // arrange 

    // act 
    result = io_cs_set_host_name(&connection_string_valid, k_string_c_invalid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_invalid_format, result);
}

// 
// Test io_cs_set_host_name passing as host_name argument an invalid const char* value 
// 
TEST_FUNCTION(io_cs_set_host_name__arg_host_name_invalid_2)
{
    static const char* k_string_c_invalid = "1234.1234";
    io_cs_t connection_string_valid;
    int32_t result;

    // arrange 

    // act 
    result = io_cs_set_host_name(&connection_string_valid, k_string_c_invalid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_invalid_format, result);
}

// 
// Test io_cs_set_host_name unhappy path 
// 
TEST_FUNCTION(io_cs_set_host_name__neg)
{
    static const char* k_string_c_valid = "hub.somehost.de";
    io_cs_t connection_string_valid;
    int32_t result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));

    // arrange 
    STRICT_EXPECTED_CALL(STRING_construct_trim(k_string_c_valid, "/\\ "))
        .SetReturn(NULL);

    // act 
    result = io_cs_set_host_name(&connection_string_valid, k_string_c_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_out_of_memory, result);
}

// 
// Test io_cs_get_hub_name happy path 
// 
TEST_FUNCTION(io_cs_get_hub_name__success_1)
{
    static const char* k_string_c_valid = "hub";
    const STRING_HANDLE k_hostname_valid = (STRING_HANDLE)0x234ff;
    const STRING_HANDLE k_hubname_valid = (STRING_HANDLE)0x13415;
    io_cs_t connection_string_valid;
    const char* result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[io_cs_entry_host_name] = k_hostname_valid;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_hostname_valid))
        .SetReturn("hub.somehost.de");
    STRICT_EXPECTED_CALL(STRING_safe_construct_n(IGNORED_PTR_ARG, 3))
        .IgnoreArgument(1)
        .SetReturn(k_hubname_valid);
    STRICT_EXPECTED_CALL(STRING_c_str(k_hubname_valid))
        .SetReturn(k_string_c_valid);

    // act 
    result = io_cs_get_hub_name(&connection_string_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, k_string_c_valid, result);
    ASSERT_ARE_EQUAL(void_ptr, k_hubname_valid, connection_string_valid.entries[io_cs_entry_hub_name]);
}

// 
// Test io_cs_get_hub_name happy path 
// 
TEST_FUNCTION(io_cs_get_hub_name__success_2)
{
    static const char* k_string_c_valid = "hub2";
    const STRING_HANDLE k_hubname_valid = (STRING_HANDLE)0x13415;
    io_cs_t connection_string_valid;
    const char* result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[io_cs_entry_hub_name] = k_hubname_valid;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_hubname_valid))
        .SetReturn(k_string_c_valid);

    // act 
    result = io_cs_get_hub_name(&connection_string_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, k_string_c_valid, result);
}

// 
// Test io_cs_get_hub_name passing as connection_string argument an invalid io_cs_t* value 
// 
TEST_FUNCTION(io_cs_get_hub_name__arg_connection_string_invalid)
{
    const STRING_HANDLE k_hostname_invalid = (STRING_HANDLE)0x234ff;
    io_cs_t connection_string_valid;
    const char* result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[io_cs_entry_host_name] = k_hostname_invalid;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_hostname_invalid))
        .SetReturn("test");

    // act 
    result = io_cs_get_hub_name(&connection_string_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test io_cs_get_hub_name passing as connection_string argument an invalid io_cs_t* value 
// 
TEST_FUNCTION(io_cs_get_hub_name__arg_connection_string_null)
{
    const char* result;

    // arrange 

    // act 
    result = io_cs_get_hub_name(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test io_cs_get_hub_name unhappy path 
// 
TEST_FUNCTION(io_cs_get_hub_name__neg)
{
    const STRING_HANDLE k_hostname_valid = (STRING_HANDLE)0x234ff;
    io_cs_t connection_string_valid;
    const char* result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[io_cs_entry_host_name] = k_hostname_valid;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_hostname_valid))
        .SetReturn("huboom.lauter.com");
    STRICT_EXPECTED_CALL(STRING_safe_construct_n(IGNORED_PTR_ARG, 6))
        .IgnoreArgument(1)
        .SetReturn(NULL);

    // act 
    result = io_cs_get_hub_name(&connection_string_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
    ASSERT_ARE_EQUAL(void_ptr, NULL, connection_string_valid.entries[io_cs_entry_hub_name]);
}

// 
// Test io_cs_get_device_id happy path 
// 
TEST_FUNCTION(io_cs_get_device_id__success)
{
    static const char* k_string_c_valid = "device0";
    const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x23423;
    io_cs_t connection_string_valid;
    const char* result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[io_cs_entry_device_id] = k_string_valid;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn(k_string_c_valid);

    // act 
    result = io_cs_get_device_id(&connection_string_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, k_string_c_valid, result);
}

// 
// Test io_cs_get_device_id passing as connection_string argument an invalid io_cs_t* value 
// 
TEST_FUNCTION(io_cs_get_device_id__arg_connection_string_invalid)
{
    io_cs_t connection_string_valid;
    const char* result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));

    // arrange 

    // act 
    result = io_cs_get_device_id(&connection_string_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test io_cs_get_device_id passing as connection_string argument an invalid io_cs_t* value 
// 
TEST_FUNCTION(io_cs_get_device_id__arg_connection_string_null)
{
    const char* result;

    // arrange 

    // act 
    result = io_cs_get_device_id(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test io_cs_set_device_id happy path 
// 
TEST_FUNCTION(io_cs_set_device_id__success)
{
    static const char* k_string_c_valid = "(device1)";
    const STRING_HANDLE k_string1_valid = (STRING_HANDLE)0x23423;
    const STRING_HANDLE k_string2_valid = (STRING_HANDLE)0x23423;
    io_cs_t connection_string_valid;
    int32_t result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[io_cs_entry_device_id] = k_string1_valid;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_delete(k_string1_valid));
    STRICT_EXPECTED_CALL(STRING_construct(k_string_c_valid))
        .SetReturn(k_string2_valid);

    // act 
    result = io_cs_set_device_id(&connection_string_valid, k_string_c_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_cs_set_device_id happy path
// 
TEST_FUNCTION(io_cs_set_device_id__success_null)
{
    const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x1;
    io_cs_t connection_string_valid;
    int32_t result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[io_cs_entry_device_id] = k_string_valid;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_delete(k_string_valid));

    // act 
    result = io_cs_set_device_id(&connection_string_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(void_ptr, NULL, connection_string_valid.entries[io_cs_entry_device_id]);
}

// 
// Test io_cs_set_device_id passing as connection_string argument an invalid io_cs_t* value 
// 
TEST_FUNCTION(io_cs_set_device_id__arg_connection_string_null)
{
    static const char* k_string_c_valid = "d3";
    int32_t result;

    // arrange 

    // act 
    result = io_cs_set_device_id(NULL, k_string_c_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_cs_set_device_id passing as device_id argument an invalid const char* value 
// 
TEST_FUNCTION(io_cs_set_device_id__arg_device_id_invalid)
{
    static const char* k_string_c_invalid = "test§r";
    const STRING_HANDLE k_string1_valid = (STRING_HANDLE)0x23423;
    io_cs_t connection_string_valid;
    int32_t result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[io_cs_entry_device_id] = k_string1_valid;

    // arrange 

    // act 
    result = io_cs_set_device_id(&connection_string_valid, k_string_c_invalid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_invalid_format, result);
}

// 
// Test io_cs_set_device_id unhappy path 
// 
TEST_FUNCTION(io_cs_set_device_id__neg)
{
    static const char* k_string_c_valid = "device1";
    io_cs_t connection_string_valid;
    int32_t result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));

    // arrange 
    STRICT_EXPECTED_CALL(STRING_construct(k_string_c_valid))
        .SetReturn(NULL);

    // act 
    result = io_cs_set_device_id(&connection_string_valid, k_string_c_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_out_of_memory, result);
}

// 
// Test io_cs_get_endpoint happy path 
// 
TEST_FUNCTION(io_cs_get_endpoint__success)
{
    static const char* k_string_c_valid = "sb://test.az.com";
    const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x2af3;
    io_cs_t connection_string_valid;
    const char* result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[io_cs_entry_endpoint] = k_string_valid;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn(k_string_c_valid);

    // act 
    result = io_cs_get_endpoint(&connection_string_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, k_string_c_valid, result);
}

// 
// Test io_cs_get_endpoint passing as connection_string argument an invalid io_cs_t* value 
// 
TEST_FUNCTION(io_cs_get_endpoint__arg_connection_string_invalid)
{
    io_cs_t connection_string_valid;
    const char* result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));

    // arrange 

    // act 
    result = io_cs_get_endpoint(&connection_string_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test io_cs_get_endpoint passing as connection_string argument an invalid io_cs_t* value 
// 
TEST_FUNCTION(io_cs_get_endpoint__arg_connection_string_null)
{
    const char* result;

    // arrange 

    // act 
    result = io_cs_get_endpoint(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test io_cs_set_endpoint happy path 
// 
TEST_FUNCTION(io_cs_set_endpoint__success)
{
    static const char* k_string_c_valid = "sb://test.test.com/";
    static const char* k_endpoint = "test.test.com";
    static size_t k_endpoint_len = 14;
    const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x23423;
    io_cs_t connection_string_valid;
    int32_t result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[io_cs_entry_endpoint] = k_string_valid;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_delete(k_string_valid));
    STRICT_EXPECTED_CALL(string_trim_scheme(k_string_c_valid, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_name_len(&k_endpoint_len, sizeof(k_endpoint_len))
        .SetReturn(k_endpoint);
    STRICT_EXPECTED_CALL(string_trim_back_len(k_endpoint, k_endpoint_len, "/\\ "))
        .SetReturn(k_endpoint_len-1);
    STRICT_EXPECTED_CALL(STRING_safe_construct_n(k_endpoint, k_endpoint_len-1))
        .SetReturn((STRING_HANDLE)1);

    // act 
    result = io_cs_set_endpoint(&connection_string_valid, k_string_c_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_cs_set_endpoint happy path
// 
TEST_FUNCTION(io_cs_set_endpoint__success_null)
{
    const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x1;
    io_cs_t connection_string_valid;
    int32_t result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[io_cs_entry_endpoint] = k_string_valid;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_delete(k_string_valid));

    // act 
    result = io_cs_set_endpoint(&connection_string_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(void_ptr, NULL, connection_string_valid.entries[io_cs_entry_endpoint]);
}

// 
// Test io_cs_set_endpoint passing as connection_string argument an invalid io_cs_t* value 
// 
TEST_FUNCTION(io_cs_set_endpoint__arg_connection_string_null)
{
    static const char* k_string_c_valid = "sb://test.test.com";
    int32_t result;

    // arrange 

    // act 
    result = io_cs_set_endpoint(NULL, k_string_c_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_cs_set_endpoint unhappy path 
// 
TEST_FUNCTION(io_cs_set_endpoint__neg)
{
    static const char* k_string_c_valid = "sb://test.test.com";
    static const char* k_endpoint = "test.test.com";
    static size_t k_endpoint_len = 14;
    io_cs_t connection_string_valid;
    int32_t result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));

    // arrange 
    STRICT_EXPECTED_CALL(string_trim_scheme(k_string_c_valid, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_name_len(&k_endpoint_len, sizeof(k_endpoint_len))
        .SetReturn(k_endpoint);
    STRICT_EXPECTED_CALL(string_trim_back_len(k_endpoint, k_endpoint_len, "/\\ "))
        .SetReturn(k_endpoint_len - 1);
    STRICT_EXPECTED_CALL(STRING_safe_construct_n(k_endpoint, k_endpoint_len - 1))
        .SetReturn(NULL);

    // act 
    result = io_cs_set_endpoint(&connection_string_valid, k_string_c_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_out_of_memory, result);
}

// 
// Test io_cs_get_endpoint_name happy path 
// 
TEST_FUNCTION(io_cs_get_endpoint_name__success_1)
{
    static const char* k_string_c_valid = "az-ep-name";
    const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x2ab3;
    io_cs_t connection_string_valid;
    const char* result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[io_cs_entry_endpoint_name] = k_string_valid;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn(k_string_c_valid);

    // act 
    result = io_cs_get_endpoint_name(&connection_string_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, k_string_c_valid, result);
}

// 
// Test io_cs_get_endpoint_name happy path 
// 
TEST_FUNCTION(io_cs_get_endpoint_name__success_2)
{
    static const char* k_string_c_valid = "hub";
    const STRING_HANDLE k_hostname_valid = (STRING_HANDLE)0x234ff;
    const STRING_HANDLE k_hubname_valid = (STRING_HANDLE)0x13415;
    io_cs_t connection_string_valid;
    const char* result;

    // arrange 
    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[io_cs_entry_host_name] = k_hostname_valid;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_hostname_valid))
        .SetReturn("hub.somehost.de");
    STRICT_EXPECTED_CALL(STRING_safe_construct_n(IGNORED_PTR_ARG, 3))
        .IgnoreArgument(1)
        .SetReturn(k_hubname_valid);
    STRICT_EXPECTED_CALL(STRING_c_str(k_hubname_valid))
        .SetReturn(k_string_c_valid);

    // act 
    result = io_cs_get_endpoint_name(&connection_string_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, k_string_c_valid, result);
    ASSERT_ARE_EQUAL(void_ptr, k_hubname_valid, connection_string_valid.entries[io_cs_entry_hub_name]);
}

// 
// Test io_cs_get_endpoint_name passing as connection_string argument an invalid io_cs_t* value 
// 
TEST_FUNCTION(io_cs_get_endpoint_name__arg_connection_string_null)
{
    const char* result;

    // arrange 

    // act 
    result = io_cs_get_endpoint_name(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test io_cs_get_endpoint_name unhappy path 
// 
TEST_FUNCTION(io_cs_get_endpoint_name__neg_2)
{
    static const char* k_string_c_valid = "hub";
    const STRING_HANDLE k_hostname_valid = (STRING_HANDLE)0x234ff;
    io_cs_t connection_string_valid;
    const char* result;

    // arrange 
    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[io_cs_entry_host_name] = k_hostname_valid;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_hostname_valid))
        .SetReturn("hub.somehost.de");
    STRICT_EXPECTED_CALL(STRING_safe_construct_n(IGNORED_PTR_ARG, 3))
        .IgnoreArgument(1)
        .SetReturn(NULL);

    // act 
    result = io_cs_get_endpoint_name(&connection_string_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_NULL(result);
}

// 
// Test io_cs_set_endpoint_name happy path 
// 
TEST_FUNCTION(io_cs_set_endpoint_name__success)
{
    static const char* k_string_c_valid = "az-ep-name";
    const STRING_HANDLE k_string1_valid = (STRING_HANDLE)0x23423;
    const STRING_HANDLE k_string2_valid = (STRING_HANDLE)0x23423;
    io_cs_t connection_string_valid;
    int32_t result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[io_cs_entry_endpoint_name] = k_string1_valid;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_delete(k_string1_valid));
    STRICT_EXPECTED_CALL(STRING_construct(k_string_c_valid))
        .SetReturn(k_string2_valid);

    // act 
    result = io_cs_set_endpoint_name(&connection_string_valid, k_string_c_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_cs_set_endpoint_name happy path
// 
TEST_FUNCTION(io_cs_set_endpoint_name__success_null)
{
    const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x1;
    io_cs_t connection_string_valid;
    int32_t result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[io_cs_entry_endpoint_name] = k_string_valid;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_delete(k_string_valid));

    // act 
    result = io_cs_set_endpoint_name(&connection_string_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(void_ptr, NULL, connection_string_valid.entries[io_cs_entry_endpoint_name]);
}

// 
// Test io_cs_set_endpoint_name passing as connection_string argument an invalid io_cs_t* value 
// 
TEST_FUNCTION(io_cs_set_endpoint_name__arg_connection_string_null)
{
    static const char* k_string_c_valid = "az-ep-name";
    int32_t result;

    // arrange 

    // act 
    result = io_cs_set_endpoint_name(NULL, k_string_c_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_cs_set_endpoint_name unhappy path 
// 
TEST_FUNCTION(io_cs_set_endpoint_name__neg)
{
    static const char* k_string_c_valid = "az-ep-name";
    io_cs_t connection_string_valid;
    int32_t result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));

    // arrange 
    STRICT_EXPECTED_CALL(STRING_construct(k_string_c_valid))
        .SetReturn(NULL);

    // act 
    result = io_cs_set_endpoint_name(&connection_string_valid, k_string_c_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_out_of_memory, result);
}

// 
// Test io_cs_get_consumer_group happy path 
// 
TEST_FUNCTION(io_cs_get_consumer_group__success)
{
    static const char* k_string_c_valid = "$default";
    const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x23423;
    io_cs_t connection_string_valid;
    const char* result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[io_cs_entry_consumer_group] = k_string_valid;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn(k_string_c_valid);

    // act 
    result = io_cs_get_consumer_group(&connection_string_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, k_string_c_valid, result);
}

// 
// Test io_cs_get_consumer_group passing as connection_string argument an invalid io_cs_t* value 
// 
TEST_FUNCTION(io_cs_get_consumer_group__arg_connection_string_invalid)
{
    io_cs_t connection_string_valid;
    const char* result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));

    // arrange 

    // act 
    result = io_cs_get_consumer_group(&connection_string_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test io_cs_get_consumer_group passing as connection_string argument an invalid io_cs_t* value 
// 
TEST_FUNCTION(io_cs_get_consumer_group__arg_connection_string_null)
{
    const char* result;

    // arrange 

    // act 
    result = io_cs_get_consumer_group(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test io_cs_set_consumer_group happy path 
// 
TEST_FUNCTION(io_cs_set_consumer_group__success)
{
    static const char* k_string_c_valid = "default$";
    const STRING_HANDLE k_string1_valid = (STRING_HANDLE)0x23423;
    const STRING_HANDLE k_string2_valid = (STRING_HANDLE)0x23423;
    io_cs_t connection_string_valid;
    int32_t result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[io_cs_entry_consumer_group] = k_string1_valid;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_delete(k_string1_valid));
    STRICT_EXPECTED_CALL(STRING_construct(k_string_c_valid))
        .SetReturn(k_string2_valid);

    // act 
    result = io_cs_set_consumer_group(&connection_string_valid, k_string_c_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_cs_set_consumer_group happy path
// 
TEST_FUNCTION(io_cs_set_consumer_group__success_null)
{
    const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x1;
    io_cs_t connection_string_valid;
    int32_t result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[io_cs_entry_consumer_group] = k_string_valid;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_delete(k_string_valid));

    // act 
    result = io_cs_set_consumer_group(&connection_string_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(void_ptr, NULL, connection_string_valid.entries[io_cs_entry_consumer_group]);
}

// 
// Test io_cs_set_consumer_group passing as connection_string argument an invalid io_cs_t* value 
// 
TEST_FUNCTION(io_cs_set_consumer_group__arg_connection_string_null)
{
    static const char* k_string_c_valid = "default$";
    int32_t result;

    // arrange 

    // act 
    result = io_cs_set_consumer_group(NULL, k_string_c_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_cs_set_consumer_group unhappy path 
// 
TEST_FUNCTION(io_cs_set_consumer_group__neg)
{
    static const char* k_string_c_valid = "default$";
    io_cs_t connection_string_valid;
    int32_t result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));

    // arrange 
    STRICT_EXPECTED_CALL(STRING_construct(k_string_c_valid))
        .SetReturn(NULL);

    // act 
    result = io_cs_set_consumer_group(&connection_string_valid, k_string_c_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_out_of_memory, result);
}

// 
// Test io_cs_get_partition_count happy path 
// 
TEST_FUNCTION(io_cs_get_partition_count__success)
{
    static const char* k_string_c_valid = "5";
    const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x23423;
    io_cs_t connection_string_valid;
    int32_t result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[io_cs_entry_partition_count] = k_string_valid;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn(k_string_c_valid);

    // act 
    result = io_cs_get_partition_count(&connection_string_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, 5, result);
}

// 
// Test io_cs_get_partition_count passing as connection_string argument an invalid io_cs_t* value 
// 
TEST_FUNCTION(io_cs_get_partition_count__arg_connection_string_invalid)
{
    io_cs_t connection_string_valid;
    int32_t result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));

    // arrange 

    // act 
    result = io_cs_get_partition_count(&connection_string_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, 0, result);
}

// 
// Test io_cs_get_partition_count passing as connection_string argument an invalid io_cs_t* value 
// 
TEST_FUNCTION(io_cs_get_partition_count__arg_connection_string_null)
{
    int32_t result;

    // arrange 

    // act 
    result = io_cs_get_partition_count(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, 0, result);
}

// 
// Test io_cs_set_partition_count happy path 
// 
TEST_FUNCTION(io_cs_set_partition_count__success)
{
    static const char* k_string_c_valid = "32";
    const STRING_HANDLE k_string1_valid = (STRING_HANDLE)0x23423;
    const STRING_HANDLE k_string2_valid = (STRING_HANDLE)0x23423;
    io_cs_t connection_string_valid;
    int32_t result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[io_cs_entry_partition_count] = k_string1_valid;

    // arrange 
    STRICT_EXPECTED_CALL(string_from_int(32, 10, IGNORED_PTR_ARG, 32))
        .CopyOutArgumentBuffer_string(k_string_c_valid, strlen(k_string_c_valid)+1)
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(STRING_delete(k_string1_valid));
    STRICT_EXPECTED_CALL(STRING_construct(k_string_c_valid))
        .SetReturn(k_string2_valid);

    // act 
    result = io_cs_set_partition_count(&connection_string_valid, 32);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_cs_set_partition_count happy path
// 
TEST_FUNCTION(io_cs_set_partition_count__success_null)
{
    const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x1;
    io_cs_t connection_string_valid;
    int32_t result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[io_cs_entry_partition_count] = k_string_valid;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_delete(k_string_valid));

    // act 
    result = io_cs_set_partition_count(&connection_string_valid, 0);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(void_ptr, NULL, connection_string_valid.entries[io_cs_entry_partition_count]);
}

// 
// Test io_cs_set_partition_count passing as connection_string argument an invalid io_cs_t* value 
// 
TEST_FUNCTION(io_cs_set_partition_count__arg_connection_string_null)
{
    static const char* k_string_c_valid = "16";
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(string_from_int(16, 10, IGNORED_PTR_ARG, 32))
        .CopyOutArgumentBuffer_string(k_string_c_valid, strlen(k_string_c_valid) + 1)
        .SetReturn(er_ok);

    // act 
    result = io_cs_set_partition_count(NULL, 16);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_cs_set_partition_count unhappy path 
// 
TEST_FUNCTION(io_cs_set_partition_count__neg)
{
    static const char* k_string_c_valid = "6";
    io_cs_t connection_string_valid;
    int32_t result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));

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
    result = io_cs_set_partition_count(&connection_string_valid, 6);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_invalid_format, er_out_of_memory);
}

// 
// Test io_cs_get_shared_access_key_name happy path 
// 
TEST_FUNCTION(io_cs_get_shared_access_key_name__success)
{
    static const char* k_string_c_valid = "iothubowner";
    const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x23423;
    io_cs_t connection_string_valid;
    const char* result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[io_cs_entry_shared_access_key_name] = k_string_valid;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn(k_string_c_valid);

    // act 
    result = io_cs_get_shared_access_key_name(&connection_string_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, k_string_c_valid, result);
}

// 
// Test io_cs_get_shared_access_key_name passing as connection_string argument an invalid io_cs_t* value 
// 
TEST_FUNCTION(io_cs_get_shared_access_key_name__arg_connection_string_invalid)
{
    io_cs_t connection_string_valid;
    const char* result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));

    // arrange 

    // act 
    result = io_cs_get_shared_access_key_name(&connection_string_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test io_cs_get_shared_access_key_name passing as connection_string argument an invalid io_cs_t* value 
// 
TEST_FUNCTION(io_cs_get_shared_access_key_name__arg_connection_string_null)
{
    const char* result;

    // arrange 

    // act 
    result = io_cs_get_shared_access_key_name(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test io_cs_get_shared_access_token happy path 
// 
TEST_FUNCTION(io_cs_get_shared_access_token__success)
{
    static const char* k_string_c_valid = "1861054187029348710239847019238471203";
    const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x4643455;
    io_cs_t connection_string_valid;
    const char* result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[io_cs_entry_shared_access_token] = k_string_valid;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn(k_string_c_valid);

    // act 
    result = io_cs_get_shared_access_token(&connection_string_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, k_string_c_valid, result);
}

// 
// Test io_cs_get_shared_access_token passing as connection_string argument an invalid io_cs_t* value 
// 
TEST_FUNCTION(io_cs_get_shared_access_tokeny__arg_connection_string_invalid)
{
    io_cs_t connection_string_valid;
    const char* result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));

    // arrange 

    // act 
    result = io_cs_get_shared_access_token(&connection_string_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test io_cs_get_shared_access_token passing as connection_string argument an invalid io_cs_t* value 
// 
TEST_FUNCTION(io_cs_get_shared_access_token__arg_connection_string_null)
{
    const char* result;

    // arrange 

    // act 
    result = io_cs_get_shared_access_token(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test io_cs_get_entity happy path 
// 
TEST_FUNCTION(io_cs_get_entity__success)
{
    static const char* k_string_c_valid = "queue1";
    const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x2af3;
    io_cs_t connection_string_valid;
    const char* result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[io_cs_entry_entity] = k_string_valid;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn(k_string_c_valid);

    // act 
    result = io_cs_get_entity(&connection_string_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, k_string_c_valid, result);
}

// 
// Test io_cs_get_entity passing as connection_string argument an invalid io_cs_t* value 
// 
TEST_FUNCTION(io_cs_get_entity__arg_connection_string_invalid)
{
    io_cs_t connection_string_valid;
    const char* result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));

    // arrange 

    // act 
    result = io_cs_get_entity(&connection_string_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test io_cs_get_entity passing as connection_string argument an invalid io_cs_t* value 
// 
TEST_FUNCTION(io_cs_get_entity__arg_connection_string_null)
{
    const char* result;

    // arrange 

    // act 
    result = io_cs_get_entity(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test io_cs_set_entity happy path 
// 
TEST_FUNCTION(io_cs_set_entity__success)
{
    static const char* k_string_c_valid = "queue3";
    const STRING_HANDLE k_string1_valid = (STRING_HANDLE)0x23423;
    const STRING_HANDLE k_string2_valid = (STRING_HANDLE)0x23423;
    io_cs_t connection_string_valid;
    int32_t result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[io_cs_entry_entity] = k_string1_valid;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_delete(k_string1_valid));
    STRICT_EXPECTED_CALL(STRING_construct(k_string_c_valid))
        .SetReturn(k_string2_valid);

    // act 
    result = io_cs_set_entity(&connection_string_valid, k_string_c_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_cs_set_entity happy path
// 
TEST_FUNCTION(io_cs_set_entity__success_null)
{
    const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x1;
    io_cs_t connection_string_valid;
    int32_t result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[io_cs_entry_entity] = k_string_valid;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_delete(k_string_valid));

    // act 
    result = io_cs_set_entity(&connection_string_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(void_ptr, NULL, connection_string_valid.entries[io_cs_entry_entity]);
}

// 
// Test io_cs_set_entity passing as connection_string argument an invalid io_cs_t* value 
// 
TEST_FUNCTION(io_cs_set_entity__arg_connection_string_null)
{
    static const char* k_string_c_valid = "queue3";
    int32_t result;

    // arrange 

    // act 
    result = io_cs_set_entity(NULL, k_string_c_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_cs_set_entity unhappy path 
// 
TEST_FUNCTION(io_cs_set_entity__neg)
{
    static const char* k_string_c_valid = "queue54";
    io_cs_t connection_string_valid;
    int32_t result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));

    // arrange 
    STRICT_EXPECTED_CALL(STRING_construct(k_string_c_valid))
        .SetReturn(NULL);

    // act 
    result = io_cs_set_entity(&connection_string_valid, k_string_c_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_out_of_memory, result);
}

// 
// Test io_cs_free happy path 
// 
TEST_FUNCTION(io_cs_free__success)
{
    io_cs_t connection_string_valid;

    for (size_t i = 0; i < _countof(connection_string_valid.entries); i++)
        connection_string_valid.entries[i] = (STRING_HANDLE)(0x8000 | i);

    // arrange 
    for (size_t i = 0; i < _countof(connection_string_valid.entries); i++)
    {
        STRICT_EXPECTED_CALL(STRING_delete((STRING_HANDLE)(0x8000 | i)));
    }
    STRICT_EXPECTED_CALL(h_free(&connection_string_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    io_cs_free(&connection_string_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test io_cs_free passing as connection_string argument an invalid io_cs_t* value 
// 
TEST_FUNCTION(io_cs_free__arg_connection_string_null)
{
    // arrange 

    // act 
    io_cs_free(NULL);

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test io_encode_cs happy path 
// 
TEST_FUNCTION(io_encode_cs__success)
{
    static const STRING_HANDLE k_string1_valid = (STRING_HANDLE)0x654;
    static const STRING_HANDLE k_string2_valid = (STRING_HANDLE)0x345;
    static io_codec_ctx_t* k_ctx_valid = (io_codec_ctx_t*)0x1234124;
    io_cs_t connection_string_valid;
    io_cs_t* connection_string_ptr_valid = &connection_string_valid;
    int32_t result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[0] = k_string2_valid;

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
    result = io_encode_cs(k_ctx_valid, &connection_string_ptr_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_encode_cs unhappy path 
// 
TEST_FUNCTION(io_encode_cs__neg)
{
    static const STRING_HANDLE k_string1_valid = (STRING_HANDLE)0x654;
    static const STRING_HANDLE k_string2_valid = (STRING_HANDLE)0x345;
    static io_codec_ctx_t* k_ctx_valid = (io_codec_ctx_t*)0x1234124;
    io_cs_t connection_string_valid;
    io_cs_t* connection_string_ptr_valid = &connection_string_valid;
    int32_t result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[0] = k_string2_valid;

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
    result = io_encode_cs(k_ctx_valid, &connection_string_ptr_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, 
        er_writing, er_out_of_memory, er_ok, er_out_of_memory, er_out_of_memory, 
        er_writing, er_ok,            er_invalid_format);
}

// 
// Test io_decode_cs happy path 
// 
TEST_FUNCTION(io_decode_cs__success)
{
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x654;
    static const STRING_HANDLE k_hostname_string_valid = (STRING_HANDLE)0x345;
    static const STRING_HANDLE k_device_id_string_valid = (STRING_HANDLE)0x222;
    static const char* k_component = "HostName=cannotfail.ever.com;DeviceId=andever";
    static io_codec_ctx_t* k_ctx_valid = (io_codec_ctx_t*)0x1234124;
    io_cs_t* connection_string_ptr_valid;
    int32_t result;

    REGISTER_GLOBAL_MOCK_HOOK(string_key_value_parser, string_key_value_parser__hook_connection_string_with_hostname_and_device_id);
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
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_cs_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
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
    result = io_decode_cs(k_ctx_valid, &connection_string_ptr_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_decode_cs unhappy path 
// 
TEST_FUNCTION(io_decode_cs__neg)
{
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x654;
    static const STRING_HANDLE k_hostname_string_valid = (STRING_HANDLE)0x345;
    static const STRING_HANDLE k_device_id_string_valid = (STRING_HANDLE)0x222;
    static const char* k_component = "HostName=cannotfail.ever.com;DeviceId=andever";
    static io_codec_ctx_t* k_ctx_valid = (io_codec_ctx_t*)0x1234124;
    io_cs_t* connection_string_ptr_valid;
    int32_t result;

    REGISTER_GLOBAL_MOCK_HOOK(string_key_value_parser, string_key_value_parser__hook_connection_string_with_hostname_and_device_id);
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
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_cs_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
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
    result = io_decode_cs(k_ctx_valid, &connection_string_ptr_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, 
        er_invalid_format, er_invalid_format, er_ok, er_out_of_memory, er_invalid_format);
}

// 
// Test io_cs_remove_keys happy path 
// 
TEST_FUNCTION(io_cs_remove_keys__success_0)
{
    static STRING_HANDLE k_string_valid = (STRING_HANDLE)0x2342;
    io_cs_t connection_string_valid;
    io_cs_t* connection_string_ptr_valid = &connection_string_valid;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[io_cs_entry_shared_access_key_handle] = k_string_valid;

    // arrange 
    STRICT_EXPECTED_CALL(pal_cred_remove(k_string_valid));
    STRICT_EXPECTED_CALL(STRING_delete(k_string_valid));

    // act 
    io_cs_remove_keys(connection_string_ptr_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_NULL(connection_string_valid.entries[io_cs_entry_shared_access_key_handle]);
}

// 
// Test io_cs_remove_keys happy path 
// 
TEST_FUNCTION(io_cs_remove_keys__success_1)
{
    io_cs_t connection_string_valid;
    io_cs_t* connection_string_ptr_valid = &connection_string_valid;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));

    // arrange 

    // act 
    io_cs_remove_keys(connection_string_ptr_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test io_cs_remove_keys passing as connection_string argument an invalid io_cs_t* value 
// 
TEST_FUNCTION(io_cs_remove_keys__arg_connection_string_invalid)
{
    // arrange 

    // act 
    io_cs_remove_keys(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test io_cs_create_token_provider happy path 
// 
TEST_FUNCTION(io_cs_create_token_provider__success_0)
{
    static const io_cs_t* k_cs_valid;
    static STRING_HANDLE k_string_valid = (STRING_HANDLE)0x2342;
    static const io_token_provider_t* k_provider_valid = (io_token_provider_t*)0x3243;
    static const char* k_string_value_valid = "test";
    io_cs_t connection_string_valid;
    io_cs_t* connection_string_ptr_valid = &connection_string_valid;
    io_token_provider_t* provider_valid;
    int32_t result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[io_cs_entry_shared_access_token] = k_string_valid;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn(k_string_value_valid);
    STRICT_EXPECTED_CALL(io_passthru_token_provider_create(k_string_value_valid, &provider_valid))
        .CopyOutArgumentBuffer_provider(&k_provider_valid, sizeof(k_provider_valid))
        .SetReturn(er_ok);

    // act 
    result = io_cs_create_token_provider(connection_string_ptr_valid, &provider_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_cs_create_token_provider happy path 
// 
TEST_FUNCTION(io_cs_create_token_provider__success_1)
{
    static STRING_HANDLE k_string_valid = (STRING_HANDLE)0x2342;
    static const io_token_provider_t* k_provider_valid = (io_token_provider_t*)0x3243;
    static const char* k_string_value_valid = "test";
    io_cs_t connection_string_valid;
    io_cs_t* connection_string_ptr_valid = &connection_string_valid;
    io_token_provider_t* provider_valid;
    int32_t result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[io_cs_entry_shared_access_token] = k_string_valid;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn(k_string_value_valid);
    STRICT_EXPECTED_CALL(io_passthru_token_provider_create(k_string_value_valid, &provider_valid))
        .CopyOutArgumentBuffer_provider(&k_provider_valid, sizeof(k_provider_valid))
        .SetReturn(er_ok);

    // act 
    result = io_cs_create_token_provider(connection_string_ptr_valid, &provider_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_cs_create_token_provider passing as cs argument an invalid io_cs_t* value 
// 
TEST_FUNCTION(io_cs_create_token_provider__arg_cs_invalid)
{
    io_token_provider_t* provider_valid;
    int32_t result;

    // arrange 

    // act 
    result = io_cs_create_token_provider(NULL, &provider_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_cs_create_token_provider passing as provider argument an invalid io_token_provider_t** value 
// 
TEST_FUNCTION(io_cs_create_token_provider__arg_provider_invalid)
{
    static STRING_HANDLE k_string_valid = (STRING_HANDLE)0x2342;
    static const char* k_string_value_valid = "test";
    io_cs_t connection_string_valid;
    io_cs_t* connection_string_ptr_valid = &connection_string_valid;
    io_token_provider_t* provider_valid;
    int32_t result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[io_cs_entry_shared_access_token] = k_string_valid;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn(k_string_value_valid);
    STRICT_EXPECTED_CALL(io_passthru_token_provider_create(k_string_value_valid, &provider_valid))
        .SetReturn(er_fault);

    // act 
    result = io_cs_create_token_provider(connection_string_ptr_valid, &provider_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_cs_create_token_provider unhappy path 
// 
TEST_FUNCTION(io_cs_create_token_provider__neg_0)
{
    static STRING_HANDLE k_string_valid = (STRING_HANDLE)0x2342;
    static const io_token_provider_t* k_provider_valid = (io_token_provider_t*)0x3243;
    static const char* k_string_value_valid = "test";
    io_cs_t connection_string_valid;
    io_cs_t* connection_string_ptr_valid = &connection_string_valid;
    io_token_provider_t* provider_valid;
    int32_t result;

    memset(&connection_string_valid, 0, sizeof(connection_string_valid));
    connection_string_valid.entries[io_cs_entry_shared_access_token] = k_string_valid;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn(k_string_value_valid);
    STRICT_EXPECTED_CALL(io_passthru_token_provider_create(k_string_value_valid, &provider_valid))
        .SetReturn(er_out_of_memory);

    // act 
    result = io_cs_create_token_provider(connection_string_ptr_valid, &provider_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_out_of_memory, result);
}


//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

