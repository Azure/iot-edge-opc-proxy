// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST pal_ws_win
#include "util_ut.h"

//
// 1. Required mocks
//
#include "os.h"
#include "prx_types.h"
#include "prx_err.h"

// winhttp.h
MOCKABLE_FUNCTION(WINAPI, WINHTTP_STATUS_CALLBACK, WinHttpSetStatusCallback, 
    HINTERNET, hInternet, WINHTTP_STATUS_CALLBACK, lpfnInternetCallback, DWORD, dwNotificationFlags, DWORD_PTR, dwReserved);
MOCKABLE_FUNCTION(WINAPI, HINTERNET, WinHttpOpen,
    LPCWSTR, pszAgentW, DWORD, dwAccessType, LPCWSTR, pszProxyW, LPCWSTR, pszProxyBypassW, DWORD, dwFlags );
MOCKABLE_FUNCTION(WINAPI, BOOL, WinHttpCloseHandle, 
    HINTERNET, hInternet);
MOCKABLE_FUNCTION(WINAPI, HINTERNET, WinHttpConnect,
    HINTERNET, hSession, LPCWSTR, pswzServerName, INTERNET_PORT, nServerPort, DWORD, dwReserved);
MOCKABLE_FUNCTION(WINAPI, BOOL, WinHttpSetOption,
    HINTERNET, hInternet, DWORD, dwOption, LPVOID, lpBuffer, DWORD, dwBufferLength);
MOCKABLE_FUNCTION(WINAPI, BOOL, WinHttpSetTimeouts, HINTERNET, hInternet, int, nResolveTimeout,
    int, nConnectTimeout, int, nSendTimeout, int, nReceiveTimeout);
MOCKABLE_FUNCTION(WINAPI, HINTERNET, WinHttpOpenRequest,
    HINTERNET, hConnect, LPCWSTR, pwszVerb, LPCWSTR, pwszObjectName, LPCWSTR, pwszVersion, 
    LPCWSTR, pwszReferrer, LPCWSTR*, ppwszAcceptTypes, DWORD, dwFlags);
MOCKABLE_FUNCTION(WINAPI, BOOL, WinHttpAddRequestHeaders,
    HINTERNET, hRequest, LPCWSTR, lpszHeaders, DWORD, dwHeadersLength, DWORD, dwModifiers);
MOCKABLE_FUNCTION(WINAPI, BOOL, WinHttpSendRequest,
    HINTERNET, hRequest, LPCWSTR, lpszHeaders, DWORD, dwHeadersLength, LPVOID, lpOptional, DWORD, dwOptionalLength,
    DWORD, dwTotalLength, DWORD_PTR, dwContext);
MOCKABLE_FUNCTION(WINAPI, BOOL, WinHttpReceiveResponse,
    HINTERNET, hRequest, LPVOID, lpReserved);
MOCKABLE_FUNCTION(WINAPI, BOOL, WinHttpQueryHeaders,
    HINTERNET, hRequest, DWORD, dwInfoLevel, LPCWSTR, pwszName, LPVOID, lpBuffer, LPDWORD, lpdwBufferLength, LPDWORD, lpdwIndex);
MOCKABLE_FUNCTION(WINAPI, HINTERNET, WinHttpWebSocketCompleteUpgrade,
    HINTERNET, hRequest, DWORD_PTR, pContext);
MOCKABLE_FUNCTION(WINAPI, DWORD, WinHttpWebSocketSend,
    HINTERNET, hWebSocket, WINHTTP_WEB_SOCKET_BUFFER_TYPE, eBufferType, PVOID, pvBuffer, DWORD, dwBufferLength);
MOCKABLE_FUNCTION(WINAPI, DWORD, WinHttpWebSocketReceive,
    HINTERNET, hWebSocket, PVOID, pvBuffer, DWORD, dwBufferLength, DWORD*, pdwBytesRead, WINHTTP_WEB_SOCKET_BUFFER_TYPE*, peBufferType);
MOCKABLE_FUNCTION(WINAPI, DWORD, WinHttpWebSocketClose,
    HINTERNET, hWebSocket, USHORT, usStatus, PVOID, pvReason, DWORD, dwReasonLength);
// stringapiset.h
MOCKABLE_FUNCTION(WINAPI, int, MultiByteToWideChar,
    UINT, CodePage, DWORD, dwFlags, LPCSTR, lpMultiByteStr, int, cbMultiByte, LPWSTR, lpWideCharStr, int, cchWideChar);
// Winbase.h
MOCKABLE_FUNCTION(WINAPI, HMODULE, LoadLibraryA,
    LPCSTR, lpLibFileName);
MOCKABLE_FUNCTION(WINAPI, DWORD, FormatMessageA,
    DWORD, dwFlags, LPCVOID, lpSource, DWORD, dwMessageId, DWORD, dwLanguageId,
    LPSTR, lpBuffer, DWORD, nSize, void**, Arguments);
MOCKABLE_FUNCTION(WINAPI, HLOCAL, LocalFree,
    HLOCAL, hMem);
MOCKABLE_FUNCTION(WINAPI, BOOL, FreeLibrary,
    HMODULE, hLibModule);
MOCKABLE_FUNCTION(WINAPI, DWORD, GetLastError);

//
// 2. Include unit under test
//
#undef ENABLE_MOCKS
#include "pal_ws.h"
#define ENABLE_MOCKS
#include UNIT_C

MOCKABLE_FUNCTION(, void, pal_wsclient_event_handler_mock,
    void*, context, pal_wsclient_event_t, ev, uint8_t**, buffer, size_t*, size, pal_wsclient_buffer_type_t*, type, int32_t, error);


//
// 3. Setup test suite
//
BEGIN_DECLARE_TEST_SUITE()
REGISTER_UMOCK_ALIAS_TYPE(prx_config_key_t, int);
REGISTER_UMOCK_ALIAS_TYPE(pal_wsclient_event_t, int);
REGISTER_UMOCK_ALIAS_TYPE(WINHTTP_WEB_SOCKET_BUFFER_TYPE, int);
REGISTER_UMOCK_ALIAS_TYPE(HINTERNET, void*);
REGISTER_UMOCK_ALIAS_TYPE(WINHTTP_STATUS_CALLBACK, void*);
REGISTER_UMOCK_ALIAS_TYPE(DWORD_PTR, void*);
REGISTER_UMOCK_ALIAS_TYPE(DWORD, unsigned int);
REGISTER_UMOCK_ALIAS_TYPE(LPDWORD, void*);
REGISTER_UMOCK_ALIAS_TYPE(UINT, unsigned int);
REGISTER_UMOCK_ALIAS_TYPE(USHORT, unsigned short);
REGISTER_UMOCK_ALIAS_TYPE(INTERNET_PORT, unsigned short);
REGISTER_UMOCK_ALIAS_TYPE(PVOID, void*);
REGISTER_UMOCK_ALIAS_TYPE(LPVOID, void*);
REGISTER_UMOCK_ALIAS_TYPE(LPCVOID, void*);
REGISTER_UMOCK_ALIAS_TYPE(HLOCAL, void*);
REGISTER_UMOCK_ALIAS_TYPE(LPSTR, char*);
REGISTER_UMOCK_ALIAS_TYPE(LPCSTR, char*);
REGISTER_UMOCK_ALIAS_TYPE(LPCWSTR, void*);
REGISTER_UMOCK_ALIAS_TYPE(LPWSTR, void*);
REGISTER_UMOCK_ALIAS_TYPE(BOOL, bool);
REGISTER_UMOCK_ALIAS_TYPE(STRING_HANDLE, void*);
REGISTER_UMOCK_ALIAS_TYPE(HMODULE, void*);
END_DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
DECLARE_TEST_SETUP()

