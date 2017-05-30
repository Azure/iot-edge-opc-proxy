// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST pal_ev_epoll
#include "util_ut.h"

//
// 1. Required mocks
//
#include "os.h"
#include "pal_sk.h"

// sys/epoll.h
MOCKABLE_FUNCTION(, int, epoll_wait, 
    int, epfd, struct epoll_event*, events, int, maxevents, int, timeout);
MOCKABLE_FUNCTION(, int, epoll_create1, 
    int, flags);
MOCKABLE_FUNCTION(, int, epoll_ctl, 
    int, epfd, int, op, int, fd, struct epoll_event*, event);
// sys/socket.h
MOCKABLE_FUNCTION(, int, socket,
    int, domain, int, type, int, protocol);
// unistd.h
MOCKABLE_FUNCTION(, int, close,
    int, fd);
MOCKABLE_FUNCTION(, int, fcntl,
    fd_t, s, int, cmd, int, val);


//
// 2. Include unit under test
//
#undef ENABLE_MOCKS
#include "pal_ev.h"
#define ENABLE_MOCKS
#include UNIT_C

MOCKABLE_FUNCTION(, int32_t, pal_event_handler_cb_mock,
    void*, context, pal_event_type, event_type, int32_t, error_code);

//
// 3. Setup test suite
//
BEGIN_DECLARE_TEST_SUITE()
REGISTER_UMOCK_ALIAS_TYPE(lock_t, void*);
REGISTER_UMOCK_ALIAS_TYPE(pal_event_type, int);
REGISTER_UMOCK_ALIAS_TYPE(uintptr_t, void*);
REGISTER_UMOCK_ALIAS_TYPE(THREADAPI_RESULT, int);
REGISTER_UMOCK_ALIAS_TYPE(THREAD_HANDLE, void*);
REGISTER_UMOCK_ALIAS_TYPE(THREAD_START_FUNC, void*);
END_DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
DECLARE_TEST_SETUP()

