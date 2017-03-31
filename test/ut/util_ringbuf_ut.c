// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST util_ringbuf
#include "util_ut.h"

//
// 1. Required mocks
//

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
REGISTER_UMOCK_ALIAS_TYPE(rwlock_t, void*);
END_DECLARE_TEST_SUITE()
// -or- DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
BEGIN_DECLARE_TEST_SETUP()
END_DECLARE_TEST_SETUP()

#ifdef ring_buffer_create 

//
// Test ring_buffer_create happy path 
// 
TEST_FUNCTION(ring_buffer_create__success)
{
    static const size_t k_size_valid;
    static const ring_buffer_t** k_rb_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = ring_buffer_create(k_size_valid, k_rb_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test ring_buffer_create passing as size argument an invalid size_t value 
// 
TEST_FUNCTION(ring_buffer_create__arg_size_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = ring_buffer_create();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test ring_buffer_create passing as rb argument an invalid ring_buffer_t** value 
// 
TEST_FUNCTION(ring_buffer_create__arg_rb_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = ring_buffer_create();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test ring_buffer_create unhappy path 
// 
TEST_FUNCTION(ring_buffer_create__neg)
{
    static const size_t k_size_valid;
    static const ring_buffer_t** k_rb_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = ring_buffer_create(k_size_valid, k_rb_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif //  ring_buffer_create;

#ifdef ring_buffer_is_empty 
// 
// Test ring_buffer_is_empty happy path 
// 
TEST_FUNCTION(ring_buffer_is_empty__success)
{
    static const ring_buffer_t* k_rb_valid;
    bool result;

    // arrange 
    // ... 

    // act 
    result = ring_buffer_is_empty(k_rb_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(bool, er_ok, result);
    // ... 
}

// 
// Test ring_buffer_is_empty passing as rb argument an invalid ring_buffer_t* value 
// 
TEST_FUNCTION(ring_buffer_is_empty__arg_rb_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = ring_buffer_is_empty();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test ring_buffer_is_empty unhappy path 
// 
TEST_FUNCTION(ring_buffer_is_empty__neg)
{
    static const ring_buffer_t* k_rb_valid;
    bool result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = ring_buffer_is_empty(k_rb_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(bool, result, er_ok);
}

#endif //  ring_buffer_is_empty;

#ifdef ring_buffer_clear
// 
// Test ring_buffer_clear happy path 
// 
TEST_FUNCTION(ring_buffer_clear__success)
{
    static const ring_buffer_t* k_rb_valid;
    void result;

    // arrange 
    // ... 

    // act 
    result = ring_buffer_clear(k_rb_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ... 
}

// 
// Test ring_buffer_clear passing as rb argument an invalid ring_buffer_t* value 
// 
TEST_FUNCTION(ring_buffer_clear__arg_rb_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = ring_buffer_clear();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test ring_buffer_clear unhappy path 
// 
TEST_FUNCTION(ring_buffer_clear__neg)
{
    static const ring_buffer_t* k_rb_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = ring_buffer_clear(k_rb_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif //  ring_buffer_clear;


#ifdef ring_buffer_capacity
// 
// Test ring_buffer_capacity happy path 
// 
TEST_FUNCTION(ring_buffer_capacity__success)
{
    static const ring_buffer_t* k_rb_valid;
    size_t result;

    // arrange 
    // ... 

    // act 
    result = ring_buffer_capacity(k_rb_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(size_t, er_ok, result);
    // ... 
}

// 
// Test ring_buffer_capacity passing as rb argument an invalid ring_buffer_t* value 
// 
TEST_FUNCTION(ring_buffer_capacity__arg_rb_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = ring_buffer_capacity();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test ring_buffer_capacity unhappy path 
// 
TEST_FUNCTION(ring_buffer_capacity__neg)
{
    static const ring_buffer_t* k_rb_valid;
    size_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = ring_buffer_capacity(k_rb_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(size_t, result, er_ok);
}

#endif //  ring_buffer_capacity;


#ifdef ring_buffer_write
// 
// Test ring_buffer_write happy path 
// 
TEST_FUNCTION(ring_buffer_write__success)
{
    static const ring_buffer_t* k_rb_valid;
    static const const uint8_t* k_b_valid;
    static const size_t k_len_valid;
    size_t result;

    // arrange 
    // ... 

    // act 
    result = ring_buffer_write(k_rb_valid, k_b_valid, k_len_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(size_t, er_ok, result);
    // ... 
}

// 
// Test ring_buffer_write passing as rb argument an invalid ring_buffer_t* value 
// 
TEST_FUNCTION(ring_buffer_write__arg_rb_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = ring_buffer_write();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test ring_buffer_write passing as b argument an invalid const uint8_t* value 
// 
TEST_FUNCTION(ring_buffer_write__arg_b_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = ring_buffer_write();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test ring_buffer_write passing as len argument an invalid size_t value 
// 
TEST_FUNCTION(ring_buffer_write__arg_len_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = ring_buffer_write();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test ring_buffer_write unhappy path 
// 
TEST_FUNCTION(ring_buffer_write__neg)
{
    static const ring_buffer_t* k_rb_valid;
    static const const uint8_t* k_b_valid;
    static const size_t k_len_valid;
    size_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = ring_buffer_write(k_rb_valid, k_b_valid, k_len_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(size_t, result, er_ok);
}

#endif //  ring_buffer_write;

#ifdef ring_buffer_available
// 
// Test ring_buffer_available happy path 
// 
TEST_FUNCTION(ring_buffer_available__success)
{
    static const ring_buffer_t* k_rb_valid;
    size_t result;

    // arrange 
    // ... 

    // act 
    result = ring_buffer_available(k_rb_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(size_t, er_ok, result);
    // ... 
}

// 
// Test ring_buffer_available passing as rb argument an invalid ring_buffer_t* value 
// 
TEST_FUNCTION(ring_buffer_available__arg_rb_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = ring_buffer_available();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test ring_buffer_available unhappy path 
// 
TEST_FUNCTION(ring_buffer_available__neg)
{
    static const ring_buffer_t* k_rb_valid;
    size_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = ring_buffer_available(k_rb_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(size_t, result, er_ok);
}

#endif //  ring_buffer_available;

#ifdef ring_buffer_read 

// 
// Test ring_buffer_read happy path 
// 
TEST_FUNCTION(ring_buffer_read__success)
{
    static const ring_buffer_t* k_rb_valid;
    static const uint8_t* k_b_valid;
    static const size_t k_len_valid;
    size_t result;

    // arrange 
    // ... 

    // act 
    result = ring_buffer_read(k_rb_valid, k_b_valid, k_len_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(size_t, er_ok, result);
    // ... 
}

// 
// Test ring_buffer_read passing as rb argument an invalid ring_buffer_t* value 
// 
TEST_FUNCTION(ring_buffer_read__arg_rb_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = ring_buffer_read();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test ring_buffer_read passing as b argument an invalid uint8_t* value 
// 
TEST_FUNCTION(ring_buffer_read__arg_b_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = ring_buffer_read();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test ring_buffer_read passing as len argument an invalid size_t value 
// 
TEST_FUNCTION(ring_buffer_read__arg_len_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = ring_buffer_read();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test ring_buffer_read unhappy path 
// 
TEST_FUNCTION(ring_buffer_read__neg)
{
    static const ring_buffer_t* k_rb_valid;
    static const uint8_t* k_b_valid;
    static const size_t k_len_valid;
    size_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = ring_buffer_read(k_rb_valid, k_b_valid, k_len_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(size_t, result, er_ok);
}

#endif //  ring_buffer_read;

#ifdef ring_buffer_free 
// 
// Test ring_buffer_free happy path 
// 
TEST_FUNCTION(ring_buffer_free__success)
{
    static const ring_buffer_t* k_rb_valid;
    void result;

    // arrange 
    // ... 

    // act 
    result = ring_buffer_free(k_rb_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ... 
}

// 
// Test ring_buffer_free passing as rb argument an invalid ring_buffer_t* value 
// 
TEST_FUNCTION(ring_buffer_free__arg_rb_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = ring_buffer_free();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test ring_buffer_free unhappy path 
// 
TEST_FUNCTION(ring_buffer_free__neg)
{
    static const ring_buffer_t* k_rb_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = ring_buffer_free(k_rb_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif //  ring_buffer_free;

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

