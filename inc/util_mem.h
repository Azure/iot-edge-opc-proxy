// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _util_mem_h_
#define _util_mem_h_

// #define LEAK_DETECT

//
// Initialize memory
//
#define mem_init() (void)0

//
// Allocate memory for a type
//
#define mem_alloc(size) \
    h_realloc(size, NULL, false, __FILE__, sizeof(__FILE__)-1, __LINE__) 

//
// Re-allocate memory 
//
#define mem_realloc(p, size) \
    h_realloc(size, p, false, __FILE__, sizeof(__FILE__)-1, __LINE__) 

//
// Allocate zeroed out memory of a particular size
//
#define mem_zalloc(size) \
    h_realloc(size, NULL, true, __FILE__, sizeof(__FILE__)-1, __LINE__) 

//
// Free memory
//
#define mem_free(p) \
    h_free(p, __FILE__, sizeof(__FILE__)-1, __LINE__) 

//
// Allocate a particular type
//
#define mem_zalloc_type(type) \
    (type*)h_realloc(sizeof(type), NULL, true, __FILE__, sizeof(__FILE__)-1, __LINE__) 

//
// Free a type
//
#define mem_free_type(type, p) \
    h_free((type*)p, __FILE__, sizeof(__FILE__)-1, __LINE__) 

//
// Allocate memory using crt
//
#define crt_alloc(size) \
    c_realloc(size, NULL, __FILE__, sizeof(__FILE__)-1, __LINE__) 

//
// Free crt memory
//
#define crt_free(p) \
    c_free(p, __FILE__, sizeof(__FILE__)-1, __LINE__) 

//
// Free refcount type
//
#define REFCOUNT_TYPE_FREE(type, p) \
    c_free((type*)p, __FILE__, sizeof(__FILE__)-1, __LINE__) 

//
// On windows, set up leak detection in CRT, unless running in unit test
//
#if defined(LEAK_DETECT) && !defined(UNIT_TEST)

#undef mem_init
#undef mem_alloc
#undef mem_realloc
#undef mem_free
#undef crt_alloc
#undef crt_free

#undef mem_zalloc
#undef mem_zalloc_type
#undef mem_free_type

#undef REFCOUNT_TYPE_FREE

#if defined(_MSC_VER)
#if !defined(_CRTDBG_MAP_ALLOC)
#define _CRTDBG_MAP_ALLOC 1
#endif // !_CRTDBG_MAP_ALLOC 

#include <crtdbg.h>
#define _CRTDBG_FLAGS \
    _CRTDBG_ALLOC_MEM_DF | \
  /*  _CRTDBG_CHECK_CRT_DF | */ \
    _CRTDBG_LEAK_CHECK_DF  | \
  /*  _CRTDBG_DELAY_FREE_MEM_DF | */ \
  /*  _CRTDBG_CHECK_EVERY_1024_DF |  */ \
    _CRTDBG_CHECK_ALWAYS_DF | \
    0
#define mem_init()              _CrtSetDbgFlag(_CRTDBG_FLAGS)
#define mem_alloc               malloc
#define mem_realloc             realloc
#define mem_free                free
#define crt_alloc               malloc
#define crt_free                free

#define mem_zalloc(size)        calloc(1, size) 
#define mem_zalloc_type(type)   (type*)calloc(1, sizeof(type)) 
#define mem_free_type(type, p)  free(p)

#define REFCOUNT_TYPE_FREE(type, p) free(p)

#else // MSC_VER

#if !defined(GB_DEBUG_ALLOC)
#define GB_DEBUG_ALLOC
#endif // !GB_DEBUG_ALLOC
#include "azure_c_shared_utility/gballoc.h"

#define mem_init()              (void)0
#define mem_alloc               gballoc_malloc
#define mem_realloc             gballoc_realloc
#define mem_free                gballoc_free
#define crt_alloc               gballoc_malloc
#define crt_free                gballoc_free

#define mem_zalloc(size)        gballoc_calloc(1, size) 
#define mem_zalloc_type(type)   gballoc_calloc(1, sizeof(type)) 
#define mem_free_type(type, p)  gballoc_free(p)

#define REFCOUNT_TYPE_FREE(type, p) free(p)

#endif // !MSC_VER
#endif // LEAK_DETECT && !UNIT_TEST

#include "util_dbg.h"
#include "api.h"

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