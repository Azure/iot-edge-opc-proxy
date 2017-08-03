// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST pal_ev_win
#include "util_ut.h"

//
// 1. Required mocks
//
#include "os.h"
#include "pal_sk.h"

// windows.h
MOCKABLE_FUNCTION(WINBASEAPI, BOOL, RegisterWaitForSingleObject,
    HANDLE*, phNewWaitObject, HANDLE, hObject, WAITORTIMERCALLBACK, Callback,
    PVOID, Context, ULONG, dwMilliseconds, ULONG, dwFlags);
MOCKABLE_FUNCTION(WINBASEAPI, BOOL, UnregisterWaitEx,
    HANDLE, WaitHandle, HANDLE, CompletionEvent);
// winsock.h
MOCKABLE_FUNCTION(WSAAPI, BOOL, WSACloseEvent,
    WSAEVENT, hEvent);
MOCKABLE_FUNCTION(WSAAPI, WSAEVENT, WSACreateEvent);
MOCKABLE_FUNCTION(WSAAPI, int, WSAEnumNetworkEvents,
    SOCKET, s, WSAEVENT, hEventObject, LPWSANETWORKEVENTS, lpNetworkEvents);
MOCKABLE_FUNCTION(WSAAPI, int, WSAEventSelect,
    SOCKET, s, WSAEVENT, hEventObject, long, lNetworkEvents);
MOCKABLE_FUNCTION(WSAAPI, DWORD, WSAWaitForMultipleEvents,
    DWORD, cEvents, const WSAEVENT*, lphEvents, BOOL, fWaitAll, DWORD, dwTimeout, BOOL, fAlertable);
// winsock2.h
MOCKABLE_FUNCTION(WSAAPI, int, closesocket,
    SOCKET, s);

//
// 2. Include unit under test
//
#undef ENABLE_MOCKS
#include "pal_ev.h"
#define ENABLE_MOCKS
#include UNIT_C

MOCKABLE_FUNCTION(, int32_t, pal_event_handler_cb_mock,
    void*, context, pal_event_type_t, event_type, int32_t, error_code);

//
// 3. Setup test suite
//
BEGIN_DECLARE_TEST_SUITE()
REGISTER_UMOCK_ALIAS_TYPE(lock_t, void*);
REGISTER_UMOCK_ALIAS_TYPE(pal_event_type_t, int);
REGISTER_UMOCK_ALIAS_TYPE(uintptr_t, void*);
REGISTER_UMOCK_ALIAS_TYPE(ULONG, unsigned long);
REGISTER_UMOCK_ALIAS_TYPE(DWORD, unsigned int);
REGISTER_UMOCK_ALIAS_TYPE(HANDLE, void*);
REGISTER_UMOCK_ALIAS_TYPE(SOCKET, void*);
REGISTER_UMOCK_ALIAS_TYPE(PVOID, void*);
REGISTER_UMOCK_ALIAS_TYPE(WSAEVENT, void*);
REGISTER_UMOCK_ALIAS_TYPE(BOOL, bool);
REGISTER_UMOCK_ALIAS_TYPE(BOOLEAN, bool);
REGISTER_UMOCK_ALIAS_TYPE(WAITORTIMERCALLBACK, void*);
REGISTER_UMOCK_ALIAS_TYPE(LPWSANETWORKEVENTS, void*);
END_DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
DECLARE_TEST_SETUP()

