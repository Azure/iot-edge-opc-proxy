// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST io_ws
#include "util_ut.h"

//
// 1. Required mocks
//
#include "prx_types.h"
#include "io_stream.h"
#include "io_token.h"
#include "prx_sched.h"
#include "io_url.h"

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

#ifdef io_ws_connection_create

//
//Test io_ws_connection_create happy path
//
TEST_FUNCTION(io_ws_connection_create__success)
{
    static const io_url_t* k_address_valid;
    static const const char* k_user_header_key_valid;
    static const const char* k_pwd_header_key_valid;
    static const prx_scheduler_t* k_scheduler_valid;
    static const io_stream_handler_t k_receiver_cb_valid;
    static const void* k_receiver_ctx_valid;
    static const io_ws_connection_t** k_connection_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_ws_connection_create(k_address_valid, k_user_header_key_valid, k_pwd_header_key_valid, k_scheduler_valid, k_receiver_cb_valid, k_receiver_ctx_valid, k_connection_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_ws_connection_create passing as address argument an invalid io_url_t* value
//
TEST_FUNCTION(io_ws_connection_create__arg_address_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_ws_connection_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_ws_connection_create passing as user_header_key argument an invalid const char* value
//
TEST_FUNCTION(io_ws_connection_create__arg_user_header_key_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_ws_connection_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_ws_connection_create passing as pwd_header_key argument an invalid const char* value
//
TEST_FUNCTION(io_ws_connection_create__arg_pwd_header_key_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_ws_connection_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_ws_connection_create passing as scheduler argument an invalid prx_scheduler_t* value
//
TEST_FUNCTION(io_ws_connection_create__arg_scheduler_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_ws_connection_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_ws_connection_create passing as receiver_cb argument an invalid io_stream_handler_t value
//
TEST_FUNCTION(io_ws_connection_create__arg_receiver_cb_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_ws_connection_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_ws_connection_create passing as receiver_ctx argument an invalid void* value
//
TEST_FUNCTION(io_ws_connection_create__arg_receiver_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_ws_connection_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_ws_connection_create passing as connection argument an invalid io_ws_connection_t** value
//
TEST_FUNCTION(io_ws_connection_create__arg_connection_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_ws_connection_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_ws_connection_create unhappy path
//
TEST_FUNCTION(io_ws_connection_create__neg)
{
    static const io_url_t* k_address_valid;
    static const const char* k_user_header_key_valid;
    static const const char* k_pwd_header_key_valid;
    static const prx_scheduler_t* k_scheduler_valid;
    static const io_stream_handler_t k_receiver_cb_valid;
    static const void* k_receiver_ctx_valid;
    static const io_ws_connection_t** k_connection_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_ws_connection_create(k_address_valid, k_user_header_key_valid, k_pwd_header_key_valid, k_scheduler_valid, k_receiver_cb_valid, k_receiver_ctx_valid, k_connection_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_ws_connection_create;

#ifdef io_ws_connection_connect

//
//Test io_ws_connection_connect happy path
//
TEST_FUNCTION(io_ws_connection_connect__success)
{
    static const io_ws_connection_t* k_connection_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_ws_connection_connect(k_connection_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_ws_connection_connect passing as connection argument an invalid io_ws_connection_t* value
//
TEST_FUNCTION(io_ws_connection_connect__arg_connection_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_ws_connection_connect();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_ws_connection_connect unhappy path
//
TEST_FUNCTION(io_ws_connection_connect__neg)
{
    static const io_ws_connection_t* k_connection_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_ws_connection_connect(k_connection_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_ws_connection_connect;

#ifdef io_ws_connection_send

//
//Test io_ws_connection_send happy path
//
TEST_FUNCTION(io_ws_connection_send__success)
{
    static const io_ws_connection_t* k_connection_valid;
    static const io_stream_handler_t k_sender_cb_valid;
    static const void* k_sender_ctx_valid;
    static const io_ws_connection_send_complete_t k_complete_cb_valid;
    static const void* k_complete_ctx_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_ws_connection_send(k_connection_valid, k_sender_cb_valid, k_sender_ctx_valid, k_complete_cb_valid, k_complete_ctx_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_ws_connection_send passing as connection argument an invalid io_ws_connection_t* value
//
TEST_FUNCTION(io_ws_connection_send__arg_connection_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_ws_connection_send();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_ws_connection_send passing as sender_cb argument an invalid io_stream_handler_t value
//
TEST_FUNCTION(io_ws_connection_send__arg_sender_cb_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_ws_connection_send();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_ws_connection_send passing as sender_ctx argument an invalid void* value
//
TEST_FUNCTION(io_ws_connection_send__arg_sender_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_ws_connection_send();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_ws_connection_send passing as complete_cb argument an invalid io_ws_connection_send_complete_t value
//
TEST_FUNCTION(io_ws_connection_send__arg_complete_cb_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_ws_connection_send();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_ws_connection_send passing as complete_ctx argument an invalid void* value
//
TEST_FUNCTION(io_ws_connection_send__arg_complete_ctx_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_ws_connection_send();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_ws_connection_send unhappy path
//
TEST_FUNCTION(io_ws_connection_send__neg)
{
    static const io_ws_connection_t* k_connection_valid;
    static const io_stream_handler_t k_sender_cb_valid;
    static const void* k_sender_ctx_valid;
    static const io_ws_connection_send_complete_t k_complete_cb_valid;
    static const void* k_complete_ctx_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_ws_connection_send(k_connection_valid, k_sender_cb_valid, k_sender_ctx_valid, k_complete_cb_valid, k_complete_ctx_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_ws_connection_send;

#ifdef io_ws_connection_close

//
//Test io_ws_connection_close happy path
//
TEST_FUNCTION(io_ws_connection_close__success)
{
    static const io_ws_connection_t* k_connection_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_ws_connection_close(k_connection_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_ws_connection_close passing as connection argument an invalid io_ws_connection_t* value
//
TEST_FUNCTION(io_ws_connection_close__arg_connection_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_ws_connection_close();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_ws_connection_close unhappy path
//
TEST_FUNCTION(io_ws_connection_close__neg)
{
    static const io_ws_connection_t* k_connection_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_ws_connection_close(k_connection_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_ws_connection_close;




//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

