// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.


#define UNIT_UNDER_TEST pal
#include "util_ut.h"

//
// 1. Required mocks
//
#include "os.h"
#include "prx_types.h"

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
// Test pal_init happy path
//
TEST_FUNCTION(pal_init__success_1)
{
    int32_t result;

    capabilities = pal_not_init;

    // arrange
    STRICT_EXPECTED_CALL(pal_err_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_time_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_rand_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_cred_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_file_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_net_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_sd_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_socket_init(&capabilities))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_scan_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_wsclient_init())
        .SetReturn(er_ok);

    // act
    result = pal_init();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_IS_TRUE(capabilities ==
        (pal_cap_sockets | pal_cap_cred | pal_cap_net | pal_cap_dnssd | pal_cap_file | pal_cap_scan | pal_cap_wsclient ));
}

//
// Test pal_init happy path
//
TEST_FUNCTION(pal_init__success_2)
{
    int32_t result;

    capabilities = pal_cap_sockets;

    // arrange

    // act
    result = pal_init();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_IS_TRUE(capabilities == pal_cap_sockets);
}

//
// Test pal_init happy path
//
TEST_FUNCTION(pal_init__success_3)
{
    int32_t result;

    capabilities = pal_not_init;

    // arrange
    STRICT_EXPECTED_CALL(pal_err_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_time_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_rand_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_cred_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_file_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_net_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_sd_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_socket_init(&capabilities))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_scan_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_wsclient_init())
        .SetReturn(er_not_supported);

    // act
    result = pal_init();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_IS_TRUE(capabilities ==
        (pal_cap_sockets | pal_cap_cred | pal_cap_net | pal_cap_dnssd | pal_cap_file | pal_cap_scan));
}

//
// Test pal_init happy path
//
TEST_FUNCTION(pal_init__success_4)
{
    int32_t result;

    capabilities = pal_not_init;

    // arrange
    STRICT_EXPECTED_CALL(pal_err_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_time_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_rand_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_cred_init())
        .SetReturn(er_not_supported);
    STRICT_EXPECTED_CALL(pal_file_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_net_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_sd_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_socket_init(&capabilities))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_scan_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_wsclient_init())
        .SetReturn(er_not_supported);

    // act
    result = pal_init();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_IS_TRUE(capabilities == (pal_cap_sockets | pal_cap_net | pal_cap_dnssd | pal_cap_file | pal_cap_scan));
}

//
// Test pal_init unhappy path
//
TEST_FUNCTION(pal_init__neg_1)
{
    int32_t result;

    capabilities = pal_not_init;

    // arrange
    STRICT_EXPECTED_CALL(pal_err_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_time_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_rand_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_cred_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_file_init())
        .SetReturn(er_fatal);
    STRICT_EXPECTED_CALL(pal_cred_deinit());
    STRICT_EXPECTED_CALL(pal_rand_deinit());
    STRICT_EXPECTED_CALL(pal_time_deinit());
    STRICT_EXPECTED_CALL(pal_err_deinit());

    // act
    result = pal_init();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fatal, result);
    ASSERT_IS_TRUE(capabilities == pal_not_init);
}

//
// Test pal_init unhappy path
//
TEST_FUNCTION(pal_init__neg_2)
{
    int32_t result;

    capabilities = pal_not_init;

    // arrange
    STRICT_EXPECTED_CALL(pal_err_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_time_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_rand_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_cred_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_file_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_net_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_sd_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_socket_init(&capabilities))
        .SetReturn(er_not_supported);
    STRICT_EXPECTED_CALL(pal_sd_deinit());
    STRICT_EXPECTED_CALL(pal_net_deinit());
    STRICT_EXPECTED_CALL(pal_file_deinit());
    STRICT_EXPECTED_CALL(pal_cred_deinit());
    STRICT_EXPECTED_CALL(pal_rand_deinit());
    STRICT_EXPECTED_CALL(pal_time_deinit());
    STRICT_EXPECTED_CALL(pal_err_deinit());

    // act
    result = pal_init();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_not_supported, result);
    ASSERT_IS_TRUE(capabilities == pal_not_init);
}

