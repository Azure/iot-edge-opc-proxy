
#include "util_mem.h"

#if _WIN32
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
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}
#endif