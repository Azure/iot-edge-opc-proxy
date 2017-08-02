// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST pal_ev_poll
#include "util_ut.h"

//
// 1. Required mocks
//
#include "os.h"
#include "pal_sk.h"

MOCKABLE_FUNCTION(, int, poll,
    struct pollfd*, fds, int, nfds, int, timeout);
MOCKABLE_FUNCTION(, int, fcntl,
    fd_t, s, int, cmd, int, val);
MOCKABLE_FUNCTION(, int, socketpair,
    int, domain, int, type, int, protocol, int*, sv);
MOCKABLE_FUNCTION(, sockssize_t, recv,
    fd_t, s, sockbuf_t*, buf, socksize_t, len, int, flags);
MOCKABLE_FUNCTION(, sockssize_t, send,
    fd_t, s, const sockbuf_t*, buf, socksize_t, len, int, flags);
MOCKABLE_FUNCTION(, int, closesocket,
    fd_t, fd);

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
REGISTER_UMOCK_ALIAS_TYPE(fd_t, int);
REGISTER_UMOCK_ALIAS_TYPE(sockssize_t, int);
REGISTER_UMOCK_ALIAS_TYPE(socksize_t, int);
REGISTER_UMOCK_ALIAS_TYPE(uintptr_t, void*);
REGISTER_UMOCK_ALIAS_TYPE(THREADAPI_RESULT, int);
REGISTER_UMOCK_ALIAS_TYPE(THREAD_HANDLE, void*);
REGISTER_UMOCK_ALIAS_TYPE(THREAD_START_FUNC, void*);
REGISTER_UMOCK_ALIAS_TYPE(PDLIST_ENTRY, void*);
REGISTER_UMOCK_ALIAS_TYPE(const PDLIST_ENTRY, const void*);
END_DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
DECLARE_TEST_SETUP()

