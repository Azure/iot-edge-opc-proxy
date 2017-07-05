// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define ENABLE_GLOBAL

#define UNIT_UNDER_TEST io_proto
#include "util_ut.h"


//
// 1. Required mocks
//
#include "prx_types.h"
#include "io_ref.h"
#include "io_codec.h"
#include "prx_buffer.h"

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
// Test io_encode_ping_request happy path 
// 
TEST_FUNCTION(io_encode_ping_request__success)
{
    static io_codec_ctx_t k_ctx_valid;
    io_ping_request_t request_valid;
    int32_t result;

    memset(&request_valid, 0xff, sizeof(request_valid));

    // arrange
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL_TO_ENCODE_OBJECT(&k_ctx_valid, prx_socket_address, &request_valid, address);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_encode_ping_request(&k_ctx_valid, &request_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_encode_ping_request unhappy path 
// 
TEST_FUNCTION(io_encode_ping_request__neg)
{
    static io_codec_ctx_t k_ctx_valid;
    io_ping_request_t request_valid;
    int32_t result;

    memset(&request_valid, 0xff, sizeof(request_valid));

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL_TO_ENCODE_OBJECT(&k_ctx_valid, prx_socket_address, &request_valid, address);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_ping_request(&k_ctx_valid, &request_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_writing);
}

// 
// Test io_decode_ping_request happy path 
// 
TEST_FUNCTION(io_decode_ping_request__success)
{
    static io_codec_ctx_t k_ctx_valid;
    io_ping_request_t request_valid;
    int32_t result;

    memset(&request_valid, 0xff, sizeof(request_valid));

    // arrange
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL_TO_DECODE_OBJECT(&k_ctx_valid, prx_socket_address, &request_valid, address);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_decode_ping_request(&k_ctx_valid, &request_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_decode_ping_request unhappy path 
// 
TEST_FUNCTION(io_decode_ping_request__neg)
{
    static io_codec_ctx_t k_ctx_valid;
    io_ping_request_t request_valid;
    int32_t result;

    memset(&request_valid, 0xff, sizeof(request_valid));

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL_TO_DECODE_OBJECT(&k_ctx_valid, prx_socket_address, &request_valid, address);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_ping_request(&k_ctx_valid, &request_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_invalid_format, er_out_of_memory);
}

// 
// Test io_encode_ping_response happy path 
// 
TEST_FUNCTION(io_encode_ping_response__success)
{
    static io_codec_ctx_t k_ctx_valid;
    io_ping_response_t response_valid;
    int32_t result;

    memset(&response_valid, 0xff, sizeof(response_valid));

    // arrange
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 3);
    STRICT_EXPECTED_CALL_TO_ENCODE_OBJECT(&k_ctx_valid, prx_socket_address, &response_valid, address);
    STRICT_EXPECTED_CALL(io_encode_bin(&k_ctx_valid, "physical_address", response_valid.physical_address, sizeof(response_valid.physical_address)))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, uint32, &response_valid, time_ms);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_encode_ping_response(&k_ctx_valid, &response_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_encode_ping_response unhappy path 
// 
TEST_FUNCTION(io_encode_ping_response__neg)
{
    static io_codec_ctx_t k_ctx_valid;
    io_ping_response_t response_valid;
    int32_t result;

    memset(&response_valid, 0xff, sizeof(response_valid));

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 3);
    STRICT_EXPECTED_CALL_TO_ENCODE_OBJECT(&k_ctx_valid, prx_socket_address, &response_valid, address);
    STRICT_EXPECTED_CALL(io_encode_bin(&k_ctx_valid, "physical_address", response_valid.physical_address, sizeof(response_valid.physical_address)))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, uint32, &response_valid, time_ms);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_ping_response(&k_ctx_valid, &response_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_writing);
}

// 
// Test io_decode_ping_response happy path 
// 
TEST_FUNCTION(io_decode_ping_response__success)
{
    static io_codec_ctx_t k_ctx_valid;
    io_ping_response_t response_valid;
    const size_t k_bin_fixed_valid = sizeof(response_valid.physical_address);
    int32_t result;

    // arrange
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 3);
    STRICT_EXPECTED_CALL_TO_DECODE_OBJECT(&k_ctx_valid, prx_socket_address, &response_valid, address);
    STRICT_EXPECTED_CALL(io_decode_bin_fixed(&k_ctx_valid, "physical_address", response_valid.physical_address, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(4, &k_bin_fixed_valid, sizeof(k_bin_fixed_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, uint32, &response_valid, time_ms);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_decode_ping_response(&k_ctx_valid, &response_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_decode_ping_response unhappy path 
// 
TEST_FUNCTION(io_decode_ping_response__neg)
{
    static io_codec_ctx_t k_ctx_valid;
    io_ping_response_t response_valid;
    const size_t k_bin_fixed_valid = sizeof(response_valid.physical_address);
    int32_t result;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 3);
    STRICT_EXPECTED_CALL_TO_DECODE_OBJECT(&k_ctx_valid, prx_socket_address, &response_valid, address);
    STRICT_EXPECTED_CALL(io_decode_bin_fixed(&k_ctx_valid, "physical_address", response_valid.physical_address, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(4, &k_bin_fixed_valid, sizeof(k_bin_fixed_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, uint32, &response_valid, time_ms);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_ping_response(&k_ctx_valid, &response_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_invalid_format, er_out_of_memory);
}

// 
// Test io_encode_link_request happy path 
// 
TEST_FUNCTION(io_encode_link_request__success)
{
    static io_codec_ctx_t k_ctx_valid;
    io_link_request_t request_valid;
    int32_t result;

    memset(&request_valid, 0xef, sizeof(request_valid));

    // arrange
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 2);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, uint8, &request_valid, version);
    STRICT_EXPECTED_CALL_TO_ENCODE_OBJECT(&k_ctx_valid, prx_socket_properties, &request_valid, props);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_encode_link_request(&k_ctx_valid, &request_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_encode_link_request unhappy path 
// 
TEST_FUNCTION(io_encode_link_request__neg)
{
    static io_codec_ctx_t k_ctx_valid;
    io_link_request_t request_valid;
    int32_t result;

    memset(&request_valid, 0xef, sizeof(request_valid));

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 2);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, uint8, &request_valid, version);
    STRICT_EXPECTED_CALL_TO_ENCODE_OBJECT(&k_ctx_valid, prx_socket_properties, &request_valid, props);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_link_request(&k_ctx_valid, &request_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_writing);
}

// 
// Test io_decode_link_request happy path 
// 
TEST_FUNCTION(io_decode_link_request__success)
{
    static io_codec_ctx_t k_ctx_valid;
    io_link_request_t request_valid;
    int32_t result;

    // arrange
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 2);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, uint8, &request_valid, version);
    STRICT_EXPECTED_CALL_TO_DECODE_OBJECT(&k_ctx_valid, prx_socket_properties, &request_valid, props);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_decode_link_request(&k_ctx_valid, &request_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_decode_link_request unhappy path 
// 
TEST_FUNCTION(io_decode_link_request__neg)
{
    static io_codec_ctx_t k_ctx_valid;
    io_link_request_t request_valid;
    int32_t result;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 2);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, uint8, &request_valid, version);
    STRICT_EXPECTED_CALL_TO_DECODE_OBJECT(&k_ctx_valid, prx_socket_properties, &request_valid, props);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_link_request(&k_ctx_valid, &request_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_invalid_format, er_out_of_memory);
}

// 
// Test io_encode_link_response happy path 
// 
TEST_FUNCTION(io_encode_link_response__success)
{
    static io_codec_ctx_t k_ctx_valid;
    io_link_response_t response_valid;
    int32_t result;

    // arrange
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 3);
    STRICT_EXPECTED_CALL_TO_ENCODE_OBJECT(&k_ctx_valid, ref, &response_valid, link_id);
    STRICT_EXPECTED_CALL_TO_ENCODE_OBJECT(&k_ctx_valid, prx_socket_address, &response_valid, local_address);
    STRICT_EXPECTED_CALL_TO_ENCODE_OBJECT(&k_ctx_valid, prx_socket_address, &response_valid, peer_address);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_encode_link_response(&k_ctx_valid, &response_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_encode_link_response unhappy path 
// 
TEST_FUNCTION(io_encode_link_response__neg)
{
    static io_codec_ctx_t k_ctx_valid;
    io_link_response_t response_valid;
    int32_t result;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 3);
    STRICT_EXPECTED_CALL_TO_ENCODE_OBJECT(&k_ctx_valid, ref, &response_valid, link_id);
    STRICT_EXPECTED_CALL_TO_ENCODE_OBJECT(&k_ctx_valid, prx_socket_address, &response_valid, local_address);
    STRICT_EXPECTED_CALL_TO_ENCODE_OBJECT(&k_ctx_valid, prx_socket_address, &response_valid, peer_address);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_link_response(&k_ctx_valid, &response_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_writing);
}

// 
// Test io_decode_link_response happy path 
// 
TEST_FUNCTION(io_decode_link_response__success)
{
    static io_codec_ctx_t k_ctx_valid;
    io_link_response_t response_valid;
    int32_t result;

    // arrange
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 3);
    STRICT_EXPECTED_CALL_TO_DECODE_OBJECT(&k_ctx_valid, ref, &response_valid, link_id);
    STRICT_EXPECTED_CALL_TO_DECODE_OBJECT(&k_ctx_valid, prx_socket_address, &response_valid, local_address);
    STRICT_EXPECTED_CALL_TO_DECODE_OBJECT(&k_ctx_valid, prx_socket_address, &response_valid, peer_address);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_decode_link_response(&k_ctx_valid, &response_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_decode_link_response unhappy path 
// 
TEST_FUNCTION(io_decode_link_response__neg)
{
    static io_codec_ctx_t k_ctx_valid;
    io_link_response_t response_valid;
    int32_t result;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 3);
    STRICT_EXPECTED_CALL_TO_DECODE_OBJECT(&k_ctx_valid, ref, &response_valid, link_id);
    STRICT_EXPECTED_CALL_TO_DECODE_OBJECT(&k_ctx_valid, prx_socket_address, &response_valid, local_address);
    STRICT_EXPECTED_CALL_TO_DECODE_OBJECT(&k_ctx_valid, prx_socket_address, &response_valid, peer_address);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_link_response(&k_ctx_valid, &response_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_invalid_format, er_out_of_memory);
}

// 
// Test io_encode_setopt_request happy path 
// 
TEST_FUNCTION(io_encode_setopt_request__success)
{
    static io_codec_ctx_t k_ctx_valid;
    io_setopt_request_t request_valid;
    int32_t result;

    // arrange
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL_TO_ENCODE_OBJECT(&k_ctx_valid, prx_property, &request_valid, so_val);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_encode_setopt_request(&k_ctx_valid, &request_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_encode_setopt_request unhappy path 
// 
TEST_FUNCTION(io_encode_setopt_request__neg)
{
    static io_codec_ctx_t k_ctx_valid;
    io_setopt_request_t request_valid;
    int32_t result;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL_TO_ENCODE_OBJECT(&k_ctx_valid, prx_property, &request_valid, so_val);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_setopt_request(&k_ctx_valid, &request_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_writing);
}

// 
// Test io_decode_setopt_request happy path 
// 
TEST_FUNCTION(io_decode_setopt_request__success)
{
    static io_codec_ctx_t k_ctx_valid;
    io_setopt_request_t request_valid;
    int32_t result;

    // arrange
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL_TO_DECODE_OBJECT(&k_ctx_valid, prx_property, &request_valid, so_val);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_decode_setopt_request(&k_ctx_valid, &request_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_decode_setopt_request unhappy path 
// 
TEST_FUNCTION(io_decode_setopt_request__neg)
{
    static io_codec_ctx_t k_ctx_valid;
    io_setopt_request_t request_valid;
    int32_t result;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL_TO_DECODE_OBJECT(&k_ctx_valid, prx_property, &request_valid, so_val);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_setopt_request(&k_ctx_valid, &request_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_invalid_format, er_out_of_memory);
}

// 
// Test io_encode_getopt_request happy path 
// 
TEST_FUNCTION(io_encode_getopt_request__success)
{
    static io_codec_ctx_t k_ctx_valid;
    io_getopt_request_t request_valid;
    int32_t result;

    request_valid.so_opt = (prx_socket_option_t)0xffff;

    // arrange
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, int64, &request_valid, so_opt);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_encode_getopt_request(&k_ctx_valid, &request_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_encode_getopt_request unhappy path 
// 
TEST_FUNCTION(io_encode_getopt_request__neg)
{
    static io_codec_ctx_t k_ctx_valid;
    io_getopt_request_t request_valid;
    int32_t result;

    request_valid.so_opt = (prx_socket_option_t)0xffff;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, int64, &request_valid, so_opt);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_getopt_request(&k_ctx_valid, &request_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_writing);
}

// 
// Test io_decode_getopt_request happy path 
// 
TEST_FUNCTION(io_decode_getopt_request__success)
{
    static io_codec_ctx_t k_ctx_valid;
    io_getopt_request_t request_valid;
    int32_t result;

    // arrange
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, int32, &request_valid, so_opt);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_decode_getopt_request(&k_ctx_valid, &request_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_decode_getopt_request unhappy path 
// 
TEST_FUNCTION(io_decode_getopt_request__neg)
{
    static io_codec_ctx_t k_ctx_valid;
    io_getopt_request_t request_valid;
    int32_t result;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, int32, &request_valid, so_opt);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_getopt_request(&k_ctx_valid, &request_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_invalid_format, er_out_of_memory);
}

// 
// Test io_encode_getopt_response happy path 
// 
TEST_FUNCTION(io_encode_getopt_response__success)
{
    static io_codec_ctx_t k_ctx_valid;
    io_getopt_response_t response_valid;
    int32_t result;

    // arrange
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL_TO_ENCODE_OBJECT(&k_ctx_valid, prx_property, &response_valid, so_val);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_encode_getopt_response(&k_ctx_valid, &response_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_encode_getopt_response unhappy path 
// 
TEST_FUNCTION(io_encode_getopt_response__neg)
{
    static io_codec_ctx_t k_ctx_valid;
    io_getopt_response_t response_valid;
    int32_t result;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL_TO_ENCODE_OBJECT(&k_ctx_valid, prx_property, &response_valid, so_val);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_getopt_response(&k_ctx_valid, &response_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_writing);
}

// 
// Test io_decode_getopt_response happy path 
// 
TEST_FUNCTION(io_decode_getopt_response__success)
{
    static io_codec_ctx_t k_ctx_valid;
    io_getopt_response_t response_valid;
    int32_t result;

    // arrange
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL_TO_DECODE_OBJECT(&k_ctx_valid, prx_property, &response_valid, so_val);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_decode_getopt_response(&k_ctx_valid, &response_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_decode_getopt_response unhappy path 
// 
TEST_FUNCTION(io_decode_getopt_response__neg)
{
    static io_codec_ctx_t k_ctx_valid;
    io_getopt_response_t response_valid;
    int32_t result;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 1);
    STRICT_EXPECTED_CALL_TO_DECODE_OBJECT(&k_ctx_valid, prx_property, &response_valid, so_val);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_getopt_response(&k_ctx_valid, &response_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_invalid_format, er_out_of_memory);
}

// 
// Test io_encode_open_request happy path 
// 
TEST_FUNCTION(io_encode_open_request__success)
{
    static const char* k_cs_valid = "HostName=test.Test;DeviceId=something;SharedAccessToken=bla";
    static io_codec_ctx_t k_ctx_valid;
    io_open_request_t request_valid;
    int32_t result;

    request_valid.connection_string = k_cs_valid;
    request_valid.polled = false;
    request_valid.encoding = 5;
    request_valid.max_recv = 800;
    request_valid.type = 0;

    // arrange
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 6);
    STRICT_EXPECTED_CALL_TO_ENCODE_OBJECT(&k_ctx_valid, ref, &request_valid, stream_id);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, int32, &request_valid, encoding);
    STRICT_EXPECTED_CALL(io_encode_string(&k_ctx_valid, "connection-string", k_cs_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, int32, &request_valid, type);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, bool, &request_valid, polled);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, uint32, &request_valid, max_recv);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_encode_open_request(&k_ctx_valid, &request_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_encode_open_request unhappy path 
// 
TEST_FUNCTION(io_encode_open_request__neg)
{
    static const char* k_cs_valid = "HostName=test.Test;DeviceId=something;SharedAccessToken=bla";
    static io_codec_ctx_t k_ctx_valid;
    io_open_request_t request_valid;
    int32_t result;

    request_valid.connection_string = k_cs_valid;
    request_valid.polled = false;
    request_valid.encoding = 5;
    request_valid.max_recv = 800;
    request_valid.type = 0;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 6);
    STRICT_EXPECTED_CALL_TO_ENCODE_OBJECT(&k_ctx_valid, ref, &request_valid, stream_id);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, int32, &request_valid, encoding);
    STRICT_EXPECTED_CALL(io_encode_string(&k_ctx_valid, "connection-string", k_cs_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, int32, &request_valid, type);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, bool, &request_valid, polled);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, uint32, &request_valid, max_recv);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_open_request(&k_ctx_valid, &request_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, 
        er_writing, er_writing, er_writing, er_writing, er_ok, er_writing);
}

// 
// Test io_decode_open_request happy path 
// 
TEST_FUNCTION(io_decode_open_request__success)
{
    static const char* k_cs_valid = "HostName=test.Test;DeviceId=something;SharedAccessToken=bla";
    static io_codec_ctx_t k_ctx_valid;
    io_open_request_t request_valid;
    int32_t result;

    // arrange
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 6);
    STRICT_EXPECTED_CALL_TO_DECODE_OBJECT(&k_ctx_valid, ref, &request_valid, stream_id);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, int32, &request_valid, encoding);
    STRICT_EXPECTED_CALL(io_decode_string_default(&k_ctx_valid, "connection-string", IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_string(&k_cs_valid, sizeof(k_cs_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, int32, &request_valid, type);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, bool, &request_valid, polled);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, uint32, &request_valid, max_recv);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_decode_open_request(&k_ctx_valid, &request_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_decode_open_request unhappy path 
// 
TEST_FUNCTION(io_decode_open_request__neg)
{
    static const char* k_cs_valid = "HostName=test.Test;DeviceId=something;SharedAccessToken=bla";
    static io_codec_ctx_t k_ctx_valid;
    io_open_request_t request_valid;
    int32_t result;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 6);
    STRICT_EXPECTED_CALL_TO_DECODE_OBJECT(&k_ctx_valid, ref, &request_valid, stream_id);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, int32, &request_valid, encoding);
    STRICT_EXPECTED_CALL(io_decode_string_default(&k_ctx_valid, "connection-string", IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_string(&k_cs_valid, sizeof(k_cs_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, int32, &request_valid, type);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, bool, &request_valid, polled);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, uint32, &request_valid, max_recv);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_open_request(&k_ctx_valid, &request_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, 
        er_invalid_format, er_out_of_memory, er_out_of_memory, er_out_of_memory, 
        er_ok, er_out_of_memory);
}

// 
// Test io_encode_poll_message happy path 
// 
TEST_FUNCTION(io_encode_poll_message__success)
{
    static io_codec_ctx_t k_ctx_valid;
    io_poll_message_t message_valid;
    int32_t result;

    message_valid.timeout = 524;
    message_valid.sequence_number = 33333333;

    // arrange
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 2);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, uint64, &message_valid, sequence_number);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, uint64, &message_valid, timeout);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_encode_poll_message(&k_ctx_valid, &message_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_encode_poll_message unhappy path 
// 
TEST_FUNCTION(io_encode_poll_message__neg)
{
    static io_codec_ctx_t k_ctx_valid;
    io_poll_message_t message_valid;
    int32_t result;

    message_valid.timeout = 123456;
    message_valid.sequence_number = 5555;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 2);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, uint64, &message_valid, sequence_number);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, uint64, &message_valid, timeout);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_poll_message(&k_ctx_valid, &message_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_writing, er_writing, er_writing, er_writing, er_ok, er_writing);
}

// 
// Test io_decode_poll_message happy path 
// 
TEST_FUNCTION(io_decode_poll_message__success)
{
    static const char* k_cs_valid = "HostName=test.Test;DeviceId=something;SharedAccessToken=bla";
    static io_codec_ctx_t k_ctx_valid;
    io_poll_message_t message_valid;
    int32_t result;

    // arrange
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 2);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, uint64, &message_valid, sequence_number);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, uint64, &message_valid, timeout);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_decode_poll_message(&k_ctx_valid, &message_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_decode_poll_message unhappy path 
// 
TEST_FUNCTION(io_decode_poll_message__neg)
{
    static const char* k_cs_valid = "HostName=test.Test;DeviceId=something;SharedAccessToken=bla";
    static io_codec_ctx_t k_ctx_valid;
    io_poll_message_t message_valid;
    int32_t result;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 2);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, uint64, &message_valid, sequence_number);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, uint64, &message_valid, timeout);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_poll_message(&k_ctx_valid, &message_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result,
        er_invalid_format, er_out_of_memory, er_out_of_memory, er_out_of_memory, er_ok, er_out_of_memory);
}

