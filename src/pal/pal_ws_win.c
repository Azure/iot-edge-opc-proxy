// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.


#include "util_mem.h"
#include "pal_ws.h"
#include "pal_mt.h"
#include "pal_err.h"
#include "util_string.h"

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
    STRING_HANDLE headers;              // Headers to set on request
    volatile long state;                      // State of the client

    HINTERNET h_session;                           // Session handle
    HINTERNET h_connection;                     // Connection handle
    HINTERNET h_request;                           // Request handle
    HINTERNET h_websocket;              // Upgraded Websocket handle

    pal_wsclient_event_handler_t cb;               // Event callback
    bool can_send;                     // Whether sending is enabled
    uint8_t* cur_send_buffer;           // Must cache during sending
    bool can_recv;                   // Whether receiving is enabled
    uint8_t* cur_recv_buffer;              // Cache during receiving
    void* context;                      // and user callback context
    log_t log;
};

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
    DWORD error
)
{
    char* message = NULL;
    if (error == ERROR_SUCCESS)
        return er_ok;
    else
    {
        FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (char*)&message, 0, NULL);

        log_error(NULL, "Winhttp error code %d: %s",
            error, message ? message : "<unknown>");
        LocalFree(message);
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
    case ERROR_WINHTTP_CONNECTION_ERROR: 
        return er_connecting;
    case ERROR_WINHTTP_RESEND_REQUEST: 
        return er_writing;
    case ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED: 
        return er_permission;
    case ERROR_WINHTTP_INVALID_SERVER_RESPONSE:
        return er_connecting;
    }
    return pal_os_to_prx_error(error);
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
    (void)wsclient, info, status;
