// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST pal_time_win
#include "util_ut.h"

//
// 1. Required mocks
//
#include "os.h"

MOCKABLE_FUNCTION(WINAPI, ULONGLONG, GetTickCount64);

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
REGISTER_UMOCK_ALIAS_TYPE(ULONGLONG, unsigned long long);
REGISTER_UMOCK_ALIAS_TYPE(ticks_t, long long);
END_DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
DECLARE_TEST_SETUP()

//
// Test pal_time_init happy path
//
TEST_FUNCTION(pal_win_time_init__success)
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
TEST_FUNCTION(pal_win_ticks_get__success)
{
    ULONGLONG k_ticks_valid = 0x1234ULL;
    ticks_t result;

    // arrange
    STRICT_EXPECTED_CALL(GetTickCount64())
        .SetReturn(k_ticks_valid);

    // act
    result = ticks_get();

    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_TRUE((ticks_t)k_ticks_valid == result);
}

//
// Test pal_time_deinit happy path
//
TEST_FUNCTION(pal_win_time_deinit__success)
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

