// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST io_ref
#include "util_ut.h"

//
// 1. Required mocks
//
#include "prx_types.h"
#include "azure_c_shared_utility/strings.h"
#include "io_codec.h"

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
REGISTER_UMOCK_ALIAS_TYPE(uint8_t*, void*)
REGISTER_UMOCK_ALIAS_TYPE(pal_uuid_t, void*)
REGISTER_UMOCK_ALIAS_TYPE(STRING_HANDLE, void*)
REGISTER_UMOCK_ALIAS_TYPE(io_codec_id_t, int)
END_DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
DECLARE_TEST_SETUP()


// 
// Test io_ref_new happy path 
// 
TEST_FUNCTION(io_ref_new__success)
{
    io_ref_t ref_valid;
    int32_t result;

    ref_valid.un.u64[0] = 11ULL;
    ref_valid.un.u64[1] = 22ULL;

    // arrange 
    STRICT_EXPECTED_CALL(pal_rand_fill(ref_valid.un.u8, 16))
        .SetReturn(er_ok);

    // act 
    result = io_ref_new(&ref_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_ref_new passing as address argument an invalid io_ref_t* value 
// 
TEST_FUNCTION(io_ref_new__arg_ref_invalid)
{
    int32_t result;

    // arrange 

    // act 
    result = io_ref_new(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_ref_new unhappy path 
// 
TEST_FUNCTION(io_ref_new__neg)
{
    io_ref_t ref_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(pal_rand_fill(ref_valid.un.u8, 16))
        .SetReturn(er_unknown);

    // act 
    result = io_ref_new(&ref_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_unknown, result);
}

// 
// Test io_ref_clear happy path 
// 
TEST_FUNCTION(io_ref_clear__success)
{
    io_ref_t ref_valid;

    ref_valid.un.u64[0] = 11ULL;
    ref_valid.un.u64[1] = 22ULL;

    // arrange 

    // act 
    io_ref_clear(&ref_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_TRUE(0ULL == ref_valid.un.u64[0]);
    ASSERT_IS_TRUE(0ULL == ref_valid.un.u64[1]);
}

// 
// Test io_ref_clear passing as address argument an invalid io_ref_t* value 
// 
TEST_FUNCTION(io_ref_clear__arg_ref_invalid)
{
    // arrange 

    // act 
    io_ref_clear(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test io_ref_from_string happy path 
// 
TEST_FUNCTION(io_ref_from_string__success_1)
{
    static const char* k_string_valid = "some_uuid_without_braces";
    io_ref_t ref_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(string_to_uuid(k_string_valid, ref_valid.un.u8))
        .SetReturn(er_ok);

    // act 
    result = io_ref_from_string(k_string_valid, &ref_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_ref_from_string happy path 
// 
TEST_FUNCTION(io_ref_from_string__success_2)
{
    static const char* k_string_valid = "{3231F694-589F-48C2-944E-49E36BAC2056}";
    io_ref_t ref_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(string_trim_back(IGNORED_PTR_ARG, "}"))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(string_to_uuid(IGNORED_PTR_ARG, ref_valid.un.u8))
        .IgnoreArgument(1)
        .SetReturn(er_ok);

    // act 
    result = io_ref_from_string(k_string_valid, &ref_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_ref_from_string happy path 
// 
TEST_FUNCTION(io_ref_from_string__success_3)
{
    static const char* k_string_valid = 
        "00000000000000000000000000000000000000000000000000000000000000";
    io_ref_t ref_valid;
    prx_socket_address_t mock_sa;
    int32_t result;

    mock_sa.un.family = prx_address_family_inet6;
    mock_sa.un.ip.un.in6.un.u64[0] = 12345ULL;
    mock_sa.un.ip.un.in6.un.u64[1] = 67890ULL;

    // arrange 
    STRICT_EXPECTED_CALL(string_to_uuid(k_string_valid, ref_valid.un.u8))
        .SetReturn(er_invalid_format);
    STRICT_EXPECTED_CALL(pal_pton(k_string_valid, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_address(&mock_sa, sizeof(mock_sa))
        .SetReturn(er_ok);

    // act 
    result = io_ref_from_string(k_string_valid, &ref_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_IS_TRUE(12345ULL == ref_valid.un.u64[0]);
    ASSERT_IS_TRUE(67890ULL == ref_valid.un.u64[1]);
}

// 
// Test io_ref_from_string passing as string argument an invalid const char* value 
// 
TEST_FUNCTION(io_ref_from_string__arg_string_invalid_1)
{
    io_ref_t ref_valid;
    int32_t result;

    // arrange 

    // act 
    result = io_ref_from_string(NULL, &ref_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_ref_from_string passing as string argument an invalid const char* value 
// 
TEST_FUNCTION(io_ref_from_string__arg_string_invalid_2)
{
    static const char* k_string_invalid =
        "000000000000000000000000000000000000000000000000000000000000001";
    io_ref_t ref_valid;
    int32_t result;

    // arrange 

    // act 
    result = io_ref_from_string(k_string_invalid, &ref_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_arg, result);
}

// 
// Test io_ref_from_string passing as address argument an invalid io_ref_t* value 
// 
TEST_FUNCTION(io_ref_from_string__arg_ref_invalid)
{
    static const char* k_string_valid = "some_ipv6ref_without_braces";
    int32_t result;

    // arrange 

    // act 
    result = io_ref_from_string(k_string_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_ref_from_string unhappy path 
// 
TEST_FUNCTION(io_ref_from_string__neg)
{
    static const char* k_string_valid = "some_ipv6ref_without_braces";
    io_ref_t ref_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(string_to_uuid(k_string_valid, ref_valid.un.u8))
        .SetReturn(er_invalid_format);
    STRICT_EXPECTED_CALL(pal_pton(k_string_valid, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .SetReturn(er_invalid_format);

    // act 
    result = io_ref_from_string(k_string_valid, &ref_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_invalid_format, result);
}

// 
// Test io_ref_from_prx_socket_address happy path 
// 
TEST_FUNCTION(io_ref_from_prx_socket_address__success)
{
    io_ref_t ref_valid;
    prx_socket_address_t mock_sa;
    int32_t result;

    mock_sa.un.family = prx_address_family_inet6;
    mock_sa.un.ip.un.in6.un.u64[0] = 12345ULL;
    mock_sa.un.ip.un.in6.un.u64[1] = 67890ULL;

    // arrange 

    // act 
    result = io_ref_from_prx_socket_address(&mock_sa, &ref_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_IS_TRUE(12345ULL == ref_valid.un.u64[0]);
    ASSERT_IS_TRUE(67890ULL == ref_valid.un.u64[1]);
}

// 
// Test io_ref_from_prx_socket_address passing as sa argument an invalid prx_socket_address_t* value 
// 
TEST_FUNCTION(io_ref_from_prx_socket_address__arg_sa_invalid)
{
    io_ref_t ref_valid;
    int32_t result;

    // arrange 

    // act 
    result = io_ref_from_prx_socket_address(NULL, &ref_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_ref_from_prx_socket_address passing as address argument an invalid io_ref_t* value 
// 
TEST_FUNCTION(io_ref_from_prx_socket_address__arg_ref_invalid)
{
    prx_socket_address_t mock_sa;
    int32_t result;

    mock_sa.un.family = prx_address_family_inet6;
    mock_sa.un.ip.un.in6.un.u64[0] = 12345ULL;
    mock_sa.un.ip.un.in6.un.u64[1] = 67890ULL;

    // arrange 

    // act 
    result = io_ref_from_prx_socket_address(&mock_sa, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_ref_to_prx_socket_address happy path 
// 
TEST_FUNCTION(io_ref_to_prx_socket_address__success)
{
    io_ref_t ref_valid;
    prx_socket_address_t mock_sa;
    int32_t result;

    ref_valid.un.u64[0] = 1234567890ULL;
    ref_valid.un.u64[1] = 9876543210ULL;

    // arrange 

    // act 
    result = io_ref_to_prx_socket_address(&ref_valid, &mock_sa);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_IS_TRUE(1234567890ULL == mock_sa.un.ip.un.in6.un.u64[0]);
    ASSERT_IS_TRUE(9876543210ULL == mock_sa.un.ip.un.in6.un.u64[1]);
}

// 
// Test io_ref_to_prx_socket_address passing as address argument an invalid io_ref_t* value 
// 
TEST_FUNCTION(io_ref_to_prx_socket_address__arg_ref_invalid)
{
    prx_socket_address_t mock_sa;
    int32_t result;

    // arrange 

    // act 
    result = io_ref_to_prx_socket_address(NULL, &mock_sa);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_ref_to_prx_socket_address passing as sa argument an invalid prx_socket_address_t* value 
// 
TEST_FUNCTION(io_ref_to_prx_socket_address__arg_sa_invalid)
{
    io_ref_t ref_valid;
    int32_t result;

    ref_valid.un.u64[0] = 1234567890;
    ref_valid.un.u64[1] = 9876543210;

    // arrange 

    // act 
    result = io_ref_to_prx_socket_address(&ref_valid, NULL);

    // assert 
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    ASSERT_EXPECTED_CALLS();
}

// 
// Test io_ref_to_string happy path 
// 
TEST_FUNCTION(io_ref_to_string__success)
{
    io_ref_t ref_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(string_from_uuid(ref_valid.un.u8, UT_MEM, 37))
        .SetReturn(er_ok);

    // act 
    result = io_ref_to_string(&ref_valid, UT_MEM, 37);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_ref_to_string passing as address argument an invalid io_ref_t* value 
// 
TEST_FUNCTION(io_ref_to_string__arg_ref_invalid)
{
    int32_t result;

    // arrange 

    // act 
    result = io_ref_to_string(NULL, UT_MEM, 37);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_ref_to_string passing as buffer argument an invalid char* value 
// 
TEST_FUNCTION(io_ref_to_string__arg_buffer_invalid)
{
    io_ref_t ref_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(string_from_uuid(ref_valid.un.u8, NULL, 37))
        .SetReturn(er_fault);

    // act 
    result = io_ref_to_string(&ref_valid, NULL, 37);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_ref_to_string passing as buf_len argument an invalid size_t value 
// 
TEST_FUNCTION(io_ref_to_string__arg_buf_len_invalid)
{
    io_ref_t ref_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(string_from_uuid(ref_valid.un.u8, UT_MEM, 36))
        .SetReturn(er_arg);

    // act 
    result = io_ref_to_string(&ref_valid, UT_MEM, 36);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_arg, result);
}

// 
// Test io_ref_to_STRING happy path 
// 
TEST_FUNCTION(io_ref_to_STRING__success)
{
    static STRING_HANDLE k_string_handle_valid = (STRING_HANDLE)0x824;
    io_ref_t ref_valid;
    STRING_HANDLE result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_new())
        .SetReturn(k_string_handle_valid);
    STRICT_EXPECTED_CALL(string_from_uuid(ref_valid.un.u8, IGNORED_PTR_ARG, 37))
        .IgnoreArgument(2)
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_handle_valid, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .SetReturn(0);

    // act 
    result = io_ref_to_STRING(&ref_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void_ptr, k_string_handle_valid, result);
}

// 
// Test io_ref_to_STRING passing as address argument an invalid io_ref_t* value 
// 
TEST_FUNCTION(io_ref_to_STRING__arg_ref_invalid)
{
    static STRING_HANDLE k_string_handle_valid = (STRING_HANDLE)0x824;
    STRING_HANDLE result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_new())
        .SetReturn(k_string_handle_valid);
    STRICT_EXPECTED_CALL(STRING_delete(k_string_handle_valid));

    // act 
    result = io_ref_to_STRING(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void_ptr, NULL, result);
}

// 
// Test io_ref_to_STRING unhappy path 
// 
TEST_FUNCTION(io_ref_to_STRING__neg)
{
    static STRING_HANDLE k_string_handle_valid = (STRING_HANDLE)0x824;
    io_ref_t ref_valid;
    STRING_HANDLE result;

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(STRING_new())
        .SetReturn(k_string_handle_valid)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(string_from_uuid(ref_valid.un.u8, IGNORED_PTR_ARG, 37))
        .IgnoreArgument(2)
        .SetReturn(er_ok)
        .SetFailReturn(er_fatal);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_handle_valid, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .SetReturn(0)
        .SetFailReturn(-1);

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_ref_to_STRING(&ref_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, result, NULL);
}

// 
// Test io_ref_append_to_STRING happy path 
// 
TEST_FUNCTION(io_ref_append_to_STRING__success)
{
    static STRING_HANDLE k_string_handle_valid = (STRING_HANDLE)0x824;
    io_ref_t ref_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(string_from_uuid(ref_valid.un.u8, IGNORED_PTR_ARG, 37))
        .IgnoreArgument(2)
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_handle_valid, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .SetReturn(0);

    // act 
    result = io_ref_append_to_STRING(&ref_valid, k_string_handle_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_ref_append_to_STRING passing as address argument an invalid io_ref_t* value 
// 
TEST_FUNCTION(io_ref_append_to_STRING__arg_ref_invalid)
{
    static STRING_HANDLE k_string_handle_valid = (STRING_HANDLE)0x824;
    int32_t result;

    // arrange 

    // act 
    result = io_ref_append_to_STRING(NULL, k_string_handle_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_ref_append_to_STRING passing as string argument an invalid STRING_HANDLE value 
// 
TEST_FUNCTION(io_ref_append_to_STRING__arg_string_invalid)
{
    io_ref_t ref_valid;
    int32_t result;

    // arrange 

    // act 
    result = io_ref_append_to_STRING(&ref_valid, NULL);

    // assert 
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    ASSERT_EXPECTED_CALLS();
}

// 
// Test io_ref_append_to_STRING unhappy path 
// 
TEST_FUNCTION(io_ref_append_to_STRING__neg)
{
    static STRING_HANDLE k_string_handle_valid = (STRING_HANDLE)0x824;
    io_ref_t ref_valid;
    int32_t result;

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(string_from_uuid(ref_valid.un.u8, IGNORED_PTR_ARG, 37))
        .IgnoreArgument(2)
        .SetReturn(er_ok)
        .SetFailReturn(er_fatal);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_handle_valid, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .SetReturn(0)
        .SetFailReturn(-1);

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_ref_append_to_STRING(&ref_valid, k_string_handle_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_fatal, er_out_of_memory);
}

// 
// Test io_ref_copy happy path 
// 
TEST_FUNCTION(io_ref_copy__success)
{
    io_ref_t src_valid;
    io_ref_t dst_valid;

    src_valid.un.u64[0] = 1234567890;
    src_valid.un.u64[1] = 9876543210;

    // arrange 

    // act 
    io_ref_copy(&src_valid, &dst_valid);

    // assert 
    ASSERT_IS_TRUE(1234567890ULL == dst_valid.un.u64[0]);
    ASSERT_IS_TRUE(9876543210ULL == dst_valid.un.u64[1]);
    ASSERT_EXPECTED_CALLS();
}

// 
// Test io_ref_copy passing as src argument an invalid io_ref_t* value 
// 
TEST_FUNCTION(io_ref_copy__arg_src_invalid)
{
    io_ref_t dst_valid;

    // arrange 

    // act 
    io_ref_copy(NULL, &dst_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test io_ref_copy passing as dst argument an invalid io_ref_t* value 
// 
TEST_FUNCTION(io_ref_copy__arg_dst_invalid)
{
    io_ref_t src_valid;

    src_valid.un.u64[0] = 1234567890;
    src_valid.un.u64[1] = 9876543210;

    // arrange 

    // act 
    io_ref_copy(&src_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test io_ref_swap happy path 
// 
TEST_FUNCTION(io_ref_swap__success)
{
    io_ref_t ref1_valid;
    io_ref_t ref2_valid;

    ref1_valid.un.u64[0] = 1234567890ULL;
    ref1_valid.un.u64[1] = 9876543210ULL;

    ref2_valid.un.u64[0] = 0xaaaaaaaaULL;
    ref2_valid.un.u64[1] = 0xbbbbbbbbULL;

    // arrange 

    // act 
    io_ref_swap(&ref1_valid, &ref2_valid);

    // assert 
    ASSERT_IS_TRUE(1234567890ULL == ref2_valid.un.u64[0]);
    ASSERT_IS_TRUE(9876543210ULL == ref2_valid.un.u64[1]);
    ASSERT_IS_TRUE(0xaaaaaaaaULL == ref1_valid.un.u64[0]);
    ASSERT_IS_TRUE(0xbbbbbbbbULL == ref1_valid.un.u64[1]);
    ASSERT_EXPECTED_CALLS();
}

// 
// Test io_ref_swap passing as ref1 argument an invalid io_ref_t* value 
// 
TEST_FUNCTION(io_ref_swap__arg_ref1_invalid)
{
    io_ref_t ref2_valid;

    ref2_valid.un.u64[0] = 0xaaaaaaaaULL;
    ref2_valid.un.u64[1] = 0xbbbbbbbbULL;

    // arrange 

    // act 
    io_ref_swap(NULL, &ref2_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test io_ref_swap passing as ref2 argument an invalid io_ref_t* value 
// 
TEST_FUNCTION(io_ref_swap__arg_ref2_invalid)
{
    io_ref_t ref1_valid;

    ref1_valid.un.u64[0] = 1234567890ULL;
    ref1_valid.un.u64[1] = 9876543210ULL;


    // act 
    io_ref_swap(&ref1_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test io_ref_equals happy path 
// 
TEST_FUNCTION(io_ref_equals__success_1)
{
    io_ref_t ref1_valid;
    io_ref_t ref2_valid;
    bool result;

    ref1_valid.un.u64[0] = 0xaaaaaaaaULL;
    ref1_valid.un.u64[1] = 0xbbbbbbbbULL;

    ref2_valid.un.u64[0] = 0xaaaaaaaaULL;
    ref2_valid.un.u64[1] = 0xbbbbbbbbULL;

    // arrange 

    // act 
    result = io_ref_equals(&ref1_valid, &ref2_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_TRUE(result);
}

// 
// Test io_ref_equals happy path 
// 
TEST_FUNCTION(io_ref_equals__success_2)
{
    bool result;

    // arrange 

    // act 
    result = io_ref_equals(NULL, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_TRUE(result);
}

// 
// Test io_ref_equals happy path 
// 
TEST_FUNCTION(io_ref_equals__success_3)
{
    io_ref_t ref1_valid;
    bool result;

    ref1_valid.un.u64[0] = ~0ULL;
    ref1_valid.un.u64[1] = ~0ULL;

    // arrange 

    // act 
    result = io_ref_equals(&ref1_valid, &io_ref_bcast);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_TRUE(result);
}

// 
// Test io_ref_equals happy path 
// 
TEST_FUNCTION(io_ref_equals__success_4)
{
    io_ref_t ref1_valid;
    bool result;

    ref1_valid.un.u64[0] = 0;
    ref1_valid.un.u64[1] = 0;

    // arrange 

    // act 
    result = io_ref_equals(&ref1_valid, &io_ref_null);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_TRUE(result);
}

// 
// Test io_ref_equals happy path 
// 
TEST_FUNCTION(io_ref_equals__success_not_equal_1)
{
    io_ref_t ref1_valid;
    io_ref_t ref2_valid;
    bool result;

    ref1_valid.un.u64[0] = 0xaaaaaaaaULL;
    ref1_valid.un.u64[1] = 0xbbbbbbbaULL;

    ref2_valid.un.u64[0] = 0xaaaaaaaaULL;
    ref2_valid.un.u64[1] = 0xbbbbbbbbULL;

    // arrange 

    // act 
    result = io_ref_equals(&ref1_valid, &ref2_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_FALSE(result);
}

// 
// Test io_ref_equals happy path 
// 
TEST_FUNCTION(io_ref_equals__success_not_equal_2)
{
    io_ref_t ref1_valid;
    io_ref_t ref2_valid;
    bool result;

    ref1_valid.un.u64[0] = 0xaaaaaaa1ULL;
    ref1_valid.un.u64[1] = 0xbbbbbbbbULL;

    ref2_valid.un.u64[0] = 0xaaaaaaaaULL;
    ref2_valid.un.u64[1] = 0xbbbbbbbbULL;

    // arrange 

    // act 
    result = io_ref_equals(&ref1_valid, &ref2_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_FALSE(result);
}

// 
// Test io_ref_equals passing as ref1 argument an invalid io_ref_t* value 
// 
TEST_FUNCTION(io_ref_equals__arg_ref1_invalid)
{
    io_ref_t ref2_valid;
    bool result;

    ref2_valid.un.u64[0] = 0;
    ref2_valid.un.u64[1] = 0;

    // arrange 

    // act 
    result = io_ref_equals(NULL, &ref2_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_FALSE(result);
}

// 
// Test io_ref_equals passing as ref2 argument an invalid io_ref_t* value 
// 
TEST_FUNCTION(io_ref_equals__arg_ref2_invalid)
{
    io_ref_t ref1_valid;
    bool result;

    ref1_valid.un.u64[0] = 1ULL;
    ref1_valid.un.u64[1] = 2ULL;

    // arrange 

    // act 
    result = io_ref_equals(&ref1_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_FALSE(result);
}

// 
// Test io_encode_ref happy path 
// 
TEST_FUNCTION(io_encode_ref__success_1)
{
    static io_codec_ctx_t* k_ctx_valid = (io_codec_ctx_t*)0x823423;
    io_ref_t ref_valid;
    int32_t result;

    ref_valid.un.u64[0] = 1ULL;
    ref_valid.un.u64[1] = 2ULL;

    // arrange 
    STRICT_EXPECTED_CALL(io_encode_type_begin(k_ctx_valid, 1))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_codec_ctx_get_codec_id(k_ctx_valid))
        .SetReturn(io_codec_mpack);
    STRICT_EXPECTED_CALL(io_encode_bin(k_ctx_valid, "id", ref_valid.un.u8, 16))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_encode_type_end(k_ctx_valid))
        .SetReturn(er_ok);

    // act 
    result = io_encode_ref(k_ctx_valid, &ref_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_encode_ref happy path 
// 
TEST_FUNCTION(io_encode_ref__success_2)
{
    static io_codec_ctx_t* k_ctx_valid = (io_codec_ctx_t*)0x823423;
    static STRING_HANDLE k_string_handle_valid = (STRING_HANDLE)0x23654;
    io_ref_t ref_valid;
    int32_t result;

    ref_valid.un.u64[0] = 1ULL;
    ref_valid.un.u64[1] = 2ULL;

    // arrange 
    STRICT_EXPECTED_CALL(io_encode_type_begin(k_ctx_valid, 1))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_codec_ctx_get_codec_id(k_ctx_valid))
        .SetReturn(io_codec_json);
    STRICT_EXPECTED_CALL(STRING_new())
        .SetReturn(k_string_handle_valid);
    STRICT_EXPECTED_CALL(string_from_uuid(ref_valid.un.u8, IGNORED_PTR_ARG, 37))
        .IgnoreArgument(2)
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_handle_valid, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .SetReturn(0);
    STRICT_EXPECTED_CALL(io_encode_STRING_HANDLE(k_ctx_valid, "id", k_string_handle_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(STRING_delete(k_string_handle_valid));
    STRICT_EXPECTED_CALL(io_encode_type_end(k_ctx_valid))
        .SetReturn(er_ok);

    // act 
    result = io_encode_ref(k_ctx_valid, &ref_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_encode_ref unhappy path 
// 
TEST_FUNCTION(io_encode_ref__neg_1)
{
    static io_codec_ctx_t* k_ctx_valid = (io_codec_ctx_t*)0x823423;
    io_ref_t ref_valid;
    int32_t result;

    ref_valid.un.u64[0] = 1ULL;
    ref_valid.un.u64[1] = 2ULL;

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(io_encode_type_begin(k_ctx_valid, 1))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL(io_codec_ctx_get_codec_id(k_ctx_valid))
        .SetReturn(io_codec_mpack)
        .SetFailReturn(io_codec_mpack);
    STRICT_EXPECTED_CALL(io_encode_bin(k_ctx_valid, "id", ref_valid.un.u8, 16))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL(io_encode_type_end(k_ctx_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_ref(k_ctx_valid, &ref_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, 
        er_writing, er_ok, er_writing);
}

// 
// Test io_encode_ref unhappy path 
// 
TEST_FUNCTION(io_encode_ref__neg_2)
{
    static io_codec_ctx_t* k_ctx_valid = (io_codec_ctx_t*)0x823423;
    static STRING_HANDLE k_string_handle_valid = (STRING_HANDLE)0x23654;
    io_ref_t ref_valid;
    int32_t result;

    ref_valid.un.u64[0] = 1ULL;
    ref_valid.un.u64[1] = 2ULL;

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(io_encode_type_begin(k_ctx_valid, 1))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL(io_codec_ctx_get_codec_id(k_ctx_valid))
        .SetReturn(io_codec_json)
        .SetFailReturn(io_codec_json);
    STRICT_EXPECTED_CALL(STRING_new())
        .SetReturn(k_string_handle_valid)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(string_from_uuid(ref_valid.un.u8, IGNORED_PTR_ARG, 37))
        .IgnoreArgument(2)
        .SetReturn(er_ok)
        .SetFailReturn(er_ok);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_handle_valid, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(io_encode_STRING_HANDLE(k_ctx_valid, "id", k_string_handle_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL(STRING_delete(k_string_handle_valid));
    STRICT_EXPECTED_CALL(io_encode_type_end(k_ctx_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_ref(k_ctx_valid, &ref_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, 
        er_writing, er_ok, er_out_of_memory, er_ok, er_out_of_memory,
        er_writing, er_ok, er_writing);
}

// 
// Test io_decode_ref happy path 
// 
TEST_FUNCTION(io_decode_ref__success_1)
{
    static io_codec_ctx_t* k_ctx_valid = (io_codec_ctx_t*)0x823423;
    static size_t k_val_size = 16;
    io_ref_t ref_valid, ref_out;
    int32_t result;

    ref_valid.un.u64[0] = 1ULL;
    ref_valid.un.u64[1] = 2ULL;

    // arrange 
    STRICT_EXPECTED_CALL(io_decode_type_begin(k_ctx_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_codec_ctx_get_codec_id(k_ctx_valid))
        .SetReturn(io_codec_mpack);
    STRICT_EXPECTED_CALL(io_decode_bin_fixed(k_ctx_valid, "id", ref_out.un.u8, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_value(ref_valid.un.u8, sizeof(ref_valid.un.u8))
        .ValidateArgumentBuffer(4, &k_val_size, sizeof(k_val_size))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_decode_type_end(k_ctx_valid))
        .SetReturn(er_ok);

    // act 
    result = io_decode_ref(k_ctx_valid, &ref_out);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_IS_TRUE(ref_valid.un.u64[0] == ref_out.un.u64[0]);
    ASSERT_IS_TRUE(ref_valid.un.u64[1] == ref_out.un.u64[1]);
}

// 
// Test io_decode_ref happy path 
// 
TEST_FUNCTION(io_decode_ref__success_2)
{
    static io_codec_ctx_t* k_ctx_valid = (io_codec_ctx_t*)0x823423;
    static STRING_HANDLE k_string_handle_valid = (STRING_HANDLE)0x23654;
    static const char* k_string_valid = "some_uuid_without_braces";
    static uint64_t k_val_0 = 1;
    static uint64_t k_val_1 = 2;

    io_ref_t ref_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(io_decode_type_begin(k_ctx_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_codec_ctx_get_codec_id(k_ctx_valid))
        .SetReturn(io_codec_json);
    STRICT_EXPECTED_CALL(io_decode_STRING_HANDLE(k_ctx_valid, "id", IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_string(&k_string_handle_valid, sizeof(k_string_handle_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_handle_valid))
        .SetReturn(k_string_valid);
    STRICT_EXPECTED_CALL(string_to_uuid(k_string_valid, ref_valid.un.u8))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(STRING_delete(k_string_handle_valid));
    STRICT_EXPECTED_CALL(io_decode_type_end(k_ctx_valid))
        .SetReturn(er_ok);

    // act 
    result = io_decode_ref(k_ctx_valid, &ref_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_decode_ref unhappy path 
// 
TEST_FUNCTION(io_decode_ref__neg_1)
{
    static io_codec_ctx_t* k_ctx_valid = (io_codec_ctx_t*)0x823423;
    static size_t k_val_size = 16;
    io_ref_t ref_valid, ref_out;
    int32_t result;

    ref_valid.un.u64[0] = 1ULL;
    ref_valid.un.u64[1] = 2ULL;

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(io_decode_type_begin(k_ctx_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_invalid_format);
    STRICT_EXPECTED_CALL(io_codec_ctx_get_codec_id(k_ctx_valid))
        .SetReturn(io_codec_mpack)
        .SetFailReturn(io_codec_mpack);
    STRICT_EXPECTED_CALL(io_decode_bin_fixed(k_ctx_valid, "id", ref_out.un.u8, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_value(ref_valid.un.u8, sizeof(ref_valid.un.u8))
        .ValidateArgumentBuffer(4, &k_val_size, sizeof(k_val_size))
        .SetReturn(er_ok)
        .SetFailReturn(er_invalid_format);
    STRICT_EXPECTED_CALL(io_decode_type_end(k_ctx_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_invalid_format);

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_ref(k_ctx_valid, &ref_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result,
        er_invalid_format, er_ok, er_invalid_format);
}

// 
// Test io_decode_ref unhappy path 
// 
TEST_FUNCTION(io_decode_ref__neg_2)
{
    static io_codec_ctx_t* k_ctx_valid = (io_codec_ctx_t*)0x823423;
    static STRING_HANDLE k_string_handle_valid = (STRING_HANDLE)0x23654;
    static const char* k_string_valid = "some_uuid_without_braces";
    static uint64_t k_val_0 = 1;
    static uint64_t k_val_1 = 2;

    io_ref_t ref_valid;
    int32_t result;

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(io_decode_type_begin(k_ctx_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_invalid_format);
    STRICT_EXPECTED_CALL(io_codec_ctx_get_codec_id(k_ctx_valid))
        .SetReturn(io_codec_json)
        .SetFailReturn(io_codec_json);
    STRICT_EXPECTED_CALL(io_decode_STRING_HANDLE(k_ctx_valid, "id", IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_string(&k_string_handle_valid, sizeof(k_string_handle_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_invalid_format);
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_handle_valid))
        .SetReturn(k_string_valid)
        .SetFailReturn(k_string_valid);
    STRICT_EXPECTED_CALL(string_to_uuid(k_string_valid, ref_valid.un.u8))
        .SetReturn(er_ok)
        .SetFailReturn(er_ok);
    STRICT_EXPECTED_CALL(STRING_delete(k_string_handle_valid));
    STRICT_EXPECTED_CALL(io_decode_type_end(k_ctx_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_invalid_format);

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_ref(k_ctx_valid, &ref_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result,
        er_invalid_format, er_ok, er_invalid_format, er_ok, er_ok,
        er_ok, er_invalid_format);
}

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

