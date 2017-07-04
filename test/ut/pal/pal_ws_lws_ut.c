// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST pal_ws_lws
#include "util_ut.h"

//
// 1. Required mocks
//
#include "os.h"
#include "prx_types.h"

// libwebsockets.h
MOCKABLE_FUNCTION(, int, lws_write, 
    struct lws*, wsi, unsigned char*, buf, size_t, len, enum lws_write_protocol, protocol);
MOCKABLE_FUNCTION(, int, lws_rx_flow_control, 
    struct lws*, wsi, int, enable);
MOCKABLE_FUNCTION(, int, lws_service,
    struct lws_context*, context, int, timeout_ms);
MOCKABLE_FUNCTION(, void, lws_cancel_service,
    struct lws_context*, context);
MOCKABLE_FUNCTION(, void, lws_cancel_service_pt,
    struct lws*, wsi);
MOCKABLE_FUNCTION(, struct lws_context*, lws_create_context,
    struct lws_context_creation_info*, info);
MOCKABLE_FUNCTION(, void, lws_context_destroy, 
    struct lws_context*, context);
MOCKABLE_FUNCTION(, void*, lws_context_user,
    struct lws_context*, context);
MOCKABLE_FUNCTION(, void*, lws_wsi_user,
    struct lws*, wsi);
MOCKABLE_FUNCTION(, const struct lws_protocols*, lws_get_protocol,
    struct lws*, wsi);
MOCKABLE_FUNCTION(, int, lws_callback_on_writable,
    struct lws*, wsi);
MOCKABLE_FUNCTION(, size_t, lws_remaining_packet_payload, 
    struct lws*, wsi);
MOCKABLE_FUNCTION(, int, lws_is_final_fragment, 
    struct lws*, wsi);
MOCKABLE_FUNCTION(, int, lws_frame_is_binary, 
    struct lws*, wsi);
MOCKABLE_FUNCTION(, struct lws*, lws_client_connect_via_info, 
    struct lws_client_connect_info*, ccinfo);
MOCKABLE_FUNCTION(, struct lws_context*, lws_get_context,
    const struct lws*, wsi);
MOCKABLE_FUNCTION(, void, lws_set_log_level,
    int, level, log_emit_function, func);
MOCKABLE_FUNCTION(, void, lws_close_reason, 
    struct lws*, wsi, enum lws_close_status, status, unsigned char*, buf, size_t, len);
// openssl.h
MOCKABLE_FUNCTION(, X509_STORE*, SSL_CTX_get_cert_store, 
    const SSL_CTX*, ctx);
MOCKABLE_FUNCTION(, void, X509_free,
    X509*, a);
MOCKABLE_FUNCTION(, X509*, PEM_read_bio_X509, 
    BIO*, bp, X509**, x, pem_password_cb*, cb, void*, u);
MOCKABLE_FUNCTION(, BIO_METHOD*, BIO_s_mem);
MOCKABLE_FUNCTION(, BIO*, BIO_new, 
    BIO_METHOD*, type);
MOCKABLE_FUNCTION(, int, BIO_free, 
    BIO*, a);
MOCKABLE_FUNCTION(, int, BIO_puts, 
    BIO*, b, const char*, buf);
MOCKABLE_FUNCTION(, int, X509_STORE_add_cert,
    X509_STORE*, store, X509*, certificate);

MOCKABLE_FUNCTION(, const char*, trusted_certs);

//
// 2. Include unit under test
//
#undef ENABLE_MOCKS
#include "pal_ws.h"
#define ENABLE_MOCKS
#include UNIT_C

//
// 3. Setup test suite
//
BEGIN_DECLARE_TEST_SUITE()
REGISTER_UMOCK_ALIAS_TYPE(lock_t, void*);
REGISTER_UMOCK_ALIAS_TYPE(prx_config_key_t, int);
REGISTER_UMOCK_ALIAS_TYPE(enum lws_write_protocol, int);
REGISTER_UMOCK_ALIAS_TYPE(enum lws_close_status, int);
REGISTER_UMOCK_ALIAS_TYPE(log_emit_function, void*);
REGISTER_UMOCK_ALIAS_TYPE(PDLIST_ENTRY, void*);
END_DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
DECLARE_TEST_SETUP()