// 
// Test pal_wsclient_create happy path 
// 
TEST_FUNCTION(pal_win_wsclient_create__success_1)
{
    static wchar_t* k_wide_char_valid = (wchar_t*)0x2354;
    static prx_scheduler_t* k_scheduler_child_valid = (prx_scheduler_t*)0x2323423;
    static const char* k_protocol_name_valid = "rpotadf";
    static const char* k_host_valid = "lkjafsd";
    static const uint16_t k_port_valid = 10;
    static const char* k_path_valid = "/9845/sadkf";
    static void* k_callback_context_valid = (void*)0x2354;
    static STRING_HANDLE k_string_valid = (STRING_HANDLE)0x234;
    static HINTERNET k_session_handle_valid = (HINTERNET)0x234;
    pal_wsclient_t* wsclient_valid;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));
    _scheduler = (prx_scheduler_t*)0x2343;

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(pal_wsclient_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);

    STRICT_EXPECTED_CALL(prx_scheduler_create(_scheduler, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_scheduler(&k_scheduler_child_valid, sizeof(k_scheduler_child_valid))
        .SetReturn(er_ok);

    STRICT_EXPECTED_CALL(STRING_new())
        .SetReturn(k_string_valid);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, "Sec-WebSocket-Protocol"))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, ": "))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, k_protocol_name_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, "\r\n"))
        .SetReturn(0);

    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_host_valid, -1, NULL, 0))
        .SetReturn(10);
    STRICT_EXPECTED_CALL(h_realloc(10 * sizeof(wchar_t), NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)k_wide_char_valid);
    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_host_valid, -1, k_wide_char_valid, 10))
        .SetReturn(10);

    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_path_valid, -1, NULL, 0))
        .SetReturn(10);
    STRICT_EXPECTED_CALL(h_realloc(10 * sizeof(wchar_t), NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)k_wide_char_valid);
    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_path_valid, -1, k_wide_char_valid, 10))
        .SetReturn(10);

    STRICT_EXPECTED_CALL(_prx_config())
        .SetReturn((prx_config_t*)0x1);
    STRICT_EXPECTED_CALL(prx_config_get_string((prx_config_t*)0x1, prx_config_key_proxy_host, NULL))
        .SetReturn(NULL);

    STRICT_EXPECTED_CALL(WinHttpOpen(NULL, WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, NULL, NULL, WINHTTP_FLAG_ASYNC))
        .SetReturn(k_session_handle_valid);
    STRICT_EXPECTED_CALL(WinHttpSetOption(k_session_handle_valid, WINHTTP_OPTION_ASSURED_NON_BLOCKING_CALLBACKS, IGNORED_PTR_ARG, sizeof(BOOL)))
        .IgnoreArgument(3)
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(WinHttpSetOption(k_session_handle_valid, WINHTTP_OPTION_CONTEXT_VALUE, IGNORED_PTR_ARG, sizeof(pal_wsclient_t*)))
        .IgnoreArgument(3)
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(WinHttpSetOption(k_session_handle_valid, WINHTTP_OPTION_MAX_CONNS_PER_SERVER, IGNORED_PTR_ARG, sizeof(DWORD)))
        .IgnoreArgument(3)
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(WinHttpSetTimeouts(k_session_handle_valid, 0, 0, 0, 0))
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(WinHttpSetStatusCallback(k_session_handle_valid, pal_wsclient_winhttp_cb, (DWORD)-1, 0))
        .SetReturn(0);

    // act 
    result = pal_wsclient_create(k_protocol_name_valid, k_host_valid, 
        k_port_valid, k_path_valid, true, pal_wsclient_event_handler_mock, k_callback_context_valid, &wsclient_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_wsclient_create happy path 
// 
TEST_FUNCTION(pal_win_wsclient_create__success_2)
{
    static wchar_t* k_wide_char_valid = (wchar_t*)0x2354;
    static prx_scheduler_t* k_scheduler_child_valid = (prx_scheduler_t*)0x2323423;
    static const char* k_protocol_name_valid = "rpotadf";
    static const char* k_host_valid = "lkjafsd";
    static const uint16_t k_port_valid = 10;
    static const char* k_path_valid = "/9845/sadkf";
    static const char* k_proxy_host = "1234567890:123456";
    static const char* k_proxy_port = "123456";
    static const char* k_proxy_valid = "1234567890:123456";
    static const char* k_proxy_user = "test";
    static const char* k_proxy_password = "1234";
    static void* k_callback_context_valid = (void*)0x2354;
    static STRING_HANDLE k_string_valid = (STRING_HANDLE)0x234;
    static HINTERNET k_session_handle_valid = (HINTERNET)0x234;
    pal_wsclient_t* wsclient_valid;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));
    _scheduler = (prx_scheduler_t*)0x2343;

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(pal_wsclient_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);

    STRICT_EXPECTED_CALL(prx_scheduler_create(_scheduler, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_scheduler(&k_scheduler_child_valid, sizeof(k_scheduler_child_valid))
        .SetReturn(er_ok);

    STRICT_EXPECTED_CALL(STRING_new())
        .SetReturn(k_string_valid);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, "Sec-WebSocket-Protocol"))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, ": "))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, k_protocol_name_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, "\r\n"))
        .SetReturn(0);

    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_host_valid, -1, NULL, 0))
        .SetReturn(10);
    STRICT_EXPECTED_CALL(h_realloc(10 * sizeof(wchar_t), NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)k_wide_char_valid);
    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_host_valid, -1, k_wide_char_valid, 10))
        .SetReturn(10);

    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_path_valid, -1, NULL, 0))
        .SetReturn(10);
    STRICT_EXPECTED_CALL(h_realloc(10 * sizeof(wchar_t), NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)k_wide_char_valid);
    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_path_valid, -1, k_wide_char_valid, 10))
        .SetReturn(10);

    STRICT_EXPECTED_CALL(_prx_config())
        .SetReturn((prx_config_t*)0x1);
    STRICT_EXPECTED_CALL(prx_config_get_string((prx_config_t*)0x1, prx_config_key_proxy_host, NULL))
        .SetReturn(k_proxy_host);

    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_proxy_valid, -1, NULL, 0))
        .SetReturn(10);
    STRICT_EXPECTED_CALL(h_realloc(10 * sizeof(wchar_t), NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)k_wide_char_valid);
    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_proxy_valid, -1, k_wide_char_valid, 10))
        .SetReturn(10);
    STRICT_EXPECTED_CALL(WinHttpOpen(NULL, WINHTTP_ACCESS_TYPE_NAMED_PROXY, k_wide_char_valid, IGNORED_PTR_ARG, WINHTTP_FLAG_ASYNC))
        .ValidateArgumentBuffer(4, L"<local>", 16)
        .SetReturn(k_session_handle_valid);


    STRICT_EXPECTED_CALL(_prx_config())
        .SetReturn((prx_config_t*)0x1);
    STRICT_EXPECTED_CALL(prx_config_get_string((prx_config_t*)0x1, prx_config_key_proxy_user, NULL))
        .SetReturn(k_proxy_user);

    STRICT_EXPECTED_CALL(h_free((void*)k_wide_char_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);
    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_proxy_user, -1, NULL, 0))
        .SetReturn(10);
    STRICT_EXPECTED_CALL(h_realloc(10 * sizeof(wchar_t), NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)k_wide_char_valid);
    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_proxy_user, -1, k_wide_char_valid, 10))
        .SetReturn(10);
    STRICT_EXPECTED_CALL(WinHttpSetOption(k_session_handle_valid, WINHTTP_OPTION_PROXY_USERNAME, IGNORED_PTR_ARG, sizeof(k_wide_char_valid)))
        .IgnoreArgument(3)
        .SetReturn(TRUE);

    STRICT_EXPECTED_CALL(_prx_config())
        .SetReturn((prx_config_t*)0x1);
    STRICT_EXPECTED_CALL(prx_config_get_string((prx_config_t*)0x1, prx_config_key_proxy_pwd, NULL))
        .SetReturn(k_proxy_password);

    STRICT_EXPECTED_CALL(h_free((void*)k_wide_char_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);
    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_proxy_password, -1, NULL, 0))
        .SetReturn(10);
    STRICT_EXPECTED_CALL(h_realloc(10 * sizeof(wchar_t), NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)k_wide_char_valid);
    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_proxy_password, -1, k_wide_char_valid, 10))
        .SetReturn(10);
    STRICT_EXPECTED_CALL(WinHttpSetOption(k_session_handle_valid, WINHTTP_OPTION_PROXY_PASSWORD, k_wide_char_valid, sizeof(k_wide_char_valid)))
        .SetReturn(TRUE);

    STRICT_EXPECTED_CALL(WinHttpSetOption(k_session_handle_valid, WINHTTP_OPTION_ASSURED_NON_BLOCKING_CALLBACKS, IGNORED_PTR_ARG, sizeof(BOOL)))
        .IgnoreArgument(3)
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(WinHttpSetOption(k_session_handle_valid, WINHTTP_OPTION_CONTEXT_VALUE, IGNORED_PTR_ARG, sizeof(pal_wsclient_t*)))
        .IgnoreArgument(3)
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(WinHttpSetOption(k_session_handle_valid, WINHTTP_OPTION_MAX_CONNS_PER_SERVER, IGNORED_PTR_ARG, sizeof(DWORD)))
        .IgnoreArgument(3)
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(WinHttpSetTimeouts(k_session_handle_valid, 0, 0, 0, 0))
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(WinHttpSetStatusCallback(k_session_handle_valid, pal_wsclient_winhttp_cb, (DWORD)-1, 0))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(h_free((void*)k_wide_char_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);


    // act 
    result = pal_wsclient_create(k_protocol_name_valid, k_host_valid,
        k_port_valid, k_path_valid, true, pal_wsclient_event_handler_mock, k_callback_context_valid, &wsclient_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_wsclient_create passing as host argument an invalid const char* value 
// 
TEST_FUNCTION(pal_win_wsclient_create__arg_host_invalid)
{
    static wchar_t* k_wide_char_valid = (wchar_t*)0x2354;
    static const char* k_protocol_name_valid = "rpotadf";
    static const char* k_host_valid = "lkjafsd";
    static const uint16_t k_port_valid = 10;
    static const char* k_path_valid = "/9845/sadkf";
    static void* k_callback_context_valid = (void*)0x2354;
    static HINTERNET k_session_handle_valid = (HINTERNET)0x234;
    pal_wsclient_t* wsclient_valid;
    int32_t result;

    // arrange 

    // act 
    result = pal_wsclient_create(k_protocol_name_valid, NULL,
        k_port_valid, k_path_valid, true, pal_wsclient_event_handler_mock, k_callback_context_valid, &wsclient_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_wsclient_create passing as path argument an invalid const char* value 
// 
TEST_FUNCTION(pal_win_wsclient_create__arg_path_invalid)
{
    static wchar_t* k_wide_char_valid = (wchar_t*)0x2354;
    static const char* k_protocol_name_valid = "rpotadf";
    static const char* k_host_valid = "lkjafsd";
    static const uint16_t k_port_valid = 10;
    static void* k_callback_context_valid = (void*)0x2354;
    static HINTERNET k_session_handle_valid = (HINTERNET)0x234;
    pal_wsclient_t* wsclient_valid;
    int32_t result;

    // arrange 

    // act 
    result = pal_wsclient_create(k_protocol_name_valid, k_host_valid,
        k_port_valid, NULL, true, pal_wsclient_event_handler_mock, k_callback_context_valid, &wsclient_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_wsclient_create passing as callback argument an invalid pal_wsclient_event_handler_t value 
// 
TEST_FUNCTION(pal_win_wsclient_create__arg_callback_invalid)
{
    static wchar_t* k_wide_char_valid = (wchar_t*)0x2354;
    static const char* k_protocol_name_valid = "rpotadf";
    static const char* k_host_valid = "lkjafsd";
    static const uint16_t k_port_valid = 10;
    static const char* k_path_valid = "/9845/sadkf";
    static void* k_callback_context_valid = (void*)0x2354;
    static HINTERNET k_session_handle_valid = (HINTERNET)0x234;
    pal_wsclient_t* wsclient_valid;
    int32_t result;

    // arrange 

    // act 
    result = pal_wsclient_create(k_protocol_name_valid, k_host_valid,
        k_port_valid, k_path_valid, true, NULL, k_callback_context_valid, &wsclient_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_wsclient_create passing as wsclient argument an invalid pal_wsclient_t** value 
// 
TEST_FUNCTION(pal_win_wsclient_create__arg_wsclient_invalid)
{
    static wchar_t* k_wide_char_valid = (wchar_t*)0x2354;
    static const char* k_protocol_name_valid = "rpotadf";
    static const char* k_host_valid = "lkjafsd";
    static const uint16_t k_port_valid = 10;
    static const char* k_path_valid = "/9845/sadkf";
    static void* k_callback_context_valid = (void*)0x2354;
    static HINTERNET k_session_handle_valid = (HINTERNET)0x234;
    int32_t result;

    // arrange 

    // act 
    result = pal_wsclient_create(k_protocol_name_valid, k_host_valid,
        k_port_valid, k_path_valid, true, pal_wsclient_event_handler_mock, k_callback_context_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_wsclient_create passing as protocol_name argument an invalid const char* value 
// 
TEST_FUNCTION(pal_win_wsclient_create__neg_1)
{
    static wchar_t* k_wide_char_valid = (wchar_t*)0x2444;
    static prx_scheduler_t* k_scheduler_child_valid = (prx_scheduler_t*)0x2323423;
    static const char* k_host_valid = "lkjafsd";
    static const uint16_t k_port_valid = 10;
    static const char* k_path_valid = "/9845/sadkf";
    static void* k_callback_context_valid = (void*)0x2354;
    pal_wsclient_t* wsclient_valid;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));
    _scheduler = (prx_scheduler_t*)0x2343;

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(pal_wsclient_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);

    STRICT_EXPECTED_CALL(prx_scheduler_create(_scheduler, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_scheduler(&k_scheduler_child_valid, sizeof(k_scheduler_child_valid))
        .SetReturn(er_ok);

    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_host_valid, -1, NULL, 0))
        .SetReturn(10);
    STRICT_EXPECTED_CALL(h_realloc(10 * sizeof(wchar_t), NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)k_wide_char_valid);
    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_host_valid, -1, k_wide_char_valid, 10))
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(h_free((void*)k_wide_char_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    STRICT_EXPECTED_CALL(prx_scheduler_release(k_scheduler_child_valid, (void*)UT_MEM));

    STRICT_EXPECTED_CALL(h_free((void*)UT_MEM, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    result = pal_wsclient_create(NULL, k_host_valid,
        k_port_valid, k_path_valid, true, pal_wsclient_event_handler_mock, k_callback_context_valid, &wsclient_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fatal, result);
}

// 
// Test pal_wsclient_create unhappy path 
// 
TEST_FUNCTION(pal_win_wsclient_create__neg_2)
{
    static wchar_t* k_wide_char_valid = (wchar_t*)0x2354;
    static prx_scheduler_t* k_scheduler_child_valid = (prx_scheduler_t*)0x2323423;
    static const char* k_protocol_name_valid = "rpotadf";
    static STRING_HANDLE k_string_valid = (STRING_HANDLE)0x234;
    static const char* k_host_valid = "lkjafsd";
    static const uint16_t k_port_valid = 10;
    static const char* k_path_valid = "/9845/sadkf";
    static void* k_callback_context_valid = (void*)0x2354;
    static HINTERNET k_session_handle_valid = (HINTERNET)0x234;
    pal_wsclient_t* wsclient_valid;
    int32_t result;

    _scheduler = NULL;

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(h_realloc(sizeof(pal_wsclient_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM)
        .SetFailReturn(NULL);

    STRICT_EXPECTED_CALL(prx_scheduler_create(NULL, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_scheduler(&k_scheduler_child_valid, sizeof(k_scheduler_child_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);

    STRICT_EXPECTED_CALL(STRING_new())
        .SetReturn(k_string_valid)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, "Sec-WebSocket-Protocol"))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, ": "))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, k_protocol_name_valid))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, "\r\n"))
        .SetReturn(0)
        .SetFailReturn(-1);

    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_host_valid, -1, NULL, 0))
        .SetReturn(10)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(h_realloc(10 * sizeof(wchar_t), NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)k_wide_char_valid)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_host_valid, -1, k_wide_char_valid, 10))
        .SetReturn(10)
        .SetFailReturn(-1);

    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_path_valid, -1, NULL, 0))
        .SetReturn(10)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(h_realloc(10 * sizeof(wchar_t), NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)k_wide_char_valid)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_path_valid, -1, k_wide_char_valid, 10))
        .SetReturn(10)
        .SetFailReturn(-1);

    STRICT_EXPECTED_CALL(_prx_config())
        .SetReturn((prx_config_t*)0x1)
        .SetFailReturn((prx_config_t*)0x1);
    STRICT_EXPECTED_CALL(prx_config_get_string((prx_config_t*)0x1, prx_config_key_proxy_host, NULL))
        .SetReturn(NULL)
        .SetFailReturn(NULL);

    STRICT_EXPECTED_CALL(WinHttpOpen(NULL, WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, NULL, NULL, WINHTTP_FLAG_ASYNC))
        .SetReturn(k_session_handle_valid)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(WinHttpSetOption(k_session_handle_valid, WINHTTP_OPTION_ASSURED_NON_BLOCKING_CALLBACKS, IGNORED_PTR_ARG, sizeof(BOOL)))
        .IgnoreArgument(3)
        .SetReturn(TRUE)
        .SetFailReturn(FALSE);
    STRICT_EXPECTED_CALL(WinHttpSetOption(k_session_handle_valid, WINHTTP_OPTION_CONTEXT_VALUE, IGNORED_PTR_ARG, sizeof(pal_wsclient_t*)))
        .IgnoreArgument(3)
        .SetReturn(TRUE)
        .SetFailReturn(FALSE);
    STRICT_EXPECTED_CALL(WinHttpSetOption(k_session_handle_valid, WINHTTP_OPTION_MAX_CONNS_PER_SERVER, IGNORED_PTR_ARG, sizeof(DWORD)))
        .IgnoreArgument(3)
        .SetReturn(TRUE)
        .SetFailReturn(FALSE);
    STRICT_EXPECTED_CALL(WinHttpSetTimeouts(k_session_handle_valid, 0, 0, 0, 0))
        .SetReturn(TRUE)
        .SetFailReturn(FALSE);
    STRICT_EXPECTED_CALL(WinHttpSetStatusCallback(k_session_handle_valid, pal_wsclient_winhttp_cb, (DWORD)-1, 0))
        .SetReturn(0)
        .SetFailReturn(WINHTTP_INVALID_STATUS_CALLBACK);

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    memset(UT_MEM, 0, sizeof(UT_MEM));
    result = pal_wsclient_create(k_protocol_name_valid, k_host_valid,
        k_port_valid, k_path_valid, true, pal_wsclient_event_handler_mock, k_callback_context_valid, &wsclient_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result,
        er_out_of_memory, er_out_of_memory, er_out_of_memory, er_out_of_memory, er_out_of_memory,
        er_out_of_memory, er_out_of_memory, er_fatal,         er_out_of_memory, er_fatal,
        er_fatal,         er_out_of_memory, er_fatal,         er_ok,            er_ok,
        er_ok); // TODO: need exploratory umock
}

