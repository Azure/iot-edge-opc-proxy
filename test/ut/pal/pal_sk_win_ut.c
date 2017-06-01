// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST pal_sk_win
#include "util_ut.h"

//
// 1. Required mocks
//
#include "os.h"
#include "prx_types.h"

// winsock2.h
MOCKABLE_FUNCTION(WSAAPI, int, WSAStartup,
    WORD, wVersionRequested, LPWSADATA, lpWSAData);
MOCKABLE_FUNCTION(WSAAPI, SOCKET, WSASocket, 
    int, family, int, type, int, protocol, LPWSAPROTOCOL_INFO, lpProtocolInfo, 
    GROUP, g, DWORD, dwFlags);
MOCKABLE_FUNCTION(WSAAPI, int, ioctlsocket,
    SOCKET, s, long, cmd, u_long*, argp);
MOCKABLE_FUNCTION(WSAAPI, int, shutdown,
    SOCKET, s, int, how);
MOCKABLE_FUNCTION(WSAAPI, int, getsockopt,
    SOCKET, s, int, optlevel, int, optname, sockbuf_t*, optval, socklen_t*, optlen);
MOCKABLE_FUNCTION(WSAAPI, int, setsockopt,
    SOCKET, s, int, optlevel, int, optname, const sockbuf_t*, optval, socklen_t, optlen);
MOCKABLE_FUNCTION(WSAAPI, int, bind,
    SOCKET, s, const struct sockaddr*, name, socklen_t, namelen);
MOCKABLE_FUNCTION(WSAAPI, int, listen,
    SOCKET, s, int, backlog);
MOCKABLE_FUNCTION(WSAAPI, int, accept,
    SOCKET, s, const struct sockaddr*, addr, socklen_t*, addrlen);
MOCKABLE_FUNCTION(WSAAPI, int, connect,
    SOCKET, s, const struct sockaddr*, name, socklen_t, namelen);
MOCKABLE_FUNCTION(WSAAPI, int, getsockname,
    SOCKET, s, struct sockaddr*, name, socklen_t*, namelen);
MOCKABLE_FUNCTION(WSAAPI, int, getpeername,
    SOCKET, s, struct sockaddr*, name, socklen_t*, namelen);
MOCKABLE_FUNCTION(WSAAPI, int, closesocket,
    SOCKET, s);
MOCKABLE_FUNCTION(WSAAPI, int, WSARecv, SOCKET, s, LPWSABUF, lpBuffers,
    DWORD, dwBufferCount, LPDWORD, lpNumberOfBytesRecvd, LPDWORD, lpFlags, 
    LPWSAOVERLAPPED, lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE, lpCompletionRoutine);
MOCKABLE_FUNCTION(WSAAPI, int, WSARecvFrom, SOCKET, s, LPWSABUF, lpBuffers, 
    DWORD, dwBufferCount, LPDWORD, lpNumberOfBytesRecvd, LPDWORD, lpFlags, 
    struct sockaddr*, lpFrom, int*, lpFromlen, LPWSAOVERLAPPED, lpOverlapped, 
    LPWSAOVERLAPPED_COMPLETION_ROUTINE, lpCompletionRoutine);
MOCKABLE_FUNCTION(WSAAPI, int, WSASend, SOCKET, s, LPWSABUF, lpBuffers, DWORD, dwBufferCount,
    LPDWORD, lpNumberOfBytesSent, DWORD, dwFlags, LPWSAOVERLAPPED, lpOverlapped, 
    LPWSAOVERLAPPED_COMPLETION_ROUTINE, lpCompletionRoutine);
MOCKABLE_FUNCTION(WSAAPI, int, WSASendTo, SOCKET, s, LPWSABUF, lpBuffers, DWORD, dwBufferCount, 
    LPDWORD, lpNumberOfBytesSent, DWORD, dwFlags, const struct sockaddr*, lpTo, int, iTolen, 
    LPWSAOVERLAPPED, lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE, lpCompletionRoutine);
MOCKABLE_FUNCTION(WSAAPI, int, WSAIoctl, SOCKET, s, DWORD, dwIoControlCode, LPVOID, lpvInBuffer,
    DWORD, cbInBuffer, LPVOID, lpvOutBuffer, DWORD, cbOutBuffer, LPDWORD, lpcbBytesReturned,
    LPWSAOVERLAPPED, lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE, lpCompletionRoutine);
MOCKABLE_FUNCTION(WSAAPI, BOOL, AcceptEx, SOCKET, sListenSocket, SOCKET, sAcceptSocket,
    PVOID, lpOutputBuffer, DWORD, dwReceiveDataLength, DWORD, dwLocalAddressLength,
    DWORD, dwRemoteAddressLength, LPDWORD, lpdwBytesReceived, LPOVERLAPPED, lpOverlapped);
MOCKABLE_FUNCTION(WSAAPI, BOOL, ConnectEx, SOCKET, s, const struct sockaddr*, name, int, namelen,
    PVOID, lpSendBuffer, DWORD, dwSendDataLength, LPDWORD, lpdwBytesSent, LPOVERLAPPED, lpOverlapped);
MOCKABLE_FUNCTION(WSAAPI, void, GetAcceptExSockAddrs, PVOID, lpOutputBuffer, DWORD, dwReceiveDataLength,
    DWORD, dwLocalAddressLength, DWORD, dwRemoteAddressLength, struct sockaddr**,LocalSockaddr,
    int*, LocalSockaddrLength, struct sockaddr**, RemoteSockaddr, int*, RemoteSockaddrLength);
MOCKABLE_FUNCTION(WSAAPI, int, WSAGetLastError);
MOCKABLE_FUNCTION(WSAAPI, void, WSASetLastError,
    int, iError);
MOCKABLE_FUNCTION(WSAAPI, int, WSACleanup);
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
MOCKABLE_FUNCTION(WINAPI, BOOL, BindIoCompletionCallback,
    HANDLE, FileHandle, LPOVERLAPPED_COMPLETION_ROUTINE, Function, ULONG, Flags);
MOCKABLE_FUNCTION(WINAPI, HANDLE, CreateFileA,
    LPCSTR, lpFileName, DWORD, dwDesiredAccess, DWORD, dwShareMode, LPSECURITY_ATTRIBUTES, lpSecurityAttributes, 
    DWORD, dwCreationDisposition, DWORD, dwFlagsAndAttributes, HANDLE, hTemplateFile);
MOCKABLE_FUNCTION(WINAPI, BOOL, CloseHandle,
    HANDLE, hHandle);
MOCKABLE_FUNCTION(WINAPI, DWORD, GetLastError);
// synchapi.h
MOCKABLE_FUNCTION(WINAPI, BOOL, WaitForSingleObject,
    HANDLE, hHandle, DWORD, dwMilliseconds);
MOCKABLE_FUNCTION(WINAPI, HANDLE, CreateEventA,
    LPSECURITY_ATTRIBUTES, lpEventAttributes, BOOL, bManualReset, BOOL, bInitialState, LPCSTR, lpName);
// fileapi.h
MOCKABLE_FUNCTION(WINAPI, BOOL, ReadFile,
    HANDLE, hFile, LPVOID, lpBuffer, DWORD, nNumberOfBytesToRead, LPDWORD, lpNumberOfBytesRead,
    LPOVERLAPPED, lpOverlapped);
MOCKABLE_FUNCTION(WINAPI, BOOL, WriteFile,
    HANDLE, hFile, LPCVOID, lpBuffer, DWORD, nNumberOfBytesToWrite, LPDWORD, lpNumberOfBytesWritten,
    LPOVERLAPPED, lpOverlapped);
// ioapiset.h
MOCKABLE_FUNCTION(WINAPI, BOOL, CancelIoEx, 
    HANDLE, hFile, LPOVERLAPPED, lpOverlapped);
// namedpipeapi.h
MOCKABLE_FUNCTION(WINAPI, BOOL, CreatePipe,
    PHANDLE, hReadPipe, PHANDLE, hWritePipe, LPSECURITY_ATTRIBUTES, lpPipeAttributes, DWORD, nSize);
