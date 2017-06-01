
#include "util_mem.h"
#include "util_zlog.h"
#include "util_misc.h"
#include <zlog.h>
#include <category.h>

static int32_t init_called = 0;

#define ZLOG_DEFAULT_CONFIG \
    "[global]\n" \
    "strict init = true\n" \
    "buffer min = 1024\n" \
    "file perms = 644\n" \
    "buffer max = 0\n" \
    "rotate lock file = zlog.lock\n" \
    "default format = \"[Pid=%p:Tid=%t %d(%T).%ms] %c %V %m [%U:%L]%n\"\n" \
    "[rules]\n" \
    "*.notice $default\n" \
    "*.info >stdout\n"

//
// Initialize zlog library
//
int32_t zlog_initialize(
    void
)
{
    // 
    // If confpath is NULL, it looks for the environment variable
    // ZLOG_CONF_PATH to find the configuration file. If $ZLOG_CONF_PATH 
    // is NULL also, all logs will be output to stdout with an internal 
    // format. 
    //
    if (init_called++)
        return er_ok;
    if (0 != dzlog_init(NULL, "root"))
        return er_fatal;
    zlog_reload_from_string(ZLOG_DEFAULT_CONFIG);
    return er_ok;
}

//
// Set log file
//
int32_t zlog_set_log_file(
    const char* file_name
)
{
    int32_t result;
    char* config;
    static const char* pre_config = ZLOG_DEFAULT_CONFIG "*.!debug \"";
    static const char* size_config = "\", 10MB * 3 ~ \"";
    static const char* post_config = ".#r\"\n";

    config = (char*)malloc(
        strlen(pre_config) + strlen(file_name) + strlen(file_name) +
        strlen(post_config) + strlen(size_config) + 1);
    if (!config)
        return er_out_of_memory;
    do
    {
        strcpy(config, pre_config);
        strcat(config, file_name);
        strcat(config, size_config);
        strcat(config, file_name);
        strcat(config, post_config);

        if (0 != zlog_reload_from_string(config))
        {
            result = er_reading;
            break;
        }

        result = er_ok;
        break;
    }
    while (0);
    free(config);
    return result;
}

//
// Configure logging facilities
//
int32_t zlog_read_config(
    const char* file_name
)
{
    if (!init_called)
        zlog_initialize();

    if (0 != zlog_reload(file_name))
        return er_reading;
    return er_ok;
}

//
// Deinit zlog.
//
void zlog_deinit(
    void
)
{
    if (!--init_called)
        zlog_fini();
}

//
// returns a zlog category
//
log_t zlog_get(
    const char* area
)
{
    return (log_t)zlog_get_category(area);
}

//
// Register callback to receive events for a target
//
int32_t zlog_register(
    const char* target,
    zlog_cb_t callback
)
{
    if (0 != zlog_set_record(target, (zlog_record_fn)callback))
        return er_bad_state;
    return er_ok;
}

//
// Unregister callback for target
//
int32_t zlog_unregister(
    const char* target
)
{
   // if (0 != zlog_set_record(target, NULL))
   //     return er_bad_state;
    return er_ok;
}

//
// Log debug message implementation
//
void __zlog_debug_v(
    log_t log,
    const char *file,
    size_t filelen,
    const char *func,
    size_t funclen,
    long line,
    const char* format,
    va_list args
)
{
    if (!log)
        vdzlog(file, filelen, 
            func, funclen, line, ZLOG_LEVEL_DEBUG, format, args);
    else
        vzlog((zlog_category_t*)log, file, filelen, 
            func, funclen, line, ZLOG_LEVEL_DEBUG, format, args);
}

//
// Log trace message implementation
//
void __zlog_trace_v(
    log_t log,
    const char *file,
    size_t filelen,
    const char *func,
    size_t funclen,
    long line,
    const char* format,
    va_list args
)
{
    if (!log)
        vdzlog(file, filelen, 
            func, funclen, line, ZLOG_LEVEL_INFO, format, args);
    else
        vzlog((zlog_category_t*)log, file, filelen, 
            func, funclen, line, ZLOG_LEVEL_INFO, format, args);
}

//
// Log event message implementation
//
void __zlog_info_v(
    log_t log,
    const char *file,
    size_t filelen,
    const char *func,
    size_t funclen,
    long line,
    const char* format,
    va_list args
)
{
    if (!log)
        vdzlog(file, filelen,
            func, funclen, line, ZLOG_LEVEL_NOTICE, format, args);
    else
        vzlog((zlog_category_t*)log, file, filelen,
            func, funclen, line, ZLOG_LEVEL_NOTICE, format, args);
}

//
// Log error message implementation
//
void __zlog_error_v(
    log_t log,
    const char *file,
    size_t filelen,
    const char *func,
    size_t funclen,
    long line,
    const char* format,
    va_list args
)
{
    if (!log)
        vdzlog(file, filelen, 
            func, funclen, line, ZLOG_LEVEL_ERROR, format, args);
    else
        vzlog((zlog_category_t*)log, file, filelen, 
            func, funclen, line, ZLOG_LEVEL_ERROR, format, args);
}

//
// Log debug message implementation
//
void __zlog_debug_b(
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
    if (!log)
        hdzlog(file, filelen,
            func, funclen, line, ZLOG_LEVEL_DEBUG, buf, buflen);
    else
        hzlog((zlog_category_t*)log, file, filelen,
            func, funclen, line, ZLOG_LEVEL_DEBUG, buf, buflen);
}