#if 0


// 
// Test pal_wsclient_create happy path 
// 
TEST_FUNCTION(pal_lws_wsclient_create__success)
{
    static wchar_t* k_wide_char_valid = (wchar_t*)0x2354;
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

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(pal_wsclient_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);

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
    STRICT_EXPECTED_CALL(h_realloc(20, NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)k_wide_char_valid);
    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_host_valid, -1, k_wide_char_valid, 10))
        .SetReturn(10);

    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_path_valid, -1, NULL, 0))
        .SetReturn(10);
    STRICT_EXPECTED_CALL(h_realloc(20, NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)k_wide_char_valid);
    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_path_valid, -1, k_wide_char_valid, 10))
        .SetReturn(10);

    STRICT_EXPECTED_CALL(WinHttpOpen(NULL, WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, NULL, NULL, WINHTTP_FLAG_ASYNC))
        .SetReturn(k_session_handle_valid);
    STRICT_EXPECTED_CALL(WinHttpSetOption(k_session_handle_valid, WINHTTP_OPTION_CONTEXT_VALUE, IGNORED_PTR_ARG, sizeof(pal_wsclient_t*)))
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
// Test pal_wsclient_create passing as host argument an invalid const char* value 
// 
TEST_FUNCTION(pal_lws_wsclient_create__arg_host_invalid)
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
TEST_FUNCTION(pal_lws_wsclient_create__arg_path_invalid)
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
TEST_FUNCTION(pal_lws_wsclient_create__arg_callback_invalid)
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
// Test pal_wsclient_create passing as websocket argument an invalid pal_wsclient_t** value 
// 
TEST_FUNCTION(pal_lws_wsclient_create__arg_websocket_invalid)
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
TEST_FUNCTION(pal_lws_wsclient_create__neg_1)
{
    static wchar_t* k_wide_char_valid = (wchar_t*)0x2444;
    static const char* k_host_valid = "lkjafsd";
    static const uint16_t k_port_valid = 10;
    static const char* k_path_valid = "/9845/sadkf";
    static void* k_callback_context_valid = (void*)0x2354;
    pal_wsclient_t* wsclient_valid;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(pal_wsclient_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);

    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_host_valid, -1, NULL, 0))
        .SetReturn(10);
    STRICT_EXPECTED_CALL(h_realloc(20, NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)k_wide_char_valid);
    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_host_valid, -1, k_wide_char_valid, 10))
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(h_free((void*)k_wide_char_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);


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
TEST_FUNCTION(pal_lws_wsclient_create__neg_2)
{
    static wchar_t* k_wide_char_valid = (wchar_t*)0x2354;
    static const char* k_protocol_name_valid = "rpotadf";
    static STRING_HANDLE k_string_valid = (STRING_HANDLE)0x234;
    static const char* k_host_valid = "lkjafsd";
    static const uint16_t k_port_valid = 10;
    static const char* k_path_valid = "/9845/sadkf";
    static void* k_callback_context_valid = (void*)0x2354;
    static HINTERNET k_session_handle_valid = (HINTERNET)0x234;
    pal_wsclient_t* wsclient_valid;
    int32_t result;

    REGISTER_GLOBAL_MOCK_RETURNS(pal_os_last_error_as_prx_error, er_out_of_memory, er_out_of_memory);

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(h_realloc(sizeof(pal_wsclient_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM)
        .SetFailReturn(NULL);

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
    STRICT_EXPECTED_CALL(h_realloc(20, NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)k_wide_char_valid)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_host_valid, -1, k_wide_char_valid, 10))
        .SetReturn(10)
        .SetFailReturn(-1);

    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_path_valid, -1, NULL, 0))
        .SetReturn(10)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(h_realloc(20, NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)k_wide_char_valid)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_path_valid, -1, k_wide_char_valid, 10))
        .SetReturn(10)
        .SetFailReturn(-1);

    STRICT_EXPECTED_CALL(WinHttpOpen(NULL, WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, NULL, NULL, WINHTTP_FLAG_ASYNC))
        .SetReturn(k_session_handle_valid)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(WinHttpSetOption(k_session_handle_valid, WINHTTP_OPTION_CONTEXT_VALUE, IGNORED_PTR_ARG, sizeof(pal_wsclient_t*)))
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
        er_out_of_memory, er_fatal, er_out_of_memory, er_fatal, er_fatal,
        er_out_of_memory, er_fatal, er_out_of_memory);
}

