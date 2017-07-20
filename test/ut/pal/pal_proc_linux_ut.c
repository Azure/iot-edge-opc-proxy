// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST pal_proc_linux
#include "util_ut.h"

//
// 1. Required mocks
//
#include "os.h"

// sys/wait.h
MOCKABLE_FUNCTION(, pid_t, waitpid,
    pid_t, pid, int*, stat_loc, int, options);
// signal.h
MOCKABLE_FUNCTION(, int, kill,
    pid_t, pid, int, sig);
// unitstd.h
MOCKABLE_FUNCTION(, int, execvpe,
    const char*, file, char**, argv, char**, envp);
MOCKABLE_FUNCTION(, pid_t, fork);

// stdio
#undef environ
MOCKABLE_FUNCTION(, char**, environ_mock);
#define environ environ_mock()
MOCKABLE_FUNCTION(, int, fflush_mock,
    void*, stream);
#define fflush fflush_mock
MOCKABLE_FUNCTION(, void, exit_mock,
    int, code);
#define exit exit_mock


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
REGISTER_UMOCK_ALIAS_TYPE(pid_t, int);
END_DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
DECLARE_TEST_SETUP()

//
// Test pal_process_spawn happy path
//
TEST_FUNCTION(pal_linux_process_spawn__success_1)
{
    static const char* k_image_valid = "test";
    static const int32_t k_argc_valid = 5;
    static const char* k_argv_valid[] = { "a", "b", "c", "d", "e" };
    static const pid_t k_process_id_valid = (pid_t)0x232423;
    char** args_valid = (char**)(UT_MEM + sizeof(process_t));
    process_t* created_valid;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange
    STRICT_EXPECTED_CALL(h_realloc(sizeof(process_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(h_realloc(sizeof(char*) * 6, NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)args_valid);
    for (int32_t i = 0; i < k_argc_valid; i++)
    {
        STRICT_EXPECTED_CALL(string_clone(k_argv_valid[i], IGNORED_PTR_ARG))
            .IgnoreArgument(2)
            .SetReturn(er_ok);
    }
    STRICT_EXPECTED_CALL(fflush(NULL))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(fork())
        .SetReturn(k_process_id_valid); // Parent

    // act
    result = pal_process_spawn(k_image_valid, k_argc_valid, k_argv_valid, &created_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

//
// Test pal_process_spawn happy path
//
TEST_FUNCTION(pal_linux_process_spawn__success_2)
{
    static const char* k_image_valid = "test";
    static char** k_env_valid = (char**)0x562345;
    char** args_valid = (char**)(UT_MEM + sizeof(process_t));
    process_t* created_valid;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange
    STRICT_EXPECTED_CALL(h_realloc(sizeof(process_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(h_realloc(sizeof(char*), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)args_valid);
    STRICT_EXPECTED_CALL(fflush(NULL))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(fork())
        .SetReturn(0);  // Child
    STRICT_EXPECTED_CALL(environ_mock())
        .SetReturn(k_env_valid);
    STRICT_EXPECTED_CALL(execvpe(k_image_valid, args_valid, k_env_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(exit(-1));

    // act
    result = pal_process_spawn(k_image_valid, 0, NULL, &created_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

//
// Test pal_process_spawn passing as created argument an invalid process_t** value
//
TEST_FUNCTION(pal_linux_process_spawn__arg_created_null)
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
TEST_FUNCTION(pal_linux_process_spawn__neg)
{
    static const char* k_image_valid = "test";
    static const char* k_argv_valid[] = { "a", "b", "c", "d", "e" };
    static const int32_t k_argc_valid = 5;
    static const char* k_cmd_line_valid[] = { "a", "b", "c", "d", "e", 0 };
    static const pid_t k_process_id_valid = (pid_t)0x232423;
    process_t* created_valid;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(h_realloc(sizeof(process_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(h_realloc(sizeof(char*) * 6, NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)k_cmd_line_valid)
        .SetFailReturn(NULL);
    for (int32_t i = 0; i < k_argc_valid; i++)
    {
        STRICT_EXPECTED_CALL(string_clone(k_argv_valid[i], IGNORED_PTR_ARG))
            .IgnoreArgument(2)
            .SetReturn(er_ok)
            .SetFailReturn(er_out_of_memory);
    }
    STRICT_EXPECTED_CALL(fflush(NULL))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(fork())
        .SetReturn(k_process_id_valid) // Parent
        .SetFailReturn(-1);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    memset(UT_MEM, 0, sizeof(UT_MEM));
    result = pal_process_spawn(k_image_valid, k_argc_valid, k_argv_valid, &created_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result,
        er_out_of_memory, er_out_of_memory, er_out_of_memory, er_out_of_memory, er_out_of_memory,
        er_out_of_memory, er_out_of_memory, er_ok);
}

//
// Test pal_process_kill happy path
//
TEST_FUNCTION(pal_linux_process_kill__success)
{
    static const pid_t k_process_id_valid = (pid_t)0x2342;
    process_t process_valid;
    process_valid.pid = k_process_id_valid;

    // arrange
    STRICT_EXPECTED_CALL(kill(k_process_id_valid, SIGINT))
        .SetReturn(0);

    // act
    pal_process_kill(&process_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
}

//
// Test pal_process_kill passing as process argument an invalid process_t* value
//
TEST_FUNCTION(pal_linux_process_kill__arg_pal_process_null)
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
TEST_FUNCTION(pal_linux_process_yield__success)
{
    static const pid_t k_process_id_valid = (pid_t)0x2342;
    static const char* k_cmd_line_valid[] = { "a", "b", "c", "d", "e", 0 };
    process_t process_valid;

    process_valid.pid = k_process_id_valid;
    process_valid.cmd_line = (char**)k_cmd_line_valid;

    // arrange
    STRICT_EXPECTED_CALL(fflush(NULL))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(waitpid(k_process_id_valid, IGNORED_PTR_ARG, 0))
        .IgnoreArgument(2)
        .SetReturn(0);
    for (size_t i = 0; i < _countof(k_cmd_line_valid) - 1; i++)
    {
        STRICT_EXPECTED_CALL(h_free((void*)k_cmd_line_valid[i], IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
            .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);
    }
    STRICT_EXPECTED_CALL(h_free((void*)k_cmd_line_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);
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
TEST_FUNCTION(pal_linux_process_yield__arg_proc_invalid)
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