// 
// Test io_encode_data_message happy path 
// 
TEST_FUNCTION(io_encode_data_message__success)
{
    static io_codec_ctx_t k_ctx_valid;
    io_data_message_t message_valid;
    int32_t result;

    message_valid.sequence_number = 555;
    message_valid.buffer = (uint8_t*)UT_MEM;
    message_valid.buffer_length = 10;
    message_valid.control_buffer = NULL;
    message_valid.control_buffer_length = 0;

    // arrange
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 4);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, uint64, &message_valid, sequence_number);
    STRICT_EXPECTED_CALL_TO_ENCODE_OBJECT(&k_ctx_valid, prx_socket_address, &message_valid, source_address);
    STRICT_EXPECTED_CALL(io_encode_bin(&k_ctx_valid, "buffer", UT_MEM, 10))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_encode_bin(&k_ctx_valid, "control_buffer", NULL, 0))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_encode_data_message(&k_ctx_valid, &message_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_encode_data_message unhappy path 
// 
TEST_FUNCTION(io_encode_data_message__neg)
{
    static io_codec_ctx_t k_ctx_valid;
    io_data_message_t message_valid;
    int32_t result;

    message_valid.sequence_number = 2;
    message_valid.buffer = (uint8_t*)UT_MEM;
    message_valid.buffer_length = 10;
    message_valid.control_buffer = NULL;
    message_valid.control_buffer_length = 0;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 4);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, uint64, &message_valid, sequence_number);
    STRICT_EXPECTED_CALL_TO_ENCODE_OBJECT(&k_ctx_valid, prx_socket_address, &message_valid, source_address);
    STRICT_EXPECTED_CALL(io_encode_bin(&k_ctx_valid, "buffer", UT_MEM, 10))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL(io_encode_bin(&k_ctx_valid, "control_buffer", NULL, 0))
        .SetReturn(er_ok)
        .SetFailReturn(er_writing);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_data_message(&k_ctx_valid, &message_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_writing);
}