// 
// Test pal_wsclient_add_header happy path 
// 
TEST_FUNCTION(pal_lws_wsclient_add_header__success_1)
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
TEST_FUNCTION(pal_lws_wsclient_add_header__success_2)
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
TEST_FUNCTION(pal_lws_wsclient_add_header__arg_wsclient_invalid)
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
TEST_FUNCTION(pal_lws_wsclient_add_header__arg_key_invalid_1)
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
TEST_FUNCTION(pal_lws_wsclient_add_header__arg_key_invalid_2)
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
TEST_FUNCTION(pal_lws_wsclient_add_header__arg_value_invalid)
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
TEST_FUNCTION(pal_lws_wsclient_add_header__neg)
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
TEST_FUNCTION(pal_lws_wsclient_connect__success)
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
    STRICT_EXPECTED_CALL(WinHttpSetOption(k_h_request_valid, WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET, NULL, 0))
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(WinHttpSetTimeouts(k_h_request_valid, 0, 0, 0, 0))
        .SetReturn(TRUE);

    STRICT_EXPECTED_CALL(STRING_c_str(k_headers_valid))
        .SetReturn(k_headers_ptr);
    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_headers_ptr, -1, NULL, 0))
        .SetReturn(10);
    STRICT_EXPECTED_CALL(h_realloc(20, NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
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
// Test pal_wsclient_connect passing as websocket argument an invalid pal_wsclient_t* value 
// 
TEST_FUNCTION(pal_lws_wsclient_connect__arg_websocket_null)
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
TEST_FUNCTION(pal_lws_wsclient_connect__neg_1)
{
    static HINTERNET k_h_request_valid = (HINTERNET)0x666;
    static HINTERNET k_h_session_valid = (HINTERNET)0x2345;
    static HINTERNET k_h_connection_valid = (HINTERNET)0x235;
    pal_wsclient_t wsclient_valid;
    int32_t result;

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
TEST_FUNCTION(pal_lws_wsclient_connect__neg_2)
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
    STRICT_EXPECTED_CALL(WinHttpSetOption(k_h_request_valid, WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET, NULL, 0))
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(WinHttpSetTimeouts(k_h_request_valid, 0, 0, 0, 0))
        .SetReturn(TRUE);

    STRICT_EXPECTED_CALL(STRING_c_str(k_headers_valid))
        .SetReturn(k_headers_ptr);
    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_headers_ptr, -1, NULL, 0))
        .SetReturn(10);
    STRICT_EXPECTED_CALL(h_realloc(20, NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
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
    STRICT_EXPECTED_CALL(pal_os_last_error_as_prx_error())
        .SetReturn(er_connecting);

    STRICT_EXPECTED_CALL(WinHttpCloseHandle(k_h_request_valid))
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(WinHttpCloseHandle(k_h_connection_valid))
        .SetReturn(TRUE);

    // act 
    result = pal_wsclient_connect(&wsclient_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_connecting, result);
    ASSERT_IS_NULL(wsclient_valid.h_connection);
    ASSERT_IS_NULL(wsclient_valid.h_request);
}

// 
// Test pal_wsclient_connect unhappy path 
// 
TEST_FUNCTION(pal_lws_wsclient_connect__neg_3)
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
    wsclient_valid.host = L"test";
    wsclient_valid.relative_path = L"foo";
    wsclient_valid.headers = k_headers_valid;
    wsclient_valid.port = 10;
    wsclient_valid.secure = true;

    REGISTER_GLOBAL_MOCK_RETURNS(pal_os_last_error_as_prx_error, er_out_of_memory, er_out_of_memory);

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(WinHttpConnect(k_h_session_valid, IGNORED_PTR_ARG, 10, 0))
        .IgnoreArgument(2)
        .SetReturn(k_h_connection_valid)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(WinHttpSetOption(k_h_connection_valid, WINHTTP_OPTION_CONTEXT_VALUE, IGNORED_PTR_ARG, sizeof(pal_wsclient_t*)))
        .IgnoreArgument(3)
        .SetReturn(TRUE)
        .SetFailReturn(FALSE);
    STRICT_EXPECTED_CALL(WinHttpOpenRequest(k_h_connection_valid, IGNORED_PTR_ARG, IGNORED_PTR_ARG, NULL, NULL, NULL, WINHTTP_FLAG_SECURE))
        .IgnoreArgument(2).IgnoreArgument(3)
        .SetReturn(k_h_request_valid)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(WinHttpSetOption(k_h_request_valid, WINHTTP_OPTION_CONTEXT_VALUE, IGNORED_PTR_ARG, sizeof(pal_wsclient_t*)))
        .IgnoreArgument(3)
        .SetReturn(TRUE)
        .SetFailReturn(FALSE);
    STRICT_EXPECTED_CALL(WinHttpSetOption(k_h_request_valid, WINHTTP_OPTION_CLIENT_CERT_CONTEXT, NULL, 0))
        .SetReturn(TRUE)
        .SetFailReturn(FALSE);
    STRICT_EXPECTED_CALL(WinHttpSetOption(k_h_request_valid, WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET, NULL, 0))
        .SetReturn(TRUE)
        .SetFailReturn(FALSE);
    STRICT_EXPECTED_CALL(WinHttpSetTimeouts(k_h_request_valid, 0, 0, 0, 0))
        .SetReturn(TRUE)
        .SetFailReturn(FALSE);
    STRICT_EXPECTED_CALL(STRING_c_str(k_headers_valid))
        .SetReturn(k_headers_ptr)
        .SetFailReturn(k_headers_ptr);
    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_headers_ptr, -1, NULL, 0))
        .SetReturn(10)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(h_realloc(20, NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)k_wide_char_valid)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(MultiByteToWideChar(CP_UTF8, 0, k_headers_ptr, -1, k_wide_char_valid, 10))
        .SetReturn(10)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(WinHttpAddRequestHeaders(k_h_request_valid, k_wide_char_valid, (DWORD)-1, WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE))
        .SetReturn(TRUE)
        .SetFailReturn(FALSE);
    STRICT_EXPECTED_CALL(h_free((void*)k_wide_char_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);
    STRICT_EXPECTED_CALL(WinHttpSendRequest(k_h_request_valid, NULL, 0, NULL, 0, 0, 0))
        .SetReturn(TRUE)
        .SetFailReturn(FALSE);

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    wsclient_valid.h_connection = NULL;
    wsclient_valid.h_request = NULL;
    result = pal_wsclient_connect(&wsclient_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result,
        er_out_of_memory, er_out_of_memory, er_out_of_memory, er_out_of_memory, er_out_of_memory,
        er_out_of_memory, er_out_of_memory, er_ok, er_fatal, er_out_of_memory,
        er_fatal, er_out_of_memory, er_ok, er_out_of_memory);
}