// 
// Test pal_event_port_create happy path 
// 
TEST_FUNCTION(pal_nix_epoll_event_port_create__success)
{
    static const lock_t k_lock_valid = (lock_t)0x1;
    static const int k_valid_fd = 3243;
    static const THREAD_HANDLE k_valid_thread_handle = (THREAD_HANDLE)0x234;
    uintptr_t port_valid;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(pal_epoll_port_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(lock_create(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_created(&k_lock_valid, sizeof(k_lock_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(epoll_create1(EPOLL_CLOEXEC))
        .SetReturn(k_valid_fd);
    STRICT_EXPECTED_CALL(ThreadAPI_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, (void*)UT_MEM))
        .IgnoreArgument(2)
        .CopyOutArgumentBuffer_threadHandle(&k_valid_thread_handle, sizeof(k_valid_thread_handle))
        .SetReturn(THREADAPI_OK);

    // act 
    result = pal_event_port_create(&port_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_event_port_create passing as port argument an invalid uintptr_t* value 
// 
TEST_FUNCTION(pal_nix_epoll_event_port_create__arg_port_null)
{
    int32_t result;

    // arrange 

    // act 
    result = pal_event_port_create(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_event_port_create unhappy path 
// 
TEST_FUNCTION(pal_nix_epoll_event_port_create__neg)
{
    static const lock_t k_lock_valid = (lock_t)0x1;
    static const int k_valid_fd = 3243;
    static const THREAD_HANDLE k_valid_thread_handle = (THREAD_HANDLE)0x234;
    uintptr_t port_valid;
    int32_t result;

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(h_realloc(sizeof(pal_epoll_port_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(lock_create(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_created(&k_lock_valid, sizeof(k_lock_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL(epoll_create1(EPOLL_CLOEXEC))
        .SetReturn(k_valid_fd)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(ThreadAPI_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, (void*)UT_MEM))
        .IgnoreArgument(2)
        .CopyOutArgumentBuffer_threadHandle(&k_valid_thread_handle, sizeof(k_valid_thread_handle))
        .SetReturn(THREADAPI_OK)
        .SetFailReturn(THREADAPI_ERROR);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    memset(UT_MEM, 0, sizeof(UT_MEM));
    result = pal_event_port_create(&port_valid);

    // assert    
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, 
        er_out_of_memory, er_out_of_memory, er_out_of_memory, er_fatal, er_ok);
}

// 
// Test pal_event_port_close happy path 
// 
TEST_FUNCTION(pal_nix_epoll_event_port_close__success_1)
{
    static const int k_valid_fd = 11111;
    static const THREAD_HANDLE k_valid_thread_handle = (THREAD_HANDLE)0x33333;
    static const int k_thread_result_valid = 0;
    static const uint32_t k_events_expected1 = EPOLLOUT | EPOLLONESHOT;
    static const uint32_t k_events_expected2 = 0;
    pal_epoll_port_t port_valid;
    uintptr_t k_port_valid = (uintptr_t)&port_valid;

    port_valid.lock = (lock_t)0x1;
    port_valid.epoll_fd = k_valid_fd;
    port_valid.thread = k_valid_thread_handle;

    // arrange 
    STRICT_EXPECTED_CALL(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))
        .SetReturn(22222);
    STRICT_EXPECTED_CALL(epoll_ctl(k_valid_fd, EPOLL_CTL_ADD, 22222, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(4, &k_events_expected1, sizeof(k_events_expected1))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(ThreadAPI_Join(k_valid_thread_handle, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_res(&k_thread_result_valid, sizeof(k_thread_result_valid))
        .SetReturn(THREADAPI_OK);
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));
    STRICT_EXPECTED_CALL(epoll_ctl(k_valid_fd, EPOLL_CTL_DEL, 22222, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(4, &k_events_expected2, sizeof(k_events_expected2))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(close(k_valid_fd))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));
    STRICT_EXPECTED_CALL(close(22222))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(lock_free((lock_t)0x1));
    STRICT_EXPECTED_CALL(h_free((void*)&port_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    pal_event_port_close(k_port_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_event_port_close happy path 
// 
TEST_FUNCTION(pal_nix_epoll_event_port_close__success_2)
{
    static const int k_invalid_fd = -1;
    static const THREAD_HANDLE k_valid_thread_handle = (THREAD_HANDLE)0x33333;
    static const int k_thread_result_valid = 0;
    static const uint32_t k_events_expected1 = EPOLLOUT | EPOLLONESHOT;
    static const uint32_t k_events_expected2 = 0;
    pal_epoll_port_t port_valid;
    uintptr_t k_port_valid = (uintptr_t)&port_valid;

    port_valid.lock = (lock_t)0x1;
    port_valid.epoll_fd = k_invalid_fd;
    port_valid.thread = k_valid_thread_handle;

    // arrange 
    STRICT_EXPECTED_CALL(ThreadAPI_Join(k_valid_thread_handle, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_res(&k_thread_result_valid, sizeof(k_thread_result_valid))
        .SetReturn(THREADAPI_OK);
    STRICT_EXPECTED_CALL(lock_free((lock_t)0x1));
    STRICT_EXPECTED_CALL(h_free((void*)&port_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    pal_event_port_close(k_port_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_event_port_close passing as port argument an invalid uintptr_t value 
// 
TEST_FUNCTION(pal_nix_epoll_event_port_close__arg_port_invalid)
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
TEST_FUNCTION(pal_nix_epoll_event_port_register__success)
{
    static const fd_t k_socket_valid = (fd_t)0xabab;
    static void* k_context_valid = (void*)0x12;
    static const uint32_t k_events_expected = EPOLLET;
    pal_epoll_port_t port_valid;
    uintptr_t k_port_valid = (uintptr_t)&port_valid;
    pal_epoll_event_t ev_data_valid;
    uintptr_t event_handle_valid;
    int32_t result;

    port_valid.lock = (lock_t)0x1;
    port_valid.epoll_fd = 256;

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(pal_epoll_event_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)&ev_data_valid);
    STRICT_EXPECTED_CALL(epoll_ctl(256, EPOLL_CTL_ADD, k_socket_valid, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(4, &k_events_expected, sizeof(k_events_expected))
        .SetReturn(0);

    // act 
    result = pal_event_port_register(k_port_valid, k_socket_valid, pal_event_handler_cb_mock, k_context_valid, &event_handle_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_IS_TRUE(event_handle_valid == (uintptr_t)&ev_data_valid);
}

// 
// Test pal_event_port_register passing as port argument an invalid uintptr_t value 
// 
TEST_FUNCTION(pal_nix_epoll_event_port_register__arg_port_invalid)
{
    static const fd_t k_socket_valid = (fd_t)0xabab;
    static void* k_context_valid = (void*)0x12;
    uintptr_t event_handle_valid;
    int32_t result;

    // arrange 

    // act 
    result = pal_event_port_register(0, k_socket_valid, pal_event_handler_cb_mock, k_context_valid, &event_handle_valid);

    // assert 
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_event_port_register passing as socket argument an invalid fd_t value 
// 
TEST_FUNCTION(pal_nix_epoll_event_port_register__arg_socket_invalid)
{
    static void* k_context_valid = (void*)0x12;
    pal_epoll_port_t port_valid;
    uintptr_t k_port_valid = (uintptr_t)&port_valid;
    uintptr_t event_handle_valid;
    int32_t result;

    // arrange 

    // act 
    result = pal_event_port_register(k_port_valid, prx_invalid_socket, pal_event_handler_cb_mock, k_context_valid, &event_handle_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_arg, result);
}

// 
// Test pal_event_port_register passing as cb argument an invalid pal_event_handler_t value 
// 
TEST_FUNCTION(pal_nix_epoll_event_port_register__arg_cb_invalid)
{
    static const fd_t k_socket_valid = (fd_t)0xabab;
    static void* k_context_valid = (void*)0x12;
    pal_epoll_port_t port_valid;
    uintptr_t k_port_valid = (uintptr_t)&port_valid;
    uintptr_t event_handle_valid;
    int32_t result;

    // arrange 

    // act 
    result = pal_event_port_register(k_port_valid, k_socket_valid, NULL, k_context_valid, &event_handle_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_event_port_register passing as event_handle argument an invalid uintptr_t* value 
// 
TEST_FUNCTION(pal_nix_epoll_event_port_register__arg_event_handle_invalid)
{
    static const fd_t k_socket_valid = (fd_t)0xabab;
    static void* k_context_valid = (void*)0x12;
    pal_epoll_port_t port_valid;
    uintptr_t k_port_valid = (uintptr_t)&port_valid;
    int32_t result;

    // arrange 

    // act 
    result = pal_event_port_register(k_port_valid, k_socket_valid, pal_event_handler_cb_mock, k_context_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_event_port_register unhappy path 
// 
TEST_FUNCTION(pal_nix_epoll_event_port_register__neg)
{
    static const fd_t k_socket_valid = (fd_t)0xabab;
    static void* k_context_valid = (void*)0x12;
    static const uint32_t k_events_expected = EPOLLET;
    pal_epoll_port_t port_valid;
    uintptr_t k_port_valid = (uintptr_t)&port_valid;
    pal_epoll_event_t ev_data_valid;
    uintptr_t event_handle_valid;
    int32_t result;

    port_valid.lock = (lock_t)0x1;
    port_valid.epoll_fd = 256;

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(h_realloc(sizeof(pal_epoll_event_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)&ev_data_valid)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(epoll_ctl(256, EPOLL_CTL_ADD, k_socket_valid, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(4, &k_events_expected, sizeof(k_events_expected))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(pal_os_last_net_error_as_prx_error())
        .SetReturn(er_out_of_memory)
        .SetFailReturn(er_out_of_memory);

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_event_port_register(k_port_valid, k_socket_valid, pal_event_handler_cb_mock, k_context_valid, &event_handle_valid);

    // assert    
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result,
        er_out_of_memory, er_out_of_memory, er_ok);
}

// 
// Test pal_event_select happy path 
// 
TEST_FUNCTION(pal_nix_epoll_event_select__success_1)
{
    static const fd_t k_socket_valid = (fd_t)0xbaba;
    static const pal_event_type k_event_type_valid = pal_event_type_write;
    static const uint32_t k_events_expected = EPOLLOUT | EPOLLIN | EPOLLET;
    pal_epoll_port_t port_valid;
    pal_epoll_event_t ev_data_valid;
    uintptr_t event_handle_valid = (uintptr_t)&ev_data_valid;
    int32_t result;

    port_valid.lock = (lock_t)0x1;
    port_valid.epoll_fd = 2;
    ev_data_valid.events = EPOLLIN;
    ev_data_valid.port = &port_valid;
    ev_data_valid.sock_fd = k_socket_valid;

    // arrange 
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));
    STRICT_EXPECTED_CALL(epoll_ctl(2, EPOLL_CTL_MOD, k_socket_valid, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(4, &k_events_expected, sizeof(k_events_expected))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));

    // act 
    result = pal_event_select(event_handle_valid, k_event_type_valid);

    // assert 
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_event_select happy path 
// 
TEST_FUNCTION(pal_nix_epoll_event_select__success_2)
{
    static const fd_t k_socket_valid = (fd_t)0xbaba;
    static const pal_event_type k_event_type_valid = pal_event_type_read;
    static const uint32_t k_events_expected = EPOLLOUT | EPOLLIN | EPOLLET;
    pal_epoll_port_t port_valid;
    pal_epoll_event_t ev_data_valid;
    uintptr_t event_handle_valid = (uintptr_t)&ev_data_valid;
    int32_t result;

    port_valid.lock = (lock_t)0x1;
    port_valid.epoll_fd = 2;
    ev_data_valid.events = EPOLLOUT;
    ev_data_valid.port = &port_valid;
    ev_data_valid.sock_fd = k_socket_valid;

    // arrange 
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));
    STRICT_EXPECTED_CALL(epoll_ctl(2, EPOLL_CTL_MOD, k_socket_valid, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(4, &k_events_expected, sizeof(k_events_expected))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));

    // act 
    result = pal_event_select(event_handle_valid, k_event_type_valid);

    // assert 
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_event_select passing as event_handle argument an invalid uintptr_t value 
// 
TEST_FUNCTION(pal_nix_epoll_event_select__arg_event_handle_null)
{
    const pal_event_type k_event_type_valid = pal_event_type_read;
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
// Test pal_event_select passing as event_type argument an invalid pal_event_type value 
// 
TEST_FUNCTION(pal_nix_epoll_event_select__arg_event_type_invalid)
{
    static const fd_t k_socket_valid = (fd_t)0xbaba;
    static const pal_event_type k_event_type_invalid = (pal_event_type)0x44444;
    pal_epoll_event_t ev_data_valid;
    uintptr_t event_handle_valid = (uintptr_t)&ev_data_valid;
    int32_t result;

    ev_data_valid.events = EPOLLOUT;

    // arrange 

    // act 
    result = pal_event_select(event_handle_valid, k_event_type_invalid);

    // assert 
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_event_select unhappy path 
// 
TEST_FUNCTION(pal_nix_epoll_event_select__neg)
{
    static const fd_t k_socket_valid = (fd_t)0xbaba;
    static const pal_event_type k_event_type_valid = pal_event_type_write;
    static const uint32_t k_events_expected = EPOLLOUT | EPOLLIN | EPOLLET;
    pal_epoll_port_t port_valid;
    pal_epoll_event_t ev_data_valid;
    uintptr_t event_handle_valid = (uintptr_t)&ev_data_valid;
    int32_t result;

    port_valid.lock = (lock_t)0x1;
    port_valid.epoll_fd = 2;
    ev_data_valid.port = &port_valid;
    ev_data_valid.sock_fd = k_socket_valid;

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));
    STRICT_EXPECTED_CALL(epoll_ctl(2, EPOLL_CTL_MOD, k_socket_valid, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(4, &k_events_expected, sizeof(k_events_expected))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(pal_os_last_net_error_as_prx_error())
        .SetReturn(er_fatal)
        .SetFailReturn(er_fatal);
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    ev_data_valid.events = EPOLLIN;
    result = pal_event_select(event_handle_valid, k_event_type_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok, er_fatal, er_ok);
}

// 
// Test pal_event_clear happy path 
// 
TEST_FUNCTION(pal_nix_epoll_event_clear__success_1)
{
    static const fd_t k_socket_valid = (fd_t)0xbaba;
    static const pal_event_type k_event_type_valid = pal_event_type_read;
    static const uint32_t k_events_expected = EPOLLET | EPOLLOUT;
    pal_epoll_port_t port_valid;
    pal_epoll_event_t ev_data_valid;
    uintptr_t event_handle_valid = (uintptr_t)&ev_data_valid;
    int32_t result;

    port_valid.lock = (lock_t)0x1;
    port_valid.epoll_fd = 3;
    ev_data_valid.events = EPOLLOUT | EPOLLIN;
    ev_data_valid.port = &port_valid;
    ev_data_valid.sock_fd = k_socket_valid;

    // arrange 
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));
    STRICT_EXPECTED_CALL(epoll_ctl(3, EPOLL_CTL_MOD, k_socket_valid, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(4, &k_events_expected, sizeof(k_events_expected))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));

    // act 
    result = pal_event_clear(event_handle_valid, k_event_type_valid);

    // assert 
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_event_clear happy path 
// 
TEST_FUNCTION(pal_nix_epoll_event_clear__success_2)
{
    static const fd_t k_socket_valid = (fd_t)0xbaba;
    static const pal_event_type k_event_type_valid = pal_event_type_write;
    static const uint32_t k_events_expected = EPOLLET;
    pal_epoll_port_t port_valid;
    pal_epoll_event_t ev_data_valid;
    uintptr_t event_handle_valid = (uintptr_t)&ev_data_valid;
    int32_t result;

    port_valid.lock = (lock_t)0x1;
    port_valid.epoll_fd = 3;
    ev_data_valid.events = EPOLLOUT;
    ev_data_valid.port = &port_valid;
    ev_data_valid.sock_fd = k_socket_valid;

    // arrange 
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));
    STRICT_EXPECTED_CALL(epoll_ctl(3, EPOLL_CTL_MOD, k_socket_valid, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(4, &k_events_expected, sizeof(k_events_expected))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));

    // act 
    result = pal_event_clear(event_handle_valid, k_event_type_valid);

    // assert 
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_event_clear passing as event_handle argument an invalid uintptr_t value 
// 
TEST_FUNCTION(pal_nix_epoll_event_clear__arg_event_handle_null)
{
    const pal_event_type k_event_type_valid = pal_event_type_read;
    int32_t result;

    // arrange 

    // act 
    result = pal_event_clear(0, k_event_type_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_event_clear passing as event_type argument an invalid pal_event_type value 
// 
TEST_FUNCTION(pal_nix_epoll_event_clear__arg_event_type_invalid)
{
    static const fd_t k_socket_valid = (fd_t)0xbaba;
    static const pal_event_type k_event_type_invalid = (pal_event_type)0x245;
    pal_epoll_event_t ev_data_valid;
    uintptr_t event_handle_valid = (uintptr_t)&ev_data_valid;
    int32_t result;

    ev_data_valid.events = EPOLLIN;

    // arrange 

    // act 
    result = pal_event_clear(event_handle_valid, k_event_type_invalid);

    // assert 
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_event_clear unhappy path 
// 
TEST_FUNCTION(pal_nix_epoll_event_clear__neg)
{
    static const fd_t k_socket_valid = (fd_t)0xbaba;
    static const pal_event_type k_event_type_valid = pal_event_type_read;
    static const uint32_t k_events_expected = EPOLLET | EPOLLOUT;
    pal_epoll_port_t port_valid;
    pal_epoll_event_t ev_data_valid;
    uintptr_t event_handle_valid = (uintptr_t)&ev_data_valid;
    int32_t result;

    port_valid.lock = (lock_t)0x1;
    port_valid.epoll_fd = 3;
    ev_data_valid.port = &port_valid;
    ev_data_valid.sock_fd = k_socket_valid;

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));
    STRICT_EXPECTED_CALL(epoll_ctl(3, EPOLL_CTL_MOD, k_socket_valid, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(4, &k_events_expected, sizeof(k_events_expected))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(pal_os_last_net_error_as_prx_error())
        .SetReturn(er_fatal)
        .SetFailReturn(er_fatal);
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    ev_data_valid.events = EPOLLOUT | EPOLLIN;
    result = pal_event_clear(event_handle_valid, k_event_type_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok, er_fatal, er_ok);
}

// 
// Test pal_event_close happy path 
// 
TEST_FUNCTION(pal_nix_epoll_event_close__success)
{
    static const fd_t k_socket_valid = (fd_t)0xbaba;
    static void* k_context_valid = (void*)0x254;
    static const int k_valid_fd = 1234;
    static const uint32_t k_events_expected = 0;
    pal_epoll_port_t port_valid;
    pal_epoll_event_t ev_data_valid;
    uintptr_t event_handle_valid = (uintptr_t)&ev_data_valid;

    port_valid.lock = (lock_t)0x1;
    port_valid.epoll_fd = k_valid_fd;

    ev_data_valid.events = EPOLLIN;
    ev_data_valid.port = &port_valid;
    ev_data_valid.sock_fd = k_socket_valid;
    ev_data_valid.cb = pal_event_handler_cb_mock;
    ev_data_valid.context = k_context_valid;

    // arrange 
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));
    STRICT_EXPECTED_CALL(epoll_ctl(k_valid_fd, EPOLL_CTL_DEL, k_socket_valid, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(4, &k_events_expected, sizeof(k_events_expected))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));
    STRICT_EXPECTED_CALL(close(k_socket_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(pal_event_handler_cb_mock(k_context_valid, pal_event_type_destroy, 0))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(h_free((void*)&ev_data_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    pal_event_close(event_handle_valid, true);

    // assert 
    ASSERT_EXPECTED_CALLS();
}
// 
// Test pal_event_close passing as event_handle argument an invalid uintptr_t value 
// 
TEST_FUNCTION(pal_nix_epoll_event_close__arg_event_handle_null)
{
    // arrange 

    // act 
    pal_event_close(0, true);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_event_close unhappy path 
// 
TEST_FUNCTION(pal_nix_epoll_event_close__neg)
{
    static const fd_t k_socket_valid = (fd_t)0xbaba;
    static void* k_context_valid = (void*)0x254;
    static const int k_valid_fd = 8888;
    static const uint32_t k_events_expected = 0;
    pal_epoll_port_t port_valid;
    pal_epoll_event_t ev_data_valid;
    uintptr_t event_handle_valid = (uintptr_t)&ev_data_valid;

    port_valid.lock = (lock_t)0x1;
    port_valid.epoll_fd = k_valid_fd;

    ev_data_valid.events = EPOLLIN;
    ev_data_valid.port = &port_valid;
    ev_data_valid.sock_fd = k_socket_valid;
    ev_data_valid.cb = pal_event_handler_cb_mock;
    ev_data_valid.context = k_context_valid;

    // arrange 
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));
    STRICT_EXPECTED_CALL(epoll_ctl(k_valid_fd, EPOLL_CTL_DEL, k_socket_valid, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(4, &k_events_expected, sizeof(k_events_expected))
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));
    STRICT_EXPECTED_CALL(close(k_socket_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(pal_event_handler_cb_mock(k_context_valid, pal_event_type_destroy, 0))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(h_free((void*)&ev_data_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    pal_event_close(event_handle_valid, true);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

