// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST pal_file_posix
#include "util_ut.h"

//
// 1. Required mocks
//
#include "os.h"
// realpath is inline in stdlib.h on older distributions
#define realpath __realpath
#include "prx_types.h"

MOCKABLE_FUNCTION(, char*, getcwd, char*, buf, size_t, size);
MOCKABLE_FUNCTION(, long, pathconf, const char*, path, int, name);
MOCKABLE_FUNCTION(, char*, realpath, const char*, path, char*, resolved_path);
MOCKABLE_FUNCTION(, int, access, const char*, pathname, int, mode);
MOCKABLE_FUNCTION(, int, closedir, DIR*, dir);
MOCKABLE_FUNCTION(, DIR*, opendir, const char*, name);
MOCKABLE_FUNCTION(, struct dirent*, readdir, DIR*, dir);
MOCKABLE_FUNCTION(, int, stat, const char*, name, struct stat*, buf);

// errno
#undef errno
MOCKABLE_FUNCTION(, int*, errno_mock);
#define errno (*errno_mock())

//
// 2. Include unit under test
//
#undef ENABLE_MOCKS
#include "pal_file.h"
#define ENABLE_MOCKS
#include UNIT_C

MOCKABLE_FUNCTION(, int32_t, pal_posix_read_dir_callback_mock,
    void*, context, int32_t, error_code, const char*, name, prx_file_info_t*, stat);

