// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST util_misc
#include "util_ut.h"

//
// 1. Required mocks
//

//
// 2. Include unit under test
//
#undef ENABLE_MOCKS
#include UNIT_H
#include "azure_c_shared_utility/tickcounter.h"
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
BEGIN_DECLARE_TEST_SETUP()
END_DECLARE_TEST_SETUP()

// 
// Test count_leading_ones happy path 
// 
TEST_FUNCTION(count_leading_ones__success)
{
    const uint8_t k_b_valid0 = 0;
    const uint8_t k_b_valid23 = 23;
    const uint8_t k_b_valid255 = 0xff;
    size_t result;

    // arrange 

    // act / assert 
    result = count_leading_ones(k_b_valid0);
    ASSERT_ARE_EQUAL(size_t, 0, result);
    result = count_leading_ones(k_b_valid23);
    ASSERT_ARE_EQUAL(size_t, 4, result);
    result = count_leading_ones(k_b_valid255);
    ASSERT_ARE_EQUAL(size_t, 8, result);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test count_leading_ones_in_buf happy path 
// 
TEST_FUNCTION(count_leading_ones_in_buf__success)
{
    const uint8_t k_b_valid0[] = { 0, 0, 0 };
    const uint8_t k_b_valid1[] = { 1, 34, 33, 1 };
    const uint8_t k_b_valid33[] = { 0xff, 33, 34, 33, 0 };
    const uint8_t k_b_valid66[] = { 0xff, 0xff, 0xff, 66, 0, 1, 0, 66 };
    const uint8_t k_b_valid255[] = { 255 };
    size_t result;

    // arrange 

    // act / assert 
    result = count_leading_ones_in_buf(NULL, 0);
    ASSERT_ARE_EQUAL(size_t, 0, result);
    result = count_leading_ones_in_buf(k_b_valid0, sizeof(k_b_valid0));
    ASSERT_ARE_EQUAL(size_t, 0, result);
    result = count_leading_ones_in_buf(k_b_valid1, sizeof(k_b_valid1));
    ASSERT_ARE_EQUAL(size_t, 1, result);
    result = count_leading_ones_in_buf(k_b_valid33, sizeof(k_b_valid33));
    ASSERT_ARE_EQUAL(size_t, 10, result);
    result = count_leading_ones_in_buf(k_b_valid66, sizeof(k_b_valid66));
    ASSERT_ARE_EQUAL(size_t, 26, result);
    result = count_leading_ones_in_buf(k_b_valid255, sizeof(k_b_valid255));
    ASSERT_ARE_EQUAL(size_t, 8, result);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test crc32 happy path 
// 
TEST_FUNCTION(crc32__success)
{
    static const size_t k_len_valid = 256;
    const void* k_buf_valid = (void*)UT_MEM;
    uint32_t result;

    // arrange 

    // act 
    memset(UT_MEM, 23, k_len_valid);
    result = crc32(k_buf_valid, k_len_valid);
    ASSERT_ARE_EQUAL(uint32_t, 2909714534, result);

    memset(UT_MEM, 1, k_len_valid);
    result = crc32(k_buf_valid, k_len_valid);
    ASSERT_ARE_EQUAL(uint32_t, 1630701510, result);

    memset(UT_MEM, 0, k_len_valid);
    result = crc32(k_buf_valid, k_len_valid);
    ASSERT_ARE_EQUAL(uint32_t, 227968344, result);

    memset(UT_MEM, -1, k_len_valid);
    result = crc32(k_buf_valid, k_len_valid);
    ASSERT_ARE_EQUAL(uint32_t, 4272465953, result);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test crc32 passing as buf argument an invalid const void* value 
// 
TEST_FUNCTION(crc32__arg_buf_invalid)
{
    uint32_t result;

    // arrange 

    // act 
    result = crc32(NULL, 0);

    // assert 
    ASSERT_ARE_EQUAL(uint32_t, 0, result);
    ASSERT_EXPECTED_CALLS();
}

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