//
// Test pal_init unhappy path
//
TEST_FUNCTION(pal_init__neg_3)
{
    int32_t result;

    capabilities = pal_not_init;

    // arrange
    STRICT_EXPECTED_CALL(pal_err_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_time_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_rand_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_cred_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_file_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_net_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_sd_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_socket_init(&capabilities))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_scan_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_wsclient_init())
        .SetReturn(er_fatal);
    STRICT_EXPECTED_CALL(pal_scan_deinit());
    STRICT_EXPECTED_CALL(pal_socket_deinit());
    STRICT_EXPECTED_CALL(pal_sd_deinit());
    STRICT_EXPECTED_CALL(pal_net_deinit());
    STRICT_EXPECTED_CALL(pal_file_deinit());
    STRICT_EXPECTED_CALL(pal_cred_deinit());
    STRICT_EXPECTED_CALL(pal_rand_deinit());
    STRICT_EXPECTED_CALL(pal_time_deinit());
    STRICT_EXPECTED_CALL(pal_err_deinit());

    // act
    result = pal_init();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fatal, result);
    ASSERT_IS_TRUE(capabilities == pal_not_init);
}

//
// Test pal_init unhappy path
//
TEST_FUNCTION(pal_init__neg_4)
{
    int32_t result;

    capabilities = pal_not_init;

    // arrange
    STRICT_EXPECTED_CALL(pal_err_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_time_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_rand_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_cred_init())
        .SetReturn(er_fatal);
    STRICT_EXPECTED_CALL(pal_rand_deinit());
    STRICT_EXPECTED_CALL(pal_time_deinit());
    STRICT_EXPECTED_CALL(pal_err_deinit());

    // act
    result = pal_init();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fatal, result);
    ASSERT_IS_TRUE(capabilities == pal_not_init);
}

//
// Test pal_init unhappy path
//
TEST_FUNCTION(pal_init__neg_5)
{
    int32_t result;

    capabilities = pal_not_init;

    // arrange
    STRICT_EXPECTED_CALL(pal_err_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_time_init())
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_rand_init())
        .SetReturn(er_fatal);
    STRICT_EXPECTED_CALL(pal_time_deinit());
    STRICT_EXPECTED_CALL(pal_err_deinit());

    // act
    result = pal_init();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fatal, result);
    ASSERT_IS_TRUE(capabilities == pal_not_init);
}

//
// Test pal_deinit happy path
//
TEST_FUNCTION(pal_deinit__success_1)
{
    int32_t result;

    capabilities = pal_cap_all;

    // arrange
    STRICT_EXPECTED_CALL(pal_wsclient_deinit());
    STRICT_EXPECTED_CALL(pal_scan_deinit());
    STRICT_EXPECTED_CALL(pal_socket_deinit());
    STRICT_EXPECTED_CALL(pal_sd_deinit());
    STRICT_EXPECTED_CALL(pal_net_deinit());
    STRICT_EXPECTED_CALL(pal_file_deinit());
    STRICT_EXPECTED_CALL(pal_cred_deinit());
    STRICT_EXPECTED_CALL(pal_rand_deinit());
    STRICT_EXPECTED_CALL(pal_time_deinit());
    STRICT_EXPECTED_CALL(pal_err_deinit());

    // act
    result = pal_deinit();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_IS_TRUE(capabilities == pal_not_init);
}

//
// Test pal_deinit happy path
//
TEST_FUNCTION(pal_deinit__success_2)
{
    int32_t result;

    capabilities = pal_cap_sockets;

    // arrange
    STRICT_EXPECTED_CALL(pal_socket_deinit());
    STRICT_EXPECTED_CALL(pal_rand_deinit());
    STRICT_EXPECTED_CALL(pal_time_deinit());
    STRICT_EXPECTED_CALL(pal_err_deinit());

    // act
    result = pal_deinit();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_IS_TRUE(capabilities == pal_not_init);
}

//
// Test pal_deinit happy path
//
TEST_FUNCTION(pal_deinit__neg)
{
    int32_t result;

    capabilities = pal_not_init;

    // arrange

    // act
    result = pal_deinit();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_bad_state, result);
}

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

