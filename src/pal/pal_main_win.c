// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os.h"
#include "util_mem.h"

//
// Dll Main entry point
//
BOOL __stdcall DllMain(
    HINSTANCE hinstDLL,
    unsigned long reason,
    void* reserved
)
{
    (void)hinstDLL;
    (void)reserved;
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        mem_init();
        break;
    case DLL_PROCESS_DETACH:
        mem_deinit();
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}


