// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST pal_file_posix
#include "util_ut.h"

//
// 1. Required mocks
//
#include "os.h"
#include "prx_types.h"

MOCKABLE_FUNCTION(, char*, getcwd, char*, buf, size_t, size);
MOCKABLE_FUNCTION(, long, pathconf, const char*, path, int, name);
MOCKABLE_FUNCTION(, char*, realpath, const char*, path, char*, resolved_path);
MOCKABLE_FUNCTION(, int, access, const char*, pathname, int, mode);

//
// 2. Include unit under test
//
#undef ENABLE_MOCKS
#include "pal_file.h"
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
// Test pal_file_init happy path 
// 
TEST_FUNCTION(pal_posix_file_init__success)
{
    int32_t result;

    // arrange 

    // act 
    result = pal_file_init();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}


// 
// Test pal_file_exists happy path 
// 
TEST_FUNCTION(pal_posix_file_exists__success_1)
{
    static const char* k_file_valid = "fileexists";
    bool result;

    // arrange 
    STRICT_EXPECTED_CALL(access(k_file_valid, F_OK))
        .SetReturn(0);

    // act 
    result = pal_file_exists(k_file_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_TRUE(result);
}

// 
// Test pal_file_exists happy path 
// 
TEST_FUNCTION(pal_posix_file_exists__success_2)
{
    static const char* k_file_valid = "/test/notexist";
    bool result;

    // arrange 
    STRICT_EXPECTED_CALL(access(k_file_valid, F_OK))
        .SetReturn(-1);

    // act 
    result = pal_file_exists(k_file_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_FALSE(result);
}

// 
// Test pal_file_exists passing as file_name argument an invalid const char* value 
// 
TEST_FUNCTION(pal_posix_file_exists__arg_file_name_invalid)
{
    bool result;

    // arrange 
    STRICT_EXPECTED_CALL(access(NULL, F_OK))
        .SetReturn(-1);

    // act 
    result = pal_file_exists(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_FALSE(result);
}

// 
// Test pal_get_real_path happy path 
// 
TEST_FUNCTION(pal_posix_get_real_path__success_1)
{
    static const char* k_file_name_valid = "/home/../foo.txt";
    static const char* k_full_path_valid = "/foo.txt";
    char* full_path_buffer = UT_MEM;
    const char* full_path;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(pathconf(".", _PC_PATH_MAX))
        .SetReturn(MAX_PATH);
    STRICT_EXPECTED_CALL(c_realloc(MAX_PATH + 1, NULL, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(3).IgnoreArgument(4).IgnoreArgument(5)
        .SetReturn((void*)full_path_buffer);
    STRICT_EXPECTED_CALL(realpath(k_file_name_valid, full_path_buffer))
        .CopyOutArgumentBuffer(2, k_full_path_valid, strlen(k_full_path_valid) + 1)
        .SetReturn((char*)k_full_path_valid);

    // act 
    result = pal_get_real_path(k_file_name_valid, &full_path);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(char_ptr, full_path_buffer, full_path);
}

// 
// Test pal_get_real_path happy path 
// 
TEST_FUNCTION(pal_posix_get_real_path__success_2)
{
    static const char* k_file_name_valid = "foo.txt";
    static const char* k_full_path_valid = "/working/dir/foo.txt";
    const char* full_path;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(pathconf(".", _PC_PATH_MAX))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(realpath(k_file_name_valid, NULL))
        .SetReturn((char*)k_full_path_valid);

    // act 
    result = pal_get_real_path(k_file_name_valid, &full_path);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(char_ptr, k_full_path_valid, full_path);
}

// 
// Test pal_get_real_path passing as file_name argument an invalid const char* value 
// 
TEST_FUNCTION(pal_posix_get_real_path__arg_file_name_invalid)
{
    const char* full_path;
    int32_t result;

    // arrange 

    // act 
    result = pal_get_real_path(NULL, &full_path);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_get_real_path passing as real_path argument an invalid const char* value 
// 
TEST_FUNCTION(pal_posix_get_real_path__arg_real_path_invalid)
{
    static const char* k_file_name_valid = "test.conf";
    int32_t result;

    // arrange 

    // act 
    result = pal_get_real_path(k_file_name_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_get_real_path unhappy path 
// 
TEST_FUNCTION(pal_posix_get_real_path__neg_1)
{
    static const char* k_file_name_valid = "/home/../foo.txt";
    const char* full_path;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(pathconf(".", _PC_PATH_MAX))
        .SetReturn(MAX_PATH);
    STRICT_EXPECTED_CALL(c_realloc(MAX_PATH + 1, NULL, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(3).IgnoreArgument(4).IgnoreArgument(5)
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(realpath(k_file_name_valid, NULL))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(pal_os_last_error_as_prx_error())
        .SetReturn(er_out_of_memory);

    // act 
    result = pal_get_real_path(k_file_name_valid, &full_path);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_out_of_memory, result);
}

// 
// Test pal_get_real_path unhappy path 
// 
TEST_FUNCTION(pal_posix_get_real_path__neg_2)
{
    static const char* k_file_name_valid = "/home/../foo.txt";
    char* full_path_buffer = UT_MEM;
    const char* full_path;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(pathconf(".", _PC_PATH_MAX))
        .SetReturn(MAX_PATH);
    STRICT_EXPECTED_CALL(c_realloc(MAX_PATH + 1, NULL, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(3).IgnoreArgument(4).IgnoreArgument(5)
        .SetReturn((void*)full_path_buffer);
    STRICT_EXPECTED_CALL(realpath(k_file_name_valid, full_path_buffer))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(c_free((void*)full_path_buffer, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);
    STRICT_EXPECTED_CALL(pal_os_last_error_as_prx_error())
        .SetReturn(er_fatal);

    // act 
    result = pal_get_real_path(k_file_name_valid, &full_path);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fatal, result);
}

// 
// Test pal_file_deinit happy path 
// 
TEST_FUNCTION(pal_posix_file_deinit__success)
{
    // arrange 

    // act 
    pal_file_deinit();

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_posix_free happy path 
// 
TEST_FUNCTION(pal_posix_free_path__success)
{
    static const char* k_file_name_valid = "test.conf";

    // arrange 
    STRICT_EXPECTED_CALL(c_free((void*)k_file_name_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    pal_free_path(k_file_name_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_free_path passing as path argument an invalid const char* value 
// 
TEST_FUNCTION(pal_posix_free_path__arg_path_null)
{
    // arrange 

    // act 
    pal_free_path(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

