// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST io_types
#include "util_ut.h"

//
// 1. Required mocks
//
#include "prx_types.h"
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
END_DECLARE_TEST_SUITE()
// -or- DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
DECLARE_TEST_SETUP()

// 
// Test io_encode_prx_addrinfo happy path 
// 
TEST_FUNCTION(io_encode_prx_addrinfo__success_1)
{
    static io_codec_ctx_t k_ctx_valid;
    prx_addrinfo_t prx_ai_valid;
    int32_t result;

    memset(&prx_ai_valid, 0xff, sizeof(prx_ai_valid));
    prx_ai_valid.name = (char*)"test";
    prx_ai_valid.address.un.family = prx_address_family_unspec;

    // arrange
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 2);
    STRICT_EXPECTED_CALL(io_encode_object(IGNORED_PTR_ARG, "address", false, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_object(&k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL(io_encode_int32(IGNORED_PTR_ARG, "family", (int32_t)prx_ai_valid.address.un.family))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);
    STRICT_EXPECTED_CALL(io_encode_string(&k_ctx_valid, "name", "test"))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_encode_prx_addrinfo(&k_ctx_valid, &prx_ai_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_encode_prx_addrinfo happy path 
// 
TEST_FUNCTION(io_encode_prx_addrinfo__success_2)
{
    static io_codec_ctx_t k_ctx_valid;
    prx_addrinfo_t prx_ai_valid;
    int32_t result;

    memset(&prx_ai_valid, 0xff, sizeof(prx_ai_valid));
    prx_ai_valid.name = NULL;
    prx_ai_valid.address.un.family = prx_address_family_unspec;

    // arrange
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 2);
    STRICT_EXPECTED_CALL(io_encode_object(IGNORED_PTR_ARG, "address", false, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_object(&k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL(io_encode_int32(IGNORED_PTR_ARG, "family", (int32_t)prx_ai_valid.address.un.family))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);
    STRICT_EXPECTED_CALL(io_encode_string(&k_ctx_valid, "name", ""))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_encode_prx_addrinfo(&k_ctx_valid, &prx_ai_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_encode_prx_addrinfo unhappy path 
// 
TEST_FUNCTION(io_encode_prx_addrinfo__neg)
{
    static io_codec_ctx_t k_ctx_valid;
    prx_addrinfo_t prx_ai_valid;
    int32_t result;

    memset(&prx_ai_valid, 0xff, sizeof(prx_ai_valid));
    prx_ai_valid.name = (char*)"foo";
    prx_ai_valid.address.un.family = prx_address_family_unspec;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 2);
    STRICT_EXPECTED_CALL(io_encode_object(IGNORED_PTR_ARG, "address", false, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_object(&k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL(io_encode_int32(IGNORED_PTR_ARG, "family", (int32_t)prx_ai_valid.address.un.family))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);
    STRICT_EXPECTED_CALL(io_encode_string(&k_ctx_valid, "name", "foo"))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_prx_addrinfo(&k_ctx_valid, &prx_ai_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_writing);
}

// 
// Test io_decode_prx_addrinfo happy path 
// 
TEST_FUNCTION(io_decode_prx_addrinfo__success)
{
    static const int32_t k_af_valid = prx_address_family_unspec;
    static io_codec_ctx_t k_ctx_valid;
    prx_addrinfo_t prx_ai_valid;
    int32_t result;

    memset(&prx_ai_valid, 0xff, sizeof(prx_ai_valid));

    // arrange
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 2);
    STRICT_EXPECTED_CALL(io_decode_object(IGNORED_PTR_ARG, "address", IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_is_null(&k_false, sizeof(k_false))
        .CopyOutArgumentBuffer_object(&k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL(io_decode_int32(IGNORED_PTR_ARG, "family", (int32_t*)&prx_ai_valid.address.un.family))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_val(&k_af_valid, sizeof(k_af_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);
    STRICT_EXPECTED_CALL(io_decode_string_default(&k_ctx_valid, "name", IGNORED_PTR_ARG))
        .IgnoreArgument(3)
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_decode_prx_addrinfo(&k_ctx_valid, &prx_ai_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_IS_TRUE(0 == prx_ai_valid.reserved);
}

// 
// Test io_decode_prx_addrinfo unhappy path 
// 
TEST_FUNCTION(io_decode_prx_addrinfo__neg)
{
    static const int32_t k_af_valid = prx_address_family_unspec;
    static io_codec_ctx_t k_ctx_valid;
    prx_addrinfo_t prx_ai_valid;
    int32_t result;
 
    memset(&prx_ai_valid, 0xff, sizeof(prx_ai_valid));

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 2);
    STRICT_EXPECTED_CALL(io_decode_object(IGNORED_PTR_ARG, "address", IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_is_null(&k_false, sizeof(k_false))
        .CopyOutArgumentBuffer_object(&k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL(io_decode_int32(&k_ctx_valid, "family", (int32_t*)&prx_ai_valid.address.un.family))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_val(&k_af_valid, sizeof(k_af_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);
  //  STRICT_EXPECTED_CALL(io_decode_string_malloc(&k_ctx_valid, "name", IGNORED_PTR_ARG))
  //      .IgnoreArgument(3)
  //      .SetReturn(er_ok)
  //      .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_prx_addrinfo(&k_ctx_valid, &prx_ai_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result,
        er_invalid_format, er_out_of_memory, er_invalid_format, er_out_of_memory);
}

// 
// Test io_encode_prx_ifaddrinfo happy path 
// 
TEST_FUNCTION(io_encode_prx_ifaddrinfo__success)
{
    static const char* k_name_valid = "eth1";
    static io_codec_ctx_t k_ctx_valid;
    prx_ifaddrinfo_t prx_ifa_valid;
    int32_t result;

    memset(&prx_ifa_valid, 0xff, sizeof(prx_ifa_valid));
    strcpy(prx_ifa_valid.name, k_name_valid);
    prx_ifa_valid.broadcast_addr.un.family = prx_ifa_valid.address.un.family = prx_address_family_unspec;

    // arrange
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 6);
    STRICT_EXPECTED_CALL(io_encode_object(IGNORED_PTR_ARG, "address", false, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_object(&k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL(io_encode_int32(IGNORED_PTR_ARG, "family", (int32_t)prx_ifa_valid.address.un.family))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, uint8, &prx_ifa_valid, prefix);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, uint8, &prx_ifa_valid, flags);
    STRICT_EXPECTED_CALL(io_encode_string(&k_ctx_valid, "name", prx_ifa_valid.name))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, int32, &prx_ifa_valid, index);
    STRICT_EXPECTED_CALL(io_encode_object(IGNORED_PTR_ARG, "broadcast_addr", false, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_object(&k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL(io_encode_int32(IGNORED_PTR_ARG, "family", (int32_t)prx_ifa_valid.broadcast_addr.un.family))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_encode_prx_ifaddrinfo(&k_ctx_valid, &prx_ifa_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_encode_prx_ifaddrinfo unhappy path 
// 
TEST_FUNCTION(io_encode_prx_ifaddrinfo__neg)
{
    static const char* k_name_valid = "eth2";
    static io_codec_ctx_t k_ctx_valid;
    prx_ifaddrinfo_t prx_ifa_valid;
    int32_t result;

    memset(&prx_ifa_valid, 0xff, sizeof(prx_ifa_valid));
    strcpy(prx_ifa_valid.name, k_name_valid);
    prx_ifa_valid.broadcast_addr.un.family = prx_ifa_valid.address.un.family = prx_address_family_unspec;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 6);
    STRICT_EXPECTED_CALL(io_encode_object(IGNORED_PTR_ARG, "address", false, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_object(&k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL(io_encode_int32(IGNORED_PTR_ARG, "family", (int32_t)prx_ifa_valid.address.un.family))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, uint8, &prx_ifa_valid, prefix);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, uint8, &prx_ifa_valid, flags);
    STRICT_EXPECTED_CALL(io_encode_string(&k_ctx_valid, "name", prx_ifa_valid.name))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, int32, &prx_ifa_valid, index);
    STRICT_EXPECTED_CALL(io_encode_object(IGNORED_PTR_ARG, "broadcast_addr", false, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_object(&k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL(io_encode_int32(IGNORED_PTR_ARG, "family", (int32_t)prx_ifa_valid.broadcast_addr.un.family))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_prx_ifaddrinfo(&k_ctx_valid, &prx_ifa_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_writing);
}

// 
// Test io_decode_prx_ifaddrinfo happy path 
// 
TEST_FUNCTION(io_decode_prx_ifaddrinfo__success)
{
    static const int32_t k_af_valid = prx_address_family_unspec;
    static const char* k_name_valid = "eth3";
    static io_codec_ctx_t k_ctx_valid;
    prx_ifaddrinfo_t prx_ifa_valid;
    int32_t result;

    strcpy(prx_ifa_valid.name, k_name_valid);

    // arrange
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 6);
    STRICT_EXPECTED_CALL(io_decode_object(IGNORED_PTR_ARG, "address", IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_is_null(&k_false, sizeof(k_false))
        .CopyOutArgumentBuffer_object(&k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL(io_decode_int32(IGNORED_PTR_ARG, "family", (int32_t*)&prx_ifa_valid.address.un.family))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_val(&k_af_valid, sizeof(k_af_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, uint8, &prx_ifa_valid, prefix);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, uint8, &prx_ifa_valid, flags);
    STRICT_EXPECTED_CALL(io_decode_string_fixed(&k_ctx_valid, "name", prx_ifa_valid.name, sizeof(prx_ifa_valid.name)))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, int32, &prx_ifa_valid, index);
    STRICT_EXPECTED_CALL(io_decode_object(IGNORED_PTR_ARG, "broadcast_addr", IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_is_null(&k_false, sizeof(k_false))
        .CopyOutArgumentBuffer_object(&k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL(io_decode_int32(IGNORED_PTR_ARG, "family", (int32_t*)&prx_ifa_valid.broadcast_addr.un.family))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_val(&k_af_valid, sizeof(k_af_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_decode_prx_ifaddrinfo(&k_ctx_valid, &prx_ifa_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(int32_t, 0, prx_ifa_valid.reserved);
}

// 
// Test io_decode_prx_ifaddrinfo unhappy path 
// 
TEST_FUNCTION(io_decode_prx_ifaddrinfo__neg)
{
    static const int32_t k_af_valid = prx_address_family_unspec;
    static const char* k_name_valid = "eth4";
    static io_codec_ctx_t k_ctx_valid;
    prx_ifaddrinfo_t prx_ifa_valid;
    int32_t result;

    memset(&prx_ifa_valid, 0xff, sizeof(prx_ifa_valid));
    strcpy(prx_ifa_valid.name, k_name_valid);

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 6);
    STRICT_EXPECTED_CALL(io_decode_object(IGNORED_PTR_ARG, "address", IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_is_null(&k_false, sizeof(k_false))
        .CopyOutArgumentBuffer_object(&k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL(io_decode_int32(IGNORED_PTR_ARG, "family", (int32_t*)&prx_ifa_valid.address.un.family))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_val(&k_af_valid, sizeof(k_af_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, uint8, &prx_ifa_valid, prefix);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, uint8, &prx_ifa_valid, flags);
    STRICT_EXPECTED_CALL(io_decode_string_fixed(&k_ctx_valid, "name", prx_ifa_valid.name, sizeof(prx_ifa_valid.name)))
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, int32, &prx_ifa_valid, index);
    STRICT_EXPECTED_CALL(io_decode_object(IGNORED_PTR_ARG, "broadcast_addr", IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_is_null(&k_false, sizeof(k_false))
        .CopyOutArgumentBuffer_object(&k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL(io_decode_int32(IGNORED_PTR_ARG, "family", (int32_t*)&prx_ifa_valid.broadcast_addr.un.family))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_val(&k_af_valid, sizeof(k_af_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_prx_ifaddrinfo(&k_ctx_valid, &prx_ifa_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, 
        er_invalid_format, er_out_of_memory, er_invalid_format, er_out_of_memory, er_out_of_memory,
        er_out_of_memory,  er_out_of_memory, er_out_of_memory,  er_out_of_memory, er_out_of_memory, 
        er_invalid_format, er_out_of_memory);
}

// 
// Test io_encode_prx_socket_address happy path 
// 
TEST_FUNCTION(io_encode_prx_socket_address__success_1)
{
    static io_codec_ctx_t k_ctx_valid;
    prx_socket_address_t prx_address_valid;
    int32_t result;

    memset(&prx_address_valid, 0xaf, sizeof(prx_address_valid));
    prx_address_valid.un.family = prx_address_family_inet;

    // arrange
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 3);
    STRICT_EXPECTED_CALL(io_encode_int32(&k_ctx_valid, "family", (int32_t)prx_address_valid.un.family))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_encode_uint16(&k_ctx_valid, "port", prx_address_valid.un.ip.port))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_encode_bin(&k_ctx_valid, "addr", prx_address_valid.un.ip.un.in4.un.u8, 4))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_encode_prx_socket_address(&k_ctx_valid, &prx_address_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_encode_prx_socket_address unhappy path 
// 
TEST_FUNCTION(io_encode_prx_socket_address__neg_1)
{
    static io_codec_ctx_t k_ctx_valid;
    prx_socket_address_t prx_address_valid;
    int32_t result;

    memset(&prx_address_valid, 0xaf, sizeof(prx_address_valid));
    prx_address_valid.un.family = prx_address_family_inet;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 3);
    STRICT_EXPECTED_CALL(io_encode_int32(&k_ctx_valid, "family", (int32_t)prx_address_valid.un.family))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL(io_encode_uint16(&k_ctx_valid, "port", prx_address_valid.un.ip.port))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL(io_encode_bin(&k_ctx_valid, "addr", prx_address_valid.un.ip.un.in4.un.u8, 4))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_prx_socket_address(&k_ctx_valid, &prx_address_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_writing);
}

// 
// Test io_encode_prx_socket_address happy path 
// 
TEST_FUNCTION(io_encode_prx_socket_address__success_2)
{
    static io_codec_ctx_t k_ctx_valid;
    prx_socket_address_t prx_address_valid;
    int32_t result;

    memset(&prx_address_valid, 0xaf, sizeof(prx_address_valid));
    prx_address_valid.un.family = prx_address_family_inet6;

    // arrange
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 5);
    STRICT_EXPECTED_CALL(io_encode_int32(&k_ctx_valid, "family", (int32_t)prx_address_valid.un.family))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_encode_uint16(&k_ctx_valid, "port", prx_address_valid.un.ip.port))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_encode_uint32(&k_ctx_valid, "flow", prx_address_valid.un.ip.flow))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_encode_bin(&k_ctx_valid, "addr", prx_address_valid.un.ip.un.in6.un.u8, 16))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_encode_uint32(&k_ctx_valid, "scope_id", prx_address_valid.un.ip.un.in6.scope_id))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_encode_prx_socket_address(&k_ctx_valid, &prx_address_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_encode_prx_socket_address unhappy path 
// 
TEST_FUNCTION(io_encode_prx_socket_address__neg_2)
{
    static io_codec_ctx_t k_ctx_valid;
    prx_socket_address_t prx_address_valid;
    int32_t result;

    memset(&prx_address_valid, 0xaf, sizeof(prx_address_valid));
    prx_address_valid.un.family = prx_address_family_inet6;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 5);
    STRICT_EXPECTED_CALL(io_encode_int32(&k_ctx_valid, "family", (int32_t)prx_address_valid.un.family))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL(io_encode_uint16(&k_ctx_valid, "port", prx_address_valid.un.ip.port))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL(io_encode_uint32(&k_ctx_valid, "flow", prx_address_valid.un.ip.flow))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL(io_encode_bin(&k_ctx_valid, "addr", prx_address_valid.un.ip.un.in6.un.u8, 16))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL(io_encode_uint32(&k_ctx_valid, "scope_id", prx_address_valid.un.ip.un.in6.scope_id))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_prx_socket_address(&k_ctx_valid, &prx_address_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_writing);
}

// 
// Test io_encode_prx_socket_address happy path 
// 
TEST_FUNCTION(io_encode_prx_socket_address__success_3)
{
    static const char* k_host_valid = "asdfsadfdfgdskfdjdhksjdfhfkse";
    static io_codec_ctx_t k_ctx_valid;
    prx_socket_address_t prx_address_valid;
    int32_t result;

    memset(&prx_address_valid, 0xaf, sizeof(prx_address_valid));
    prx_address_valid.un.family = prx_address_family_proxy;
    strcpy(prx_address_valid.un.proxy.host, k_host_valid);

    // arrange
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 5);
    STRICT_EXPECTED_CALL(io_encode_int32(&k_ctx_valid, "family", (int32_t)prx_address_valid.un.family))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_encode_uint16(&k_ctx_valid, "port", prx_address_valid.un.proxy.port))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_encode_uint16(&k_ctx_valid, "flags", prx_address_valid.un.proxy.flags))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_encode_int32(&k_ctx_valid, "itf_index", prx_address_valid.un.proxy.itf_index))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_encode_string(&k_ctx_valid, "host", k_host_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_encode_prx_socket_address(&k_ctx_valid, &prx_address_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_encode_prx_socket_address unhappy path 
// 
TEST_FUNCTION(io_encode_prx_socket_address__neg_3)
{
    static const char* k_host_valid = "sdfsdsfdsfsdfs";
    static io_codec_ctx_t k_ctx_valid;
    prx_socket_address_t prx_address_valid;
    int32_t result;

    memset(&prx_address_valid, 0xaf, sizeof(prx_address_valid));
    prx_address_valid.un.family = prx_address_family_proxy;
    strcpy(prx_address_valid.un.proxy.host, k_host_valid);

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 5);
    STRICT_EXPECTED_CALL(io_encode_int32(&k_ctx_valid, "family", (int32_t)prx_address_valid.un.family))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL(io_encode_uint32(&k_ctx_valid, "port", prx_address_valid.un.proxy.port))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL(io_encode_uint32(&k_ctx_valid, "flags", prx_address_valid.un.proxy.flags))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL(io_encode_int32(&k_ctx_valid, "itf_index", prx_address_valid.un.proxy.itf_index))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL(io_encode_string(&k_ctx_valid, "host", k_host_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_prx_socket_address(&k_ctx_valid, &prx_address_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_writing);
}

// 
// Test io_encode_prx_socket_address happy path 
// 
TEST_FUNCTION(io_encode_prx_socket_address__success_4)
{
    static const char* k_path_valid = "asdfsadfdfgdskfdjdhksjdfhfkse";
    static io_codec_ctx_t k_ctx_valid;
    prx_socket_address_t prx_address_valid;
    int32_t result;

    memset(&prx_address_valid, 0xaf, sizeof(prx_address_valid));
    prx_address_valid.un.family = prx_address_family_unix;
    strcpy(prx_address_valid.un.ux.path, k_path_valid);

    // arrange
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 2);
    STRICT_EXPECTED_CALL(io_encode_int32(&k_ctx_valid, "family", (int32_t)prx_address_valid.un.family))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_encode_string(&k_ctx_valid, "path", k_path_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_encode_prx_socket_address(&k_ctx_valid, &prx_address_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_encode_prx_socket_address unhappy path 
// 
TEST_FUNCTION(io_encode_prx_socket_address__neg_4)
{
    static const char* k_path_valid = "asdfsadfdfgdskfdjdhksjdfhfkse";
    static io_codec_ctx_t k_ctx_valid;
    prx_socket_address_t prx_address_valid;
    int32_t result;

    memset(&prx_address_valid, 0xaf, sizeof(prx_address_valid));
    prx_address_valid.un.family = prx_address_family_unix;
    strcpy(prx_address_valid.un.ux.path, k_path_valid);

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 2);
    STRICT_EXPECTED_CALL(io_encode_int32(&k_ctx_valid, "family", (int32_t)prx_address_valid.un.family))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL(io_encode_string(&k_ctx_valid, "path", k_path_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_prx_socket_address(&k_ctx_valid, &prx_address_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_writing);
}

// 
// Test io_encode_prx_socket_address happy path 
// 
TEST_FUNCTION(io_encode_prx_socket_address__success_5)
{
    static io_codec_ctx_t k_ctx_valid;
    prx_socket_address_t prx_address_valid;
    int32_t result;

    memset(&prx_address_valid, 0xaf, sizeof(prx_address_valid));
    prx_address_valid.un.family = prx_address_family_unspec;

    // arrange
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL(io_encode_int32(&k_ctx_valid, "family", (int32_t)prx_address_valid.un.family))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_encode_prx_socket_address(&k_ctx_valid, &prx_address_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_encode_prx_socket_address happy path 
// 
TEST_FUNCTION(io_encode_prx_socket_address__neg_5)
{
    static io_codec_ctx_t k_ctx_valid;
    prx_socket_address_t prx_address_valid;
    int32_t result;

    memset(&prx_address_valid, 0, sizeof(prx_address_valid));
    prx_address_valid.un.family = (prx_address_family_t)-1;

    // arrange

    // act 
    result = io_encode_prx_socket_address(&k_ctx_valid, &prx_address_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_arg, result);
}

// 
// Test io_decode_prx_socket_address happy path 
// 
TEST_FUNCTION(io_decode_prx_socket_address__success_1)
{
    static const int32_t k_af_valid = prx_address_family_inet;
    static io_codec_ctx_t k_ctx_valid;
    static const size_t k_size_valid = 4;
    prx_socket_address_t prx_address_valid;
    int32_t result;

    memset(&prx_address_valid, 0xff, sizeof(prx_address_valid));

    // arrange
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 3);
    STRICT_EXPECTED_CALL(io_decode_int32(&k_ctx_valid, "family", (int32_t*)&prx_address_valid.un.family))
        .CopyOutArgumentBuffer_val(&k_af_valid, sizeof(k_af_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_decode_uint16(&k_ctx_valid, "port", IGNORED_PTR_ARG))
        .IgnoreArgument(3)
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_decode_bin_fixed(&k_ctx_valid, "addr", IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(3)
        .ValidateArgumentBuffer(4, &k_size_valid, sizeof(k_size_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_decode_prx_socket_address(&k_ctx_valid, &prx_address_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_decode_prx_socket_address unhappy path 
// 
TEST_FUNCTION(io_decode_prx_socket_address__neg_1)
{
    static const int32_t k_af_valid = prx_address_family_inet;
    static io_codec_ctx_t k_ctx_valid;
    static const size_t k_size_valid = 4;
    prx_socket_address_t prx_address_valid;
    int32_t result;

    memset(&prx_address_valid, 0xff, sizeof(prx_address_valid));

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 3);
    STRICT_EXPECTED_CALL(io_decode_int32(&k_ctx_valid, "family", (int32_t*)&prx_address_valid.un.family))
        .CopyOutArgumentBuffer_val(&k_af_valid, sizeof(k_af_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL(io_decode_uint16(&k_ctx_valid, "port", IGNORED_PTR_ARG))
        .IgnoreArgument(3)
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL(io_decode_bin_fixed(&k_ctx_valid, "addr", IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(3)
        .ValidateArgumentBuffer(4, &k_size_valid, sizeof(k_size_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_prx_socket_address(&k_ctx_valid, &prx_address_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result,  er_invalid_format, er_out_of_memory);
}

// 
// Test io_decode_prx_socket_address happy path 
// 
TEST_FUNCTION(io_decode_prx_socket_address__success_2)
{
    static const int32_t k_af_valid = prx_address_family_inet6;
    static io_codec_ctx_t k_ctx_valid;
    static const size_t k_size_valid = 16;
    prx_socket_address_t prx_address_valid;
    int32_t result;

    memset(&prx_address_valid, 0xff, sizeof(prx_address_valid));

    // arrange
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 5);
    STRICT_EXPECTED_CALL(io_decode_int32(&k_ctx_valid, "family", (int32_t*)&prx_address_valid.un.family))
        .CopyOutArgumentBuffer_val(&k_af_valid, sizeof(k_af_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_decode_uint16(&k_ctx_valid, "port", IGNORED_PTR_ARG))
        .IgnoreArgument(3)
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_decode_uint32(&k_ctx_valid, "flow", IGNORED_PTR_ARG))
        .IgnoreArgument(3)
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_decode_bin_fixed(&k_ctx_valid, "addr", IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(3)
        .ValidateArgumentBuffer(4, &k_size_valid, sizeof(k_size_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_decode_uint32(&k_ctx_valid, "scope_id", IGNORED_PTR_ARG))
        .IgnoreArgument(3)
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_decode_prx_socket_address(&k_ctx_valid, &prx_address_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_decode_prx_socket_address unhappy path 
// 
TEST_FUNCTION(io_decode_prx_socket_address__neg_2)
{
    static const int32_t k_af_valid = prx_address_family_inet6;
    static io_codec_ctx_t k_ctx_valid;
    static const size_t k_size_valid = 16;
    prx_socket_address_t prx_address_valid;
    int32_t result;

    memset(&prx_address_valid, 0xff, sizeof(prx_address_valid));

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 5);
    STRICT_EXPECTED_CALL(io_decode_int32(&k_ctx_valid, "family", (int32_t*)&prx_address_valid.un.family))
        .CopyOutArgumentBuffer_val(&k_af_valid, sizeof(k_af_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL(io_decode_uint16(&k_ctx_valid, "port", IGNORED_PTR_ARG))
        .IgnoreArgument(3)
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL(io_decode_uint32(&k_ctx_valid, "flow", IGNORED_PTR_ARG))
        .IgnoreArgument(3)
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL(io_decode_bin_fixed(&k_ctx_valid, "addr", IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(3)
        .ValidateArgumentBuffer(4, &k_size_valid, sizeof(k_size_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL(io_decode_uint32(&k_ctx_valid, "scope_id", IGNORED_PTR_ARG))
        .IgnoreArgument(3)
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_prx_socket_address(&k_ctx_valid, &prx_address_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_invalid_format, er_out_of_memory);
}

// 
// Test io_decode_prx_socket_address happy path 
// 
TEST_FUNCTION(io_decode_prx_socket_address__success_3)
{
    static const int32_t k_af_valid = prx_address_family_proxy;
    static io_codec_ctx_t k_ctx_valid;
    prx_socket_address_t prx_address_valid;
    int32_t result;

    memset(&prx_address_valid, 0xff, sizeof(prx_address_valid));
    prx_address_valid.un.proxy.host[0] = 0;

    // arrange
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 5);
    STRICT_EXPECTED_CALL(io_decode_int32(&k_ctx_valid, "family", (int32_t*)&prx_address_valid.un.family))
        .CopyOutArgumentBuffer_val(&k_af_valid, sizeof(k_af_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_decode_uint16(&k_ctx_valid, "port", IGNORED_PTR_ARG))
        .IgnoreArgument(3)
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_decode_uint16(&k_ctx_valid, "flags", IGNORED_PTR_ARG))
        .IgnoreArgument(3)
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_decode_int32(&k_ctx_valid, "itf_index", IGNORED_PTR_ARG))
        .IgnoreArgument(3)
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_decode_string_fixed(&k_ctx_valid, "host", IGNORED_PTR_ARG, sizeof(prx_address_valid.un.proxy.host)))
        .IgnoreArgument(3)
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_decode_prx_socket_address(&k_ctx_valid, &prx_address_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_decode_prx_socket_address unhappy path 
// 
TEST_FUNCTION(io_decode_prx_socket_address__neg_3)
{
    static const int32_t k_af_valid = prx_address_family_proxy;
    static io_codec_ctx_t k_ctx_valid;
    prx_socket_address_t prx_address_valid;
    int32_t result;

    memset(&prx_address_valid, 0xff, sizeof(prx_address_valid));
    prx_address_valid.un.proxy.host[0] = 0;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 5);
    STRICT_EXPECTED_CALL(io_decode_int32(&k_ctx_valid, "family", (int32_t*)&prx_address_valid.un.family))
        .CopyOutArgumentBuffer_val(&k_af_valid, sizeof(k_af_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL(io_decode_uint16(&k_ctx_valid, "port", IGNORED_PTR_ARG))
        .IgnoreArgument(3)
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL(io_decode_uint16(&k_ctx_valid, "flags", IGNORED_PTR_ARG))
        .IgnoreArgument(3)
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL(io_decode_int32(&k_ctx_valid, "itf_index", IGNORED_PTR_ARG))
        .IgnoreArgument(3)
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL(io_decode_string_fixed(&k_ctx_valid, "host", IGNORED_PTR_ARG, sizeof(prx_address_valid.un.proxy.host)))
        .IgnoreArgument(3)
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_prx_socket_address(&k_ctx_valid, &prx_address_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_invalid_format, er_out_of_memory);
}

// 
// Test io_decode_prx_socket_address happy path 
// 
TEST_FUNCTION(io_decode_prx_socket_address__success_4)
{
    static const int32_t k_af_valid = prx_address_family_unix;
    static io_codec_ctx_t k_ctx_valid;
    prx_socket_address_t prx_address_valid;
    int32_t result;

    memset(&prx_address_valid, 0xff, sizeof(prx_address_valid));
    prx_address_valid.un.ux.path[0] = 0;

    // arrange
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 2);
    STRICT_EXPECTED_CALL(io_decode_int32(&k_ctx_valid, "family", (int32_t*)&prx_address_valid.un.family))
        .CopyOutArgumentBuffer_val(&k_af_valid, sizeof(k_af_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_decode_string_fixed(&k_ctx_valid, "path", IGNORED_PTR_ARG, sizeof(prx_address_valid.un.ux.path)))
        .IgnoreArgument(3)
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_decode_prx_socket_address(&k_ctx_valid, &prx_address_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_decode_prx_socket_address unhappy path 
// 
TEST_FUNCTION(io_decode_prx_socket_address__neg_4)
{
    static const int32_t k_af_valid = prx_address_family_unix;
    static io_codec_ctx_t k_ctx_valid;
    prx_socket_address_t prx_address_valid;
    int32_t result;

    memset(&prx_address_valid, 0xff, sizeof(prx_address_valid));
    prx_address_valid.un.ux.path[0] = 0;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 2);
    STRICT_EXPECTED_CALL(io_decode_int32(&k_ctx_valid, "family", (int32_t*)&prx_address_valid.un.family))
        .CopyOutArgumentBuffer_val(&k_af_valid, sizeof(k_af_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL(io_decode_string_fixed(&k_ctx_valid, "path", IGNORED_PTR_ARG, sizeof(prx_address_valid.un.ux.path)))
        .IgnoreArgument(3)
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_prx_socket_address(&k_ctx_valid, &prx_address_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_invalid_format, er_out_of_memory);
}

// 
// Test io_decode_prx_socket_address happy path 
// 
TEST_FUNCTION(io_decode_prx_socket_address__success_5)
{
    static const int32_t k_af_valid = prx_address_family_unspec;
    static io_codec_ctx_t k_ctx_valid;
    prx_socket_address_t prx_address_valid;
    int32_t result;

    memset(&prx_address_valid, 0xff, sizeof(prx_address_valid));

    // arrange
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL(io_decode_int32(&k_ctx_valid, "family", (int32_t*)&prx_address_valid.un.family))
        .CopyOutArgumentBuffer_val(&k_af_valid, sizeof(k_af_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_decode_prx_socket_address(&k_ctx_valid, &prx_address_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_decode_prx_socket_address happy path 
// 
TEST_FUNCTION(io_decode_prx_socket_address__neg_5)
{
    static const int32_t k_af_invalid = -1;
    static io_codec_ctx_t k_ctx_valid;
    prx_socket_address_t prx_address_valid;
    int32_t result;

    memset(&prx_address_valid, 0xff, sizeof(prx_address_valid));

    // arrange
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL(io_decode_int32(&k_ctx_valid, "family", (int32_t*)&prx_address_valid.un.family))
        .CopyOutArgumentBuffer_val(&k_af_invalid, sizeof(k_af_invalid))
        .SetReturn(er_ok);

    // act 
    result = io_decode_prx_socket_address(&k_ctx_valid, &prx_address_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_invalid_format, result);
}

// 
// Test io_encode_prx_socket_properties happy path 
// 
TEST_FUNCTION(io_encode_prx_socket_properties__success_1)
{
    static io_codec_ctx_t k_ctx_valid;
    prx_socket_properties_t prx_sp_valid;
    int32_t result;

    memset(&prx_sp_valid, 0, sizeof(prx_sp_valid));
    prx_sp_valid.family = prx_sp_valid.address.un.family = prx_address_family_unspec;

    // arrange
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 7);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, int32, &prx_sp_valid, family);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, int32, &prx_sp_valid, sock_type);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, int32, &prx_sp_valid, proto_type);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, uint32, &prx_sp_valid, flags);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, uint64, &prx_sp_valid, timeout);
    STRICT_EXPECTED_CALL(io_encode_object(IGNORED_PTR_ARG, "address", false, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_object(&k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL(io_encode_int32(IGNORED_PTR_ARG, "family", (int32_t)prx_sp_valid.address.un.family))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);
    STRICT_EXPECTED_CALL(io_encode_array(&k_ctx_valid, "options", 0, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_array(&k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_encode_prx_socket_properties(&k_ctx_valid, &prx_sp_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_encode_prx_socket_properties happy path 
// 
TEST_FUNCTION(io_encode_prx_socket_properties__success_2)
{
    static io_codec_ctx_t k_ctx_valid;
    const size_t k_size_valid = 2;
    prx_socket_properties_t prx_sp_valid;
    prx_property_t prx_sp_props[2];
    int32_t result;

    memset(&prx_sp_valid, 0, sizeof(prx_sp_valid));
    prx_sp_valid.family = prx_sp_valid.address.un.family = prx_address_family_unspec;
    prx_sp_valid.options_len = k_size_valid;
    prx_sp_valid.options = prx_sp_props;
    prx_sp_props[0].type = prx_so_debug;
    prx_sp_props[0].property.value = 1;
    prx_sp_props[1].type = prx_so_broadcast;
    prx_sp_props[1].property.value = 1;

    // arrange
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 7);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, int32, &prx_sp_valid, family);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, int32, &prx_sp_valid, sock_type);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, int32, &prx_sp_valid, proto_type);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, uint32, &prx_sp_valid, flags);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, uint64, &prx_sp_valid, timeout);
    STRICT_EXPECTED_CALL(io_encode_object(IGNORED_PTR_ARG, "address", false, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_object(&k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL(io_encode_int32(IGNORED_PTR_ARG, "family", (int32_t)prx_sp_valid.address.un.family))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);
    STRICT_EXPECTED_CALL(io_encode_array(&k_ctx_valid, "options", k_size_valid, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_array(&k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_encode_object(IGNORED_PTR_ARG, NULL, false, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_object(&k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 2);
    STRICT_EXPECTED_CALL(io_encode_int32(&k_ctx_valid, "type", prx_so_debug))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_encode_uint64(&k_ctx_valid, "property", 1))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);
    STRICT_EXPECTED_CALL(io_encode_object(IGNORED_PTR_ARG, NULL, false, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_object(&k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 2);
    STRICT_EXPECTED_CALL(io_encode_int32(&k_ctx_valid, "type", prx_so_broadcast))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_encode_uint64(&k_ctx_valid, "property", 1))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

        // act 
    result = io_encode_prx_socket_properties(&k_ctx_valid, &prx_sp_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_encode_prx_socket_properties unhappy path 
// 
TEST_FUNCTION(io_encode_prx_socket_properties__neg)
{
    static io_codec_ctx_t k_ctx_valid;
    const size_t k_size_valid = 2;
    prx_socket_properties_t prx_sp_valid;
    prx_property_t prx_sp_props[2];
    int32_t result;

    memset(&prx_sp_valid, 0, sizeof(prx_sp_valid));
    prx_sp_valid.family = prx_sp_valid.address.un.family = prx_address_family_unspec;
    prx_sp_valid.options_len = k_size_valid;
    prx_sp_valid.options = prx_sp_props;
    prx_sp_props[0].type = prx_so_debug;
    prx_sp_props[0].property.value = 1;
    prx_sp_props[1].type = prx_so_broadcast;
    prx_sp_props[1].property.value = 1;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 7);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, int32, &prx_sp_valid, family);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, int32, &prx_sp_valid, sock_type);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, int32, &prx_sp_valid, proto_type);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, uint32, &prx_sp_valid, flags);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, uint64, &prx_sp_valid, timeout);
    STRICT_EXPECTED_CALL(io_encode_object(IGNORED_PTR_ARG, "address", false, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t)) 
        .CopyOutArgumentBuffer_object(&k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL(io_encode_int32(IGNORED_PTR_ARG, "family", (int32_t)prx_sp_valid.address.un.family))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);
    STRICT_EXPECTED_CALL(io_encode_array(&k_ctx_valid, "options", k_size_valid, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_array(&k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL(io_encode_object(IGNORED_PTR_ARG, NULL, false, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_object(&k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 2);
    STRICT_EXPECTED_CALL(io_encode_int32(&k_ctx_valid, "type", prx_so_debug))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL(io_encode_uint64(&k_ctx_valid, "property", 1))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);
    STRICT_EXPECTED_CALL(io_encode_object(IGNORED_PTR_ARG, NULL, false, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_object(&k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 2);
    STRICT_EXPECTED_CALL(io_encode_int32(&k_ctx_valid, "type", prx_so_broadcast))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL(io_encode_uint64(&k_ctx_valid, "property", 1))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_prx_socket_properties(&k_ctx_valid, &prx_sp_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_writing);
}

// 
// Test io_decode_prx_socket_properties happy path 
// 
TEST_FUNCTION(io_decode_prx_socket_properties__success_1)
{
    static const int32_t k_af_valid = prx_address_family_unspec;
    static const size_t k_size_valid = 0;
    static io_codec_ctx_t k_ctx_valid;
    prx_socket_properties_t prx_sp_valid;
    int32_t result;

    memset(&prx_sp_valid, 0xff, sizeof(prx_sp_valid));

    // arrange
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 7);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, int32, &prx_sp_valid, family);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, int32, &prx_sp_valid, sock_type);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, int32, &prx_sp_valid, proto_type);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, uint32, &prx_sp_valid, flags);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, uint64, &prx_sp_valid, timeout);
    STRICT_EXPECTED_CALL(io_decode_object(IGNORED_PTR_ARG, "address", IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_is_null(&k_false, sizeof(k_false))
        .CopyOutArgumentBuffer_object(&k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok); 
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL(io_decode_int32(IGNORED_PTR_ARG, "family", (int32_t*)&prx_sp_valid.address.un.family))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_val(&k_af_valid, sizeof(k_af_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);
    STRICT_EXPECTED_CALL(io_decode_array(&k_ctx_valid, "options", IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_array(&k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_size(&k_size_valid, sizeof(k_size_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_decode_prx_socket_properties(&k_ctx_valid, &prx_sp_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_decode_prx_socket_properties happy path 
// 
TEST_FUNCTION(io_decode_prx_socket_properties__success_2)
{
    static const int32_t k_af_valid = prx_address_family_unspec;
    static const size_t k_size_valid = 2;
    static io_codec_ctx_t k_ctx_valid;
    static int32_t prx_so_valid1 = prx_so_unknown + 1;
    static int32_t prx_so_valid2 = prx_so_unknown + 2;
    prx_socket_properties_t prx_sp_valid;
    int32_t result;

    memset(&prx_sp_valid, 0xff, sizeof(prx_sp_valid));

    // arrange
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 7);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, int32, &prx_sp_valid, family);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, int32, &prx_sp_valid, sock_type);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, int32, &prx_sp_valid, proto_type);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, uint32, &prx_sp_valid, flags);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, uint64, &prx_sp_valid, timeout);
    STRICT_EXPECTED_CALL(io_decode_object(IGNORED_PTR_ARG, "address", IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_is_null(&k_false, sizeof(k_false))
        .CopyOutArgumentBuffer_object(&k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL(io_decode_int32(IGNORED_PTR_ARG, "family", (int32_t*)&prx_sp_valid.address.un.family))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_val(&k_af_valid, sizeof(k_af_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);
    STRICT_EXPECTED_CALL(io_decode_array(&k_ctx_valid, "options", IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_array(&k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_size(&k_size_valid, sizeof(k_size_valid))
        .SetReturn(er_ok);

    STRICT_EXPECTED_CALL(h_realloc(2 * sizeof(prx_property_t), NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);

    STRICT_EXPECTED_CALL(io_decode_object(IGNORED_PTR_ARG, NULL, NULL, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_object(&k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 2);
    STRICT_EXPECTED_CALL(io_decode_int32(&k_ctx_valid, "type", IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_val(&prx_so_valid1, sizeof(prx_so_valid1))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_decode_uint64(&k_ctx_valid, "property", IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .IgnoreArgument(3)
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);
    STRICT_EXPECTED_CALL(io_decode_object(IGNORED_PTR_ARG, NULL, NULL, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_object(&k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 2);
    STRICT_EXPECTED_CALL(io_decode_int32(&k_ctx_valid, "type", IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_val(&prx_so_valid2, sizeof(prx_so_valid2))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_decode_uint64(&k_ctx_valid, "property", IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .IgnoreArgument(3)
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_decode_prx_socket_properties(&k_ctx_valid, &prx_sp_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_decode_prx_socket_properties unhappy path 
// 
TEST_FUNCTION(io_decode_prx_socket_properties__neg)
{
    static const int32_t k_af_valid = prx_address_family_unspec;
    static const size_t k_size_valid = 2;
    static io_codec_ctx_t k_ctx_valid;
    static int32_t prx_so_valid1 = prx_so_unknown + 1;
    static int32_t prx_so_valid2 = prx_so_unknown + 2;
    prx_socket_properties_t prx_sp_valid;
    int32_t result;

    memset(&prx_sp_valid, 0xff, sizeof(prx_sp_valid));

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 7);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, int32, &prx_sp_valid, family);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, int32, &prx_sp_valid, sock_type);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, int32, &prx_sp_valid, proto_type);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, uint32, &prx_sp_valid, flags);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, uint64, &prx_sp_valid, timeout);
    STRICT_EXPECTED_CALL(io_decode_object(IGNORED_PTR_ARG, "address", IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_is_null(&k_false, sizeof(k_false))
        .CopyOutArgumentBuffer_object(&k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL(io_decode_int32(IGNORED_PTR_ARG, "family", (int32_t*)&prx_sp_valid.address.un.family))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_val(&k_af_valid, sizeof(k_af_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);
    STRICT_EXPECTED_CALL(io_decode_array(&k_ctx_valid, "options", IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_array(&k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_size(&k_size_valid, sizeof(k_size_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL(h_realloc(2 * sizeof(prx_property_t), NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(io_decode_object(IGNORED_PTR_ARG, NULL, NULL, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_object(&k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 2);
    STRICT_EXPECTED_CALL(io_decode_int32(&k_ctx_valid, "type", IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_val(&prx_so_valid1, sizeof(prx_so_valid1))
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL(io_decode_uint64(&k_ctx_valid, "property", IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .IgnoreArgument(3)
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);
    STRICT_EXPECTED_CALL(io_decode_object(IGNORED_PTR_ARG, NULL, NULL, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_object(&k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 2);
    STRICT_EXPECTED_CALL(io_decode_int32(&k_ctx_valid, "type", IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .CopyOutArgumentBuffer_val(&prx_so_valid2, sizeof(prx_so_valid2))
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL(io_decode_uint64(&k_ctx_valid, "property", IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .IgnoreArgument(3)
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_prx_socket_properties(&k_ctx_valid, &prx_sp_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, 
        er_invalid_format, er_out_of_memory,  er_out_of_memory,  er_out_of_memory,  er_out_of_memory,
        er_out_of_memory,  er_out_of_memory,  er_invalid_format, er_out_of_memory,  er_out_of_memory, 
        er_out_of_memory,  er_out_of_memory,  er_out_of_memory,  er_invalid_format, er_out_of_memory, 
        er_out_of_memory,  er_out_of_memory,  er_out_of_memory,  er_invalid_format, er_out_of_memory);
}

// 
// Test io_encode_prx_property happy path 
// 
TEST_FUNCTION(io_encode_prx_property__success)
{
    static io_codec_ctx_t k_ctx_valid;
    prx_property_t prx_so_val_valid;
    int32_t result;

    memset(&prx_so_val_valid, 0, sizeof(prx_so_val_valid));
    prx_so_val_valid.type = prx_so_debug;

    // arrange
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 2);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, int32, &prx_so_val_valid, type);
    STRICT_EXPECTED_CALL(io_encode_uint64(&k_ctx_valid, "property", 0))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_encode_prx_property(&k_ctx_valid, &prx_so_val_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_encode_prx_property unhappy path 
// 
TEST_FUNCTION(io_encode_prx_property__neg)
{
    static io_codec_ctx_t k_ctx_valid;
    prx_property_t prx_so_val_valid;
    int32_t result;

    memset(&prx_so_val_valid, 0, sizeof(prx_so_val_valid));
    prx_so_val_valid.type = prx_so_debug;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 2);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, int32, &prx_so_val_valid, type);
    STRICT_EXPECTED_CALL(io_encode_uint64(&k_ctx_valid, "property", 0))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_prx_property(&k_ctx_valid, &prx_so_val_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_writing);
}

// 
// Test io_decode_prx_property happy path 
// 
TEST_FUNCTION(io_decode_prx_property__success)
{
    static io_codec_ctx_t k_ctx_valid;
    prx_property_t prx_so_val_valid;
    int32_t result;

    memset(&prx_so_val_valid, 0xff, sizeof(prx_so_val_valid));
    prx_so_val_valid.type = prx_so_unknown + 1;

    // arrange
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 2);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, int32, &prx_so_val_valid, type);
    STRICT_EXPECTED_CALL(io_decode_uint64(&k_ctx_valid, "property", IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .IgnoreArgument(3)
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_decode_prx_property(&k_ctx_valid, &prx_so_val_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_decode_prx_property unhappy path 
// 
TEST_FUNCTION(io_decode_prx_property__neg)
{
    static io_codec_ctx_t k_ctx_valid;
    prx_property_t prx_so_val_valid;
    int32_t result;

    memset(&prx_so_val_valid, 0xff, sizeof(prx_so_val_valid));
    prx_so_val_valid.type = prx_so_unknown + 1;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 2);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, int32, &prx_so_val_valid, type);
    STRICT_EXPECTED_CALL(io_decode_uint64(&k_ctx_valid, "property", IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(1, &k_ctx_valid, sizeof(io_codec_ctx_t))
        .IgnoreArgument(3)
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_prx_property(&k_ctx_valid, &prx_so_val_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_invalid_format, er_out_of_memory);
}


//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

