// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "pal_ws.h"
#include "pal_mt.h"
#include "pal_err.h"
#include "util_string.h"
#include "prx_sched.h"
#include "prx_config.h"

#include "wchar.h"
#if !defined(UNIT_TEST)
#include "windows.h"
#include "winhttp.h"
#endif

// #define LOG_VERBOSE

// 
// State flags
// 
typedef enum pal_wsclient_flag
{
    pal_wsclient_connecting_bit,
    pal_wsclient_connected_bit,
    pal_wsclient_disconnecting_bit,
    pal_wsclient_disconnected_bit,
    pal_wsclient_closing_bit
}
pal_wsclient_flag_t;

//
// Represents a web socket client instance
//
struct pal_wsclient
{
    LPWSTR host;                                  // Host, port, ...
    INTERNET_PORT port;         
    LPWSTR relative_path;               //... and path to connect to
    bool secure;
    STRING_HANDLE headers;              // Headers to set on request
    volatile long state;                      // State of the client

    HINTERNET h_session;                           // Session handle
    HINTERNET h_connection;                     // Connection handle
    HINTERNET h_request;                           // Request handle
    HINTERNET h_websocket;              // Upgraded Websocket handle
    HINTERNET h_closing;                 // Currently closing handle

    pal_wsclient_event_handler_t cb;               // Event callback
    bool can_send;                     // Whether sending is enabled
    uint8_t* cur_send_buffer;           // Must cache during sending
    bool can_recv;                   // Whether receiving is enabled
    uint8_t* cur_recv_buffer;              // Cache during receiving
    void* context;                      // and user callback context
    
    prx_scheduler_t* scheduler;  // Scheduler to synchronize winhttp
    log_t log;
};

static HMODULE _winhttp = NULL;
static prx_scheduler_t* _scheduler = NULL;

#define PROTOCOL_HEADER "Sec-WebSocket-Protocol"

//
// Makes a wide char clone of passed in utf8 string
//
int32_t pal_string_clone_as_wide_string(
    const char* utf8,
    LPWSTR* clone
)
{
    int32_t string_size;
    dbg_assert_ptr(utf8);
    dbg_assert_ptr(clone);

    string_size = MultiByteToWideChar(
        CP_UTF8, 0, utf8, -1, NULL, 0);
    if (string_size <= 0)
        return er_fatal;
    *clone = (LPWSTR)mem_alloc(string_size * sizeof(wchar_t));
    if (!*clone)
        return er_out_of_memory;
    string_size = MultiByteToWideChar(
        CP_UTF8, 0, utf8, -1, *clone, string_size);
    if (string_size <= 0)
    {
        mem_free(*clone);
        *clone = NULL;
        return er_fatal;
    }
    return er_ok;
}

//
// Get error code for winhttp error
//
static int32_t pal_wsclient_from_winhttp_error(
    pal_wsclient_t* wsclient,
    DWORD error
)
{
    char* message = NULL;
    (void)wsclient;

    if (error == ERROR_SUCCESS)
        return er_ok;
    if (error == ERROR_INVALID_OPERATION)
        return er_aborted;
    if (error == ERROR_WINHTTP_CONNECTION_ERROR)
        return er_closed;
    if (error < WINHTTP_ERROR_BASE || error > WINHTTP_ERROR_LAST)
        return pal_os_to_prx_error(error);

    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS |
        FORMAT_MESSAGE_MAX_WIDTH_MASK,
        _winhttp, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (char*)&message, 0, NULL);
    if (message)
    {
        string_trim_back(message, "\r\n\t ");
        log_error(NULL, "%s (%d)", message, error);
        LocalFree(message);
    }
    else
    {
        log_error(NULL, "Unknown Winhttp error %d.", error);
    }

    switch (error) 
    {
    case ERROR_WINHTTP_OUT_OF_HANDLES: 
        return er_out_of_memory;
    case ERROR_WINHTTP_TIMEOUT: 
        return er_timeout;
    case ERROR_WINHTTP_INTERNAL_ERROR:
        return er_fatal;
    case ERROR_WINHTTP_INVALID_URL: 
        return er_no_address;
    case ERROR_WINHTTP_UNRECOGNIZED_SCHEME: 
        return er_no_address;
    case ERROR_WINHTTP_NAME_NOT_RESOLVED: 
        return er_host_unknown;
    case ERROR_WINHTTP_SHUTDOWN: 
        return er_shutdown;
    case ERROR_WINHTTP_LOGIN_FAILURE: 
        return er_permission;
    case ERROR_WINHTTP_OPERATION_CANCELLED:
        return er_aborted;
    case ERROR_WINHTTP_INCORRECT_HANDLE_TYPE:
        return er_arg;
    case ERROR_WINHTTP_INCORRECT_HANDLE_STATE: 
        return er_bad_state;
    case ERROR_WINHTTP_CANNOT_CONNECT: 
        return er_connecting;
    case ERROR_WINHTTP_RESEND_REQUEST: 
        return er_retry;
    case ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED: 
        return er_permission;
    case ERROR_WINHTTP_INVALID_SERVER_RESPONSE:
        return er_invalid_format;
    case ERROR_WINHTTP_SECURE_FAILURE:
        return er_crypto;
    default:
        dbg_assert(0, "Unknown Winhttp error %d", error);
        return er_unknown;
    }
}

