// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.


#define UNIT_UNDER_TEST pal_rand_openssl
#include "util_ut.h"

//
// 1. Required mocks
//
#include "os.h"
#include "prx_types.h"

MOCKABLE_FUNCTION(, int, RAND_bytes,
    unsigned char*, buf, int, num);
MOCKABLE_FUNCTION(, unsigned long, ERR_get_error);
MOCKABLE_FUNCTION(, void, ERR_error_string_n,
    unsigned long, e, char*, buf, size_t, len);

//
// 2. Include unit under test
//
#undef ENABLE_MOCKS
#include "pal_rand.h"
#define ENABLE_MOCKS
#include UNIT_C

//
// 3. Setup test suite
//
BEGIN_DECLARE_TEST_SUITE()
END_DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
DECLARE_TEST_SETUP()


// 
// Test pal_rand_fill happy path 
// 
TEST_FUNCTION(pal_openssl_rand_fill__success_1)
{
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(RAND_bytes((unsigned char*)UT_MEM, 100))
        .SetReturn(1);

    // act 
    result = pal_rand_fill(UT_MEM, 100);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_rand_fill happy path 
// 
TEST_FUNCTION(pal_openssl_rand_fill__success_2)
{
    int32_t result;

    // arrange 

    // act 
    result = pal_rand_fill(UT_MEM, 0);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_rand_fill passing as buf argument an invalid ptr
// 
TEST_FUNCTION(pal_openssl_rand_fill__arg_buf_null)
{
    int32_t result;

    // arrange 

    // act 
    result = pal_rand_fill(NULL, 100);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_rand_fill happy path 
// 
TEST_FUNCTION(pal_openssl_rand_fill__neg)
{
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(RAND_bytes((unsigned char*)UT_MEM, 100))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(ERR_get_error())
        .SetReturn(123);
    STRICT_EXPECTED_CALL(ERR_error_string_n(123, IGNORED_PTR_ARG, 255))
        .IgnoreArgument(2);
    STRICT_EXPECTED_CALL(ERR_get_error())
        .SetReturn(0);
    
    // act 
    result = pal_rand_fill(UT_MEM, 100);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fatal, result);
}


// 
// Test pal_rand_deinit happy path
// 
TEST_FUNCTION(pal_openssl_rand_deinit__success)
{
    // arrange 

    // act 
    pal_rand_deinit();

    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_rand_init happy path 
// 
TEST_FUNCTION(pal_openssl_rand_init__success)
{
    int32_t result;

    // arrange 

    // act 
    result = pal_rand_init();

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}


//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

