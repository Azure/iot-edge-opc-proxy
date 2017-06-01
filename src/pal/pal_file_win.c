// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os.h"
#include "util_mem.h"
#include "pal_file.h"
#include "pal_err.h"
#include "util_string.h"
#include "util_misc.h"

const char* k_sep = "\\";

//
// Enumerate all drives
//
static int32_t pal_read_drives(
    pal_dir_cb_t cb,
    void* context
)
{
    int32_t result;
    DWORD drives;
    prx_file_info_t palstat;
    char drive_path[4];

    dbg_assert_ptr(cb);

    drive_path[0] = 'C';
    drive_path[1] = ':';
    drive_path[2] = '\\';
    drive_path[3] = 0;

    memset(&palstat, 0, sizeof(prx_file_info_t));
    palstat.type = prx_file_type_dir;

    result = er_nomore;
    drives = GetLogicalDrives();
    for (char x = 'A'; drives && x <= 'Z'; x++)
    {
        if (drives & 1)
        {
            drive_path[0] = x;
            if (er_ok != cb(context, er_ok, drive_path, &palstat))
            {
                result = er_ok;
                break;
            }
        }
        drives >>= 1;
    }
    return result != er_ok ? cb(context, result, NULL, NULL) : er_ok;
}

//
// Initialize 
//
int32_t pal_file_init(
    void
)
{
    return er_ok;
}

//
// Test whether file exists
//
bool pal_file_exists(
    const char* file_name
)
{
    DWORD attributes = GetFileAttributesA(file_name);
    return 
         (attributes != INVALID_FILE_ATTRIBUTES &&
        !(attributes & FILE_ATTRIBUTE_DIRECTORY));
}

//
// Get Full path of a file name
//
int32_t pal_get_real_path(
    const char* file_name,
    const char** path
) 
{
    DWORD copied;
    char *buf = NULL;
    chk_arg_fault_return(file_name);
    chk_arg_fault_return(path);

    buf = (char*)mem_alloc(MAX_PATH);
    if (!buf)
        return er_out_of_memory;

    copied = GetFullPathNameA(file_name, MAX_PATH, buf, NULL);
    if (!copied || copied > MAX_PATH)
    {
        mem_free(buf);
        return copied ? er_fatal : pal_os_last_error_as_prx_error();
    }
    *path = buf;
    return er_ok;
}

//
// Traverse a folder and call callback for each file
//
int32_t pal_read_dir(
    const char* dir_name,
    pal_dir_cb_t cb,
    void* context
)
{
    int32_t result;
    HANDLE h_find;
    WIN32_FIND_DATAA data;
    ULARGE_INTEGER ull;
    STRING_HANDLE root, file = NULL;
    prx_file_info_t palstat;

    chk_arg_fault_return(dir_name);
    chk_arg_fault_return(cb);

    // Handle drive letter enumeration
    if (!*dir_name || *dir_name == '/')
        return pal_read_drives(cb, context);

    root = STRING_construct_n(dir_name, 
        string_trim_back_len(dir_name, strlen(dir_name), k_sep));
    if (!root)
        return er_out_of_memory;
    do
    {
        result = er_out_of_memory;
        if (0 != STRING_concat(root, k_sep))
            break;
        file = STRING_clone(root);
        if (!file || 0 != STRING_concat(file, "*"))
            break;

        h_find = FindFirstFileA(STRING_c_str(file), &data);
        if (h_find == INVALID_HANDLE_VALUE)
        {
            result = cb(context, pal_os_last_error_as_prx_error(), NULL, NULL);
            if (result == er_nomore)
                result = er_ok;
            break;
        }

        result = er_not_found;
        while(true)
        {
            if (!data.cFileName[0] || 
                (data.cFileName[0] == '.' && !data.cFileName[1]) ||
                (data.cFileName[0] == '.' && data.cFileName[1] == '.' && !data.cFileName[2]))
            {
                // Skipping ".", ".." or empty path...
            }
            else
            {
                STRING_delete(file);
                file = STRING_clone(root);
                if (!file || 0 != STRING_concat(file, data.cFileName))
                {
                    result = er_out_of_memory;
                    break;
                }

                memset(&palstat, 0, sizeof(prx_file_info_t));

                ull.u.HighPart = data.nFileSizeHigh;
                ull.u.LowPart = data.nFileSizeLow;
                palstat.total_size = ull.QuadPart;

                ull.u.LowPart = data.ftLastAccessTime.dwLowDateTime;
                ull.u.HighPart = data.ftLastAccessTime.dwHighDateTime;
                palstat.last_atime = ull.QuadPart / 10000000ULL - 11644473600ULL;

                ull.u.LowPart = data.ftLastWriteTime.dwLowDateTime;
                ull.u.HighPart = data.ftLastWriteTime.dwHighDateTime;
                palstat.last_mtime = ull.QuadPart / 10000000ULL - 11644473600ULL;

                /**/ if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    palstat.type = prx_file_type_dir;
                else
                    palstat.type = prx_file_type_file;

                if (er_ok != cb(context, er_ok, STRING_c_str(file), &palstat))
                {
                    result = er_ok;
                    break;
                }
            }

            if (!FindNextFileA(h_find, &data))
            {
                result = pal_os_last_error_as_prx_error();
                break;
            }
        }
        (void)FindClose(h_find);
        if (result != er_ok)
            result = cb(context, result, NULL, NULL);
        if (result == er_nomore)
            result = er_ok;
        break;
    } 
    while (0);

    if (file)
        STRING_delete(file);
    STRING_delete(root);
    return result;
}

//
// Free path
//
void pal_free_path(
    const char* path
)
{
    if (!path)
        return;
    mem_free((char*)path);
}

//
// Deinit
//
void pal_file_deinit(
    void
)
{
    // no-op
}
