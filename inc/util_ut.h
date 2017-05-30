// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _util_ut_h_
#define _util_ut_h_

#if !defined(__STRICT_ANSI__)
#define __STRICT_ANSI__ 1
#endif
#if !defined(__STDC__)
#define __STDC__ 1
#endif

#ifdef __cplusplus
#include <cstdlib>
#include <cstring>
#else
#include <stdlib.h>
#include <string.h>
#endif

#if !defined(UNIT_UNDER_TEST)
    // Must define UNIT_UNDER_TEST to enable unit test functionality here...
#else

#include "macro_utils.h"

#define UNIT_TEST C2(UNIT_UNDER_TEST,_ut)

#include "testrunnerswitcher.h"

#if !defined(USE_CTEST)
#define UMOCK_STATIC static
#endif

#include "umock_c.h"
#include "umock_c_negative_tests.h"
#include "umocktypes_charptr.h"
#include "umocktypes_stdint.h"
#include "umocktypes_bool.h"

#define UNIT_H_ UNIT_UNDER_TEST.h
#define UNIT_H TOSTRING(UNIT_H_)

#define UNIT_C_ UNIT_UNDER_TEST.c
#define UNIT_C TOSTRING(UNIT_C_)

//
// Global mutex
//
static TEST_MUTEX_HANDLE test_serialize_mutex;

//
// Tracing
//
#if !defined(USE_CTEST)
#define TEST_TRACE(...) \
    do { \
        sprintf(UT_MEM, __VA_ARGS__); \
        Logger::WriteMessage(UT_MEM); \
    } while (0)
#else
#define TEST_TRACE(...) \
    printf(__VA_ARGS)
#endif

//
// Common error output
//
static void on_umock_c_error(
    UMOCK_C_ERROR_CODE error
)
{
    switch (error)
    {
    case UMOCK_C_ARG_INDEX_OUT_OF_RANGE:      ASSERT_FAIL("ARG_INDEX_OUT_OF_RANGE");
    case UMOCK_C_MALLOC_ERROR:                ASSERT_FAIL("MALLOC_ERROR");
    case UMOCK_C_INVALID_ARGUMENT_BUFFER:     ASSERT_FAIL("INVALID_ARGUMENT_BUFFER");
    case UMOCK_C_COMPARE_CALL_ERROR:          ASSERT_FAIL("COMPARE_CALL_ERROR");
    case UMOCK_C_RESET_CALLS_ERROR:           ASSERT_FAIL("RESET_CALLS_ERROR");
    case UMOCK_C_CAPTURE_RETURN_ALREADY_USED: ASSERT_FAIL("CAPTURE_RETURN_ALREADY_USED");
    case UMOCK_C_NULL_ARGUMENT:               ASSERT_FAIL("NULL_ARGUMENT");
    case UMOCK_C_INVALID_PAIRED_CALLS:        ASSERT_FAIL("INVALID_PAIRED_CALLS");
    case UMOCK_C_REGISTER_TYPE_FAILED:        ASSERT_FAIL("UMOCK_C_REGISTER_TYPE_FAILED");
    case UMOCK_C_ERROR:                       
        break; // ASSERT_FAIL("ERROR"); 
    }
}

static char UT_MEM[1024];
static void* UT_MEM_ALLOCED;
static uint64_t k_zero = 0;
static uint64_t k_one = 1;
static bool k_false = false;
static bool k_true = true;

//
// Boilerplate macro to set up test suite
//
#define BEGIN_DECLARE_TEST_SUITE() \
    BEGIN_TEST_SUITE(UNIT_TEST) \
    TEST_SUITE_INITIALIZE(suite_init) { \
        test_serialize_mutex = TEST_MUTEX_CREATE(); \
        ASSERT_IS_NOT_NULL(test_serialize_mutex); \
        umock_c_init(on_umock_c_error); \
        if (0 != umocktypes_stdint_register_types() || \
            0 != umocktypes_bool_register_types() || \
            0 != umocktypes_charptr_register_types()) { \
             ASSERT_FAIL("Could not register types."); \
        }
#define END_DECLARE_TEST_SUITE() \
    }
#define DECLARE_TEST_SUITE() \
    BEGIN_DECLARE_TEST_SUITE() \
    END_DECLARE_TEST_SUITE()

//
// Declare setup for each test
//
#define BEGIN_DECLARE_TEST_SETUP() \
    TEST_FUNCTION_INITIALIZE(method_init) { \
        if (TEST_MUTEX_ACQUIRE(test_serialize_mutex)) { \
            ASSERT_FAIL("Could not acquire test serialization mutex."); \
        } \
        memset(UT_MEM, 0xde, sizeof(UT_MEM)); \
        if (UT_MEM_ALLOCED) { \
            free(UT_MEM_ALLOCED); \
            UT_MEM_ALLOCED = NULL; \
        } \
        umock_c_reset_all_calls();
#define END_DECLARE_TEST_SETUP() \
    }
#define DECLARE_TEST_SETUP() \
    BEGIN_DECLARE_TEST_SETUP() \
    END_DECLARE_TEST_SETUP()

