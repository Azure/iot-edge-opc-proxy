// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST io_transport
#include "util_ut.h"

//
// 1. Required mocks
//
#include "prx_ns.h"
#include "prx_buffer.h"
#include "prx_log.h"
#include "prx_sched.h"
#include "io_proto.h"

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

#ifdef io_connection_send

//
//Test io_connection_send happy path 
// 
TEST_FUNCTION(io_connection_send__success)
{
    static const io_connection_t* k_connection_valid;
    static const io_message_t* k_message_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = io_connection_send(k_connection_valid, k_message_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test io_connection_send passing as connection argument an invalid io_connection_t* value 
// 
TEST_FUNCTION(io_connection_send__arg_connection_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_connection_send();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_connection_send passing as message argument an invalid io_message_t* value 
// 
TEST_FUNCTION(io_connection_send__arg_message_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_connection_send();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_connection_send unhappy path 
// 
TEST_FUNCTION(io_connection_send__neg)
{
    static const io_connection_t* k_connection_valid;
    static const io_message_t* k_message_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_connection_send(k_connection_valid, k_message_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_connection_send

#ifdef io_connection_close

//
//Test io_connection_close happy path 
// 
TEST_FUNCTION(io_connection_close__success)
{
    static const io_connection_t* k_connection_valid;
    void result;

    // arrange 
    // ... 

    // act 
    result = io_connection_close(k_connection_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ... 
}

// 
// Test io_connection_close passing as connection argument an invalid io_connection_t* value 
// 
TEST_FUNCTION(io_connection_close__arg_connection_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_connection_close();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_connection_close unhappy path 
// 
TEST_FUNCTION(io_connection_close__neg)
{
    static const io_connection_t* k_connection_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_connection_close(k_connection_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // io_connection_close

#ifdef io_connection_free

//
//Test io_connection_free happy path 
// 
TEST_FUNCTION(io_connection_free__success)
{
    static const io_connection_t* k_connection_valid;
    void result;

    // arrange 
    // ... 

    // act 
    result = io_connection_free(k_connection_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ... 
}

// 
// Test io_connection_free passing as connection argument an invalid io_connection_t* value 
// 
TEST_FUNCTION(io_connection_free__arg_connection_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_connection_free();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_connection_free unhappy path 
// 
TEST_FUNCTION(io_connection_free__neg)
{
    static const io_connection_t* k_connection_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_connection_free(k_connection_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // io_connection_free

#ifdef io_transport_create

//
//Test io_transport_create happy path 
// 
TEST_FUNCTION(io_transport_create__success)
{
    static const io_transport_t* k_transport_valid;
    static const prx_ns_entry_t* k_entry_valid;
    static const io_connection_cb_t k_cb_valid;
    static const void* k_context_valid;
    static const prx_scheduler_t* k_scheduler_valid;
    static const io_connection_t** k_connection_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = io_transport_create(k_transport_valid, k_entry_valid, k_cb_valid, k_context_valid, k_scheduler_valid, k_connection_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test io_transport_create passing as transport argument an invalid io_transport_t* value 
// 
TEST_FUNCTION(io_transport_create__arg_transport_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_transport_create();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_transport_create passing as entry argument an invalid prx_ns_entry_t* value 
// 
TEST_FUNCTION(io_transport_create__arg_entry_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_transport_create();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_transport_create passing as cb argument an invalid io_connection_cb_t value 
// 
TEST_FUNCTION(io_transport_create__arg_cb_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_transport_create();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_transport_create passing as context argument an invalid void* value 
// 
TEST_FUNCTION(io_transport_create__arg_context_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_transport_create();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_transport_create passing as scheduler argument an invalid prx_scheduler_t* value 
// 
TEST_FUNCTION(io_transport_create__arg_scheduler_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_transport_create();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_transport_create passing as connection argument an invalid io_connection_t** value 
// 
TEST_FUNCTION(io_transport_create__arg_connection_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_transport_create();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_transport_create unhappy path 
// 
TEST_FUNCTION(io_transport_create__neg)
{
    static const io_transport_t* k_transport_valid;
    static const prx_ns_entry_t* k_entry_valid;
    static const io_connection_cb_t k_cb_valid;
    static const void* k_context_valid;
    static const prx_scheduler_t* k_scheduler_valid;
    static const io_connection_t** k_connection_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_transport_create(k_transport_valid, k_entry_valid, k_cb_valid, k_context_valid, k_scheduler_valid, k_connection_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_transport_create



//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

