// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST prx_browse
#include "util_ut.h"

//
// 1. Required mocks
//
#include "io_codec.h"
#include "prx_sched.h"
#include "pal_sk.h"

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


#ifdef prx_browse_server_create 
// 
// Test prx_browse_server_create happy path 
// 
TEST_FUNCTION(prx_browse_server_create__success)
{
    static const prx_scheduler_t* k_scheduler_valid;
    static const prx_browse_server_t** k_created_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = prx_browse_server_create(k_scheduler_valid, k_created_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test prx_browse_server_create passing as scheduler argument an invalid prx_scheduler_t* value 
// 
TEST_FUNCTION(prx_browse_server_create__arg_scheduler_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_browse_server_create();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_browse_server_create passing as created argument an invalid prx_browse_server_t** value 
// 
TEST_FUNCTION(prx_browse_server_create__arg_created_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_browse_server_create();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_browse_server_create unhappy path 
// 
TEST_FUNCTION(prx_browse_server_create__neg)
{
    static const prx_scheduler_t* k_scheduler_valid;
    static const prx_browse_server_t** k_created_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_browse_server_create(k_scheduler_valid, k_created_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_browse_server_create 

#ifdef prx_browse_server_accept 
// 
// Test prx_browse_server_accept happy path 
// 
TEST_FUNCTION(prx_browse_server_accept__success)
{
    static const prx_browse_server_t* k_server_valid;
    static const io_codec_id_t k_codec_id_valid;
    static const pal_socket_client_itf_t* k_itf_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = prx_browse_server_accept(k_server_valid, k_codec_id_valid, k_itf_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test prx_browse_server_accept passing as server argument an invalid prx_browse_server_t* value 
// 
TEST_FUNCTION(prx_browse_server_accept__arg_server_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_browse_server_accept();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_browse_server_accept passing as codec_id argument an invalid io_codec_id_t value 
// 
TEST_FUNCTION(prx_browse_server_accept__arg_codec_id_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_browse_server_accept();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_browse_server_accept passing as itf argument an invalid pal_socket_client_itf_t* value 
// 
TEST_FUNCTION(prx_browse_server_accept__arg_itf_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_browse_server_accept();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_browse_server_accept unhappy path 
// 
TEST_FUNCTION(prx_browse_server_accept__neg)
{
    static const prx_browse_server_t* k_server_valid;
    static const io_codec_id_t k_codec_id_valid;
    static const pal_socket_client_itf_t* k_itf_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_browse_server_accept(k_server_valid, k_codec_id_valid, k_itf_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_browse_server_accept 

#ifdef prx_browse_server_free 
// 
// Test prx_browse_server_free happy path 
// 
TEST_FUNCTION(prx_browse_server_free__success)
{
    static const prx_browse_server_t* k_server_valid;
    void result;

    // arrange 
    // ... 

    // act 
    result = prx_browse_server_free(k_server_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ... 
}

// 
// Test prx_browse_server_free passing as server argument an invalid prx_browse_server_t* value 
// 
TEST_FUNCTION(prx_browse_server_free__arg_server_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_browse_server_free();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_browse_server_free unhappy path 
// 
TEST_FUNCTION(prx_browse_server_free__neg)
{
    static const prx_browse_server_t* k_server_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_browse_server_free(k_server_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // prx_browse_server_free 


//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

