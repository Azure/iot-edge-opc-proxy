// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST pal_proc_win
#include "util_ut.h"

//
// 1. Required mocks
//
#include "os.h"

// synchapi.h
MOCKABLE_FUNCTION(WINAPI, DWORD, WaitForSingleObject,
    HANDLE, hHandle, DWORD, dwMilliseconds);
// processthreadsapi.h
MOCKABLE_FUNCTION(WINAPI, BOOL, CreateProcessA,
    LPCSTR, lpApplicationName, LPSTR, lpCommandLine, LPSECURITY_ATTRIBUTES, lpProcessAttributes,
    LPSECURITY_ATTRIBUTES, lpThreadAttributes, BOOL, bInheritHandles, DWORD, dwCreationFlags,
    LPVOID, lpEnvironment, LPCSTR, lpCurrentDirectory, LPSTARTUPINFO, lpStartupInfo,
    LPPROCESS_INFORMATION, lpProcessInformation);
MOCKABLE_FUNCTION(WINAPI, BOOL, TerminateProcess,
    HANDLE, hProcess, UINT, uExitCode);
// handleapi.h
MOCKABLE_FUNCTION(WINAPI, BOOL, CloseHandle,
    HANDLE, hObject);

//
// 2. Include unit under test
//
#undef ENABLE_MOCKS
#include "pal_proc.h"
#define ENABLE_MOCKS
#include UNIT_C

//
// 3. Setup test suite
//
BEGIN_DECLARE_TEST_SUITE()
REGISTER_UMOCK_ALIAS_TYPE(STRING_HANDLE, void*);
REGISTER_UMOCK_ALIAS_TYPE(BOOL, bool);
REGISTER_UMOCK_ALIAS_TYPE(LPCSTR, const char*);
REGISTER_UMOCK_ALIAS_TYPE(LPSTR, char*);
REGISTER_UMOCK_ALIAS_TYPE(LPSECURITY_ATTRIBUTES, void*);
REGISTER_UMOCK_ALIAS_TYPE(DWORD, unsigned int);
REGISTER_UMOCK_ALIAS_TYPE(LPVOID, void*);
REGISTER_UMOCK_ALIAS_TYPE(LPSTARTUPINFO, void*);
REGISTER_UMOCK_ALIAS_TYPE(LPPROCESS_INFORMATION, void*);
REGISTER_UMOCK_ALIAS_TYPE(HANDLE, void*);
REGISTER_UMOCK_ALIAS_TYPE(UINT, unsigned int);
END_DECLARE_TEST_SUITE()
// -or- DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
DECLARE_TEST_SETUP()


