// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST pal_scan_stub
#include "util_ut.h"

//
// 1. Required mocks
//
#include "os.h"

//
// 2. Include unit under test
//
#undef ENABLE_MOCKS
#include "pal_scan.h"
#include "pal_types.h"
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
// Test pal_scan_init happy path
//
TEST_FUNCTION(pal_stub_scan_init__success)
{
    int32_t result;

    // arrange

    // act
    result = pal_scan_init();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_not_supported, result);
}

//
// Test pal_scan_net happy path
//
TEST_FUNCTION(pal_stub_scan_net__success)
{
    static const int32_t k_flags_valid;
    static const uint16_t k_port_valid;
    static const pal_scan_cb_t k_cb_valid;
    static void* k_context_valid = UT_MEM;
    pal_scan_t* scanner_valid;
    int32_t result;

    // arrange

    // act
    result = pal_scan_net(k_port_valid, k_flags_valid, k_cb_valid, k_context_valid, &scanner_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_not_supported, result);
}

//
// Test pal_scan_ports happy path
//
TEST_FUNCTION(pal_stub_scan_ports__success)
{
    static const prx_socket_address_t* k_addr_valid;
    static const uint16_t k_port_range_low_valid;
    static const uint16_t k_port_range_high_valid;
    static const int32_t k_flags_valid;
    static pal_scan_cb_t k_cb_valid;
    static void* k_context_valid = UT_MEM;
    pal_scan_t* scanner_valid;
    int32_t result;

    // arrange

    // act
    result = pal_scan_ports(k_addr_valid, k_port_range_low_valid, k_port_range_high_valid, k_flags_valid, k_cb_valid, k_context_valid, &scanner_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_not_supported, result);
}

//
// Test pal_scan_close happy path
//
TEST_FUNCTION(pal_stub_scan_close__success)
{
    static pal_scan_t* scanner_invalid = (pal_scan_t*)0x234;

    // arrange

    // act
    pal_scan_close(scanner_invalid);

    // assert
    ASSERT_EXPECTED_CALLS();
}

//
// Test pal_scan_deinit happy path
//
TEST_FUNCTION(pal_stub_scan_deinit__success)
{
    // arrange

    // act
    pal_scan_deinit();

    // assert
    ASSERT_EXPECTED_CALLS();
}


//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

