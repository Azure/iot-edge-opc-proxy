// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST pal_time_posix
#include "util_ut.h"

//
// 1. Required mocks
//
#include "os.h"

MOCKABLE_FUNCTION(, int, clock_gettime, clockid_t, clk_id, struct timespec*, tp);

//
// 2. Include unit under test
//
#undef ENABLE_MOCKS
#include "pal_time.h"
#define ENABLE_MOCKS
#include UNIT_C

//
// 3. Setup test suite
//
BEGIN_DECLARE_TEST_SUITE()
REGISTER_UMOCK_ALIAS_TYPE(ticks_t, long long);
REGISTER_UMOCK_ALIAS_TYPE(clockid_t, int);
END_DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
DECLARE_TEST_SETUP()

//
// Test pal_time_init happy path
//
TEST_FUNCTION(pal_posix_time_init__success)
{
    int32_t result;

    // arrange

    // act
    result = pal_time_init();

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

//
// Test ticks_get happy path
//
TEST_FUNCTION(pal_posix_ticks_get__success_1)
{
    static const ticks_t k_ticks_valid = 1235234;
    struct timespec k_timeval_valid;
    ticks_t result;

    k_timeval_valid.tv_nsec = 1234567890;
    k_timeval_valid.tv_sec = 1234;

    // arrange
    STRICT_EXPECTED_CALL(clock_gettime(CLOCK_MONOTONIC, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_tp(&k_timeval_valid, sizeof(k_timeval_valid))
        .SetReturn(0);

    // act
    result = ticks_get();

    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_TRUE(k_ticks_valid == result);
}

//
// Test ticks_get happy path
//
TEST_FUNCTION(pal_posix_ticks_get__success_2)
{
    ticks_t k_ticks_valid = (ticks_t)(10LL * 365LL * 24LL * 60LL * 60LL * 1000LL);
    struct timespec k_timeval_valid;
    ticks_t result;

    k_timeval_valid.tv_nsec = 0;
    k_timeval_valid.tv_sec = 10LL * 365LL * 24LL * 60LL * 60LL;  // 10 years

    // arrange
    STRICT_EXPECTED_CALL(clock_gettime(CLOCK_MONOTONIC, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_tp(&k_timeval_valid, sizeof(k_timeval_valid))
        .SetReturn(0);

    // act
    result = ticks_get();

    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_TRUE(k_ticks_valid == result);
    ASSERT_IS_TRUE(k_ticks_valid > k_timeval_valid.tv_sec);
}

//
// Test ticks_get happy path
//
TEST_FUNCTION(pal_posix_ticks_get__success_3)
{
    struct timespec k_timeval_valid;
    ticks_t result;

    memset(&k_timeval_valid, 0, sizeof(k_timeval_valid));

    // arrange
    STRICT_EXPECTED_CALL(clock_gettime(CLOCK_MONOTONIC, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_tp(&k_timeval_valid, sizeof(k_timeval_valid))
        .SetReturn(0);

    // act
    result = ticks_get();

    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_TRUE(0 == result);
}

//
// Test pal_time_deinit happy path
//
TEST_FUNCTION(pal_posix_time_deinit__success)
{
    // arrange

    // act
    pal_time_deinit();

    ASSERT_EXPECTED_CALLS();
}

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

