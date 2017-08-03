// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST pal_net_stub
#include "util_ut.h"

//
// 1. Required mocks
//
#include "os.h"
#include "prx_types.h"

// socket
MOCKABLE_FUNCTION(, int, gethostname,
    char*, name, socksize_t, namelen);

//
// 2. Include unit under test
//
#undef ENABLE_MOCKS
#include "pal_net.h"
#include "pal_types.h"
#define ENABLE_MOCKS
#include UNIT_C

//
// 3. Setup test suite
//
BEGIN_DECLARE_TEST_SUITE()
REGISTER_UMOCK_ALIAS_TYPE(socksize_t, int);
END_DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
DECLARE_TEST_SETUP()

//
// Test pal_getifaddrinfo happy path
//
TEST_FUNCTION(pal_stub_getifaddrinfo__success)
{
    static const char* k_if_name_valid = "eth0";
    prx_ifaddrinfo_t* info_valid;
    size_t info_count_valid;
    int32_t result;

    // arrange

    // act
    result = pal_getifaddrinfo(k_if_name_valid, 0, &info_valid, &info_count_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_not_supported, result);
}

//
// Test pal_freeifaddrinfo happy path
//
TEST_FUNCTION(pal_stub_freeifaddrinfo__success)
{
    static prx_ifaddrinfo_t* k_info_valid = (prx_ifaddrinfo_t*)0x2624;
    int32_t result;

    // arrange

    // act
    result = pal_freeifaddrinfo(k_info_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_not_supported, result);
}

//
// Test pal_getifnameinfo happy path
//
TEST_FUNCTION(pal_stub_getifnameinfo__success)
{
    static prx_socket_address_t* k_if_address_valid = (prx_socket_address_t*)0x23423;
    char* if_name_valid = UT_MEM;
    size_t if_name_length_valid = 256;
    uint64_t if_index_valid;
    int32_t result;

    // arrange

    // act
    result = pal_getifnameinfo(k_if_address_valid, if_name_valid, if_name_length_valid, &if_index_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_not_supported, result);
}

//
// Test pal_gethostname happy path
//
TEST_FUNCTION(pal_stub_gethostname__success)
{
    static const char* k_host_name_valid = "my_host1";
    static const size_t k_name_length_valid = 256;
    char* name_valid = UT_MEM;
    int32_t result;

    // arrange
    STRICT_EXPECTED_CALL(gethostname(name_valid, (socksize_t)k_name_length_valid))
        .CopyOutArgumentBuffer_name(k_host_name_valid, strlen(k_host_name_valid) + 1)
        .SetReturn(0);

    // act
    result = pal_gethostname(name_valid, k_name_length_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

//
// Test pal_gethostname passing as name argument an invalid char* value
//
TEST_FUNCTION(pal_stub_gethostname__arg_name_null)
{
    static const size_t k_name_length_valid = 256;
    int32_t result;

    // arrange

    // act
    result = pal_gethostname(NULL, (socksize_t)k_name_length_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

//
// Test pal_gethostname passing as name_length argument an invalid size_t value
//
TEST_FUNCTION(pal_stub_gethostname__arg_name_length_zero)
{
    char* name_valid = UT_MEM;
    int32_t result;

    // arrange

    // act
    result = pal_gethostname(name_valid, 0);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

//
// Test pal_gethostname unhappy path
//
TEST_FUNCTION(pal_stub_gethostname__neg)
{
    static const size_t k_name_length_invalid = 3;
    char* name_valid = UT_MEM;
    int32_t result;

    REGISTER_GLOBAL_MOCK_RETURNS(pal_os_last_error_as_prx_error, er_fault, er_fault);

    // arrange
    STRICT_EXPECTED_CALL(gethostname(IGNORED_PTR_ARG, (socksize_t)k_name_length_invalid))
        .IgnoreArgument(1)
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(pal_os_last_error_as_prx_error());

    // act
    result = pal_gethostname(name_valid, k_name_length_invalid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()
