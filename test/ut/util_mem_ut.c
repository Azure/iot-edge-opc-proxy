// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST util_mem
#include "util_ut.h"

//
// 1. Required mocks
//

MOCKABLE_FUNCTION(, void*, my_realloc, void*, ptr, size_t, size);
MOCKABLE_FUNCTION(, void, my_free, void*, ptr);
MOCKABLE_FUNCTION(, void, my_memset, void*, ptr, int, val, size_t, size);

//
// 2. Include unit under test
//
#undef ENABLE_MOCKS
#include UNIT_H
#define ENABLE_MOCKS

#define realloc my_realloc
#define free my_free
#define memset my_memset
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
// Test mem_alloc happy path 
// 
TEST_FUNCTION(mem_alloc__success)
{
    static const size_t k_size_valid = 16;
    static void* k_ptr_valid = (void*)0x2000;
    void* result;

    // arrange 
    STRICT_EXPECTED_CALL(my_realloc(NULL, k_size_valid))
        .SetReturn(k_ptr_valid);

    // act 
    result = mem_alloc(k_size_valid);

    // assert 
    ASSERT_ARE_EQUAL(void_ptr, k_ptr_valid, result);
    ASSERT_EXPECTED_CALLS();
}

// 
// Test mem_alloc unhappy path 
// 
TEST_FUNCTION(mem_alloc__neg)
{
    static const size_t k_size_valid = 16;
    static void* k_ptr_valid = (void*)0x2000;
    void* result;

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(my_realloc(NULL, k_size_valid))
        .SetReturn(k_ptr_valid)
        .SetFailReturn(NULL);

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = mem_alloc(k_size_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, result, NULL);
}

// 
// Test mem_zalloc happy path 
// 
TEST_FUNCTION(mem_zalloc__success)
{
    static const size_t k_size_valid = 16;
    static void* k_ptr_valid = (void*)UT_MEM;
    void* result;

    // arrange 
    STRICT_EXPECTED_CALL(my_realloc(NULL, k_size_valid))
        .SetReturn(k_ptr_valid);
    STRICT_EXPECTED_CALL(my_memset(k_ptr_valid, 0, k_size_valid));

    // act 
    result = mem_zalloc(k_size_valid);

    // assert 
    ASSERT_ARE_EQUAL(void_ptr, k_ptr_valid, result);
    ASSERT_EXPECTED_CALLS();
}

// 
// Test mem_zalloc unhappy path 
// 
TEST_FUNCTION(mem_zalloc__neg_alloc_fails)
{
    static const size_t k_size_valid = 16;
    void* result;

    // arrange 
    STRICT_EXPECTED_CALL(my_realloc(NULL, k_size_valid))
        .SetReturn(NULL);

    // act 
    result = mem_alloc(k_size_valid);

    // assert 
    ASSERT_ARE_EQUAL(void_ptr, NULL, result);
    ASSERT_EXPECTED_CALLS();
}

// 
// Test mem_realloc happy path 
// 
TEST_FUNCTION(mem_realloc__success)
{
    static const size_t k_size_valid = 16;
    static void* k_ptr1_valid = (void*)0x1000;
    static void* k_ptr2_valid = (void*)0x2000;
    void* result;

    // arrange 
    STRICT_EXPECTED_CALL(my_realloc(k_ptr1_valid, k_size_valid))
        .SetReturn(k_ptr2_valid);

    // act 
    result = mem_realloc(k_ptr1_valid, k_size_valid);

    // assert 
    ASSERT_ARE_EQUAL(void_ptr, k_ptr2_valid, result);
    ASSERT_EXPECTED_CALLS();
}

// 
// Test mem_realloc unhappy path 
// 
TEST_FUNCTION(mem_realloc__neg)
{
    static const size_t k_size_valid = 16;
    static void* k_ptr1_valid = (void*)0x1000;
    static void* k_ptr2_valid = (void*)0x2000;
    void* result;

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(my_realloc(k_ptr1_valid, k_size_valid))
        .SetReturn(k_ptr2_valid)
        .SetFailReturn(NULL);

    // act 
        UMOCK_C_NEGATIVE_TESTS_ACT();
        result = mem_realloc(k_ptr1_valid, k_size_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, result, NULL);
}

// 
// Test mem_free happy path 
// 
TEST_FUNCTION(mem_free__success)
{
    static void* k_ptr_valid = (void*)0x3333;

    // arrange 
    STRICT_EXPECTED_CALL(my_free(k_ptr_valid));

    // act 
    mem_free(k_ptr_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
}
// 
// Test crt_alloc happy path 
// 
TEST_FUNCTION(crt_alloc__success)
{
    static const size_t k_size_valid = 333;
    static void* k_ptr_valid = (void*)0x1;
    void* result;

    // arrange 
    STRICT_EXPECTED_CALL(my_realloc(NULL, k_size_valid))
        .SetReturn(k_ptr_valid);

    // act 
    result = crt_alloc(k_size_valid);

    // assert 
    ASSERT_ARE_EQUAL(void_ptr, k_ptr_valid, result);
    ASSERT_EXPECTED_CALLS();
}

// 
// Test mem_alloc unhappy path 
// 
TEST_FUNCTION(crt_alloc__neg)
{
    static const size_t k_size_valid = 66;
    static void* k_ptr_valid = (void*)0x666;
    void* result;

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(my_realloc(NULL, k_size_valid))
        .SetReturn(k_ptr_valid)
        .SetFailReturn(NULL);

    // act 
        UMOCK_C_NEGATIVE_TESTS_ACT();
        result = crt_alloc(k_size_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, result, NULL);
}

// 
// Test crt_free happy path 
// 
TEST_FUNCTION(crt_free__success)
{
    static void* k_ptr_valid = (void*)0x256;

    // arrange 
    STRICT_EXPECTED_CALL(my_free(k_ptr_valid));

    // act 
    crt_free(k_ptr_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