#if defined(_MSC_VER)
#define __case(v, fmt, ...) \
    case v: log_info(wsclient->log, #v fmt, __VA_ARGS__); break;
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define __case(v, fmt, ...) \
    case v: log_info(wsclient->log, #v fmt, ##__VA_ARGS__); break;
#else
#define __case(v, fmt, args...) \
    case v: log_info(wsclient->log, #v fmt, ## args); break;
#endif
    switch (status)
    {
    __case(WINHTTP_CALLBACK_STATUS_CONNECTION_CLOSED,
        "");
    __case(WINHTTP_CALLBACK_STATUS_DETECTING_PROXY, 
        "");
    __case(WINHTTP_CALLBACK_STATUS_SECURE_FAILURE,
        "");
    __case(WINHTTP_CALLBACK_STATUS_REQUEST_ERROR, 
        "");
    __case(WINHTTP_CALLBACK_STATUS_CONNECTED_TO_SERVER,
        " %S", (LPWSTR)info);
    __case(WINHTTP_CALLBACK_STATUS_RESOLVING_NAME, 
        " %S", (LPWSTR)info);
    __case(WINHTTP_CALLBACK_STATUS_NAME_RESOLVED, 
        " (%S)", info ? (LPWSTR)info : L"???");
    __case(WINHTTP_CALLBACK_STATUS_CONNECTING_TO_SERVER, 
        " %S", (LPWSTR)info);
    __case(WINHTTP_CALLBACK_STATUS_REQUEST_SENT,
        " %d bytes ...", (int)(*(DWORD*)info));
    __case(WINHTTP_CALLBACK_STATUS_RESPONSE_RECEIVED, 
        " %d bytes ...", (int)(*(DWORD*)info));
    __case(WINHTTP_CALLBACK_STATUS_REDIRECT, 
        " to %S...", (LPWSTR)info);
    __case(WINHTTP_CALLBACK_STATUS_INTERMEDIATE_RESPONSE, 
        " %d...", (int)(*(DWORD*)info));
    __case(WINHTTP_CALLBACK_STATUS_SENDING_REQUEST,
        "...");
    __case(WINHTTP_CALLBACK_STATUS_RECEIVING_RESPONSE,
        "...");
#ifdef LOG_VERBOSE
    __case(WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE, 
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
    __case(WINHTTP_CALLBACK_STATUS_CLOSE_COMPLETE, 
        "");
    __case(WINHTTP_CALLBACK_STATUS_SHUTDOWN_COMPLETE, 
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
    
    result = WinHttpWebSocketReceive(wsclient->h_websocket, 
        wsclient->cur_recv_buffer, (DWORD)length, &read, &type);
    
    result = pal_wsclient_from_winhttp_error(result);
    if (result != er_ok)
    {
        log_error(wsclient->log, "Unexpected receive error %s",
            prx_err_string(result));

        length = 0;
        wsclient->cb(wsclient->context, pal_wsclient_event_end_recv,
            &wsclient->cur_recv_buffer, &length, NULL, result);
        wsclient->cur_recv_buffer = NULL;
    }
}

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
    wsclient->cb(wsclient->context, pal_wsclient_event_end_recv,
        &buffer, &len, type, error);
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

    result = WinHttpWebSocketSend(wsclient->h_websocket,
        pal_wsclient_to_winhttp_buffer_type(type),
        wsclient->cur_send_buffer, (DWORD)length);

    result = pal_wsclient_from_winhttp_error(result);
    if (result != er_ok)
    {
        log_error(wsclient->log, "Unexpected send error %s",
            prx_err_string(result));

        length = 0;
        wsclient->cb(wsclient->context, pal_wsclient_event_end_send,
            &wsclient->cur_send_buffer, &length, NULL, result);
        wsclient->cur_send_buffer = NULL;
    }
}

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
    wsclient->cb(wsclient->context, pal_wsclient_event_end_send,
        &buffer, &len, NULL, error);
}

//
// Free websocket client
//
static void pal_wsclient_free(
    pal_wsclient_t* wsclient
)
{
    dbg_assert_ptr(wsclient);

    wsclient->h_session = NULL;
    dbg_assert(!wsclient->h_request && !wsclient->h_connection &&
        !wsclient->h_websocket, "Leaking open handle. ");
    dbg_assert(!wsclient->cur_send_buffer && !wsclient->cur_recv_buffer,
        "Leaking buffer. Wait for disconnect callback before calling close.");

    if (wsclient->headers)
        STRING_delete(wsclient->headers);

    if (wsclient->host)
        mem_free(wsclient->host);

    if (wsclient->relative_path)
        mem_free(wsclient->relative_path);

    mem_free_type(pal_wsclient_t, wsclient);
}

//
// Close handles and clean up state
//
static void pal_wsclient_close_context(
    pal_wsclient_t* wsclient
)
{
    int32_t result;
    DWORD error;

    dbg_assert_ptr(wsclient);

    if (wsclient->h_websocket)
    {
        if (atomic_bit_clear(wsclient->state, pal_wsclient_connected_bit))
        {
            atomic_bit_set(wsclient->state, pal_wsclient_disconnecting_bit);

            // We are connected, close the web socket, which will clean up the other handles
            error = WinHttpWebSocketClose(wsclient->h_websocket,
                WINHTTP_WEB_SOCKET_ENDPOINT_TERMINATED_CLOSE_STATUS, NULL, 0);
            result = pal_wsclient_from_winhttp_error(error);
            if (result == er_ok)
                return;  // Wait for close complete

            log_error(wsclient->log, "Failed closing websocket with close status (%s).",
                prx_err_string(result));
            atomic_bit_clear(wsclient->state, pal_wsclient_disconnecting_bit);
        }

        if (WinHttpCloseHandle(wsclient->h_websocket))
            return;  // Wait for handle close complete
        {
            error = GetLastError();
            log_error(wsclient->log, "Failed closing websocket handle (%s).",
                prx_err_string(pal_wsclient_from_winhttp_error(error)));
            wsclient->h_websocket = NULL;
        }

        // fall through to close request and connection
    }

    if (wsclient->h_request)
    {
        if (!WinHttpCloseHandle(wsclient->h_request))
        {
            error = GetLastError();
            log_error(wsclient->log, "Failed closing request handle (%s).",
                prx_err_string(pal_wsclient_from_winhttp_error(error)));
            wsclient->h_request = NULL;
        }
        else if (!wsclient->h_connection)
            return; // Wait for request to close

        // fall through to close connection...
    }

    if (wsclient->h_connection)
    {
        if (WinHttpCloseHandle(wsclient->h_connection))
            return;
        else
        {
            error = GetLastError();
            log_error(wsclient->log, "Failed closing connection handle (%s).",
                prx_err_string(pal_wsclient_from_winhttp_error(error)));
            wsclient->h_connection = NULL;
        }
    }

    if (wsclient->state & (1 << pal_wsclient_closing_bit))
    {
        // Close session
        dbg_assert(wsclient->h_session != NULL, "Expected session handle");
        if (!WinHttpCloseHandle(wsclient->h_session))
        {
            error = GetLastError();
            log_error(wsclient->log, "Failed closing session handle (%s).",
                prx_err_string(pal_wsclient_from_winhttp_error(error)));
            pal_wsclient_free(wsclient);
        }
    }
    else if (atomic_bit_clear(wsclient->state, pal_wsclient_disconnected_bit))
    {
        //
        // If connection, socket, and request are closed, return any in 
        // progress buffers and signal disconnect, so caller can free 
        // buffer pools without leaking.  This should be running in 
        // callback context.  However, if all handles are closed, then
        // it is fine to run on caller context.
        //
        pal_wsclient_end_send(wsclient, 0, er_aborted);
        pal_wsclient_end_recv(wsclient, 0, NULL, er_aborted);

        wsclient->cb(wsclient->context, pal_wsclient_event_disconnected,
            NULL, NULL, NULL, er_ok);
    }
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
        dbg_assert(handle == wsclient->h_request, "");
        if (!WinHttpReceiveResponse(handle, NULL))
        {
            result = GetLastError();
            log_error(wsclient->log, "Error WinHttpReceiveResponse %d.", result);
            wsclient->cb(wsclient->context, pal_wsclient_event_connected, 
                NULL, NULL, NULL, pal_wsclient_from_winhttp_error(result));
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
                NULL, NULL, NULL, pal_wsclient_from_winhttp_error(result));
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
                NULL, NULL, NULL, pal_wsclient_from_winhttp_error(result));
            break;
        }

        if (!WinHttpSetOption(
            wsclient->h_websocket, WINHTTP_OPTION_CONTEXT_VALUE, &wsclient, sizeof(wsclient)))
        {
            result = GetLastError();
            log_error(wsclient->log, "Failed attaching context %d.", result);
            (void)WinHttpWebSocketClose(
                wsclient->h_websocket, WINHTTP_WEB_SOCKET_ABORTED_CLOSE_STATUS, NULL, 0);
            wsclient->h_websocket = NULL;
            wsclient->cb(wsclient->context, pal_wsclient_event_connected,
                NULL, NULL, NULL, pal_wsclient_from_winhttp_error(result));
            break;
        }

        // Close request handle now that we have a socket
        (void)WinHttpCloseHandle(handle);
        // 
        // Now we are open for send / receive business,
        // send any pending io and begin receiving
        // 
        atomic_bit_clear(wsclient->state, pal_wsclient_connecting_bit);
        atomic_bit_set(wsclient->state, pal_wsclient_connected_bit);
        wsclient->cb(wsclient->context, pal_wsclient_event_connected, 
            NULL, NULL, NULL, er_ok);
        pal_wsclient_begin_recv(wsclient);
        pal_wsclient_begin_send(wsclient);
        break;
    case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
        if (!wsclient->h_websocket)
            break;
        websocket_status = (WINHTTP_WEB_SOCKET_STATUS *)info;
        type = pal_wsclient_from_winhttp_buffer_type(websocket_status->eBufferType);
        len = (size_t)websocket_status->dwBytesTransferred;
        pal_wsclient_end_recv(wsclient, len, &type, er_ok);
        pal_wsclient_begin_recv(wsclient);
        break;
    case WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE:
        if (!wsclient->h_websocket)
            break;
        websocket_status = (WINHTTP_WEB_SOCKET_STATUS *)info;
        len = (size_t)websocket_status->dwBytesTransferred;
        pal_wsclient_end_send(wsclient, len, er_ok);
        pal_wsclient_begin_send(wsclient);
        break;
    case WINHTTP_CALLBACK_STATUS_CLOSE_COMPLETE:
        // The connection was successfully closed via a call to WinHttpWebSocketClose
        wsclient->can_recv = false;
        wsclient->can_send = false;
        atomic_bit_clear(wsclient->state, pal_wsclient_disconnecting_bit);
        pal_wsclient_close_context(wsclient);
        break;
    case WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING:
        // Check which handle was closed
        /**/ if (handle == wsclient->h_request)
        {
            // Request close as part of upgrade
            wsclient->h_request = NULL;
            break;
        }
        else if (handle == wsclient->h_connection)
            wsclient->h_connection = NULL;
        else if (handle == wsclient->h_websocket)
            wsclient->h_websocket = NULL;
        else if (handle == wsclient->h_session)
        {
            // Destroy was called and h_session is now fully closed
            pal_wsclient_free(wsclient);
            break;
        }
        else
        {
            dbg_assert(0, "Unexpected handle closed");
            break;
        }
        pal_wsclient_close_context(wsclient);
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
            wsclient->cb(wsclient->context, pal_wsclient_event_connected,
                NULL, NULL, NULL, pal_wsclient_from_winhttp_error(result));
        }
        else if (wsclient->state & (1 << pal_wsclient_connected_bit))
        {
            result = pal_wsclient_from_winhttp_error(result);
            pal_wsclient_end_send(wsclient, 0, result);
            pal_wsclient_end_recv(wsclient, 0, NULL, result);
        }
        else if (atomic_bit_clear(
            wsclient->state, pal_wsclient_disconnecting_bit))
        {
            // Failed close, continue disconnecting
            log_error(wsclient->log, "Failed close - continue...");
            pal_wsclient_close_context(wsclient);
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
    if (!wsclient || !key || !value)
        return er_fault;
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
    pal_wsclient_event_handler_t callback,
    void* context,
    pal_wsclient_t** created
)
{
    int32_t result;
    pal_wsclient_t* wsclient;

    if (!host || !path || !created || !callback)
        return er_fault;
    wsclient = mem_zalloc_type(pal_wsclient_t);
    if (!wsclient)
        return er_out_of_memory;
    do
    {
        wsclient->log = log_get("websocket");
        wsclient->context = context;
        wsclient->cb = callback;

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

        wsclient->h_session = WinHttpOpen(NULL,
            WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, NULL, NULL, WINHTTP_FLAG_ASYNC);
        if (!wsclient->h_session)
        {
            result = pal_os_last_error_as_prx_error();
            log_error(wsclient->log, "Error WinHttpOpen %s.",
                prx_err_string(result));
            break;
        }

        if (!WinHttpSetOption(wsclient->h_session, 
            WINHTTP_OPTION_CONTEXT_VALUE, &wsclient, sizeof(wsclient)) ||
            !WinHttpSetTimeouts(wsclient->h_session, 0, 0, 0, 0))
        {
            result = pal_os_last_error_as_prx_error();
            log_error(wsclient->log, "Error WinHttpSetOption to set context %s.",
                prx_err_string(result));
            break;
        }

        if (WINHTTP_INVALID_STATUS_CALLBACK == WinHttpSetStatusCallback(
            wsclient->h_session, pal_wsclient_winhttp_cb, (DWORD)-1, 0))
        {
            result = pal_os_last_error_as_prx_error();
            log_error(wsclient->log, "Error WinHttpSetStatusCallback %s.",
                prx_err_string(result));
            break;
        }

        *created = wsclient;
        return er_ok;
    } 
    while (0);

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

    if (!wsclient)
        return er_fault;
    if (wsclient->h_connection)
        return er_bad_state;
    do
    {
        atomic_bit_set(wsclient->state, pal_wsclient_connecting_bit);

        wsclient->h_connection = WinHttpConnect(
            wsclient->h_session, wsclient->host, (INTERNET_PORT)wsclient->port, 0);
        if (!wsclient->h_connection)
        {
            result = pal_os_last_error_as_prx_error();
            log_error(wsclient->log, "Error WinHttpConnect failed with %s.",
                prx_err_string(result));
            break;
        }

        if (!WinHttpSetOption(wsclient->h_connection,
            WINHTTP_OPTION_CONTEXT_VALUE, &wsclient, sizeof(wsclient)))
        {
            result = pal_os_last_error_as_prx_error();
            log_error(wsclient->log, "Error setting option on connection (%s).",
                prx_err_string(result));
            break;
        }

        wsclient->h_request = WinHttpOpenRequest(wsclient->h_connection, L"GET", 
            wsclient->relative_path, NULL, NULL, NULL, WINHTTP_FLAG_SECURE);
        if (!wsclient->h_request)
        {
            result = pal_os_last_error_as_prx_error();
            log_error(wsclient->log, "Error WinHttpOpenRequest failed with %s.",
                prx_err_string(result));
            break;
        }

        if (!WinHttpSetOption(wsclient->h_request, 
                WINHTTP_OPTION_CONTEXT_VALUE, &wsclient, sizeof(wsclient)) ||
            !WinHttpSetOption(wsclient->h_request, 
                WINHTTP_OPTION_CLIENT_CERT_CONTEXT, NULL, 0) ||
            !WinHttpSetOption(wsclient->h_request, 
                WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET, NULL, 0) ||
            !WinHttpSetTimeouts(wsclient->h_request, 0, 0, 0, 0))
        {
            result = pal_os_last_error_as_prx_error();
            log_error(wsclient->log, "Error setting option on request (%s).",
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
                result = pal_os_last_error_as_prx_error();
                log_error(wsclient->log, "Error adding headers to request (%s).",
                    prx_err_string(result));
                break;
            }
            mem_free(headers);
            headers = NULL;
        }

        if (!WinHttpSendRequest(wsclient->h_request, NULL, 0, NULL, 0, 0, 0))
        {
            result = pal_os_last_error_as_prx_error();
            break;
        }

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
    if (!wsclient)
        return er_fault;
    if (wsclient->can_recv == enable)
        return er_ok;

    wsclient->can_recv = enable;
    pal_wsclient_begin_recv(wsclient);
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
    if (!wsclient)
        return er_fault;
    if (wsclient->can_send == enable)
        return er_ok;

    wsclient->can_send = enable;
    pal_wsclient_begin_send(wsclient);
    return er_ok;
}

//
// Disconnect client
//
int32_t pal_wsclient_disconnect(
    pal_wsclient_t* wsclient
)
{
    if (!wsclient)
        return er_fault;

    atomic_bit_set(wsclient->state, pal_wsclient_disconnected_bit);

    // Return buffers to caller to break dependencies
    pal_wsclient_end_send(wsclient, 0, er_aborted);
    pal_wsclient_end_recv(wsclient, 0, NULL, er_aborted);

    pal_wsclient_close_context(wsclient);
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
    pal_wsclient_close_context(wsclient);
    // Wait for callback to free context
}

//
// Initialize global websocket layer
//
int32_t pal_wsclient_init(
    void
)
{
    // no-op
    return er_ok;
}

//
// Free the global websocket layer
//
void pal_wsclient_deinit(
    void
)
{
    // no-op
}