// 
// Test pal_wsclient_add_header happy path 
// 
TEST_FUNCTION(pal_win_wsclient_add_header__success_1)
{
    static STRING_HANDLE k_string_valid = (STRING_HANDLE)0x2423;
    static const char* k_key_valid = "key";
    static const char* k_value_valid = "value";
    pal_wsclient_t wsclient_valid;
    int32_t result;

    wsclient_valid.headers = NULL;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_new())
        .SetReturn(k_string_valid);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, k_key_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, ": "))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, k_value_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, "\r\n"))
        .SetReturn(0);

    // act 
    result = pal_wsclient_add_header(&wsclient_valid, k_key_valid, k_value_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_wsclient_add_header happy path 
// 
TEST_FUNCTION(pal_win_wsclient_add_header__success_2)
{
    static STRING_HANDLE k_string_valid = (STRING_HANDLE)0x2423;
    static const char* k_key_valid = "key";
    static const char* k_value_valid = "";
    pal_wsclient_t wsclient_valid;
    int32_t result;

    wsclient_valid.headers = k_string_valid;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, k_key_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, ": "))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, k_value_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, "\r\n"))
        .SetReturn(0);

    // act 
    result = pal_wsclient_add_header(&wsclient_valid, k_key_valid, k_value_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_wsclient_add_header passing as wsclient argument an invalid pal_wsclient_t* value 
// 
TEST_FUNCTION(pal_win_wsclient_add_header__arg_wsclient_invalid)
{
    static const char* k_key_valid = "key";
    static const char* k_value_valid = "value";
    int32_t result;

    // arrange 

    // act 
    result = pal_wsclient_add_header(NULL, k_key_valid, k_value_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_wsclient_add_header passing as key argument an invalid const char* value 
// 
TEST_FUNCTION(pal_win_wsclient_add_header__arg_key_invalid_1)
{
    static pal_wsclient_t* k_wsclient_valid = (pal_wsclient_t*)0x21343;
    static const char* k_value_valid = "value";
    int32_t result;

    // arrange 

    // act 
    result = pal_wsclient_add_header(k_wsclient_valid, NULL, k_value_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_wsclient_add_header passing as key argument an invalid const char* value 
// 
TEST_FUNCTION(pal_win_wsclient_add_header__arg_key_invalid_2)
{
    static pal_wsclient_t* k_wsclient_valid = (pal_wsclient_t*)0x21343;
    static const char* k_key_valid = "";
    static const char* k_value_valid = "value";
    int32_t result;

    // arrange 

    // act 
    result = pal_wsclient_add_header(k_wsclient_valid, k_key_valid, k_value_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_arg, result);
}

// 
// Test pal_wsclient_add_header passing as value argument an invalid const char* value 
// 
TEST_FUNCTION(pal_win_wsclient_add_header__arg_value_invalid)
{
    static pal_wsclient_t* k_wsclient_valid = (pal_wsclient_t*)0x21343;
    static const char* k_key_valid = "key";
    int32_t result;

    // arrange 

    // act 
    result = pal_wsclient_add_header(k_wsclient_valid, k_key_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_wsclient_add_header unhappy path 
// 
TEST_FUNCTION(pal_win_wsclient_add_header__neg)
{
    static STRING_HANDLE k_string_valid = (STRING_HANDLE)0x2423;
    static const char* k_key_valid = "key";
    static const char* k_value_valid = "value";
    pal_wsclient_t wsclient_valid;
    int32_t result;

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(STRING_new())
        .SetReturn(k_string_valid)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, k_key_valid))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, ": "))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, k_value_valid))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, "\r\n"))
        .SetReturn(0)
        .SetFailReturn(-1);

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    wsclient_valid.headers = NULL;
    result = pal_wsclient_add_header(&wsclient_valid, k_key_valid, k_value_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_out_of_memory);
}

