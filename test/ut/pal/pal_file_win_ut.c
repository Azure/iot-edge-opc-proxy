// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST pal_file_win
#include "util_ut.h"

//
// 1. Required mocks
//
#include "os.h"
#include "prx_types.h"
#include "util_string.h"

// fileapi.h
MOCKABLE_FUNCTION(WINAPI, DWORD, GetFileAttributesA,
    LPCSTR, lpFileName);
MOCKABLE_FUNCTION(WINAPI, DWORD, GetFullPathNameA,
    LPCSTR, lpFileName, DWORD, nBufferLength, LPSTR, lpBuffer, LPSTR*, lpFilePart);

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
REGISTER_UMOCK_ALIAS_TYPE(DWORD, unsigned int);
REGISTER_UMOCK_ALIAS_TYPE(LPSTR, char*);
REGISTER_UMOCK_ALIAS_TYPE(LPCSTR, const char*);
END_DECLARE_TEST_SUITE()
// -or- DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
DECLARE_TEST_SETUP()

// 
// Test pal_file_init happy path 
// 
TEST_FUNCTION(pal_win_file_init__success)
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
TEST_FUNCTION(pal_win_file_exists__success_1)
{
    static const char* k_file_valid = "dir.txt";
    bool result;

    // arrange 
    STRICT_EXPECTED_CALL(GetFileAttributesA(k_file_valid))
        .SetReturn((DWORD)~FILE_ATTRIBUTE_DIRECTORY);

    // act 
    result = pal_file_exists(k_file_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_TRUE(result);
}

// 
// Test pal_file_exists happy path 
// 
TEST_FUNCTION(pal_win_file_exists__success_2)
{
    static const char* k_file_valid = "dir";
    bool result;

    // arrange 
    STRICT_EXPECTED_CALL(GetFileAttributesA(k_file_valid))
        .SetReturn(FILE_ATTRIBUTE_DIRECTORY);

    // act 
    result = pal_file_exists(k_file_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_FALSE(result);
}

// 
// Test pal_file_exists happy path 
// 
TEST_FUNCTION(pal_win_file_exists__success_3)
{
    static const char* k_file_valid = "noneexistantfile";
    bool result;

    // arrange 
    STRICT_EXPECTED_CALL(GetFileAttributesA(k_file_valid))
        .SetReturn(INVALID_FILE_ATTRIBUTES);

    // act 
    result = pal_file_exists(k_file_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_FALSE(result);
}

// 
// Test pal_file_exists passing as file_name argument an invalid const char* value 
// 
TEST_FUNCTION(pal_win_file_exists__arg_file_name_invalid)
{
    bool result;

    // arrange 
    STRICT_EXPECTED_CALL(GetFileAttributesA(NULL))
        .SetReturn(INVALID_FILE_ATTRIBUTES);

    // act 
    result = pal_file_exists(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_FALSE(result);
}

// 
// Test pal_get_real_path happy path 
// 
TEST_FUNCTION(pal_win_get_real_path__success_1)
{
    static const char* k_file_name_valid = "c:\\test\\..\\foo.txt";
    static const char* k_full_path_valid = "c:\\foo.txt";
    char* full_path_buffer = UT_MEM + MAX_PATH;
    const char* full_path;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(MAX_PATH, NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)full_path_buffer);
    STRICT_EXPECTED_CALL(GetFullPathNameA(k_file_name_valid, MAX_PATH, IGNORED_PTR_ARG, NULL))
        .CopyOutArgumentBuffer(3, k_full_path_valid, strlen(k_full_path_valid) + 1)
        .SetReturn(MAX_PATH);

    // act 
    result = pal_get_real_path(k_file_name_valid, &full_path);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(char_ptr, k_full_path_valid, full_path);
}

// 
// Test pal_get_real_path happy path 
// 
TEST_FUNCTION(pal_win_get_real_path__success_2)
{
    static const char* k_file_name_valid = "foo.txt";
    static const char* k_full_path_valid = "c:\\working\\dir\\foo.txt";
    char* full_path_buffer = UT_MEM + MAX_PATH;
    const char* full_path;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(MAX_PATH, NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)full_path_buffer);
    STRICT_EXPECTED_CALL(GetFullPathNameA(k_file_name_valid, MAX_PATH, IGNORED_PTR_ARG, NULL))
        .CopyOutArgumentBuffer(3, k_full_path_valid, strlen(k_full_path_valid) + 1)
        .SetReturn(MAX_PATH);

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
TEST_FUNCTION(pal_win_get_real_path__arg_file_name_invalid)
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
TEST_FUNCTION(pal_win_get_real_path__arg_real_path_invalid)
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
TEST_FUNCTION(pal_win_get_real_path__neg_1)
{
    static const char* k_file_name_valid = "foo.txt";
    const char* full_path;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(MAX_PATH, NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn(NULL);

    // act 
    result = pal_get_real_path(k_file_name_valid, &full_path);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_out_of_memory, result);
}

// 
// Test pal_get_real_path unhappy path 
// 
TEST_FUNCTION(pal_win_get_real_path__neg_2)
{
    static const char* k_file_name_valid = "foo.txt";
    static const char* k_full_path_valid = "c:\\working\\dir\\foo.txt";
    char* full_path_buffer = UT_MEM + MAX_PATH;
    const char* full_path;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(MAX_PATH, NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)full_path_buffer);
    STRICT_EXPECTED_CALL(GetFullPathNameA(k_file_name_valid, MAX_PATH, IGNORED_PTR_ARG, NULL))
        .CopyOutArgumentBuffer(3, k_full_path_valid, strlen(k_full_path_valid) + 1)
        .SetReturn(0);
    STRICT_EXPECTED_CALL(h_free((void*)full_path_buffer, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
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
// Test pal_get_real_path unhappy path 
// 
TEST_FUNCTION(pal_win_get_real_path__neg_3)
{
    static const char* k_file_name_valid = "superlongfilenamethatexceedsmaxpathyaddyyaddyda...";
    char* full_path_buffer = UT_MEM + MAX_PATH;
    const char* full_path;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(MAX_PATH, NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)full_path_buffer);
    STRICT_EXPECTED_CALL(GetFullPathNameA(k_file_name_valid, MAX_PATH, IGNORED_PTR_ARG, NULL))
        .IgnoreArgument(3)
        .SetReturn(MAX_PATH + 1);
    STRICT_EXPECTED_CALL(h_free((void*)full_path_buffer, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    result = pal_get_real_path(k_file_name_valid, &full_path);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fatal, result);
}

// 
// Test pal_file_deinit happy path 
// 
TEST_FUNCTION(pal_win_file_deinit__success_1)
{
    // arrange 

    // act 
    pal_file_deinit();

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_file_deinit happy path 
// 
TEST_FUNCTION(pal_win_file_deinit__success_2)
{
    // arrange 

    // act 
    pal_file_deinit();

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_free_path happy path 
// 
TEST_FUNCTION(pal_win_free_path__success)
{
    static const char* k_path_valid = "";

    // arrange 
    STRICT_EXPECTED_CALL(h_free((void*)k_path_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    pal_free_path(k_path_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_free_path passing as path argument an invalid const char* value 
// 
TEST_FUNCTION(pal_win_free_path__arg_path_null)
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

