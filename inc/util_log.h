// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _util_log_h_
#define _util_log_h_

#include "common.h"

//
// Inline logging 
//

#if !defined(UNIT_TEST)

#include <stdarg.h>

#if defined(NO_LOGGING) || defined(NO_ZLOG) 
#define NO_ZLOG 1

// 
// Null or xlogging
//
#define log_t \
    void*
#define log_read_config(x) \
    er_not_supported
#define log_set_log_file(x) \
    er_not_supported
#define log_get(x) \
    NULL

#if !defined(NO_LOGGING)
//
// Xlogging adapter
//
#include "azure_c_shared_utility/xlogging.h"
//
// Logs xlogging trace to log system and stdout
//
void xlogging_logger_v(
    LOG_CATEGORY log_category,
    const char* file,
    const char* func,
    const int line,
    unsigned int options,
    const char* format,
    va_list args
);
#endif // !NO_LOGGING

#define __nolog(a, f, fl, c, cl, n, b, bl) \
     (void)a; (void)f; (void)fl; (void)c; \
    (void)cl; (void)n; (void)b; (void)bl;

#else // !NO_LOGGING && !NO_ZLOG 

//
// Zlog logging adapter
//
#include "util_zlog.h"

#define log_t \
    zlog_t
#define log_read_config \
    zlog_read_config
#define log_set_log_file \
    zlog_set_log_file
#define log_get \
    zlog_get

#endif // !NO_LOGGING && !NO_ZLOG 

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
#if defined(NO_LOGGING) || (defined(NO_ZLOG) && !defined(LOG_VERBOSE))
    __nolog(log, file, filelen, func, funclen, line, format, 0);
#else
    va_list va;
    va_start(va, format);
#if !defined(NO_ZLOG)
    __zlog_debug_v(log, file, filelen, func, funclen, line, format, va);
#else
    (void)log; (void)funclen; (void)filelen;
    xlogging_logger_v(AZ_LOG_TRACE, file, func, line, LOG_LINE, format, va);
#endif
    va_end(va);
#endif
}

//
// Log trace message implementation
//
_inl__ void __log_trace(
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
#if defined(NO_LOGGING)
    __nolog(log, file, filelen, func, funclen, line, format, 0);
#else
    va_list va;
    va_start(va, format);
#if !defined(NO_ZLOG)
    __zlog_trace_v(log, file, filelen, func, funclen, line, format, va);
#else
    (void)log; (void)funclen; (void)filelen;
    xlogging_logger_v(AZ_LOG_TRACE, file, func, line, LOG_LINE, format, va);
#endif
    va_end(va);
#endif
}

//
// Log event message implementation
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
#if defined(NO_LOGGING)
    __nolog(log, file, filelen, func, funclen, line, format, 0);
#else
    va_list va;
    va_start(va, format);
#if !defined(NO_ZLOG)
    __zlog_info_v(log, file, filelen, func, funclen, line, format, va);
#else
    (void)log; (void)funclen; (void)filelen;
    xlogging_logger_v(AZ_LOG_INFO, file, func, line, LOG_LINE, format, va);
#endif
    va_end(va);
#endif
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
#ifdef break_on_error
    break_on_error();
#endif
#if defined(NO_LOGGING)
    __nolog(log, file, filelen, func, funclen, line, format, 0);
#else
    va_list va;
    va_start(va, format);
#if !defined(NO_ZLOG)
    __zlog_error_v(log, file, filelen, func, funclen, line, format, va);
#else
    (void)log; (void)funclen; (void)filelen;
    xlogging_logger_v(AZ_LOG_ERROR, file, func, line, LOG_LINE, format, va);
#endif
    va_end(va);
#endif
}

//
// Log debug message implementation
//
_inl__ void __log_debug_b(
    log_t log,
    const char *file,
    size_t filelen,
    const char *func,
    size_t funclen,
    long line,
    const char* buf,
    size_t buflen
)
{
#if !defined(NO_ZLOG) && !defined(NO_LOGGING)
    __zlog_debug_b(log, file, filelen, func, funclen, line, buf, buflen);
#else
    __nolog(log, file, filelen, func, funclen, line, buf, buflen);
#if !defined(NO_LOGGING) && defined(LOG_VERBOSE)
    xlogging_dump_buffer(buf, buflen);
#endif
#endif
}

//
// Log trace message implementation
//
_inl__ void __log_trace_b(
    log_t log,
    const char *file,
    size_t filelen,
    const char *func,
    size_t funclen,
    long line,
    const char* buf,
    size_t buflen
)
{
#if !defined(NO_ZLOG) && !defined(NO_LOGGING)
    __zlog_trace_b(log, file, filelen, func, funclen, line, buf, buflen);
#else
    __nolog(log, file, filelen, func, funclen, line, buf, buflen);
#if !defined(NO_LOGGING)
    xlogging_dump_buffer(buf, buflen);
#endif
#endif
}

//
// Log trace message implementation
//
_inl__ void __log_info_b(
    log_t log,
    const char *file,
    size_t filelen,
    const char *func,
    size_t funclen,
    long line,
    const char* buf,
    size_t buflen
)
{
#if !defined(NO_ZLOG) && !defined(NO_LOGGING)
    __zlog_info_b(log, file, filelen, func, funclen, line, buf, buflen);
#else
    __nolog(log, file, filelen, func, funclen, line, buf, buflen);
#if !defined(NO_LOGGING)
    xlogging_dump_buffer(buf, buflen);
#endif
#endif
}

//
// Log error message implementation
//
_inl__ void __log_error_b(
    log_t log,
    const char *file,
    size_t filelen,
    const char *func,
    size_t funclen,
    long line,
    const char* buf,
    size_t buflen
)
{
#ifdef break_on_error
    break_on_error();
#endif
#if !defined(NO_ZLOG) && !defined(NO_LOGGING)
    __zlog_error_b(log, file, filelen, func, funclen, line, buf, buflen);
#else
    __nolog(log, file, filelen, func, funclen, line, buf, buflen);
#if !defined(NO_LOGGING)
    xlogging_dump_buffer(buf, buflen);
#endif
#endif
}

//
// Log an error message for a category
//
#if defined(_MSC_VER) 
#define log_error(log, fmt, ...) \
     __log_error(log, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
        fmt, __VA_ARGS__) 
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#define log_error(log, fmt, ...) \
     __log_error(log, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
        fmt, ##__VA_ARGS__) 
#else
#define log_error(log, fmt, args...) \
     __log_error(log, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
        fmt, ## args )
#endif

//
// Log an event message for a category
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
// Log a trace message for a category
//
#if defined(_MSC_VER) 
#define log_trace(log, fmt, ...) \
      __log_trace(log, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
        fmt, __VA_ARGS__) 
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#define log_trace(log, fmt, ...) \
      __log_trace(log, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
        fmt, ##__VA_ARGS__) 
#else
#define log_trace(log, fmt, args...) \
      __log_trace(log, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
        fmt, ## args )
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
#define log_error_b(log, b, len) \
    __log_error_b(log, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
        b, (size_t)len)
#define  log_info_b(log, b, len) \
     __log_info_b(log, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
        b, (size_t)len)
#define log_trace_b(log, b, len) \
    __log_trace_b(log, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
        b, (size_t)len)
#define log_debug_b(log, b, len) \
    __log_debug_b(log, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
        b, (size_t)len)

#endif // !UNIT_TEST
#endif // _util_log_h_
