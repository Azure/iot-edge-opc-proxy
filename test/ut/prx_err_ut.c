// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST prx_err
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
END_DECLARE_TEST_SUITE()
// -or- DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
DECLARE_TEST_SETUP()


//
// Test prx_err_string happy path
//
TEST_FUNCTION(prx_err_string__success)
{
    const int32_t k_error_valid = er_arg;
    const char* result;

    // arrange

    // act
    result = prx_err_string(k_error_valid);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "er_arg", result);
    ASSERT_EXPECTED_CALLS();
}

//
// Test prx_err_string passing as error argument an invalid int32_t value
//
TEST_FUNCTION(prx_err_string__arg_error_invalid)
{
    const char* result;

    // arrange

    // act
    result = prx_err_string(-1);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, "<unknown>", result);
    ASSERT_EXPECTED_CALLS();
}


//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