// 
// Test pal_wsclient_can_recv happy path 
// 
TEST_FUNCTION(pal_lws_wsclient_can_recv__success_0)
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
TEST_FUNCTION(pal_lws_wsclient_can_recv__success_1)
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
TEST_FUNCTION(pal_lws_wsclient_can_recv__success_2)
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
TEST_FUNCTION(pal_lws_wsclient_can_recv__arg_wsclient_invalid)
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
TEST_FUNCTION(pal_lws_wsclient_can_recv__neg)
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
    STRICT_EXPECTED_CALL(FormatMessageA(IGNORED_NUM_ARG, NULL, ERROR_WINHTTP_OPERATION_CANCELLED, IGNORED_NUM_ARG, IGNORED_PTR_ARG, 0, NULL))
        .IgnoreArgument(1).IgnoreArgument(4)
        .CopyOutArgumentBuffer_lpBuffer(&k_message, sizeof(const char*))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(LocalFree((HLOCAL)k_message))
        .SetReturn((HLOCAL)NULL);
    STRICT_EXPECTED_CALL(pal_wsclient_event_handler_mock((void*)0x1, pal_wsclient_event_end_recv, IGNORED_PTR_ARG, IGNORED_PTR_ARG, NULL, er_aborted))
        .ValidateArgumentBuffer(3, &k_buffer_ptr, sizeof(k_buffer_ptr))
        .IgnoreArgument(4);

    // act 
    result = pal_wsclient_can_recv(&wsclient_valid, true);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_wsclient_can_send happy path 