MOCKABLE_FUNCTION(WINAPI, BOOL, ConnectNamedPipe,
    HANDLE, hNamedPipe, LPOVERLAPPED, lpOverlapped);
MOCKABLE_FUNCTION(WINAPI, BOOL, DisconnectNamedPipe,
    HANDLE, hNamedPipe);
MOCKABLE_FUNCTION(WINAPI, BOOL, SetNamedPipeHandleState,
    HANDLE, hNamedPipe, LPDWORD, lpMode, LPDWORD, lpMaxCollectionCount, LPDWORD, lpCollectDataTimeout);
MOCKABLE_FUNCTION(WINAPI, BOOL, PeekNamedPipe,
    HANDLE, hNamedPipe, LPVOID, lpBuffer, DWORD, nBufferSize, LPDWORD, lpBytesRead,
    LPDWORD, lpTotalBytesAvail, LPDWORD, lpBytesLeftThisMessage);
MOCKABLE_FUNCTION(WINAPI, HANDLE, CreateNamedPipeA,
    LPCSTR, lpName,DWORD, dwOpenMode, DWORD, dwPipeMode, DWORD, nMaxInstances, DWORD, nOutBufferSize,
    DWORD, nInBufferSize, DWORD, nDefaultTimeOut, LPSECURITY_ATTRIBUTES, lpSecurityAttributes);
// Custom
MOCKABLE_FUNCTION(WINAPI, BOOL, HasOverlappedIoCompleted,
    LPOVERLAPPED, lpOverlapped);

//
// 2. Include unit under test
//
#undef ENABLE_MOCKS
#include "pal_sk.h"
#define ENABLE_MOCKS
#include UNIT_C

//
// 3. Setup test suite
//
BEGIN_DECLARE_TEST_SUITE()
REGISTER_UMOCK_ALIAS_TYPE(ULONG, unsigned long);
REGISTER_UMOCK_ALIAS_TYPE(PULONG, void*);
REGISTER_UMOCK_ALIAS_TYPE(DWORD, unsigned int);
REGISTER_UMOCK_ALIAS_TYPE(LPDWORD, void*);
REGISTER_UMOCK_ALIAS_TYPE(WORD, unsigned short);
REGISTER_UMOCK_ALIAS_TYPE(HANDLE, void*);
REGISTER_UMOCK_ALIAS_TYPE(PHANDLE, void*);
REGISTER_UMOCK_ALIAS_TYPE(socklen_t, int);
REGISTER_UMOCK_ALIAS_TYPE(LPWSAOVERLAPPED, void*);
REGISTER_UMOCK_ALIAS_TYPE(LPOVERLAPPED, void*);
REGISTER_UMOCK_ALIAS_TYPE(LPWSAOVERLAPPED_COMPLETION_ROUTINE, void*);
REGISTER_UMOCK_ALIAS_TYPE(PIP_ADAPTER_ADDRESSES, void*);
REGISTER_UMOCK_ALIAS_TYPE(SOCKET, void*);
REGISTER_UMOCK_ALIAS_TYPE(PVOID, void*);
REGISTER_UMOCK_ALIAS_TYPE(LPVOID, void*);
REGISTER_UMOCK_ALIAS_TYPE(LPSTR, char*);
REGISTER_UMOCK_ALIAS_TYPE(LPCSTR, char*);
REGISTER_UMOCK_ALIAS_TYPE(BOOL, bool);
REGISTER_UMOCK_ALIAS_TYPE(LPCVOID, void*);
REGISTER_UMOCK_ALIAS_TYPE(HLOCAL, void*);
REGISTER_UMOCK_ALIAS_TYPE(HMODULE, void*);
END_DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
DECLARE_TEST_SETUP()


#ifdef pal_socket_create

