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
    const char* real;
    long path_max;
    char *buf = NULL;
    if (!file_name || !path)
        return er_fault;

    //
    // Get max path, if this fails, then it is 
    // assumed that realpath will malloc
    //
    path_max = pathconf(".", _PC_PATH_MAX);
    if (path_max > 0)
        buf = (char*)crt_alloc(path_max + 1);

    // Get real path
    real = realpath(file_name, buf);
    if (!real)
    {
        if (buf)
            crt_free(buf);
        return pal_os_last_error_as_prx_error();
    }
    *path = real;
    return er_ok;
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
