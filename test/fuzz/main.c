// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "util_string.h"
#include "prx_log.h"
#include "pal.h"
#include "pal_file.h"

//
// Entry point for fuzz harness
//
extern int32_t fuzz(
    const char* option,
    const char* input_file,
    const char* out_file
);

// Optional option argument for fuzz entry point...
static const char* option = NULL;

//
// Folder callback for debug mode
//
int32_t main_for_each(
    void* context,
    int32_t error_code,
    const char* input_file,
    prx_file_info_t* stats
)
{
    int32_t result;
    if (error_code != er_ok)
        return error_code;
    if (!stats || !input_file)
        return er_fault;
    if (stats->type != prx_file_type_file)
        return er_ok;
    result = fuzz(option, input_file, (const char*)context);
    return er_ok;
}

//
// Harness for fuzz testing and result debugging
//
int32_t main(
    int32_t argc,
    char *argv[]
)
{
    int32_t result;
    bool debug = false;
    const char* input = NULL, *output = NULL;
    int32_t index;

    for (index = 1; index < argc; index++)
    {
        /**/ if (0 == string_compare(argv[index], "--debug"))
            debug = true;
        else if (!argv[index][0])
            return er_arg;
        else if (argv[index][0] != '-')
        {
            if (!input)
                input = argv[index];
            else if (!output)
                output = argv[index];
            else
                return er_arg;
        }
        else if (argv[index][1] && argv[index][1] != '-')
            option = &argv[index][1];
        else
            return er_arg;
    }

    if (!input)
        return er_arg;
    if (!output)
        output = "._out_tmp";

    log_init();
    result = pal_file_init();
    if (result == er_ok)
    {
        if (debug)
        {
            // Open each file one by one from input folder
            result = pal_read_dir(input, 
                main_for_each, (void*)output);
        }
        else
        {
#ifdef __AFL_HAVE_MANUAL_CONTROL
            __AFL_INIT();
            while (__AFL_LOOP(1000))
#endif
            {
                result = fuzz(option, input, output);
            }
        }
    }
    log_deinit();
    pal_file_deinit();
    return result;
}
