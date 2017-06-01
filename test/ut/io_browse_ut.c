// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST io_browse
#include "util_ut.h"

//
// 1. Required mocks
//
#include "prx_browse.h"
#include "io_codec.h"
#include "io_ref.h"

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

#ifdef io_encode_browse_request 
// 
// Test io_encode_browse_request happy path 
// 
TEST_FUNCTION(io_encode_browse_request__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const io_browse_request_t* k_prx_br_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = io_encode_browse_request(k_ctx_valid, k_prx_br_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test io_encode_browse_request passing as ctx argument an invalid io_codec_ctx_t* value 
// 
TEST_FUNCTION(io_encode_browse_request__arg_ctx_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_encode_browse_request();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_encode_browse_request passing as prx_br argument an invalid const io_browse_request_t* value 
// 
TEST_FUNCTION(io_encode_browse_request__arg_prx_br_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_encode_browse_request();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_encode_browse_request unhappy path 
// 
TEST_FUNCTION(io_encode_browse_request__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const io_browse_request_t* k_prx_br_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_browse_request(k_ctx_valid, k_prx_br_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_encode_browse_request 

#ifdef io_decode_browse_request 
// 
// Test io_decode_browse_request happy path 
// 
TEST_FUNCTION(io_decode_browse_request__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const io_browse_request_t* k_prx_br_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = io_decode_browse_request(k_ctx_valid, k_prx_br_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test io_decode_browse_request passing as ctx argument an invalid io_codec_ctx_t* value 
// 
TEST_FUNCTION(io_decode_browse_request__arg_ctx_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_decode_browse_request();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_decode_browse_request passing as prx_br argument an invalid io_browse_request_t* value 
// 
TEST_FUNCTION(io_decode_browse_request__arg_prx_br_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_decode_browse_request();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_decode_browse_request unhappy path 
// 
TEST_FUNCTION(io_decode_browse_request__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const io_browse_request_t* k_prx_br_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_browse_request(k_ctx_valid, k_prx_br_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_decode_browse_request 

#ifdef io_encode_browse_response 
// 
// Test io_encode_browse_response happy path 
// 
TEST_FUNCTION(io_encode_browse_response__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const io_browse_response_t* k_prx_br_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = io_encode_browse_response(k_ctx_valid, k_prx_br_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test io_encode_browse_response passing as ctx argument an invalid io_codec_ctx_t* value 
// 
TEST_FUNCTION(io_encode_browse_response__arg_ctx_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_encode_browse_response();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_encode_browse_response passing as prx_br argument an invalid const io_browse_response_t* value 
// 
TEST_FUNCTION(io_encode_browse_response__arg_prx_br_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_encode_browse_response();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_encode_browse_response unhappy path 
// 
TEST_FUNCTION(io_encode_browse_response__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const const io_browse_response_t* k_prx_br_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_encode_browse_response(k_ctx_valid, k_prx_br_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_encode_browse_response 


#ifdef io_decode_browse_response 
// 
// Test io_decode_browse_response happy path 
// 
TEST_FUNCTION(io_decode_browse_response__success)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const io_browse_response_t* k_prx_br_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = io_decode_browse_response(k_ctx_valid, k_prx_br_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test io_decode_browse_response passing as ctx argument an invalid io_codec_ctx_t* value 
// 
TEST_FUNCTION(io_decode_browse_response__arg_ctx_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_decode_browse_response();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_decode_browse_response passing as prx_br argument an invalid io_browse_response_t* value 
// 
TEST_FUNCTION(io_decode_browse_response__arg_prx_br_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_decode_browse_response();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_decode_browse_response unhappy path 
// 
TEST_FUNCTION(io_decode_browse_response__neg)
{
    static const io_codec_ctx_t* k_ctx_valid;
    static const io_browse_response_t* k_prx_br_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_decode_browse_response(k_ctx_valid, k_prx_br_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_decode_browse_response 

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

