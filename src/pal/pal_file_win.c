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
