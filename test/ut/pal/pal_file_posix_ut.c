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
BEGIN_DECLARE_TEST_SETUP()
working_dir = NULL;
END_DECLARE_TEST_SETUP()


// 
// Test pal_file_init happy path 
// 
TEST_FUNCTION(pal_posix_file_init__success_1)
{
    static const char* k_folder_valid = "Test/test/TEST/";
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(getcwd(IGNORED_PTR_ARG, 4095))
        .CopyOutArgumentBuffer_buf(k_folder_valid, strlen(k_folder_valid))
        .SetReturn((char*)k_folder_valid);
    STRICT_EXPECTED_CALL(string_clone(k_folder_valid, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_copy(&k_folder_valid, sizeof(k_folder_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);

    // act 
    result = pal_file_init();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(char_ptr, k_folder_valid, working_dir);
}

// 
// Test pal_file_init happy path 
// 
TEST_FUNCTION(pal_posix_file_init__success_2)
{
    static const char* k_folder_valid = "Test/test/TEST/";
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(getcwd(IGNORED_PTR_ARG, 4095))
        .CopyOutArgumentBuffer_buf(k_folder_valid, strlen(k_folder_valid))
        .SetReturn((char*)NULL);

    // act 
    result = pal_file_init();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_IS_NULL(working_dir);
}

// 
// Test pal_file_init unhappy path 
// 
TEST_FUNCTION(pal_posix_file_init__neg)
{
    static const char* k_folder_valid = "Test/test/TEST/";
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(getcwd(IGNORED_PTR_ARG, 4095))
        .CopyOutArgumentBuffer_buf(k_folder_valid, strlen(k_folder_valid))
        .SetReturn((char*)k_folder_valid);
    STRICT_EXPECTED_CALL(string_clone(k_folder_valid, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .SetReturn(er_out_of_memory);

    // act 
    result = pal_file_init();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_out_of_memory, result);
    ASSERT_IS_NULL(working_dir);
}

// 
// Test pal_create_full_path happy path 
// 
TEST_FUNCTION(pal_posix_create_full_path__success_1)
{
    static const char* k_file_name_valid = "foo.txt";
    char* full_path_buffer = UT_MEM + MAX_PATH;
    const char* result;

    working_dir = NULL;

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(8, NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)full_path_buffer)
        .SetFailReturn(NULL);

    // act 
    result = pal_create_full_path(k_file_name_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, k_file_name_valid, result);
}

// 
// Test pal_create_full_path happy path 
// 
TEST_FUNCTION(pal_posix_create_full_path__success_2)
{
    static const char* k_file_name_valid = "foo.txt";
    static const char* k_working_dir_valid = "working/dir";
    static const char* k_full_path_valid = "working/dir/foo.txt";
    char* full_path_buffer = UT_MEM + MAX_PATH;
    const char* result;

    working_dir = (char*)k_working_dir_valid;

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(20, NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)full_path_buffer)
        .SetFailReturn(NULL);

    // act 
    result = pal_create_full_path(k_file_name_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, k_full_path_valid, result);
}

// 
// Test pal_create_full_path passing as file_name argument an invalid const char* value 
// 
TEST_FUNCTION(pal_posix_create_full_path__arg_file_name_invalid)
{
    const char* result;

    // arrange 

    // act 
    result = pal_create_full_path(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test pal_create_full_path unhappy path 
// 
TEST_FUNCTION(pal_posix_create_full_path__neg)
{
    static const char* k_file_name_valid = "test.conf";
    static const char* k_working_dir_valid = "a/b/c/d/e";
    static const char* k_full_path_valid = "a/b/c/d/e/test.conf";
    char* full_path_buffer = UT_MEM + MAX_PATH;
    const char* result;

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(h_realloc(20, NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)full_path_buffer)
        .SetFailReturn(NULL);

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    working_dir = (char*)k_working_dir_valid;
    result = pal_create_full_path(k_file_name_valid);

    // assert    
    UMOCK_C_NEGATIVE_TESTS_ASSERT(char_ptr, result, NULL);
}

// 
// Test pal_file_deinit happy path 
// 
TEST_FUNCTION(pal_posix_file_deinit__success_1)
{
    static char* k_path_valid = "";
    working_dir = k_path_valid;

    // arrange 
    STRICT_EXPECTED_CALL(h_free((void*)k_path_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    pal_file_deinit();

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_file_deinit happy path 
// 
TEST_FUNCTION(pal_posix_file_deinit__success_2)
{
    working_dir = NULL;

    // arrange 

    // act 
    pal_file_deinit();

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
// Test pal_set_working_dir happy path 
// 
TEST_FUNCTION(pal_posix_set_working_dir__success_1)
{
    static const char* k_dir_valid = "wdur";
    int32_t result;

    working_dir = NULL;

    // arrange 
    STRICT_EXPECTED_CALL(string_clone(k_dir_valid, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_copy(&k_dir_valid, sizeof(k_dir_valid))
        .SetReturn(er_ok);

    // act 
    result = pal_set_working_dir(k_dir_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(void_ptr, k_dir_valid, working_dir);
}

// 
// Test pal_set_working_dir happy path 
// 
TEST_FUNCTION(pal_posix_set_working_dir__success_2)
{
    static const char* k_dir_valid = "fme";
    int32_t result;

    working_dir = (char*)k_dir_valid;

    // arrange 
    STRICT_EXPECTED_CALL(h_free((void*)k_dir_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);
    STRICT_EXPECTED_CALL(string_clone(k_dir_valid, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_copy(&k_dir_valid, sizeof(k_dir_valid))
        .SetReturn(er_ok);

    // act 
    result = pal_set_working_dir(k_dir_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(void_ptr, k_dir_valid, working_dir);
}

// 
// Test pal_set_working_dir passing as dir argument an invalid const char* value 
// 
TEST_FUNCTION(pal_posix_set_working_dir__arg_dir_null)
{
    int32_t result;

    // arrange 

    // act 
    result = pal_set_working_dir(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_set_working_dir unhappy path 
// 
TEST_FUNCTION(pal_posix_set_working_dir__neg)
{
    static const char* k_dir_valid = "naja";
    int32_t result;

    working_dir = NULL;

    // arrange 
    STRICT_EXPECTED_CALL(string_clone(k_dir_valid, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .SetReturn(er_out_of_memory);

    // act 
    result = pal_set_working_dir(k_dir_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_out_of_memory, result);
}

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

