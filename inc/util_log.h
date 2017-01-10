// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _util_log_h_
#define _util_log_h_

#include "common.h"
#include "util_misc.h"

#if defined(UNIT_TEST)
#define NO_LOGGING 1
#endif

#if defined(NO_LOGGING)
#define NO_ZLOG 1
#endif

#if defined(NO_ZLOG)

#define   log_t               void*
#define   log_entry_t         void
#define   log_init()          er_ok
#define   log_deinit()        (void)0
#define   log_register(t, c)  er_ok
#define   log_configure(x)    er_ok
#define   log_unregister(t)   er_ok
#define   log_get(x)          NULL

#if !defined(NO_LOGGING)
//
// Console logger
//
#include "util_clog.h"
#define __log_debug_v         __clog_debug_v
#define __log_info_v          __clog_info_v
#define __log_error_v         __clog_error_v
#define __log_debug           __clog_debug
#define __log_info            __clog_info
#define __log_error           __clog_error
#else // NO_LOGGING           
                              
//                            
// Null logger                
//                            
#define __log_debug_v(...)    (void)0
#define __log_info_v(...)     (void)0
#define __log_error_v(...)    (void)0
#define __log_debug(...)      (void)0
#define __log_info(...)       (void)0
#define __log_error(...)      (void)0
                              
#endif // !NO_LOGGING         
                              
#define __log_debug_b(...)    (void)0
#define __log_info_b(...)     (void)0
#define __log_error_b(...)    (void)0

#else // !NO_ZLOG 

//
// Zlog logging
//
#include "util_zlog.h"

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

#define   log_t               zlog_t
#define   log_entry_t         zlog_entry_t
#define   log_cb_t            zlog_cb_t
#define   log_register        zlog_register
#define   log_configure       zlog_configure
#define   log_get             zlog_get
#define   log_unregister      zlog_unregister
#define __log_debug_v       __zlog_debug_v
#define __log_info_v        __zlog_info_v
#define __log_error_v       __zlog_error_v
#define __log_debug         __zlog_debug
#define __log_info          __zlog_info
#define __log_error         __zlog_error
#define __log_debug_b       __zlog_debug_b
#define __log_info_b        __zlog_info_b
#define __log_error_b       __zlog_error_b
#endif

//
// Log a info message for a category
//
#if defined(_MSC_VER) || (defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L)
#define log_info(log, ...) \
      __log_info(log, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
        __VA_ARGS__) 
#else
#define log_info(log, args...) \
      __log_info(log, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
        ## args )
#endif

//
// Log an error message for a category
//
#ifndef break_on_error
#define break_on_error()
#endif
#if defined(_MSC_VER) || (defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L)
#define log_error(log, ...) \
    { __log_error(log, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
        __VA_ARGS__); break_on_error(); }
#else
#define log_error(log, args...) \
    { __log_error(log, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
        ## args ); break_on_error(); }
#endif

//
// Log a debug message for a category
//
#if defined(_MSC_VER) || (defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L)
#define log_debug(log, ...) \
      __log_debug(log, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
        __VA_ARGS__) 
#else
#define log_debug(log, args...) \
      __log_debug(log, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
        ## args )
#endif

//
// Same just with var args
//
#define log_info_v(log, FORMAT, va) \
    __log_info_v(log, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
        FORMAT, va) 
#define log_error_v(log, FORMAT, va) \
    __log_error_v(log, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
        FORMAT, va) 
#define log_debug_v(log, FORMAT, va) \
    __log_debug_v(log, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
        FORMAT, va) 

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
