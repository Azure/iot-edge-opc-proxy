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
MOCKABLE_FUNCTION(WINAPI, HANDLE, FindFirstFileA,
    LPCSTR, lpFileName, LPWIN32_FIND_DATAA, lpFindFileData);
MOCKABLE_FUNCTION(WINAPI, BOOL, FindNextFileA,
    HANDLE, hFindFile, LPWIN32_FIND_DATAA, lpFindFileData);
MOCKABLE_FUNCTION(WINAPI, BOOL, FindClose,
    HANDLE, hFindFile);
MOCKABLE_FUNCTION(WINAPI, DWORD, GetLogicalDrives);

//
// 2. Include unit under test
//
#undef ENABLE_MOCKS
#include "pal_file.h"
#define ENABLE_MOCKS
#include UNIT_C

MOCKABLE_FUNCTION(, int32_t, pal_win_read_dir_callback_mock,
    void*, context, int32_t, error_code, const char*, name, prx_file_info_t*, stat);

//
// 3. Setup test suite
//
BEGIN_DECLARE_TEST_SUITE()
REGISTER_UMOCK_ALIAS_TYPE(DWORD, unsigned int);
REGISTER_UMOCK_ALIAS_TYPE(LPSTR, char*);
REGISTER_UMOCK_ALIAS_TYPE(LPCSTR, const char*);
REGISTER_UMOCK_ALIAS_TYPE(HANDLE, void*);
REGISTER_UMOCK_ALIAS_TYPE(BOOL, bool);
REGISTER_UMOCK_ALIAS_TYPE(STRING_HANDLE, void*);
REGISTER_UMOCK_ALIAS_TYPE(LPWIN32_FIND_DATAA, void*);
REGISTER_UMOCK_ALIAS_TYPE(prx_file_type_t, int);
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
// Test pal_read_dir happy path 
// 
TEST_FUNCTION(pal_win_read_dir__success_0)
{
    static const char* k_dir_name_valid = "valid\\dir";
    static void* k_context_valid = (void*)0x2354;
    static HANDLE k_find_valid = (HANDLE)0x23554;
    WIN32_FIND_DATAA find_data_valid;
    static const char* k_file_name_valid = "file";
    static STRING_HANDLE k_valid_root = (STRING_HANDLE)0x4007;
    static const char* k_find_string_valid = "valid\\dir\\*";
    static STRING_HANDLE k_valid_file = (STRING_HANDLE)0x4507;
    int32_t result;
    prx_file_info_t pal_stat_valid;
    memset(&pal_stat_valid, 0, sizeof(pal_stat_valid));
    pal_stat_valid.type = prx_file_type_file;
    pal_stat_valid.last_atime = pal_stat_valid.last_mtime = (uint64_t)-11644473600LL;

    memset(&find_data_valid, 0, sizeof(find_data_valid));
    strcpy(find_data_valid.cFileName, k_file_name_valid);
    find_data_valid.dwFileAttributes = 0;

    // arrange 
    STRICT_EXPECTED_CALL(string_trim_back_len(k_dir_name_valid, 9, "\\"))
        .SetReturn(9);
    STRICT_EXPECTED_CALL(STRING_construct_n(k_dir_name_valid, 9))
        .SetReturn(k_valid_root);
    STRICT_EXPECTED_CALL(STRING_concat(k_valid_root, "\\"))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_clone(k_valid_root))
        .SetReturn(k_valid_file);
    STRICT_EXPECTED_CALL(STRING_concat(k_valid_file, "*"))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_c_str(k_valid_file))
        .SetReturn(k_find_string_valid);
    STRICT_EXPECTED_CALL(FindFirstFileA(k_find_string_valid, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_lpFindFileData(&find_data_valid, sizeof(find_data_valid))
        .SetReturn(k_find_valid);
    STRICT_EXPECTED_CALL(STRING_delete(k_valid_file));
    STRICT_EXPECTED_CALL(STRING_clone(k_valid_root))
        .SetReturn(k_valid_file);
    STRICT_EXPECTED_CALL(STRING_concat(k_valid_file, k_file_name_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_c_str(k_valid_file))
        .SetReturn(k_find_string_valid);
    STRICT_EXPECTED_CALL(pal_win_read_dir_callback_mock(k_context_valid, er_ok, k_find_string_valid, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(4, &pal_stat_valid, sizeof(prx_file_info_t))
        .SetReturn(er_aborted);
    STRICT_EXPECTED_CALL(FindClose(k_find_valid))
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(STRING_delete(k_valid_file));
    STRICT_EXPECTED_CALL(STRING_delete(k_valid_root));

    // act 
    result = pal_read_dir(k_dir_name_valid, pal_win_read_dir_callback_mock, k_context_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_read_dir happy path 
// 
TEST_FUNCTION(pal_win_read_dir__success_1)
{
    static const char* k_dir_name_valid = "valid\\dir";
    static void* k_context_valid = (void*)0x2354;
    static HANDLE k_find_valid = (HANDLE)0x23554;
    WIN32_FIND_DATAA find_data_valid;
    static const char* k_file_name_valid = "file";
    static STRING_HANDLE k_valid_root = (STRING_HANDLE)0x4007;
    static const char* k_find_string_valid = "valid\\dir\\*";
    static STRING_HANDLE k_valid_file = (STRING_HANDLE)0x4507;
    int32_t result;
    prx_file_info_t pal_stat_valid;
    memset(&pal_stat_valid, 0, sizeof(pal_stat_valid));
    pal_stat_valid.type = prx_file_type_dir;
    pal_stat_valid.last_atime = pal_stat_valid.last_mtime = (uint64_t)-11644473600LL;

    memset(&find_data_valid, 0, sizeof(find_data_valid));
    strcpy(find_data_valid.cFileName, k_file_name_valid);
    find_data_valid.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;

    // arrange 
    STRICT_EXPECTED_CALL(string_trim_back_len(k_dir_name_valid, 9, "\\"))
        .SetReturn(9);
    STRICT_EXPECTED_CALL(STRING_construct_n(k_dir_name_valid, 9))
        .SetReturn(k_valid_root);
    STRICT_EXPECTED_CALL(STRING_concat(k_valid_root, "\\"))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_clone(k_valid_root))
        .SetReturn(k_valid_file);
    STRICT_EXPECTED_CALL(STRING_concat(k_valid_file, "*"))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_c_str(k_valid_file))
        .SetReturn(k_find_string_valid);
    STRICT_EXPECTED_CALL(FindFirstFileA(k_find_string_valid, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_lpFindFileData(&find_data_valid, sizeof(find_data_valid))
        .SetReturn(k_find_valid);
    STRICT_EXPECTED_CALL(STRING_delete(k_valid_file));
    STRICT_EXPECTED_CALL(STRING_clone(k_valid_root))
        .SetReturn(k_valid_file);
    STRICT_EXPECTED_CALL(STRING_concat(k_valid_file, k_file_name_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_c_str(k_valid_file))
        .SetReturn(k_find_string_valid);
    STRICT_EXPECTED_CALL(pal_win_read_dir_callback_mock(k_context_valid, er_ok, k_find_string_valid, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(4, &pal_stat_valid, sizeof(prx_file_info_t))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(FindNextFileA(k_find_valid, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_lpFindFileData(&find_data_valid, sizeof(find_data_valid))
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(STRING_delete(k_valid_file));
    STRICT_EXPECTED_CALL(STRING_clone(k_valid_root))
        .SetReturn(k_valid_file);
    STRICT_EXPECTED_CALL(STRING_concat(k_valid_file, k_file_name_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_c_str(k_valid_file))
        .SetReturn(k_find_string_valid);
    STRICT_EXPECTED_CALL(pal_win_read_dir_callback_mock(k_context_valid, er_ok, k_find_string_valid, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(4, &pal_stat_valid, sizeof(prx_file_info_t))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(FindNextFileA(k_find_valid, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_lpFindFileData(&find_data_valid, sizeof(find_data_valid))
        .SetReturn(FALSE);
    STRICT_EXPECTED_CALL(pal_os_last_error_as_prx_error())
        .SetReturn(er_nomore);
    STRICT_EXPECTED_CALL(FindClose(k_find_valid))
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(pal_win_read_dir_callback_mock(k_context_valid, er_nomore, NULL, NULL))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(STRING_delete(k_valid_file));
    STRICT_EXPECTED_CALL(STRING_delete(k_valid_root));

    // act 
    result = pal_read_dir(k_dir_name_valid, pal_win_read_dir_callback_mock, k_context_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_read_dir happy path 
// 
TEST_FUNCTION(pal_win_read_dir__success_2)
{
    static const char* k_dir_name_valid = "";
    static void* k_context_valid = (void*)0x2354;

    int32_t result;

    prx_file_info_t pal_stat_valid;
    memset(&pal_stat_valid, 0, sizeof(pal_stat_valid));
    pal_stat_valid.type = prx_file_type_dir;

    // arrange 
    STRICT_EXPECTED_CALL(GetLogicalDrives())
        .SetReturn(0xc);
    STRICT_EXPECTED_CALL(pal_win_read_dir_callback_mock(k_context_valid, er_ok, "C:\\", IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(4, &pal_stat_valid, sizeof(prx_file_info_t))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_win_read_dir_callback_mock(k_context_valid, er_ok, "D:\\", IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(4, &pal_stat_valid, sizeof(prx_file_info_t))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_win_read_dir_callback_mock(k_context_valid, er_nomore, NULL, NULL))
        .SetReturn(er_ok);

    // act 
    result = pal_read_dir(k_dir_name_valid, pal_win_read_dir_callback_mock, k_context_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_read_dir passing as dir_name argument an invalid const char* value 
// 
TEST_FUNCTION(pal_win_read_dir__arg_dir_name_invalid)
{
    static const pal_dir_cb_t k_cb_valid = (pal_dir_cb_t)0x235234;
    static void* k_context_valid = (void*)0x2354;
    int32_t result;

    // arrange 

    // act 
    result = pal_read_dir(NULL, k_cb_valid, k_context_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_read_dir passing as cb argument an invalid pal_dir_cb_t value 
// 
TEST_FUNCTION(pal_win_read_dir__arg_cb_invalid)
{
    static const char* k_dir_name_valid = "valid\\dir";
    int32_t result;

    // arrange 

    // act 
    result = pal_read_dir(k_dir_name_valid, NULL, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_read_dir unhappy path 
// 
TEST_FUNCTION(pal_win_read_dir__neg_0)
{
    static const char* k_dir_name_valid = "valid\\dir";
    static void* k_context_valid = (void*)0x2354;
    static HANDLE k_find_valid = (HANDLE)0x23554;
    WIN32_FIND_DATAA find_data_valid;
    static const char* k_file_name_valid = "file";
    static STRING_HANDLE k_valid_root = (STRING_HANDLE)0x4007;
    static const char* k_find_string_valid = "valid\\dir\\*";
    static STRING_HANDLE k_valid_file = (STRING_HANDLE)0x4507;
    int32_t result;

    strcpy(find_data_valid.cFileName, k_file_name_valid);
    find_data_valid.dwFileAttributes = 0;

    // arrange 
    STRICT_EXPECTED_CALL(string_trim_back_len(k_dir_name_valid, 9, "\\"))
        .SetReturn(9);
    STRICT_EXPECTED_CALL(STRING_construct_n(k_dir_name_valid, 9))
        .SetReturn(k_valid_root);
    STRICT_EXPECTED_CALL(STRING_concat(k_valid_root, "\\"))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_clone(k_valid_root))
        .SetReturn(k_valid_file);
    STRICT_EXPECTED_CALL(STRING_concat(k_valid_file, "*"))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_c_str(k_valid_file))
        .SetReturn(k_find_string_valid);
    STRICT_EXPECTED_CALL(FindFirstFileA(k_find_string_valid, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_lpFindFileData(&find_data_valid, sizeof(find_data_valid))
        .SetReturn(INVALID_HANDLE_VALUE);
    STRICT_EXPECTED_CALL(pal_os_last_error_as_prx_error())
        .SetReturn(er_disk_io);
    STRICT_EXPECTED_CALL(pal_win_read_dir_callback_mock(k_context_valid, er_disk_io, NULL, NULL))
        .SetReturn(er_disk_io);
    STRICT_EXPECTED_CALL(STRING_delete(k_valid_file));
    STRICT_EXPECTED_CALL(STRING_delete(k_valid_root));

    // act 
    result = pal_read_dir(k_dir_name_valid, pal_win_read_dir_callback_mock, k_context_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_disk_io, result);
}

// 
// Test pal_read_dir unhappy path 
// 
TEST_FUNCTION(pal_win_read_dir__neg_1)
{
    static const char* k_dir_name_valid = "valid\\dir";
    static void* k_context_valid = (void*)0x2354;
    static HANDLE k_find_valid = (HANDLE)0x23554;
    WIN32_FIND_DATAA find_data_valid;
    static const char* k_file_name_valid = "file";
    static STRING_HANDLE k_valid_root = (STRING_HANDLE)0x4007;
    static const char* k_find_string_valid = "valid\\dir\\*";
    static STRING_HANDLE k_valid_file = (STRING_HANDLE)0x4507;
    int32_t result;

    strcpy(find_data_valid.cFileName, k_file_name_valid);
    find_data_valid.dwFileAttributes = 0;

    REGISTER_GLOBAL_MOCK_RETURNS(pal_os_last_error_as_prx_error, er_fatal, er_fatal);

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(string_trim_back_len(k_dir_name_valid, 9, "\\"))
        .SetReturn(9)
        .SetFailReturn(9);
    STRICT_EXPECTED_CALL(STRING_construct_n(k_dir_name_valid, 9))
        .SetReturn(k_valid_root)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(STRING_concat(k_valid_root, "\\"))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_clone(k_valid_root))
        .SetReturn(k_valid_file)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(STRING_concat(k_valid_file, "*"))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_c_str(k_valid_file))
        .SetReturn(k_find_string_valid)
        .SetFailReturn(k_find_string_valid);
    STRICT_EXPECTED_CALL(FindFirstFileA(k_find_string_valid, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_lpFindFileData(&find_data_valid, sizeof(find_data_valid))
        .SetReturn(k_find_valid)
        .SetFailReturn(INVALID_HANDLE_VALUE);
    STRICT_EXPECTED_CALL(STRING_delete(k_valid_file));
    STRICT_EXPECTED_CALL(STRING_clone(k_valid_root))
        .SetReturn(k_valid_file)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(STRING_concat(k_valid_file, k_file_name_valid))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_c_str(k_valid_file))
        .SetReturn(k_find_string_valid)
        .SetFailReturn(k_find_string_valid);
    STRICT_EXPECTED_CALL(pal_win_read_dir_callback_mock(k_context_valid, er_ok, k_find_string_valid, IGNORED_PTR_ARG))
        .IgnoreArgument(3)
        .SetReturn(er_aborted)
        .SetFailReturn(er_fatal);
    STRICT_EXPECTED_CALL(FindClose(k_find_valid))
        .SetReturn(TRUE)
        .SetFailReturn(FALSE);
    STRICT_EXPECTED_CALL(STRING_delete(k_valid_file));
    STRICT_EXPECTED_CALL(STRING_delete(k_valid_root));

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_read_dir(k_dir_name_valid, pal_win_read_dir_callback_mock, k_context_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result,
        er_ok, er_out_of_memory, er_out_of_memory, er_out_of_memory, er_out_of_memory, er_ok);
}

// 
// Test pal_read_dir unhappy path 
// 
TEST_FUNCTION(pal_win_read_dir__neg_2)
{
    static const char* k_dir_name_valid = "/";
    static void* k_context_valid = (void*)0x2354;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(GetLogicalDrives())
        .SetReturn(0);
    STRICT_EXPECTED_CALL(pal_win_read_dir_callback_mock(k_context_valid, er_nomore, NULL, NULL))
        .SetReturn(er_nomore);

    // act 
    result = pal_read_dir(k_dir_name_valid, pal_win_read_dir_callback_mock, k_context_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_nomore, result);
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

