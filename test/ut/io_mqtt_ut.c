// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST io_mqtt
#include "util_ut.h"

//
// 1. Required mocks
//
#include "io_url.h"
#include "io_token.h"
#include "prx_sched.h"

MOCKABLE_FUNCTION(, const char*, trusted_certs);

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

#ifdef io_mqtt_connection_create

//
//Test io_mqtt_connection_create happy path
//
TEST_FUNCTION(io_mqtt_connection_create__success)
{
    static const io_url_t* k_address_valid;
    static const const char* k_client_id_valid;
    static const prx_scheduler_t* k_scheduler_valid;
    static const io_mqtt_connection_t** k_created_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_mqtt_connection_create(k_address_valid, k_client_id_valid, k_scheduler_valid, k_created_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_mqtt_connection_create passing as address argument an invalid io_url_t* value
//
TEST_FUNCTION(io_mqtt_connection_create__arg_address_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_mqtt_connection_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_mqtt_connection_create passing as client_id argument an invalid const char* value
//
TEST_FUNCTION(io_mqtt_connection_create__arg_client_id_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_mqtt_connection_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_mqtt_connection_create passing as scheduler argument an invalid prx_scheduler_t* value
//
TEST_FUNCTION(io_mqtt_connection_create__arg_scheduler_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_mqtt_connection_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_mqtt_connection_create passing as created argument an invalid io_mqtt_connection_t** value
//
TEST_FUNCTION(io_mqtt_connection_create__arg_created_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_mqtt_connection_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_mqtt_connection_create unhappy path
//
TEST_FUNCTION(io_mqtt_connection_create__neg)
{
    static const io_url_t* k_address_valid;
    static const const char* k_client_id_valid;
    static const prx_scheduler_t* k_scheduler_valid;
    static const io_mqtt_connection_t** k_created_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_mqtt_connection_create(k_address_valid, k_client_id_valid, k_scheduler_valid, k_created_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_mqtt_connection_create;

#ifdef io_mqtt_connection_subscribe

//
//Test io_mqtt_connection_subscribe happy path
//
TEST_FUNCTION(io_mqtt_connection_subscribe__success)
{
    static const io_mqtt_connection_t* k_connection_valid;
    static const const char* k_uri_valid;
    static const io_mqtt_subscription_receiver_t k_cb_valid;
    static const void* k_context_valid;
    static const io_mqtt_subscription_t** k_subscription_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_mqtt_connection_subscribe(k_connection_valid, k_uri_valid, k_cb_valid, k_context_valid, k_subscription_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_mqtt_connection_subscribe passing as connection argument an invalid io_mqtt_connection_t* value
//
TEST_FUNCTION(io_mqtt_connection_subscribe__arg_connection_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_mqtt_connection_subscribe();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_mqtt_connection_subscribe passing as uri argument an invalid const char* value
//
TEST_FUNCTION(io_mqtt_connection_subscribe__arg_uri_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_mqtt_connection_subscribe();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_mqtt_connection_subscribe passing as cb argument an invalid io_mqtt_subscription_receiver_t value
//
TEST_FUNCTION(io_mqtt_connection_subscribe__arg_cb_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_mqtt_connection_subscribe();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_mqtt_connection_subscribe passing as context argument an invalid void* value
//
TEST_FUNCTION(io_mqtt_connection_subscribe__arg_context_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_mqtt_connection_subscribe();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_mqtt_connection_subscribe passing as subscription argument an invalid io_mqtt_subscription_t** value
//
TEST_FUNCTION(io_mqtt_connection_subscribe__arg_subscription_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_mqtt_connection_subscribe();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_mqtt_connection_subscribe unhappy path
//
TEST_FUNCTION(io_mqtt_connection_subscribe__neg)
{
    static const io_mqtt_connection_t* k_connection_valid;
    static const const char* k_uri_valid;
    static const io_mqtt_subscription_receiver_t k_cb_valid;
    static const void* k_context_valid;
    static const io_mqtt_subscription_t** k_subscription_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_mqtt_connection_subscribe(k_connection_valid, k_uri_valid, k_cb_valid, k_context_valid, k_subscription_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_mqtt_connection_subscribe;

#ifdef io_mqtt_connection_connect

//
//Test io_mqtt_connection_connect happy path
//
TEST_FUNCTION(io_mqtt_connection_connect__success)
{
    static const io_mqtt_connection_t* k_connection_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_mqtt_connection_connect(k_connection_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_mqtt_connection_connect passing as connection argument an invalid io_mqtt_connection_t* value
//
TEST_FUNCTION(io_mqtt_connection_connect__arg_connection_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_mqtt_connection_connect();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_mqtt_connection_connect unhappy path
//
TEST_FUNCTION(io_mqtt_connection_connect__neg)
{
    static const io_mqtt_connection_t* k_connection_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_mqtt_connection_connect(k_connection_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_mqtt_connection_connect;

#ifdef io_mqtt_properties_create

//
//Test io_mqtt_properties_create happy path
//
TEST_FUNCTION(io_mqtt_properties_create__success)
{
    static const io_mqtt_properties_t** k_properties_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_mqtt_properties_create(k_properties_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_mqtt_properties_create passing as properties argument an invalid io_mqtt_properties_t** value
//
TEST_FUNCTION(io_mqtt_properties_create__arg_properties_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_mqtt_properties_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_mqtt_properties_create unhappy path
//
TEST_FUNCTION(io_mqtt_properties_create__neg)
{
    static const io_mqtt_properties_t** k_properties_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_mqtt_properties_create(k_properties_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_mqtt_properties_create;

#ifdef io_mqtt_properties_add

//
//Test io_mqtt_properties_add happy path
//
TEST_FUNCTION(io_mqtt_properties_add__success)
{
    static const io_mqtt_properties_t* k_properties_valid;
    static const const char* k_key_valid;
    static const const char* k_value_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_mqtt_properties_add(k_properties_valid, k_key_valid, k_value_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_mqtt_properties_add passing as properties argument an invalid io_mqtt_properties_t* value
//
TEST_FUNCTION(io_mqtt_properties_add__arg_properties_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_mqtt_properties_add();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_mqtt_properties_add passing as key argument an invalid const char* value
//
TEST_FUNCTION(io_mqtt_properties_add__arg_key_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_mqtt_properties_add();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_mqtt_properties_add passing as value argument an invalid const char* value
//
TEST_FUNCTION(io_mqtt_properties_add__arg_value_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_mqtt_properties_add();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_mqtt_properties_add unhappy path
//
TEST_FUNCTION(io_mqtt_properties_add__neg)
{
    static const io_mqtt_properties_t* k_properties_valid;
    static const const char* k_key_valid;
    static const const char* k_value_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_mqtt_properties_add(k_properties_valid, k_key_valid, k_value_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_mqtt_properties_add;

#ifdef io_mqtt_properties_get

//
//Test io_mqtt_properties_get happy path
//
TEST_FUNCTION(io_mqtt_properties_get__success)
{
    static const io_mqtt_properties_t* k_properties_valid;
    static const const char* k_key_valid;
    static const char* k_value_buf_valid;
    static const size_t k_value_len_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_mqtt_properties_get(k_properties_valid, k_key_valid, k_value_buf_valid, k_value_len_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_mqtt_properties_get passing as properties argument an invalid io_mqtt_properties_t* value
//
TEST_FUNCTION(io_mqtt_properties_get__arg_properties_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_mqtt_properties_get();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_mqtt_properties_get passing as key argument an invalid const char* value
//
TEST_FUNCTION(io_mqtt_properties_get__arg_key_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_mqtt_properties_get();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_mqtt_properties_get passing as value_buf argument an invalid char* value
//
TEST_FUNCTION(io_mqtt_properties_get__arg_value_buf_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_mqtt_properties_get();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_mqtt_properties_get passing as value_len argument an invalid size_t value
//
TEST_FUNCTION(io_mqtt_properties_get__arg_value_len_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_mqtt_properties_get();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_mqtt_properties_get unhappy path
//
TEST_FUNCTION(io_mqtt_properties_get__neg)
{
    static const io_mqtt_properties_t* k_properties_valid;
    static const const char* k_key_valid;
    static const char* k_value_buf_valid;
    static const size_t k_value_len_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_mqtt_properties_get(k_properties_valid, k_key_valid, k_value_buf_valid, k_value_len_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_mqtt_properties_get;

#ifdef io_mqtt_properties_free

//
//Test io_mqtt_properties_free happy path
//
TEST_FUNCTION(io_mqtt_properties_free__success)
{
    static const io_mqtt_properties_t* k_properties_valid;
    void result;

    // arrange
    // ...

    // act
    result = io_mqtt_properties_free(k_properties_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ...
}

//
// Test io_mqtt_properties_free passing as properties argument an invalid io_mqtt_properties_t* value
//
TEST_FUNCTION(io_mqtt_properties_free__arg_properties_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_mqtt_properties_free();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_mqtt_properties_free unhappy path
//
TEST_FUNCTION(io_mqtt_properties_free__neg)
{
    static const io_mqtt_properties_t* k_properties_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_mqtt_properties_free(k_properties_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // io_mqtt_properties_free;

#ifdef io_mqtt_connection_publish

//
//Test io_mqtt_connection_publish happy path
//
TEST_FUNCTION(io_mqtt_connection_publish__success)
{
    static const io_mqtt_connection_t* k_connection_valid;
    static const const char* k_uri_valid;
    static const io_mqtt_properties_t* k_properties_valid;
    static const const uint8_t* k_body_valid;
    static const size_t k_body_len_valid;
    static const io_mqtt_publish_complete_t k_cb_valid;
    static const void* k_context_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_mqtt_connection_publish(k_connection_valid, k_uri_valid, k_properties_valid, k_body_valid, k_body_len_valid, k_cb_valid, k_context_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_mqtt_connection_publish passing as connection argument an invalid io_mqtt_connection_t* value
//
TEST_FUNCTION(io_mqtt_connection_publish__arg_connection_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_mqtt_connection_publish();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_mqtt_connection_publish passing as uri argument an invalid const char* value
//
TEST_FUNCTION(io_mqtt_connection_publish__arg_uri_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_mqtt_connection_publish();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_mqtt_connection_publish passing as properties argument an invalid io_mqtt_properties_t* value
//
TEST_FUNCTION(io_mqtt_connection_publish__arg_properties_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_mqtt_connection_publish();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_mqtt_connection_publish passing as body argument an invalid const uint8_t* value
//
TEST_FUNCTION(io_mqtt_connection_publish__arg_body_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_mqtt_connection_publish();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_mqtt_connection_publish passing as body_len argument an invalid size_t value
//
TEST_FUNCTION(io_mqtt_connection_publish__arg_body_len_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_mqtt_connection_publish();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_mqtt_connection_publish passing as cb argument an invalid io_mqtt_publish_complete_t value
//
TEST_FUNCTION(io_mqtt_connection_publish__arg_cb_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_mqtt_connection_publish();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_mqtt_connection_publish passing as context argument an invalid void* value
//
TEST_FUNCTION(io_mqtt_connection_publish__arg_context_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_mqtt_connection_publish();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_mqtt_connection_publish unhappy path
//
TEST_FUNCTION(io_mqtt_connection_publish__neg)
{
    static const io_mqtt_connection_t* k_connection_valid;
    static const const char* k_uri_valid;
    static const io_mqtt_properties_t* k_properties_valid;
    static const const uint8_t* k_body_valid;
    static const size_t k_body_len_valid;
    static const io_mqtt_publish_complete_t k_cb_valid;
    static const void* k_context_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_mqtt_connection_publish(k_connection_valid, k_uri_valid, k_properties_valid, k_body_valid, k_body_len_valid, k_cb_valid, k_context_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_mqtt_connection_publish;

#ifdef io_mqtt_subscription_release

//
//Test io_mqtt_subscription_release happy path
//
TEST_FUNCTION(io_mqtt_subscription_release__success)
{
    static const io_mqtt_subscription_t* k_subscription_valid;
    void result;

    // arrange
    // ...

    // act
    result = io_mqtt_subscription_release(k_subscription_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ...
}

//
// Test io_mqtt_subscription_release passing as subscription argument an invalid io_mqtt_subscription_t* value
//
TEST_FUNCTION(io_mqtt_subscription_release__arg_subscription_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_mqtt_subscription_release();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_mqtt_subscription_release unhappy path
//
TEST_FUNCTION(io_mqtt_subscription_release__neg)
{
    static const io_mqtt_subscription_t* k_subscription_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_mqtt_subscription_release(k_subscription_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // io_mqtt_subscription_release;

#ifdef io_mqtt_connection_close

//
//Test io_mqtt_connection_close happy path
//
TEST_FUNCTION(io_mqtt_connection_close__success)
{
    static const io_mqtt_connection_t* k_connection_valid;
    void result;

    // arrange
    // ...

    // act
    result = io_mqtt_connection_close(k_connection_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ...
}

//
// Test io_mqtt_connection_close passing as connection argument an invalid io_mqtt_connection_t* value
//
TEST_FUNCTION(io_mqtt_connection_close__arg_connection_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_mqtt_connection_close();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_mqtt_connection_close unhappy path
//
TEST_FUNCTION(io_mqtt_connection_close__neg)
{
    static const io_mqtt_connection_t* k_connection_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_mqtt_connection_close(k_connection_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // io_mqtt_connection_close;


//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