//
// Convert to winhttp buffer type
//
static WINHTTP_WEB_SOCKET_BUFFER_TYPE pal_wsclient_to_winhttp_buffer_type(
    pal_wsclient_buffer_type_t type
)
{
    switch (type)
    {
    case pal_wsclient_buffer_type_binary_fragment:
        return WINHTTP_WEB_SOCKET_BINARY_FRAGMENT_BUFFER_TYPE;
    case pal_wsclient_buffer_type_utf8_msg:
        return WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE;
    case pal_wsclient_buffer_type_utf8_fragment:
        return WINHTTP_WEB_SOCKET_UTF8_FRAGMENT_BUFFER_TYPE;
    case pal_wsclient_buffer_type_binary_msg:
        return WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE;
    default:
        dbg_assert(0, "Unexpected type");
        return WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE;
    }
}

//
// Convert from winhttp to pal buffer type
//
static pal_wsclient_buffer_type_t pal_wsclient_from_winhttp_buffer_type(
    WINHTTP_WEB_SOCKET_BUFFER_TYPE type
)
{
    switch (type)
    {
    case WINHTTP_WEB_SOCKET_BINARY_FRAGMENT_BUFFER_TYPE:
        return pal_wsclient_buffer_type_binary_fragment;
    case WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE:
        return pal_wsclient_buffer_type_utf8_msg;
    case WINHTTP_WEB_SOCKET_UTF8_FRAGMENT_BUFFER_TYPE:
        return pal_wsclient_buffer_type_utf8_fragment;
    case WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE:
        return pal_wsclient_buffer_type_binary_msg;
    default:
        return pal_wsclient_buffer_type_unknown;
    }
}

