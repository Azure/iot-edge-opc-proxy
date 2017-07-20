// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST prx_sched
#include "util_ut.h"

//
// 1. Required mocks
//
#include "util_misc.h"


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


#ifdef prx_scheduler_create

//
//Test prx_scheduler_create happy path
//
TEST_FUNCTION(prx_scheduler_create__success)
{
    static const prx_scheduler_t* k_parent_valid;
    static const prx_scheduler_t** k_scheduler_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = prx_scheduler_create(k_parent_valid, k_scheduler_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test prx_scheduler_create passing as parent argument an invalid prx_scheduler_t* value
//
TEST_FUNCTION(prx_scheduler_create__arg_parent_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_scheduler_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_scheduler_create passing as scheduler argument an invalid prx_scheduler_t** value
//
TEST_FUNCTION(prx_scheduler_create__arg_scheduler_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_scheduler_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_scheduler_create unhappy path
//
TEST_FUNCTION(prx_scheduler_create__neg)
{
    static const prx_scheduler_t* k_parent_valid;
    static const prx_scheduler_t** k_scheduler_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_scheduler_create(k_parent_valid, k_scheduler_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_scheduler_create;

#ifdef prx_scheduler_queue

//
//Test prx_scheduler_queue happy path
//
TEST_FUNCTION(prx_scheduler_queue__success)
{
    static const prx_scheduler_t* k_scheduler_valid;
    static const const char* k_name_valid;
    static const prx_task_t k_task_valid;
    static const void* k_context_valid;
    static const uint32_t k_delay_valid;
    static const const char* k_func_valid;
    static const int k_line_valid;
    intptr_t result;

    // arrange
    // ...

    // act
    result = prx_scheduler_queue(k_scheduler_valid, k_name_valid, k_task_valid, k_context_valid, k_delay_valid, k_func_valid, k_line_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(intptr_t, er_ok, result);
    // ...
}

//
// Test prx_scheduler_queue passing as scheduler argument an invalid prx_scheduler_t* value
//
TEST_FUNCTION(prx_scheduler_queue__arg_scheduler_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_scheduler_queue();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_scheduler_queue passing as name argument an invalid const char* value
//
TEST_FUNCTION(prx_scheduler_queue__arg_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_scheduler_queue();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_scheduler_queue passing as task argument an invalid prx_task_t value
//
TEST_FUNCTION(prx_scheduler_queue__arg_task_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_scheduler_queue();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_scheduler_queue passing as context argument an invalid void* value
//
TEST_FUNCTION(prx_scheduler_queue__arg_context_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_scheduler_queue();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_scheduler_queue passing as delay argument an invalid uint32_t value
//
TEST_FUNCTION(prx_scheduler_queue__arg_delay_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_scheduler_queue();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_scheduler_queue passing as func argument an invalid const char* value
//
TEST_FUNCTION(prx_scheduler_queue__arg_func_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_scheduler_queue();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_scheduler_queue passing as line argument an invalid int value
//
TEST_FUNCTION(prx_scheduler_queue__arg_line_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_scheduler_queue();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_scheduler_queue unhappy path
//
TEST_FUNCTION(prx_scheduler_queue__neg)
{
    static const prx_scheduler_t* k_scheduler_valid;
    static const const char* k_name_valid;
    static const prx_task_t k_task_valid;
    static const void* k_context_valid;
    static const uint32_t k_delay_valid;
    static const const char* k_func_valid;
    static const int k_line_valid;
    intptr_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_scheduler_queue(k_scheduler_valid, k_name_valid, k_task_valid, k_context_valid, k_delay_valid, k_func_valid, k_line_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(intptr_t, result, er_ok);
}

#endif // prx_scheduler_queue;

#ifdef prx_scheduler_kill

//
//Test prx_scheduler_kill happy path
//
TEST_FUNCTION(prx_scheduler_kill__success)
{
    static const prx_scheduler_t* k_scheduler_valid;
    static const intptr_t k_id_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = prx_scheduler_kill(k_scheduler_valid, k_id_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test prx_scheduler_kill passing as scheduler argument an invalid prx_scheduler_t* value
//
TEST_FUNCTION(prx_scheduler_kill__arg_scheduler_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_scheduler_kill();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_scheduler_kill passing as id argument an invalid intptr_t value
//
TEST_FUNCTION(prx_scheduler_kill__arg_id_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_scheduler_kill();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_scheduler_kill unhappy path
//
TEST_FUNCTION(prx_scheduler_kill__neg)
{
    static const prx_scheduler_t* k_scheduler_valid;
    static const intptr_t k_id_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_scheduler_kill(k_scheduler_valid, k_id_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_scheduler_kill;

#ifdef prx_scheduler_clear

//
//Test prx_scheduler_clear happy path
//
TEST_FUNCTION(prx_scheduler_clear__success)
{
    static const prx_scheduler_t* k_scheduler_valid;
    static const prx_task_t k_task_valid;
    static const void* k_context_valid;
    void result;

    // arrange
    // ...

    // act
    result = prx_scheduler_clear(k_scheduler_valid, k_task_valid, k_context_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ...
}

//
// Test prx_scheduler_clear passing as scheduler argument an invalid prx_scheduler_t* value
//
TEST_FUNCTION(prx_scheduler_clear__arg_scheduler_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_scheduler_clear();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_scheduler_clear passing as task argument an invalid prx_task_t value
//
TEST_FUNCTION(prx_scheduler_clear__arg_task_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_scheduler_clear();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_scheduler_clear passing as context argument an invalid void* value
//
TEST_FUNCTION(prx_scheduler_clear__arg_context_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_scheduler_clear();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_scheduler_clear unhappy path
//
TEST_FUNCTION(prx_scheduler_clear__neg)
{
    static const prx_scheduler_t* k_scheduler_valid;
    static const prx_task_t k_task_valid;
    static const void* k_context_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_scheduler_clear(k_scheduler_valid, k_task_valid, k_context_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // prx_scheduler_clear;

#ifdef prx_scheduler_release

//
//Test prx_scheduler_release happy path
//
TEST_FUNCTION(prx_scheduler_release__success)
{
    static const prx_scheduler_t* k_scheduler_valid;
    static const void* k_context_valid;
    void result;

    // arrange
    // ...

    // act
    result = prx_scheduler_release(k_scheduler_valid, k_context_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ...
}

//
// Test prx_scheduler_release passing as scheduler argument an invalid prx_scheduler_t* value
//
TEST_FUNCTION(prx_scheduler_release__arg_scheduler_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_scheduler_release();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_scheduler_release passing as context argument an invalid void* value
//
TEST_FUNCTION(prx_scheduler_release__arg_context_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_scheduler_release();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_scheduler_release unhappy path
//
TEST_FUNCTION(prx_scheduler_release__neg)
{
    static const prx_scheduler_t* k_scheduler_valid;
    static const void* k_context_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_scheduler_release(k_scheduler_valid, k_context_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // prx_scheduler_release;

#ifdef prx_scheduler_at_exit

//
//Test prx_scheduler_at_exit happy path
//
TEST_FUNCTION(prx_scheduler_at_exit__success)
{
    static const prx_scheduler_t* k_scheduler_valid;
    void result;

    // arrange
    // ...

    // act
    result = prx_scheduler_at_exit(k_scheduler_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ...
}

//
// Test prx_scheduler_at_exit passing as scheduler argument an invalid prx_scheduler_t* value
//
TEST_FUNCTION(prx_scheduler_at_exit__arg_scheduler_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_scheduler_at_exit();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_scheduler_at_exit unhappy path
//
TEST_FUNCTION(prx_scheduler_at_exit__neg)
{
    static const prx_scheduler_t* k_scheduler_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_scheduler_at_exit(k_scheduler_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // prx_scheduler_at_exit;

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

