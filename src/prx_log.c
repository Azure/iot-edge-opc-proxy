// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "util_string.h"
#include "util_misc.h"
#include "util_ringbuf.h"
#include "pal_mt.h"

#include "prx_log.h"
#include "io_stream.h"

#if !defined(NO_ZLOG)
#include "util_zlog.h"
#endif

#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/doublylinkedlist.h"

#include <stdarg.h>

//
// Logging system handle
//
typedef struct log_system
{
    DLIST_ENTRY streams;  // All streams created in the log system
    rw_lock_t streams_lock;          // Safe multi threaded access
    io_stream_t* init;          // A stream that exists since init
}
log_system_t;

//
// The log buffer stream
//
typedef struct log_stream
{
    io_stream_t itf;
    ring_buffer_t* buffer;        // The buffer used by the stream
    rw_lock_t buffer_lock;            // Safe multi threaded write
    log_system_t* ls;                     // The owning log system
    DLIST_ENTRY link;   // To link into the log system stream list
}
log_stream_t;

//
// Reads from log stream
//
static int32_t log_stream_reader(
    log_stream_t* log_stream,
    void *data,
    size_t count,
    size_t* read
)
{
    dbg_assert_ptr(read);
    dbg_assert_ptr(data);
    dbg_assert_ptr(log_stream);
    rw_lock_enter(log_stream->buffer_lock);
    *read = ring_buffer_read(log_stream->buffer, (uint8_t*)data, count);
    rw_lock_exit(log_stream->buffer_lock);
    return er_ok;
}

//
// Available in stream to read
//
static size_t log_stream_readable(
    log_stream_t* log_stream
)
{
    size_t avail;
    dbg_assert_ptr(log_stream);
    rw_lock_enter(log_stream->buffer_lock);
    avail = ring_buffer_available(log_stream->buffer);
    rw_lock_exit(log_stream->buffer_lock);
    return avail;
}

//
// Reset log stream
//
static int32_t log_stream_reset(
    log_stream_t* log_stream
)
{
    dbg_assert_ptr(log_stream);
    rw_lock_enter_w(log_stream->buffer_lock);
    ring_buffer_clear(log_stream->buffer);
    rw_lock_exit_w(log_stream->buffer_lock);
    return er_ok;
}

//
// Close log stream
//
static void log_stream_close(
    log_stream_t* log_stream
)
{
    log_system_t* ls;
    if (!log_stream)
        return;

    ls = log_stream->ls;
    if (ls)
    {
        rw_lock_enter_w(ls->streams_lock);
        log_stream->ls = NULL;
        DList_RemoveEntryList(&log_stream->link);
        rw_lock_exit_w(ls->streams_lock);
    }

    if (log_stream->buffer)
        ring_buffer_free(log_stream->buffer);
    if (log_stream->buffer_lock)
        rw_lock_free(log_stream->buffer_lock);

    mem_free_type(log_stream_t, log_stream);
}

//
// Free log system
//
static void log_system_free(
    log_system_t* ls
)
{
    if (!ls)
        return;

    if (ls->init)
        io_stream_close(ls->init);

    if (ls->streams_lock)
        rw_lock_enter_w(ls->streams_lock);

    // Clear streams list of any remaining streams.
    while (!DList_IsListEmpty(&ls->streams))
        containingRecord(DList_RemoveHeadList(&ls->streams),
            log_stream_t, link)->ls = NULL;

    if (ls->streams_lock)
        rw_lock_exit_w(ls->streams_lock);

    if (ls->streams_lock)
        rw_lock_free(ls->streams_lock);
    mem_free_type(log_system_t, ls);
}

//
// Create a log target stream for telemetry to read log messages from
//
static int32_t log_system_create_stream(
    log_system_t* ls,
    io_stream_t** created
)
{
    int32_t result;
    log_stream_t* stream;

    dbg_assert_ptr(ls);
    dbg_assert_ptr(created);

    if (ls->init)
    {
        // Hand out the init stream of the log system
        rw_lock_enter_w(ls->streams_lock);
        *created = ls->init;
        ls->init = NULL;
        rw_lock_exit_w(ls->streams_lock);

        if (*created)
            return er_ok;
    }

    stream = mem_zalloc_type(log_stream_t);
    if (!stream)
        return er_out_of_memory;
    do
    {
        DList_InitializeListHead(&stream->link);

        result = rw_lock_create(&stream->buffer_lock);
        if (result != er_ok)
            break;

        result = ring_buffer_create(0x10000, &stream->buffer);
        if (result != er_ok)
            break;

        stream->itf.context =
            stream;
        stream->itf.writer =
            NULL;
        stream->itf.writeable =
            NULL;
        stream->itf.reader = (io_stream_reader_t)
            log_stream_reader;
        stream->itf.readable = (io_stream_available_t)
            log_stream_readable;
        stream->itf.reset = (io_stream_reset_t)
            log_stream_reset;
        stream->itf.close = (io_stream_close_t)
            log_stream_close;

        rw_lock_enter_w(ls->streams_lock);
        DList_InsertTailList(&ls->streams, &stream->link);
        stream->ls = ls;
        rw_lock_exit_w(ls->streams_lock);

        *created = &stream->itf;
        return er_ok;
    } while (0);

    log_stream_close(stream);
    return result;
}