//
// Log win http callback status
//
static void pal_wsclient_log_winhttp_callback_status(
    pal_wsclient_t* wsclient,
    DWORD status,
    LPVOID info
)
{
    (void)wsclient;
    (void)info;
    (void)status;
#if defined(_MSC_VER)
#define __case(v, fmt, ...) \
    case v: log_trace(wsclient->log, #v fmt, __VA_ARGS__); break;
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define __case(v, fmt, ...) \
    case v: log_trace(wsclient->log, #v fmt, ##__VA_ARGS__); break;
#else
#define __case(v, fmt, args...) \
    case v: log_trace(wsclient->log, #v fmt, ## args); break;
#endif
    switch (status)
    {
    __case(WINHTTP_CALLBACK_STATUS_CONNECTION_CLOSED,
        "");
    __case(WINHTTP_CALLBACK_STATUS_DETECTING_PROXY, 
        "");
    __case(WINHTTP_CALLBACK_STATUS_SECURE_FAILURE,
        "");
    __case(WINHTTP_CALLBACK_STATUS_CONNECTED_TO_SERVER,
        " %S", (LPWSTR)info);
    __case(WINHTTP_CALLBACK_STATUS_NAME_RESOLVED, 
        " (%S)", info ? (LPWSTR)info : L"???");
    __case(WINHTTP_CALLBACK_STATUS_CONNECTING_TO_SERVER, 
        " %S", (LPWSTR)info);
    __case(WINHTTP_CALLBACK_STATUS_REQUEST_SENT,
        " with %d bytes ...", (int)(*(DWORD*)info));
    __case(WINHTTP_CALLBACK_STATUS_RESPONSE_RECEIVED, 
        " with %d bytes ...", (int)(*(DWORD*)info));
    __case(WINHTTP_CALLBACK_STATUS_REDIRECT, 
        " to %S...", (LPWSTR)info);
    __case(WINHTTP_CALLBACK_STATUS_INTERMEDIATE_RESPONSE, 
        " %d...", (int)(*(DWORD*)info));
    __case(WINHTTP_CALLBACK_STATUS_CLOSE_COMPLETE,
        "");
#ifdef LOG_VERBOSE
    __case(WINHTTP_CALLBACK_STATUS_RESOLVING_NAME,
        " %S", (LPWSTR)info);
    __case(WINHTTP_CALLBACK_STATUS_SHUTDOWN_COMPLETE,
        "");
    __case(WINHTTP_CALLBACK_STATUS_SENDING_REQUEST,
        "...");
    __case(WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE,
        "");
    __case(WINHTTP_CALLBACK_STATUS_RECEIVING_RESPONSE,
        "...");
    __case(WINHTTP_CALLBACK_STATUS_REQUEST_ERROR,
        "");
    __case(WINHTTP_CALLBACK_STATUS_CLOSING_CONNECTION, 
        "");
    __case(WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE, 
        "");
    __case(WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE, 
        "");
    __case(WINHTTP_CALLBACK_STATUS_READ_COMPLETE, 
        "");
    __case(WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE, 
        "");
    __case(WINHTTP_CALLBACK_STATUS_HANDLE_CREATED, 
        "");
    __case(WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING, 
        "");
    default:
        log_error(wsclient->log, 
            "WINHTTP_CALLBACK_STATUS_UNKNOWN %x", status);
        break;
#endif
    }
}

//
// Continue to receive with the provided client
//
static void pal_wsclient_begin_recv(
    pal_wsclient_t* wsclient
);

//
// Complete receive with the provided client
//
static void pal_wsclient_end_recv(
    pal_wsclient_t* wsclient,
    size_t len,
    pal_wsclient_buffer_type_t* type,
    int32_t error
)
{
    uint8_t* buffer;
    buffer = (uint8_t*)set_atomic_ptr(wsclient->cur_recv_buffer, NULL);
    if (!buffer)
        return;

    log_debug(wsclient->log, "Websocket %p buffer %p (%zu) received (%s)!", 
        wsclient, buffer, len, prx_err_string(error));

    wsclient->cb(wsclient->context, pal_wsclient_event_end_recv,
        &buffer, &len, type, error);

    if (error == er_ok)
    {
        // Schedule next receive
        __do_next(wsclient, pal_wsclient_begin_recv);
    }
    else
    {
        wsclient->can_recv = false;
    }
}

//
// Start to receive with the provided client
//
static void pal_wsclient_begin_recv(
    pal_wsclient_t* wsclient
)
{
    int32_t result;
    DWORD read;
    size_t length;
    WINHTTP_WEB_SOCKET_BUFFER_TYPE type;
    pal_wsclient_buffer_type_t unknown = pal_wsclient_buffer_type_unknown;

    if (!wsclient->can_recv || !wsclient->h_websocket)
        return;
    if (wsclient->cur_recv_buffer)
        return; // recv in progress, should not be here...

    // Get a buffer to receive into from upper layer
    wsclient->cb(wsclient->context, pal_wsclient_event_begin_recv,
        &wsclient->cur_recv_buffer, &length, NULL, er_ok);
    if (!wsclient->cur_recv_buffer)
    {
        wsclient->can_recv = false;
        return;
    }

    log_debug(wsclient->log, "Websocket %p receiving into buffer %p (%zu)...", 
        wsclient, wsclient->cur_recv_buffer, length);

    result = WinHttpWebSocketReceive(wsclient->h_websocket, 
        wsclient->cur_recv_buffer, (DWORD)length, &read, &type);
    
    result = pal_wsclient_from_winhttp_error(wsclient, result);
    if (result != er_ok)
    {
        if (result != er_aborted && result != er_closed)
        {
            log_error(wsclient->log, "Error %s receiving %d bytes (%p)",
                prx_err_string(result), length, wsclient->cur_recv_buffer);
        }
        pal_wsclient_end_recv(wsclient, 0, &unknown, result);
    }
}

//
// Continue to send with the provided client
//
static void pal_wsclient_begin_send(
    pal_wsclient_t* wsclient
);

//
// Complete send with the provided client
//
static void pal_wsclient_end_send(
    pal_wsclient_t* wsclient,
    size_t len,
    int32_t error
)
{
    uint8_t* buffer;
    buffer = (uint8_t*)set_atomic_ptr(wsclient->cur_send_buffer, NULL);
    if (!buffer)
        return;

    log_debug(wsclient->log, "Websocket %p buffer %p (%zu) sent (%s)!",
        wsclient, buffer, len, prx_err_string(error));

    wsclient->cb(wsclient->context, pal_wsclient_event_end_send,
        &buffer, &len, NULL, error);

    if (error == er_ok)
    {
        // Schedule next send
        __do_next(wsclient, pal_wsclient_begin_send);
    }
    else
    {
        wsclient->can_send = false;
    }
}

//
// Start to send with the provided client
//
static void pal_wsclient_begin_send(
    pal_wsclient_t* wsclient
)
{
    int32_t result;
    size_t length;
    pal_wsclient_buffer_type_t type;

    if (!wsclient->can_send || !wsclient->h_websocket)
        return;
    if (wsclient->cur_send_buffer)
        return; // send in progress, should not be here...

    // Get a buffer to send from upper layer
    wsclient->cb(wsclient->context, pal_wsclient_event_begin_send,
        &wsclient->cur_send_buffer, &length, &type, er_ok);
    if (!wsclient->cur_send_buffer)
    {
        wsclient->can_send = false;
        return;
    }

    log_debug(wsclient->log, "Websocket %p sending buffer %p (%zu)...", 
        wsclient, wsclient->cur_send_buffer, length);

    result = WinHttpWebSocketSend(wsclient->h_websocket,
        pal_wsclient_to_winhttp_buffer_type(type),
        wsclient->cur_send_buffer, (DWORD)length);

    result = pal_wsclient_from_winhttp_error(wsclient, result);
    if (result != er_ok)
    {
        if (result != er_aborted && result != er_closed)
        {
            log_error(wsclient->log, "Error %s sending %d bytes (%p)",
                prx_err_string(result), length, wsclient->cur_send_buffer);
        }
        pal_wsclient_end_send(wsclient, 0, result);
    }
}

//
// Free websocket client
//
static void pal_wsclient_free(
    pal_wsclient_t* wsclient
)
{
    dbg_assert_ptr(wsclient);

    dbg_assert(!wsclient->h_request && !wsclient->h_connection &&
        !wsclient->h_session && !wsclient->h_websocket, "Leaking open handle. ");

    if (wsclient->cb)
    {
        pal_wsclient_end_send(wsclient, 0, er_aborted);
        pal_wsclient_end_recv(wsclient, 0, NULL, er_aborted);

        log_debug(wsclient->log, "Websocket client destroying... (%p [%p]).",
            wsclient, wsclient->context);
        wsclient->cb(wsclient->context, pal_wsclient_event_closed,
            NULL, NULL, NULL, er_ok);
        wsclient->cb = NULL;
        wsclient->context = NULL;
    }

    if (wsclient->scheduler)
        prx_scheduler_release(wsclient->scheduler, wsclient);

    if (wsclient->headers)
        STRING_delete(wsclient->headers);

    if (wsclient->host)
        mem_free(wsclient->host);

    if (wsclient->relative_path)
        mem_free(wsclient->relative_path);

    log_debug(wsclient->log, "Websocket client destroyed (%p [%p]).",
        wsclient, wsclient->context);

    mem_free_type(pal_wsclient_t, wsclient);
}

//
// Close handles in the correct order from the scheduler thread.
//
static void pal_wsclient_close_handle(
    pal_wsclient_t* wsclient
)
{
    int32_t result;
    DWORD error;
    dbg_assert_ptr(wsclient);
    do
    {
        if (wsclient->h_closing)
            return;  // Still waiting to close a handle

        if (wsclient->h_websocket)
        {
            if (atomic_bit_clear(wsclient->state, pal_wsclient_connected_bit))
            {
                atomic_bit_set(wsclient->state, pal_wsclient_disconnecting_bit);

                // We are connected, close the web socket
                log_debug(wsclient->log, "Websocket closing... (%p [%p], h:%p)",
                    wsclient, wsclient->context, wsclient->h_websocket);

                error = WinHttpWebSocketClose(wsclient->h_websocket,
                    WINHTTP_WEB_SOCKET_ENDPOINT_TERMINATED_CLOSE_STATUS, NULL, 0);
                result = pal_wsclient_from_winhttp_error(wsclient, error);
                if (result == er_ok)
                    return;  // Wait for close complete - do not touch wsclient ptr
                if (result != er_aborted && result != er_closed)
                {
                    log_error(wsclient->log, "Failed closing websocket with close status (%s).",
                        prx_err_string(result));
                }
                atomic_bit_clear(wsclient->state, pal_wsclient_disconnecting_bit);

                // Continue...
            }
            wsclient->h_closing = wsclient->h_websocket;
            wsclient->h_websocket = NULL;
            break;
        }

        if (wsclient->h_request)
        {
            wsclient->h_closing = wsclient->h_request;
            wsclient->h_request = NULL;
            break;
        }

        if (wsclient->h_connection)
        {
            wsclient->h_closing = wsclient->h_connection;
            wsclient->h_connection = NULL;
            break;
        }

        if (wsclient->state & (1 << pal_wsclient_closing_bit))
        {
            if (wsclient->h_session)
            {
                wsclient->h_closing = wsclient->h_session;
                wsclient->h_session = NULL;
                break;
            }

            // Complete closing by freeing client handle
            __do_next(wsclient, pal_wsclient_free);
            return;
        }
        
        if (atomic_bit_clear(wsclient->state, pal_wsclient_disconnected_bit))
        {
            log_trace(wsclient->log, "Websocket successfully disconnected! (%p [%p])",
                wsclient, wsclient->context);
            //
            // If connection, socket, and request are closed, return any in 
            // progress buffers and signal disconnect, so caller can free 
            // buffer pools without leaking before reconnecting.  
            //
            pal_wsclient_end_send(wsclient, 0, er_aborted);
            pal_wsclient_end_recv(wsclient, 0, NULL, er_aborted);

            wsclient->cb(wsclient->context, pal_wsclient_event_disconnected,
                NULL, NULL, NULL, er_ok);
        }
        return;
    }
    while (0);

    log_debug(wsclient->log, "Closing handle ... (%p [%p], h:%p)",
        wsclient, wsclient->context, wsclient->h_closing);

    // Close the selected handle and wait for it to complete
    if (WinHttpCloseHandle(wsclient->h_closing))
        return;  // Wait for handle close complete - do not touch wsclient ptr
    
    log_error(wsclient->log, "Failed closing handle h:%p (%s).", wsclient->h_closing,
        prx_err_string(pal_wsclient_from_winhttp_error(wsclient, GetLastError())));
    wsclient->h_closing = NULL;
    __do_next(wsclient, pal_wsclient_close_handle);
}

//
// Internal winhttp callback for connection and request
//
static void CALLBACK pal_wsclient_winhttp_cb(
    HINTERNET handle,
    DWORD_PTR context,
    DWORD status,
    LPVOID info,
    DWORD info_size
)
{
    pal_wsclient_t* wsclient = (pal_wsclient_t*)context;
    DWORD result, status_code;
    DWORD size = sizeof(status_code);
    pal_wsclient_buffer_type_t type;
    size_t len = 0;
    WINHTTP_WEB_SOCKET_STATUS* websocket_status;

    (void)info_size;
    if (!wsclient)
        return;
    pal_wsclient_log_winhttp_callback_status(wsclient, status, info);
    switch (status)
    {
    case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
        if (handle != wsclient->h_request)
        {
            if (!wsclient->h_request)
                log_trace(wsclient->log, "Request complete but handle was closed!");
            else
            {
                log_error(wsclient->log, "Unexpected error: Bad handle passed.");
                wsclient->cb(wsclient->context, pal_wsclient_event_connected,
                    NULL, NULL, NULL, er_arg);
            }
            break;
        }
        if (!WinHttpReceiveResponse(handle, NULL))
        {
            result = GetLastError();
            log_error(wsclient->log, "Error WinHttpReceiveResponse %d.", result);
            wsclient->cb(wsclient->context, pal_wsclient_event_connected, 
                NULL, NULL, NULL, pal_wsclient_from_winhttp_error(wsclient, result));
        }
        break;
    case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:
        if (handle != wsclient->h_request)
        {
            log_error(wsclient->log, "Unexpected error: Bad handle passed.");
            wsclient->cb(wsclient->context, pal_wsclient_event_connected, 
                NULL, NULL, NULL, er_arg);
            break;
        }
        if (!WinHttpQueryHeaders(
            handle, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            NULL, &status_code, &size, NULL))
        {
            result = GetLastError();
            log_error(wsclient->log, "Error WinHttpQueryHeaders %d.", result);
            wsclient->cb(wsclient->context, pal_wsclient_event_connected,
                NULL, NULL, NULL, pal_wsclient_from_winhttp_error(wsclient, result));
            break;
        }
        if (status_code != HTTP_STATUS_SWITCH_PROTOCOLS)
        {
            log_error(wsclient->log, "Error switching protocols.");
            wsclient->cb(wsclient->context, pal_wsclient_event_connected,
                NULL, NULL, NULL, er_refused);
            break;
        }

        dbg_assert(wsclient->h_websocket == NULL, "wsclient->h_websocket != NULL");

        /* Complete the upgrade to web socket */
        wsclient->h_websocket = WinHttpWebSocketCompleteUpgrade(handle, 0);
        if (wsclient->h_websocket == NULL)
        {
            result = GetLastError();
            log_error(wsclient->log, "Failed upgrading %d.", result);
            wsclient->cb(wsclient->context, pal_wsclient_event_connected,
                NULL, NULL, NULL, pal_wsclient_from_winhttp_error(wsclient, result));
            break;
        }

        log_trace(wsclient->log, "Connection upgraded to websocket (%p)!", wsclient);

        if (!WinHttpSetOption(
            wsclient->h_websocket, WINHTTP_OPTION_CONTEXT_VALUE, &wsclient, sizeof(wsclient)))
        {
            result = GetLastError();
            log_error(wsclient->log, "Failed attaching context %d.", result);
            (void)WinHttpWebSocketClose(
                wsclient->h_websocket, WINHTTP_WEB_SOCKET_ABORTED_CLOSE_STATUS, NULL, 0);
            wsclient->h_websocket = NULL;
            wsclient->cb(wsclient->context, pal_wsclient_event_connected,
                NULL, NULL, NULL, pal_wsclient_from_winhttp_error(wsclient, result));
            break;
        }

        // Close request handle now that we have a socket
        (void)WinHttpCloseHandle(handle);

        atomic_bit_clear(wsclient->state, pal_wsclient_connecting_bit);
        atomic_bit_set(wsclient->state, pal_wsclient_connected_bit);
        wsclient->cb(wsclient->context, pal_wsclient_event_connected, 
            NULL, NULL, NULL, er_ok);

        __do_next(wsclient, pal_wsclient_begin_recv);
        __do_next(wsclient, pal_wsclient_begin_send);
        break;
    case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
        if (!wsclient->h_websocket)
            break;
        websocket_status = (WINHTTP_WEB_SOCKET_STATUS *)info;
        type = pal_wsclient_from_winhttp_buffer_type(websocket_status->eBufferType);
        len = (size_t)websocket_status->dwBytesTransferred;
        pal_wsclient_end_recv(wsclient, len, &type, er_ok);
        break;
    case WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE:
        if (!wsclient->h_websocket)
            break;
        websocket_status = (WINHTTP_WEB_SOCKET_STATUS *)info;
        len = (size_t)websocket_status->dwBytesTransferred;
        pal_wsclient_end_send(wsclient, len, er_ok);
        break;
    case WINHTTP_CALLBACK_STATUS_CLOSE_COMPLETE:
        // The connection was successfully closed via a call to WinHttpWebSocketClose
        atomic_bit_clear(wsclient->state, pal_wsclient_disconnecting_bit);
        __do_next(wsclient, pal_wsclient_close_handle);
        break;
    case WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING:
        // Check which handle was closed
        /**/ if (handle == wsclient->h_closing)
        {
            wsclient->h_closing = NULL;
            __do_next(wsclient, pal_wsclient_close_handle);
        }
        // Close during connect
        else if (handle == wsclient->h_request)
            wsclient->h_request = NULL;
        else
        {
            dbg_assert(0, "Unexpected handle closing.");
        }
        break;
    case WINHTTP_CALLBACK_STATUS_SECURE_FAILURE:
        wsclient->cb(wsclient->context, pal_wsclient_event_connected, 
            NULL, NULL, NULL, er_permission);
        break;
    case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:
        result = ((WINHTTP_ASYNC_RESULT*)info)->dwError;
        if (result == ERROR_WINHTTP_OPERATION_CANCELLED)
            break;
        /**/ if (wsclient->state & (1 << pal_wsclient_connecting_bit))
        {
            result = pal_wsclient_from_winhttp_error(wsclient, result);
            wsclient->cb(wsclient->context, pal_wsclient_event_connected,
                NULL, NULL, NULL, result);
        }
        else if (wsclient->state & (1 << pal_wsclient_connected_bit))
        {
            result = pal_wsclient_from_winhttp_error(wsclient, result);
            pal_wsclient_end_send(wsclient, 0, result);
            pal_wsclient_end_recv(wsclient, 0, NULL, result);
        }
        else if (atomic_bit_clear(wsclient->state, pal_wsclient_disconnecting_bit))
        {
            if (result == ERROR_WINHTTP_TIMEOUT)
            {
                // If remote side has closed, we will time out, that is expected.
                log_info(wsclient->log, "Timeout closing websocket "
                    "with remote side likely disconnected - continue...");
            }
            else if (result != ERROR_WINHTTP_CONNECTION_ERROR)
            {
                log_error(wsclient->log, "Error closing websocket (%s) - continue...",
                    prx_err_string(pal_wsclient_from_winhttp_error(wsclient, result)));
            }
            // Failed close, continue disconnecting
            __do_next(wsclient, pal_wsclient_close_handle);
        }
        break;
    }
}

//
// Add key value to header
//
int32_t pal_wsclient_add_header(
    pal_wsclient_t* wsclient,
    const char* key,
    const char* value
)
{
    chk_arg_fault_return(wsclient);
    chk_arg_fault_return(key);
    chk_arg_fault_return(value);
    if (!strlen(key))
        return er_arg;

    if (!wsclient->headers)
    {
        wsclient->headers = STRING_new();
        if (!wsclient->headers)
            return er_out_of_memory;
    }

    if (0 != STRING_concat(wsclient->headers, key) ||
        0 != STRING_concat(wsclient->headers, ": ") ||
        0 != STRING_concat(wsclient->headers, value) ||
        0 != STRING_concat(wsclient->headers, "\r\n"))
    {
        STRING_delete(wsclient->headers);
        wsclient->headers = NULL;
        return er_out_of_memory;
    }
    return er_ok;
}

//
// Create websocket client - wait for event
//
int32_t pal_wsclient_create(
    const char* protocol_name,
    const char* host,
    uint16_t port,
    const char* path,
    bool secure,
    pal_wsclient_event_handler_t callback,
    void* context,
    pal_wsclient_t** created
)
{
    int32_t result;
    DWORD max_conns;
    BOOL non_blocking;
    const char* value;
    wchar_t* w_value = NULL;
    pal_wsclient_t* wsclient;

    chk_arg_fault_return(created);
    chk_arg_fault_return(host);
    chk_arg_fault_return(path);
    chk_arg_fault_return(callback);

    wsclient = mem_zalloc_type(pal_wsclient_t);
    if (!wsclient)
        return er_out_of_memory;
    do
    {
        wsclient->log = log_get("pal_ws");
        wsclient->secure = secure;

        result = prx_scheduler_create(_scheduler, &wsclient->scheduler);
        if (result != er_ok)
            break;

        if (protocol_name)
        {
            result = pal_wsclient_add_header(wsclient, PROTOCOL_HEADER,
                protocol_name);
            if (result != er_ok)
                break;
        }

        result = pal_string_clone_as_wide_string(
            host, &wsclient->host);
        if (result != er_ok)
            break;
        wsclient->port = port;
        result = pal_string_clone_as_wide_string(
            path, &wsclient->relative_path);
        if (result != er_ok)
            break;

        value = __prx_config_get(prx_config_key_proxy_host, NULL);
        if (!value)
        {
            // No proxy configuration - use automatic proxy
            wsclient->h_session = WinHttpOpen(NULL,
                WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, NULL, NULL, WINHTTP_FLAG_ASYNC);
            if (!wsclient->h_session)
            {
                result = pal_wsclient_from_winhttp_error(wsclient, GetLastError());
                log_error(wsclient->log, "Error WinHttpOpen (automatic proxy) %s.", 
                    prx_err_string(result));
                break;
            }
        }
        else
        {
            result = pal_string_clone_as_wide_string(value, &w_value);
            if (result != er_ok)
                break;
            wsclient->h_session = WinHttpOpen(NULL,
                WINHTTP_ACCESS_TYPE_NAMED_PROXY, w_value, L"<local>", WINHTTP_FLAG_ASYNC);
            if (!wsclient->h_session)
            {
                result = pal_wsclient_from_winhttp_error(wsclient, GetLastError());
                log_error(wsclient->log, "Error WinHttpOpen with proxy %s.", 
                    prx_err_string(result));
                break;
            }
            value = __prx_config_get(prx_config_key_proxy_user, NULL);
            if (value)
            {
                mem_free(w_value);
                w_value = NULL;
                result = pal_string_clone_as_wide_string(value, &w_value);
                if (result != er_ok)
                    break;
                if (!WinHttpSetOption(wsclient->h_session,
                    WINHTTP_OPTION_PROXY_USERNAME, w_value, sizeof(w_value)))
                {
                    result = pal_wsclient_from_winhttp_error(wsclient, GetLastError());
                    break;
                }
            }
            value = __prx_config_get(prx_config_key_proxy_pwd, NULL);
            if (value)
            {
                mem_free(w_value);
                w_value = NULL;
                result = pal_string_clone_as_wide_string(value, &w_value);
                if (result != er_ok)
                    break;
                if (!WinHttpSetOption(wsclient->h_session,
                    WINHTTP_OPTION_PROXY_PASSWORD, w_value, sizeof(w_value)))
                {
                    result = pal_wsclient_from_winhttp_error(wsclient, GetLastError());
                    break;
                }
            }
        }
        max_conns = 1;
        non_blocking = TRUE;
        if (!WinHttpSetOption(wsclient->h_session,
                WINHTTP_OPTION_ASSURED_NON_BLOCKING_CALLBACKS, &non_blocking, sizeof(BOOL)) ||
            !WinHttpSetOption(wsclient->h_session, 
                WINHTTP_OPTION_CONTEXT_VALUE, &wsclient, sizeof(wsclient)) ||
            !WinHttpSetOption(wsclient->h_session,
                WINHTTP_OPTION_MAX_CONNS_PER_SERVER, &max_conns, sizeof(DWORD)) ||
            !WinHttpSetTimeouts(wsclient->h_session, 0, 0, 0, 0))
        {
            result = pal_wsclient_from_winhttp_error(wsclient, GetLastError());
            log_error(wsclient->log, "Error WinHttpSetOption to set context %s.",
                prx_err_string(result));
            break;
        }

        if (WINHTTP_INVALID_STATUS_CALLBACK == WinHttpSetStatusCallback(
            wsclient->h_session, pal_wsclient_winhttp_cb, (DWORD)-1, 0))
        {
            result = pal_wsclient_from_winhttp_error(wsclient, GetLastError());
            log_error(wsclient->log, "Error WinHttpSetStatusCallback %s.",
                prx_err_string(result));
            break;
        }

        wsclient->context = context;
        wsclient->cb = callback;

        log_debug(wsclient->log, "Websocket client created (%p [%p]).",
            wsclient, wsclient->context);

        if (w_value)
            mem_free(w_value);
        *created = wsclient;
        return er_ok;
    } 
    while (0);

    if (w_value)
        mem_free(w_value);

    if (wsclient->h_session)
    {
        (void)WinHttpCloseHandle(wsclient->h_session);
        wsclient->h_session = NULL;
    }

    pal_wsclient_free(wsclient);
    return result;
}

//
// Connect websocket client - wait for event
//
int32_t pal_wsclient_connect(
    pal_wsclient_t* wsclient
    )
{
    int32_t result;
    LPWSTR headers = NULL;
    DWORD max_retries = 10;

    chk_arg_fault_return(wsclient);
    if (wsclient->h_connection)
        return er_bad_state;
    dbg_assert(!wsclient->h_request && !wsclient->h_connection &&
        !wsclient->h_websocket, "Unexpected open handle. Close not completed...");
    do
    {
        atomic_bit_set(wsclient->state, pal_wsclient_connecting_bit);

        wsclient->h_connection = WinHttpConnect(
            wsclient->h_session, wsclient->host, (INTERNET_PORT)wsclient->port, 0);
        if (!wsclient->h_connection)
        {
            result = pal_wsclient_from_winhttp_error(wsclient, GetLastError());
            log_error(wsclient->log, "Error WinHttpConnect failed with %s.",
                prx_err_string(result));
            break;
        }

        if (!WinHttpSetOption(wsclient->h_connection,
                WINHTTP_OPTION_CONTEXT_VALUE, &wsclient, sizeof(wsclient)))
        {
            result = pal_wsclient_from_winhttp_error(wsclient, GetLastError());
            log_error(wsclient->log, "Error setting option on connection (%s).",
                prx_err_string(result));
            break;
        }

        wsclient->h_request = WinHttpOpenRequest(
            wsclient->h_connection, L"GET", wsclient->relative_path, NULL, NULL, 
            NULL, wsclient->secure ? WINHTTP_FLAG_SECURE : 0);
        if (!wsclient->h_request)
        {
            result = pal_wsclient_from_winhttp_error(wsclient, GetLastError());
            log_error(wsclient->log, "Error WinHttpOpenRequest failed with %s.",
                prx_err_string(result));
            break;
        }

        if (!WinHttpSetOption(wsclient->h_request,
            WINHTTP_OPTION_CONTEXT_VALUE, &wsclient, sizeof(wsclient)))
        {
            result = pal_wsclient_from_winhttp_error(wsclient, GetLastError());
            log_error(wsclient->log, "Error setting context option on request (%s).",
                prx_err_string(result));
            break;
        }

        if (wsclient->secure && !WinHttpSetOption(wsclient->h_request,
                WINHTTP_OPTION_CLIENT_CERT_CONTEXT, NULL, 0))
        {
            result = pal_wsclient_from_winhttp_error(wsclient, GetLastError());
            log_error(wsclient->log, "Error setting client cert on request (%s).",
                prx_err_string(result));
            break;
        }

        if (!WinHttpSetOption(wsclient->h_request,
                WINHTTP_OPTION_CONNECT_RETRIES, &max_retries, sizeof(DWORD)) ||
            __analysis_suppress(6387) !WinHttpSetOption(wsclient->h_request,
                WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET, NULL, 0) ||
            !WinHttpSetTimeouts(wsclient->h_request, 0, 0, 0, 0))
        {
            result = pal_wsclient_from_winhttp_error(wsclient, GetLastError());
            log_error(wsclient->log, "Error setting options on request (%s).",
                prx_err_string(result));
            break;
        }

        // Add any headers
        if (wsclient->headers)
        {
            result = pal_string_clone_as_wide_string(
                STRING_c_str(wsclient->headers), &headers);
            if (result != er_ok)
                break;
            if (!WinHttpAddRequestHeaders(wsclient->h_request, headers,
                (DWORD)-1, WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE))
            {
                result = pal_wsclient_from_winhttp_error(wsclient, GetLastError());
                log_error(wsclient->log, "Error adding headers to request (%s).",
                    prx_err_string(result));
                break;
            }
            mem_free(headers);
            headers = NULL;
        }

        if (!WinHttpSendRequest(wsclient->h_request, NULL, 0, NULL, 0, 0, 0))
        {
            result = pal_wsclient_from_winhttp_error(wsclient, GetLastError());
            log_error(wsclient->log, "Error sending request (%s).",
                prx_err_string(result));
            break;
        }

        log_debug(wsclient->log, "Websocket client connected (%p [%p]).",
            wsclient, wsclient->context);
        return er_ok;
    } 
    while (0);

    if (headers)
        mem_free(headers);

    if (wsclient->h_request)
    {
        (void)WinHttpCloseHandle(wsclient->h_request);
        wsclient->h_request = NULL;
    }
    if (wsclient->h_connection)
    {
        (void)WinHttpCloseHandle(wsclient->h_connection);
        wsclient->h_connection = NULL;
    }
    return result;
}

//
// Start to receive with the provided client
//
int32_t pal_wsclient_can_recv(
    pal_wsclient_t* wsclient,
    bool enable
)
{
    chk_arg_fault_return(wsclient);
    if (wsclient->can_recv == enable)
        return er_ok;

    wsclient->can_recv = enable;
    __do_next(wsclient, pal_wsclient_begin_recv);
    return er_ok;
}

//
// Begin send operation submitting buffer
//
int32_t pal_wsclient_can_send(
    pal_wsclient_t* wsclient,
    bool enable
)
{
    chk_arg_fault_return(wsclient);
    if (wsclient->can_send == enable)
        return er_ok;

    wsclient->can_send = enable;
    __do_next(wsclient, pal_wsclient_begin_send);
    return er_ok;
}

//
// Disconnect client
//
int32_t pal_wsclient_disconnect(
    pal_wsclient_t* wsclient
)
{
    chk_arg_fault_return(wsclient);
    atomic_bit_set(wsclient->state, pal_wsclient_disconnected_bit);

    // Return buffers to caller to break dependencies
    pal_wsclient_end_send(wsclient, 0, er_aborted);
    pal_wsclient_end_recv(wsclient, 0, NULL, er_aborted);

    log_debug(wsclient->log, "Websocket client disconnecting... (%p [%p]).",
        wsclient, wsclient->context);

    __do_next(wsclient, pal_wsclient_close_handle);
    return er_ok;
}

//
// Close websocket client
//
void pal_wsclient_close(
    pal_wsclient_t* wsclient
)
{
    if (!wsclient || !wsclient->h_session)
        return;

    atomic_bit_set(wsclient->state, pal_wsclient_closing_bit);

    log_debug(wsclient->log, "Websocket client closing... (%p [%p]).",
        wsclient, wsclient->context);

    __do_next(wsclient, pal_wsclient_close_handle);
    // Wait for callback to free context - do not touch wsclient afterwards...
}

//
// Initialize global websocket layer
//
int32_t pal_wsclient_init(
    void
)
{
    int32_t result;
    if (_scheduler)
        return er_bad_state;
    result = prx_scheduler_create(NULL, &_scheduler);
    if (result != er_ok)
        return result;
    _winhttp = LoadLibraryA("WINHTTP.DLL");
    return er_ok;
}

//
// Free the global websocket layer
//
void pal_wsclient_deinit(
    void
)
{
    if (_scheduler)
    {
        prx_scheduler_release(_scheduler, NULL);
        prx_scheduler_at_exit(_scheduler);
        _scheduler = NULL;
    }

    if (_winhttp)
        (void)FreeLibrary(_winhttp);
    _winhttp = NULL;
}