// 
TEST_FUNCTION(pal_lws_wsclient_can_send__success_0)
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
TEST_FUNCTION(pal_lws_wsclient_can_send__success_1)
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
TEST_FUNCTION(pal_lws_wsclient_can_send__success_2)
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
TEST_FUNCTION(pal_lws_wsclient_can_send__arg_wsclient_invalid)
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
TEST_FUNCTION(pal_lws_wsclient_can_send__neg)
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
    STRICT_EXPECTED_CALL(FormatMessageA(IGNORED_NUM_ARG, NULL, ERROR_WINHTTP_OPERATION_CANCELLED, IGNORED_NUM_ARG, IGNORED_PTR_ARG, 0, NULL))
        .IgnoreArgument(1).IgnoreArgument(4)
        .CopyOutArgumentBuffer_lpBuffer(&k_message, sizeof(const char*))
        .SetReturn(0);
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
TEST_FUNCTION(pal_lws_wsclient_disconnect__success_1)
{
    pal_wsclient_t wsclient_valid;
    int32_t result;

    wsclient_valid.h_request = NULL;
    wsclient_valid.h_connection = NULL;
    wsclient_valid.h_websocket = NULL;
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
TEST_FUNCTION(pal_lws_wsclient_disconnect__success_2)
{
    pal_wsclient_t wsclient_valid;
    int32_t result;

    wsclient_valid.h_request = NULL;
    wsclient_valid.h_connection = NULL;
    wsclient_valid.h_websocket = NULL;
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
TEST_FUNCTION(pal_lws_wsclient_disconnect__success_3)
{
    static HINTERNET k_h_wsclient_valid = (HINTERNET)0x56789;
    pal_wsclient_t wsclient_valid;
    int32_t result;

    wsclient_valid.h_websocket = k_h_wsclient_valid;
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
TEST_FUNCTION(pal_lws_wsclient_disconnect__success_4)
{
    static HINTERNET k_h_wsclient_valid = (HINTERNET)0x56789;
    pal_wsclient_t wsclient_valid;
    int32_t result;

    wsclient_valid.h_websocket = k_h_wsclient_valid;
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
TEST_FUNCTION(pal_lws_wsclient_disconnect__success_5)
{
    static HINTERNET k_h_request_valid = (HINTERNET)0x666;
    static HINTERNET k_h_connection_valid = (HINTERNET)0x235;
    pal_wsclient_t wsclient_valid;
    int32_t result;

    wsclient_valid.h_request = k_h_request_valid;
    wsclient_valid.h_connection = k_h_connection_valid;
    wsclient_valid.h_websocket = NULL;
    wsclient_valid.cur_send_buffer = NULL;
    wsclient_valid.cur_recv_buffer = NULL;
    wsclient_valid.cb = pal_wsclient_event_handler_mock;
    wsclient_valid.context = (void*)0x1;
    wsclient_valid.state = 0;

    // arrange 
    STRICT_EXPECTED_CALL(WinHttpCloseHandle(k_h_request_valid))
        .SetReturn(FALSE);
    STRICT_EXPECTED_CALL(GetLastError())
        .SetReturn(ERROR_WINHTTP_INCORRECT_HANDLE_TYPE);
    STRICT_EXPECTED_CALL(WinHttpCloseHandle(k_h_connection_valid))
        .SetReturn(FALSE);
    STRICT_EXPECTED_CALL(GetLastError())
        .SetReturn(ERROR_WINHTTP_INCORRECT_HANDLE_TYPE);
    STRICT_EXPECTED_CALL(pal_wsclient_event_handler_mock((void*)0x1, pal_wsclient_event_disconnected, NULL, NULL, NULL, er_ok));

    // act 
    result = pal_wsclient_disconnect(&wsclient_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_wsclient_disconnect passing as websocket argument an invalid pal_wsclient_t* value 
// 
TEST_FUNCTION(pal_lws_wsclient_disconnect__arg_websocket_null)
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
TEST_FUNCTION(pal_lws_wsclient_close__success_1)
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
TEST_FUNCTION(pal_lws_wsclient_close__success_2)
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
    STRICT_EXPECTED_CALL(GetLastError())
        .SetReturn(ERROR_WINHTTP_INCORRECT_HANDLE_TYPE);
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
TEST_FUNCTION(pal_lws_wsclient_close__success_3)
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
// Test pal_wsclient_close passing as websocket argument an invalid pal_wsclient_t* value 
// 
TEST_FUNCTION(pal_lws_wsclient_close__arg_websocket_invalid)
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
TEST_FUNCTION(pal_lws_wsclient_deinit__success_1)
{
    static const lock_t k_lock_valid = (lock_t)0x1;
    pal_wsworker_pool_t k_pool_valid;

    k_pool_valid.pool_lock = k_lock_valid;
    global_wsworker_pool = &k_pool_valid;

    // arrange 
    STRICT_EXPECTED_CALL(lock_enter(k_lock_valid));
    STRICT_EXPECTED_CALL(DList_IsListEmpty(IGNORED_PTR_ARG))
        .IgnoreArgument(1)
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(lock_exit(k_lock_valid));
    STRICT_EXPECTED_CALL(lock_free(k_lock_valid));
    STRICT_EXPECTED_CALL(h_free((void*)&k_pool_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    pal_wsclient_deinit();

    ASSERT_EXPECTED_CALLS();
}
#endif

// 
// Test pal_wsclient_deinit happy path 
// 
TEST_FUNCTION(pal_lws_wsclient_deinit__success_2)
{
    global_wsworker_pool = NULL;

    // arrange 

    // act 
    pal_wsclient_deinit();

    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_wsclient_init happy path 
// 
TEST_FUNCTION(pal_lws_wsclient_init__success_1)
{
    int32_t result;

    global_wsworker_pool = NULL;

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(pal_wsworker_pool_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(DList_InitializeListHead(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(lock_create(IGNORED_PTR_ARG))
        .IgnoreArgument(1)
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(lws_set_log_level(-1, pal_wsclient_lws_log));

    // act 
    result = pal_wsclient_init();

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_wsclient_init happy path 
// 
TEST_FUNCTION(pal_lws_wsclient_init__neg_0)
{
    int32_t result;

    global_wsworker_pool = (pal_wsworker_pool_t*)0x1;

    // arrange 

    // act 
    result = pal_wsclient_init();

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_bad_state, result);
}

// 
// Test pal_wsclient_init happy path 
// 
TEST_FUNCTION(pal_lws_wsclient_init__neg_1)
{
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));
    global_wsworker_pool = NULL;

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(pal_wsworker_pool_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(DList_InitializeListHead(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(lock_create(IGNORED_PTR_ARG))
        .IgnoreArgument(1)
        .SetReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL(h_free((void*)UT_MEM, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    result = pal_wsclient_init();

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_out_of_memory, result);
}

// 
// Test pal_wsclient_init happy path 
// 
TEST_FUNCTION(pal_lws_wsclient_init__neg_2)
{
    int32_t result;
    global_wsworker_pool = NULL;

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(pal_wsworker_pool_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn(NULL);

    // act 
    result = pal_wsclient_init();

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_out_of_memory, result);
}

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

