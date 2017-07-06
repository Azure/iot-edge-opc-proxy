// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST pal_err_win
#include "util_ut.h"

//
// 1. Required mocks
//
#include "os.h"
#include "prx_err.h"

// Winbase.h
MOCKABLE_FUNCTION(WINAPI, void, SetLastError,
    DWORD, dwErrCode);
MOCKABLE_FUNCTION(WINAPI, DWORD, GetLastError);
MOCKABLE_FUNCTION(WINAPI, DWORD, FormatMessageA,
    DWORD, dwFlags, LPCVOID, lpSource, DWORD, dwMessageId, DWORD, dwLanguageId,
    LPSTR, lpBuffer, DWORD, nSize, void**, Arguments);
MOCKABLE_FUNCTION(WINAPI, HLOCAL, LocalFree,
    HLOCAL, hMem);


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
REGISTER_UMOCK_ALIAS_TYPE(DWORD, unsigned int);
REGISTER_UMOCK_ALIAS_TYPE(LPCVOID, void*);
REGISTER_UMOCK_ALIAS_TYPE(HLOCAL, void*);
REGISTER_UMOCK_ALIAS_TYPE(LPSTR, char*);
END_DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
DECLARE_TEST_SETUP()

// 
// Test pal_os_to_prx_net_error happy path 
// 
TEST_FUNCTION(pal_win_os_to_prx_error__success)
{
    int32_t result;

    // arrange 
    UMOCK_C_RANGE_TESTS_ARRANGE_FROM_TO(int, input, WIN_ERRORS_RANGE_BEGIN, WIN_ERRORS_RANGE_END);
    STRICT_EXPECTED_CALL(pal_os_to_prx_net_error(input))
        .SetReturn(er_unknown);

    // act 
    UMOCK_C_RANGE_TESTS_ACT();
    result = pal_os_to_prx_error(input);

    // assert 
    UMOCK_C_RANGE_TESTS_ASSERT(int32_t, result, er_unknown, 
        er_ok,        er_not_found,   er_arg,        er_out_of_memory,  er_not_found, 
        er_fault,     er_aborted,     er_aborted,    er_arg,            er_waiting,
        er_not_found, er_not_found,   er_nomore,     er_busy,           er_closed,    
        er_closed,    er_not_found,   er_permission,
        er_timeout,   er_aborted,     er_aborted,    er_aborted,        er_closed,
        er_refused,   er_closed,      er_closed,     
        er_unknown);
}

// 
// Test pal_os_from_prx_error happy path 
// 
TEST_FUNCTION(pal_win_os_from_prx_error__success)
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
    UMOCK_C_RANGE_TESTS_ASSERT(int, result, -1, 3, 6, 4, 2, 18, 13, 14, 10, 8, 1);  // see os_mock.h
}

// 
// Test pal_os_last_error_as_prx_error happy path 
// 
TEST_FUNCTION(pal_win_os_last_error_as_prx_error__success_1)
{
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(GetLastError())
        .SetReturn(ERROR_SUCCESS);

    // act 
    result = pal_os_last_error_as_prx_error();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_os_last_error_as_prx_error happy path 
// 
TEST_FUNCTION(pal_win_os_last_error_as_prx_error__success_2)
{
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(GetLastError())
        .SetReturn(ERROR_NO_DATA);
    STRICT_EXPECTED_CALL(FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, ERROR_NO_DATA, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), IGNORED_PTR_ARG, 0, NULL))
        .IgnoreArgument(5)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(LocalFree(IGNORED_PTR_ARG))
        .SetReturn(NULL);

    // act 
    result = pal_os_last_error_as_prx_error();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_not_found, result);
}

// 
// Test pal_os_set_error_as_prx_error happy path 
// 
TEST_FUNCTION(pal_win_os_set_error_as_prx_error__success)
{
    static const int32_t k_error_valid = er_ok;

    // arrange 
    STRICT_EXPECTED_CALL(SetLastError(ERROR_SUCCESS));

    // act 
    pal_os_set_error_as_prx_error(k_error_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

