// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST pal_err_posix
#include "util_ut.h"

//
// 1. Required mocks
//
#include "os.h"
#include "prx_err.h"

#undef errno
MOCKABLE_FUNCTION(, int*, errno_mock);
#define errno (*errno_mock())

//
// 2. Include unit under test
//
#undef ENABLE_MOCKS
#include "pal_err.h"
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
// Test pal_os_to_prx_error happy path
//
TEST_FUNCTION(pal_posix_os_to_prx_error__success)
{
    int32_t result;

    // arrange
    UMOCK_C_RANGE_TESTS_ARRANGE_FROM_TO(int, input, ERRNO_RANGE_BEGIN, ERRNO_RANGE_END);
    STRICT_EXPECTED_CALL(pal_os_to_prx_net_error(input))
        .SetReturn(er_unknown);

    // act
    UMOCK_C_RANGE_TESTS_ACT();
    result = pal_os_to_prx_error(input);

    // assert
    UMOCK_C_RANGE_TESTS_ASSERT(int32_t, result, er_unknown,
        er_ok,             er_permission,     er_not_found,      er_fatal,          er_aborted,
        er_disk_io,        er_not_found,      er_arg,            er_invalid_format, er_arg,
        er_fatal,          er_retry,          er_out_of_memory,  er_permission,     er_fault,
        er_disk_io,        er_busy,           er_already_exists, er_fatal,          er_not_found,
        er_disk_io,        er_disk_io,        er_arg,            er_disk_io,        er_out_of_memory,
        er_fatal,          er_disk_io,        er_disk_io,        er_disk_io,        er_disk_io,
        er_disk_io,        er_disk_io,        er_reset,          er_arg,            er_arg,
        er_fatal,          er_arg,            er_fatal,          er_not_impl,       er_disk_io,
        er_disk_io,        er_fatal,          er_fatal,          er_arg,            er_fatal,
        er_fatal,          er_fatal,          er_fatal,          er_fatal,          er_fatal,
        er_fatal,          er_fatal,          er_fatal,          er_fatal,          er_fatal,
        er_fatal,          er_fatal,          er_fatal,          er_fatal,          er_nomore,
        er_timeout,        er_out_of_memory,  er_network,        er_fatal,          er_fatal,
        er_fatal,          er_fatal,          er_fatal,          er_comm,           er_network,
        er_network,        er_fatal,          er_invalid_format, er_fault,          er_fatal,
        er_bad_state,      er_fatal,          er_fatal,          er_fatal,          er_fatal,
        er_fatal,          er_fatal,          er_fatal,          er_retry,          er_comm,
        er_fatal,          er_arg,            er_arg,            er_invalid_format, er_not_supported,
        er_not_supported,  er_not_supported,  er_not_supported,  er_not_supported,  er_not_supported,
        er_address_family, er_already_exists, er_no_address,     er_network,        er_network,
        er_network,        er_connecting,     er_reset,          er_out_of_memory,  er_already_exists,
        er_closed,         er_shutdown,       er_fatal,          er_timeout,        er_refused,
        er_no_host,        er_host_unknown,   er_waiting,        er_waiting,        er_fatal,
        er_fatal,          er_fatal,          er_fatal,          er_fatal,          er_fatal,
        er_fatal,          er_fatal,          er_fatal,          er_aborted,        er_fatal,
        er_fatal,          er_fatal,          er_fatal,          er_fatal,          er_fatal,
        er_retry);
}

//
// Test pal_os_from_prx_error happy path
//
TEST_FUNCTION(pal_posix_os_from_prx_error__success)
{
    int result;

    // arrange
    UMOCK_C_RANGE_TESTS_ARRANGE_FROM_TO(int32_t, input, er_unknown, er_ok);
    STRICT_EXPECTED_CALL(pal_os_from_prx_net_error(input))
        .SetReturn(-1);

    // act
    UMOCK_C_RANGE_TESTS_ACT();
    result = pal_os_from_prx_error(input);

    // assert
    UMOCK_C_RANGE_TESTS_ASSERT(int, result, -1,
        129, 22,  14,  75,  12,  17,  6,   93,  38,  13, 11,  59,  98,  111, 16,  68,  68,  113, 60,  4,
        105, 106, 109, 97,  110, 111, 95,  22,  22,  5,  69,  69,  102, 108, 1,   68,  0);  // see os_mock.h
}

//
// Test pal_os_last_error_as_prx_error happy path
//
TEST_FUNCTION(pal_posix_os_last_error_as_prx_error__success)
{
    int errno_valid = 0;
    int32_t result;

    // arrange
    STRICT_EXPECTED_CALL(errno_mock())
        .SetReturn(&errno_valid);

    // act
    result = pal_os_last_error_as_prx_error();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

//
// Test pal_os_set_error_as_prx_error happy path
//
TEST_FUNCTION(pal_posix_os_set_error_as_prx_error__success)
{
    static const int32_t k_error_valid = er_ok;
    int errno_valid = -1;

    // arrange
    STRICT_EXPECTED_CALL(errno_mock())
        .SetReturn(&errno_valid);

    // act
    pal_os_set_error_as_prx_error(k_error_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int, 0, errno_valid);
}


//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