//
// Test pal_event_port_create happy path
//
TEST_FUNCTION(pal_win_event_port_create__success)
{
    uintptr_t port_valid;
    int32_t result;

    // arrange

    // act
    result = pal_event_port_create(NULL, NULL, &port_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_IS_TRUE(EVENT_PORT_HANDLE == port_valid);
}

//
// Test pal_event_port_create passing as port argument an invalid uintptr_t* value
//
TEST_FUNCTION(pal_win_event_port_create__arg_port_null)
{
    int32_t result;

    // arrange

    // act
    result = pal_event_port_create(NULL, NULL, NULL);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

//
// Test pal_event_port_create passing as timeout argument an invalid value
//
TEST_FUNCTION(pal_win_event_port_create__arg_timeout_handler_invalid)
{
    uintptr_t port_valid;
    static pal_timeout_handler_t k_timeout_handler_invalid = (pal_timeout_handler_t)0x234;
    static void* k_timeout_context = (void*)0x234;
    int32_t result;

    // arrange

    // act
    result = pal_event_port_create(k_timeout_handler_invalid, k_timeout_context, &port_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_not_supported, result);
}

//
// Test pal_event_port_close happy path
//
TEST_FUNCTION(pal_win_event_port_close__success)
{
    static const uintptr_t k_port_valid = (uintptr_t)EVENT_PORT_HANDLE;

    // arrange

    // act
    pal_event_port_close(k_port_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
}

//
// Test pal_event_port_close passing as port argument an invalid uintptr_t value
//
TEST_FUNCTION(pal_win_event_port_close__arg_port_invalid)
{
    // arrange

    // act
    pal_event_port_close(0);

    // assert
    ASSERT_EXPECTED_CALLS();
}

//
// Test pal_event_port_register happy path
//
TEST_FUNCTION(pal_win_event_port_register__success)
{
    static const uintptr_t k_port_valid = (uintptr_t)0x2345;
    static const intptr_t k_socket_valid = (intptr_t)0xabab;
    static const lock_t k_lock1_valid = (lock_t)0x1;
    static const lock_t k_lock2_valid = (lock_t)0x2;
    static const WSAEVENT k_handle1_valid = (WSAEVENT)0x3;
    static const HANDLE k_handle2_valid = (HANDLE)0x2;
    static void* k_context_valid = (void*)0x12;
    pal_event_data_t event_data_valid;
    uintptr_t event_handle_valid;
    int32_t result;

    // arrange
    STRICT_EXPECTED_CALL(h_realloc(sizeof(pal_event_data_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)&event_data_valid);
    STRICT_EXPECTED_CALL(lock_create(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_created(&k_lock1_valid, sizeof(k_lock1_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(lock_create(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_created(&k_lock2_valid, sizeof(k_lock2_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(WSACreateEvent())
        .SetReturn(k_handle1_valid);
    STRICT_EXPECTED_CALL(RegisterWaitForSingleObject(IGNORED_PTR_ARG, k_handle1_valid, IGNORED_PTR_ARG, (void*)&event_data_valid, INFINITE, WT_EXECUTEDEFAULT))
        .CopyOutArgumentBuffer_phNewWaitObject(&k_handle2_valid, sizeof(k_handle2_valid))
        .IgnoreArgument(3)
        .SetReturn(TRUE);

    // act
    result = pal_event_port_register(
        k_port_valid, k_socket_valid, pal_event_handler_cb_mock, k_context_valid, &event_handle_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

//
// Test pal_event_port_register passing as socket argument an invalid intptr_t value
//
TEST_FUNCTION(pal_win_event_port_register__arg_socket_invalid)
{
    static const uintptr_t k_port_valid = (uintptr_t)0x2345;
    static void* k_context_valid = (void*)0x12;
    uintptr_t event_handle_valid;
    int32_t result;

    // arrange

    // act
    result = pal_event_port_register(
        k_port_valid, _invalid_fd, pal_event_handler_cb_mock, k_context_valid, &event_handle_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_arg, result);
}

//
// Test pal_event_port_register passing as cb argument an invalid pal_event_handler_t value
//
TEST_FUNCTION(pal_win_event_port_register__arg_cb_invalid)
{
    static const uintptr_t k_port_valid = (uintptr_t)0x2345;
    static const intptr_t k_socket_valid = (intptr_t)0xabab;
    static void* k_context_valid = (void*)0x12;
    uintptr_t event_handle_valid;
    int32_t result;

    // arrange

    // act
    result = pal_event_port_register(
        k_port_valid, k_socket_valid, NULL, k_context_valid, &event_handle_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

//
// Test pal_event_port_register passing as event_handle argument an invalid uintptr_t* value
//
TEST_FUNCTION(pal_win_event_port_register__arg_event_handle_invalid)
{
    static const uintptr_t k_port_valid = (uintptr_t)0x2345;
    static const intptr_t k_socket_valid = (intptr_t)0xabab;
    static void* k_context_valid = (void*)0x12;
    int32_t result;

    // arrange

    // act
    result = pal_event_port_register(
        k_port_valid, k_socket_valid, pal_event_handler_cb_mock, k_context_valid, NULL);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

//
// Test pal_event_port_register unhappy path
//
TEST_FUNCTION(pal_win_event_port_register__neg)
{
    static const uintptr_t k_port_valid = (uintptr_t)0x2345;
    static const intptr_t k_socket_valid = (intptr_t)0xabab;
    static const lock_t k_lock1_valid = (lock_t)0x1;
    static const lock_t k_lock2_valid = (lock_t)0x2;
    static const WSAEVENT k_handle1_valid = (WSAEVENT)0x3;
    static const HANDLE k_handle2_valid = (HANDLE)0x4;
    static const WSAEVENT k_handle3_valid = (WSAEVENT)0x5;
    static void* k_context_valid = (void*)0x12;
    pal_event_data_t event_data_valid;
    uintptr_t event_handle_valid;
    int32_t result;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(h_realloc(sizeof(pal_event_data_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)&event_data_valid)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(lock_create(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_created(&k_lock1_valid, sizeof(k_lock1_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL(lock_create(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_created(&k_lock2_valid, sizeof(k_lock2_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL(WSACreateEvent())
        .SetReturn(k_handle1_valid)
        .SetFailReturn(INVALID_HANDLE_VALUE);
    STRICT_EXPECTED_CALL(RegisterWaitForSingleObject(IGNORED_PTR_ARG, k_handle1_valid, IGNORED_PTR_ARG, (void*)&event_data_valid, INFINITE, WT_EXECUTEDEFAULT))
        .CopyOutArgumentBuffer_phNewWaitObject(&k_handle2_valid, sizeof(k_handle2_valid))
        .IgnoreArgument(3)
        .SetReturn(TRUE)
        .SetFailReturn(FALSE);
    STRICT_EXPECTED_CALL(pal_os_last_error_as_prx_error())
        .SetReturn(er_fatal)
        .SetFailReturn(er_fatal);
    STRICT_EXPECTED_CALL(WSAEventSelect(k_socket_valid, k_handle1_valid, 0))
        .SetReturn(0)
        .SetFailReturn(SOCKET_ERROR);
    STRICT_EXPECTED_CALL(pal_os_last_net_error_as_prx_error())
        .SetReturn(er_ok)
        .SetFailReturn(er_fatal);
    STRICT_EXPECTED_CALL(WSACreateEvent())
        .SetReturn(k_handle3_valid)
        .SetFailReturn(INVALID_HANDLE_VALUE);
    STRICT_EXPECTED_CALL(UnregisterWaitEx(k_handle2_valid, k_handle3_valid))
        .SetReturn(0)
        .SetFailReturn(0);
    STRICT_EXPECTED_CALL(WSAWaitForMultipleEvents(1, &k_handle3_valid, TRUE, 60000, FALSE))
        .SetReturn(WSA_WAIT_EVENT_0)
        .SetFailReturn((DWORD)-1);
    STRICT_EXPECTED_CALL(WSACloseEvent(k_handle3_valid))
        .SetReturn(TRUE)
        .SetFailReturn(FALSE);
    STRICT_EXPECTED_CALL(WSACloseEvent(k_handle1_valid))
        .SetReturn(TRUE)
        .SetFailReturn(FALSE);
    STRICT_EXPECTED_CALL(pal_event_handler_cb_mock(k_context_valid, pal_event_type_destroy, er_ok))
        .SetReturn(er_ok)
        .SetFailReturn(er_fatal);
    STRICT_EXPECTED_CALL(lock_free(k_lock1_valid));
    STRICT_EXPECTED_CALL(lock_free(k_lock2_valid));
    STRICT_EXPECTED_CALL(h_free((void*)&event_data_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    memset(&event_data_valid, 0, sizeof(event_data_valid));
    result = pal_event_port_register(
        k_port_valid, k_socket_valid, pal_event_handler_cb_mock, k_context_valid, &event_handle_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result,
        er_out_of_memory, er_out_of_memory, er_out_of_memory, er_out_of_memory, er_fatal, er_ok);
}

//
// Test pal_event_select happy path
//
TEST_FUNCTION(pal_win_event_select__success_1)
{
    static const WSAEVENT k_handle1_valid = (WSAEVENT)0x3;
    static const intptr_t k_socket_valid = (intptr_t)0xbaba;
    static void* k_context_valid = (void*)0x12;
    const pal_event_type_t k_event_type_valid = pal_event_type_write;
    pal_event_data_t ev_data_valid;
    uintptr_t event_handle_valid = (uintptr_t)&ev_data_valid;
    int32_t result;

    ev_data_valid.events = (FD_READ | FD_ACCEPT | FD_CLOSE);
    ev_data_valid.net_event = k_handle1_valid;
    ev_data_valid.sock = k_socket_valid;
    ev_data_valid.cb = pal_event_handler_cb_mock;
    ev_data_valid.context = k_context_valid;

    // arrange
    STRICT_EXPECTED_CALL(WSAEventSelect(k_socket_valid, k_handle1_valid, (FD_READ | FD_ACCEPT | FD_CLOSE | FD_WRITE)))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(pal_event_handler_cb_mock(k_context_valid, pal_event_type_write, er_ok))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_event_handler_cb_mock(k_context_valid, pal_event_type_write, er_ok))
        .SetReturn(er_retry);

    // act
    result = pal_event_select(event_handle_valid, k_event_type_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

//
// Test pal_event_select happy path
//
TEST_FUNCTION(pal_win_event_select__success_2)
{
    static const WSAEVENT k_handle1_valid = (WSAEVENT)0x3;
    static const intptr_t k_socket_valid = (intptr_t)0xbaba;
    static void* k_context_valid = (void*)0x12;
    const pal_event_type_t k_event_type_valid = pal_event_type_read;
    pal_event_data_t ev_data_valid;
    uintptr_t event_handle_valid = (uintptr_t)&ev_data_valid;
    int32_t result;

    ev_data_valid.events = (FD_WRITE);
    ev_data_valid.net_event = k_handle1_valid;
    ev_data_valid.sock = k_socket_valid;
    ev_data_valid.cb = pal_event_handler_cb_mock;
    ev_data_valid.context = k_context_valid;

    // arrange
    STRICT_EXPECTED_CALL(WSAEventSelect(k_socket_valid, k_handle1_valid, (FD_READ | FD_ACCEPT | FD_CLOSE | FD_WRITE)))
        .SetReturn(0);

    // act
    result = pal_event_select(event_handle_valid, k_event_type_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}


//
// Test pal_event_select passing as event_handle argument an invalid uintptr_t value
//
TEST_FUNCTION(pal_win_event_select__arg_event_handle_null)
{
    const pal_event_type_t k_event_type_valid = pal_event_type_read;
    int32_t result;

    // arrange

    // act
    result = pal_event_select(0, k_event_type_valid);

    // assert
    // ...
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

//
// Test pal_event_select passing as event_type argument an invalid pal_event_type_t value
//
TEST_FUNCTION(pal_win_event_select__arg_event_type_invalid)
{
    const pal_event_type_t k_event_type_invalid = (pal_event_type_t)0x44444;
    pal_event_data_t ev_data_valid;
    uintptr_t event_handle_valid = (uintptr_t)&ev_data_valid;
    int32_t result;

    ev_data_valid.events = (FD_READ | FD_ACCEPT | FD_CLOSE | FD_WRITE);

    // arrange

    // act
    result = pal_event_select(event_handle_valid, k_event_type_invalid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

//
// Test pal_event_select unhappy path
//
TEST_FUNCTION(pal_win_event_select__neg)
{
    static const WSAEVENT k_handle1_valid = (WSAEVENT)0x3;
    static const intptr_t k_socket_valid = (intptr_t)0xbaba;
    static void* k_context_valid = (void*)0x12;
    const pal_event_type_t k_event_type_valid = pal_event_type_write;
    pal_event_data_t ev_data_valid;
    uintptr_t event_handle_valid = (uintptr_t)&ev_data_valid;
    int32_t result;

    ev_data_valid.events = (FD_READ | FD_ACCEPT | FD_CLOSE);
    ev_data_valid.net_event = k_handle1_valid;
    ev_data_valid.sock = k_socket_valid;
    ev_data_valid.cb = pal_event_handler_cb_mock;
    ev_data_valid.context = k_context_valid;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(WSAEventSelect((SOCKET)k_socket_valid, k_handle1_valid, (FD_READ | FD_ACCEPT | FD_CLOSE | FD_WRITE)))
        .SetReturn(0)
        .SetFailReturn(SOCKET_ERROR);
    STRICT_EXPECTED_CALL(pal_os_last_net_error_as_prx_error())
        .SetReturn(er_fatal)
        .SetFailReturn(er_fatal);
    STRICT_EXPECTED_CALL(pal_event_handler_cb_mock(k_context_valid, pal_event_type_write, er_ok))
        .SetReturn(er_ok)
        .SetFailReturn(er_fatal);
    STRICT_EXPECTED_CALL(pal_event_handler_cb_mock(k_context_valid, pal_event_type_write, er_ok))
        .SetReturn(er_retry)
        .SetFailReturn(er_fatal);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_event_select(event_handle_valid, k_event_type_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_fatal, er_ok);
}

//
// Test pal_event_clear happy path
//
TEST_FUNCTION(pal_win_event_clear__success_1)
{
    static const WSAEVENT k_handle1_valid = (WSAEVENT)0x3;
    static const intptr_t k_socket_valid = (intptr_t)0xbaba;
    const pal_event_type_t k_event_type_valid = pal_event_type_write;
    pal_event_data_t ev_data_valid;
    uintptr_t event_handle_valid = (uintptr_t)&ev_data_valid;
    int32_t result;

    ev_data_valid.events = (FD_READ | FD_ACCEPT | FD_CLOSE | FD_WRITE);
    ev_data_valid.net_event = k_handle1_valid;
    ev_data_valid.sock = k_socket_valid;

    // arrange
    STRICT_EXPECTED_CALL(WSAEventSelect(k_socket_valid, k_handle1_valid, (FD_READ | FD_ACCEPT | FD_CLOSE)))
        .SetReturn(0);

    // act
    result = pal_event_clear(event_handle_valid, k_event_type_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

//
// Test pal_event_clear happy path
//
TEST_FUNCTION(pal_win_event_clear__success_2)
{
    static const WSAEVENT k_handle1_valid = (WSAEVENT)0x3;
    static const intptr_t k_socket_valid = (intptr_t)0xbaba;
    const pal_event_type_t k_event_type_valid = pal_event_type_read;
    pal_event_data_t ev_data_valid;
    uintptr_t event_handle_valid = (uintptr_t)&ev_data_valid;
    int32_t result;

    ev_data_valid.events = (FD_READ | FD_ACCEPT | FD_CLOSE | FD_WRITE);
    ev_data_valid.net_event = k_handle1_valid;
    ev_data_valid.sock = k_socket_valid;

    // arrange
    STRICT_EXPECTED_CALL(WSAEventSelect(k_socket_valid, k_handle1_valid, FD_WRITE))
        .SetReturn(0);

    // act
    result = pal_event_clear(event_handle_valid, k_event_type_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

//
// Test pal_event_clear happy path
//
TEST_FUNCTION(pal_win_event_clear__success_3)
{
    static const WSAEVENT k_handle1_valid = (WSAEVENT)0x3;
    static const intptr_t k_socket_valid = (intptr_t)0xbaba;
    const pal_event_type_t k_event_type_valid = pal_event_type_read;
    pal_event_data_t ev_data_valid;
    uintptr_t event_handle_valid = (uintptr_t)&ev_data_valid;
    int32_t result;

    ev_data_valid.events = FD_WRITE;
    ev_data_valid.net_event = k_handle1_valid;
    ev_data_valid.sock = k_socket_valid;

    // arrange

    // act
    result = pal_event_clear(event_handle_valid, k_event_type_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

//
// Test pal_event_clear passing as event_handle argument an invalid uintptr_t value
//
TEST_FUNCTION(pal_win_event_clear__arg_event_handle_null)
{
    const pal_event_type_t k_event_type_valid = pal_event_type_read;
    int32_t result;

    // arrange

    // act
    result = pal_event_clear(0, k_event_type_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

//
// Test pal_event_clear passing as event_type argument an invalid pal_event_type_t value
//
TEST_FUNCTION(pal_win_event_clear__arg_event_type_invalid)
{
    const pal_event_type_t k_event_type_invalid = (pal_event_type_t)0x245;
    pal_event_data_t ev_data_valid;
    uintptr_t event_handle_valid = (uintptr_t)&ev_data_valid;
    int32_t result;

    ev_data_valid.events = (FD_READ | FD_ACCEPT | FD_CLOSE | FD_WRITE);

    // arrange

    // act
    result = pal_event_clear(event_handle_valid, k_event_type_invalid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

//
// Test pal_event_clear unhappy path
//
TEST_FUNCTION(pal_win_event_clear__neg)
{
    static const WSAEVENT k_handle1_valid = (WSAEVENT)0x3;
    static const intptr_t k_socket_valid = (intptr_t)0xbaba;
    const pal_event_type_t k_event_type_valid = pal_event_type_write;
    pal_event_data_t ev_data_valid;
    uintptr_t event_handle_valid = (uintptr_t)&ev_data_valid;
    int32_t result;

    ev_data_valid.events = (FD_READ | FD_ACCEPT | FD_CLOSE | FD_WRITE);
    ev_data_valid.net_event = k_handle1_valid;
    ev_data_valid.sock = k_socket_valid;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(WSAEventSelect(k_socket_valid, k_handle1_valid, (FD_READ | FD_ACCEPT | FD_CLOSE)))
        .SetReturn(0)
        .SetFailReturn(SOCKET_ERROR);
    STRICT_EXPECTED_CALL(pal_os_last_net_error_as_prx_error())
        .SetReturn(er_fatal)
        .SetFailReturn(er_fatal);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_event_clear(event_handle_valid, k_event_type_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_fatal, er_ok);
}

//
// Test pal_event_close happy path
//
TEST_FUNCTION(pal_win_event_close__success_1)
{
    static const uintptr_t k_port_valid = (uintptr_t)0x2345;
    static const intptr_t k_socket_valid = (intptr_t)0xabab;
    static const lock_t k_lock1_valid = (lock_t)0x1;
    static const lock_t k_lock2_valid = (lock_t)0x2;
    static const WSAEVENT k_handle1_valid = (WSAEVENT)0x3;
    static const HANDLE k_handle2_valid = (HANDLE)0x4;
    static const WSAEVENT k_handle3_valid = (WSAEVENT)0x5;
    static void* k_context_valid = (void*)0x12;
    pal_event_data_t ev_data_valid;
    uintptr_t event_handle_valid = (uintptr_t)&ev_data_valid;

    ev_data_valid.events = (FD_READ | FD_ACCEPT | FD_CLOSE | FD_WRITE);
    ev_data_valid.net_event = k_handle1_valid;
    ev_data_valid.wait_object = k_handle2_valid;
    ev_data_valid.sock = k_socket_valid;
    ev_data_valid.cb = pal_event_handler_cb_mock;
    ev_data_valid.context = k_context_valid;
    ev_data_valid.read_lock = k_lock1_valid;
    ev_data_valid.write_lock = k_lock2_valid;

    // arrange
    STRICT_EXPECTED_CALL(WSAEventSelect(k_socket_valid, k_handle1_valid, 0))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(WSACreateEvent())
        .SetReturn(k_handle3_valid);
    STRICT_EXPECTED_CALL(UnregisterWaitEx(k_handle2_valid, k_handle3_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(WSAWaitForMultipleEvents(1, IGNORED_PTR_ARG, TRUE, 60000, FALSE))
        .IgnoreArgument(2)
        .SetReturn(WSA_WAIT_EVENT_0);
    STRICT_EXPECTED_CALL(WSACloseEvent(k_handle3_valid))
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(WSACloseEvent(k_handle1_valid))
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(pal_event_handler_cb_mock(k_context_valid, pal_event_type_destroy, er_ok))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(lock_free(k_lock1_valid));
    STRICT_EXPECTED_CALL(lock_free(k_lock2_valid));
    STRICT_EXPECTED_CALL(h_free((void*)&ev_data_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act
    pal_event_close(event_handle_valid, false);

    // assert
    ASSERT_EXPECTED_CALLS();
}

//
// Test pal_event_close happy path
//
TEST_FUNCTION(pal_win_event_close__success_2)
{
    static const uintptr_t k_port_valid = (uintptr_t)0x2345;
    static const intptr_t k_socket_valid = (intptr_t)0xabab;
    static const lock_t k_lock1_valid = (lock_t)0x1;
    static const lock_t k_lock2_valid = (lock_t)0x2;
    static const WSAEVENT k_handle1_valid = (WSAEVENT)0x3;
    static void* k_context_valid = (void*)0x12;
    pal_event_data_t ev_data_valid;
    uintptr_t event_handle_valid = (uintptr_t)&ev_data_valid;

    ev_data_valid.events = (FD_READ | FD_ACCEPT | FD_CLOSE | FD_WRITE);
    ev_data_valid.net_event = k_handle1_valid;
    ev_data_valid.wait_object = INVALID_HANDLE_VALUE;
    ev_data_valid.sock = k_socket_valid;
    ev_data_valid.cb = pal_event_handler_cb_mock;
    ev_data_valid.context = k_context_valid;
    ev_data_valid.read_lock = k_lock1_valid;
    ev_data_valid.write_lock = k_lock2_valid;

    // arrange
    STRICT_EXPECTED_CALL(WSAEventSelect(k_socket_valid, k_handle1_valid, 0))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(WSACloseEvent(k_handle1_valid))
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(pal_event_handler_cb_mock(k_context_valid, pal_event_type_destroy, er_ok))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(lock_free(k_lock1_valid));
    STRICT_EXPECTED_CALL(lock_free(k_lock2_valid));
    STRICT_EXPECTED_CALL(h_free((void*)&ev_data_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act
    pal_event_close(event_handle_valid, false);

    // assert
    ASSERT_EXPECTED_CALLS();
}

//
// Test pal_event_close happy path
//
TEST_FUNCTION(pal_win_event_close__success_3)
{
    static const uintptr_t k_port_valid = (uintptr_t)0x2345;
    static const intptr_t k_socket_valid = (intptr_t)0xabab;
    static const lock_t k_lock1_valid = (lock_t)0x1;
    static const lock_t k_lock2_valid = (lock_t)0x2;
    static const HANDLE k_handle2_valid = (HANDLE)0x4;
    static const WSAEVENT k_handle3_valid = (WSAEVENT)0x5;
    static void* k_context_valid = (void*)0x12;
    pal_event_data_t ev_data_valid;
    uintptr_t event_handle_valid = (uintptr_t)&ev_data_valid;

    ev_data_valid.events = 0;
    ev_data_valid.net_event = INVALID_HANDLE_VALUE;
    ev_data_valid.wait_object = k_handle2_valid;
    ev_data_valid.sock = k_socket_valid;
    ev_data_valid.cb = pal_event_handler_cb_mock;
    ev_data_valid.context = k_context_valid;
    ev_data_valid.read_lock = k_lock1_valid;
    ev_data_valid.write_lock = k_lock2_valid;

    // arrange
    STRICT_EXPECTED_CALL(WSACreateEvent())
        .SetReturn(k_handle3_valid);
    STRICT_EXPECTED_CALL(UnregisterWaitEx(k_handle2_valid, k_handle3_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(WSAWaitForMultipleEvents(1, IGNORED_PTR_ARG, TRUE, 60000, FALSE))
        .IgnoreArgument(2)
        .SetReturn(WSA_WAIT_EVENT_0);
    STRICT_EXPECTED_CALL(WSACloseEvent(k_handle3_valid))
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(pal_event_handler_cb_mock(k_context_valid, pal_event_type_destroy, er_ok))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(lock_free(k_lock1_valid));
    STRICT_EXPECTED_CALL(lock_free(k_lock2_valid));
    STRICT_EXPECTED_CALL(h_free((void*)&ev_data_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act
    pal_event_close(event_handle_valid, false);

    // assert
    ASSERT_EXPECTED_CALLS();
}
//
// Test pal_event_close passing as event_handle argument an invalid uintptr_t value
//
TEST_FUNCTION(pal_win_event_close__arg_event_handle_null)
{
    // arrange

    // act
    pal_event_close(0, false);

    // assert
    ASSERT_EXPECTED_CALLS();
}


//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