// 
// Test pal_wsclient_connect happy path 
// 
TEST_FUNCTION(pal_win_wsclient_connect__success)
{
    static HINTERNET k_h_request_valid = (HINTERNET)0x666;
    static HINTERNET k_h_session_valid = (HINTERNET)0x2345;
    static HINTERNET k_h_connection_valid = (HINTERNET)0x235;
    static wchar_t* k_wide_char_valid = (wchar_t*)0x2354;
    static const char* k_headers_ptr = "headers";
    STRING_HANDLE k_headers_valid = (STRING_HANDLE)0x11;
    pal_wsclient_t wsclient_valid;
    int32_t result;

    wsclient_valid.h_session = k_h_session_valid;
    wsclient_valid.h_connection = NULL;
    wsclient_valid.h_request = NULL;
    wsclient_valid.h_websocket = NULL;
    wsclient_valid.host = L"test";
    wsclient_valid.state = 0;
    wsclient_valid.relative_path = L"foo";
    wsclient_valid.headers = k_headers_valid;
    wsclient_valid.port = 10;
    wsclient_valid.secure = true;

    // arrange 
    STRICT_EXPECTED_CALL(WinHttpConnect(k_h_session_valid, IGNORED_PTR_ARG, 10, 0))
        .ValidateArgumentBuffer(2, L"test", 10)
        .SetReturn(k_h_connection_valid);
    STRICT_EXPECTED_CALL(WinHttpSetOption(k_h_connection_valid, WINHTTP_OPTION_CONTEXT_VALUE, IGNORED_PTR_ARG, sizeof(pal_wsclient_t*)))
        .IgnoreArgument(3)
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(WinHttpOpenRequest(k_h_connection_valid, IGNORED_PTR_ARG, IGNORED_PTR_ARG, NULL, NULL, NULL, WINHTTP_FLAG_SECURE))
        .ValidateArgumentBuffer(2, L"GET", 8)
        .ValidateArgumentBuffer(3, L"foo", 8)
        .SetReturn(k_h_request_valid);
    STRICT_EXPECTED_CALL(WinHttpSetOption(k_h_request_valid, WINHTTP_OPTION_CONTEXT_VALUE, IGNORED_PTR_ARG, sizeof(pal_wsclient_t*)))
        .IgnoreArgument(3)
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(WinHttpSetOption(k_h_request_valid, WINHTTP_OPTION_CLIENT_CERT_CONTEXT, NULL, 0))
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(WinHttpSetOption(k_h_request_valid, WINHTTP_OPTION_CONNECT_RETRIES, NULL, sizeof(DWORD)))
        .IgnoreArgument(3)
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(WinHttpSetOption(k_h_request_valid, WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET, NULL, 0))
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(WinHttpSetTimeouts(k_h_request_valid, 0, 0, 0, 0))
        .SetReturn(TRUE);

    STRICT_EXPECTED_CALL(STRING_c_str(k_headers_valid))
        .SetReturn(k_headers_ptr);
    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_headers_ptr, -1, NULL, 0))
        .SetReturn(10);
    STRICT_EXPECTED_CALL(h_realloc(10 * sizeof(wchar_t), NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)k_wide_char_valid);
    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_headers_ptr, -1, k_wide_char_valid, 10))
        .SetReturn(10);

    STRICT_EXPECTED_CALL(WinHttpAddRequestHeaders(k_h_request_valid, k_wide_char_valid, (DWORD)-1, WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE))
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(h_free((void*)k_wide_char_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    STRICT_EXPECTED_CALL(WinHttpSendRequest(k_h_request_valid, NULL, 0, NULL, 0, 0, 0))
        .SetReturn(TRUE);

    // act 
    result = pal_wsclient_connect(&wsclient_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_wsclient_connect passing as wsclient argument an invalid pal_wsclient_t* value 
// 
TEST_FUNCTION(pal_win_wsclient_connect__arg_wsclient_null)
{
    int32_t result;

    // arrange 

    // act 
    result = pal_wsclient_connect(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_wsclient_connect unhappy path 
// 
TEST_FUNCTION(pal_win_wsclient_connect__neg_1)
{
    static HINTERNET k_h_request_valid = (HINTERNET)0x666;
    static HINTERNET k_h_session_valid = (HINTERNET)0x2345;
    static HINTERNET k_h_connection_valid = (HINTERNET)0x235;
    pal_wsclient_t wsclient_valid;
    int32_t result;

    memset(&wsclient_valid, 0xff, sizeof(wsclient_valid));
    wsclient_valid.h_session = k_h_session_valid;
    wsclient_valid.h_connection = k_h_connection_valid;
    wsclient_valid.h_request = NULL;

    // arrange 

    // act 
    result = pal_wsclient_connect(&wsclient_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_bad_state, result);
}

// 
// Test pal_wsclient_connect unhappy path 
// 
TEST_FUNCTION(pal_win_wsclient_connect__neg_2)
{
    static HINTERNET k_h_request_valid = (HINTERNET)0x666;
    static HINTERNET k_h_session_valid = (HINTERNET)0x2345;
    static HINTERNET k_h_connection_valid = (HINTERNET)0x235;
    static wchar_t* k_wide_char_valid = (wchar_t*)0x2354;
    static const char* k_headers_ptr = "headers";
    static const char* k_message = "Error";
    STRING_HANDLE k_headers_valid = (STRING_HANDLE)0x11;
    pal_wsclient_t wsclient_valid;
    int32_t result;

    memset(&wsclient_valid, 0xff, sizeof(wsclient_valid));
    wsclient_valid.h_session = k_h_session_valid;
    wsclient_valid.h_connection = NULL;
    wsclient_valid.h_request = NULL;
    wsclient_valid.host = L"test";
    wsclient_valid.relative_path = L"foo";
    wsclient_valid.headers = k_headers_valid;
    wsclient_valid.port = 10;
    wsclient_valid.secure = true;

    // arrange 
    STRICT_EXPECTED_CALL(WinHttpConnect(k_h_session_valid, IGNORED_PTR_ARG, 10, 0))
        .ValidateArgumentBuffer(2, L"test", 10)
        .SetReturn(k_h_connection_valid);
    STRICT_EXPECTED_CALL(WinHttpSetOption(k_h_connection_valid, WINHTTP_OPTION_CONTEXT_VALUE, IGNORED_PTR_ARG, sizeof(pal_wsclient_t*)))
        .IgnoreArgument(3)
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(WinHttpOpenRequest(k_h_connection_valid, IGNORED_PTR_ARG, IGNORED_PTR_ARG, NULL, NULL, NULL, WINHTTP_FLAG_SECURE))
        .ValidateArgumentBuffer(2, L"GET", 8)
        .ValidateArgumentBuffer(3, L"foo", 8)
        .SetReturn(k_h_request_valid);
    STRICT_EXPECTED_CALL(WinHttpSetOption(k_h_request_valid, WINHTTP_OPTION_CONTEXT_VALUE, IGNORED_PTR_ARG, sizeof(pal_wsclient_t*)))
        .IgnoreArgument(3)
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(WinHttpSetOption(k_h_request_valid, WINHTTP_OPTION_CLIENT_CERT_CONTEXT, NULL, 0))
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(WinHttpSetOption(k_h_request_valid, WINHTTP_OPTION_CONNECT_RETRIES, NULL, sizeof(DWORD)))
        .IgnoreArgument(3)
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(WinHttpSetOption(k_h_request_valid, WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET, NULL, 0))
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(WinHttpSetTimeouts(k_h_request_valid, 0, 0, 0, 0))
        .SetReturn(TRUE);

    STRICT_EXPECTED_CALL(STRING_c_str(k_headers_valid))
        .SetReturn(k_headers_ptr);
    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_headers_ptr, -1, NULL, 0))
        .SetReturn(10);
    STRICT_EXPECTED_CALL(h_realloc(10 * sizeof(wchar_t), NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)k_wide_char_valid);
    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_headers_ptr, -1, k_wide_char_valid, 10))
        .SetReturn(10);
    STRICT_EXPECTED_CALL(WinHttpAddRequestHeaders(k_h_request_valid, k_wide_char_valid, (DWORD)-1, WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE))
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(h_free((void*)k_wide_char_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    STRICT_EXPECTED_CALL(WinHttpSendRequest(k_h_request_valid, NULL, 0, NULL, 0, 0, 0))
        .SetReturn(FALSE);
    STRICT_EXPECTED_CALL(GetLastError())
        .SetReturn(ERROR_WINHTTP_INTERNAL_ERROR);
    STRICT_EXPECTED_CALL(FormatMessageA(IGNORED_NUM_ARG, IGNORED_PTR_ARG, ERROR_WINHTTP_INTERNAL_ERROR, IGNORED_NUM_ARG, IGNORED_PTR_ARG, 0, NULL))
        .IgnoreArgument(1).IgnoreArgument(2).IgnoreArgument(4)
        .CopyOutArgumentBuffer_lpBuffer(&k_message, sizeof(const char*))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(string_trim_back((char*)k_message, "\r\n\t "));
    STRICT_EXPECTED_CALL(LocalFree((HLOCAL)k_message))
        .SetReturn((HLOCAL)NULL);

    STRICT_EXPECTED_CALL(WinHttpCloseHandle(k_h_request_valid))
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(WinHttpCloseHandle(k_h_connection_valid))
        .SetReturn(TRUE);

    // act 
    result = pal_wsclient_connect(&wsclient_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fatal, result);
    ASSERT_IS_NULL(wsclient_valid.h_connection);
    ASSERT_IS_NULL(wsclient_valid.h_request);
}

// 
// Test pal_wsclient_can_recv happy path 
// 
TEST_FUNCTION(pal_win_wsclient_can_recv__success_0)
{
    pal_wsclient_t wsclient_valid;
    int32_t result;

    wsclient_valid.can_recv = false;

    // arrange 

    // act 
    result = pal_wsclient_can_recv(&wsclient_valid, false);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_wsclient_can_recv happy path 
// 
TEST_FUNCTION(pal_win_wsclient_can_recv__success_1)
{
    static uint8_t *k_buffer_ptr = (uint8_t*)0x23423;
    static const size_t k_length_valid = 100;
    static HINTERNET k_h_wsclient_valid = (HINTERNET)0x56789;
    pal_wsclient_t wsclient_valid;
    int32_t result;

    wsclient_valid.h_websocket = k_h_wsclient_valid;
    wsclient_valid.can_recv = false;
    wsclient_valid.cur_recv_buffer = NULL;
    wsclient_valid.cb = pal_wsclient_event_handler_mock;
    wsclient_valid.context = (void*)0x1;

    // arrange 
    STRICT_EXPECTED_CALL(pal_wsclient_event_handler_mock((void*)0x1, pal_wsclient_event_begin_recv, IGNORED_PTR_ARG, IGNORED_PTR_ARG, NULL, er_ok))
        .CopyOutArgumentBuffer_buffer(&k_buffer_ptr, sizeof(k_buffer_ptr))
        .CopyOutArgumentBuffer_size(&k_length_valid, sizeof(k_length_valid));
    STRICT_EXPECTED_CALL(WinHttpWebSocketReceive(k_h_wsclient_valid, k_buffer_ptr, (DWORD)k_length_valid, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(4).IgnoreArgument(5)
        .SetReturn(ERROR_SUCCESS);

    // act 
    result = pal_wsclient_can_recv(&wsclient_valid, true);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_wsclient_can_recv happy path 
// 
TEST_FUNCTION(pal_win_wsclient_can_recv__success_2)
{
    pal_wsclient_t wsclient_valid;
    int32_t result;

    // arrange 
    wsclient_valid.can_recv = true;

    // act 
    result = pal_wsclient_can_recv(&wsclient_valid, false);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_wsclient_can_recv passing as wsclient argument an invalid pal_wsclient_t* value 
// 
TEST_FUNCTION(pal_win_wsclient_can_recv__arg_wsclient_invalid)
{
    static uint8_t k_buffer_valid[100] = { 1,2,3,4,5,6,7 };
    static const size_t k_length_valid = sizeof(k_buffer_valid);
    int32_t result;

    // arrange 

    // act 
    result = pal_wsclient_can_recv(NULL, true);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_wsclient_can_recv unhappy path 
// 
TEST_FUNCTION(pal_win_wsclient_can_recv__neg)
{
    static uint8_t *k_buffer_ptr = (uint8_t*)0x23423;
    static const size_t k_length_valid = 100;
    static HINTERNET k_h_wsclient_valid = (HINTERNET)0x56789;
    static const char* k_message = "Error";
    pal_wsclient_t wsclient_valid;
    int32_t result;

    wsclient_valid.h_websocket = k_h_wsclient_valid;
    wsclient_valid.can_recv = false;
    wsclient_valid.cur_recv_buffer = NULL;
    wsclient_valid.cb = pal_wsclient_event_handler_mock;
    wsclient_valid.context = (void*)0x1;

    // arrange 
    STRICT_EXPECTED_CALL(pal_wsclient_event_handler_mock((void*)0x1, pal_wsclient_event_begin_recv, IGNORED_PTR_ARG, IGNORED_PTR_ARG, NULL, er_ok))
        .CopyOutArgumentBuffer_buffer(&k_buffer_ptr, sizeof(k_buffer_ptr))
        .CopyOutArgumentBuffer_size(&k_length_valid, sizeof(k_length_valid));
    STRICT_EXPECTED_CALL(WinHttpWebSocketReceive(k_h_wsclient_valid, k_buffer_ptr, (DWORD)k_length_valid, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(4).IgnoreArgument(5)
        .SetReturn(ERROR_WINHTTP_OPERATION_CANCELLED);
    STRICT_EXPECTED_CALL(FormatMessageA(IGNORED_NUM_ARG, IGNORED_PTR_ARG, ERROR_WINHTTP_OPERATION_CANCELLED, IGNORED_NUM_ARG, IGNORED_PTR_ARG, 0, NULL))
        .IgnoreArgument(1).IgnoreArgument(2).IgnoreArgument(4)
        .CopyOutArgumentBuffer_lpBuffer(&k_message, sizeof(const char*))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(string_trim_back((char*)k_message, "\r\n\t "));
    STRICT_EXPECTED_CALL(LocalFree((HLOCAL)k_message))
        .SetReturn((HLOCAL)NULL);
    STRICT_EXPECTED_CALL(pal_wsclient_event_handler_mock((void*)0x1, pal_wsclient_event_end_recv, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, er_aborted))
        .ValidateArgumentBuffer(3, &k_buffer_ptr, sizeof(k_buffer_ptr))
        .IgnoreArgument(4).IgnoreArgument(5);

    // act 
    result = pal_wsclient_can_recv(&wsclient_valid, true);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_wsclient_can_send happy path 
// 
TEST_FUNCTION(pal_win_wsclient_can_send__success_0)
{
    pal_wsclient_t wsclient_valid;
    int32_t result;

    wsclient_valid.can_send = false;

    // arrange 

    // act 
    result = pal_wsclient_can_send(&wsclient_valid, false);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_wsclient_can_send happy path 
// 
TEST_FUNCTION(pal_win_wsclient_can_send__success_1)
{
    static uint8_t *k_buffer_ptr = (uint8_t*)0x23423;
    static const size_t k_length_valid = 100;
    static HINTERNET k_h_wsclient_valid = (HINTERNET)0x56789;
    static pal_wsclient_buffer_type_t k_type_valid = pal_wsclient_buffer_type_binary_msg;
    pal_wsclient_t wsclient_valid;
    int32_t result;

    wsclient_valid.h_websocket = k_h_wsclient_valid;
    wsclient_valid.can_send = false;
    wsclient_valid.cur_send_buffer = NULL;
    wsclient_valid.cb = pal_wsclient_event_handler_mock;
    wsclient_valid.context = (void*)0x1;

    // arrange 
    STRICT_EXPECTED_CALL(pal_wsclient_event_handler_mock((void*)0x1, pal_wsclient_event_begin_send, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, er_ok))
        .CopyOutArgumentBuffer_buffer(&k_buffer_ptr, sizeof(k_buffer_ptr))
        .CopyOutArgumentBuffer_size(&k_length_valid, sizeof(k_length_valid))
        .CopyOutArgumentBuffer_type(&k_type_valid, sizeof(k_type_valid));
    STRICT_EXPECTED_CALL(WinHttpWebSocketSend(k_h_wsclient_valid, WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE, k_buffer_ptr, (DWORD)k_length_valid))
        .SetReturn(ERROR_SUCCESS);

    // act 
    result = pal_wsclient_can_send(&wsclient_valid, true);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_wsclient_can_send happy path 
// 
TEST_FUNCTION(pal_win_wsclient_can_send__success_2)
{
    pal_wsclient_t wsclient_valid;
    int32_t result;

    wsclient_valid.can_send = true;

    // arrange 

    // act 
    result = pal_wsclient_can_send(&wsclient_valid, false);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_wsclient_can_send passing as wsclient argument an invalid pal_wsclient_t* value 
// 
TEST_FUNCTION(pal_win_wsclient_can_send__arg_wsclient_invalid)
{
    int32_t result;

    // arrange 

    // act 
    result = pal_wsclient_can_send(NULL, false);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_wsclient_can_send unhappy path 
// 
TEST_FUNCTION(pal_win_wsclient_can_send__neg)
{
    static uint8_t *k_buffer_ptr = (uint8_t*)0x23423;
    static const size_t k_length_valid = 100;
    static HINTERNET k_h_wsclient_valid = (HINTERNET)0x56789;
    static pal_wsclient_buffer_type_t k_type_valid = pal_wsclient_buffer_type_binary_msg;
    static const char* k_message = "Error";
    pal_wsclient_t wsclient_valid;
    int32_t result;

    wsclient_valid.h_websocket = k_h_wsclient_valid;
    wsclient_valid.can_send = false;
    wsclient_valid.cur_send_buffer = NULL;
    wsclient_valid.cb = pal_wsclient_event_handler_mock;
    wsclient_valid.context = (void*)0x1;

    // arrange 
    STRICT_EXPECTED_CALL(pal_wsclient_event_handler_mock((void*)0x1, pal_wsclient_event_begin_send, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, er_ok))
        .CopyOutArgumentBuffer_buffer(&k_buffer_ptr, sizeof(k_buffer_ptr))
        .CopyOutArgumentBuffer_size(&k_length_valid, sizeof(k_length_valid))
        .CopyOutArgumentBuffer_type(&k_type_valid, sizeof(k_type_valid));
    STRICT_EXPECTED_CALL(WinHttpWebSocketSend(k_h_wsclient_valid, WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE, k_buffer_ptr, (DWORD)k_length_valid))
        .SetReturn(ERROR_WINHTTP_OPERATION_CANCELLED);
    STRICT_EXPECTED_CALL(FormatMessageA(IGNORED_NUM_ARG, IGNORED_PTR_ARG, ERROR_WINHTTP_OPERATION_CANCELLED, IGNORED_NUM_ARG, IGNORED_PTR_ARG, 0, NULL))
        .IgnoreArgument(1).IgnoreArgument(2).IgnoreArgument(4)
        .CopyOutArgumentBuffer_lpBuffer(&k_message, sizeof(const char*))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(string_trim_back((char*)k_message, "\r\n\t "));
    STRICT_EXPECTED_CALL(LocalFree((HLOCAL)k_message))
        .SetReturn((HLOCAL)NULL);
    STRICT_EXPECTED_CALL(pal_wsclient_event_handler_mock((void*)0x1, pal_wsclient_event_end_send, IGNORED_PTR_ARG, IGNORED_PTR_ARG, NULL, er_aborted))
        .ValidateArgumentBuffer(3, &k_buffer_ptr, sizeof(k_buffer_ptr))
        .IgnoreArgument(4);

    // act 
    result = pal_wsclient_can_send(&wsclient_valid, true);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_wsclient_disconnect happy path 
// 
TEST_FUNCTION(pal_win_wsclient_disconnect__success_1)
{
    pal_wsclient_t wsclient_valid;
    int32_t result;

    wsclient_valid.h_request = NULL;
    wsclient_valid.h_connection = NULL;
    wsclient_valid.h_websocket = NULL;
    wsclient_valid.h_closing = (HINTERNET)-1;
    wsclient_valid.cur_send_buffer = NULL;
    wsclient_valid.cur_recv_buffer = NULL;
    wsclient_valid.cb = pal_wsclient_event_handler_mock;
    wsclient_valid.context = (void*)0x1;
    wsclient_valid.state = 0;

    // arrange 

    // act 
    result = pal_wsclient_disconnect(&wsclient_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_wsclient_disconnect happy path 
// 
TEST_FUNCTION(pal_win_wsclient_disconnect__success_2)
{
    pal_wsclient_t wsclient_valid;
    int32_t result;

    wsclient_valid.h_request = NULL;
    wsclient_valid.h_connection = NULL;
    wsclient_valid.h_websocket = NULL;
    wsclient_valid.h_closing = NULL;
    wsclient_valid.cur_send_buffer = NULL;
    wsclient_valid.cur_recv_buffer = NULL;
    wsclient_valid.cb = pal_wsclient_event_handler_mock;
    wsclient_valid.context = (void*)0x1;
    wsclient_valid.state = 0;

    // arrange 
    STRICT_EXPECTED_CALL(pal_wsclient_event_handler_mock((void*)0x1, pal_wsclient_event_disconnected, NULL, NULL, NULL, er_ok));

    // act 
    result = pal_wsclient_disconnect(&wsclient_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_wsclient_disconnect happy path 
// 
TEST_FUNCTION(pal_win_wsclient_disconnect__success_3)
{
    static HINTERNET k_h_wsclient_valid = (HINTERNET)0x56789;
    pal_wsclient_t wsclient_valid;
    int32_t result;

    wsclient_valid.h_websocket = k_h_wsclient_valid;
    wsclient_valid.h_closing = NULL;
    wsclient_valid.cur_send_buffer = (uint8_t*)0x2;
    wsclient_valid.cur_recv_buffer = (uint8_t*)0x3;
    wsclient_valid.cb = pal_wsclient_event_handler_mock;
    wsclient_valid.context = (void*)0x1;
    wsclient_valid.state = (1 << pal_wsclient_connected_bit);

    // arrange 
    STRICT_EXPECTED_CALL(pal_wsclient_event_handler_mock((void*)0x1, pal_wsclient_event_end_send, IGNORED_PTR_ARG, IGNORED_PTR_ARG, NULL, er_aborted))
        .IgnoreArgument(3).IgnoreArgument(4);
    STRICT_EXPECTED_CALL(pal_wsclient_event_handler_mock((void*)0x1, pal_wsclient_event_end_recv, IGNORED_PTR_ARG, IGNORED_PTR_ARG, NULL, er_aborted))
        .IgnoreArgument(3).IgnoreArgument(4);
    STRICT_EXPECTED_CALL(WinHttpWebSocketClose(k_h_wsclient_valid, WINHTTP_WEB_SOCKET_ENDPOINT_TERMINATED_CLOSE_STATUS, NULL, 0))
        .SetReturn(ERROR_SUCCESS);

    // act 
    result = pal_wsclient_disconnect(&wsclient_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_wsclient_disconnect happy path 
// 
TEST_FUNCTION(pal_win_wsclient_disconnect__success_4)
{
    static HINTERNET k_h_wsclient_valid = (HINTERNET)0x56789;
    pal_wsclient_t wsclient_valid;
    int32_t result;

    wsclient_valid.h_websocket = k_h_wsclient_valid;
    wsclient_valid.h_closing = NULL;
    wsclient_valid.cur_send_buffer = (uint8_t*)0x2;
    wsclient_valid.cur_recv_buffer = (uint8_t*)0x3;
    wsclient_valid.cb = pal_wsclient_event_handler_mock;
    wsclient_valid.context = (void*)0x1;
    wsclient_valid.state = 0;

    // arrange 
    STRICT_EXPECTED_CALL(pal_wsclient_event_handler_mock((void*)0x1, pal_wsclient_event_end_send, IGNORED_PTR_ARG, IGNORED_PTR_ARG, NULL, er_aborted))
        .IgnoreArgument(3).IgnoreArgument(4);
    STRICT_EXPECTED_CALL(pal_wsclient_event_handler_mock((void*)0x1, pal_wsclient_event_end_recv, IGNORED_PTR_ARG, IGNORED_PTR_ARG, NULL, er_aborted))
        .IgnoreArgument(3).IgnoreArgument(4);
    STRICT_EXPECTED_CALL(WinHttpCloseHandle(k_h_wsclient_valid))
        .SetReturn(TRUE);

    // act 
    result = pal_wsclient_disconnect(&wsclient_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_wsclient_disconnect happy path 
// 
TEST_FUNCTION(pal_win_wsclient_disconnect__success_5)
{
    static HINTERNET k_h_request_valid = (HINTERNET)0x666;
    static HINTERNET k_h_connection_valid = (HINTERNET)0x235;
    pal_wsclient_t wsclient_valid;
    int32_t result;

    wsclient_valid.h_request = k_h_request_valid;
    wsclient_valid.h_connection = k_h_connection_valid;
    wsclient_valid.h_closing = NULL;
    wsclient_valid.h_websocket = NULL;
    wsclient_valid.cur_send_buffer = NULL;
    wsclient_valid.cur_recv_buffer = NULL;
    wsclient_valid.cb = pal_wsclient_event_handler_mock;
    wsclient_valid.context = (void*)0x1;
    wsclient_valid.state = 0;

    // arrange 
    STRICT_EXPECTED_CALL(WinHttpCloseHandle(k_h_request_valid))
        .SetReturn(FALSE);
    STRICT_EXPECTED_CALL(WinHttpCloseHandle(k_h_connection_valid))
        .SetReturn(FALSE);
    STRICT_EXPECTED_CALL(pal_wsclient_event_handler_mock((void*)0x1, pal_wsclient_event_disconnected, NULL, NULL, NULL, er_ok));

    // act 
    result = pal_wsclient_disconnect(&wsclient_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_wsclient_disconnect passing as wsclient argument an invalid pal_wsclient_t* value 
// 
TEST_FUNCTION(pal_win_wsclient_disconnect__arg_wsclient_null)
{
    int32_t result;

    // arrange 

    // act 
    result = pal_wsclient_disconnect(NULL);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_wsclient_close happy path 
// 
TEST_FUNCTION(pal_win_wsclient_close__success_1)
{
    static HINTERNET k_h_session_valid = (HINTERNET)0x666;
    pal_wsclient_t wsclient_valid;

    memset(&wsclient_valid, 0, sizeof(wsclient_valid));
    wsclient_valid.h_session = k_h_session_valid;
    wsclient_valid.host = (wchar_t*)0x2;
    wsclient_valid.relative_path = (wchar_t*)0x3;
    wsclient_valid.headers = (STRING_HANDLE)0x4;
    wsclient_valid.port = 10;

    // arrange 
    STRICT_EXPECTED_CALL(WinHttpCloseHandle(k_h_session_valid))
        .SetReturn(TRUE);

    // act 
    pal_wsclient_close(&wsclient_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_wsclient_close happy path 
// 
TEST_FUNCTION(pal_win_wsclient_close__success_2)
{
    static HINTERNET k_h_session_valid = (HINTERNET)0x666;
    static const char* k_message = "Error";
    pal_wsclient_t wsclient_valid;

    memset(&wsclient_valid, 0, sizeof(wsclient_valid));
    wsclient_valid.h_session = k_h_session_valid;
    wsclient_valid.host = (wchar_t*)0x2;
    wsclient_valid.relative_path = (wchar_t*)0x3;
    wsclient_valid.headers = (STRING_HANDLE)0x4;
    wsclient_valid.port = 10;

    // arrange 
    STRICT_EXPECTED_CALL(WinHttpCloseHandle(k_h_session_valid))
        .SetReturn(FALSE);
    STRICT_EXPECTED_CALL(STRING_delete((STRING_HANDLE)0x4));
    STRICT_EXPECTED_CALL(h_free((void*)0x2, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);
    STRICT_EXPECTED_CALL(h_free((void*)0x3, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);
    STRICT_EXPECTED_CALL(h_free((void*)&wsclient_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    pal_wsclient_close(&wsclient_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_wsclient_close happy path 
// 
TEST_FUNCTION(pal_win_wsclient_close__success_3)
{
    pal_wsclient_t wsclient_valid;

    memset(&wsclient_valid, 0, sizeof(wsclient_valid));
    wsclient_valid.port = 10;

    // arrange 

    // act 
    pal_wsclient_close(&wsclient_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_wsclient_close passing as wsclient argument an invalid pal_wsclient_t* value 
// 
TEST_FUNCTION(pal_win_wsclient_close__arg_wsclient_invalid)
{
    // arrange 

    // act 
    pal_wsclient_close(NULL);

    // assert
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_wsclient_deinit happy path 
// 
TEST_FUNCTION(pal_win_wsclient_deinit__success_1)
{
    static HMODULE k_valid_dll = (HMODULE)0x1234;
    static prx_scheduler_t* k_valid_scheduler = (prx_scheduler_t*)0x4321;
    _scheduler = k_valid_scheduler;
    _winhttp = k_valid_dll;

    // arrange 
    STRICT_EXPECTED_CALL(prx_scheduler_release(k_valid_scheduler, NULL));
    STRICT_EXPECTED_CALL(prx_scheduler_at_exit(k_valid_scheduler));
    STRICT_EXPECTED_CALL(FreeLibrary(k_valid_dll))
        .SetReturn(TRUE);

    // act 
    pal_wsclient_deinit();

    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_TRUE(_winhttp == NULL);
    ASSERT_IS_TRUE(_scheduler == NULL);
}

// 
// Test pal_wsclient_deinit happy path 
// 
TEST_FUNCTION(pal_win_wsclient_deinit__success_2)
{
    static HMODULE k_valid_dll = (HMODULE)0x1234;
    _scheduler = NULL;
    _winhttp = k_valid_dll;

    // arrange 
    STRICT_EXPECTED_CALL(FreeLibrary(k_valid_dll))
        .SetReturn(TRUE);

    // act 
    pal_wsclient_deinit();

    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_TRUE(_winhttp == NULL);
    ASSERT_IS_TRUE(_scheduler == NULL);
}

// 
// Test pal_wsclient_init happy path 
// 
TEST_FUNCTION(pal_win_wsclient_init__success)
{
    static HMODULE k_valid_dll = (HMODULE)0x1234;
    static prx_scheduler_t* k_valid_scheduler = (prx_scheduler_t*)0x4321;
    int32_t result;

    _scheduler = NULL;
    _winhttp = NULL;

    // arrange 
    STRICT_EXPECTED_CALL(prx_scheduler_create(NULL, &_scheduler))
        .CopyOutArgumentBuffer_scheduler(&k_valid_scheduler, sizeof(k_valid_scheduler))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(LoadLibraryA("WINHTTP.DLL"))
        .SetReturn(k_valid_dll);

    // act 
    result = pal_wsclient_init();

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_IS_TRUE(_winhttp == k_valid_dll);
    ASSERT_IS_TRUE(_scheduler == k_valid_scheduler);
}

// 
// Test pal_wsclient_init unhappy path 
// 
TEST_FUNCTION(pal_win_wsclient_init__neg)
{
    static HMODULE k_valid_dll = (HMODULE)0x1234;
    int32_t result;

    _scheduler = NULL;
    _winhttp = NULL;

    // arrange 
    STRICT_EXPECTED_CALL(prx_scheduler_create(NULL, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .SetReturn(er_out_of_memory);

    // act 
    result = pal_wsclient_init();

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_out_of_memory, result);
    ASSERT_IS_TRUE(_scheduler == NULL);
    ASSERT_IS_TRUE(_winhttp == NULL);
}

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

