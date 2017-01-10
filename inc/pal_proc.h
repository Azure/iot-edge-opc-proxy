// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _pal_mt_h
#define _pal_mt_h

#include "common.h"

//
// Process - optional pal component, used by tests/tools
//
typedef struct process process_t;

//
// Spwans a process with specified command line
//
decl_public_4(int32_t, pal_process_spawn,
    const char*, image,
    int32_t, argc,
    const char**, argv,
    process_t**, created
);

//
// Kills a process, call yield to wait for exit
//
decl_public_1(void, pal_process_kill,
    process_t*, process
);

//
// Waits for a process to exit
//
decl_public_1(void, pal_process_yield,
    process_t*, proc
);


#endif // _pal_mt_h