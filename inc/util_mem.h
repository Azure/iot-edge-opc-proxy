// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _util_mem_h_
#define _util_mem_h_

// #define LEAK_DETECT
// #define MEM_CHECK
// #define DBG_MEM

#if defined(DBG_MEM)
#define DEBUG 1
#endif

#if !defined(UNIT_TEST)
#if defined(LEAK_DETECT) || defined(MEM_CHECK)

#if defined(GB_DEBUG_ALLOC)
#include "azure_c_shared_utility/gballoc.h"

#define mem_alloc               gballoc_malloc
#define mem_realloc             gballoc_realloc
#define mem_free                gballoc_free
#define crt_alloc               gballoc_malloc
#define crt_realloc             gballoc_realloc
#define crt_free                gballoc_free

#define mem_zalloc(size)        gballoc_calloc(1, size) 
#define mem_zalloc_type(type)   gballoc_calloc(1, sizeof(type)) 
#define mem_free_type(type, p)  gballoc_free(p)

#define REFCOUNT_TYPE_FREE(type, p) free(p)

#elif defined(_MSC_VER)
//
// On windows, set up leak detection in CRT.  
//
#if !defined(_CRTDBG_MAP_ALLOC)
#define _CRTDBG_MAP_ALLOC 1
#endif // !_CRTDBG_MAP_ALLOC 

#include <crtdbg.h>

#if defined(LEAK_DETECT) && defined (MEM_CHECK)
#define _CRTDBG_FLAGS                         \
    _CRTDBG_ALLOC_MEM_DF |                    \
    _CRTDBG_LEAK_CHECK_DF |                   \
    _CRTDBG_CHECK_ALWAYS_DF                 
#elif defined(MEM_CHECK) 
#if defined(DEBUG)
#define _CRTDBG_FLAGS                         \
    _CRTDBG_ALLOC_MEM_DF |                    \
    _CRTDBG_DELAY_FREE_MEM_DF
#else
#define _CRTDBG_FLAGS                         \
    _CRTDBG_ALLOC_MEM_DF 
#endif
#else                                         
#define _CRTDBG_FLAGS                         \
    _CRTDBG_ALLOC_MEM_DF |                    \
    _CRTDBG_CHECK_CRT_DF |                    \
    _CRTDBG_LEAK_CHECK_DF |                   \
    _CRTDBG_CHECK_EVERY_1024_DF
#endif

#define mem_init() _CrtSetDbgFlag(_CRTDBG_FLAGS)

#define mem_alloc               malloc
#define mem_realloc             realloc
#define mem_free                free
#define crt_alloc               malloc
#define crt_realloc             realloc
#define crt_free                free

#if defined(MEM_CHECK)
#define mem_check()             _CrtCheckMemory()
#else
#define mem_check()             (void)0
#endif

#define mem_zalloc(size)        calloc(1, size) 

#define mem_zalloc_type(type) \
    (type*)calloc(1, sizeof(type))

#define mem_free_type(type, p)  \
    do { memset(p, 0, sizeof(type)); free(p); } while(0)

#define REFCOUNT_TYPE_FREE      mem_free_type

#else // !MSC_VER

#include <mcheck.h>
//
// export MALLOC_TRACE=<file> before running <executable> to 
// write trace file. Then Run mtrace <executable> <file> to 
// analyze any leaks.
//

#if defined(LEAK_DETECT) && defined (MEM_CHECK)
#define mem_init()              mcheck(NULL); mtrace()
#elif defined(MEM_CHECK)  
#if defined(DEBUG)
#define mem_init()              mcheck_pedantic(NULL)
#else
#define mem_init()              mcheck(NULL)
#endif
#else
#define mem_init()              mtrace()
#endif

#define mem_deinit()            /* muntrace() */
#define mem_alloc               malloc
#define mem_realloc             realloc
#define mem_free                free
#define crt_alloc               malloc
#define crt_realloc             realloc
#define crt_free                free

#if defined(MEM_CHECK) && !defined(DEBUG)
#define mem_check()             mcheck_check_all()
#else
#define mem_check()             (void)0
#endif

#define mem_zalloc(size)        calloc(1, size) 

#define mem_zalloc_type(type) \
    (type*)calloc(1, sizeof(type))

#define mem_free_type(type, p)  \
    do { memset(p, 0, sizeof(type)); free(p); } while(0)

