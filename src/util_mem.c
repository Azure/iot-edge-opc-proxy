// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"

//
// Allocate size memory using crt
//
void* c_realloc(
    size_t size,
    void* ptr,
    const char* file,
    size_t file_len,
    int32_t line_number
)
{
    void* result;
    (void)file;
    (void)file_len;
    (void)line_number;
    result = realloc(ptr, size);
    if (!result)
    {
        log_error(NULL, "c_realloc (%d) at %s:%d failed.",
            size, file, line_number);
    }
    return result;
}

//
// Free size memory with crt
//
void c_free(
    void* ptr,
    const char* file,
    size_t file_len,
    int32_t line_number
)
{
    (void)file;
    (void)file_len;
    (void)line_number;
    free(ptr);
}

//
// Allocate size memory from heap (TODO)
//
void* h_realloc(
    size_t size,
    void* ptr,
    bool zero_mem,
    const char* file,
    size_t file_len,
    int32_t line_number
)
{
    void* result;
    (void)file;
    (void)file_len;
    (void)line_number;
    result = realloc(ptr, size);
    if (!result)
    {
        log_error(NULL, "h_realloc (%d) at %s:%d failed.", 
            size, file, line_number);
        return NULL;
    }
        
    if (!zero_mem)
        return result;
    memset(result, 0, size);
    return result;
}

//
// Free memory on heap (TODO)
//
void h_free(
    void* ptr,
    const char* file,
    size_t file_len,
    int32_t line_number
)
{
    (void)file;
    (void)file_len;
    (void)line_number;
    free(ptr);
}
