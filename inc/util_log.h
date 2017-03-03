// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _util_log_h_
#define _util_log_h_

#include "common.h"
#include "util_misc.h"

#if defined(UNIT_TEST)
#define NO_LOGGING 1
#endif

#if defined(NO_LOGGING) || defined(NO_ZLOG)

#define NO_ZLOG 1

// 
// Null or xlogger
//
#define   log_init()          er_ok
#define   log_deinit()        (void)0

#define   log_t               void*
#define   log_entry_t         void
#define   log_register(t, c)  er_ok
#define   log_read_config(x)  er_not_supported
#define   log_set_log_file(x) er_not_supported
#define   log_unregister(t)   er_ok
#define   log_get(x)          NULL

#if defined(NO_LOGGING)

//
// No logging whatsoever
//

#define __log(log, c, file, func, line, fmt, ...) \
    do { (void)log; \
    } while(0)

#define __log_b(log, c, file, func, line, fmt, b, len) \
    do { (void)log; \
    } while(0)

#else // !NO_LOGGING

//
// Xlogging adapter
//
#include "azure_c_shared_utility/xlogging.h"
#define CRLF "\n"

#ifdef LOG_VERBOSE
#define __should_log(c) (true)
#else
#define __should_log(c) (c != AZ_LOG_TRACE)
#endif

