// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST prx_server
#include "util_ut.h"

//
// 1. Required mocks
//
#include "prx_sched.h"
#include "io_transport.h"

#include "hashtable.h"

typedef unsigned int(*hashfunction_t) (void*);
typedef int(*key_eq_fn_t) (void*, void*);

MOCKABLE_FUNCTION(, int, hashtable_insert, struct hashtable*, h, void*, k, void*, v);
MOCKABLE_FUNCTION(, void*, hashtable_remove, struct hashtable*, h, void*, k);
MOCKABLE_FUNCTION(, void*, hashtable_search, struct hashtable*, h, void*, k);
MOCKABLE_FUNCTION(, unsigned int, hashtable_count, struct hashtable*, h);
MOCKABLE_FUNCTION(, void, hashtable_destroy, struct hashtable*, h, int, free_values);
MOCKABLE_FUNCTION(, struct hashtable_itr*, hashtable_iterator, struct hashtable*, h);
MOCKABLE_FUNCTION(, int, hashtable_iterator_advance, struct hashtable_itr*, itr);
MOCKABLE_FUNCTION(, int, hashtable_iterator_remove, struct hashtable_itr*, itr);
MOCKABLE_FUNCTION(, struct hashtable*, create_hashtable, unsigned int, minsize,
    hashfunction_t, hashfunction, key_eq_fn_t, key_eq_fn);

//
// 2. Include unit under test
//
#undef ENABLE_MOCKS
#include UNIT_H
#define ENABLE_MOCKS
#include UNIT_C

// io_transport.h
MOCKABLE_FUNCTION(, io_transport_t*, io_iot_hub_ws_server_transport);

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


#ifdef prx_server_create

//
//Test prx_server_create happy path 
// 
TEST_FUNCTION(prx_server_create__success)
{
    static const io_transport_t* k_transport_valid;
    static const prx_ns_entry_t* k_entry_valid;
    static const prx_scheduler_t* k_scheduler_valid;
    static const prx_server_t** k_server_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = prx_server_create(k_transport_valid, k_entry_valid, k_scheduler_valid, k_server_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test prx_server_create passing as transport argument an invalid io_transport_t* value 
// 
TEST_FUNCTION(prx_server_create__arg_transport_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_server_create();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_server_create passing as entry argument an invalid prx_ns_entry_t* value 
// 
TEST_FUNCTION(prx_server_create__arg_entry_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_server_create();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_server_create passing as scheduler argument an invalid prx_scheduler_t* value 
// 
TEST_FUNCTION(prx_server_create__arg_scheduler_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_server_create();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_server_create passing as server argument an invalid prx_server_t** value 
// 
TEST_FUNCTION(prx_server_create__arg_server_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_server_create();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_server_create unhappy path 
// 
TEST_FUNCTION(prx_server_create__neg)
{
    static const io_transport_t* k_transport_valid;
    static const prx_ns_entry_t* k_entry_valid;
    static const prx_scheduler_t* k_scheduler_valid;
    static const prx_server_t** k_server_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_server_create(k_transport_valid, k_entry_valid, k_scheduler_valid, k_server_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_server_create;

#ifdef prx_server_release

//
//Test prx_server_release happy path 
// 
TEST_FUNCTION(prx_server_release__success)
{
    static const prx_server_t* k_server_valid;
    void result;

    // arrange 
    // ... 

    // act 
    result = prx_server_release(k_server_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ... 
}

// 
// Test prx_server_release passing as server argument an invalid prx_server_t* value 
// 
TEST_FUNCTION(prx_server_release__arg_server_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_server_release();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_server_release unhappy path 
// 
TEST_FUNCTION(prx_server_release__neg)
{
    static const prx_server_t* k_server_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_server_release(k_server_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // prx_server_release;



//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

