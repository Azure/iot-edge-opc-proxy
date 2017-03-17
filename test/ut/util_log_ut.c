// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST util_log
#include "util_ut.h"

//
// 1. Required mocks
//
#include "util_misc.h"

//
// 2. Include unit under test
//
#undef ENABLE_MOCKS
#include UNIT_H
#define ENABLE_MOCKS
#include UNIT_C

//
// 3. Setup test suite
//
BEGIN_DECLARE_TEST_SUITE()
END_DECLARE_TEST_SUITE()
// -or- DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
DECLARE_TEST_SETUP()

#if 0

// 
// Test zlog_initialize happy path 
// 
TEST_FUNCTION(zlog_initialize__success)
{

    // arrange 
    // ... 

    // act 
    result = zlog_initialize();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test zlog_initialize unhappy path 
// 
TEST_FUNCTION(zlog_initialize__neg)
{

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = zlog_initialize();

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, 0, result);

}

;
// line37 "util_zlog.h"




// 
// Test zlog_register happy path 
// 
TEST_FUNCTION(zlog_register__success)
{
    static const char* k_target_valid;
    const zlog_cb_t k_callback_valid;

    // arrange 
    // ... 

    // act 
    result = zlog_register(k_target_valid, k_callback_valid);

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test zlog_register passing as target argument an invalid const char* value 
// 
TEST_FUNCTION(zlog_register__arg_target_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = zlog_register();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test zlog_register passing as callback argument an invalid zlog_cb_t value 
// 
TEST_FUNCTION(zlog_register__arg_callback_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = zlog_register();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test zlog_register unhappy path 
// 
TEST_FUNCTION(zlog_register__neg)
{
    static const char* k_target_valid;
    const zlog_cb_t k_callback_valid;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = zlog_register(k_target_valid, k_callback_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, 0, result);

}

;
// line45 "util_zlog.h"




// 
// Test zlog_read_config happy path 
// 
TEST_FUNCTION(zlog_read_config__success)
{
    static const char* k_file_name_valid;

    // arrange 
    // ... 

    // act 
    result = zlog_read_config(k_file_name_valid);

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test zlog_read_config passing as file_name argument an invalid const char* value 
// 
TEST_FUNCTION(zlog_read_config__arg_file_name_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = zlog_read_config();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test zlog_read_config unhappy path 
// 
TEST_FUNCTION(zlog_read_config__neg)
{
    static const char* k_file_name_valid;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = zlog_read_config(k_file_name_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, 0, result);

}

;
// line52 "util_zlog.h"




// 
// Test zlog_get happy path 
// 
TEST_FUNCTION(zlog_get__success)
{
    static const char* k_area_valid;

    // arrange 
    // ... 

    // act 
    result = zlog_get(k_area_valid);

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test zlog_get passing as area argument an invalid const char* value 
// 
TEST_FUNCTION(zlog_get__arg_area_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = zlog_get();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test zlog_get unhappy path 
// 
TEST_FUNCTION(zlog_get__neg)
{
    static const char* k_area_valid;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = zlog_get(k_area_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, 0, result);

}

;
// line59 "util_zlog.h"




// 
// Test zlog_unregister happy path 
// 
TEST_FUNCTION(zlog_unregister__success)
{
    static const char* k_target_valid;

    // arrange 
    // ... 

    // act 
    result = zlog_unregister(k_target_valid);

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test zlog_unregister passing as target argument an invalid const char* value 
// 
TEST_FUNCTION(zlog_unregister__arg_target_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = zlog_unregister();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test zlog_unregister unhappy path 
// 
TEST_FUNCTION(zlog_unregister__neg)
{
    static const char* k_target_valid;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = zlog_unregister(k_target_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, 0, result);

}

;
// line66 "util_zlog.h"




// 
// Test zlog_deinit happy path 
// 
TEST_FUNCTION(zlog_deinit__success)
{

    // arrange 
    // ... 

    // act 
    result = zlog_deinit();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test zlog_deinit unhappy path 
// 
TEST_FUNCTION(zlog_deinit__neg)
{

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = zlog_deinit();

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, 0, result);

}

;
// line73 "util_zlog.h"






// 
// Test __zlog_debug_v happy path 
// 
TEST_FUNCTION(__zlog_debug_v__success)
{
    const zlog_t k_log_valid;
    static const char* k_file_valid;
    const size_t k_filelen_valid;
    static const char* k_func_valid;
    const size_t k_funclen_valid;
    const long k_line_valid;
    static const char* k_format_valid;
    const va_list k_args_valid;

    // arrange 
    // ... 

    // act 
    result = __zlog_debug_v(k_log_valid, k_file_valid, k_filelen_valid, k_func_valid, k_funclen_valid, k_line_valid, k_format_valid, k_args_valid);

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_debug_v passing as log argument an invalid zlog_t value 
// 
TEST_FUNCTION(__zlog_debug_v__arg_log_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_debug_v();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_debug_v passing as file argument an invalid const char* value 
// 
TEST_FUNCTION(__zlog_debug_v__arg_file_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_debug_v();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_debug_v passing as filelen argument an invalid size_t value 
// 
TEST_FUNCTION(__zlog_debug_v__arg_filelen_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_debug_v();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_debug_v passing as func argument an invalid const char* value 
// 
TEST_FUNCTION(__zlog_debug_v__arg_func_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_debug_v();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_debug_v passing as funclen argument an invalid size_t value 
// 
TEST_FUNCTION(__zlog_debug_v__arg_funclen_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_debug_v();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_debug_v passing as line argument an invalid long value 
// 
TEST_FUNCTION(__zlog_debug_v__arg_line_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_debug_v();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_debug_v passing as format argument an invalid const char* value 
// 
TEST_FUNCTION(__zlog_debug_v__arg_format_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_debug_v();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_debug_v passing as args argument an invalid va_list value 
// 
TEST_FUNCTION(__zlog_debug_v__arg_args_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_debug_v();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_debug_v unhappy path 
// 
TEST_FUNCTION(__zlog_debug_v__neg)
{
    const zlog_t k_log_valid;
    static const char* k_file_valid;
    const size_t k_filelen_valid;
    static const char* k_func_valid;
    const size_t k_funclen_valid;
    const long k_line_valid;
    static const char* k_format_valid;
    const va_list k_args_valid;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = __zlog_debug_v(k_log_valid, k_file_valid, k_filelen_valid, k_func_valid, k_funclen_valid, k_line_valid, k_format_valid, k_args_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, 0, result);

}

;
// line89 "util_zlog.h"




// 
// Test __zlog_info_v happy path 
// 
TEST_FUNCTION(__zlog_info_v__success)
{
    const zlog_t k_log_valid;
    static const char* k_file_valid;
    const size_t k_filelen_valid;
    static const char* k_func_valid;
    const size_t k_funclen_valid;
    const long k_line_valid;
    static const char* k_format_valid;
    const va_list k_args_valid;

    // arrange 
    // ... 

    // act 
    result = __zlog_info_v(k_log_valid, k_file_valid, k_filelen_valid, k_func_valid, k_funclen_valid, k_line_valid, k_format_valid, k_args_valid);

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_info_v passing as log argument an invalid zlog_t value 
// 
TEST_FUNCTION(__zlog_info_v__arg_log_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_info_v();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_info_v passing as file argument an invalid const char* value 
// 
TEST_FUNCTION(__zlog_info_v__arg_file_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_info_v();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_info_v passing as filelen argument an invalid size_t value 
// 
TEST_FUNCTION(__zlog_info_v__arg_filelen_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_info_v();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_info_v passing as func argument an invalid const char* value 
// 
TEST_FUNCTION(__zlog_info_v__arg_func_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_info_v();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_info_v passing as funclen argument an invalid size_t value 
// 
TEST_FUNCTION(__zlog_info_v__arg_funclen_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_info_v();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_info_v passing as line argument an invalid long value 
// 
TEST_FUNCTION(__zlog_info_v__arg_line_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_info_v();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_info_v passing as format argument an invalid const char* value 
// 
TEST_FUNCTION(__zlog_info_v__arg_format_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_info_v();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_info_v passing as args argument an invalid va_list value 
// 
TEST_FUNCTION(__zlog_info_v__arg_args_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_info_v();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_info_v unhappy path 
// 
TEST_FUNCTION(__zlog_info_v__neg)
{
    const zlog_t k_log_valid;
    static const char* k_file_valid;
    const size_t k_filelen_valid;
    static const char* k_func_valid;
    const size_t k_funclen_valid;
    const long k_line_valid;
    static const char* k_format_valid;
    const va_list k_args_valid;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = __zlog_info_v(k_log_valid, k_file_valid, k_filelen_valid, k_func_valid, k_funclen_valid, k_line_valid, k_format_valid, k_args_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, 0, result);

}

;
// line103 "util_zlog.h"




// 
// Test __zlog_error_v happy path 
// 
TEST_FUNCTION(__zlog_error_v__success)
{
    const zlog_t k_log_valid;
    static const char* k_file_valid;
    const size_t k_filelen_valid;
    static const char* k_func_valid;
    const size_t k_funclen_valid;
    const long k_line_valid;
    static const char* k_format_valid;
    const va_list k_args_valid;

    // arrange 
    // ... 

    // act 
    result = __zlog_error_v(k_log_valid, k_file_valid, k_filelen_valid, k_func_valid, k_funclen_valid, k_line_valid, k_format_valid, k_args_valid);

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_error_v passing as log argument an invalid zlog_t value 
// 
TEST_FUNCTION(__zlog_error_v__arg_log_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_error_v();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_error_v passing as file argument an invalid const char* value 
// 
TEST_FUNCTION(__zlog_error_v__arg_file_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_error_v();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_error_v passing as filelen argument an invalid size_t value 
// 
TEST_FUNCTION(__zlog_error_v__arg_filelen_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_error_v();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_error_v passing as func argument an invalid const char* value 
// 
TEST_FUNCTION(__zlog_error_v__arg_func_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_error_v();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_error_v passing as funclen argument an invalid size_t value 
// 
TEST_FUNCTION(__zlog_error_v__arg_funclen_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_error_v();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_error_v passing as line argument an invalid long value 
// 
TEST_FUNCTION(__zlog_error_v__arg_line_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_error_v();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_error_v passing as format argument an invalid const char* value 
// 
TEST_FUNCTION(__zlog_error_v__arg_format_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_error_v();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_error_v passing as args argument an invalid va_list value 
// 
TEST_FUNCTION(__zlog_error_v__arg_args_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_error_v();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_error_v unhappy path 
// 
TEST_FUNCTION(__zlog_error_v__neg)
{
    const zlog_t k_log_valid;
    static const char* k_file_valid;
    const size_t k_filelen_valid;
    static const char* k_func_valid;
    const size_t k_funclen_valid;
    const long k_line_valid;
    static const char* k_format_valid;
    const va_list k_args_valid;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = __zlog_error_v(k_log_valid, k_file_valid, k_filelen_valid, k_func_valid, k_funclen_valid, k_line_valid, k_format_valid, k_args_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, 0, result);

}

;
// line117 "util_zlog.h"




static __inline void __zlog_debug(
    zlog_t log,
    const char *file,
    size_t filelen,
    const char *func,
    size_t funclen,
    long line,
    const char* format,
    ...
)
{
    va_list va;
    ((void)(__vcrt_va_start_verify_argument_type<decltype(format)>(), ((void)(__va_start(&va, format)))));
    __zlog_debug_v(log, file, filelen, func, funclen, line, format, va);
    ((void)(va = (va_list)0));
}




static __inline void __zlog_info(
    zlog_t log,
    const char *file,
    size_t filelen,
    const char *func,
    size_t funclen,
    long line,
    const char* format,
    ...
)
{
    va_list va;
    ((void)(__vcrt_va_start_verify_argument_type<decltype(format)>(), ((void)(__va_start(&va, format)))));
    __zlog_info_v(log, file, filelen, func, funclen, line, format, va);
    ((void)(va = (va_list)0));
}




static __inline void  __zlog_error(
    zlog_t log,
    const char *file,
    size_t filelen,
    const char *func,
    size_t funclen,
    long line,
    const char* format,
    ...
)
{
    va_list va;
    ((void)(__vcrt_va_start_verify_argument_type<decltype(format)>(), ((void)(__va_start(&va, format)))));
    __zlog_error_v(log, file, filelen, func, funclen, line, format, va);
    ((void)(va = (va_list)0));
}




// 
// Test __zlog_debug_b happy path 
// 
TEST_FUNCTION(__zlog_debug_b__success)
{
    const zlog_t k_log_valid;
    static const char* k_file_valid;
    const size_t k_filelen_valid;
    static const char* k_func_valid;
    const size_t k_funclen_valid;
    const long k_line_valid;
    static const char* k_buf_valid;
    const size_t k_buflen_valid;

    // arrange 
    // ... 

    // act 
    result = __zlog_debug_b(k_log_valid, k_file_valid, k_filelen_valid, k_func_valid, k_funclen_valid, k_line_valid, k_buf_valid, k_buflen_valid);

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_debug_b passing as log argument an invalid zlog_t value 
// 
TEST_FUNCTION(__zlog_debug_b__arg_log_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_debug_b();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_debug_b passing as file argument an invalid const char* value 
// 
TEST_FUNCTION(__zlog_debug_b__arg_file_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_debug_b();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_debug_b passing as filelen argument an invalid size_t value 
// 
TEST_FUNCTION(__zlog_debug_b__arg_filelen_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_debug_b();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_debug_b passing as func argument an invalid const char* value 
// 
TEST_FUNCTION(__zlog_debug_b__arg_func_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_debug_b();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_debug_b passing as funclen argument an invalid size_t value 
// 
TEST_FUNCTION(__zlog_debug_b__arg_funclen_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_debug_b();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_debug_b passing as line argument an invalid long value 
// 
TEST_FUNCTION(__zlog_debug_b__arg_line_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_debug_b();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_debug_b passing as buf argument an invalid const char* value 
// 
TEST_FUNCTION(__zlog_debug_b__arg_buf_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_debug_b();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_debug_b passing as buflen argument an invalid size_t value 
// 
TEST_FUNCTION(__zlog_debug_b__arg_buflen_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_debug_b();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_debug_b unhappy path 
// 
TEST_FUNCTION(__zlog_debug_b__neg)
{
    const zlog_t k_log_valid;
    static const char* k_file_valid;
    const size_t k_filelen_valid;
    static const char* k_func_valid;
    const size_t k_funclen_valid;
    const long k_line_valid;
    static const char* k_buf_valid;
    const size_t k_buflen_valid;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = __zlog_debug_b(k_log_valid, k_file_valid, k_filelen_valid, k_func_valid, k_funclen_valid, k_line_valid, k_buf_valid, k_buflen_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, 0, result);

}

// 
// Test __zlog_info_b happy path 
// 
TEST_FUNCTION(__zlog_info_b__success)
{
    const zlog_t k_log_valid;
    static const char* k_file_valid;
    const size_t k_filelen_valid;
    static const char* k_func_valid;
    const size_t k_funclen_valid;
    const long k_line_valid;
    static const char* k_buf_valid;
    const size_t k_buflen_valid;

    // arrange 
    // ... 

    // act 
    result = __zlog_info_b(k_log_valid, k_file_valid, k_filelen_valid, k_func_valid, k_funclen_valid, k_line_valid, k_buf_valid, k_buflen_valid);

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_info_b passing as log argument an invalid zlog_t value 
// 
TEST_FUNCTION(__zlog_info_b__arg_log_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_info_b();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_info_b passing as file argument an invalid const char* value 
// 
TEST_FUNCTION(__zlog_info_b__arg_file_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_info_b();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_info_b passing as filelen argument an invalid size_t value 
// 
TEST_FUNCTION(__zlog_info_b__arg_filelen_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_info_b();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_info_b passing as func argument an invalid const char* value 
// 
TEST_FUNCTION(__zlog_info_b__arg_func_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_info_b();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_info_b passing as funclen argument an invalid size_t value 
// 
TEST_FUNCTION(__zlog_info_b__arg_funclen_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_info_b();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_info_b passing as line argument an invalid long value 
// 
TEST_FUNCTION(__zlog_info_b__arg_line_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_info_b();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_info_b passing as buf argument an invalid const char* value 
// 
TEST_FUNCTION(__zlog_info_b__arg_buf_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_info_b();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_info_b passing as buflen argument an invalid size_t value 
// 
TEST_FUNCTION(__zlog_info_b__arg_buflen_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_info_b();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_info_b unhappy path 
// 
TEST_FUNCTION(__zlog_info_b__neg)
{
    const zlog_t k_log_valid;
    static const char* k_file_valid;
    const size_t k_filelen_valid;
    static const char* k_func_valid;
    const size_t k_funclen_valid;
    const long k_line_valid;
    static const char* k_buf_valid;
    const size_t k_buflen_valid;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = __zlog_info_b(k_log_valid, k_file_valid, k_filelen_valid, k_func_valid, k_funclen_valid, k_line_valid, k_buf_valid, k_buflen_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, 0, result);

}

// 
// Test __zlog_error_b happy path 
// 
TEST_FUNCTION(__zlog_error_b__success)
{
    const zlog_t k_log_valid;
    static const char* k_file_valid;
    const size_t k_filelen_valid;
    static const char* k_func_valid;
    const size_t k_funclen_valid;
    const long k_line_valid;
    static const char* k_buf_valid;
    const size_t k_buflen_valid;

    // arrange 
    // ... 

    // act 
    result = __zlog_error_b(k_log_valid, k_file_valid, k_filelen_valid, k_func_valid, k_funclen_valid, k_line_valid, k_buf_valid, k_buflen_valid);

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_error_b passing as log argument an invalid zlog_t value 
// 
TEST_FUNCTION(__zlog_error_b__arg_log_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_error_b();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_error_b passing as file argument an invalid const char* value 
// 
TEST_FUNCTION(__zlog_error_b__arg_file_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_error_b();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_error_b passing as filelen argument an invalid size_t value 
// 
TEST_FUNCTION(__zlog_error_b__arg_filelen_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_error_b();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_error_b passing as func argument an invalid const char* value 
// 
TEST_FUNCTION(__zlog_error_b__arg_func_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_error_b();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_error_b passing as funclen argument an invalid size_t value 
// 
TEST_FUNCTION(__zlog_error_b__arg_funclen_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_error_b();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_error_b passing as line argument an invalid long value 
// 
TEST_FUNCTION(__zlog_error_b__arg_line_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_error_b();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_error_b passing as buf argument an invalid const char* value 
// 
TEST_FUNCTION(__zlog_error_b__arg_buf_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_error_b();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_error_b passing as buflen argument an invalid size_t value 
// 
TEST_FUNCTION(__zlog_error_b__arg_buflen_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_error_b();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_error_b unhappy path 
// 
TEST_FUNCTION(__zlog_error_b__neg)
{
    const zlog_t k_log_valid;
    static const char* k_file_valid;
    const size_t k_filelen_valid;
    static const char* k_func_valid;
    const size_t k_funclen_valid;
    const long k_line_valid;
    static const char* k_buf_valid;
    const size_t k_buflen_valid;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = __zlog_error_b(k_log_valid, k_file_valid, k_filelen_valid, k_func_valid, k_funclen_valid, k_line_valid, k_buf_valid, k_buflen_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, 0, result);

}

// 
// Test __zlog_info_a happy path 
// 
TEST_FUNCTION(__zlog_info_a__success)
{
    const zlog_t k_log_valid;
    static const char* k_format_valid;
    const va_list k_args_valid;

    // arrange 
    // ... 

    // act 
    result = __zlog_info_a(k_log_valid, k_format_valid, k_args_valid);

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_info_a passing as log argument an invalid zlog_t value 
// 
TEST_FUNCTION(__zlog_info_a__arg_log_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_info_a();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_info_a passing as format argument an invalid const char* value 
// 
TEST_FUNCTION(__zlog_info_a__arg_format_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_info_a();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_info_a passing as args argument an invalid va_list value 
// 
TEST_FUNCTION(__zlog_info_a__arg_args_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_info_a();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_info_a unhappy path 
// 
TEST_FUNCTION(__zlog_info_a__neg)
{
    const zlog_t k_log_valid;
    static const char* k_format_valid;
    const va_list k_args_valid;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = __zlog_info_a(k_log_valid, k_format_valid, k_args_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, 0, result);

}

// 
// Test __zlog_info_a_flush happy path 
// 
TEST_FUNCTION(__zlog_info_a_flush__success)
{
    const zlog_t k_log_valid;
    static const char* k_file_valid;
    const size_t k_filelen_valid;
    static const char* k_func_valid;
    const size_t k_funclen_valid;
    const long k_line_valid;
    static const char* k_format_valid;
    const va_list k_args_valid;

    // arrange 
    // ... 

    // act 
    result = __zlog_info_a_flush(k_log_valid, k_file_valid, k_filelen_valid, k_func_valid, k_funclen_valid, k_line_valid, k_format_valid, k_args_valid);

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_info_a_flush passing as log argument an invalid zlog_t value 
// 
TEST_FUNCTION(__zlog_info_a_flush__arg_log_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_info_a_flush();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_info_a_flush passing as file argument an invalid const char* value 
// 
TEST_FUNCTION(__zlog_info_a_flush__arg_file_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_info_a_flush();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_info_a_flush passing as filelen argument an invalid size_t value 
// 
TEST_FUNCTION(__zlog_info_a_flush__arg_filelen_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_info_a_flush();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_info_a_flush passing as func argument an invalid const char* value 
// 
TEST_FUNCTION(__zlog_info_a_flush__arg_func_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_info_a_flush();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_info_a_flush passing as funclen argument an invalid size_t value 
// 
TEST_FUNCTION(__zlog_info_a_flush__arg_funclen_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_info_a_flush();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_info_a_flush passing as line argument an invalid long value 
// 
TEST_FUNCTION(__zlog_info_a_flush__arg_line_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_info_a_flush();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_info_a_flush passing as format argument an invalid const char* value 
// 
TEST_FUNCTION(__zlog_info_a_flush__arg_format_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_info_a_flush();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_info_a_flush passing as args argument an invalid va_list value 
// 
TEST_FUNCTION(__zlog_info_a_flush__arg_args_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = __zlog_info_a_flush();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test __zlog_info_a_flush unhappy path 
// 
TEST_FUNCTION(__zlog_info_a_flush__neg)
{
    const zlog_t k_log_valid;
    static const char* k_file_valid;
    const size_t k_filelen_valid;
    static const char* k_func_valid;
    const size_t k_funclen_valid;
    const long k_line_valid;
    static const char* k_format_valid;
    const va_list k_args_valid;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = __zlog_info_a_flush(k_log_valid, k_file_valid, k_filelen_valid, k_func_valid, k_funclen_valid, k_line_valid, k_format_valid, k_args_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, 0, result);

}

// 
// Test log_init happy path 
// 
TEST_FUNCTION(log_init__success)
{

    // arrange 
    // ... 

    // act 
    result = log_init();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test log_init unhappy path 
// 
TEST_FUNCTION(log_init__neg)
{

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = log_init();

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, 0, result);

}

// 
// Test log_deinit happy path 
// 
TEST_FUNCTION(log_deinit__success)
{

    // arrange 
    // ... 

    // act 
    result = log_deinit();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test log_deinit unhappy path 
// 
TEST_FUNCTION(log_deinit__neg)
{

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = log_deinit();

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, 0, result);

}

#endif

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