// 
// Log messages using xlogging
//
#if defined(_MSC_VER)
#define __log(log, c, file, func, line, fmt, ...) \
    do { LOGGER_LOG logger = xlogging_get_log_function(); \
        (void)log; if (logger && __should_log(c)) logger( \
            c, file, func, line, 0, fmt "\r\n", __VA_ARGS__); \
    } while(0)
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define __log(log, c, file, func, line, fmt, ...) \
    do { LOGGER_LOG logger = xlogging_get_log_function(); \
        (void)log; if (logger && __should_log(c)) logger( \
            c, file, func, line, 0, fmt "\n", ##__VA_ARGS__); \
    } while(0)
#else
#define __log(log, c, file, func, line, fmt, args...) \
    do { LOGGER_LOG logger = xlogging_get_log_function(); \
        (void)log; if (logger && __should_log(c)) logger( \
            c, file, func, line, 0, fmt "\n", ## args); \
    } while(0)
#endif

//
// Log a buffer using xlogging
//
#define __log_b(log, c, file, func, line, fmt, b, len) \
    do { (void)log; \
        xlogging_dump_buffer(b, len); \
    } while(0)

#endif // !NO_LOGGING

#if defined(_MSC_VER)
#define __log_debug(log, file, a1, func, a2, line, fmt, ...) \
      __log(log, AZ_LOG_TRACE, file, func, line, fmt, __VA_ARGS__)
#define __log_info(log, file, a1, func, a2, line, fmt, ...) \
      __log(log, AZ_LOG_INFO, file, func, line, fmt, __VA_ARGS__)
#define __log_error(log, file, a1, func, a2, line, fmt, ...) \
      __log(log, AZ_LOG_ERROR, file, func, line, fmt, __VA_ARGS__)
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#define __log_debug(log, file, a1, func, a2, line, fmt, ...) \
      __log(log, AZ_LOG_TRACE, file, func, line, fmt, ##__VA_ARGS__)
#define __log_info(log, file, a1, func, a2, line, fmt, ...) \
      __log(log, AZ_LOG_INFO, file, func, line, fmt, ##__VA_ARGS__)
#define __log_error(log, file, a1, func, a2, line, fmt, ...) \
      __log(log, AZ_LOG_ERROR, file, func, line, fmt, ##__VA_ARGS__)
#else
#define __log_debug(log, file, a1, func, a2, line, args...) \
      __log(log, AZ_LOG_TRACE, file, func, line, ## args)
#define __log_info(log, file, a1, func, a2, line, args...) \
      __log(log, AZ_LOG_INFO, file, func, line, ## args)
#define __log_error(log, file, a1, func, a2, line, args...) \
      __log(log, AZ_LOG_ERROR, file, func, line, ## args)
#endif

#define __log_debug_b(log, file, a1, func, a2, line, fmt, b, len) \
      __log_b(log, AZ_LOG_TRACE, file, func, line, fmt, b, len)
#define __log_info_b(log, file, a1, func, a2, line, fmt, b, len) \
      __log_b(log, AZ_LOG_INFO, file, func, line, fmt, b, len)
#define __log_error_b(log, file, a1, func, a2, line, fmt, b, len) \
      __log_b(log, AZ_LOG_ERROR, file, func, line, fmt, b, len)

#else // !NO_ZLOG 

//
// Zlog logging adapter
//

#include "util_zlog.h"

#define   log_t               zlog_t
#define   log_entry_t         zlog_entry_t
#define   log_cb_t            zlog_cb_t
#define   log_register        zlog_register
#define   log_read_config     zlog_read_config
#define   log_set_log_file    zlog_set_log_file
#define   log_get             zlog_get
#define   log_unregister      zlog_unregister

//
// Initialize logging facilities
//
decl_public_0(int32_t, log_init,
    void
);

//
// Deinitialize logging facilities
//
decl_public_0(void, log_deinit,
    void
);

#include <stdarg.h>

//
// Log debug message implementation
//
_inl__ void __log_debug(
    log_t log,
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
    va_start(va, format);
    __zlog_debug_v(log, file, filelen, func, funclen, line, format, va);
    va_end(va);
}

//
// Log trace message implementation
//
_inl__ void __log_info(
    log_t log,
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
    va_start(va, format);
    __zlog_info_v(log, file, filelen, func, funclen, line, format, va);
    va_end(va);
}

//
// Log error message implementation
//
_inl__ void  __log_error(
    log_t log,
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
    va_start(va, format);
    __zlog_error_v(log, file, filelen, func, funclen, line, format, va);
    va_end(va);
}

#define __log_debug_b       __zlog_debug_b
#define __log_info_b        __zlog_info_b
#define __log_error_b       __zlog_error_b

#endif // !NO_ZLOG 

//
// Log a info message for a category
//
#if defined(_MSC_VER) 
#define log_info(log, fmt, ...) \
      __log_info(log, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
        fmt, __VA_ARGS__) 
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#define log_info(log, fmt, ...) \
      __log_info(log, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
        fmt, ##__VA_ARGS__) 
#else
#define log_info(log, fmt, args...) \
      __log_info(log, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
        fmt, ## args )
#endif

//
// Log an error message for a category
//
#ifndef break_on_error
#define break_on_error()
#endif

#if defined(_MSC_VER) 
#define log_error(log, fmt, ...) \
    { __log_error(log, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
        fmt, __VA_ARGS__); break_on_error(); }
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#define log_error(log, fmt, ...) \
      __log_error(log, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
        fmt, ##__VA_ARGS__) 
#else
#define log_error(log, fmt, args...) \
    { __log_error(log, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
        fmt, ## args ); break_on_error(); }
#endif

//
// Log a debug message for a category
//
#if defined(_MSC_VER) 
#define log_debug(log, fmt, ...) \
      __log_debug(log, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
        fmt, __VA_ARGS__) 
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#define log_debug(log, fmt, ...) \
      __log_debug(log, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
        fmt, ##__VA_ARGS__) 
#else
#define log_debug(log, fmt, args...) \
      __log_debug(log, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
        fmt, ## args )
#endif

//
// Log buffers
//
#define log_info_b(log, b, len) \
      __log_info_b(log, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
        b, (size_t)len)
#define log_error_b(log, b, len) \
      __log_error_b(log, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
        b, (size_t)len)
#define log_debug_b(log, b, len) \
      __log_debug_b(log, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
        b, (size_t)len)


#endif // _util_log_h_