//
// Test pal_event_port_create happy path
//
TEST_FUNCTION(pal_nix_poll_event_port_create__success)
{
    static const lock_t k_lock_valid = (lock_t)0x1;
    static const fd_t k_valid_fd = 3243;
    static const fd_t k_control_valid = (fd_t)3443;
    static const THREAD_HANDLE k_valid_thread_handle = (THREAD_HANDLE)0x234;
    uintptr_t port_valid;
    fd_t pipe_valid[2];
    int32_t result;

    pipe_valid[0] = k_valid_fd;
    pipe_valid[1] = k_control_valid;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange
    STRICT_EXPECTED_CALL(h_realloc(sizeof(pal_poll_port_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(DList_InitializeListHead(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(lock_create(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_created(&k_lock_valid, sizeof(k_lock_valid))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(socketpair(AF_UNIX, SOCK_STREAM, 0, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(4, pipe_valid, sizeof(pipe_valid))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(send(k_valid_fd, IGNORED_PTR_ARG, 1, 0))
        .IgnoreArgument(2)
        .SetReturn(1)
        .SetFailReturn(0);
    STRICT_EXPECTED_CALL(ThreadAPI_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, (void*)UT_MEM))
        .IgnoreArgument(2)
        .CopyOutArgumentBuffer_threadHandle(&k_valid_thread_handle, sizeof(k_valid_thread_handle))
        .SetReturn(THREADAPI_OK);

    // act
    result = pal_event_port_create(NULL, NULL, &port_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

//
// Test pal_event_port_create passing as port argument an invalid uintptr_t* value
//
TEST_FUNCTION(pal_nix_poll_event_port_create__arg_port_null)
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
// Test pal_event_port_create unhappy path
//
TEST_FUNCTION(pal_nix_poll_event_port_create__neg)
{
    static const lock_t k_lock_valid = (lock_t)0x1;
    static const fd_t k_valid_fd = 3243;
    static const fd_t k_control_socket_valid = (fd_t)3443;
    static const THREAD_HANDLE k_valid_thread_handle = (THREAD_HANDLE)0x234;
    uintptr_t port_valid;
    int32_t result;

    REGISTER_GLOBAL_MOCK_RETURNS(pal_os_last_error_as_prx_error, er_fatal, er_fatal);

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(h_realloc(sizeof(pal_poll_port_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(DList_InitializeListHead(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(lock_create(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_created(&k_lock_valid, sizeof(k_lock_valid))
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL(send(k_valid_fd, IGNORED_PTR_ARG, 1, 0))
        .IgnoreArgument(2)
        .SetReturn(1)
        .SetFailReturn(0);
    STRICT_EXPECTED_CALL(ThreadAPI_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, (void*)UT_MEM))
        .IgnoreArgument(2)
        .CopyOutArgumentBuffer_threadHandle(&k_valid_thread_handle, sizeof(k_valid_thread_handle))
        .SetReturn(THREADAPI_OK)
        .SetFailReturn(THREADAPI_ERROR);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    memset(UT_MEM, 0, sizeof(UT_MEM));
    result = pal_event_port_create(NULL, NULL, &port_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result,
        er_out_of_memory, er_fatal, er_out_of_memory, er_fatal, er_fatal);
}

//
// Test pal_event_port_close happy path
//
TEST_FUNCTION(pal_nix_poll_event_port_close__success_1)
{
    static const fd_t k_valid_fd = 11111;
    static const THREAD_HANDLE k_valid_thread_handle = (THREAD_HANDLE)0x33333;
    static const int k_thread_result_valid = 0;
    pal_poll_port_t port_valid;
    uintptr_t k_port_valid = (uintptr_t)&port_valid;

    port_valid.lock = (lock_t)0x1;
    port_valid.control_fd[0] = k_valid_fd;
    port_valid.control_fd[1] = _invalid_fd;
    port_valid.thread = k_valid_thread_handle;
    port_valid.poll_buffer = (struct pollfd*)UT_MEM;
    port_valid.running = true;

    // arrange
    STRICT_EXPECTED_CALL(send(k_valid_fd, IGNORED_PTR_ARG, 1, 0))
        .IgnoreArgument(2)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(ThreadAPI_Join(k_valid_thread_handle, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_res(&k_thread_result_valid, sizeof(k_thread_result_valid))
        .SetReturn(THREADAPI_OK);
    STRICT_EXPECTED_CALL(closesocket(k_valid_fd))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(h_free((void*)UT_MEM, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);
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
TEST_FUNCTION(pal_nix_poll_event_port_close__success_2)
{
    static const fd_t k_invalid_fd = _invalid_fd;
    static const THREAD_HANDLE k_valid_thread_handle = (THREAD_HANDLE)0x33333;
    static const int k_thread_result_valid = 0;
    pal_poll_port_t port_valid;
    uintptr_t k_port_valid = (uintptr_t)&port_valid;

    port_valid.lock = (lock_t)0x1;
    port_valid.control_fd[0] = k_invalid_fd;
    port_valid.control_fd[1] = k_invalid_fd;
    port_valid.thread = k_valid_thread_handle;
    port_valid.poll_buffer = NULL;
    port_valid.running = true;

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
TEST_FUNCTION(pal_nix_poll_event_port_close__arg_port_invalid)
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
TEST_FUNCTION(pal_nix_poll_event_port_register__success)
{
    static const fd_t k_socket_valid = (fd_t)0xabab;
    static void* k_context_valid = (void*)0x12;
    static const fd_t k_valid_fd = 11111;
    pal_poll_port_t port_valid;
    uintptr_t k_port_valid = (uintptr_t)&port_valid;
    pal_poll_event_t ev_data_valid;
    uintptr_t event_handle_valid;
    int32_t result;

    port_valid.lock = (lock_t)0x1;
    port_valid.control_fd[0] = k_valid_fd;
    port_valid.control_fd[1] = 0;

    // arrange
    STRICT_EXPECTED_CALL(h_realloc(sizeof(pal_poll_event_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)&ev_data_valid);
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));
    STRICT_EXPECTED_CALL(DList_InsertTailList(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(1).IgnoreArgument(2);
    STRICT_EXPECTED_CALL(send(k_valid_fd, IGNORED_PTR_ARG, 1, 0))
        .IgnoreArgument(2)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));

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
TEST_FUNCTION(pal_nix_poll_event_port_register__arg_port_invalid)
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
TEST_FUNCTION(pal_nix_poll_event_port_register__arg_socket_invalid)
{
    static void* k_context_valid = (void*)0x12;
    pal_poll_port_t port_valid;
    uintptr_t k_port_valid = (uintptr_t)&port_valid;
    uintptr_t event_handle_valid;
    int32_t result;

    // arrange

    // act
    result = pal_event_port_register(k_port_valid, _invalid_fd, pal_event_handler_cb_mock, k_context_valid, &event_handle_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_arg, result);
}

//
// Test pal_event_port_register passing as cb argument an invalid pal_event_handler_t value
//
TEST_FUNCTION(pal_nix_poll_event_port_register__arg_cb_invalid)
{
    static const fd_t k_socket_valid = (fd_t)0xabab;
    static void* k_context_valid = (void*)0x12;
    pal_poll_port_t port_valid;
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
TEST_FUNCTION(pal_nix_poll_event_port_register__arg_event_handle_invalid)
{
    static const fd_t k_socket_valid = (fd_t)0xabab;
    static void* k_context_valid = (void*)0x12;
    pal_poll_port_t port_valid;
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
TEST_FUNCTION(pal_nix_poll_event_port_register__neg)
{
    static const fd_t k_socket_valid = (fd_t)0xabab;
    static void* k_context_valid = (void*)0x12;
    static const fd_t k_valid_fd = 11111;
    pal_poll_port_t port_valid;
    uintptr_t k_port_valid = (uintptr_t)&port_valid;
    pal_poll_event_t ev_data_valid;
    uintptr_t event_handle_valid;
    int32_t result;

    port_valid.lock = (lock_t)0x1;
    port_valid.control_fd[0] = k_valid_fd;
    port_valid.control_fd[1] = 0;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(h_realloc(sizeof(pal_poll_event_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)&ev_data_valid)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));
    STRICT_EXPECTED_CALL(DList_InsertTailList(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(1).IgnoreArgument(2);
    STRICT_EXPECTED_CALL(send(k_valid_fd, IGNORED_PTR_ARG, 1, 0))
        .IgnoreArgument(2)
        .SetReturn(1)
        .SetFailReturn(0);
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_event_port_register(k_port_valid, k_socket_valid, pal_event_handler_cb_mock, k_context_valid, &event_handle_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result,
        er_out_of_memory, er_ok);
}

//
// Test pal_event_select happy path
//
TEST_FUNCTION(pal_nix_poll_event_select__success_1)
{
    static const fd_t k_socket_valid = (fd_t)0xbaba;
    static const fd_t k_valid_fd = 11111;
    static const pal_event_type_t k_event_type_valid = pal_event_type_write;
    static const uint32_t k_events_expected = POLLWRNORM | POLLRDNORM;
    pal_poll_port_t port_valid;
    pal_poll_event_t ev_data_valid;
    uintptr_t event_handle_valid = (uintptr_t)&ev_data_valid;
    int32_t result;

    port_valid.lock = (lock_t)0x1;
    port_valid.control_fd[0] = k_valid_fd;
    port_valid.control_fd[1] = 0;
    ev_data_valid.poll_struct.events = POLLRDNORM;
    ev_data_valid.poll_struct.fd = k_socket_valid;
    ev_data_valid.port = &port_valid;

    // arrange
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));
    STRICT_EXPECTED_CALL(send(k_valid_fd, IGNORED_PTR_ARG, 1, 0))
        .IgnoreArgument(2)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));

    // act
    result = pal_event_select(event_handle_valid, k_event_type_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(uint32_t, k_events_expected, ev_data_valid.poll_struct.events);
}

//
// Test pal_event_select happy path
//
TEST_FUNCTION(pal_nix_poll_event_select__success_2)
{
    static const fd_t k_socket_valid = (fd_t)0xbaba;
    static const fd_t k_valid_fd = 11111;
    static const pal_event_type_t k_event_type_valid = pal_event_type_read;
    static const uint32_t k_events_expected = POLLWRNORM | POLLRDNORM;
    pal_poll_port_t port_valid;
    pal_poll_event_t ev_data_valid;
    uintptr_t event_handle_valid = (uintptr_t)&ev_data_valid;
    int32_t result;

    port_valid.lock = (lock_t)0x1;
    port_valid.control_fd[0] = k_valid_fd;
    port_valid.control_fd[1] = 0;
    ev_data_valid.poll_struct.events = POLLWRNORM;
    ev_data_valid.poll_struct.fd = k_socket_valid;
    ev_data_valid.port = &port_valid;

    // arrange
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));
    STRICT_EXPECTED_CALL(send(k_valid_fd, IGNORED_PTR_ARG, 1, 0))
        .IgnoreArgument(2)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));

    // act
    result = pal_event_select(event_handle_valid, k_event_type_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(uint32_t, k_events_expected, ev_data_valid.poll_struct.events);
}

//
// Test pal_event_select happy path
//
TEST_FUNCTION(pal_nix_poll_event_select__success_3)
{
    static const fd_t k_socket_valid = (fd_t)0xbaba;
    static const pal_event_type_t k_event_type_valid = pal_event_type_write;
    pal_poll_event_t ev_data_valid;
    uintptr_t event_handle_valid = (uintptr_t)&ev_data_valid;
    int32_t result;

    ev_data_valid.poll_struct.events = POLLWRNORM;

    // arrange

    // act
    result = pal_event_select(event_handle_valid, k_event_type_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

//
// Test pal_event_select passing as event_handle argument an invalid uintptr_t value
//
TEST_FUNCTION(pal_nix_poll_event_select__arg_event_handle_null)
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
TEST_FUNCTION(pal_nix_poll_event_select__arg_event_type_invalid)
{
    static const fd_t k_socket_valid = (fd_t)0xbaba;
    static const pal_event_type_t k_event_type_invalid = (pal_event_type_t)0x44444;
    pal_poll_event_t ev_data_valid;
    uintptr_t event_handle_valid = (uintptr_t)&ev_data_valid;
    int32_t result;

    ev_data_valid.poll_struct.events = POLLWRNORM;

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
TEST_FUNCTION(pal_nix_poll_event_select__neg)
{
    static const fd_t k_socket_valid = (fd_t)0xbaba;
    static const fd_t k_valid_fd = 11111;
    static const pal_event_type_t k_event_type_valid = pal_event_type_write;
    static const uint32_t k_events_expected = POLLWRNORM | POLLRDNORM;
    pal_poll_port_t port_valid;
    pal_poll_event_t ev_data_valid;
    uintptr_t event_handle_valid = (uintptr_t)&ev_data_valid;
    int32_t result;

    port_valid.lock = (lock_t)0x1;
    port_valid.control_fd[0] = k_valid_fd;
    port_valid.control_fd[1] = 0;
    ev_data_valid.port = &port_valid;
    ev_data_valid.poll_struct.fd = k_socket_valid;

    REGISTER_GLOBAL_MOCK_RETURNS(pal_os_last_error_as_prx_error, er_fatal, er_fatal);

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));
    STRICT_EXPECTED_CALL(send(k_valid_fd, IGNORED_PTR_ARG, 1, 0))
        .IgnoreArgument(2)
        .SetReturn(1)
        .SetFailReturn(0);
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    ev_data_valid.poll_struct.events = POLLRDNORM;
    result = pal_event_select(event_handle_valid, k_event_type_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok, er_fatal, er_ok);
}

//
// Test pal_event_clear happy path
//
TEST_FUNCTION(pal_nix_poll_event_clear__success_1)
{
    static const fd_t k_socket_valid = (fd_t)0xbaba;
    static const fd_t k_valid_fd = 11111;
    static const pal_event_type_t k_event_type_valid = pal_event_type_read;
    static const uint32_t k_events_expected = POLLWRNORM;
    pal_poll_port_t port_valid;
    pal_poll_event_t ev_data_valid;
    uintptr_t event_handle_valid = (uintptr_t)&ev_data_valid;
    int32_t result;

    port_valid.lock = (lock_t)0x1;
    port_valid.control_fd[0] = k_valid_fd;
    port_valid.control_fd[1] = 0;
    ev_data_valid.poll_struct.events = POLLWRNORM | POLLRDNORM;
    ev_data_valid.poll_struct.fd = k_socket_valid;
    ev_data_valid.port = &port_valid;

    // arrange
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));
    STRICT_EXPECTED_CALL(send(k_valid_fd, IGNORED_PTR_ARG, 1, 0))
        .IgnoreArgument(2)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));

    // act
    result = pal_event_clear(event_handle_valid, k_event_type_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(uint32_t, k_events_expected, ev_data_valid.poll_struct.events);
}

//
// Test pal_event_clear happy path
//
TEST_FUNCTION(pal_nix_poll_event_clear__success_2)
{
    static const fd_t k_socket_valid = (fd_t)0xbaba;
    static const fd_t k_valid_fd = 11111;
    static const pal_event_type_t k_event_type_valid = pal_event_type_write;
    static const uint32_t k_events_expected = 0;
    pal_poll_port_t port_valid;
    pal_poll_event_t ev_data_valid;
    uintptr_t event_handle_valid = (uintptr_t)&ev_data_valid;
    int32_t result;

    port_valid.lock = (lock_t)0x1;
    port_valid.control_fd[0] = k_valid_fd;
    port_valid.control_fd[1] = 0;
    ev_data_valid.poll_struct.events = POLLWRNORM;
    ev_data_valid.poll_struct.fd = k_socket_valid;
    ev_data_valid.port = &port_valid;

    // arrange
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));
    STRICT_EXPECTED_CALL(send(k_valid_fd, IGNORED_PTR_ARG, 1, 0))
        .IgnoreArgument(2)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));

    // act
    result = pal_event_clear(event_handle_valid, k_event_type_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(uint32_t, k_events_expected, ev_data_valid.poll_struct.events);
}

//
// Test pal_event_clear happy path
//
TEST_FUNCTION(pal_nix_poll_event_clear__success_3)
{
    static const pal_event_type_t k_event_type_valid = pal_event_type_write;
    pal_poll_event_t ev_data_valid;
    uintptr_t event_handle_valid = (uintptr_t)&ev_data_valid;
    int32_t result;

    ev_data_valid.poll_struct.events = POLLRDNORM;

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
TEST_FUNCTION(pal_nix_poll_event_clear__arg_event_handle_null)
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
TEST_FUNCTION(pal_nix_poll_event_clear__arg_event_type_invalid)
{
    static const fd_t k_socket_valid = (fd_t)0xbaba;
    static const pal_event_type_t k_event_type_invalid = (pal_event_type_t)0x99999;
    pal_poll_event_t ev_data_valid;
    uintptr_t event_handle_valid = (uintptr_t)&ev_data_valid;
    int32_t result;

    ev_data_valid.poll_struct.events = POLLWRNORM;

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
TEST_FUNCTION(pal_nix_poll_event_clear__neg)
{
    static const fd_t k_socket_valid = (fd_t)0xbaba;
    static const fd_t k_valid_fd = 11111;
    static const pal_event_type_t k_event_type_valid = pal_event_type_read;
    pal_poll_port_t port_valid;
    pal_poll_event_t ev_data_valid;
    uintptr_t event_handle_valid = (uintptr_t)&ev_data_valid;
    int32_t result;

    port_valid.lock = (lock_t)0x1;
    port_valid.control_fd[0] = k_valid_fd;
    port_valid.control_fd[1] = 0;
    ev_data_valid.poll_struct.fd = k_socket_valid;
    ev_data_valid.port = &port_valid;

    REGISTER_GLOBAL_MOCK_RETURNS(pal_os_last_error_as_prx_error, er_fatal, er_fatal);

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));
    STRICT_EXPECTED_CALL(send(k_valid_fd, IGNORED_PTR_ARG, 1, 0))
        .IgnoreArgument(2)
        .SetReturn(1)
        .SetFailReturn(0);
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    ev_data_valid.poll_struct.events = POLLRDNORM | POLLWRNORM;
    result = pal_event_clear(event_handle_valid, k_event_type_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok, er_fatal, er_ok);
}

//
// Test pal_event_close happy path
//
TEST_FUNCTION(pal_nix_poll_event_close__success)
{
    static const THREAD_HANDLE k_valid_thread_handle = (THREAD_HANDLE)0x33333;
    static const fd_t k_socket_valid = (fd_t)0xbaba;
    static const int k_valid_fd = 1234;
    static const uint32_t k_events_expected = 0;
    pal_poll_port_t port_valid;
    pal_poll_event_t ev_data_valid;
    uintptr_t event_handle_valid = (uintptr_t)&ev_data_valid;

    port_valid.lock = (lock_t)0x1;
    port_valid.thread = k_valid_thread_handle;
    port_valid.control_fd[0] = k_valid_fd;
    port_valid.control_fd[1] = 0;

    ev_data_valid.poll_struct.fd = k_socket_valid;
    ev_data_valid.poll_struct.events = POLLRDNORM | POLLWRNORM;
    ev_data_valid.port = &port_valid;

    // arrange
    STRICT_EXPECTED_CALL(lock_enter((lock_t)0x1));
    STRICT_EXPECTED_CALL(send(k_valid_fd, IGNORED_PTR_ARG, 1, 0))
        .IgnoreArgument(2)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(lock_exit((lock_t)0x1));

    // act
    pal_event_close(event_handle_valid, true);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(uint32_t, k_events_expected, ev_data_valid.poll_struct.events);
    ASSERT_IS_NULL(ev_data_valid.port);
}
//
// Test pal_event_close passing as event_handle argument an invalid uintptr_t value
//
TEST_FUNCTION(pal_nix_poll_event_close__arg_event_handle_null)
{
    // arrange

    // act
    pal_event_close(0, true);

    // assert
    ASSERT_EXPECTED_CALLS();
}

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

