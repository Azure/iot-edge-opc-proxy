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
    int32_t result;
    char *cur, buf[4096];
    buf[0] = 0;
    cur = getcwd(buf, sizeof(buf)-1);
    if (cur)
        result = string_clone(cur, &working_dir);
    else
        result = er_ok;
    return result;
}

//
// Get Full path of a file name
//
const char* pal_create_full_path(
    const char* file_name
)
{
    char* path;
    size_t path_len;
    if (!file_name)
        return NULL;

    path_len = strlen(file_name) + 1;
    if (working_dir)
    {
        path_len += strlen(working_dir);
        path_len += strlen(k_sep);
    }

    path = (char*)mem_alloc(path_len);
    if (!path)
        return NULL;
    path[0] = 0;

    if (working_dir)
    {
        strcat(path, working_dir);
        strcat(path, k_sep);
    }
    strcat(path, file_name);
    return path;
}

//
// Set working dir
//
int32_t pal_set_working_dir(
    const char* dir
)
{
    if (!dir)
        return er_fault;
    if (working_dir)
        pal_free_path(working_dir);
    return string_clone(dir, &working_dir);
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
    if (working_dir)
    {
        pal_free_path(working_dir);
        working_dir = NULL;
    }
}
