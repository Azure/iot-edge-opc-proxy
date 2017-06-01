// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "util_string.h"
#include "prx_ns.h"

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
    prx_ns_t* ns;
    (void)out_file;

    /**/ if (0 == string_compare(option, "hub"))
        result = prx_ns_iot_hub_create(input_file, &ns);
    else if (0 == string_compare(option, "local"))
        result = prx_ns_generic_create(input_file, &ns);
    else
        return er_arg;
        
    if (result == er_ok)
        prx_ns_close(ns);
    return result;
}
