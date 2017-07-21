// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.


//
// To avoid pulling in azure-c-shared into public headers, we define a subset of macros to support
// function declarations that can be turned into mocks. #define ENABLE_MOCKS to generate mocks,
// and UNIT_HEADER around
//

#undef decl_g
#undef decl_f_0
#undef decl_f_1
#undef decl_f_2
#undef decl_f_3
#undef decl_f_4
#undef decl_f_5
#undef decl_f_6
#undef decl_f_7
#undef decl_f_8

#undef _attr_
#if defined(_WIN32)
#define _attr_ __cdecl
#else
#define _attr_
#endif

#undef _ext__
#ifdef __cplusplus
#define _ext__ extern "C"
#else
#define _ext__ extern
#endif

#undef _inl__
#if defined(_WIN32)
#define _inl__ static __inline
#else
#define _inl__ static inline
#endif

#if defined(ENABLE_GLOBAL)
#define decl_g(t, name, ...) _ext__ t name; t name = __VA_ARGS__
#else
#define decl_g(t, name, ...) _ext__ t name
#endif

#if defined(ENABLE_MOCKS) || defined(DECL_F_CUSTOM)

#if defined(ENABLE_MOCKS)
//
// Declare mocks using umock-c
//
#include "umock_c.h"

#define decl_f(s, r, m, name, ...) \
    MOCKABLE_FUNCTION_UMOCK_INTERNAL_WITH_MOCK(m, r, name, __VA_ARGS__)


//
// Inline - mockable - api
//
#define UMOCK_C_ARG_IN_SIGNATURE(count, arg_type, arg_name) \
    arg_type arg_name IFCOMMA(count)

#define decl_inline(r, name, ...) \
    MOCKABLE_FUNCTION_UMOCK_INTERNAL_WITH_MOCK(_attr_, r, name, __VA_ARGS__); \
    _inl__ r _attr_ C2(name, __inline_func)(IF(COUNT_ARG(__VA_ARGS__), , void) \
        FOR_EACH_2_COUNTED(UMOCK_C_ARG_IN_SIGNATURE, __VA_ARGS__))

#undef decl_inline_0
#undef decl_inline_1
#undef decl_inline_2
#undef decl_inline_3
#undef decl_inline_4
#undef decl_inline_5
#undef decl_inline_6
#undef decl_inline_7
#undef decl_inline_8

#define decl_inline_0 decl_inline
#define decl_inline_1 decl_inline
#define decl_inline_2 decl_inline
#define decl_inline_3 decl_inline
#define decl_inline_4 decl_inline
#define decl_inline_5 decl_inline
#define decl_inline_6 decl_inline
#define decl_inline_7 decl_inline
#define decl_inline_8 decl_inline

#else // DECL_F_CUSTOM
//
// Use custom mock macro
//
#define decl_f(s, r, m, name, ...) \
    DECL_F_CUSTOM(s, r, m, name, __VA_ARGS__)
#endif

#define decl_f_0 decl_f
#define decl_f_1 decl_f
#define decl_f_2 decl_f
#define decl_f_3 decl_f
#define decl_f_4 decl_f
#define decl_f_5 decl_f
#define decl_f_6 decl_f
#define decl_f_7 decl_f
#define decl_f_8 decl_f

#else // !defined(ENABLE_MOCKS) && !defined(DECL_F_CUSTOM)

#if defined(API_EXPORT) && !defined(UNIT_HEADER)
#if defined(_MSC_VER)
#undef _ext__
#define _ext__ extern __declspec(dllexport)
#elif defined(_GCC)
#undef _attr_
#define _attr_ __attribute__((visibility("default")))
#else
//...
#endif
#endif // API_EXPORT && !UNIT_HEADER

//
// Declare functions - scope, return type, attribute, name, plus arguments (up to 8)
//

#define decl_f_0(s, r, m, name) \
                                                                                  s r m name(void)
#define decl_f_1(s, r, m, name, t1, a1) \
                                                                                 s r m name(t1 a1)
#define decl_f_2(s, r, m, name, t1, a1, t2, a2) \
                                                                          s r m name(t1 a1, t2 a2)
#define decl_f_3(s, r, m, name, t1, a1, t2, a2, t3, a3) \
                                                                   s r m name(t1 a1, t2 a2, t3 a3)
#define decl_f_4(s, r, m, name, t1, a1, t2, a2, t3, a3, t4, a4) \
                                                            s r m name(t1 a1, t2 a2, t3 a3, t4 a4)
#define decl_f_5(s, r, m, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5) \
                                                     s r m name(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5)
#define decl_f_6(s, r, m, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6) \
                                              s r m name(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6)
#define decl_f_7(s, r, m, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6, t7, a7) \
                                       s r m name(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6, t7 a7)
#define decl_f_8(s, r, m, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6, t7, a7, t8, a8) \
                                s r m name(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6, t7 a7, t8 a8)

