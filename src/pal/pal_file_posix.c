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
    char *buf = NULL;
    if (!file_name || !path)
        return er_fault;

    //
    // Get max path, if this fails, then it is 
    // assumed that realpath will malloc
    //
    path_max = (size_t)pathconf(".", _PC_PATH_MAX);
    if ((long)path_max <= 0)
        path_max = 4096;

    buf = (char*)crt_alloc(path_max + 1);
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

        err = errno;
        result = pal_os_to_prx_error(err);
        if (!buf)
            break;

        *path = buf;
#if defined(GNU_SOURCE)
        if (err == ENOENT || err == EACCES)
        {
            buf = NULL;
            result = er_ok;
            break;
        }
#endif
        if (*file_name == '/')
        {
            strcpy(buf, file_name);
            buf = NULL;
            result = er_ok;
            break;
        }
        
        // Relative path to absolute
        if (*file_name == '.')
        {
            file_name++;
            while (*file_name == '/')
                file_name++;
        }

        real = getcwd(buf, path_max);
        if (!real)
        {
            result = pal_os_last_error_as_prx_error();
            break;
        }

        path_max -= strlen(buf);
        if (path_max < strlen(file_name) + 1)
        {
            result = er_fault;
            break;
        }
        strcat(buf, "/");
        strcat(buf, file_name);

        buf = NULL;
        result = er_ok;
        break;
    }
    while(0);

    if (buf)
        crt_free(buf);
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