//
//Test pal_socket_create happy path 
// 
TEST_FUNCTION(pal_socket_create__success)
{
    static const pal_socket_client_itf_t* k_itf_valid;
    static const pal_socket_t** k_sock_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = pal_socket_create(k_itf_valid, k_sock_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test pal_socket_create passing as itf argument an invalid pal_socket_client_itf_t* value 
// 
TEST_FUNCTION(pal_socket_create__arg_itf_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_socket_create();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_socket_create passing as sock argument an invalid pal_socket_t** value 
// 
TEST_FUNCTION(pal_socket_create__arg_sock_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_socket_create();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_socket_create unhappy path 
// 
TEST_FUNCTION(pal_socket_create__neg)
{
    static const pal_socket_client_itf_t* k_itf_valid;
    static const pal_socket_t** k_sock_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_socket_create(k_itf_valid, k_sock_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // pal_socket_create;

#ifdef pal_socket_open

//
//Test pal_socket_open happy path 
// 
TEST_FUNCTION(pal_socket_open__success)
{
    static const pal_socket_t* k_sock_valid;
    static const void* k_op_context_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = pal_socket_open(k_sock_valid, k_op_context_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test pal_socket_open passing as sock argument an invalid pal_socket_t* value 
// 
TEST_FUNCTION(pal_socket_open__arg_sock_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_socket_open();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_socket_open passing as op_context argument an invalid void* value 
// 
TEST_FUNCTION(pal_socket_open__arg_op_context_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_socket_open();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_socket_open unhappy path 
// 
TEST_FUNCTION(pal_socket_open__neg)
{
    static const pal_socket_t* k_sock_valid;
    static const void* k_op_context_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_socket_open(k_sock_valid, k_op_context_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // pal_socket_open;

#ifdef pal_socket_get_properties

//
//Test pal_socket_get_properties happy path 
// 
TEST_FUNCTION(pal_socket_get_properties__success)
{
    static const pal_socket_t* k_sock_valid;
    static const prx_socket_properties_t* k_props_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = pal_socket_get_properties(k_sock_valid, k_props_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test pal_socket_get_properties passing as sock argument an invalid pal_socket_t* value 
// 
TEST_FUNCTION(pal_socket_get_properties__arg_sock_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_socket_get_properties();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_socket_get_properties passing as props argument an invalid prx_socket_properties_t* value 
// 
TEST_FUNCTION(pal_socket_get_properties__arg_props_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_socket_get_properties();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_socket_get_properties unhappy path 
// 
TEST_FUNCTION(pal_socket_get_properties__neg)
{
    static const pal_socket_t* k_sock_valid;
    static const prx_socket_properties_t* k_props_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_socket_get_properties(k_sock_valid, k_props_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // pal_socket_get_properties;

#ifdef pal_socket_getpeername

//
//Test pal_socket_getpeername happy path 
// 
TEST_FUNCTION(pal_socket_getpeername__success)
{
    static const pal_socket_t* k_sock_valid;
    static const prx_socket_address_t* k_socket_address_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = pal_socket_getpeername(k_sock_valid, k_socket_address_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test pal_socket_getpeername passing as sock argument an invalid pal_socket_t* value 
// 
TEST_FUNCTION(pal_socket_getpeername__arg_sock_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_socket_getpeername();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_socket_getpeername passing as socket_address argument an invalid prx_socket_address_t* value 
// 
TEST_FUNCTION(pal_socket_getpeername__arg_socket_address_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_socket_getpeername();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_socket_getpeername unhappy path 
// 
TEST_FUNCTION(pal_socket_getpeername__neg)
{
    static const pal_socket_t* k_sock_valid;
    static const prx_socket_address_t* k_socket_address_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_socket_getpeername(k_sock_valid, k_socket_address_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // pal_socket_getpeername;

#ifdef pal_socket_getsockname

//
//Test pal_socket_getsockname happy path 
// 
TEST_FUNCTION(pal_socket_getsockname__success)
{
    static const pal_socket_t* k_sock_valid;
    static const prx_socket_address_t* k_socket_address_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = pal_socket_getsockname(k_sock_valid, k_socket_address_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test pal_socket_getsockname passing as sock argument an invalid pal_socket_t* value 
// 
TEST_FUNCTION(pal_socket_getsockname__arg_sock_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_socket_getsockname();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_socket_getsockname passing as socket_address argument an invalid prx_socket_address_t* value 
// 
TEST_FUNCTION(pal_socket_getsockname__arg_socket_address_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_socket_getsockname();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_socket_getsockname unhappy path 
// 
TEST_FUNCTION(pal_socket_getsockname__neg)
{
    static const pal_socket_t* k_sock_valid;
    static const prx_socket_address_t* k_socket_address_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_socket_getsockname(k_sock_valid, k_socket_address_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // pal_socket_getsockname;

#ifdef pal_socket_can_send

//
//Test pal_socket_can_send happy path 
// 
TEST_FUNCTION(pal_socket_can_send__success)
{
    static const pal_socket_t* k_sock_valid;
    static const bool k_ready_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = pal_socket_can_send(k_sock_valid, k_ready_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test pal_socket_can_send passing as sock argument an invalid pal_socket_t* value 
// 
TEST_FUNCTION(pal_socket_can_send__arg_sock_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_socket_can_send();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_socket_can_send passing as ready argument an invalid bool value 
// 
TEST_FUNCTION(pal_socket_can_send__arg_ready_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_socket_can_send();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_socket_can_send unhappy path 
// 
TEST_FUNCTION(pal_socket_can_send__neg)
{
    static const pal_socket_t* k_sock_valid;
    static const bool k_ready_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_socket_can_send(k_sock_valid, k_ready_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // pal_socket_can_send;

#ifdef pal_socket_can_recv

//
//Test pal_socket_can_recv happy path 
// 
TEST_FUNCTION(pal_socket_can_recv__success)
{
    static const pal_socket_t* k_sock_valid;
    static const bool k_ready_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = pal_socket_can_recv(k_sock_valid, k_ready_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test pal_socket_can_recv passing as sock argument an invalid pal_socket_t* value 
// 
TEST_FUNCTION(pal_socket_can_recv__arg_sock_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_socket_can_recv();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_socket_can_recv passing as ready argument an invalid bool value 
// 
TEST_FUNCTION(pal_socket_can_recv__arg_ready_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_socket_can_recv();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_socket_can_recv unhappy path 
// 
TEST_FUNCTION(pal_socket_can_recv__neg)
{
    static const pal_socket_t* k_sock_valid;
    static const bool k_ready_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_socket_can_recv(k_sock_valid, k_ready_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // pal_socket_can_recv;

#ifdef pal_socket_setsockopt

//
//Test pal_socket_setsockopt happy path 
// 
TEST_FUNCTION(pal_socket_setsockopt__success)
{
    static const pal_socket_t* k_sock_valid;
    static const prx_socket_option_t k_socket_option_valid;
    static const uint64_t k_value_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = pal_socket_setsockopt(k_sock_valid, k_socket_option_valid, k_value_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test pal_socket_setsockopt passing as sock argument an invalid pal_socket_t* value 
// 
TEST_FUNCTION(pal_socket_setsockopt__arg_sock_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_socket_setsockopt();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_socket_setsockopt passing as socket_option argument an invalid prx_socket_option_t value 
// 
TEST_FUNCTION(pal_socket_setsockopt__arg_socket_option_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_socket_setsockopt();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_socket_setsockopt passing as value argument an invalid uint64_t value 
// 
TEST_FUNCTION(pal_socket_setsockopt__arg_value_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_socket_setsockopt();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_socket_setsockopt unhappy path 
// 
TEST_FUNCTION(pal_socket_setsockopt__neg)
{
    static const pal_socket_t* k_sock_valid;
    static const prx_socket_option_t k_socket_option_valid;
    static const uint64_t k_value_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_socket_setsockopt(k_sock_valid, k_socket_option_valid, k_value_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // pal_socket_setsockopt;

#ifdef pal_socket_getsockopt

//
//Test pal_socket_getsockopt happy path 
// 
TEST_FUNCTION(pal_socket_getsockopt__success)
{
    static const pal_socket_t* k_sock_valid;
    static const prx_socket_option_t k_socket_option_valid;
    static const uint64_t* k_value_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = pal_socket_getsockopt(k_sock_valid, k_socket_option_valid, k_value_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test pal_socket_getsockopt passing as sock argument an invalid pal_socket_t* value 
// 
TEST_FUNCTION(pal_socket_getsockopt__arg_sock_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_socket_getsockopt();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_socket_getsockopt passing as socket_option argument an invalid prx_socket_option_t value 
// 
TEST_FUNCTION(pal_socket_getsockopt__arg_socket_option_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_socket_getsockopt();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_socket_getsockopt passing as value argument an invalid uint64_t* value 
// 
TEST_FUNCTION(pal_socket_getsockopt__arg_value_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_socket_getsockopt();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_socket_getsockopt unhappy path 
// 
TEST_FUNCTION(pal_socket_getsockopt__neg)
{
    static const pal_socket_t* k_sock_valid;
    static const prx_socket_option_t k_socket_option_valid;
    static const uint64_t* k_value_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_socket_getsockopt(k_sock_valid, k_socket_option_valid, k_value_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // pal_socket_getsockopt;

#ifdef pal_socket_join_multicast_group

//
//Test pal_socket_join_multicast_group happy path 
// 
TEST_FUNCTION(pal_socket_join_multicast_group__success)
{
    static const pal_socket_t* k_sock_valid;
    static const prx_multicast_option_t* k_option_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = pal_socket_join_multicast_group(k_sock_valid, k_option_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test pal_socket_join_multicast_group passing as sock argument an invalid pal_socket_t* value 
// 
TEST_FUNCTION(pal_socket_join_multicast_group__arg_sock_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_socket_join_multicast_group();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_socket_join_multicast_group passing as option argument an invalid prx_multicast_option_t* value 
// 
TEST_FUNCTION(pal_socket_join_multicast_group__arg_option_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_socket_join_multicast_group();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_socket_join_multicast_group unhappy path 
// 
TEST_FUNCTION(pal_socket_join_multicast_group__neg)
{
    static const pal_socket_t* k_sock_valid;
    static const prx_multicast_option_t* k_option_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_socket_join_multicast_group(k_sock_valid, k_option_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // pal_socket_join_multicast_group;

#ifdef pal_socket_leave_multicast_group

//
//Test pal_socket_leave_multicast_group happy path 
// 
TEST_FUNCTION(pal_socket_leave_multicast_group__success)
{
    static const pal_socket_t* k_sock_valid;
    static const prx_multicast_option_t* k_option_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = pal_socket_leave_multicast_group(k_sock_valid, k_option_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test pal_socket_leave_multicast_group passing as sock argument an invalid pal_socket_t* value 
// 
TEST_FUNCTION(pal_socket_leave_multicast_group__arg_sock_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_socket_leave_multicast_group();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_socket_leave_multicast_group passing as option argument an invalid prx_multicast_option_t* value 
// 
TEST_FUNCTION(pal_socket_leave_multicast_group__arg_option_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_socket_leave_multicast_group();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_socket_leave_multicast_group unhappy path 
// 
TEST_FUNCTION(pal_socket_leave_multicast_group__neg)
{
    static const pal_socket_t* k_sock_valid;
    static const prx_multicast_option_t* k_option_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_socket_leave_multicast_group(k_sock_valid, k_option_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // pal_socket_leave_multicast_group;

#ifdef pal_socket_close

//
//Test pal_socket_close happy path 
// 
TEST_FUNCTION(pal_socket_close__success)
{
    static const pal_socket_t* k_socket_valid;
    void result;

    // arrange 
    // ... 

    // act 
    result = pal_socket_close(k_socket_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ... 
}

// 
// Test pal_socket_close passing as socket argument an invalid pal_socket_t* value 
// 
TEST_FUNCTION(pal_socket_close__arg_socket_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_socket_close();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_socket_close unhappy path 
// 
TEST_FUNCTION(pal_socket_close__neg)
{
    static const pal_socket_t* k_socket_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_socket_close(k_socket_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // pal_socket_close;

#ifdef pal_socket_free

//
//Test pal_socket_free happy path 
// 
TEST_FUNCTION(pal_socket_free__success)
{
    static const pal_socket_t* k_sock_valid;
    void result;

    // arrange 
    // ... 

    // act 
    result = pal_socket_free(k_sock_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ... 
}

// 
// Test pal_socket_free passing as sock argument an invalid pal_socket_t* value 
// 
TEST_FUNCTION(pal_socket_free__arg_sock_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_socket_free();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_socket_free unhappy path 
// 
TEST_FUNCTION(pal_socket_free__neg)
{
    static const pal_socket_t* k_sock_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_socket_free(k_sock_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // pal_socket_free;

#ifdef pal_socket_init

//
//Test pal_socket_init happy path 
// 
TEST_FUNCTION(pal_socket_init__success)
{
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = pal_socket_init();

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test pal_socket_init unhappy path 
// 
TEST_FUNCTION(pal_socket_init__neg)
{
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_socket_init();

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // pal_socket_init;

#ifdef pal_socket_deinit

//
//Test pal_socket_deinit happy path 
// 
TEST_FUNCTION(pal_socket_deinit__success)
{
    void result;

    // arrange 
    // ... 

    // act 
    result = pal_socket_deinit();

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ... 
}

// 
// Test pal_socket_deinit unhappy path 
// 
TEST_FUNCTION(pal_socket_deinit__neg)
{
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_socket_deinit();

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // pal_socket_deinit;


#if 0
// 
// Test pal_getifaddrinfo happy path 
// 
TEST_FUNCTION(pal_win_getifaddrinfo__success_1)
{
    static const char* k_if_name_valid = "def";
    IP_ADAPTER_ADDRESSES address_valid[4];
    IP_ADAPTER_UNICAST_ADDRESS unicast_addresses_valid[3];
    struct sockaddr_in6 sock_addr_in6_valid;
    struct sockaddr* sock_addr_valid = (struct sockaddr*)&sock_addr_in6_valid;
    const unsigned long k_addresses_size_valid = _countof(address_valid);
    prx_ifaddrinfo_t* info_valid;
    size_t info_count_valid;
    int32_t result;

    UT_MEM_ALLOCED = malloc(20000);

    sock_addr_valid->sa_family = AF_INET6;

    memset(unicast_addresses_valid, 0, sizeof(unicast_addresses_valid));
    unicast_addresses_valid[0].Address.lpSockaddr = sock_addr_valid;
    unicast_addresses_valid[0].Next = &unicast_addresses_valid[1];
    unicast_addresses_valid[1].Address.lpSockaddr = sock_addr_valid;
    unicast_addresses_valid[1].Next = &unicast_addresses_valid[2];
    unicast_addresses_valid[2].Address.lpSockaddr = sock_addr_valid;

    memset(address_valid, 0, sizeof(address_valid));
    address_valid[0].AdapterName = "abc";
    address_valid[0].Next = &address_valid[1];
    address_valid[1].AdapterName = "def";
    address_valid[1].FirstUnicastAddress = &unicast_addresses_valid[0];
    address_valid[1].Next = &address_valid[2];
    address_valid[2].AdapterName = "ghi";
    address_valid[2].Next = &address_valid[3];
    address_valid[3].AdapterName = "jkl";

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(15000, IGNORED_PTR_ARG, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)address_valid);
    STRICT_EXPECTED_CALL(GetAdaptersAddresses(AF_UNSPEC, IGNORED_NUM_ARG, NULL, (PIP_ADAPTER_ADDRESSES)address_valid, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .CopyOutArgumentBuffer_SizePointer(&k_addresses_size_valid, sizeof(k_addresses_size_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(string_compare("abc", k_if_name_valid))
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(string_compare("def", k_if_name_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(string_compare("ghi", k_if_name_valid))
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(string_compare("jkl", k_if_name_valid))
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(h_realloc(4 * sizeof(prx_ifaddrinfo_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM_ALLOCED);
    STRICT_EXPECTED_CALL(string_compare("abc", k_if_name_valid))
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(string_compare("def", k_if_name_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(pal_os_to_prx_socket_address(
        unicast_addresses_valid[0].Address.lpSockaddr, unicast_addresses_valid[0].Address.iSockaddrLength, IGNORED_PTR_ARG))
        .IgnoreArgument(3)
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_os_to_prx_socket_address(
        unicast_addresses_valid[1].Address.lpSockaddr, unicast_addresses_valid[1].Address.iSockaddrLength, IGNORED_PTR_ARG))
        .IgnoreArgument(3)
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(pal_os_to_prx_socket_address(
        unicast_addresses_valid[2].Address.lpSockaddr, unicast_addresses_valid[2].Address.iSockaddrLength, IGNORED_PTR_ARG))
        .IgnoreArgument(3)
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(string_compare("ghi", k_if_name_valid))
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(string_compare("jkl", k_if_name_valid))
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(h_free((void*)address_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    result = pal_getifaddrinfo(k_if_name_valid, 0, &info_valid, &info_count_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_IS_TRUE(info_count_valid == 3);
}

// 
// Test pal_getifaddrinfo happy path 
// 
TEST_FUNCTION(pal_win_getifaddrinfo__success_2)
{
    IP_ADAPTER_ADDRESSES address_valid[4];
    IP_ADAPTER_UNICAST_ADDRESS unicast_addresses_valid[3];
    struct sockaddr_in6 sock_addr_in6_valid;
    struct sockaddr* sock_addr_valid = (struct sockaddr*)&sock_addr_in6_valid;
    const unsigned long k_addresses_size_valid = _countof(address_valid);
    prx_ifaddrinfo_t* info_valid;
    size_t info_count_valid;
    int32_t result;

    UT_MEM_ALLOCED = malloc(40000);

    sock_addr_valid->sa_family = AF_INET6;

    memset(unicast_addresses_valid, 0, sizeof(unicast_addresses_valid));
    unicast_addresses_valid[0].Address.lpSockaddr = sock_addr_valid;
    unicast_addresses_valid[0].Next = &unicast_addresses_valid[1];
    unicast_addresses_valid[1].Address.lpSockaddr = sock_addr_valid;
    unicast_addresses_valid[1].Next = &unicast_addresses_valid[2];
    unicast_addresses_valid[2].Address.lpSockaddr = sock_addr_valid;

    memset(address_valid, 0, sizeof(address_valid));
    address_valid[0].AdapterName = "abc";
    address_valid[0].FirstUnicastAddress = &unicast_addresses_valid[0];
    address_valid[0].Next = &address_valid[1];
    address_valid[1].AdapterName = "def";
    address_valid[1].FirstUnicastAddress = &unicast_addresses_valid[0];
    address_valid[1].Next = &address_valid[2];
    address_valid[2].AdapterName = "ghi";
    address_valid[2].FirstUnicastAddress = &unicast_addresses_valid[0];
    address_valid[2].Next = &address_valid[3];
    address_valid[3].AdapterName = "jkl";
    address_valid[3].FirstUnicastAddress = &unicast_addresses_valid[0];

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(15000, IGNORED_PTR_ARG, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)address_valid);
    STRICT_EXPECTED_CALL(GetAdaptersAddresses(AF_UNSPEC, IGNORED_NUM_ARG, NULL, (PIP_ADAPTER_ADDRESSES)address_valid, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .CopyOutArgumentBuffer_SizePointer(&k_addresses_size_valid, sizeof(k_addresses_size_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(h_realloc(13 * sizeof(prx_ifaddrinfo_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM_ALLOCED);
    for (size_t i = 0; i < _countof(address_valid); i++)
    {
        STRICT_EXPECTED_CALL(pal_os_to_prx_socket_address(
            unicast_addresses_valid[0].Address.lpSockaddr, unicast_addresses_valid[0].Address.iSockaddrLength, IGNORED_PTR_ARG))
            .IgnoreArgument(3)
            .SetReturn(er_ok);
        STRICT_EXPECTED_CALL(pal_os_to_prx_socket_address(
            unicast_addresses_valid[1].Address.lpSockaddr, unicast_addresses_valid[1].Address.iSockaddrLength, IGNORED_PTR_ARG))
            .IgnoreArgument(3)
            .SetReturn(er_ok);
        STRICT_EXPECTED_CALL(pal_os_to_prx_socket_address(
            unicast_addresses_valid[2].Address.lpSockaddr, unicast_addresses_valid[2].Address.iSockaddrLength, IGNORED_PTR_ARG))
            .IgnoreArgument(3)
            .SetReturn(er_ok);
    }
    STRICT_EXPECTED_CALL(h_free((void*)address_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    result = pal_getifaddrinfo(NULL, 0, &info_valid, &info_count_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_IS_TRUE(info_count_valid == 12);
}

// 
// Test pal_getifaddrinfo passing as info argument an invalid prx_ifaddrinfo_t** value 
// 
TEST_FUNCTION(pal_win_getifaddrinfo__arg_info_null)
{
    static const char* k_if_name_valid = "ggs";
    size_t info_count_valid;
    int32_t result;

    // arrange 

    // act 
    result = pal_getifaddrinfo(k_if_name_valid, 0, NULL, &info_count_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_getifaddrinfo passing as info_count argument an invalid size_t* value 
// 
TEST_FUNCTION(pal_win_getifaddrinfo__arg_info_count_null)
{
    static const char* k_if_name_valid = "fff";
    prx_ifaddrinfo_t* info_valid;
    int32_t result;

    // arrange 

    // act 
    result = pal_getifaddrinfo(k_if_name_valid, 0, &info_valid, NULL);

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_getifaddrinfo unhappy path 
// 
TEST_FUNCTION(pal_win_getifaddrinfo__neg)
{
    IP_ADAPTER_ADDRESSES address_valid[4];
    IP_ADAPTER_UNICAST_ADDRESS unicast_addresses_valid[3];
    struct sockaddr_in6 sock_addr_in6_valid;
    struct sockaddr* sock_addr_valid = (struct sockaddr*)&sock_addr_in6_valid;
    const unsigned long k_addresses_size_valid = _countof(address_valid);
    prx_ifaddrinfo_t* info_valid;
    size_t info_count_valid;
    int32_t result;

    UT_MEM_ALLOCED = malloc(40000); // 13 prx_ifaddr structs a 2.3 k

    sock_addr_valid->sa_family = AF_INET6;

    memset(unicast_addresses_valid, 0, sizeof(unicast_addresses_valid));
    unicast_addresses_valid[0].Address.lpSockaddr = sock_addr_valid;
    unicast_addresses_valid[0].Next = &unicast_addresses_valid[1];
    unicast_addresses_valid[1].Address.lpSockaddr = sock_addr_valid;
    unicast_addresses_valid[1].Next = &unicast_addresses_valid[2];
    unicast_addresses_valid[2].Address.lpSockaddr = sock_addr_valid;

    memset(address_valid, 0, sizeof(address_valid));
    address_valid[0].AdapterName = "abc";
    address_valid[0].FirstUnicastAddress = &unicast_addresses_valid[0];
    address_valid[0].Next = &address_valid[1];
    address_valid[1].AdapterName = "def";
    address_valid[1].FirstUnicastAddress = &unicast_addresses_valid[0];
    address_valid[1].Next = &address_valid[2];
    address_valid[2].AdapterName = "ghi";
    address_valid[2].FirstUnicastAddress = &unicast_addresses_valid[0];
    address_valid[2].Next = &address_valid[3];
    address_valid[3].AdapterName = "jkl";
    address_valid[3].FirstUnicastAddress = &unicast_addresses_valid[0];

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(h_realloc(15000, IGNORED_PTR_ARG, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)address_valid)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(GetAdaptersAddresses(AF_UNSPEC, IGNORED_NUM_ARG, NULL, (PIP_ADAPTER_ADDRESSES)address_valid, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .CopyOutArgumentBuffer_SizePointer(&k_addresses_size_valid, sizeof(k_addresses_size_valid))
        .SetReturn((ULONG)0)
        .SetFailReturn((ULONG)-1);
    STRICT_EXPECTED_CALL(h_realloc(13 * sizeof(prx_ifaddrinfo_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM_ALLOCED)
        .SetFailReturn(NULL);
    for (size_t i = 0; i < _countof(address_valid); i++)
    {
        STRICT_EXPECTED_CALL(pal_os_to_prx_socket_address(
            unicast_addresses_valid[0].Address.lpSockaddr, unicast_addresses_valid[0].Address.iSockaddrLength, IGNORED_PTR_ARG))
            .IgnoreArgument(3)
            .SetReturn(er_ok)
            .SetFailReturn(er_invalid_format);
        STRICT_EXPECTED_CALL(pal_os_to_prx_socket_address(
            unicast_addresses_valid[1].Address.lpSockaddr, unicast_addresses_valid[1].Address.iSockaddrLength, IGNORED_PTR_ARG))
            .IgnoreArgument(3)
            .SetReturn(er_ok)
            .SetFailReturn(er_invalid_format);
        STRICT_EXPECTED_CALL(pal_os_to_prx_socket_address(
            unicast_addresses_valid[2].Address.lpSockaddr, unicast_addresses_valid[2].Address.iSockaddrLength, IGNORED_PTR_ARG))
            .IgnoreArgument(3)
            .SetReturn(er_ok)
            .SetFailReturn(er_invalid_format);
    }
    STRICT_EXPECTED_CALL(h_free((void*)address_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_getifaddrinfo(NULL, 0, &info_valid, &info_count_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_out_of_memory, er_ok, er_out_of_memory, er_ok);
}

// 
// Test pal_freeifaddrinfo happy path 
// 
TEST_FUNCTION(pal_win_freeifaddrinfo__success)
{
    static prx_ifaddrinfo_t* k_info_valid = (prx_ifaddrinfo_t*)0x2624;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(h_free((void*)k_info_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    result = pal_freeifaddrinfo(k_info_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_freeifaddrinfo passing as info argument an invalid prx_ifaddrinfo_t* value 
// 
TEST_FUNCTION(pal_win_freeifaddrinfo__arg_info_null)
{
    int32_t result;

    // arrange 

    // act 
    result = pal_freeifaddrinfo(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

#if 0
// 
// Test pal_getifnameinfo happy path 
// 
TEST_FUNCTION(pal_win_getifnameinfo__success)
{
    static prx_socket_address_t* k_if_address_valid = (prx_socket_address_t*)0x23423;
    char* if_name_valid = UT_MEM;
    size_t if_name_length_valid = 256;
    uint64_t if_index_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = pal_getifnameinfo(k_if_address_valid, if_name_valid, if_name_length_valid, &if_index_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_getifnameinfo passing as if_address argument an invalid prx_socket_address_t* value 
// 
TEST_FUNCTION(pal_win_getifnameinfo__arg_if_address_null)
{
    char* if_name_valid = UT_MEM;
    size_t if_name_length_valid = 256;
    uint64_t if_index_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = pal_getifnameinfo(NULL, if_name_valid, if_name_length_valid, &if_index_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_getifnameinfo passing as if_name argument an invalid char* value 
// 
TEST_FUNCTION(pal_win_getifnameinfo__arg_if_name_null)
{
    static prx_socket_address_t* k_if_address_valid = (prx_socket_address_t*)0x23423;
    size_t if_name_length_valid = 256;
    uint64_t if_index_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = pal_getifnameinfo(k_if_address_valid, NULL, if_name_length_valid, &if_index_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_getifnameinfo passing as if_name_length argument an invalid size_t value 
// 
TEST_FUNCTION(pal_win_getifnameinfo__arg_if_name_length_0)
{
    static prx_socket_address_t* k_if_address_valid = (prx_socket_address_t*)0x23423;
    char* if_name_valid = UT_MEM;
    uint64_t if_index_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = pal_getifnameinfo(k_if_address_valid, if_name_valid, 0, &if_index_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_getifnameinfo passing as if_index argument an invalid uint64_t* value 
// 
TEST_FUNCTION(pal_win_getifnameinfo__arg_if_index_null)
{
    static prx_socket_address_t* k_if_address_valid = (prx_socket_address_t*)0x23423;
    char* if_name_valid = UT_MEM;
    size_t if_name_length_valid = 256;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = pal_getifnameinfo(k_if_address_valid, if_name_valid, if_name_length_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_getifnameinfo passing null for all out args
// 
TEST_FUNCTION(pal_win_getifnameinfo__arg_out_all_null)
{
    static prx_socket_address_t* k_if_address_valid = (prx_socket_address_t*)0x23423;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = pal_getifnameinfo(k_if_address_valid, NULL, 0, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_arg, result);
}

// 
// Test pal_getifnameinfo unhappy path 
// 
TEST_FUNCTION(pal_win_getifnameinfo__neg)
{
    static prx_socket_address_t* k_if_address_valid = (prx_socket_address_t*)0x23423;
    char* if_name_valid = UT_MEM;
    size_t if_name_length_valid = 256;
    uint64_t if_index_valid;
    int32_t result;

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_getifnameinfo(k_if_address_valid, if_name_valid, if_name_length_valid, &if_index_valid);

    // assert   
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_out_of_memory);
}
#endif

// 
// Test pal_gethostname happy path 
// 
TEST_FUNCTION(pal_win_gethostname__success_1)
{
    static const char* k_host_name_valid = "my_host1";
    static const size_t k_name_length_valid = 256;
    char* name_valid = UT_MEM;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(gethostname(name_valid, k_name_length_valid))
        .CopyOutArgumentBuffer_name(k_host_name_valid, strlen(k_host_name_valid) + 1)
        .SetReturn(0);

    // act 
    result = pal_gethostname(name_valid, k_name_length_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_gethostname happy path 
// 
TEST_FUNCTION(pal_win_gethostname__success_2)
{
    static const char* k_computer_name_valid = "my_computer1";
    static const size_t k_name_length_valid = 256;
    char* name_valid = UT_MEM;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(gethostname(IGNORED_PTR_ARG, k_name_length_valid))
        .IgnoreArgument(1)
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(WSAGetLastError())
        .SetReturn(WSAENETDOWN);
    STRICT_EXPECTED_CALL(FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, WSAENETDOWN, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), IGNORED_PTR_ARG, 0, NULL))
        .IgnoreArgument(5)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(LocalFree(IGNORED_PTR_ARG))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(GetComputerNameA(name_valid, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .CopyOutArgumentBuffer_lpBuffer(k_computer_name_valid, strlen(k_computer_name_valid) + 1)
        .SetReturn(TRUE);

    // act 
    result = pal_gethostname(name_valid, k_name_length_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_gethostname passing as name argument an invalid char* value 
// 
TEST_FUNCTION(pal_win_gethostname__arg_name_null)
{
    static const size_t k_name_length_valid = 256;
    int32_t result;

    // arrange 

    // act 
    result = pal_gethostname(NULL, k_name_length_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_gethostname passing as name_length argument an invalid size_t value 
// 
TEST_FUNCTION(pal_win_gethostname__arg_name_length_zero)
{
    char* name_valid = UT_MEM;
    int32_t result;

    // arrange 

    // act 
    result = pal_gethostname(name_valid, 0);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_gethostname passing as name_length argument an invalid size_t value 
// 
TEST_FUNCTION(pal_win_gethostname__arg_name_length_too_small)
{
    static const char* k_computer_name_valid = "my_computer1";
    static const size_t k_name_length_invalid = 3;
    char* name_valid = UT_MEM;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(gethostname(IGNORED_PTR_ARG, k_name_length_invalid))
        .IgnoreArgument(1)
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(WSAGetLastError())
        .SetReturn(WSAEFAULT);
    STRICT_EXPECTED_CALL(FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, WSAEFAULT, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), IGNORED_PTR_ARG, 0, NULL))
        .IgnoreArgument(5)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(LocalFree(IGNORED_PTR_ARG))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(GetComputerNameA(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(1).IgnoreArgument(2)
        .SetReturn(FALSE);

    // act 
    result = pal_gethostname(name_valid, k_name_length_invalid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_gethostname unhappy path 
// 
TEST_FUNCTION(pal_win_gethostname__neg)
{
    static const size_t k_name_length_valid = 256;
    char* name_valid = UT_MEM;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(gethostname(IGNORED_PTR_ARG, k_name_length_valid))
        .IgnoreArgument(1)
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(WSAGetLastError())
        .SetReturn(WSAENETDOWN);
    STRICT_EXPECTED_CALL(FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, WSAENETDOWN, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), IGNORED_PTR_ARG, 0, NULL))
        .IgnoreArgument(5)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(LocalFree(IGNORED_PTR_ARG))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(GetComputerNameA(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(1).IgnoreArgument(2)
        .SetReturn(FALSE);

    // act 
    result = pal_gethostname(name_valid, k_name_length_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_network, result);
}

// 
// Test pal_join_multicast_group happy path 
// 
TEST_FUNCTION(pal_win_join_multicast_group__success_1)
{
    static const prx_SOCKET k_fd_valid = (prx_SOCKET)23423;
    prx_multicast_option_t option_valid;
    int32_t result;

    option_valid.family = prx_address_family_inet;

    // arrange 
    STRICT_EXPECTED_CALL(setsockopt((SOCKET)k_fd_valid, IPPROTO_IP, IP_ADD_MEMBERSHIP, IGNORED_PTR_ARG, sizeof(struct ip_mreq)))
        .IgnoreArgument(4)
        .SetReturn(0);

    // act 
    result = pal_join_multicast_group(k_fd_valid, &option_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_join_multicast_group happy path 
// 
TEST_FUNCTION(pal_win_join_multicast_group__success_2)
{
    static const prx_SOCKET k_fd_valid = (prx_SOCKET)4567;
    prx_multicast_option_t option_valid;
    int32_t result;

    option_valid.family = prx_address_family_inet6;

    // arrange 
    STRICT_EXPECTED_CALL(setsockopt((SOCKET)k_fd_valid, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, IGNORED_PTR_ARG, sizeof(struct ipv6_mreq)))
        .IgnoreArgument(4)
        .SetReturn(0);

    // act 
    result = pal_join_multicast_group(k_fd_valid, &option_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_join_multicast_group passing as fd argument an invalid prx_SOCKET value 
// 
TEST_FUNCTION(pal_win_join_multicast_group__arg_fd_invalid)
{
    prx_multicast_option_t option_valid;
    int32_t result;

    option_valid.family = prx_address_family_inet6;

    // arrange 
    STRICT_EXPECTED_CALL(setsockopt(INVALID_SOCKET, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, IGNORED_PTR_ARG, sizeof(struct ipv6_mreq)))
        .IgnoreArgument(4)
        .SetReturn(SOCKET_ERROR);
    STRICT_EXPECTED_CALL(WSAGetLastError())
        .SetReturn(WSAENOTSOCK);
    STRICT_EXPECTED_CALL(FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, WSAENOTSOCK, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), IGNORED_PTR_ARG, 0, NULL))
        .IgnoreArgument(5)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(LocalFree(IGNORED_PTR_ARG))
        .SetReturn(NULL);

    // act 
    result = pal_join_multicast_group(prx_invalid_socket, &option_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_arg, result);
}

// 
// Test pal_join_multicast_group passing as option argument an invalid prx_multicast_option_t* value 
// 
TEST_FUNCTION(pal_win_join_multicast_group__arg_option_invalid)
{
    static const prx_SOCKET k_fd_valid = (prx_SOCKET)4546;
    prx_multicast_option_t option_invalid;
    int32_t result;

    option_invalid.family = prx_address_family_proxy;

    // arrange 

    // act 
    result = pal_join_multicast_group(k_fd_valid, &option_invalid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_not_supported, result);
}

// 
// Test pal_join_multicast_group passing as option argument an invalid prx_multicast_option_t* value 
// 
TEST_FUNCTION(pal_win_join_multicast_group__arg_option_null)
{
    static const prx_SOCKET k_fd_valid = (prx_SOCKET)3465;
    int32_t result;

    // arrange 

    // act 
    result = pal_join_multicast_group(k_fd_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_leave_multicast_group happy path 
// 
TEST_FUNCTION(pal_win_leave_multicast_group__success_1)
{
    static const prx_SOCKET k_fd_valid = (prx_SOCKET)94560;
    prx_multicast_option_t option_valid;
    int32_t result;

    option_valid.family = prx_address_family_inet;

    // arrange 
    STRICT_EXPECTED_CALL(setsockopt((SOCKET)k_fd_valid, IPPROTO_IP, IP_DROP_MEMBERSHIP, IGNORED_PTR_ARG, sizeof(struct ip_mreq)))
        .IgnoreArgument(4)
        .SetReturn(0);

    // act 
    result = pal_leave_multicast_group(k_fd_valid, &option_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_leave_multicast_group happy path 
// 
TEST_FUNCTION(pal_win_leave_multicast_group__success_2)
{
    static const prx_SOCKET k_fd_valid = (prx_SOCKET)4567;
    prx_multicast_option_t option_valid;
    int32_t result;

    option_valid.family = prx_address_family_inet6;

    // arrange 
    STRICT_EXPECTED_CALL(setsockopt((SOCKET)k_fd_valid, IPPROTO_IPV6, IPV6_DROP_MEMBERSHIP, IGNORED_PTR_ARG, sizeof(struct ipv6_mreq)))
        .IgnoreArgument(4)
        .SetReturn(0);

    // act 
    result = pal_leave_multicast_group(k_fd_valid, &option_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_leave_multicast_group passing as fd argument an invalid prx_SOCKET value 
// 
TEST_FUNCTION(pal_win_leave_multicast_group__arg_fd_invalid)
{
    prx_multicast_option_t option_valid;
    int32_t result;

    option_valid.family = prx_address_family_inet6;

    // arrange 
    STRICT_EXPECTED_CALL(setsockopt(INVALID_SOCKET, IPPROTO_IPV6, IPV6_DROP_MEMBERSHIP, IGNORED_PTR_ARG, sizeof(struct ipv6_mreq)))
        .IgnoreArgument(4)
        .SetReturn(SOCKET_ERROR);
    STRICT_EXPECTED_CALL(WSAGetLastError())
        .SetReturn(WSAENOTSOCK);
    STRICT_EXPECTED_CALL(FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, WSAENOTSOCK, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), IGNORED_PTR_ARG, 0, NULL))
        .IgnoreArgument(5)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(LocalFree(IGNORED_PTR_ARG))
        .SetReturn(NULL);

    // act 
    result = pal_leave_multicast_group(prx_invalid_socket, &option_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_arg, result);
}

// 
// Test pal_leave_multicast_group passing as option argument an invalid prx_multicast_option_t* value 
// 
TEST_FUNCTION(pal_win_leave_multicast_group__arg_option_invalid)
{
    static const prx_SOCKET k_fd_valid = (prx_SOCKET)4546;
    prx_multicast_option_t option_invalid;
    int32_t result;

    option_invalid.family = prx_address_family_proxy;

    // arrange 

    // act 
    result = pal_leave_multicast_group(k_fd_valid, &option_invalid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_not_supported, result);
}

// 
// Test pal_leave_multicast_group passing as option argument an invalid prx_multicast_option_t* value 
// 
TEST_FUNCTION(pal_win_leave_multicast_group__arg_option_null)
{
    static const prx_SOCKET k_fd_valid = (prx_SOCKET)23423;
    int32_t result;

    // option_valid.

    // arrange 

    // act 
    result = pal_leave_multicast_group(k_fd_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_close happy path 
// 
TEST_FUNCTION(pal_win_close__success)
{
    static const prx_SOCKET k_fd_valid = (prx_SOCKET)2725;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(closesocket((SOCKET)k_fd_valid))
        .SetReturn(0);

    // act 
    result = pal_close(k_fd_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_close passing as fd argument an invalid prx_SOCKET value 
// 
TEST_FUNCTION(pal_win_close__arg_fd_invalid)
{
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(closesocket(INVALID_SOCKET))
        .SetReturn(SOCKET_ERROR);
    STRICT_EXPECTED_CALL(WSAGetLastError())
        .SetReturn(WSA_INVALID_HANDLE);
    STRICT_EXPECTED_CALL(FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, WSA_INVALID_HANDLE, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), IGNORED_PTR_ARG, 0, NULL))
        .IgnoreArgument(5)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(LocalFree(IGNORED_PTR_ARG))
        .SetReturn(NULL);

    // act 
    result = pal_close(prx_invalid_socket);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_arg, result);
}

// 
// Test pal_os_to_prx_net_error happy path 
// 
TEST_FUNCTION(pal_win_os_to_prx_net_error__success)
{
    int32_t result;

    // arrange 
    UMOCK_C_RANGE_TESTS_ARRANGE_FROM_TO(int, input, 
        WSA_ERRORS_RANGE_BEGIN, 
        WSA_ERRORS_RANGE_END);

    // act 
    UMOCK_C_RANGE_TESTS_ACT();
    result = pal_os_to_prx_net_error(input);

    // assert 
    UMOCK_C_RANGE_TESTS_ASSERT(int32_t, result, er_unknown,
        er_aborted,       er_arg,           er_permission,    er_fault,         er_arg,           
        er_out_of_memory, er_retry,         er_retry,         er_waiting,       er_arg,
        er_arg,           er_arg,           er_arg,           er_arg,           er_not_impl,      
        er_not_impl,      er_not_impl,      er_busy,          er_host_unknown,  er_network,       
        er_undelivered,   er_network,       er_connecting,    er_aborted,       er_out_of_memory, 
        er_connecting,    er_closed,        er_timeout,       er_refused,       er_arg,
        er_arg,           er_connecting,    er_disk_io,       er_waiting,       er_waiting,
        er_arg,           er_arg,           er_out_of_memory, er_aborted,       er_not_impl,
        er_not_impl,      er_shutdown,      er_out_of_memory, er_connecting,    er_out_of_memory, 
        er_out_of_memory, er_out_of_memory, er_disk_io,       er_disk_io,       er_closed,
        er_nomore,        er_aborted,       er_refused,       er_bad_state,     er_unknown);
}

// 
// Test pal_os_from_prx_net_error happy path 
// 
TEST_FUNCTION(pal_win_os_from_prx_net_error__success)
{
    int result;

    // arrange 
    UMOCK_C_RANGE_TESTS_ARRANGE_FROM_TO(int32_t, input,
        er_unknown, 
        er_ok);

    // act 
    UMOCK_C_RANGE_TESTS_ACT();
    result = pal_os_from_prx_net_error(input);

    // assert 
    UMOCK_C_RANGE_TESTS_ASSERT(int, result, -1, 
        1056, 1005, 1004, 1055, 1039, 1019, 1016, 1016, 1003, 1007, 
        1052, 1021, 1024, 1018, 1009, 1029, 1053, 1028, 1043, 1054, 
        1020, 1034, 1022, 0);  // see os_mock.h
}

// 
// Test pal_os_last_net_error_as_prx_error happy path 
// 
TEST_FUNCTION(pal_win_get_prx_net_error__success_1)
{
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(WSAGetLastError())
        .SetReturn(0);

    // act 
    result = pal_os_last_net_error_as_prx_error();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_os_last_net_error_as_prx_error happy path 
// 
TEST_FUNCTION(pal_win_get_prx_net_error__success_2)
{
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(WSAGetLastError())
        .SetReturn(WSAEWOULDBLOCK);

    // act 
    result = pal_os_last_net_error_as_prx_error();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_retry, result);
}

// 
// Test pal_os_last_net_error_as_prx_error happy path 
// 
TEST_FUNCTION(pal_win_get_prx_net_error__success_3)
{
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(WSAGetLastError())
        .SetReturn(WSAENETDOWN);
    STRICT_EXPECTED_CALL(FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, WSAENETDOWN, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), IGNORED_PTR_ARG, 0, NULL))
        .IgnoreArgument(5)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(LocalFree(IGNORED_PTR_ARG))
        .SetReturn(NULL);

    // act 
    result = pal_os_last_net_error_as_prx_error();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_network, result);
}

// 
// Test pal_os_set_net_error_as_prx_error happy path 
// 
TEST_FUNCTION(pal_win_set_prx_net_error__success)
{
    // arrange 
    STRICT_EXPECTED_CALL(WSASetLastError(WSA_NOT_ENOUGH_MEMORY));

    // act 
    pal_os_set_net_error_as_prx_error(er_out_of_memory);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_socket_init happy path 
// 
TEST_FUNCTION(pal_win_socket_init__success)
{
    static const SOCKET k_socket_valid = (SOCKET)34534;
    static const LPFN_WSARECVMSG k_wsa_recvmsg_valid = (LPFN_WSARECVMSG)0x7354;
    static const LPFN_WSASENDMSG k_wsa_sendmsg_valid = (LPFN_WSASENDMSG)0x9898;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(WSAStartup(MAKEWORD(2, 2), IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .SetReturn(0);
    STRICT_EXPECTED_CALL(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))
        .SetReturn(k_socket_valid);
    STRICT_EXPECTED_CALL(WSAIoctl(k_socket_valid, SIO_GET_EXTENSION_FUNCTION_POINTER, IGNORED_PTR_ARG, sizeof(GUID),
        &wsa_recvmsg, sizeof(LPFN_WSARECVMSG), IGNORED_PTR_ARG, NULL, NULL))
        .IgnoreArgument(3).IgnoreArgument(7)
        .CopyOutArgumentBuffer_lpvOutBuffer(&k_wsa_recvmsg_valid, sizeof(k_wsa_recvmsg_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(WSAIoctl(k_socket_valid, SIO_GET_EXTENSION_FUNCTION_POINTER, IGNORED_PTR_ARG, sizeof(GUID),
        &wsa_sendmsg, sizeof(LPFN_WSASENDMSG), IGNORED_PTR_ARG, NULL, NULL))
        .IgnoreArgument(3).IgnoreArgument(7)
        .CopyOutArgumentBuffer_lpvOutBuffer(&k_wsa_sendmsg_valid, sizeof(k_wsa_sendmsg_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(closesocket(k_socket_valid))
        .SetReturn(0);

    // act 
    result = pal_socket_init();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_socket_init unhappy path 
// 
TEST_FUNCTION(pal_win_socket_init__neg)
{
    static const SOCKET k_socket_valid = (SOCKET)34534;
    static const LPFN_WSARECVMSG k_wsa_recvmsg_valid = (LPFN_WSARECVMSG)0x7354;
    static const LPFN_WSASENDMSG k_wsa_sendmsg_valid = (LPFN_WSASENDMSG)0x9898;
    int32_t result;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(WSAStartup(MAKEWORD(2, 2), IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .SetReturn(0)
        .SetFailReturn(SOCKET_ERROR);
    STRICT_EXPECTED_CALL(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))
        .SetReturn(k_socket_valid)
        .SetFailReturn(INVALID_SOCKET);
    STRICT_EXPECTED_CALL(WSAIoctl(k_socket_valid, SIO_GET_EXTENSION_FUNCTION_POINTER, IGNORED_PTR_ARG, sizeof(GUID),
        &wsa_recvmsg, sizeof(LPFN_WSARECVMSG), IGNORED_PTR_ARG, NULL, NULL))
        .IgnoreArgument(3).IgnoreArgument(7)
        .CopyOutArgumentBuffer_lpvOutBuffer(&k_wsa_recvmsg_valid, sizeof(k_wsa_recvmsg_valid))
        .SetReturn(0)
        .SetFailReturn(SOCKET_ERROR);
    STRICT_EXPECTED_CALL(WSAIoctl(k_socket_valid, SIO_GET_EXTENSION_FUNCTION_POINTER, IGNORED_PTR_ARG, sizeof(GUID),
        &wsa_sendmsg, sizeof(LPFN_WSASENDMSG), IGNORED_PTR_ARG, NULL, NULL))
        .IgnoreArgument(3).IgnoreArgument(7)
        .CopyOutArgumentBuffer_lpvOutBuffer(&k_wsa_sendmsg_valid, sizeof(k_wsa_sendmsg_valid))
        .SetReturn(0)
        .SetFailReturn(SOCKET_ERROR);
    STRICT_EXPECTED_CALL(closesocket(k_socket_valid))
        .SetReturn(0)
        .SetFailReturn(SOCKET_ERROR);
    STRICT_EXPECTED_CALL(WSACleanup())
        .SetReturn(0)
        .SetFailReturn(SOCKET_ERROR);

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_socket_init();

    // assert    
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_fatal, er_fatal, er_ok, er_ok);
}

// 
// Test pal_socket_deinit happy path 
// 
TEST_FUNCTION(pal_win_socket_deinit__success)
{
    // arrange 
    STRICT_EXPECTED_CALL(WSACleanup())
        .SetReturn(0);

    // act 
    pal_socket_deinit();

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_get_available happy path 
// 
TEST_FUNCTION(pal_win_get_available__success)
{
    static const prx_SOCKET k_fd_valid = (prx_SOCKET)2725;
    static unsigned long k_avail_valid = 10000;
    size_t avail_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(ioctlsocket(k_fd_valid, FIONREAD, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_argp(&k_avail_valid, sizeof(k_avail_valid))
        .SetReturn(0);

    // act 
    result = pal_get_available(k_fd_valid, &avail_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_IS_TRUE(k_avail_valid == avail_valid);
}

// 
// Test pal_get_available passing as fd argument an invalid prx_SOCKET value 
// 
TEST_FUNCTION(pal_win_get_available__arg_fd_invalid)
{
    size_t avail_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(ioctlsocket(INVALID_SOCKET, FIONREAD, IGNORED_PTR_ARG))
        .IgnoreArgument(3)
        .SetReturn(SOCKET_ERROR);
    STRICT_EXPECTED_CALL(WSAGetLastError())
        .SetReturn(WSA_INVALID_HANDLE);
    STRICT_EXPECTED_CALL(FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, WSA_INVALID_HANDLE, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), IGNORED_PTR_ARG, 0, NULL))
        .IgnoreArgument(5)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(LocalFree(IGNORED_PTR_ARG))
        .SetReturn(NULL);

    // act 
    result = pal_get_available(prx_invalid_socket, &avail_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_arg, result);
}

// 
// Test pal_get_available passing as avail argument a null size_t value 
// 
TEST_FUNCTION(pal_win_get_available__avail_invalid)
{
    static const prx_SOCKET k_fd_valid = (prx_SOCKET)72435;
    int32_t result;

    // arrange 

    // act 
    result = pal_get_available(k_fd_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_set_nonblocking happy path 
// 
TEST_FUNCTION(pal_win_set_nonblocking__success_1)
{
    static const prx_SOCKET k_fd_valid = (prx_SOCKET)2725;
    static unsigned long k_nb_on = 1;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(ioctlsocket(k_fd_valid, FIONBIO, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &k_nb_on, sizeof(k_nb_on))
        .SetReturn(0);

    // act 
    result = pal_set_nonblocking(k_fd_valid, true);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_set_nonblocking happy path 
// 
TEST_FUNCTION(pal_win_set_nonblocking__success_2)
{
    static const prx_SOCKET k_fd_valid = (prx_SOCKET)2725;
    static unsigned long k_nb_off = 0;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(ioctlsocket(k_fd_valid, FIONBIO, IGNORED_PTR_ARG))
        .ValidateArgumentBuffer(3, &k_nb_off, sizeof(k_nb_off))
        .SetReturn(0);

    // act 
    result = pal_set_nonblocking(k_fd_valid, false);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_set_nonblocking passing as fd argument an invalid prx_SOCKET value 
// 
TEST_FUNCTION(pal_win_set_nonblocking__arg_fd_invalid)
{
    int32_t result;

    // arrange 
    // ... 
    STRICT_EXPECTED_CALL(ioctlsocket(INVALID_SOCKET, FIONBIO, IGNORED_PTR_ARG))
        .IgnoreArgument(3)
        .SetReturn(SOCKET_ERROR);
    STRICT_EXPECTED_CALL(WSAGetLastError())
        .SetReturn(WSA_INVALID_HANDLE);
    STRICT_EXPECTED_CALL(FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, WSA_INVALID_HANDLE, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), IGNORED_PTR_ARG, 0, NULL))
        .IgnoreArgument(5)
        .SetReturn(1);
    STRICT_EXPECTED_CALL(LocalFree(IGNORED_PTR_ARG))
        .SetReturn(NULL);

    // act 
    result = pal_set_nonblocking(prx_invalid_socket, true);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_arg, result);
}
#endif

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()


