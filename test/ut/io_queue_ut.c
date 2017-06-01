// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST io_queue
#include "util_ut.h"

//
// 1. Required mocks
//
#include "io_stream.h"
#include "azure_c_shared_utility/doublylinkedlist.h"

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
REGISTER_UMOCK_ALIAS_TYPE(PDLIST_ENTRY, void*);
REGISTER_UMOCK_ALIAS_TYPE(const PDLIST_ENTRY, const void*);
REGISTER_UMOCK_ALIAS_TYPE(lock_t, void*);
END_DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
DECLARE_TEST_SETUP()


#ifdef io_queue_create

//
//Test io_queue_create happy path 
// 
TEST_FUNCTION(io_queue_create__success)
{
    static const const char* k_name_valid;
    static const io_queue_t** k_queue_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = io_queue_create(k_name_valid, k_queue_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test io_queue_create passing as name argument an invalid const char* value 
// 
TEST_FUNCTION(io_queue_create__arg_name_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_queue_create();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_queue_create passing as queue argument an invalid io_queue_t** value 
// 
TEST_FUNCTION(io_queue_create__arg_queue_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_queue_create();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_queue_create unhappy path 
// 
TEST_FUNCTION(io_queue_create__neg)
{
    static const const char* k_name_valid;
    static const io_queue_t** k_queue_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_queue_create(k_name_valid, k_queue_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_queue_create;

#ifdef io_queue_free

//
//Test io_queue_free happy path 
// 
TEST_FUNCTION(io_queue_free__success)
{
    static const io_queue_t* k_queue_valid;
    void result;

    // arrange 
    // ... 

    // act 
    result = io_queue_free(k_queue_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ... 
}

// 
// Test io_queue_free passing as queue argument an invalid io_queue_t* value 
// 
TEST_FUNCTION(io_queue_free__arg_queue_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_queue_free();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_queue_free unhappy path 
// 
TEST_FUNCTION(io_queue_free__neg)
{
    static const io_queue_t* k_queue_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_queue_free(k_queue_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // io_queue_free;

#ifdef io_queue_has_ready

//
//Test io_queue_has_ready happy path 
// 
TEST_FUNCTION(io_queue_has_ready__success)
{
    static const io_queue_t* k_queue_valid;
    bool result;

    // arrange 
    // ... 

    // act 
    result = io_queue_has_ready(k_queue_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(bool, er_ok, result);
    // ... 
}

// 
// Test io_queue_has_ready passing as queue argument an invalid io_queue_t* value 
// 
TEST_FUNCTION(io_queue_has_ready__arg_queue_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_queue_has_ready();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_queue_has_ready unhappy path 
// 
TEST_FUNCTION(io_queue_has_ready__neg)
{
    static const io_queue_t* k_queue_valid;
    bool result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_queue_has_ready(k_queue_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(bool, result, er_ok);
}

#endif // io_queue_has_ready;

#ifdef io_queue_pop_ready

//
//Test io_queue_pop_ready happy path 
// 
TEST_FUNCTION(io_queue_pop_ready__success)
{
    static const io_queue_t* k_queue_valid;
    io_queue_buffer_t* result;

    // arrange 
    // ... 

    // act 
    result = io_queue_pop_ready(k_queue_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(io_queue_buffer_t*, er_ok, result);
    // ... 
}

// 
// Test io_queue_pop_ready passing as queue argument an invalid io_queue_t* value 
// 
TEST_FUNCTION(io_queue_pop_ready__arg_queue_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_queue_pop_ready();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_queue_pop_ready unhappy path 
// 
TEST_FUNCTION(io_queue_pop_ready__neg)
{
    static const io_queue_t* k_queue_valid;
    io_queue_buffer_t* result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_queue_pop_ready(k_queue_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(io_queue_buffer_t*, result, er_ok);
}

#endif // io_queue_pop_ready;

#ifdef io_queue_has_inprogress

//
//Test io_queue_has_inprogress happy path 
// 
TEST_FUNCTION(io_queue_has_inprogress__success)
{
    static const io_queue_t* k_queue_valid;
    bool result;

    // arrange 
    // ... 

    // act 
    result = io_queue_has_inprogress(k_queue_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(bool, er_ok, result);
    // ... 
}

// 
// Test io_queue_has_inprogress passing as queue argument an invalid io_queue_t* value 
// 
TEST_FUNCTION(io_queue_has_inprogress__arg_queue_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_queue_has_inprogress();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_queue_has_inprogress unhappy path 
// 
TEST_FUNCTION(io_queue_has_inprogress__neg)
{
    static const io_queue_t* k_queue_valid;
    bool result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_queue_has_inprogress(k_queue_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(bool, result, er_ok);
}

#endif // io_queue_has_inprogress;

#ifdef io_queue_pop_inprogress

//
//Test io_queue_pop_inprogress happy path 
// 
TEST_FUNCTION(io_queue_pop_inprogress__success)
{
    static const io_queue_t* k_queue_valid;
    io_queue_buffer_t* result;

    // arrange 
    // ... 

    // act 
    result = io_queue_pop_inprogress(k_queue_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(io_queue_buffer_t*, er_ok, result);
    // ... 
}

// 
// Test io_queue_pop_inprogress passing as queue argument an invalid io_queue_t* value 
// 
TEST_FUNCTION(io_queue_pop_inprogress__arg_queue_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_queue_pop_inprogress();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_queue_pop_inprogress unhappy path 
// 
TEST_FUNCTION(io_queue_pop_inprogress__neg)
{
    static const io_queue_t* k_queue_valid;
    io_queue_buffer_t* result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_queue_pop_inprogress(k_queue_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(io_queue_buffer_t*, result, er_ok);
}

#endif // io_queue_pop_inprogress;

#ifdef io_queue_has_done

//
//Test io_queue_has_done happy path 
// 
TEST_FUNCTION(io_queue_has_done__success)
{
    static const io_queue_t* k_queue_valid;
    bool result;

    // arrange 
    // ... 

    // act 
    result = io_queue_has_done(k_queue_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(bool, er_ok, result);
    // ... 
}

// 
// Test io_queue_has_done passing as queue argument an invalid io_queue_t* value 
// 
TEST_FUNCTION(io_queue_has_done__arg_queue_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_queue_has_done();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_queue_has_done unhappy path 
// 
TEST_FUNCTION(io_queue_has_done__neg)
{
    static const io_queue_t* k_queue_valid;
    bool result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_queue_has_done(k_queue_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(bool, result, er_ok);
}

#endif // io_queue_has_done;

#ifdef io_queue_pop_done

//
//Test io_queue_pop_done happy path 
// 
TEST_FUNCTION(io_queue_pop_done__success)
{
    static const io_queue_t* k_queue_valid;
    io_queue_buffer_t* result;

    // arrange 
    // ... 

    // act 
    result = io_queue_pop_done(k_queue_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(io_queue_buffer_t*, er_ok, result);
    // ... 
}

// 
// Test io_queue_pop_done passing as queue argument an invalid io_queue_t* value 
// 
TEST_FUNCTION(io_queue_pop_done__arg_queue_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_queue_pop_done();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_queue_pop_done unhappy path 
// 
TEST_FUNCTION(io_queue_pop_done__neg)
{
    static const io_queue_t* k_queue_valid;
    io_queue_buffer_t* result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_queue_pop_done(k_queue_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(io_queue_buffer_t*, result, er_ok);
}

#endif // io_queue_pop_done;

#ifdef io_queue_rollback

//
//Test io_queue_rollback happy path 
// 
TEST_FUNCTION(io_queue_rollback__success)
{
    static const io_queue_t* k_queue_valid;
    void result;

    // arrange 
    // ... 

    // act 
    result = io_queue_rollback(k_queue_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ... 
}

// 
// Test io_queue_rollback passing as queue argument an invalid io_queue_t* value 
// 
TEST_FUNCTION(io_queue_rollback__arg_queue_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_queue_rollback();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_queue_rollback unhappy path 
// 
TEST_FUNCTION(io_queue_rollback__neg)
{
    static const io_queue_t* k_queue_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_queue_rollback(k_queue_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // io_queue_rollback;

#ifdef io_queue_abort

//
//Test io_queue_abort happy path 
// 
TEST_FUNCTION(io_queue_abort__success)
{
    static const io_queue_t* k_queue_valid;
    void result;

    // arrange 
    // ... 

    // act 
    result = io_queue_abort(k_queue_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ... 
}

// 
// Test io_queue_abort passing as queue argument an invalid io_queue_t* value 
// 
TEST_FUNCTION(io_queue_abort__arg_queue_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_queue_abort();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_queue_abort unhappy path 
// 
TEST_FUNCTION(io_queue_abort__neg)
{
    static const io_queue_t* k_queue_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_queue_abort(k_queue_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // io_queue_abort;

#ifdef io_queue_create_buffer

//
//Test io_queue_create_buffer happy path 
// 
TEST_FUNCTION(io_queue_create_buffer__success)
{
    static const io_queue_t* k_queue_valid;
    static const const void* k_payload_valid;
    static const size_t k_size_valid;
    static const io_queue_buffer_t** k_buffer_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = io_queue_create_buffer(k_queue_valid, k_payload_valid, k_size_valid, k_buffer_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test io_queue_create_buffer passing as queue argument an invalid io_queue_t* value 
// 
TEST_FUNCTION(io_queue_create_buffer__arg_queue_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_queue_create_buffer();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_queue_create_buffer passing as payload argument an invalid const void* value 
// 
TEST_FUNCTION(io_queue_create_buffer__arg_payload_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_queue_create_buffer();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_queue_create_buffer passing as size argument an invalid size_t value 
// 
TEST_FUNCTION(io_queue_create_buffer__arg_size_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_queue_create_buffer();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_queue_create_buffer passing as buffer argument an invalid io_queue_buffer_t** value 
// 
TEST_FUNCTION(io_queue_create_buffer__arg_buffer_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_queue_create_buffer();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_queue_create_buffer unhappy path 
// 
TEST_FUNCTION(io_queue_create_buffer__neg)
{
    static const io_queue_t* k_queue_valid;
    static const const void* k_payload_valid;
    static const size_t k_size_valid;
    static const io_queue_buffer_t** k_buffer_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_queue_create_buffer(k_queue_valid, k_payload_valid, k_size_valid, k_buffer_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_queue_create_buffer;

#ifdef io_queue_buffer_set_done

//
//Test io_queue_buffer_set_done happy path 
// 
TEST_FUNCTION(io_queue_buffer_set_done__success)
{
    static const io_queue_buffer_t* k_buffer_valid;
    void result;

    // arrange 
    // ... 

    // act 
    result = io_queue_buffer_set_done(k_buffer_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ... 
}

// 
// Test io_queue_buffer_set_done passing as buffer argument an invalid io_queue_buffer_t* value 
// 
TEST_FUNCTION(io_queue_buffer_set_done__arg_buffer_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_queue_buffer_set_done();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_queue_buffer_set_done unhappy path 
// 
TEST_FUNCTION(io_queue_buffer_set_done__neg)
{
    static const io_queue_buffer_t* k_buffer_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_queue_buffer_set_done(k_buffer_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // io_queue_buffer_set_done;

#ifdef io_queue_buffer_set_inprogress

//
//Test io_queue_buffer_set_inprogress happy path 
// 
TEST_FUNCTION(io_queue_buffer_set_inprogress__success)
{
    static const io_queue_buffer_t* k_buffer_valid;
    void result;

    // arrange 
    // ... 

    // act 
    result = io_queue_buffer_set_inprogress(k_buffer_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ... 
}

// 
// Test io_queue_buffer_set_inprogress passing as buffer argument an invalid io_queue_buffer_t* value 
// 
TEST_FUNCTION(io_queue_buffer_set_inprogress__arg_buffer_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_queue_buffer_set_inprogress();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_queue_buffer_set_inprogress unhappy path 
// 
TEST_FUNCTION(io_queue_buffer_set_inprogress__neg)
{
    static const io_queue_buffer_t* k_buffer_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_queue_buffer_set_inprogress(k_buffer_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // io_queue_buffer_set_inprogress;

#ifdef io_queue_buffer_set_ready

//
//Test io_queue_buffer_set_ready happy path 
// 
TEST_FUNCTION(io_queue_buffer_set_ready__success)
{
    static const io_queue_buffer_t* k_buffer_valid;
    void result;

    // arrange 
    // ... 

    // act 
    result = io_queue_buffer_set_ready(k_buffer_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ... 
}

// 
// Test io_queue_buffer_set_ready passing as buffer argument an invalid io_queue_buffer_t* value 
// 
TEST_FUNCTION(io_queue_buffer_set_ready__arg_buffer_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_queue_buffer_set_ready();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_queue_buffer_set_ready unhappy path 
// 
TEST_FUNCTION(io_queue_buffer_set_ready__neg)
{
    static const io_queue_buffer_t* k_buffer_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_queue_buffer_set_ready(k_buffer_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // io_queue_buffer_set_ready;

#ifdef io_queue_buffer_to_ptr

//
//Test io_queue_buffer_to_ptr happy path 
// 
TEST_FUNCTION(io_queue_buffer_to_ptr__success)
{
    static const io_queue_buffer_t* k_buffer_valid;
    uint8_t* result;

    // arrange 
    // ... 

    // act 
    result = io_queue_buffer_to_ptr(k_buffer_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(uint8_t*, er_ok, result);
    // ... 
}

// 
// Test io_queue_buffer_to_ptr passing as buffer argument an invalid io_queue_buffer_t* value 
// 
TEST_FUNCTION(io_queue_buffer_to_ptr__arg_buffer_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_queue_buffer_to_ptr();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_queue_buffer_to_ptr unhappy path 
// 
TEST_FUNCTION(io_queue_buffer_to_ptr__neg)
{
    static const io_queue_buffer_t* k_buffer_valid;
    uint8_t* result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_queue_buffer_to_ptr(k_buffer_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(uint8_t*, result, er_ok);
}

#endif // io_queue_buffer_to_ptr;

#ifdef io_queue_buffer_from_ptr

//
//Test io_queue_buffer_from_ptr happy path 
// 
TEST_FUNCTION(io_queue_buffer_from_ptr__success)
{
    static const uint8_t* k_payload_valid;
    io_queue_buffer_t* result;

    // arrange 
    // ... 

    // act 
    result = io_queue_buffer_from_ptr(k_payload_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(io_queue_buffer_t*, er_ok, result);
    // ... 
}

// 
// Test io_queue_buffer_from_ptr passing as payload argument an invalid uint8_t* value 
// 
TEST_FUNCTION(io_queue_buffer_from_ptr__arg_payload_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_queue_buffer_from_ptr();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_queue_buffer_from_ptr unhappy path 
// 
TEST_FUNCTION(io_queue_buffer_from_ptr__neg)
{
    static const uint8_t* k_payload_valid;
    io_queue_buffer_t* result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_queue_buffer_from_ptr(k_payload_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(io_queue_buffer_t*, result, er_ok);
}

#endif // io_queue_buffer_from_ptr;

#ifdef io_queue_buffer_release

//
//Test io_queue_buffer_release happy path 
// 
TEST_FUNCTION(io_queue_buffer_release__success)
{
    static const io_queue_buffer_t* k_buffer_valid;
    void result;

    // arrange 
    // ... 

    // act 
    result = io_queue_buffer_release(k_buffer_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ... 
}

// 
// Test io_queue_buffer_release passing as buffer argument an invalid io_queue_buffer_t* value 
// 
TEST_FUNCTION(io_queue_buffer_release__arg_buffer_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = io_queue_buffer_release();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test io_queue_buffer_release unhappy path 
// 
TEST_FUNCTION(io_queue_buffer_release__neg)
{
    static const io_queue_buffer_t* k_buffer_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_queue_buffer_release(k_buffer_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // io_queue_buffer_release;




#if 0

// 
// Test io_queue_create_buffer happy path 
// 
TEST_FUNCTION(io_queue_create_buffer__success_1)
{
    static char buffer_valid[8] = { 't', 'e', 's', 't', 't', 'e', 's', 't' };
    static const size_t k_size_valid = sizeof(buffer_valid);
    static prx_buffer_factory_t* k_factory_valid = (prx_buffer_factory_t*)0x1235;
    static void* k_buffer_valid = (void*)0x1254;
    io_queue_buffer_t* queue_buffer_valid;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_queue_buffer_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(prx_buffer_factory_get_default(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_factory(&k_factory_valid, sizeof(k_factory_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(prx_buffer_factory_create_buffer(k_factory_valid, k_size_valid, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_buffer(&k_buffer_valid, sizeof(k_buffer_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_queue_buffer_set_size(k_factory_valid, IGNORED_PTR_ARG, k_size_valid))
        .ValidateArgumentBuffer(2, &k_buffer_valid, sizeof(k_buffer_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(prx_buffer_factory_set_buffer(k_factory_valid, k_buffer_valid, 0, buffer_valid, k_size_valid));

    // act 
    result = io_queue_create_buffer(buffer_valid, k_size_valid, NULL, &queue_buffer_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_queue_create_buffer happy path 
// 
TEST_FUNCTION(io_queue_create_buffer__success_2)
{
    static char buffer_valid[8] = { 't', 'e', 's', 't', 't', 'e', 's', 't' };
    static const size_t k_size_valid = sizeof(buffer_valid);
    static prx_buffer_factory_t* k_factory_valid = (prx_buffer_factory_t*)0x1235;
    static void* k_buffer_valid = (void*)0x1254;
    io_queue_buffer_t* queue_buffer_valid;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_queue_buffer_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(prx_buffer_factory_create_buffer(k_factory_valid, k_size_valid, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_buffer(&k_buffer_valid, sizeof(k_buffer_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(io_queue_buffer_set_size(k_factory_valid, IGNORED_PTR_ARG, k_size_valid))
        .ValidateArgumentBuffer(2, &k_buffer_valid, sizeof(k_buffer_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(prx_buffer_factory_set_buffer(k_factory_valid, k_buffer_valid, 0, buffer_valid, k_size_valid));

    // act 
    result = io_queue_create_buffer(buffer_valid, k_size_valid, k_factory_valid, &queue_buffer_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_queue_create_buffer happy path 
// 
TEST_FUNCTION(io_queue_create_buffer__success_3)
{
    static char buffer_valid[8] = { 't', 'e', 's', 't', 't', 'e', 's', 't' };
    static prx_buffer_factory_t* k_factory_valid = (prx_buffer_factory_t*)0x1235;
    static void* k_buffer_valid = (void*)0x1254;
    io_queue_buffer_t* queue_buffer_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_queue_buffer_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(prx_buffer_factory_get_default(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_factory(&k_factory_valid, sizeof(k_factory_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(prx_buffer_factory_create_buffer(k_factory_valid, 0, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_buffer(&k_buffer_valid, sizeof(k_buffer_valid))
        .SetReturn(er_ok);

    // act 
    result = io_queue_create_buffer(buffer_valid, 0, NULL, &queue_buffer_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_queue_create_buffer happy path 
// 
TEST_FUNCTION(io_queue_create_buffer__success_4)
{
    static char buffer_valid[8] = { 't', 'e', 's', 't', 't', 'e', 's', 't' };
    static prx_buffer_factory_t* k_factory_valid = (prx_buffer_factory_t*)0x1235;
    static void* k_buffer_valid = (void*)0x1254;
    io_queue_buffer_t* queue_buffer_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_queue_buffer_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(prx_buffer_factory_create_buffer(k_factory_valid, 0, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_buffer(&k_buffer_valid, sizeof(k_buffer_valid))
        .SetReturn(er_ok);

    // act 
    result = io_queue_create_buffer(buffer_valid, 0, k_factory_valid, &queue_buffer_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_queue_create_buffer passing as queue_buffer argument an invalid io_queue_buffer_t** value 
// 
TEST_FUNCTION(io_queue_create_buffer__arg_queue_buffer_invalid)
{
    static char buffer_valid[8] = { 't', 'e', 's', 't', 't', 'e', 's', 't' };
    static const size_t k_size_valid = sizeof(buffer_valid);
    static prx_buffer_factory_t* k_factory_valid = (prx_buffer_factory_t*)0x1235;
    int32_t result;

    // arrange 

    // act 
    result = io_queue_create_buffer(buffer_valid, k_size_valid, k_factory_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_queue_create_buffer unhappy path 
// 
TEST_FUNCTION(io_queue_create_buffer__neg_1)
{
    static char buffer_valid[8] = { 't', 'e', 's', 't', 't', 'e', 's', 't' };
    static const size_t k_size_valid = sizeof(buffer_valid);
    static prx_buffer_factory_t* k_factory_valid = (prx_buffer_factory_t*)0x1235;
    static void* k_buffer_valid = (void*)0x1254;
    io_queue_buffer_t* queue_buffer_valid;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_queue_buffer_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(prx_buffer_factory_get_default(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_factory(&k_factory_valid, sizeof(k_factory_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(prx_buffer_factory_create_buffer(k_factory_valid, k_size_valid, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_buffer(&k_buffer_valid, sizeof(k_buffer_valid))
        .SetReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL(prx_buffer_factory_free_buffer(k_factory_valid, k_buffer_valid));
    STRICT_EXPECTED_CALL(h_free((void*)UT_MEM, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    result = io_queue_create_buffer(buffer_valid, k_size_valid, NULL, &queue_buffer_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_out_of_memory, result);
}

// 
// Test io_queue_create_buffer unhappy path 
// 
TEST_FUNCTION(io_queue_create_buffer__neg_2)
{
    static char buffer_valid[8] = { 't', 'e', 's', 't', 't', 'e', 's', 't' };
    static const size_t k_size_valid = sizeof(buffer_valid);
    static prx_buffer_factory_t* k_factory_valid = (prx_buffer_factory_t*)0x1235;
    static void* k_buffer_valid = (void*)0x1254;
    io_queue_buffer_t* queue_buffer_valid;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_queue_buffer_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(prx_buffer_factory_get_default(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_factory(&k_factory_valid, sizeof(k_factory_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL(prx_buffer_factory_create_buffer(k_factory_valid, k_size_valid, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_buffer(&k_buffer_valid, sizeof(k_buffer_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL(io_queue_buffer_set_size(k_factory_valid, IGNORED_PTR_ARG, k_size_valid))
        .ValidateArgumentBuffer(2, &k_buffer_valid, sizeof(k_buffer_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL(prx_buffer_factory_set_buffer(k_factory_valid, k_buffer_valid, 0, buffer_valid, k_size_valid));
    STRICT_EXPECTED_CALL(prx_buffer_factory_free_buffer(k_factory_valid, k_buffer_valid));
    STRICT_EXPECTED_CALL(h_free((void*)UT_MEM, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_queue_create_buffer(buffer_valid, k_size_valid, NULL, &queue_buffer_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result,
        er_out_of_memory, er_out_of_memory, er_out_of_memory, er_out_of_memory, er_ok);
}

// 
// Test io_queue_buffer_release happy path 
// 
TEST_FUNCTION(io_queue_buffer_release__success_1)
{
    static prx_buffer_factory_t* k_factory_valid = (prx_buffer_factory_t*)0x1235;
    static void* k_buffer_valid = (void*)0x1254;
    io_queue_buffer_t queue_buffer_struct_valid;
    io_queue_buffer_t* queue_buffer_valid = (io_queue_buffer_t*)&queue_buffer_struct_valid;

    queue_buffer_struct_valid.buf_obj = k_buffer_valid;
    queue_buffer_struct_valid.factory = k_factory_valid;

    // arrange 
    STRICT_EXPECTED_CALL(prx_buffer_factory_free_buffer(k_factory_valid, k_buffer_valid));
    STRICT_EXPECTED_CALL(h_free((void*)queue_buffer_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    io_queue_buffer_release(queue_buffer_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test io_queue_buffer_release happy path 
// 
TEST_FUNCTION(io_queue_buffer_release__success_2)
{
    io_queue_buffer_t queue_buffer_struct_valid;
    io_queue_buffer_t* queue_buffer_valid = (io_queue_buffer_t*)&queue_buffer_struct_valid;

    queue_buffer_struct_valid.buf_obj = NULL;

    // arrange 
    STRICT_EXPECTED_CALL(h_free((void*)queue_buffer_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    io_queue_buffer_release(queue_buffer_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test io_queue_buffer_release passing as queue_buffer argument an invalid io_queue_buffer_t* value 
// 
TEST_FUNCTION(io_queue_buffer_release__arg_queue_buffer_invalid)
{
    // arrange 

    // act 
    io_queue_buffer_release(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test io_queue_buffer_get_size happy path 
// 
TEST_FUNCTION(io_queue_buffer_get_size__success)
{
    io_queue_buffer_t queue_buffer_struct_valid;
    io_queue_buffer_t *queue_buffer_valid = (io_queue_buffer_t*)&queue_buffer_struct_valid;
    size_t result;

    queue_buffer_valid->length = 100;

    // arrange 

    // act 
    result = io_queue_buffer_get_size(queue_buffer_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, 100, (int32_t)result);
}

// 
// Test io_queue_buffer_get_size passing as queue_buffer argument an invalid io_queue_buffer_t* value 
// 
TEST_FUNCTION(io_queue_buffer_get_size__arg_queue_buffer_null)
{
    size_t result;

    // arrange 

    // act 
    result = io_queue_buffer_get_size(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, 0, (int32_t)result);
}

// 
// Test io_queue_buffer_to_ptr happy path 
// 
TEST_FUNCTION(io_queue_buffer_get_raw_buffer__success_1)
{
    static prx_buffer_factory_t* k_factory_valid = (prx_buffer_factory_t*)0x1235;
    static void* k_buffer_valid = (void*)0x1254;
    static const size_t k_size_valid = sizeof(UT_MEM);
    io_queue_buffer_t queue_buffer_struct_valid;
    io_queue_buffer_t *queue_buffer_valid = (io_queue_buffer_t*)&queue_buffer_struct_valid;
    size_t alloc_size_valid;
    const void* result;

    memset(&queue_buffer_struct_valid, 0xab, sizeof(queue_buffer_struct_valid));
    queue_buffer_struct_valid.buf_obj = k_buffer_valid;
    queue_buffer_struct_valid.factory = k_factory_valid;

    // arrange 
    STRICT_EXPECTED_CALL(prx_buffer_factory_get_buffer(k_factory_valid, k_buffer_valid, 0, IGNORED_PTR_ARG, NULL))
        .CopyOutArgumentBuffer_len(&k_size_valid, sizeof(k_size_valid))
        .SetReturn(UT_MEM);

    // act 
    result = io_queue_buffer_to_ptr(queue_buffer_valid, &alloc_size_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void_ptr, (void*)UT_MEM, (void*)result);
    ASSERT_ARE_EQUAL(int32_t, (int32_t)k_size_valid, (int32_t)alloc_size_valid);
}

// 
// Test io_queue_buffer_to_ptr happy path 
// 
TEST_FUNCTION(io_queue_buffer_get_raw_buffer__success_2)
{
    static prx_buffer_factory_t* k_factory_valid = (prx_buffer_factory_t*)0x1235;
    static void* k_buffer_valid = (void*)0x1254;
    io_queue_buffer_t queue_buffer_struct_valid;
    io_queue_buffer_t *queue_buffer_valid = (io_queue_buffer_t*)&queue_buffer_struct_valid;
    const void* result;

    memset(&queue_buffer_struct_valid, 0xab, sizeof(queue_buffer_struct_valid));
    queue_buffer_struct_valid.buf_obj = k_buffer_valid;
    queue_buffer_struct_valid.factory = k_factory_valid;

    // arrange 
    STRICT_EXPECTED_CALL(prx_buffer_factory_get_buffer(k_factory_valid, k_buffer_valid, 0, NULL, NULL))
        .SetReturn(UT_MEM);

    // act 
    result = io_queue_buffer_to_ptr(queue_buffer_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void_ptr, (void*)UT_MEM, (void*)result);
}

// 
// Test io_queue_buffer_to_ptr passing as queue_buffer argument an invalid io_queue_buffer_t* value 
// 
TEST_FUNCTION(io_queue_buffer_get_raw_buffer__arg_queue_buffer_null_1)
{
    size_t alloc_size_valid = 10000;
    const void* result;

    // arrange 

    // act 
    result = io_queue_buffer_to_ptr(NULL, &alloc_size_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(int32_t, 0, (int32_t)alloc_size_valid);
}

// 
// Test io_queue_buffer_to_ptr passing as queue_buffer argument an invalid io_queue_buffer_t* value 
// 
TEST_FUNCTION(io_queue_buffer_get_raw_buffer__arg_queue_buffer_null_2)
{
    const void* result;

    // arrange 

    // act 
    result = io_queue_buffer_to_ptr(NULL, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_NULL(result);
}

// 
// Test io_queue_buffer_as_stream happy path 
// 
TEST_FUNCTION(io_queue_buffer_as_stream__success)
{
    io_queue_buffer_t queue_buffer_struct_valid;
    io_queue_buffer_t *queue_buffer_valid = (io_queue_buffer_t*)&queue_buffer_struct_valid;
    io_stream_t* result;

    // arrange 

    // act 
    result = io_queue_buffer_as_stream(queue_buffer_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void_ptr, &queue_buffer_struct_valid.stream, result);
}

// 
// Test io_queue_buffer_as_stream passing as queue_buffer argument an invalid io_queue_buffer_t* value 
// 
TEST_FUNCTION(io_queue_buffer_as_stream__arg_queue_buffer_invalid)
{
    io_stream_t* result;

    // arrange 

    // act 
    result = io_queue_buffer_as_stream(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void_ptr, NULL, result);
}

// 
// Test io_queue_buffer_seek happy path 
// 
TEST_FUNCTION(io_queue_buffer_seek__success_1)
{
    static const size_t k_offset_valid = 100;
    io_queue_buffer_t queue_buffer_struct_valid;
    io_queue_buffer_t *queue_buffer_valid = (io_queue_buffer_t*)&queue_buffer_struct_valid;
    int32_t result;

    queue_buffer_valid->length = 1000;
    queue_buffer_valid->offset = 10;

    // arrange 

    // act 
    result = io_queue_buffer_seek(queue_buffer_valid, k_offset_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(int32_t, k_offset_valid, queue_buffer_struct_valid.msg.offset);
    ASSERT_ARE_EQUAL(int32_t, 1000, queue_buffer_struct_valid.msg.length);
}

// 
// Test io_queue_buffer_seek happy path 
// 
TEST_FUNCTION(io_queue_buffer_seek__success_2)
{
    static prx_buffer_factory_t* k_factory_valid = (prx_buffer_factory_t*)0x1235;
    static void* k_buffer_valid = (void*)0x1254;
    static const size_t k_offset_valid = 1100;
    io_queue_buffer_t queue_buffer_struct_valid;
    io_queue_buffer_t *queue_buffer_valid = (io_queue_buffer_t*)&queue_buffer_struct_valid;
    int32_t result;

    queue_buffer_valid->length = 100;
    queue_buffer_valid->offset = 50;
    queue_buffer_struct_valid.buf_obj = k_buffer_valid;
    queue_buffer_struct_valid.factory = k_factory_valid;

    // arrange 
    STRICT_EXPECTED_CALL(io_queue_buffer_set_size(k_factory_valid, IGNORED_PTR_ARG, k_offset_valid))
        .ValidateArgumentBuffer(2, &k_buffer_valid, sizeof(k_buffer_valid))
        .SetReturn(er_ok);

    // act 
    result = io_queue_buffer_seek(queue_buffer_valid, k_offset_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(int32_t, k_offset_valid, queue_buffer_struct_valid.msg.offset);
    ASSERT_ARE_EQUAL(int32_t, k_offset_valid, queue_buffer_struct_valid.msg.length);
}

// 
// Test io_queue_buffer_seek happy path 
// 
TEST_FUNCTION(io_queue_buffer_seek__success_3)
{
    static const size_t k_offset_valid = 1000;
    io_queue_buffer_t queue_buffer_struct_valid;
    io_queue_buffer_t *queue_buffer_valid = (io_queue_buffer_t*)&queue_buffer_struct_valid;
    int32_t result;

    queue_buffer_valid->length = 1000;
    queue_buffer_valid->offset = 1000;

    // arrange 

    // act 
    result = io_queue_buffer_seek(queue_buffer_valid, k_offset_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(int32_t, k_offset_valid, queue_buffer_struct_valid.msg.offset);
    ASSERT_ARE_EQUAL(int32_t, k_offset_valid, queue_buffer_struct_valid.msg.length);
}

// 
// Test io_queue_buffer_seek passing as queue_buffer argument an invalid io_queue_buffer_t* value 
// 
TEST_FUNCTION(io_queue_buffer_seek__arg_queue_buffer_invalid)
{
    static const size_t k_offset_valid = 1000;
    int32_t result;

    // arrange 

    // act 
    result = io_queue_buffer_seek(NULL, k_offset_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_queue_buffer_seek unhappy path 
// 
TEST_FUNCTION(io_queue_buffer_seek__neg)
{
    static prx_buffer_factory_t* k_factory_valid = (prx_buffer_factory_t*)0x1235;
    static void* k_buffer_valid = (void*)0x1254;
    static const size_t k_offset_valid = 1100;
    io_queue_buffer_t queue_buffer_struct_valid;
    io_queue_buffer_t *queue_buffer_valid = (io_queue_buffer_t*)&queue_buffer_struct_valid;
    int32_t result;

    queue_buffer_valid->length = 100;
    queue_buffer_valid->offset = 50;
    queue_buffer_struct_valid.buf_obj = k_buffer_valid;
    queue_buffer_struct_valid.factory = k_factory_valid;

    // arrange 
    STRICT_EXPECTED_CALL(io_queue_buffer_set_size(k_factory_valid, IGNORED_PTR_ARG, k_offset_valid))
        .ValidateArgumentBuffer(2, &k_buffer_valid, sizeof(k_buffer_valid))
        .SetReturn(er_out_of_memory);

    // act 
    result = io_queue_buffer_seek(queue_buffer_valid, k_offset_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_out_of_memory, result);
    ASSERT_ARE_EQUAL(int32_t, 50, queue_buffer_struct_valid.msg.offset);
    ASSERT_ARE_EQUAL(int32_t, 100, queue_buffer_struct_valid.msg.length);
}

// 
// Test io_queue_buffer_write happy path 
// 
TEST_FUNCTION(io_queue_buffer_write__success_1)
{
    static prx_buffer_factory_t* k_factory_valid = (prx_buffer_factory_t*)0x1235;
    static void* k_buffer_valid = (void*)0x1254;
    static char in_valid[8] = { 't', 'e', 's', 't', 't', 'e', 's', 't' };
    static const size_t k_size_valid = sizeof(in_valid);
    io_queue_buffer_t queue_buffer_struct_valid;
    io_queue_buffer_t *queue_buffer_valid = (io_queue_buffer_t*)&queue_buffer_struct_valid;
    int32_t result;

    queue_buffer_valid->length = 10000;
    queue_buffer_valid->offset = 0;
    queue_buffer_struct_valid.buf_obj = k_buffer_valid;
    queue_buffer_struct_valid.factory = k_factory_valid;
    queue_buffer_struct_valid.is_const = true;

    // arrange 
    STRICT_EXPECTED_CALL(prx_buffer_factory_clone_buffer(k_factory_valid, k_buffer_valid, true, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_out(&k_buffer_valid, sizeof(k_buffer_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(prx_buffer_factory_free_buffer(k_factory_valid, k_buffer_valid));
    STRICT_EXPECTED_CALL(io_queue_buffer_set_size(k_factory_valid, IGNORED_PTR_ARG, k_size_valid))
        .ValidateArgumentBuffer(2, &k_buffer_valid, sizeof(k_buffer_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(prx_buffer_factory_set_buffer(k_factory_valid, k_buffer_valid, 0, in_valid, k_size_valid));

    // act 
    result = io_queue_buffer_write(queue_buffer_valid, in_valid, k_size_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_queue_buffer_write happy path 
// 
TEST_FUNCTION(io_queue_buffer_write__success_2)
{
    static prx_buffer_factory_t* k_factory_valid = (prx_buffer_factory_t*)0x1235;
    static void* k_buffer_valid = (void*)0x1254;
    static const size_t k_offset_valid = 1100;
    static char in_valid[8] = { 't', 'e', 's', 't', 't', 'e', 's', 't' };
    static const size_t k_size_valid = sizeof(in_valid);
    io_queue_buffer_t queue_buffer_struct_valid;
    io_queue_buffer_t *queue_buffer_valid = (io_queue_buffer_t*)&queue_buffer_struct_valid;
    int32_t result;

    queue_buffer_valid->length = 100;
    queue_buffer_valid->offset = k_offset_valid;
    queue_buffer_struct_valid.buf_obj = k_buffer_valid;
    queue_buffer_struct_valid.factory = k_factory_valid;
    queue_buffer_struct_valid.is_const = true;

    // arrange 
    STRICT_EXPECTED_CALL(prx_buffer_factory_clone_buffer(k_factory_valid, k_buffer_valid, true, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_out(&k_buffer_valid, sizeof(k_buffer_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(prx_buffer_factory_free_buffer(k_factory_valid, k_buffer_valid));
    STRICT_EXPECTED_CALL(io_queue_buffer_set_size(k_factory_valid, IGNORED_PTR_ARG, k_size_valid + k_offset_valid))
        .ValidateArgumentBuffer(2, &k_buffer_valid, sizeof(k_buffer_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(prx_buffer_factory_set_buffer(k_factory_valid, k_buffer_valid, k_offset_valid, in_valid, k_size_valid));

    // act 
    result = io_queue_buffer_write(queue_buffer_valid, in_valid, k_size_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_queue_buffer_write happy path 
// 
TEST_FUNCTION(io_queue_buffer_write__success_3)
{
    static prx_buffer_factory_t* k_factory_valid = (prx_buffer_factory_t*)0x1235;
    static void* k_buffer_valid = (void*)0x1254;
    static const size_t k_offset_valid = 1100;
    static char in_valid[8] = { 't', 'e', 's', 't', 't', 'e', 's', 't' };
    static const size_t k_size_valid = sizeof(in_valid);
    io_queue_buffer_t queue_buffer_struct_valid;
    io_queue_buffer_t *queue_buffer_valid = (io_queue_buffer_t*)&queue_buffer_struct_valid;
    int32_t result;

    queue_buffer_valid->length = 100;
    queue_buffer_valid->offset = k_offset_valid;
    queue_buffer_struct_valid.buf_obj = k_buffer_valid;
    queue_buffer_struct_valid.factory = k_factory_valid;
    queue_buffer_struct_valid.is_const = false;

    // arrange 
    STRICT_EXPECTED_CALL(io_queue_buffer_set_size(k_factory_valid, IGNORED_PTR_ARG, k_size_valid + k_offset_valid))
        .ValidateArgumentBuffer(2, &k_buffer_valid, sizeof(k_buffer_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(prx_buffer_factory_set_buffer(k_factory_valid, k_buffer_valid, k_offset_valid, in_valid, k_size_valid));

    // act 
    result = io_queue_buffer_write(queue_buffer_valid, in_valid, k_size_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_queue_buffer_write passing as queue_buffer argument an invalid io_queue_buffer_t* value 
// 
TEST_FUNCTION(io_queue_buffer_write__arg_queue_buffer_null)
{
    static char in_valid[8] = { 't', 'e', 's', 't', 't', 'e', 's', 't' };
    static const size_t k_size_valid = sizeof(in_valid);
    int32_t result;

    // arrange 

    // act 
    result = io_queue_buffer_write(NULL, in_valid, k_size_valid);

    // assert 
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    ASSERT_EXPECTED_CALLS();
}

// 
// Test io_queue_buffer_write passing as buf argument an invalid const void* value 
// 
TEST_FUNCTION(io_queue_buffer_write__arg_buf_null)
{
    static prx_buffer_factory_t* k_factory_valid = (prx_buffer_factory_t*)0x1235;
    static void* k_buffer_valid = (void*)0x1254;
    static const size_t k_offset_valid = 1100;
    static const size_t k_size_valid = 1100;
    io_queue_buffer_t queue_buffer_struct_valid;
    io_queue_buffer_t *queue_buffer_valid = (io_queue_buffer_t*)&queue_buffer_struct_valid;
    int32_t result;

    queue_buffer_valid->length = 100;
    queue_buffer_valid->offset = k_offset_valid;
    queue_buffer_struct_valid.buf_obj = k_buffer_valid;
    queue_buffer_struct_valid.factory = k_factory_valid;
    queue_buffer_struct_valid.is_const = false;

    // arrange 
    STRICT_EXPECTED_CALL(io_queue_buffer_set_size(k_factory_valid, IGNORED_PTR_ARG, k_size_valid + k_offset_valid))
        .ValidateArgumentBuffer(2, &k_buffer_valid, sizeof(k_buffer_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(prx_buffer_factory_set_buffer(k_factory_valid, k_buffer_valid, k_offset_valid, NULL, k_size_valid));

    // act 
    result = io_queue_buffer_write(queue_buffer_valid, NULL, k_size_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_queue_buffer_write passing as len argument an invalid size_t value 
// 
TEST_FUNCTION(io_queue_buffer_write__arg_len_zero)
{
    static prx_buffer_factory_t* k_factory_valid = (prx_buffer_factory_t*)0x1235;
    static void* k_buffer_valid = (void*)0x1254;
    io_queue_buffer_t queue_buffer_struct_valid;
    io_queue_buffer_t *queue_buffer_valid = (io_queue_buffer_t*)&queue_buffer_struct_valid;
    int32_t result;

    queue_buffer_valid->length = 100;
    queue_buffer_valid->offset = 1;
    queue_buffer_struct_valid.buf_obj = k_buffer_valid;
    queue_buffer_struct_valid.factory = k_factory_valid;
    queue_buffer_struct_valid.is_const = true;

    // arrange 
    STRICT_EXPECTED_CALL(prx_buffer_factory_clone_buffer(k_factory_valid, k_buffer_valid, true, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_out(&k_buffer_valid, sizeof(k_buffer_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(prx_buffer_factory_free_buffer(k_factory_valid, k_buffer_valid));

    // act 
    result = io_queue_buffer_write(queue_buffer_valid, NULL, 0);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_queue_buffer_write unhappy path 
// 
TEST_FUNCTION(io_queue_buffer_write__neg)
{
    static prx_buffer_factory_t* k_factory_valid = (prx_buffer_factory_t*)0x1235;
    static void* k_buffer_valid = (void*)0x1254;
    static char in_valid[8] = { 't', 'e', 's', 't', 't', 'e', 's', 't' };
    static const size_t k_size_valid = sizeof(in_valid);
    io_queue_buffer_t queue_buffer_struct_valid;
    io_queue_buffer_t *queue_buffer_valid = (io_queue_buffer_t*)&queue_buffer_struct_valid;
    int32_t result;

    queue_buffer_struct_valid.buf_obj = k_buffer_valid;
    queue_buffer_struct_valid.factory = k_factory_valid;

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(prx_buffer_factory_clone_buffer(k_factory_valid, k_buffer_valid, true, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_out(&k_buffer_valid, sizeof(k_buffer_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL(prx_buffer_factory_free_buffer(k_factory_valid, k_buffer_valid));
    STRICT_EXPECTED_CALL(io_queue_buffer_set_size(k_factory_valid, IGNORED_PTR_ARG, k_size_valid))
        .IgnoreArgument(2)
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL(prx_buffer_factory_set_buffer(k_factory_valid, k_buffer_valid, 0, in_valid, k_size_valid));

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    queue_buffer_valid->length = 10000;
    queue_buffer_valid->offset = 0;
    queue_buffer_struct_valid.is_const = true;
    result = io_queue_buffer_write(queue_buffer_valid, in_valid, k_size_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_out_of_memory, er_ok, er_out_of_memory, er_ok);
}

// 
// Test io_queue_buffer_read happy path 
// 
TEST_FUNCTION(io_queue_buffer_read__success_1)
{
    static prx_buffer_factory_t* k_factory_valid = (prx_buffer_factory_t*)0x1235;
    static void* k_buffer_valid = (void*)0x1254;
    static const size_t k_len_valid = 199;
    static char k_buf_valid[100] = { 't', 'e', 's', 't', 't', 'e', 's', 't', 't', 0 };
    static const size_t k_size_valid = sizeof(k_buf_valid);
    static const size_t k_offset_valid = 90;
    uint8_t* buf_valid = (uint8_t*)UT_MEM;
    io_queue_buffer_t queue_buffer_struct_valid;
    io_queue_buffer_t *queue_buffer_valid = (io_queue_buffer_t*)&queue_buffer_struct_valid;
    size_t read_valid;
    int32_t result;

    queue_buffer_valid->length = 100;
    queue_buffer_valid->offset = k_offset_valid;
    queue_buffer_struct_valid.buf_obj = k_buffer_valid;
    queue_buffer_struct_valid.factory = k_factory_valid;

    // arrange 
    STRICT_EXPECTED_CALL(prx_buffer_factory_get_buffer(k_factory_valid, k_buffer_valid, k_offset_valid, NULL, NULL))
        .SetReturn(k_buf_valid);

    // act 
    result = io_queue_buffer_read(queue_buffer_valid, buf_valid, k_len_valid, &read_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(int32_t, 10, read_valid);
    ASSERT_ARE_EQUAL(char_ptr, "testtestt", (char*)buf_valid);
}

// 
// Test io_queue_buffer_read happy path 
// 
TEST_FUNCTION(io_queue_buffer_read__success_2)
{
    static prx_buffer_factory_t* k_factory_valid = (prx_buffer_factory_t*)0x1235;
    static void* k_buffer_valid = (void*)0x1254;
    static const size_t k_len_valid = 5;
    static char k_buf_valid[30] = { 't', 'e', 's', 't', 0 };
    static const size_t k_size_valid = sizeof(k_buf_valid);
    static const size_t k_offset_valid = 90;
    uint8_t* buf_valid = (uint8_t*)UT_MEM;
    io_queue_buffer_t queue_buffer_struct_valid;
    io_queue_buffer_t *queue_buffer_valid = (io_queue_buffer_t*)&queue_buffer_struct_valid;
    size_t read_valid;
    int32_t result;

    queue_buffer_valid->length = 100;
    queue_buffer_valid->offset = k_offset_valid;
    queue_buffer_struct_valid.buf_obj = k_buffer_valid;
    queue_buffer_struct_valid.factory = k_factory_valid;

    // arrange 
    STRICT_EXPECTED_CALL(prx_buffer_factory_get_buffer(k_factory_valid, k_buffer_valid, k_offset_valid, NULL, NULL))
        .SetReturn(k_buf_valid);

    // act 
    result = io_queue_buffer_read(queue_buffer_valid, buf_valid, k_len_valid, &read_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(int32_t, 5, read_valid);
    ASSERT_ARE_EQUAL(char_ptr, "test", (char*)buf_valid);
}

// 
// Test io_queue_buffer_read passing as queue_buffer argument an invalid io_queue_buffer_t* value 
// 
TEST_FUNCTION(io_queue_buffer_read__arg_queue_buffer_invalid)
{
    static const size_t k_len_valid = 5;
    uint8_t* buf_valid = (uint8_t*)UT_MEM;
    size_t read_valid;
    int32_t result;

    // arrange 

    // act 
    result = io_queue_buffer_read(NULL, buf_valid, k_len_valid, &read_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_queue_buffer_read passing as buf argument an invalid void* value 
// 
TEST_FUNCTION(io_queue_buffer_read__arg_buf_invalid)
{
    static prx_buffer_factory_t* k_factory_valid = (prx_buffer_factory_t*)0x1235;
    static void* k_buffer_valid = (void*)0x1254;
    static const size_t k_len_valid = 5;
    io_queue_buffer_t queue_buffer_struct_valid;
    io_queue_buffer_t *queue_buffer_valid = (io_queue_buffer_t*)&queue_buffer_struct_valid;
    size_t read_valid;
    int32_t result;

    queue_buffer_valid->length = 100;
    queue_buffer_valid->offset = 90;
    queue_buffer_struct_valid.buf_obj = k_buffer_valid;
    queue_buffer_struct_valid.factory = k_factory_valid;

    // arrange 

    // act 
    result = io_queue_buffer_read(queue_buffer_valid, NULL, k_len_valid, &read_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_queue_buffer_read passing as len argument an invalid size_t value 
// 
TEST_FUNCTION(io_queue_buffer_read__arg_len_invalid)
{
    static prx_buffer_factory_t* k_factory_valid = (prx_buffer_factory_t*)0x1235;
    static void* k_buffer_valid = (void*)0x1254;
    uint8_t* buf_valid = (uint8_t*)UT_MEM;
    io_queue_buffer_t queue_buffer_struct_valid;
    io_queue_buffer_t *queue_buffer_valid = (io_queue_buffer_t*)&queue_buffer_struct_valid;
    size_t read_valid;
    int32_t result;

    queue_buffer_valid->length = 100;
    queue_buffer_valid->offset = 90;
    queue_buffer_struct_valid.buf_obj = k_buffer_valid;
    queue_buffer_struct_valid.factory = k_factory_valid;

    // arrange 

    // act 
    result = io_queue_buffer_read(queue_buffer_valid, buf_valid, 0, &read_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_queue_buffer_read passing as read argument an invalid size_t* value 
// 
TEST_FUNCTION(io_queue_buffer_read__arg_read_invalid)
{
    static prx_buffer_factory_t* k_factory_valid = (prx_buffer_factory_t*)0x1235;
    static void* k_buffer_valid = (void*)0x1254;
    static const size_t k_len_valid = 5;
    uint8_t* buf_valid = (uint8_t*)UT_MEM;
    io_queue_buffer_t queue_buffer_struct_valid;
    io_queue_buffer_t *queue_buffer_valid = (io_queue_buffer_t*)&queue_buffer_struct_valid;
    int32_t result;

    queue_buffer_valid->length = 100;
    queue_buffer_valid->offset = 90;
    queue_buffer_struct_valid.buf_obj = k_buffer_valid;
    queue_buffer_struct_valid.factory = k_factory_valid;

    // arrange 

    // act 
    result = io_queue_buffer_read(queue_buffer_valid, buf_valid, k_len_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_queue_create happy path 
// 
TEST_FUNCTION(io_queue_create__success)
{
    static lock_t k_lock_valid = (lock_t)0x1;
    static void* k_context_valid = (void*)0x235234;
    io_queue_t* queue_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_queue_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(DList_InitializeListHead(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(DList_InitializeListHead(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(lock_create(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_created(&k_lock_valid, sizeof(k_lock_valid))
        .SetReturn(er_ok);

    // act 
    result = io_queue_create(k_context_valid, &queue_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(void_ptr, UT_MEM, queue_valid);
    ASSERT_ARE_EQUAL(void_ptr, k_context_valid, queue_valid->context);
}

// 
// Test io_queue_create passing as context argument an invalid void* value 
// 
TEST_FUNCTION(io_queue_create__arg_context_null)
{
    static lock_t k_lock_valid = (lock_t)0x1;
    io_queue_t* queue_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_queue_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(DList_InitializeListHead(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(DList_InitializeListHead(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(lock_create(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_created(&k_lock_valid, sizeof(k_lock_valid))
        .SetReturn(er_ok);

    // act 
    result = io_queue_create(NULL, &queue_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(void_ptr, UT_MEM, queue_valid);
    ASSERT_ARE_EQUAL(void_ptr, NULL, queue_valid->context);
}

// 
// Test io_queue_create passing as queue argument an invalid io_queue_t** value 
// 
TEST_FUNCTION(io_queue_create__arg_queue_invalid)
{
    static lock_t k_lock_valid = (lock_t)0x1;
    static void* k_context_valid = (void*)0x235234;
    int32_t result;

    // arrange 

    // act 
    result = io_queue_create(k_context_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_queue_create unhappy path 
// 
TEST_FUNCTION(io_queue_create__neg)
{
    static lock_t k_lock_valid = (lock_t)0x1;
    static void* k_context_valid = (void*)0x235234;
    io_queue_t* queue_valid;
    int32_t result;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_queue_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(DList_InitializeListHead(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(DList_InitializeListHead(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(lock_create(IGNORED_PTR_ARG))
        .IgnoreArgument(1)
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL(h_free((void*)UT_MEM, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    memset(UT_MEM, 0, sizeof(UT_MEM));
    result = io_queue_create(k_context_valid, &queue_valid);

    // assert    
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_out_of_memory, er_ok, er_ok, er_out_of_memory, er_ok);
}

// 
// Test io_queue_free happy path 
// 
TEST_FUNCTION(io_queue_free__success_1)
{
    io_queue_t queue_valid;

    queue_valid.queue_lock = (lock_t)0x1;

    // arrange 
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));
    STRICT_EXPECTED_CALL(DList_IsListEmpty(IGNORED_PTR_ARG))
        .IgnoreArgument(1)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(DList_InitializeListHead(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(DList_IsListEmpty(IGNORED_PTR_ARG))
        .IgnoreArgument(1)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(DList_InitializeListHead(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));
    STRICT_EXPECTED_CALL(lock_free((lock_t)0x1));
    STRICT_EXPECTED_CALL(h_free((void*)&queue_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    io_queue_free(&queue_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test io_queue_free happy path 
// 
TEST_FUNCTION(io_queue_free__success_2)
{
    io_queue_buffer_t queue_buffers_valid[5];
    io_queue_t queue_valid;

    queue_valid.ready_list.Flink = &queue_buffers_valid[0].qlink;
    queue_valid.ready_list.Blink = &queue_buffers_valid[_countof(queue_buffers_valid) - 1].qlink;
    for (size_t i = 0; i < _countof(queue_buffers_valid); i++)
    {
        queue_buffers_valid[i].qlink.Blink = i > 0 ? &queue_buffers_valid[i - 1].qlink : &queue_valid.ready_list;
        queue_buffers_valid[i].qlink.Flink = i < _countof(queue_buffers_valid) - 1 ? &queue_buffers_valid[i + 1].qlink : &queue_valid.ready_list;
    }
    queue_valid.queue_lock = (lock_t)0x1;

    // arrange 
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));

    STRICT_EXPECTED_CALL(DList_IsListEmpty(IGNORED_PTR_ARG))
        .IgnoreArgument(1)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(DList_InitializeListHead(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    for (size_t i = 0; i < _countof(queue_buffers_valid); i++)
    {
        STRICT_EXPECTED_CALL(DList_IsListEmpty(IGNORED_PTR_ARG))
            .IgnoreArgument(1)
            .SetReturn(0);
        STRICT_EXPECTED_CALL(DList_RemoveHeadList(IGNORED_PTR_ARG))
            .IgnoreArgument(1)
            .SetReturn(&queue_buffers_valid[i].qlink);
        STRICT_EXPECTED_CALL(io_queue_buffer_release(&queue_buffers_valid[i]));
    }
    STRICT_EXPECTED_CALL(DList_IsListEmpty(IGNORED_PTR_ARG))
        .IgnoreArgument(1)
        .SetReturn(1);

    STRICT_EXPECTED_CALL(DList_InitializeListHead(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));
    STRICT_EXPECTED_CALL(lock_free((lock_t)0x1));
    STRICT_EXPECTED_CALL(h_free((void*)&queue_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    io_queue_free(&queue_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test io_queue_free happy path 
// 
TEST_FUNCTION(io_queue_free__success_3)
{
    io_queue_buffer_t queue_buffers_valid[5];
    io_queue_buffer_t inprogress_valid[10];
    io_queue_t queue_valid;

    queue_valid.inprogress_list.Flink = &inprogress_valid[0].qlink;
    queue_valid.inprogress_list.Blink = &inprogress_valid[_countof(inprogress_valid) - 1].qlink;
    for (size_t i = 0; i < _countof(inprogress_valid); i++)
    {
        inprogress_valid[i].qlink.Blink = i > 0 ? &inprogress_valid[i - 1].qlink : &queue_valid.inprogress_list;
        inprogress_valid[i].qlink.Flink = i < _countof(inprogress_valid) - 1 ? &inprogress_valid[i + 1].qlink : &queue_valid.inprogress_list;
    }
    queue_valid.ready_list.Flink = &queue_buffers_valid[0].qlink;
    queue_valid.ready_list.Blink = &queue_buffers_valid[_countof(queue_buffers_valid) - 1].qlink;
    for (size_t i = 0; i < _countof(queue_buffers_valid); i++)
    {
        queue_buffers_valid[i].qlink.Blink = i > 0 ? &queue_buffers_valid[i - 1].qlink : &queue_valid.ready_list;
        queue_buffers_valid[i].qlink.Flink = i < _countof(queue_buffers_valid) - 1 ? &queue_buffers_valid[i + 1].qlink : &queue_valid.ready_list;
    }
    queue_valid.queue_lock = (lock_t)0x1;

    // arrange 
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));

    for (size_t i = 0; i < _countof(inprogress_valid); i++)
    {
        STRICT_EXPECTED_CALL(DList_IsListEmpty(IGNORED_PTR_ARG))
            .IgnoreArgument(1)
            .SetReturn(0);
        STRICT_EXPECTED_CALL(DList_RemoveHeadList(IGNORED_PTR_ARG))
            .IgnoreArgument(1)
            .SetReturn(&inprogress_valid[i].qlink);
        STRICT_EXPECTED_CALL(io_queue_buffer_release(&inprogress_valid[i]));
    }
    STRICT_EXPECTED_CALL(DList_IsListEmpty(IGNORED_PTR_ARG))
        .IgnoreArgument(1)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(DList_InitializeListHead(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    for (size_t i = 0; i < _countof(queue_buffers_valid); i++)
    {
        STRICT_EXPECTED_CALL(DList_IsListEmpty(IGNORED_PTR_ARG))
            .IgnoreArgument(1)
            .SetReturn(0);
        STRICT_EXPECTED_CALL(DList_RemoveHeadList(IGNORED_PTR_ARG))
            .IgnoreArgument(1)
            .SetReturn(&queue_buffers_valid[i].qlink);
        STRICT_EXPECTED_CALL(io_queue_buffer_release(&queue_buffers_valid[i]));
    }
    STRICT_EXPECTED_CALL(DList_IsListEmpty(IGNORED_PTR_ARG))
        .IgnoreArgument(1)
        .SetReturn(1);

    STRICT_EXPECTED_CALL(DList_InitializeListHead(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));
    STRICT_EXPECTED_CALL(lock_free((lock_t)0x1));
    STRICT_EXPECTED_CALL(h_free((void*)&queue_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    io_queue_free(&queue_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test io_queue_free passing as queue argument an invalid io_queue_t* value 
// 
TEST_FUNCTION(io_queue_free__arg_queue_invalid)
{
    // arrange 

    // act 
    io_queue_free(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test io_queue_has_ready happy path 
// 
TEST_FUNCTION(io_queue_has_ready__success_1)
{
    io_queue_t queue_valid;
    bool result;

    queue_valid.queue_lock = (lock_t)0x1;

    // arrange 
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));
    STRICT_EXPECTED_CALL(DList_IsListEmpty(IGNORED_PTR_ARG))
        .IgnoreArgument(1)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));

    // act 
    result = io_queue_has_ready(&queue_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_FALSE(result);
}

// 
// Test io_queue_has_ready happy path 
// 
TEST_FUNCTION(io_queue_has_ready__success_2)
{
    io_queue_t queue_valid;
    bool result;

    queue_valid.queue_lock = (lock_t)0x1;

    // arrange 
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));
    STRICT_EXPECTED_CALL(DList_IsListEmpty(IGNORED_PTR_ARG))
        .IgnoreArgument(1)
        .SetReturn(0);
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));

    // act 
    result = io_queue_has_ready(&queue_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_TRUE(result);
}

// 
// Test io_queue_has_ready passing as queue argument an invalid io_queue_t* value 
// 
TEST_FUNCTION(io_queue_has_ready__arg_queue_invalid)
{
    bool result;

    // arrange 

    // act 
    result = io_queue_has_ready(NULL);

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_FALSE(result);
}

// 
// Test io_queue_pop_ready happy path 
// 
TEST_FUNCTION(io_queue_pop_ready__success_1)
{
    io_queue_buffer_t queue_buffer_valid;
    io_queue_t queue_valid;
    io_queue_buffer_t* result;

    queue_valid.queue_lock = (lock_t)0x1;

    // arrange 
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));
    STRICT_EXPECTED_CALL(DList_IsListEmpty(IGNORED_PTR_ARG))
        .IgnoreArgument(1)
        .SetReturn(0);
    STRICT_EXPECTED_CALL(DList_RemoveHeadList(IGNORED_PTR_ARG))
        .IgnoreArgument(1)
        .SetReturn(&queue_buffer_valid.qlink);
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));

    // act 
    result = io_queue_pop_ready(&queue_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void_ptr, (void*)&queue_buffer_valid, (void*)result);
}

// 
// Test io_queue_pop_ready happy path 
// 
TEST_FUNCTION(io_queue_pop_ready__success_2)
{
    io_queue_t queue_valid;
    io_queue_buffer_t* result;

    queue_valid.queue_lock = (lock_t)0x1;

    // arrange 
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));
    STRICT_EXPECTED_CALL(DList_IsListEmpty(IGNORED_PTR_ARG))
        .IgnoreArgument(1)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));

    // act 
    result = io_queue_pop_ready(&queue_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_NULL(result);
}

// 
// Test io_queue_pop_ready passing as queue argument an invalid io_queue_t* value 
// 
TEST_FUNCTION(io_queue_pop_ready__arg_queue_null)
{
    io_queue_buffer_t* result;

    // arrange 

    // act 
    result = io_queue_pop_ready(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_NULL(result);
}

// 
// Test io_queue_has_inprogress happy path 
// 
TEST_FUNCTION(io_queue_has_inprogress__success_1)
{
    io_queue_t queue_valid;
    bool result;

    queue_valid.queue_lock = (lock_t)0x1;

    // arrange 
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));
    STRICT_EXPECTED_CALL(DList_IsListEmpty(IGNORED_PTR_ARG))
        .IgnoreArgument(1)
        .SetReturn(0);
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));

    // act 
    result = io_queue_has_inprogress(&queue_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_TRUE(result);
}

// 
// Test io_queue_has_inprogress happy path 
// 
TEST_FUNCTION(io_queue_has_inprogress__success_2)
{
    io_queue_t queue_valid;
    bool result;

    queue_valid.queue_lock = (lock_t)0x1;

    // arrange 
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));
    STRICT_EXPECTED_CALL(DList_IsListEmpty(IGNORED_PTR_ARG))
        .IgnoreArgument(1)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));

    // act 
    result = io_queue_has_inprogress(&queue_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_FALSE(result);
}

// 
// Test io_queue_has_inprogress passing as queue argument an invalid io_queue_t* value 
// 
TEST_FUNCTION(io_queue_has_inprogress__arg_queue_null)
{
    bool result;

    // arrange 

    // act 
    result = io_queue_has_inprogress(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_FALSE(result);
}

// 
// Test io_queue_pop_inprogress happy path 
// 
TEST_FUNCTION(io_queue_pop_inprogress__success_1)
{
    io_queue_buffer_t queue_buffer_valid;
    io_queue_t queue_valid;
    io_queue_buffer_t* result;

    queue_valid.queue_lock = (lock_t)0x1;

    // arrange 
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));
    STRICT_EXPECTED_CALL(DList_IsListEmpty(IGNORED_PTR_ARG))
        .IgnoreArgument(1)
        .SetReturn(0);
    STRICT_EXPECTED_CALL(DList_RemoveHeadList(IGNORED_PTR_ARG))
        .IgnoreArgument(1)
        .SetReturn(&queue_buffer_valid.qlink);
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));

    // act 
    result = io_queue_pop_inprogress(&queue_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void_ptr, (void*)&queue_buffer_valid, (void*)result);
}

// 
// Test io_queue_pop_inprogress happy path 
// 
TEST_FUNCTION(io_queue_pop_inprogress__success_2)
{
    io_queue_t queue_valid;
    io_queue_buffer_t* result;

    queue_valid.queue_lock = (lock_t)0x1;

    // arrange 
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));
    STRICT_EXPECTED_CALL(DList_IsListEmpty(IGNORED_PTR_ARG))
        .IgnoreArgument(1)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));

    // act 
    result = io_queue_pop_inprogress(&queue_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_NULL(result);
}

// 
// Test io_queue_pop_inprogress passing as queue argument an invalid io_queue_t* value 
// 
TEST_FUNCTION(io_queue_pop_inprogress__arg_queue_null)
{
    io_queue_buffer_t* result;

    // arrange 

    // act 
    result = io_queue_pop_inprogress(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_NULL(result);
}

// 
// Test io_queue_rollback happy path 
// 
TEST_FUNCTION(io_queue_rollback__success)
{
    io_queue_t queue_valid;

    queue_valid.queue_lock = (lock_t)0x1;

    // arrange 
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));
    STRICT_EXPECTED_CALL(DList_AppendTailList(queue_valid.ready_list.Flink, &queue_valid.inprogress_list));
    STRICT_EXPECTED_CALL(DList_RemoveEntryList(&queue_valid.inprogress_list));
    STRICT_EXPECTED_CALL(DList_InitializeListHead(&queue_valid.inprogress_list));
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));

    // act 
    io_queue_rollback(&queue_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test io_queue_rollback passing as queue argument an invalid io_queue_t* value 
// 
TEST_FUNCTION(io_queue_rollback__arg_queue_invalid)
{
    // arrange 

    // act 
    io_queue_rollback(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test io_queue_reset happy path 
// 
TEST_FUNCTION(io_queue_reset__success_1)
{
    io_queue_buffer_t queue_buffers_valid[15];
    io_queue_buffer_t inprogress_valid[10];
    io_queue_t queue_valid;

    queue_valid.inprogress_list.Flink = &inprogress_valid[0].qlink;
    queue_valid.inprogress_list.Blink = &inprogress_valid[_countof(inprogress_valid) - 1].qlink;
    for (size_t i = 0; i < _countof(inprogress_valid); i++)
    {
        inprogress_valid[i].qlink.Blink = i > 0 ? &inprogress_valid[i - 1].qlink : &queue_valid.inprogress_list;
        inprogress_valid[i].qlink.Flink = i < _countof(inprogress_valid) - 1 ? &inprogress_valid[i + 1].qlink : &queue_valid.inprogress_list;
    }
    queue_valid.ready_list.Flink = &queue_buffers_valid[0].qlink;
    queue_valid.ready_list.Blink = &queue_buffers_valid[_countof(queue_buffers_valid) - 1].qlink;
    for (size_t i = 0; i < _countof(queue_buffers_valid); i++)
    {
        queue_buffers_valid[i].qlink.Blink = i > 0 ? &queue_buffers_valid[i - 1].qlink : &queue_valid.ready_list;
        queue_buffers_valid[i].qlink.Flink = i < _countof(queue_buffers_valid) - 1 ? &queue_buffers_valid[i + 1].qlink : &queue_valid.ready_list;
    }
    queue_valid.queue_lock = (lock_t)0x1;

    // arrange 
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));

    for (size_t i = 0; i < _countof(inprogress_valid); i++)
    {
        STRICT_EXPECTED_CALL(DList_IsListEmpty(IGNORED_PTR_ARG))
            .IgnoreArgument(1)
            .SetReturn(0);
        STRICT_EXPECTED_CALL(DList_RemoveHeadList(IGNORED_PTR_ARG))
            .IgnoreArgument(1)
            .SetReturn(&inprogress_valid[i].qlink);
        STRICT_EXPECTED_CALL(io_queue_buffer_release(&inprogress_valid[i]));
    }
    STRICT_EXPECTED_CALL(DList_IsListEmpty(IGNORED_PTR_ARG))
        .IgnoreArgument(1)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(DList_InitializeListHead(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    for (size_t i = 0; i < _countof(queue_buffers_valid); i++)
    {
        STRICT_EXPECTED_CALL(DList_IsListEmpty(IGNORED_PTR_ARG))
            .IgnoreArgument(1)
            .SetReturn(0);
        STRICT_EXPECTED_CALL(DList_RemoveHeadList(IGNORED_PTR_ARG))
            .IgnoreArgument(1)
            .SetReturn(&queue_buffers_valid[i].qlink);
        STRICT_EXPECTED_CALL(io_queue_buffer_release(&queue_buffers_valid[i]));
    }
    STRICT_EXPECTED_CALL(DList_IsListEmpty(IGNORED_PTR_ARG))
        .IgnoreArgument(1)
        .SetReturn(1);

    STRICT_EXPECTED_CALL(DList_InitializeListHead(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));

    // act 
    io_queue_reset(&queue_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test io_queue_reset happy path 
// 
TEST_FUNCTION(io_queue_reset__success_2)
{
    io_queue_buffer_t inprogress_valid[5];
    io_queue_t queue_valid;

    queue_valid.inprogress_list.Flink = &inprogress_valid[0].qlink;
    queue_valid.inprogress_list.Blink = &inprogress_valid[_countof(inprogress_valid) - 1].qlink;
    for (size_t i = 0; i < _countof(inprogress_valid); i++)
    {
        inprogress_valid[i].qlink.Blink = i > 0 ? &inprogress_valid[i - 1].qlink : &queue_valid.inprogress_list;
        inprogress_valid[i].qlink.Flink = i < _countof(inprogress_valid) - 1 ? &inprogress_valid[i + 1].qlink : &queue_valid.inprogress_list;
    }
    queue_valid.ready_list.Flink = queue_valid.ready_list.Blink = &queue_valid.ready_list;
    queue_valid.queue_lock = (lock_t)0x1;

    // arrange 
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));

    for (size_t i = 0; i < _countof(inprogress_valid); i++)
    {
        STRICT_EXPECTED_CALL(DList_IsListEmpty(IGNORED_PTR_ARG))
            .IgnoreArgument(1)
            .SetReturn(0);
        STRICT_EXPECTED_CALL(DList_RemoveHeadList(IGNORED_PTR_ARG))
            .IgnoreArgument(1)
            .SetReturn(&inprogress_valid[i].qlink);
        STRICT_EXPECTED_CALL(io_queue_buffer_release(&inprogress_valid[i]));
    }
    STRICT_EXPECTED_CALL(DList_IsListEmpty(IGNORED_PTR_ARG))
        .IgnoreArgument(1)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(DList_InitializeListHead(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(DList_IsListEmpty(IGNORED_PTR_ARG))
        .IgnoreArgument(1)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(DList_InitializeListHead(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));

    // act 
    io_queue_reset(&queue_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test io_queue_reset happy path 
// 
TEST_FUNCTION(io_queue_reset__success_3)
{
    io_queue_buffer_t queue_buffers_valid[15];
    io_queue_t queue_valid;

    queue_valid.inprogress_list.Flink = queue_valid.inprogress_list.Blink = &queue_valid.inprogress_list;
    queue_valid.ready_list.Flink = &queue_buffers_valid[0].qlink;
    queue_valid.ready_list.Blink = &queue_buffers_valid[_countof(queue_buffers_valid) - 1].qlink;
    for (size_t i = 0; i < _countof(queue_buffers_valid); i++)
    {
        queue_buffers_valid[i].qlink.Blink = i > 0 ? &queue_buffers_valid[i - 1].qlink : &queue_valid.ready_list;
        queue_buffers_valid[i].qlink.Flink = i < _countof(queue_buffers_valid) - 1 ? &queue_buffers_valid[i + 1].qlink : &queue_valid.ready_list;
    }
    queue_valid.queue_lock = (lock_t)0x1;

    // arrange 
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));

    STRICT_EXPECTED_CALL(DList_IsListEmpty(IGNORED_PTR_ARG))
        .IgnoreArgument(1)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(DList_InitializeListHead(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    for (size_t i = 0; i < _countof(queue_buffers_valid); i++)
    {
        STRICT_EXPECTED_CALL(DList_IsListEmpty(IGNORED_PTR_ARG))
            .IgnoreArgument(1)
            .SetReturn(0);
        STRICT_EXPECTED_CALL(DList_RemoveHeadList(IGNORED_PTR_ARG))
            .IgnoreArgument(1)
            .SetReturn(&queue_buffers_valid[i].qlink);
        STRICT_EXPECTED_CALL(io_queue_buffer_release(&queue_buffers_valid[i]));
    }
    STRICT_EXPECTED_CALL(DList_IsListEmpty(IGNORED_PTR_ARG))
        .IgnoreArgument(1)
        .SetReturn(1);

    STRICT_EXPECTED_CALL(DList_InitializeListHead(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));

    // act 
    io_queue_reset(&queue_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test io_queue_reset happy path 
// 
TEST_FUNCTION(io_queue_reset__success_4)
{
    io_queue_t queue_valid;

    queue_valid.inprogress_list.Flink = queue_valid.inprogress_list.Blink = &queue_valid.inprogress_list;
    queue_valid.ready_list.Flink = queue_valid.ready_list.Blink = &queue_valid.ready_list;
    queue_valid.queue_lock = (lock_t)0x1;

    // arrange 
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));

    STRICT_EXPECTED_CALL(DList_IsListEmpty(IGNORED_PTR_ARG))
        .IgnoreArgument(1)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(DList_InitializeListHead(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(DList_IsListEmpty(IGNORED_PTR_ARG))
        .IgnoreArgument(1)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(DList_InitializeListHead(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));

    // act 
    io_queue_reset(&queue_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
}


// 
// Test io_queue_reset passing as queue argument an invalid io_queue_t* value 
// 
TEST_FUNCTION(io_queue_reset__arg_queue_invalid)
{
    // arrange 

    // act 
    io_queue_reset(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test io_queue_buffer_set_ready happy path 
// 
TEST_FUNCTION(io_queue_add__success)
{
    io_queue_t queue_valid;
    io_queue_buffer_t queue_buffer_valid;
    int32_t result;

    queue_valid.queue_lock = (lock_t)0x1;

    // arrange 
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));
    STRICT_EXPECTED_CALL(DList_InsertTailList(&queue_valid.ready_list, &queue_buffer_valid.qlink));
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));

    // act 
    result = io_queue_buffer_set_ready(&queue_valid, &queue_buffer_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_queue_buffer_set_ready passing as queue argument an invalid io_queue_t* value 
// 
TEST_FUNCTION(io_queue_add__arg_queue_null)
{
    io_queue_buffer_t queue_buffer_valid;
    int32_t result;

    // arrange 

    // act 
    result = io_queue_buffer_set_ready(NULL, &queue_buffer_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_queue_buffer_set_ready passing as queue_buffer argument an invalid io_queue_buffer_t* value 
// 
TEST_FUNCTION(io_queue_add__arg_queue_buffer_null)
{
    io_queue_t queue_valid;
    int32_t result;

    queue_valid.queue_lock = (lock_t)0x1;

    // arrange 

    // act 
    result = io_queue_buffer_set_ready(&queue_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_queue_buffer_set_ready unhappy path 
// 
TEST_FUNCTION(io_queue_add__neg)
{
    io_queue_t queue_valid;
    io_queue_buffer_t queue_buffer_valid;
    int32_t result;

    queue_valid.queue_lock = (lock_t)0x1;

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));
    STRICT_EXPECTED_CALL(DList_InsertTailList(&queue_valid.ready_list, &queue_buffer_valid.qlink));
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_queue_buffer_set_ready(&queue_valid, &queue_buffer_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

// 
// Test io_queue_buffer_set_inprogress happy path 
// 
TEST_FUNCTION(io_queue_buffer_set_inprogress__success)
{
    io_queue_t queue_valid;
    io_queue_buffer_t queue_buffer_valid;
    int32_t result;

    queue_valid.queue_lock = (lock_t)0x1;
    queue_buffer_valid.queue = &queue_valid;

    // arrange 
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));
    STRICT_EXPECTED_CALL(DList_InsertTailList(&queue_valid.inprogress_list, &queue_buffer_valid.qlink));
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));

    // act 
    result = io_queue_buffer_set_inprogress(&queue_buffer_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test io_queue_buffer_set_inprogress passing as queue_buffer argument an invalid io_queue_buffer_t* value 
// 
TEST_FUNCTION(io_queue_buffer_set_inprogress__arg_queue_buffer_null)
{
    int32_t result;

    // arrange 

    // act 
    result = io_queue_buffer_set_inprogress(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test io_queue_buffer_set_inprogress passing as queue_buffer argument an invalid io_queue_buffer_t* value 
// 
TEST_FUNCTION(io_queue_buffer_set_inprogress__arg_queue_buffer_invalid)
{
    io_queue_buffer_t queue_buffer_valid;
    int32_t result;

    queue_buffer_valid.queue = NULL;

    // arrange 

    // act 
    result = io_queue_buffer_set_inprogress(&queue_buffer_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_arg, result);
}

#endif

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