//
// Create log system
//
static int32_t log_system_create(
    log_system_t** created
)
{
    int32_t result;
    log_system_t* ls;

    ls = mem_zalloc_type(log_system_t);
    if (!ls)
        return er_out_of_memory;
    do
    {
        DList_InitializeListHead(&ls->streams);
        result = rw_lock_create(&ls->streams_lock);
        if (result != er_ok)
            break;

        result = log_system_create_stream(ls, &ls->init);
        if (result != er_ok)
            break;

        dbg_assert(!DList_IsListEmpty(&ls->streams), 
            "unexpected");
        *created = ls;
        return er_ok;
    } while (0);

    log_system_free(ls);
    return result;
}

//
// Log a message with length to log system
//
static void log_system_log(
    log_system_t* ls,
    const char* message,
    size_t len
)
{
    log_stream_t* next;
    if (!ls || !len || !message)
        return;

    rw_lock_enter(ls->streams_lock);
    for (PDLIST_ENTRY p = ls->streams.Flink; p != &ls->streams; p = p->Flink)
    {
        next = containingRecord(p, log_stream_t, link);
        rw_lock_enter_w(next->buffer_lock);
        ring_buffer_write(next->buffer, (uint8_t*)message, len);
        rw_lock_exit_w(next->buffer_lock);
    }
    rw_lock_exit(ls->streams_lock);
}

//
// Global log system instance for this process
//
static log_system_t* _log = NULL;

//
// Get a log stream to read telemetry log messages from
//
io_stream_t* log_stream_get(
    void
)
{
    io_stream_t* stream;
    if (er_ok != log_system_create_stream(_log, &stream))
        return NULL;
    return stream;
}

#if !defined(NO_ZLOG)
//
// Logs zlog $default trace to log system streams
//
static void zlog_subscription(
    zlog_entry_t* entry
)
{
    dbg_assert_ptr(_log);
    dbg_assert_ptr(entry);
    log_system_log(_log, entry->msg, entry->len);
}

#else // !defined(NO_ZLOG)
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
)
{
    time_t t;
    char message[2048];
    size_t buf_len, written;
    char* buf;

    (void)file;
    dbg_assert_ptr(func);
    dbg_assert_ptr(format);

    buf_len = sizeof(message) - 1;
    buf = message;
    t = time(NULL);
    written = snprintf(buf, buf_len, 
        "%.24s (%s:%d) [%s] ", ctime(&t), func, line,
        log_category == AZ_LOG_ERROR ? "ERROR" :
        log_category == AZ_LOG_INFO  ? "INFO" : 
                                       "DEBUG");
    if (!written)
        return;
    buf_len -= written;
    buf += written;
    if (buf_len > 0)
    {
        written = vsnprintf(buf, buf_len, format, args);
        if (!written)
            return;
        buf_len -= written;
        buf += written;
        if (LOG_LINE == (options & LOG_LINE) && buf_len >= 2)
        {
            strcpy(buf, "\r\n");
            buf_len -= 2;
            buf += 2;
        }
    }
    printf("%s", message);
    if (log_category != AZ_LOG_TRACE)
        log_system_log(_log, message, (size_t)(buf - message));
}

#endif // !defined(NO_ZLOG)

//
// Logs xlogging trace to log system and stdout
//
static void xlogging_logger(
    LOG_CATEGORY log_category,
    const char* file,
    const char* func,
    const int line,
    unsigned int options,
    const char* format,
    ...
)
{
    va_list args;
#if !defined(NO_ZLOG)
    log_t log;
    log = log_get("xlog");

    va_start(args, format);
    switch (log_category)
    {
    case AZ_LOG_ERROR:
        __zlog_error_v(log, file, strlen(file), func,
            strlen(func), line, format, args);
        break;
    case AZ_LOG_INFO:
        __zlog_info_v(log, file, strlen(file), func,
            strlen(func), line, format, args);
    case AZ_LOG_TRACE:
    default:
#ifndef LINE_BREAK
        (void)options;
        __zlog_info_v(log, file, strlen(file), func,
            strlen(func), line, format, args);
#else
        if (options & LOG_LINE)
            __zlog_info_a_flush(log, file, strlen(file), func,
                strlen(func), line, format, args);
        else
            __zlog_info_a(log, format, args);
#endif
        break;
    }
#else // defined(NO_ZLOG)
    va_start(args, format);
    xlogging_logger_v(log_category, file, func, line, options, format, args);
#endif
    va_end(args);
}

//
// Initialize global logging system
//
int32_t log_init(
    void
)
{
    int32_t result;
    if (_log)
        return er_bad_state;
#if !defined(NO_ZLOG)
    // Init zlog first, for any messages that are logged
    result = zlog_initialize();
    if (result != er_ok)
        return result;
#endif

    result = log_system_create(&_log);
    if (result == er_ok)
    {
#if !defined(NO_LOGGING)
        xlogging_set_log_function(xlogging_logger);
#endif
#if !defined(NO_ZLOG)
        zlog_register("default", zlog_subscription);
    }
    else
    {
        zlog_deinit();
#endif
    }
    return result;
}

//
// Deinit logging system
//
void log_deinit(
    void
)
{
    if (!_log)
        return;
    xlogging_set_log_function(NULL);

#if !defined(NO_ZLOG)
    zlog_unregister("default");
#endif

    log_system_free(_log);

#if !defined(NO_ZLOG)
    zlog_deinit();
#endif
    _log = NULL;
}