//
// One shot test declaration
//
#define BEGIN_DECLARE_TEST_START(name) \
    BEGIN_DECLARE_TEST_SUITE(name) \
    DECLARE_TEST_SETUP()

//
// End of all tests in suite
//
#define BEGIN_DECLARE_TEST_COMPLETE() \
    TEST_FUNCTION_CLEANUP(method_cleanup) { \
        umock_c_negative_tests_deinit(); \
        TEST_MUTEX_RELEASE(test_serialize_mutex);
#define END_DECLARE_TEST_COMPLETE() \
    } \
    TEST_SUITE_CLEANUP(suite_cleanup) { \
        if (UT_MEM_ALLOCED) { \
            free(UT_MEM_ALLOCED); \
        } \
        TEST_MUTEX_DESTROY(test_serialize_mutex); \
        umock_c_deinit(); \
    } \
    END_TEST_SUITE(UNIT_TEST)
#define DECLARE_TEST_COMPLETE() \
    BEGIN_DECLARE_TEST_COMPLETE() \
    END_DECLARE_TEST_COMPLETE()


//
// Helper macros
//
#define ASSERT_EXPECTED_CALLS() \
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls())

// 
// Negative tests arrange
//
#define UMOCK_C_NEGATIVE_TESTS_ARRANGE() \
    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init())

//
// negative tests act
//
#define UMOCK_C_NEGATIVE_TESTS_ACT() \
    umock_c_negative_tests_snapshot(); \
    for (size_t __i = 0; __i < umock_c_negative_tests_call_count(); __i++) { \
        umock_c_negative_tests_reset(); \
        umock_c_negative_tests_fail_call(__i)


#if !defined(min)
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#if !defined(_MSC_VER) || !defined(_countof)
#define _countof(a) (sizeof(a) / sizeof((a)[0]))
#endif

//
// Assert for each failed iteration result
//
#define UMOCK_C_NEGATIVE_TESTS_ASSERT(type, B, ...) \
        do { \
            type A[] =  { __VA_ARGS__ }; \
            char __msg[128]; \
            sprintf(__msg, "On failed call# %zu", __i+1); \
            ASSERT_ARE_EQUAL_WITH_MSG(type, A[min(__i, _countof(A)-1)], B, __msg); \
        } while(0); \
    }

// 
// Range tests arrange - from list
//
#define UMOCK_C_RANGE_TESTS_ARRANGE(type, I, ...) \
    do { type R[] =  { __VA_ARGS__ }; \
        for (size_t __j = 0, __i = 0; __j < _countof(R); __j++) { \
            type I = R[__j]; \
            umock_c_reset_all_calls()

//
// Range tests arrange - from bounds
//
#define UMOCK_C_RANGE_TESTS_ARRANGE_FROM_TO(type, I, a, b) \
    do { size_t __i = 0; \
        for (type I = a; I <= b; I++) { \
            umock_c_reset_all_calls()

//
// Range tests act - no op
//
#define UMOCK_C_RANGE_TESTS_ACT()

//
// Assert for each call in the range with a default compare
//
#define UMOCK_C_RANGE_TESTS_ASSERT(type, B, d, ...) \
            do { \
                type A[] =  { __VA_ARGS__ }; \
                type e = A[min(__i, _countof(A)-1)]; \
                char __msg[128]; \
                if ((type)d == (type)B) \
                    break; \
                sprintf(__msg, "On failed call# %zu", __i+1); \
                __i++; \
                ASSERT_ARE_EQUAL_WITH_MSG(type, e, B, __msg); \
            } while(0); \
        } \
    } while(0)


// ---- BEGIN Delete
#define PRINT(x) TEST_TRACE("%d, ", x)
#define PRINT_PI(x) TEST_TRACE("%s, ", prx_err_string(x))
#define UMOCK_C_RANGE_TESTS_PRINT(type, B, d, p) \
    do { if ((type)d == (type)B) break; p(B); __i++; } while(0); } } while(0)
// ---- END Delete

//
// Entry point
//
#if defined(USE_CTEST)

// Bring in global symbols as well
#define ENABLE_GLOBAL