// 
// Test io_decode_data happy path 
// 
TEST_FUNCTION(io_decode_data__success)
{
    static const size_t k_read_valid = 10;
    static void* k_value_valid = UT_MEM;
    static io_codec_ctx_t k_ctx_valid;
    io_data_message_t message_valid;
    int32_t result;

    message_valid.buffer = (uint8_t*)UT_MEM;
    message_valid.buffer_length = 0;

    // arrange
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 4);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, uint64, &message_valid, sequence_number);
    STRICT_EXPECTED_CALL_TO_DECODE_OBJECT(&k_ctx_valid, prx_socket_address, &message_valid, source_address);
    STRICT_EXPECTED_CALL(io_decode_bin_default(&k_ctx_valid, "buffer", IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_size(&k_read_valid, sizeof(k_read_valid))
        .CopyOutArgumentBuffer_value(&k_value_valid, sizeof(k_value_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_decode_bin_default(&k_ctx_valid, "control_buffer", IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_size(&k_read_valid, sizeof(k_read_valid))
        .CopyOutArgumentBuffer_value(&k_value_valid, sizeof(k_value_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_decode_data_message(&k_ctx_valid, &message_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_decode_data unhappy path 
// 
TEST_FUNCTION(io_decode_data__neg)
{
    static const size_t k_read_valid = 10;
    static void* k_value_valid = UT_MEM;
    static io_codec_ctx_t k_ctx_valid;
    io_data_message_t message_valid;
    int32_t result;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 4);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, uint64, &message_valid, sequence_number);
    STRICT_EXPECTED_CALL_TO_DECODE_OBJECT(&k_ctx_valid, prx_socket_address, &message_valid, source_address);
    STRICT_EXPECTED_CALL(io_decode_bin_default(&k_ctx_valid, "buffer", IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_size(&k_read_valid, sizeof(k_read_valid))
        .CopyOutArgumentBuffer_value(&k_value_valid, sizeof(k_value_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL(io_decode_bin_default(&k_ctx_valid, "control_buffer", IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_size(&k_read_valid, sizeof(k_read_valid))
        .CopyOutArgumentBuffer_value(&k_value_valid, sizeof(k_value_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    message_valid.buffer = (uint8_t*)UT_MEM;
    message_valid.buffer_length = 0;
    result = io_decode_data_message(&k_ctx_valid, &message_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_invalid_format, er_out_of_memory);
}

// 
// Test io_encode_close_response happy path 
// 
TEST_FUNCTION(io_encode_close_response__success)
{
    static io_codec_ctx_t k_ctx_valid;
    io_close_response_t response_valid;
    int32_t result;

    memset(&response_valid, 0xef, sizeof(response_valid));

    // arrange
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 4);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, uint64, &response_valid, time_open);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, uint64, &response_valid, bytes_received);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, uint64, &response_valid, bytes_sent);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, int32, &response_valid, error_code);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_encode_close_response(&k_ctx_valid, &response_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_encode_close_response unhappy path 
// 
TEST_FUNCTION(io_encode_close_response__neg)
{
    static io_codec_ctx_t k_ctx_valid;
    io_close_response_t response_valid;
    int32_t result;

    memset(&response_valid, 0xef, sizeof(response_valid));

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(&k_ctx_valid, 4);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, uint64, &response_valid, time_open);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, uint64, &response_valid, bytes_received);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, uint64, &response_valid, bytes_sent);
    STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(&k_ctx_valid, int32, &response_valid, error_code);
    STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_close_response(&k_ctx_valid, &response_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_writing);
}

// 
// Test io_decode_close_response happy path 
// 
TEST_FUNCTION(io_decode_close_response__success)
{
    static io_codec_ctx_t k_ctx_valid;
    io_close_response_t response_valid;
    int32_t result;

    // arrange
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 4);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, uint64, &response_valid, time_open);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, uint64, &response_valid, bytes_received);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, uint64, &response_valid, bytes_sent);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, int32, &response_valid, error_code);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act 
    result = io_decode_close_response(&k_ctx_valid, &response_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_decode_close_response unhappy path 
// 
TEST_FUNCTION(io_decode_close_response__neg)
{
    static io_codec_ctx_t k_ctx_valid;
    io_close_response_t response_valid;
    int32_t result;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(&k_ctx_valid, 4);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, uint64, &response_valid, time_open);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, uint64, &response_valid, bytes_received);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, uint64, &response_valid, bytes_sent);
    STRICT_EXPECTED_CALL_TO_DECODE_VALUE(&k_ctx_valid, int32, &response_valid, error_code);
    STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(&k_ctx_valid);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_close_response(&k_ctx_valid, &response_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_invalid_format, er_out_of_memory);
}

#if 0

// 
// Test io_message_type_as_string happy path 
// 
TEST_FUNCTION(io_message_type_as_string__success)
{
    uint32_t k_type_valid;
    int32_t result;

    // arrange
    // ... 

    // act 
    result = io_message_type_as_string(k_type_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_message_type_as_string passing as type argument an invalid uint32_t value 
// 
TEST_FUNCTION(io_message_type_as_string__arg_type_invalid)
{
    // ... 
    int32_t result;

    // arrange
    // ... 

    // act 
    result = io_message_type_as_string();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_message_type_as_string unhappy path 
// 
TEST_FUNCTION(io_message_type_as_string__neg)
{
    uint32_t k_type_valid;
    int32_t result;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_message_type_as_string(k_type_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_writing);
}

#endif

#ifdef io_message_factory_create

//
//Test io_message_factory_create happy path 
// 
TEST_FUNCTION(io_message_factory_create__success)
{
    static const size_t k_pool_size_valid;
    static const size_t k_low_watermark_valid;
    static const size_t k_high_watermark_valid;
    static const prx_buffer_pool_cb_t k_cb_valid;
    static const void* k_context_valid;
    static const io_message_factory_t** k_factory_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = io_message_factory_create(k_pool_size_valid, k_low_watermark_valid, k_high_watermark_valid, k_cb_valid, k_context_valid, k_factory_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test io_message_factory_create passing as pool_size argument an invalid size_t value 
// 
TEST_FUNCTION(io_message_factory_create__arg_pool_size_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_message_factory_create();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_message_factory_create passing as low_watermark argument an invalid size_t value 
// 
TEST_FUNCTION(io_message_factory_create__arg_low_watermark_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_message_factory_create();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_message_factory_create passing as high_watermark argument an invalid size_t value 
// 
TEST_FUNCTION(io_message_factory_create__arg_high_watermark_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_message_factory_create();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_message_factory_create passing as cb argument an invalid prx_buffer_pool_cb_t value 
// 
TEST_FUNCTION(io_message_factory_create__arg_cb_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_message_factory_create();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_message_factory_create passing as context argument an invalid void* value 
// 
TEST_FUNCTION(io_message_factory_create__arg_context_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_message_factory_create();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_message_factory_create passing as factory argument an invalid io_message_factory_t** value 
// 
TEST_FUNCTION(io_message_factory_create__arg_factory_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_message_factory_create();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_message_factory_create unhappy path 
// 
TEST_FUNCTION(io_message_factory_create__neg)
{
    static const size_t k_pool_size_valid;
    static const size_t k_low_watermark_valid;
    static const size_t k_high_watermark_valid;
    static const prx_buffer_pool_cb_t k_cb_valid;
    static const void* k_context_valid;
    static const io_message_factory_t** k_factory_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_message_factory_create(k_pool_size_valid, k_low_watermark_valid, k_high_watermark_valid, k_cb_valid, k_context_valid, k_factory_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_message_factory_create;

#ifdef io_message_factory_free

//
//Test io_message_factory_free happy path 
// 
TEST_FUNCTION(io_message_factory_free__success)
{
    static const io_message_factory_t* k_factory_valid;
    void result;

    // arrange 
    // ... 

    // act 
    result = io_message_factory_free(k_factory_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ... 
}

// 
// Test io_message_factory_free passing as factory argument an invalid io_message_factory_t* value 
// 
TEST_FUNCTION(io_message_factory_free__arg_factory_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_message_factory_free();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_message_factory_free unhappy path 
// 
TEST_FUNCTION(io_message_factory_free__neg)
{
    static const io_message_factory_t* k_factory_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_message_factory_free(k_factory_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // io_message_factory_free;

#ifdef io_message_create

//
//Test io_message_create happy path 
// 
TEST_FUNCTION(io_message_create__success)
{
    static const io_message_factory_t* k_factory_valid;
    static const int32_t k_type_valid;
    static const io_ref_t* k_source_valid;
    static const io_ref_t* k_target_valid;
    static const io_message_t** k_message_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = io_message_create(k_factory_valid, k_type_valid, k_source_valid, k_target_valid, k_message_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test io_message_create passing as factory argument an invalid io_message_factory_t* value 
// 
TEST_FUNCTION(io_message_create__arg_factory_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_message_create();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_message_create passing as type argument an invalid int32_t value 
// 
TEST_FUNCTION(io_message_create__arg_type_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_message_create();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_message_create passing as source argument an invalid io_ref_t* value 
// 
TEST_FUNCTION(io_message_create__arg_source_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_message_create();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_message_create passing as target argument an invalid io_ref_t* value 
// 
TEST_FUNCTION(io_message_create__arg_target_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_message_create();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_message_create passing as message argument an invalid io_message_t** value 
// 
TEST_FUNCTION(io_message_create__arg_message_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_message_create();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_message_create unhappy path 
// 
TEST_FUNCTION(io_message_create__neg)
{
    static const io_message_factory_t* k_factory_valid;
    static const int32_t k_type_valid;
    static const io_ref_t* k_source_valid;
    static const io_ref_t* k_target_valid;
    static const io_message_t** k_message_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_message_create(k_factory_valid, k_type_valid, k_source_valid, k_target_valid, k_message_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_message_create;

#ifdef io_message_clone

//
//Test io_message_clone happy path 
// 
TEST_FUNCTION(io_message_clone__success)
{
    static const io_message_t* k_original_valid;
    static const io_message_t** k_created_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = io_message_clone(k_original_valid, k_created_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test io_message_clone passing as original argument an invalid io_message_t* value 
// 
TEST_FUNCTION(io_message_clone__arg_original_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_message_clone();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_message_clone passing as created argument an invalid io_message_t** value 
// 
TEST_FUNCTION(io_message_clone__arg_created_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_message_clone();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_message_clone unhappy path 
// 
TEST_FUNCTION(io_message_clone__neg)
{
    static const io_message_t* k_original_valid;
    static const io_message_t** k_created_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_message_clone(k_original_valid, k_created_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_message_clone;

#ifdef io_encode_message

//
//Test io_encode_message happy path 
// 
TEST_FUNCTION(io_encode_message__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const io_message_t* k_message_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = io_encode_message(k_ctx_valid, k_message_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test io_encode_message passing as ctx argument an invalid io_codec_ctx_t* value 
// 
TEST_FUNCTION(io_encode_message__arg_ctx_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_encode_message();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_encode_message passing as message argument an invalid io_message_t* value 
// 
TEST_FUNCTION(io_encode_message__arg_message_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_encode_message();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_encode_message unhappy path 
// 
TEST_FUNCTION(io_encode_message__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const io_message_t* k_message_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_message(k_ctx_valid, k_message_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_encode_message;

#ifdef io_message_create_empty

//
//Test io_message_create_empty happy path 
// 
TEST_FUNCTION(io_message_create_empty__success)
{
    static const io_message_factory_t* k_factory_valid;
    static const io_message_t** k_message_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = io_message_create_empty(k_factory_valid, k_message_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test io_message_create_empty passing as factory argument an invalid io_message_factory_t* value 
// 
TEST_FUNCTION(io_message_create_empty__arg_factory_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_message_create_empty();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_message_create_empty passing as message argument an invalid io_message_t** value 
// 
TEST_FUNCTION(io_message_create_empty__arg_message_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_message_create_empty();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_message_create_empty unhappy path 
// 
TEST_FUNCTION(io_message_create_empty__neg)
{
    static const io_message_factory_t* k_factory_valid;
    static const io_message_t** k_message_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_message_create_empty(k_factory_valid, k_message_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_message_create_empty;

#ifdef io_decode_message

//
//Test io_decode_message happy path 
// 
TEST_FUNCTION(io_decode_message__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const io_message_t* k_message_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = io_decode_message(k_ctx_valid, k_message_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test io_decode_message passing as ctx argument an invalid io_codec_ctx_t* value 
// 
TEST_FUNCTION(io_decode_message__arg_ctx_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_decode_message();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_decode_message passing as message argument an invalid io_message_t* value 
// 
TEST_FUNCTION(io_decode_message__arg_message_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_decode_message();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_decode_message unhappy path 
// 
TEST_FUNCTION(io_decode_message__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const io_message_t* k_message_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_message(k_ctx_valid, k_message_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_decode_message;

#ifdef io_message_as_response

//
//Test io_message_as_response happy path 
// 
TEST_FUNCTION(io_message_as_response__success)
{
    static const io_message_t* k_message_valid;
    void result;

    // arrange 
    // ... 

    // act 
    result = io_message_as_response(k_message_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ... 
}

// 
// Test io_message_as_response passing as message argument an invalid io_message_t* value 
// 
TEST_FUNCTION(io_message_as_response__arg_message_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_message_as_response();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_message_as_response unhappy path 
// 
TEST_FUNCTION(io_message_as_response__neg)
{
    static const io_message_t* k_message_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_message_as_response(k_message_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // io_message_as_response;

#ifdef io_message_allocate_buffer

//
//Test io_message_allocate_buffer happy path 
// 
TEST_FUNCTION(io_message_allocate_buffer__success)
{
    static const io_message_t* k_message_valid;
    static const size_t k_size_valid;
    static const void** k_mem_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = io_message_allocate_buffer(k_message_valid, k_size_valid, k_mem_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test io_message_allocate_buffer passing as message argument an invalid io_message_t* value 
// 
TEST_FUNCTION(io_message_allocate_buffer__arg_message_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_message_allocate_buffer();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_message_allocate_buffer passing as size argument an invalid size_t value 
// 
TEST_FUNCTION(io_message_allocate_buffer__arg_size_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_message_allocate_buffer();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_message_allocate_buffer passing as mem argument an invalid void** value 
// 
TEST_FUNCTION(io_message_allocate_buffer__arg_mem_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_message_allocate_buffer();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_message_allocate_buffer unhappy path 
// 
TEST_FUNCTION(io_message_allocate_buffer__neg)
{
    static const io_message_t* k_message_valid;
    static const size_t k_size_valid;
    static const void** k_mem_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_message_allocate_buffer(k_message_valid, k_size_valid, k_mem_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_message_allocate_buffer;

#ifdef io_message_release

//
//Test io_message_release happy path 
// 
TEST_FUNCTION(io_message_release__success)
{
    static const io_message_t* k_message_valid;
    void result;

    // arrange 
    // ... 

    // act 
    result = io_message_release(k_message_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ... 
}

// 
// Test io_message_release passing as message argument an invalid io_message_t* value 
// 
TEST_FUNCTION(io_message_release__arg_message_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_message_release();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_message_release unhappy path 
// 
TEST_FUNCTION(io_message_release__neg)
{
    static const io_message_t* k_message_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_message_release(k_message_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // io_message_release;

#ifdef io_message_type_as_string

//
//Test io_message_type_as_string happy path 
// 
TEST_FUNCTION(io_message_type_as_string__success)
{
    static const uint32_t k_type_valid;
    const char* result;

    // arrange 
    // ... 

    // act 
    result = io_message_type_as_string(k_type_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(const char*, er_ok, result);
    // ... 
}

// 
// Test io_message_type_as_string passing as type argument an invalid uint32_t value 
// 
TEST_FUNCTION(io_message_type_as_string__arg_type_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_message_type_as_string();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_message_type_as_string unhappy path 
// 
TEST_FUNCTION(io_message_type_as_string__neg)
{
    static const uint32_t k_type_valid;
    const char* result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_message_type_as_string(k_type_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(const char*, result, er_ok);
}

#endif // io_message_type_as_string;


//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