// 
// Test pal_process_spawn happy path 
// 
TEST_FUNCTION(pal_win_process_spawn__success)
{
    static const char* k_image_valid = "test";
    static const int32_t k_argc_valid = 5;
    static const char* k_argv_valid[] = { "a", "b", "c", "d", "e" };
    static const STRING_HANDLE k_image_string_handle = (STRING_HANDLE)0x232423;
    static char* cmd_line = "foobar";
    process_t* created_valid;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(process_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(STRING_new())
        .SetReturn(k_image_string_handle);
    STRICT_EXPECTED_CALL(STRING_concat(k_image_string_handle, "\""))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat(k_image_string_handle, k_image_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat(k_image_string_handle, "\""))
        .SetReturn(0);
    for (int32_t i = 0; i < k_argc_valid; i++)
    {
        STRICT_EXPECTED_CALL(STRING_concat(k_image_string_handle, " "))
            .SetReturn(0);
        STRICT_EXPECTED_CALL(STRING_concat(k_image_string_handle, k_argv_valid[i]))
            .SetReturn(0);
    }
    STRICT_EXPECTED_CALL(STRING_c_str(k_image_string_handle))
        .SetReturn(cmd_line);
    STRICT_EXPECTED_CALL(CreateProcessA(NULL, cmd_line, NULL, NULL, FALSE, CREATE_NEW_CONSOLE | NORMAL_PRIORITY_CLASS,
        NULL, NULL, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(9).IgnoreArgument(10)
        .SetReturn(TRUE);

    // act 
    result = pal_process_spawn(k_image_valid, k_argc_valid, k_argv_valid, &created_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_process_spawn passing as created argument an invalid process_t** value 
// 
TEST_FUNCTION(pal_win_process_spawn__arg_created_null)
{
    static const char* k_image_valid = "test";
    const int32_t k_argc_valid = 3;
    static const char* k_argv_valid[] = { "a", "b", "c" };
    int32_t result;

    // arrange 

    // act 
    result = pal_process_spawn(k_image_valid, k_argc_valid, k_argv_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_process_spawn unhappy path 
// 
TEST_FUNCTION(pal_win_process_spawn__neg_1)
{
    static const char* k_image_valid = "test";
    static const int32_t k_argc_valid = 5;
    static const char* k_argv_valid[] = { "a", "b", "c", "d", "e" };
    static const STRING_HANDLE k_image_string_handle = (STRING_HANDLE)0x232423;
    static char* cmd_line = "foobar";
    process_t* created_valid;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(process_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(STRING_new())
        .SetReturn(k_image_string_handle);
    STRICT_EXPECTED_CALL(STRING_concat(k_image_string_handle, "\""))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat(k_image_string_handle, k_image_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat(k_image_string_handle, "\""))
        .SetReturn(0);
    for (int32_t i = 0; i < k_argc_valid; i++)
    {
        STRICT_EXPECTED_CALL(STRING_concat(k_image_string_handle, " "))
            .SetReturn(0);
        STRICT_EXPECTED_CALL(STRING_concat(k_image_string_handle, k_argv_valid[i]))
            .SetReturn(0);
    }
    STRICT_EXPECTED_CALL(STRING_c_str(k_image_string_handle))
        .SetReturn(cmd_line);
    STRICT_EXPECTED_CALL(CreateProcessA(NULL, cmd_line, NULL, NULL, FALSE, CREATE_NEW_CONSOLE | NORMAL_PRIORITY_CLASS,
        NULL, NULL, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(9).IgnoreArgument(10)
        .SetReturn(FALSE);
    STRICT_EXPECTED_CALL(pal_os_last_error_as_prx_error())
        .SetReturn(er_fatal);
    STRICT_EXPECTED_CALL(STRING_delete(k_image_string_handle));
    STRICT_EXPECTED_CALL(h_free((void*)UT_MEM, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    result = pal_process_spawn(k_image_valid, k_argc_valid, k_argv_valid, &created_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fatal, result);
}

// 
// Test pal_process_spawn unhappy path 
// 
TEST_FUNCTION(pal_win_process_spawn__neg_2)
{
    static const char* k_image_valid = "test";
    static const int32_t k_argc_valid = 5;
    static const char* k_argv_valid[] = { "a", "b", "c", "d", "e" };
    static const STRING_HANDLE k_image_string_handle = (STRING_HANDLE)0x232423;
    static char* cmd_line = "foobar";
    process_t* created_valid;
    int32_t result;

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(h_realloc(sizeof(process_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(STRING_new())
        .SetReturn(k_image_string_handle)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(STRING_concat(k_image_string_handle, "\""))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_concat(k_image_string_handle, k_image_valid))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_concat(k_image_string_handle, "\""))
        .SetReturn(0)
        .SetFailReturn(-1);
    for (int32_t i = 0; i < k_argc_valid; i++)
    {
        STRICT_EXPECTED_CALL(STRING_concat(k_image_string_handle, " "))
            .SetReturn(0)
            .SetFailReturn(-1);
        STRICT_EXPECTED_CALL(STRING_concat(k_image_string_handle, k_argv_valid[i]))
            .SetReturn(0)
            .SetFailReturn(-1);
    }
    STRICT_EXPECTED_CALL(STRING_c_str(k_image_string_handle))
        .SetReturn(cmd_line)
        .SetFailReturn(cmd_line);
    STRICT_EXPECTED_CALL(CreateProcessA(NULL, cmd_line, NULL, NULL, FALSE, CREATE_NEW_CONSOLE | NORMAL_PRIORITY_CLASS,
        NULL, NULL, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(9).IgnoreArgument(10)
        .SetReturn(TRUE)
        .SetFailReturn(FALSE);

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    memset(UT_MEM, 0, sizeof(UT_MEM));
    result = pal_process_spawn(k_image_valid, k_argc_valid, k_argv_valid, &created_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result,
        er_out_of_memory, er_out_of_memory, er_out_of_memory, er_out_of_memory, er_out_of_memory,
        er_out_of_memory, er_out_of_memory, er_out_of_memory, er_out_of_memory, er_out_of_memory,
        er_out_of_memory, er_out_of_memory, er_out_of_memory, er_out_of_memory, er_out_of_memory,
        er_ok);
}

// 
// Test pal_process_kill happy path 
// 
TEST_FUNCTION(pal_win_process_kill__success)
{
    static const HANDLE k_process_handle_valid = (HANDLE)0x2342;
    process_t process_valid;
    process_valid.pi.hProcess = k_process_handle_valid;

    // arrange 
    STRICT_EXPECTED_CALL(TerminateProcess(k_process_handle_valid, 0))
        .SetReturn(TRUE);

    // act 
    pal_process_kill(&process_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_process_kill passing as process argument an invalid process_t* value 
// 
TEST_FUNCTION(pal_win_process_kill__arg_pal_process_null)
{
    // arrange 

    // act 
    pal_process_kill(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_process_yield happy path 
// 
TEST_FUNCTION(pal_win_process_yield__success_1)
{
    static const HANDLE k_process_handle_valid = (HANDLE)0x2342;
    static const HANDLE k_thread_handle_valid = (HANDLE)0x6454;
    static STRING_HANDLE k_cmd_line_handle_valid = (STRING_HANDLE)0x26234;
    process_t process_valid;

    process_valid.pi.hProcess = k_process_handle_valid;
    process_valid.pi.hThread = k_thread_handle_valid;
    process_valid.cmd_line = k_cmd_line_handle_valid;

    // arrange 
    STRICT_EXPECTED_CALL(WaitForSingleObject(k_process_handle_valid, INFINITE))
        .SetReturn(WAIT_OBJECT_0);
    STRICT_EXPECTED_CALL(CloseHandle(k_process_handle_valid))
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(CloseHandle(k_thread_handle_valid))
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(STRING_delete(k_cmd_line_handle_valid));
    STRICT_EXPECTED_CALL(h_free((void*)&process_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    pal_process_yield(&process_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_process_yield happy path 
// 
TEST_FUNCTION(pal_win_process_yield__success_2)
{
    static const HANDLE k_process_handle_valid = (HANDLE)0x2342;
    static const HANDLE k_thread_handle_valid = (HANDLE)0x6454;
    static STRING_HANDLE k_cmd_line_handle_valid = (STRING_HANDLE)0x26234;
    process_t process_valid;

    memset(&process_valid, 0, sizeof(process_t));
    process_valid.pi.hProcess = k_process_handle_valid;

    // arrange 
    STRICT_EXPECTED_CALL(WaitForSingleObject(k_process_handle_valid, INFINITE))
        .SetReturn(WAIT_OBJECT_0);
    STRICT_EXPECTED_CALL(CloseHandle(k_process_handle_valid))
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(h_free((void*)&process_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    pal_process_yield(&process_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_process_yield passing as proc argument an invalid process_t* value 
// 
TEST_FUNCTION(pal_win_process_yield__arg_proc_invalid)
{
    // arrange 

    // act 
    pal_process_yield(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
}


//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

