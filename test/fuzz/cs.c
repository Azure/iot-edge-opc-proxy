// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "util_string.h"
#include "io_cs.h"

//
// Entry point for fuzz harness
//
int32_t fuzz(
    const char* option,
    const char* input_file,
    const char* out_file
)
{
    int32_t result;
    io_cs_t* cs;
    (void)out_file;
    /**/ if (0 == string_compare(option, "cs"))
        return er_arg;
    result = io_cs_create_from_raw_file(input_file, &cs);
    if (result == er_ok)
        io_cs_free(cs);
    return result;
}
