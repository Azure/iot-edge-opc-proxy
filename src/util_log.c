// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "util_log.h"
#include "util_string.h"
#include "util_misc.h"
#if !defined(NO_ZLOG)
#include "util_zlog.h"
#include "azure_c_shared_utility/xlogging.h"
#endif
#include <stdarg.h>

#if !defined(NO_ZLOG)

#define LINE_BREAK

//
// Logs xlogging trace
//
static void zlog_logger_log(
    LOG_CATEGORY log_category,
    const char* file, 
    const char* func, 
    const int line,
    unsigned int options,
    const char* format, 
    ...
)
{
    log_t log;
    va_list args;
    log = log_get("xlog");

    va_start(args, format);
    switch (log_category)
    {
    case AZ_LOG_ERROR:
        __zlog_error_v(log, file, strlen(file), func, strlen(func), line, format, args);
        break;
    case AZ_LOG_INFO:
        __zlog_info_v(log, file, strlen(file), func, strlen(func), line, format, args);
    case AZ_LOG_TRACE:
    default:
#ifndef LINE_BREAK
        (void)options;
        __zlog_info_v(log, file, strlen(file), func, strlen(func), line, format, args);
#else
        if (options & LOG_LINE)
            __zlog_info_a_flush(log, file, strlen(file), func, strlen(func), line, format, args);
        else
            __zlog_info_a(log, format, args);
#endif
        break;
    }
    va_end(args);
}

//
// Initialize zlog library
//
int32_t log_init(
    void
)
{
    int32_t result;
    result = zlog_initialize();
    if (result != er_ok)
        return result;
    xlogging_set_log_function(zlog_logger_log);
    return result;
}

//
// Deinit zlog.
//
void log_deinit(
    void
)
{
    xlogging_set_log_function(NULL);
    zlog_deinit();
}

#endif