//
// 3. Setup test suite
//
BEGIN_DECLARE_TEST_SUITE()
REGISTER_UMOCK_ALIAS_TYPE(prx_file_type_t, int);
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
// Test pal_get_real_path happy path 
// 
TEST_FUNCTION(pal_posix_get_real_path__success_3)
{
    static const char* k_file_name_valid = "foo.txt";
    static const char* k_cwd_path_valid = "/working/dir";
    static const char* k_full_path_valid = "/working/dir/foo.txt";
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
    STRICT_EXPECTED_CALL(getcwd(full_path_buffer, MAX_PATH))
        .CopyOutArgumentBuffer_buf(k_cwd_path_valid, strlen(k_cwd_path_valid) + 1)
        .SetReturn(full_path_buffer);

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
TEST_FUNCTION(pal_posix_get_real_path__success_4)
{
    static const char* k_file_name_valid = "./foo.txt";
    static const char* k_cwd_path_valid = "/working/dir";
    static const char* k_full_path_valid = "/working/dir/foo.txt";
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
    STRICT_EXPECTED_CALL(getcwd(full_path_buffer, MAX_PATH))
        .CopyOutArgumentBuffer_buf(k_cwd_path_valid, strlen(k_cwd_path_valid)+1)
        .SetReturn(full_path_buffer);

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
TEST_FUNCTION(pal_posix_get_real_path__success_5)
{
    static const char* k_file_name_valid = "/working/dir/foo.txt";
    char* full_path_buffer = UT_MEM;
    const char* full_path;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(pathconf(".", _PC_PATH_MAX))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(realpath(k_file_name_valid, NULL))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(c_realloc(20 + 1, NULL, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(3).IgnoreArgument(4).IgnoreArgument(5)
        .SetReturn((void*)full_path_buffer);

    // act 
    result = pal_get_real_path(k_file_name_valid, &full_path);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(char_ptr, k_file_name_valid, full_path);
}

// 
// Test pal_get_real_path happy path 
// 
TEST_FUNCTION(pal_posix_get_real_path__success_6)
{
    static const char* k_file_name_valid = "./working/dir/foo.txt";
    static const char* k_cwd_path_valid = "/working/dir";
    static const char* k_full_path_valid = "/working/dir/working/dir/foo.txt";
    char* full_path_buffer = UT_MEM;
    const char* full_path;
    int errno_valid = ERANGE;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 
    STRICT_EXPECTED_CALL(pathconf(".", _PC_PATH_MAX))
        .SetReturn(16);
    STRICT_EXPECTED_CALL(c_realloc(16 + 1, NULL, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(3).IgnoreArgument(4).IgnoreArgument(5)
        .SetReturn((void*)full_path_buffer);
    STRICT_EXPECTED_CALL(realpath(k_file_name_valid, full_path_buffer))
        .SetReturn(NULL);

    STRICT_EXPECTED_CALL(getcwd(full_path_buffer, 16))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(errno_mock())
        .SetReturn(&errno_valid);
    STRICT_EXPECTED_CALL(c_realloc(32 + 1, full_path_buffer, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(3).IgnoreArgument(4).IgnoreArgument(5)
        .SetReturn((void*)full_path_buffer);

    STRICT_EXPECTED_CALL(getcwd(full_path_buffer, 32))
        .CopyOutArgumentBuffer_buf(k_cwd_path_valid, strlen(k_cwd_path_valid) + 1)
        .SetReturn(full_path_buffer);

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
    STRICT_EXPECTED_CALL(c_realloc(16 + 1, NULL, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(3).IgnoreArgument(4).IgnoreArgument(5)
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
TEST_FUNCTION(pal_posix_get_real_path__neg_2)
{
    static const char* k_file_name_valid = "./foo.txt";
    static const char* k_cwd_path_valid = "/working/dir";
    const char* full_path;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(pathconf(".", _PC_PATH_MAX))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(realpath(k_file_name_valid, NULL))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(c_realloc(1024 + 1, NULL, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(3).IgnoreArgument(4).IgnoreArgument(5)
        .SetReturn(NULL);

    // act 
    result = pal_get_real_path(k_file_name_valid, &full_path);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_out_of_memory, result);
}

// 
// Test pal_get_real_path happy path 
// 
TEST_FUNCTION(pal_posix_get_real_path__neg_3)
{
    static const char* k_file_name_valid = "./working/dir/foo.txt";
    static const char* k_cwd_path_valid = "/working/dir";
    char* full_path_buffer = UT_MEM;
    const char* full_path;
    int errno_valid = ERANGE;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 
    STRICT_EXPECTED_CALL(pathconf(".", _PC_PATH_MAX))
        .SetReturn(16);
    STRICT_EXPECTED_CALL(c_realloc(16 + 1, NULL, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(3).IgnoreArgument(4).IgnoreArgument(5)
        .SetReturn((void*)full_path_buffer);
    STRICT_EXPECTED_CALL(realpath(k_file_name_valid, full_path_buffer))
        .SetReturn(NULL);

    STRICT_EXPECTED_CALL(getcwd(full_path_buffer, 16))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(errno_mock())
        .SetReturn(&errno_valid);
    STRICT_EXPECTED_CALL(c_realloc(32 + 1, full_path_buffer, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(3).IgnoreArgument(4).IgnoreArgument(5)
        .SetReturn(NULL);

    STRICT_EXPECTED_CALL(c_free(full_path_buffer, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    result = pal_get_real_path(k_file_name_valid, &full_path);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_out_of_memory, result);
}

// 
// Test pal_read_dir happy path 
// 
TEST_FUNCTION(pal_posix_read_dir__success_0)
{
    static const char* k_dir_name_valid = "valid/dir";
    static void* k_context_valid = (void*)0x2354;
    static DIR* k_dir_valid = (DIR*)0x23554;
    static struct dirent dp_valid;
    static const char* k_file_name_valid = "file";
    static struct stat stat_valid;
    int32_t result;
    prx_file_info_t pal_stat_valid;

    memset(&pal_stat_valid, 0, sizeof(prx_file_info_t));
    pal_stat_valid.type = prx_file_type_file;

    dp_valid.d_name = (char*)k_file_name_valid;
    memset(&stat_valid, 0, sizeof(stat_valid));
    stat_valid.st_mode = 0;

    // arrange 
    STRICT_EXPECTED_CALL(opendir(k_dir_name_valid))
        .SetReturn(k_dir_valid);
    STRICT_EXPECTED_CALL(readdir(k_dir_valid))
        .SetReturn(&dp_valid);
    STRICT_EXPECTED_CALL(stat(k_file_name_valid, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_buf(&stat_valid, sizeof(stat_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(pal_posix_read_dir_callback_mock(k_context_valid, er_ok, k_file_name_valid, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(4, &pal_stat_valid, sizeof(prx_file_info_t))
        .SetReturn(er_aborted);
    STRICT_EXPECTED_CALL(closedir(k_dir_valid))
        .SetReturn(0);

    // act 
    result = pal_read_dir(k_dir_name_valid, pal_posix_read_dir_callback_mock, k_context_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_read_dir happy path 
// 
TEST_FUNCTION(pal_posix_read_dir__success_1)
{
    static const char* k_dir_name_valid = "";
    static void* k_context_valid = (void*)0x2354;
    static DIR* k_dir_valid = (DIR*)0x23554;
    static struct dirent dp_valid;
    static const char* k_file_name_valid = "file";
    static struct stat stat_valid;
    int32_t result;
    prx_file_info_t pal_stat_valid;

    memset(&pal_stat_valid, 0, sizeof(prx_file_info_t));
    pal_stat_valid.type = prx_file_type_dir;

    dp_valid.d_name = (char*)k_file_name_valid;
    memset(&stat_valid, 0, sizeof(stat_valid));
    stat_valid.st_mode = S_IFDIR;

    // arrange 
    STRICT_EXPECTED_CALL(opendir("/"))
        .SetReturn(k_dir_valid);
    STRICT_EXPECTED_CALL(readdir(k_dir_valid))
        .SetReturn(&dp_valid);
    STRICT_EXPECTED_CALL(stat(k_file_name_valid, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_buf(&stat_valid, sizeof(stat_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(pal_posix_read_dir_callback_mock(k_context_valid, er_ok, k_file_name_valid, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(4, &pal_stat_valid, sizeof(prx_file_info_t))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(readdir(k_dir_valid))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(pal_os_last_error_as_prx_error())
        .SetReturn(er_nomore);
    STRICT_EXPECTED_CALL(closedir(k_dir_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(pal_posix_read_dir_callback_mock(k_context_valid, er_nomore, NULL, NULL))
        .SetReturn(er_ok);

    // act 
    result = pal_read_dir(k_dir_name_valid, pal_posix_read_dir_callback_mock, k_context_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_read_dir passing as dir_name argument an invalid const char* value 
// 
TEST_FUNCTION(pal_posix_read_dir__arg_dir_name_invalid)
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
TEST_FUNCTION(pal_posix_read_dir__arg_cb_invalid)
{
    static const char* k_dir_name_valid = "valid/dir";
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
TEST_FUNCTION(pal_posix_read_dir__neg_0)
{
    static const char* k_dir_name_valid = "valid/dir";
    static void* k_context_valid = (void*)0x2354;
    static DIR* k_dir_valid = (DIR*)0x23554;
    static struct dirent dp_valid;
    static const char* k_file_name_valid = "file";
    int32_t result;

    dp_valid.d_name = (char*)k_file_name_valid;

    // arrange 
    STRICT_EXPECTED_CALL(opendir(k_dir_name_valid))
        .SetReturn(k_dir_valid);
    STRICT_EXPECTED_CALL(readdir(k_dir_valid))
        .SetReturn(&dp_valid);
    STRICT_EXPECTED_CALL(stat(k_file_name_valid, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(pal_os_last_error_as_prx_error())
        .SetReturn(er_disk_io);
    STRICT_EXPECTED_CALL(closedir(k_dir_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(pal_posix_read_dir_callback_mock(k_context_valid, er_disk_io, NULL, NULL))
        .SetReturn(er_disk_io);

    // act 
    result = pal_read_dir(k_dir_name_valid, pal_posix_read_dir_callback_mock, k_context_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_disk_io, result);
}

// 
// Test pal_read_dir unhappy path 
// 
TEST_FUNCTION(pal_posix_read_dir__neg_1)
{
    static const char* k_dir_name_valid = "valid/dir";
    static void* k_context_valid = (void*)0x2354;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(opendir(k_dir_name_valid))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(pal_os_last_error_as_prx_error())
        .SetReturn(er_not_found);

    // act 
    result = pal_read_dir(k_dir_name_valid, pal_posix_read_dir_callback_mock, k_context_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_not_found, result);
}

// 
// Test pal_read_dir unhappy path 
// 
TEST_FUNCTION(pal_posix_read_dir__neg_2)
{
    static const char* k_dir_name_valid = "valid/dir";
    static void* k_context_valid = (void*)0x2354;
    static DIR* k_dir_valid = (DIR*)0x23554;
    static struct dirent dp_valid;
    static const char* k_file_name_valid = "file";
    static struct stat stat_valid;
    int32_t result;

    dp_valid.d_name = (char*)k_file_name_valid;
    stat_valid.st_mode = 0;

    // arrange 
    STRICT_EXPECTED_CALL(opendir(k_dir_name_valid))
        .SetReturn(k_dir_valid)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(readdir(k_dir_valid))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(pal_os_last_error_as_prx_error())
        .SetReturn(er_permission);
    STRICT_EXPECTED_CALL(closedir(k_dir_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(pal_posix_read_dir_callback_mock(k_context_valid, er_permission, NULL, NULL))
        .SetReturn(er_permission);

    // act 
    result = pal_read_dir(k_dir_name_valid, pal_posix_read_dir_callback_mock, k_context_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_permission, result);
}

// 
// Test pal_read_dir unhappy path 
// 
TEST_FUNCTION(pal_posix_read_dir__neg_3)
{
    static const char* k_dir_name_valid = "valid/dir";
    static void* k_context_valid = (void*)0x2354;
    static DIR* k_dir_valid = (DIR*)0x23554;
    static struct dirent dp_valid;
    static const char* k_file_name_valid = "file";
    static struct stat stat_valid;
    int32_t result;

    dp_valid.d_name = (char*)k_file_name_valid;
    stat_valid.st_mode = 0;

    // arrange 
    STRICT_EXPECTED_CALL(opendir(k_dir_name_valid))
        .SetReturn(k_dir_valid)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(readdir(k_dir_valid))
        .SetReturn(&dp_valid)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(stat(k_file_name_valid, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_buf(&stat_valid, sizeof(stat_valid))
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(pal_os_last_error_as_prx_error())
        .SetReturn(er_permission);
    STRICT_EXPECTED_CALL(closedir(k_dir_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(pal_posix_read_dir_callback_mock(k_context_valid, er_permission, NULL, NULL))
        .SetReturn(er_not_found);

    // act 
    result = pal_read_dir(k_dir_name_valid, pal_posix_read_dir_callback_mock, k_context_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_not_found, result);
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

