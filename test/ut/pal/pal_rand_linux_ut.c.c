// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.


#define UNIT_UNDER_TEST pal_rand_linux
#include "util_ut.h"

//
// 1. Required mocks
//
#include "os.h"
#include "prx_types.h"

#undef errno
MOCKABLE_FUNCTION(, int*, errno_mock);
#define errno (*errno_mock())

MOCKABLE_FUNCTION(, long, syscall, 
    long, number, void*, buf, size_t, buflen, unsigned int, flags);

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
TEST_FUNCTION(pal_linux_rand_fill__success_1)
{
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(syscall(SYS_getrandom, UT_MEM, 100, 0))
        .SetReturn(100);

    // act 
    result = pal_rand_fill(UT_MEM, 100);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_rand_fill happy path 
// 
TEST_FUNCTION(pal_linux_rand_fill__success_2)
{
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(syscall(SYS_getrandom, UT_MEM, 100, 0))
        .SetReturn(40);
    STRICT_EXPECTED_CALL(syscall(SYS_getrandom, UT_MEM + 40, 60, 0))
        .SetReturn(60);

    // act 
    result = pal_rand_fill(UT_MEM, 100);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_rand_fill happy path 
// 
TEST_FUNCTION(pal_linux_rand_fill__success_3)
{
    static int errno_intr = EINTR;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(syscall(SYS_getrandom, UT_MEM, 100, 0))
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(errno_mock())
        .SetReturn(&errno_intr);
    STRICT_EXPECTED_CALL(syscall(SYS_getrandom, UT_MEM, 100, 0))
        .SetReturn(100);

    // act 
    result = pal_rand_fill(UT_MEM, 100);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_rand_fill happy path 
// 
TEST_FUNCTION(pal_linux_rand_fill__success_4)
{
    static int errno_again = EAGAIN;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(syscall(SYS_getrandom, UT_MEM, 100, 0))
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(errno_mock())
        .SetReturn(&errno_again);
    STRICT_EXPECTED_CALL(syscall(SYS_getrandom, UT_MEM, 100, 0))
        .SetReturn(40);
    STRICT_EXPECTED_CALL(syscall(SYS_getrandom, UT_MEM + 40, 60, 0))
        .SetReturn(60);

    // act 
    result = pal_rand_fill(UT_MEM, 100);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_rand_fill passing as buf argument an invalid ptr
// 
TEST_FUNCTION(pal_linux_rand_fill__arg_buf_null)
{
    static int errno_valid = EFAULT;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(syscall(SYS_getrandom, NULL, 100, 0))
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(errno_mock())
        .SetReturn(&errno_valid);
    STRICT_EXPECTED_CALL(pal_os_to_prx_error(EFAULT))
        .SetReturn(er_fault);

    // act 
    result = pal_rand_fill(NULL, 100);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_rand_deinit happy path
// 
TEST_FUNCTION(pal_linux_rand_deinit__success)
{
    // arrange 

    // act 
    pal_rand_deinit();

    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_rand_init happy path 
// 
TEST_FUNCTION(pal_linux_rand_init__success)
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

