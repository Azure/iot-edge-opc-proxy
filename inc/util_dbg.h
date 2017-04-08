// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _util_dbg_h_
#define _util_dbg_h_

#if defined(DEBUG) && !defined(_DEBUG)
#define _DEBUG 1
#elif defined(_DEBUG) && !defined(DEBUG)
#define DEBUG 1
#endif

//
// Static assert in C 99
//
#ifndef compile_assert
#define __expand_2(e, l) e##l
#define __expand_1(l) __expand_2(__assert_at_line_, l)
#define compile_assert(cond) enum { __expand_1(__LINE__) = 1 / (!!(cond)) }
#endif

#if !defined(dbg_brk)
#if !defined(_WIN32) 
#if defined(__GNUC__) && defined(DEBUG)
#define dbg_brk __builtin_trap
#else
#define dbg_brk() 
#endif // !__GNUC__
#else // WIN32
#if defined(DEBUG)
#ifdef __cplusplus
extern "C" 
{
#endif // __cplusplus
    __declspec(dllimport) void __stdcall DebugBreak(
        void
    );
#ifdef __cplusplus
}
#endif // __cplusplus
#define dbg_brk DebugBreak
#else // !DEBUG
#define dbg_brk() 
#endif // DEBUG
#endif // !WIN32
#endif // !dbg_brk

#include "util_log.h"

#define __analysis_suppress(w)
#ifndef dbg_assert
#if !defined(UNIT_TEST)
#if defined(_MSC_VER)
#undef __analysis_suppress
#define __analysis_suppress(w) __pragma(warning(suppress: w))
#define dbg_assert(cond, fmt, ...) \
    do { __analysis_assume(cond); \
    if (!(cond)) { log_error(NULL, "[!!! ASSERT !!!] " fmt, __VA_ARGS__); dbg_brk(); } \
    } while(0)
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#define dbg_assert(cond, fmt, ...) \
    if (!(cond)) { log_error(NULL, "[!!! ASSERT !!!] " fmt, ##__VA_ARGS__); dbg_brk(); }
#else
#define dbg_assert(cond, fmt, args...) \
    if (!(cond)) { log_error(NULL, "[!!! ASSERT !!!] " fmt, ## args); dbg_brk(); }
#endif
#else
#define dbg_assert(...)
#endif
#endif // dbg_assert

#ifndef dbg_assert_ptr
#define dbg_assert_ptr(arg) dbg_assert(arg, "Pointer null") 
#endif // dbg_assert_ptr

#ifndef chk_arg_fault_return
#define chk_arg_fault_return(p) \
    if (!(p)) { log_error(NULL, "arg " #p " is null - return er_fault..."); return er_fault; }
#endif // chk_arg_ptr_fail_er_fault

#endif // _util_dbg_h_