#endif // !defined(ENABLE_MOCKS) && !defined(DECL_F_CUSTOM)

#if !defined(ENABLE_MOCKS)

//
// Inline api
//
#undef decl_inline_0
#undef decl_inline_1
#undef decl_inline_2
#undef decl_inline_3
#undef decl_inline_4
#undef decl_inline_5
#undef decl_inline_6
#undef decl_inline_7
#undef decl_inline_8

#define decl_inline_8(r, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6, t7, a7, t8, a8) \
decl_f_8(_inl__, r, _attr_, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6, t7, a7, t8, a8)
#define decl_inline_7(r, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6, t7, a7) \
        decl_f_7(_inl__, r, _attr_, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6, t7, a7)
#define decl_inline_6(r, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6) \
                decl_f_6(_inl__, r, _attr_, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6)
#define decl_inline_5(r, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5) \
                        decl_f_5(_inl__, r, _attr_, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5)
#define decl_inline_4(r, name, t1, a1, t2, a2, t3, a3, t4, a4) \
                                decl_f_4(_inl__, r, _attr_, name, t1, a1, t2, a2, t3, a3, t4, a4)
#define decl_inline_3(r, name, t1, a1, t2, a2, t3, a3) \
                                        decl_f_3(_inl__, r, _attr_, name, t1, a1, t2, a2, t3, a3)
#define decl_inline_2(r, name, t1, a1, t2, a2) \
                                                decl_f_2(_inl__, r, _attr_, name, t1, a1, t2, a2)
#define decl_inline_1(r, name, t1, a1) \
                                                        decl_f_1(_inl__, r, _attr_, name, t1, a1)
#define decl_inline_0(r, name, v) \
                                                                decl_f_0(_inl__, r, _attr_, name)

#endif // !defined(ENABLE_MOCKS)

//
// Public api, exported by the dll
//
#undef decl_public_0
#undef decl_public_1
#undef decl_public_2
#undef decl_public_3
#undef decl_public_4
#undef decl_public_5
#undef decl_public_6
#undef decl_public_7
#undef decl_public_8

#define decl_public_8(r, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6, t7, a7, t8, a8) \
decl_f_8(_ext__, r, _attr_, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6, t7, a7, t8, a8)
#define decl_public_7(r, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6, t7, a7) \
        decl_f_7(_ext__, r, _attr_, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6, t7, a7)
#define decl_public_6(r, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6) \
                decl_f_6(_ext__, r, _attr_, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6)
#define decl_public_5(r, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5) \
                        decl_f_5(_ext__, r, _attr_, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5)
#define decl_public_4(r, name, t1, a1, t2, a2, t3, a3, t4, a4) \
                                decl_f_4(_ext__, r, _attr_, name, t1, a1, t2, a2, t3, a3, t4, a4)
#define decl_public_3(r, name, t1, a1, t2, a2, t3, a3) \
                                        decl_f_3(_ext__, r, _attr_, name, t1, a1, t2, a2, t3, a3)
#define decl_public_2(r, name, t1, a1, t2, a2) \
                                                decl_f_2(_ext__, r, _attr_, name, t1, a1, t2, a2)
#define decl_public_1(r, name, t1, a1) \
                                                        decl_f_1(_ext__, r, _attr_, name, t1, a1)
#define decl_public_0(r, name, v) \
                                                                decl_f_0(_ext__, r, _attr_, name)

//
// Internal api, not exported but used between compilation units
//
#undef decl_internal_0
#undef decl_internal_1
#undef decl_internal_2
#undef decl_internal_3
#undef decl_internal_4
#undef decl_internal_5
#undef decl_internal_6
#undef decl_internal_7
#undef decl_internal_8

#define decl_internal_0(r, name, v) \
                                                                       decl_f_0(extern, r, , name)
#define decl_internal_1(r, name, t1, a1) \
                                                               decl_f_1(extern, r, , name, t1, a1)
#define decl_internal_2(r, name, t1, a1, t2, a2) \
                                                       decl_f_2(extern, r, , name, t1, a1, t2, a2)
#define decl_internal_3(r, name, t1, a1, t2, a2, t3, a3) \
                                               decl_f_3(extern, r, , name, t1, a1, t2, a2, t3, a3)
#define decl_internal_4(r, name, t1, a1, t2, a2, t3, a3, t4, a4) \
                                       decl_f_4(extern, r, , name, t1, a1, t2, a2, t3, a3, t4, a4)
#define decl_internal_5(r, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5) \
                               decl_f_5(extern, r, , name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5)
#define decl_internal_6(r, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6) \
                       decl_f_6(extern, r, , name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6)
#define decl_internal_7(r, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6, t7, a7) \
               decl_f_7(extern, r, , name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6, t7, a7)
#define decl_internal_8(r, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6, t7, a7, t8, a8) \
       decl_f_8(extern, r, , name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6, t7, a7, t8, a8)