//
// Log trace message implementation
//
void __zlog_trace_b(
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
    if (!log)
        hdzlog(file, filelen,
            func, funclen, line, ZLOG_LEVEL_INFO, buf, buflen);
    else
        hzlog((zlog_category_t*)log, file, filelen,
            func, funclen, line, ZLOG_LEVEL_INFO, buf, buflen);
}

//
// Log trace message implementation
//
void __zlog_info_b(
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
    if (!log)
        hdzlog(file, filelen,
            func, funclen, line, ZLOG_LEVEL_NOTICE, buf, buflen);
    else
        hzlog((zlog_category_t*)log, file, filelen,
            func, funclen, line, ZLOG_LEVEL_NOTICE, buf, buflen);
}

//
// Log error message implementation
//
void __zlog_error_b(
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
    if (!log)
        hdzlog(file, filelen,
            func, funclen, line, ZLOG_LEVEL_ERROR, buf, buflen);
    else
        hzlog((zlog_category_t*)log, file, filelen,
            func, funclen, line, ZLOG_LEVEL_ERROR, buf, buflen);
}

#define MAX_BUF 900 

//
// Remove line breaks
//
static void remove_line_break(
    char* val
)
{
    const char* trim;
    size_t len = strlen(val);
    if (!len)
        return;
    while (--len > 0)
    {
        trim = "\r\n";
        while (*trim && val[len] != *trim)
            trim++;
        if (!*trim)
            break; // do not trim further
        val[len] = 0;
    }
}

//
// Internal Helper to accumulate a partial log event in mdc
//
static void __zlog_info_a_to_mdc(
    log_t log,
    const char* format,
    va_list args
)
{
    const char* name;
    char msg[512];
    char* buf;
    size_t buf_len, msg_len;
    if (!log)
        return;

    vsnprintf(msg, sizeof(msg) - 1, format, args);
    name = ((zlog_category_t*)log)->name;
    buf = zlog_get_mdc(name);
    if (!buf)
        zlog_put_mdc(name, msg);
    else
    {
        buf_len = strlen(buf);
        msg_len = strlen(msg);
        if (buf_len + msg_len > MAX_BUF)
        {
            msg_len = MAX_BUF - buf_len;
            if (msg_len < 4)
                return;
            msg[msg_len - 3] = '.';
            msg[msg_len - 2] = '.';
            msg[msg_len - 1] = '.';
            msg[msg_len - 0] = 0;
        }
        else
            remove_line_break(msg);
        strcat(buf, msg);
    }
}

//
// Internal helper to flush the mdc
//
void __zlog_info_a_flush_mdc(
    log_t log,
    const char *file,
    size_t filelen,
    const char *func,
    size_t funclen,
    long line,
    const char* format,
    va_list args
)
{
    char* buf;
    zlog_category_t* cat;
    size_t buf_len, msg_len;
    if (!log)
        return;

    cat = (zlog_category_t*)log;
    buf = zlog_get_mdc(cat->name);
    if (buf)
    {
        buf_len = strlen(buf);
        msg_len = strlen(format);
        if (buf_len + msg_len > MAX_BUF)
        {
            buf[MAX_BUF - msg_len - 3] = '.';
            buf[MAX_BUF - msg_len - 2] = '.';
            buf[MAX_BUF - msg_len - 1] = '.';
            buf[MAX_BUF - msg_len - 0] = 0;
        }

        // Append format to buf and log
        strcat(buf, format);
        remove_line_break(buf);

        vzlog(cat, file, filelen,
            func, funclen, line, ZLOG_LEVEL_INFO, buf, args);
        zlog_put_mdc(cat->name, "");
    }
    else
    {
        vzlog(cat, file, filelen,
            func, funclen, line, ZLOG_LEVEL_INFO, format, args);
    }
}

//
// Accumulate a partial log event
//
void __zlog_info_a(
    log_t log,
    const char* format,
    va_list args
)
{
    if (!log || zlog_category_needless_level(((zlog_category_t*)log), ZLOG_LEVEL_INFO))
        return;
    __zlog_info_a_to_mdc(log, format, args);
}

//
// Flush all partial log events and this one
//
void __zlog_info_a_flush(
    log_t log,
    const char *file,
    size_t filelen,
    const char *func,
    size_t funclen,
    long line,
    const char* format,
    va_list args
)
{
    if (!log || zlog_category_needless_level(((zlog_category_t*)log), ZLOG_LEVEL_INFO))
        return;
    __zlog_info_a_flush_mdc(log, file, filelen, func, funclen, line, format, args);
}

#if _WIN32

//
// Log invalid parameter
//
static void invalid_parameter(
    const wchar_t* expression,
    const wchar_t* function,
    const wchar_t* file,
    uint32_t line,
    uintptr_t reserved
)
{
    wprintf(L"Invalid parameter detected in function %s."
        L" File: %s Line: %d\n", function, file, line);
    wprintf(L"Expression: %s\n", expression);
}

//
// Dll Main entry point
//
BOOL __stdcall DllMain(
    HINSTANCE hinstDLL,
    unsigned long reason,
    void* reserved
)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        mem_init();
        _set_invalid_parameter_handler(invalid_parameter);
        break;
    case DLL_PROCESS_DETACH:
        mem_deinit();
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}
#endif