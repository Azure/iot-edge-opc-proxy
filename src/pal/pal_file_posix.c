// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os.h"
#include "util_mem.h"
#include "pal_file.h"
#include "pal_err.h"
#include "util_string.h"
#include "util_misc.h"

const char* k_sep = "/";
static char* working_dir = NULL;

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
    return -1 != access(file_name, F_OK);
}

//
// Get Full path of a file name
//
int32_t pal_get_real_path(
    const char* file_name, 
    const char** path
)
{
    int32_t result;
    const char* real;
    size_t path_max;
    int err;
    char *buf, *ptr;
    chk_arg_fault_return(file_name);
    chk_arg_fault_return(path);

    // Get max path, if this fails, then we see if realpath will malloc
    path_max = (size_t)pathconf(".", _PC_PATH_MAX);
    if ((long)path_max > 0)
        buf = (char*)crt_alloc(path_max + 1);
    else
    {
        path_max = 1024;
        buf = NULL;
    }

    do
    {
        // First get real path
        real = realpath(file_name, buf);
        if (real)
        {
            *path = real;
            buf = NULL;
            result = er_ok;
            break;
        }

#if defined(GNU_SOURCE)
        err = errno;
        if (buf && (err == ENOENT || err == EACCES))
        {
            *path = buf;
            buf = NULL;
            result = er_ok;
            break;
        }
#endif
        // If absolute path, return as is
        if (*file_name == '/')
        {
            if (!buf || strlen(file_name) > path_max)
            {
                ptr = (char*)crt_realloc(buf, strlen(file_name) + 1);
                if (!ptr)
                {
                    result = er_out_of_memory;
                    break;
                }
                buf = ptr;
            }
            strcpy(buf, file_name);
            *path = buf;
            buf = NULL;
            result = er_ok;
            break;
        }
        
        // Convert relative path to absolute using current working dir
        if (*file_name == '.')
        {
            file_name++;
            while (*file_name == '/')
                file_name++;
        }

        ptr = buf;
        while(true)
        {
            if (!ptr)
            {
                ptr = (char*)crt_realloc(buf, path_max + 1);
                if (!ptr)
                {
                    result = er_out_of_memory;
                    break;
                }
                buf = ptr;
            }
            ptr = NULL;
            real = getcwd(buf, path_max);
            if (!real)
            {
                err = errno;
                if (err != ERANGE)
                {
                    result = pal_os_to_prx_error(err);
                    break;
                }
                // Increase size of path up to 16 k
                path_max *= 2;
                if (path_max >= 0x4000)
                {
                    result = er_invalid_format;
                    break;
                }
            }
            // See if relative portion fits into the remainder of buffer
            else if (path_max < strlen(buf) + strlen(file_name) + 1)
                     path_max = strlen(buf) + strlen(file_name) + 1;
            else
            {
                strcat(buf, "/");
                strcat(buf, file_name);
                *path = buf; 
                buf = NULL;
                result = er_ok;
                break;
            }
        }
    }
    while(0);

    if (buf)
        crt_free(buf);
    return result;
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
    DIR *dir;
    struct dirent *dp;
    struct stat statbuf;
    prx_file_info_t palstat;

    chk_arg_fault_return(dir_name);
    chk_arg_fault_return(cb);

    if (!*dir_name)
        dir_name = "/";

    dir = opendir(dir_name);
    if (dir == NULL)
        return pal_os_last_error_as_prx_error();
    result = er_not_found;
    while (true) 
    {
        dp = readdir(dir);
        if (!dp)
        {
            result = pal_os_last_error_as_prx_error();
            break;
        }

        if (!dp->d_name[0] ||
            (dp->d_name[0] == '.' && !dp->d_name[1]) ||
            (dp->d_name[0] == '.' && dp->d_name[1] == '.' && !dp->d_name[2]))
        {
            // Skipping ".", ".." or empty paths...
            continue;
        }

        if (0 != stat(dp->d_name, &statbuf))
        {
            result = pal_os_last_error_as_prx_error();
            break;
        }

        memset(&palstat, 0, sizeof(prx_file_info_t));
        palstat.inode_number = (uint64_t)statbuf.st_ino;
        palstat.device_id    = (uint64_t)statbuf.st_dev;
        palstat.total_size   = (uint64_t)statbuf.st_size;
        palstat.last_atime   = (uint64_t)statbuf.st_atime;
        palstat.last_mtime   = (uint64_t)statbuf.st_mtime;

        /**/ if (S_ISDIR(statbuf.st_mode))
            palstat.type = prx_file_type_dir;
        else 
            palstat.type = prx_file_type_file;

        if (er_ok != cb(context, er_ok, dp->d_name, &palstat))
        {
            result = er_ok;
            break;
        }
    }
    (void)closedir(dir);
    if (result != er_ok)
        result = cb(context, result, NULL, NULL);
    if (result == er_nomore)
        result = er_ok;
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
    crt_free((char*)path);
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