#define REFCOUNT_TYPE_FREE      mem_free_type

#endif // !MSC_VER
#endif // LEAK_DETECT || MEM_CHECK
#endif // !UNIT_TEST

#ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
// Ensure no intrinsic memcmp is used to aid afl
inline int fuzzing_memcmp(
    char* ptr1,
    char* ptr2,
    int len
) __attribute__((always_inline));

inline int fuzzing_memcmp(
    char* ptr1,
    char* ptr2,
    int len
)
{
    while (len--) if (*(ptr1++) ^ *(ptr2++)) return 1;
    return 0;
}
#define memcmp fuzzing_memcmp
#endif // FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION

#include "util_dbg.h"
#include "api.h"

//
// Initialize memory
//
#if !defined(mem_init)
#define mem_init() (void)0
#endif

//
// Deinit memory
//
#if !defined(mem_deinit)
#define mem_deinit() (void)0
#endif

//
// Check memory
//
#if !defined(mem_check)
#define mem_check() (void)0
#endif

//
// Allocate memory for a type
//
#if !defined(mem_alloc)
#define mem_alloc(size) \
    h_realloc(size, NULL, false, __FILE__, sizeof(__FILE__)-1, __LINE__) 
#endif

//
// Re-allocate memory 
//
#if !defined(mem_realloc)
#define mem_realloc(p, size) \
    h_realloc(size, p, false, __FILE__, sizeof(__FILE__)-1, __LINE__) 
#endif

//
// Allocate zeroed out memory of a particular size
//
#if !defined(mem_zalloc)
#define mem_zalloc(size) \
    h_realloc(size, NULL, true, __FILE__, sizeof(__FILE__)-1, __LINE__) 
#endif

//
// Free memory
//
#if !defined(mem_free)
#define mem_free(p) \
    h_free(p, __FILE__, sizeof(__FILE__)-1, __LINE__) 
#endif

//
// Allocate a particular type
//
#if !defined(mem_zalloc_type)
#define mem_zalloc_type(type) \
    (type*)h_realloc(sizeof(type), NULL, true, __FILE__, sizeof(__FILE__)-1, __LINE__) 
#endif

//
// Free a type
//
#if !defined(mem_free_type)
#if defined(DEBUG) && !defined(UNIT_TEST)
#define mem_free_type(type, p) \
    do { \
        memset(p, 0, sizeof(type)); \
        h_free((type*)p, __FILE__, sizeof(__FILE__) - 1, __LINE__); \
    } while(0)
#else
#define mem_free_type(type, p) \
    h_free((type*)p, __FILE__, sizeof(__FILE__)-1, __LINE__) 
#endif
#endif

//
// Allocate memory using crt
//
#if !defined(crt_alloc)
#define crt_alloc(size) \
    c_realloc(size, NULL, __FILE__, sizeof(__FILE__)-1, __LINE__) 
#endif

//
// Re-allocate memory 
//
#if !defined(crt_realloc)
#define crt_realloc(p, size) \
    c_realloc(size, p,  __FILE__, sizeof(__FILE__)-1, __LINE__) 
#endif

//
// Free crt memory
//
#if !defined(crt_free)
#define crt_free(p) \
    c_free(p, __FILE__, sizeof(__FILE__)-1, __LINE__) 
#endif

//
// Free refcount type
//
#if !defined(REFCOUNT_TYPE_FREE)
#define REFCOUNT_TYPE_FREE(type, p) \
    c_free((type*)p, __FILE__, sizeof(__FILE__)-1, __LINE__) 
#endif

#if !defined(mem_cmp)
#define mem_cmp \
    memcmp
#endif

//
// Allocate crt memory
//
decl_internal_5(void*, c_realloc,
    size_t, size,
    void*, ptr,
    const char*, file,
    size_t, file_len,
    int32_t, line_number
);

//
// Free crt memory
//
decl_internal_4(void, c_free,
    void*, ptr,
    const char*, file,
    size_t, file_len,
    int32_t, line_number
);

//
// Allocate heap memory
//
decl_internal_6(void*, h_realloc,
    size_t, size,
    void*, ptr,
    bool, zero_mem,
    const char*, file,
    size_t, file_len,
    int32_t, line_number
);

//
// Free heap memory
//
decl_internal_4(void, h_free,
    void*, ptr,
    const char*, file,
    size_t, file_len,
    int32_t, line_number
);

#endif // _util_mem_h_