int main(
#else // CPPTEST
__declspec(dllexport) int C2(UNIT_TEST, _init)(
#endif // !CTEST
    void
)
{
    size_t failed = 0;
    RUN_TEST_SUITE(UNIT_TEST, failed);
    return (int)failed;
}

//
// Remove logging in unit tests
//
#define log_t \
    void*
#define log_read_config(x) \
    er_not_supported
#define log_set_log_file(x) \
    er_not_supported
#define log_get(x) \
    NULL

#define log_error_b(log, b, len) (void)0
#define log_error(log, fmt, ...) (void)0
#define  log_info_b(log, b, len) (void)0
#define  log_info(log, fmt, ...) (void)0
#define log_trace_b(log, b, len) (void)0
#define log_trace(log, fmt, ...) (void)0
#define log_debug_b(log, b, len) (void)0
#define log_debug(log, fmt, ...) (void)0

#define dbg_assert_ptr(arg) (void)arg

//
// Simplify ref counting in unit tests - UT is always single threaded...
//
#undef WIN32
#undef REFCOUNT_USE_STD_ATOMIC
#define REFCOUNT_ATOMIC_DONTCARE
#include "azure_c_shared_utility/refcount.h"
#define REFCOUNT_TYPE_SIZE(t) \
    sizeof(t) + 8
#undef REFCOUNT_TYPE_CREATE
#define REFCOUNT_TYPE_CREATE(t) \
    (t*)crt_alloc(REFCOUNT_TYPE_SIZE(t))
#undef INC_REF
#define INC_REF(type, var) ++((((REFCOUNT_TYPE(type)*)var)->count))
#undef DEC_REF
#define DEC_REF(type, var) --((((REFCOUNT_TYPE(type)*)var)->count))

//
// Assert calls to encoder / decoder
//
#define STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_BEGIN(ctx, n) \
    STRICT_EXPECTED_CALL(io_encode_type_begin (IGNORED_PTR_ARG, n)) \
        .ValidateArgumentBuffer(1, ctx, sizeof(io_codec_ctx_t)) \
        .SetReturn(er_ok).SetFailReturn(er_writing)
#define STRICT_EXPECTED_CALL_TO_DECODE_TYPE_BEGIN(ctx, n) \
    STRICT_EXPECTED_CALL(io_decode_type_begin (IGNORED_PTR_ARG)) \
        .ValidateArgumentBuffer(1, ctx, sizeof(io_codec_ctx_t)) \
        .SetReturn(er_ok).SetFailReturn(er_invalid_format)
#define STRICT_EXPECTED_CALL_TO_ENCODE_VALUE(ctx, type, s, m) \
    STRICT_EXPECTED_CALL(io_encode_##type (IGNORED_PTR_ARG, #m, ( type##_t)(s)->m)) \
        .ValidateArgumentBuffer(1, ctx, sizeof(io_codec_ctx_t)) \
        .SetReturn(er_ok).SetFailReturn(er_writing)
#define STRICT_EXPECTED_CALL_TO_DECODE_VALUE(ctx, type, s, m) \
    STRICT_EXPECTED_CALL(io_decode_##type (IGNORED_PTR_ARG, #m, ( type##_t*)&(s)->m)) \
        .ValidateArgumentBuffer(1, ctx, sizeof(io_codec_ctx_t)) \
        .SetReturn(er_ok).SetFailReturn(er_out_of_memory)
#define STRICT_EXPECTED_CALL_TO_ENCODE_OBJECT(ctx, type, s, m) \
    STRICT_EXPECTED_CALL(io_encode_object(IGNORED_PTR_ARG, #m, false, IGNORED_PTR_ARG)) \
        .ValidateArgumentBuffer(1, ctx, sizeof(io_codec_ctx_t)) \
        .CopyOutArgumentBuffer_object(ctx, sizeof(io_codec_ctx_t)) \
        .SetReturn(er_ok).SetFailReturn(er_writing); \
    STRICT_EXPECTED_CALL(io_encode_##type (IGNORED_PTR_ARG, &(s)->m)) \
        .ValidateArgumentBuffer(1, ctx, sizeof(io_codec_ctx_t)) \
        .SetReturn(er_ok).SetFailReturn(er_writing)
#define STRICT_EXPECTED_CALL_TO_DECODE_OBJECT(ctx, type, s, m) \
    STRICT_EXPECTED_CALL(io_decode_object(IGNORED_PTR_ARG, #m, IGNORED_PTR_ARG, IGNORED_PTR_ARG)) \
        .ValidateArgumentBuffer(1, ctx, sizeof(io_codec_ctx_t)) \
        .CopyOutArgumentBuffer_is_null(&k_false, sizeof(k_false)) \
        .CopyOutArgumentBuffer_object(ctx, sizeof(io_codec_ctx_t)) \
        .SetReturn(er_ok).SetFailReturn(er_out_of_memory); \
    STRICT_EXPECTED_CALL(io_decode_##type (IGNORED_PTR_ARG, &(s)->m)) \
        .ValidateArgumentBuffer(1, ctx, sizeof(io_codec_ctx_t)) \
        .SetReturn(er_ok).SetFailReturn(er_out_of_memory)
#define STRICT_EXPECTED_CALL_TO_ENCODE_TYPE_END(ctx) \
    STRICT_EXPECTED_CALL(io_encode_type_end (IGNORED_PTR_ARG)) \
        .ValidateArgumentBuffer(1, ctx, sizeof(io_codec_ctx_t)) \
        .SetReturn(er_ok).SetFailReturn(er_writing)
#define STRICT_EXPECTED_CALL_TO_DECODE_TYPE_END(ctx) \
    STRICT_EXPECTED_CALL(io_decode_type_end (IGNORED_PTR_ARG)) \
        .ValidateArgumentBuffer(1, ctx, sizeof(io_codec_ctx_t)) \
        .SetReturn(er_ok).SetFailReturn(er_out_of_memory)


#define ENABLE_MOCKS
#include "umock_c_prod.h"
// ... followed by unit under test

#endif // UNIT_UNDER_TEST
#endif // _util_ut_h_