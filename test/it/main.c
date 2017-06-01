// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "prx_log.h"

#include "tests.h"

#if defined(_WIN32) && !defined(EXE)
#include "windows.h"

//
// Dll Main entry point
//
BOOL __stdcall DllMain(
    HINSTANCE hinstDLL,
    unsigned long reason,
    void* reserved
)
{
    (void)reserved;
    (void)hinstDLL;
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
#endif

// 
// Runs individual tests
//
int tests(const char* arg, int argc, char *argv[])
{
#undef __decl_test
#define __decl_test(name, arg, usage) \
    if (!arg) \
        printf(" %15.15s  %s", #name, usage); \
    if (!strcmp(arg, #name)) { \
        return main_##name (argc, argv); \
    }
#include "tests.h"
#undef __decl_test
    tests(NULL, 0, NULL);  // Show help
    return -1;
}

//
// Integration test utility
//
int main(int argc, char *argv[])
{
    int32_t result;
    char* arg = NULL;
    mem_init();

    result = log_init();
    if (result != er_ok)
        return result;

    if (argc > 1)
    {
        arg = argv[1];
        argc--;
        argv++;
    }
    printf("=== proxy-it test harness ===\n\n");
    result = tests(arg, argc, argv);

    log_deinit();
    mem_deinit();
    return result;
}
