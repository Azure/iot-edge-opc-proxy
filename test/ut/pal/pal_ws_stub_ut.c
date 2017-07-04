// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST pal_ws_stub
#include "util_ut.h"

//
// 1. Required mocks
//
#include "os.h"
#include "prx_types.h"
#include "prx_err.h"

//
// 2. Include unit under test
//
#undef ENABLE_MOCKS
#include "pal_ws.h"
#define ENABLE_MOCKS
#include UNIT_C

MOCKABLE_FUNCTION(, void, pal_wsclient_event_handler_mock,
    void*, context, pal_wsclient_event_t, ev, uint8_t**, buffer, size_t*, size, pal_wsclient_buffer_type_t*, type, int32_t, error);

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
// Test pal_wsclient_create happy path 
// 
TEST_FUNCTION(pal_stub_wsclient_create__neg)
{
    static const char* k_protocol_name_valid = "rpotadf";
    static const char* k_host_valid = "lkjafsd";
    static const uint16_t k_port_valid = 10;
    static const char* k_path_valid = "/9845/sadkf";
    static void* k_callback_context_valid = (void*)0x2354;
    pal_wsclient_t* wsclient_valid;
    int32_t result;

    // arrange 

    // act 
    result = pal_wsclient_create(k_protocol_name_valid, k_host_valid, 
        k_port_valid, k_path_valid, true, pal_wsclient_event_handler_mock, k_callback_context_valid, &wsclient_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_not_supported, result);
}

// 
// Test pal_wsclient_deinit happy path 
// 
TEST_FUNCTION(pal_win_wsclient_deinit__success)
{
    // arrange 

    // act 
    pal_wsclient_deinit();

    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_wsclient_init happy path 
// 
TEST_FUNCTION(pal_win_wsclient_init__success)
{
    int32_t result;
    // arrange 

    // act 
    result = pal_wsclient_init();

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_not_supported, result);
}

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

