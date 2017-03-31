
#ifndef _util_zlog_h_
#define _util_zlog_h_

#include "common.h"
#include <stdarg.h>

//
// A log category
//
typedef void* zlog_t;

//
// Log messages - note same format as zlog for simplicity
//
typedef struct zlog_entry
{
    char *msg;
    size_t len;
    char *target;
}
zlog_entry_t;

//
// A logger callback to receive log events
//
typedef void (*zlog_cb_t)(
    zlog_entry_t* msg
);

//
// Initialize logging facilities
//
decl_public_0(int32_t, zlog_initialize,
    void
);

//
// Register callback to receive events
//
decl_public_2(int32_t, zlog_register,
    const char*, target,
    zlog_cb_t, callback
);

//
// Configure log file
//
decl_public_1(int32_t, zlog_set_log_file,
    const char*, file_name
);

//
// Configure logging facilities from configuration file
//
decl_public_1(int32_t, zlog_read_config,
    const char*, file_name
);

//
// Gets a logger to log message to
//
decl_public_1(zlog_t, zlog_get,
    const char*, area
);

//
// Unregister callback 
//
decl_public_1(int32_t, zlog_unregister,
    const char*, target
);

//
// Deinitialize logging facilities
//
decl_public_0(void, zlog_deinit,
    void
);

// Internal implementation

//
// Log debug message implementation
//
decl_public_8(void, __zlog_debug_v,
    zlog_t, log,
    const char*, file,
    size_t, filelen,
    const char*, func,
    size_t, funclen,
    long, line,
    const char*, format,
    va_list, args
);

//
// Log trace message implementation
//
decl_public_8(void, __zlog_trace_v,
    zlog_t, log,
    const char*, file,
    size_t, filelen,
    const char*, func,
    size_t, funclen,
    long, line,
    const char*, format,
    va_list, args
);

//
// Log event message implementation
//
decl_public_8(void, __zlog_info_v,
    zlog_t, log,
    const char*, file,
    size_t, filelen,
    const char*, func,
    size_t, funclen,
    long, line,
    const char*, format,
    va_list, args
);

//
// Log error message implementation
//
decl_public_8(void, __zlog_error_v,
    zlog_t, log,
    const char*, file,
    size_t, filelen,
    const char*, func,
    size_t, funclen,
    long, line,
    const char*, format,
    va_list, args
);

//
// Log debug message implementation
//
decl_public_8(void, __zlog_debug_b,
    zlog_t, log,
    const char*, file,
    size_t, filelen,
    const char*, func,
    size_t, funclen,
    long, line,
    const char*, buf,
    size_t, buflen
);

//
// Log event message implementation
//
decl_public_8(void, __zlog_info_b,
    zlog_t, log,
    const char*, file,
    size_t, filelen,
    const char*, func,
    size_t, funclen,
    long, line,
    const char*, buf,
    size_t, buflen
);

//
// Log trace message implementation
//
decl_public_8(void, __zlog_trace_b,
    zlog_t, log,
    const char*, file,
    size_t, filelen,
    const char*, func,
    size_t, funclen,
    long, line,
    const char*, buf,
    size_t, buflen
);

//
// Log error message implementation
//
decl_public_8(void, __zlog_error_b,
    zlog_t, log,
    const char*, file,
    size_t, filelen,
    const char*, func,
    size_t, funclen,
    long, line,
    const char*, buf,
    size_t, buflen
);

//
// Accumulate a partial log event
//
decl_public_3(void, __zlog_info_a,
    zlog_t, log,
    const char*, format,
    va_list, args
);

//
// Flush all partial log events and this one
//
decl_public_8(void, __zlog_info_a_flush,
    zlog_t, log,
    const char*, file,
    size_t, filelen,
    const char*, func,
    size_t, funclen,
    long, line,
    const char*, format,
    va_list, args
);

#endif // _util_zlog_h_
