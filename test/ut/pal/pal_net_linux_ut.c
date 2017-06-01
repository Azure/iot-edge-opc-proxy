// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST pal_net_linux
#include "util_ut.h"

//
// 1. Required mocks
//
#include "os.h"
#include "prx_types.h"

// ifaddrs.h
MOCKABLE_FUNCTION(, int, getifaddrs, 
    struct ifaddrs**, ifap);
MOCKABLE_FUNCTION(, void, freeifaddrs, 
    struct ifaddrs*, ifa);
// socket
MOCKABLE_FUNCTION(, int, gethostname, 
    char*, name, socksize_t, namelen);
MOCKABLE_FUNCTION(, int, getsockopt,
    fd_t, s, int, optlevel, int, optname, sockbuf_t*, optval, socklen_t*, optlen);
MOCKABLE_FUNCTION(, int, setsockopt,
    fd_t, s, int, optlevel, int, optname, const sockbuf_t*, optval, socklen_t, optlen);
MOCKABLE_FUNCTION(, int, shutdown,
    fd_t, s, int, how);
MOCKABLE_FUNCTION(, int, listen,
    fd_t, s, int, backlog);
// ioctl
MOCKABLE_FUNCTION(, int, ioctl, 
    fd_t, fd, unsigned int, cmd, int*, arg);
// unistd.h
MOCKABLE_FUNCTION(, int, close,
    fd_t, s);
MOCKABLE_FUNCTION(, int, fcntl,
    fd_t, s, int, cmd, int, val);
// net/if.h
MOCKABLE_FUNCTION(, unsigned int, 
    if_nametoindex, const char*, ifname);
// netdb.h
//#undef h_errno
//MOCKABLE_FUNCTION(, int*, h_errno_mock);
//#define h_errno (*h_errno_mock())


//
// 2. Include unit under test
//
#undef ENABLE_MOCKS
#include "pal_net.h"
#include "pal_types.h"
#define ENABLE_MOCKS
#include UNIT_C


// pal_types.h - platform independent
MOCKABLE_FUNCTION(, int32_t, pal_os_to_prx_socket_address,
    const struct sockaddr*, sa, socklen_t, sa_len, prx_socket_address_t*, prx_address);

//
// 3. Setup test suite
//
BEGIN_DECLARE_TEST_SUITE()
REGISTER_UMOCK_ALIAS_TYPE(fd_t, int);
REGISTER_UMOCK_ALIAS_TYPE(socklen_t, int);
REGISTER_UMOCK_ALIAS_TYPE(socksize_t, int);
END_DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
DECLARE_TEST_SETUP()

// 
// Test pal_getifaddrinfo happy path 
// 
TEST_FUNCTION(pal_linux_getifaddrinfo__success_1)
{
    static const char* k_if_name_valid = "eth1";
    struct ifaddrs address_valid[6], *address_valid_ptr = &address_valid[0];
    struct sockaddr_in6 sock_addr_in6_valid;
    struct sockaddr* sock_addr_valid = (struct sockaddr*)&sock_addr_in6_valid;
    prx_ifaddrinfo_t* info_valid;
    size_t info_count_valid;
    int32_t result;

    UT_MEM_ALLOCED = malloc(20000);

    sock_addr_valid->sa_family = AF_INET6;
    memset(address_valid, 0, sizeof(address_valid));
    address_valid[0].ifa_name = (char*)"eth0";
    address_valid[0].ifa_next = &address_valid[1];
    address_valid[1].ifa_name = (char*)k_if_name_valid;
    address_valid[1].ifa_addr = sock_addr_valid;
    address_valid[1].ifa_netmask = sock_addr_valid;
    address_valid[1].ifa_next = &address_valid[2];
    address_valid[2].ifa_name = (char*)"eth2";
    address_valid[2].ifa_next = &address_valid[3];
    address_valid[3].ifa_name = (char*)"eth3";
    address_valid[3].ifa_next = &address_valid[4];
    address_valid[4].ifa_name = (char*)k_if_name_valid;
    address_valid[4].ifa_addr = sock_addr_valid;
    address_valid[4].ifa_netmask = sock_addr_valid;
    address_valid[4].ifa_next = &address_valid[5];
    address_valid[5].ifa_name = (char*)k_if_name_valid;
    address_valid[5].ifa_addr = sock_addr_valid;
    address_valid[5].ifa_netmask = sock_addr_valid;
    address_valid[5].ifa_next = NULL;

    // arrange 
    STRICT_EXPECTED_CALL(getifaddrs(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_ifap(&address_valid_ptr, sizeof(address_valid_ptr))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(string_compare("eth0", k_if_name_valid))
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(string_compare(k_if_name_valid, k_if_name_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(string_compare("eth2", k_if_name_valid))
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(string_compare("eth3", k_if_name_valid))
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(string_compare(k_if_name_valid, k_if_name_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(string_compare(k_if_name_valid, k_if_name_valid))
        .SetReturn(0);

    STRICT_EXPECTED_CALL(h_realloc(4 * sizeof(prx_ifaddrinfo_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM_ALLOCED);
    STRICT_EXPECTED_CALL(string_compare("eth0", k_if_name_valid))
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(string_compare(k_if_name_valid, k_if_name_valid))
        .SetReturn(0);
    
    STRICT_EXPECTED_CALL(pal_os_to_prx_socket_address(
        sock_addr_valid, sizeof(sock_addr_in6_valid), IGNORED_PTR_ARG))
        .IgnoreArgument(3)
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(count_leading_ones_in_buf(IGNORED_PTR_ARG, 16))
        .IgnoreArgument(1)
        .SetReturn(10);
    STRICT_EXPECTED_CALL(if_nametoindex(k_if_name_valid))
        .SetReturn(1);

    STRICT_EXPECTED_CALL(string_compare("eth2", k_if_name_valid))
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(string_compare("eth3", k_if_name_valid))
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(string_compare(k_if_name_valid, k_if_name_valid))
        .SetReturn(0);
    
    STRICT_EXPECTED_CALL(pal_os_to_prx_socket_address(
        sock_addr_valid, sizeof(sock_addr_in6_valid), IGNORED_PTR_ARG))
        .IgnoreArgument(3)
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(count_leading_ones_in_buf(IGNORED_PTR_ARG, 16))
        .IgnoreArgument(1)
        .SetReturn(10);
    STRICT_EXPECTED_CALL(if_nametoindex(k_if_name_valid))
        .SetReturn(1);
    
    STRICT_EXPECTED_CALL(string_compare(k_if_name_valid, k_if_name_valid))
        .SetReturn(0);
    
    STRICT_EXPECTED_CALL(pal_os_to_prx_socket_address(
        sock_addr_valid, sizeof(sock_addr_in6_valid), IGNORED_PTR_ARG))
        .IgnoreArgument(3)
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(count_leading_ones_in_buf(IGNORED_PTR_ARG, 16))
        .IgnoreArgument(1)
        .SetReturn(10);
    STRICT_EXPECTED_CALL(if_nametoindex(k_if_name_valid))
        .SetReturn(1);
    
    STRICT_EXPECTED_CALL(freeifaddrs(address_valid_ptr));

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
TEST_FUNCTION(pal_linux_getifaddrinfo__success_2)
{
    struct ifaddrs address_valid[6], *address_valid_ptr = &address_valid[0];
    struct sockaddr_in6 sock_addr_in6_valid;
    struct sockaddr* sock_addr_valid = (struct sockaddr*)&sock_addr_in6_valid;
    prx_ifaddrinfo_t* info_valid;
    size_t info_count_valid;
    int32_t result;

    UT_MEM_ALLOCED = malloc(40000);

    sock_addr_valid->sa_family = AF_INET6;
    memset(address_valid, 0, sizeof(address_valid));
    address_valid[0].ifa_name = (char*)"eth0";
    address_valid[0].ifa_addr = sock_addr_valid;
    address_valid[0].ifa_netmask = sock_addr_valid;
    address_valid[0].ifa_next = &address_valid[1];
    address_valid[1].ifa_name = (char*)"eth0";
    address_valid[1].ifa_addr = sock_addr_valid;
    address_valid[1].ifa_netmask = sock_addr_valid;
    address_valid[1].ifa_next = &address_valid[2];
    address_valid[2].ifa_name = (char*)"eth1";
    address_valid[2].ifa_addr = sock_addr_valid;
    address_valid[2].ifa_netmask = sock_addr_valid;
    address_valid[2].ifa_next = &address_valid[3];
    address_valid[3].ifa_name = (char*)"eth1";
    address_valid[3].ifa_addr = sock_addr_valid;
    address_valid[3].ifa_netmask = sock_addr_valid;
    address_valid[3].ifa_next = &address_valid[4];
    address_valid[4].ifa_name = (char*)"eth2";
    address_valid[4].ifa_addr = sock_addr_valid;
    address_valid[4].ifa_netmask = sock_addr_valid;
    address_valid[4].ifa_next = &address_valid[5];
    address_valid[5].ifa_name = (char*)"eth2";
    address_valid[5].ifa_addr = sock_addr_valid;
    address_valid[5].ifa_netmask = sock_addr_valid;
    address_valid[5].ifa_next = NULL;

    // arrange 
    STRICT_EXPECTED_CALL(getifaddrs(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_ifap(&address_valid_ptr, sizeof(address_valid_ptr))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(h_realloc(7 * sizeof(prx_ifaddrinfo_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM_ALLOCED);
    for (size_t i = 0; i < _countof(address_valid); i++)
    {
        STRICT_EXPECTED_CALL(pal_os_to_prx_socket_address(
            sock_addr_valid, sizeof(sock_addr_in6_valid), IGNORED_PTR_ARG))
            .IgnoreArgument(3)
            .SetReturn(er_ok);
        STRICT_EXPECTED_CALL(count_leading_ones_in_buf(IGNORED_PTR_ARG, 16))
            .IgnoreArgument(1)
            .SetReturn(10);
        STRICT_EXPECTED_CALL(if_nametoindex(address_valid[i].ifa_name))
            .SetReturn((unsigned int)i+1);
    }
    STRICT_EXPECTED_CALL(freeifaddrs(address_valid_ptr));


    // act 
    result = pal_getifaddrinfo(NULL, 0, &info_valid, &info_count_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_IS_TRUE(info_count_valid == 6);
}

// 
// Test pal_getifaddrinfo passing as info argument an invalid prx_ifaddrinfo_t** value 
// 
TEST_FUNCTION(pal_linux_getifaddrinfo__arg_info_null)
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
TEST_FUNCTION(pal_linux_getifaddrinfo__arg_info_count_null)
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
TEST_FUNCTION(pal_linux_getifaddrinfo__neg)
{
    struct ifaddrs address_valid[6], *address_valid_ptr = &address_valid[0];
    struct sockaddr_in6 sock_addr_in6_valid;
    struct sockaddr* sock_addr_valid = (struct sockaddr*)&sock_addr_in6_valid;
    prx_ifaddrinfo_t* info_valid;
    size_t info_count_valid;
    int32_t result;

    UT_MEM_ALLOCED = malloc(40000);

    sock_addr_valid->sa_family = AF_INET6;
    memset(address_valid, 0, sizeof(address_valid));
    address_valid[0].ifa_name = (char*)"eth0";
    address_valid[0].ifa_addr = sock_addr_valid;
    address_valid[0].ifa_next = &address_valid[1];
    address_valid[1].ifa_name = (char*)"eth0";
    address_valid[1].ifa_addr = sock_addr_valid;
    address_valid[1].ifa_next = &address_valid[2];
    address_valid[2].ifa_name = (char*)"eth1";
    address_valid[2].ifa_addr = sock_addr_valid;
    address_valid[2].ifa_next = &address_valid[3];
    address_valid[3].ifa_name = (char*)"eth1";
    address_valid[3].ifa_addr = sock_addr_valid;
    address_valid[3].ifa_next = &address_valid[4];
    address_valid[4].ifa_name = (char*)"eth2";
    address_valid[4].ifa_addr = sock_addr_valid;
    address_valid[4].ifa_next = &address_valid[5];
    address_valid[5].ifa_name = (char*)"eth2";
    address_valid[5].ifa_addr = sock_addr_valid;
    address_valid[5].ifa_next = NULL;

    REGISTER_GLOBAL_MOCK_RETURNS(pal_os_last_error_as_prx_error, er_fatal, er_fatal);

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(getifaddrs(IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_ifap(&address_valid_ptr, sizeof(address_valid_ptr))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(h_realloc(7 * sizeof(prx_ifaddrinfo_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM_ALLOCED)
        .SetFailReturn(NULL);
    for (size_t i = 0; i < _countof(address_valid); i++)
    {
        STRICT_EXPECTED_CALL(pal_os_to_prx_socket_address(
            sock_addr_valid, sizeof(sock_addr_in6_valid), IGNORED_PTR_ARG))
            .IgnoreArgument(3)
            .SetReturn(er_ok)
            .SetFailReturn(er_invalid_format);
        STRICT_EXPECTED_CALL(if_nametoindex(IGNORED_PTR_ARG))
            .IgnoreArgument(1)
            .SetReturn((unsigned int)i+1)
            .SetFailReturn(0);
    }
    STRICT_EXPECTED_CALL(freeifaddrs(address_valid_ptr));

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_getifaddrinfo(NULL, 0, &info_valid, &info_count_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, 
        er_fatal, er_out_of_memory, er_ok, er_fatal, er_ok);
}

// 
// Test pal_freeifaddrinfo happy path 
// 
TEST_FUNCTION(pal_linux_freeifaddrinfo__success)
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
TEST_FUNCTION(pal_linux_freeifaddrinfo__arg_info_null)
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
TEST_FUNCTION(pal_linux_getifnameinfo__success)
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
TEST_FUNCTION(pal_linux_getifnameinfo__arg_if_address_null)
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
TEST_FUNCTION(pal_linux_getifnameinfo__arg_if_name_null)
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
TEST_FUNCTION(pal_linux_getifnameinfo__arg_if_name_length_0)
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
TEST_FUNCTION(pal_linux_getifnameinfo__arg_if_index_null)
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
TEST_FUNCTION(pal_linux_getifnameinfo__arg_out_all_null)
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
TEST_FUNCTION(pal_linux_getifnameinfo__neg)
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
TEST_FUNCTION(pal_linux_gethostname__success)
{
    static const char* k_host_name_valid = "my_host1";
    static const size_t k_name_length_valid = 256;
    char* name_valid = UT_MEM;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(gethostname(name_valid, (socksize_t)k_name_length_valid))
        .CopyOutArgumentBuffer_name(k_host_name_valid, strlen(k_host_name_valid) + 1)
        .SetReturn(0);

    // act 
    result = pal_gethostname(name_valid, k_name_length_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_gethostname passing as name argument an invalid char* value 
// 
TEST_FUNCTION(pal_linux_gethostname__arg_name_null)
{
    static const size_t k_name_length_valid = 256;
    int32_t result;

    // arrange 

    // act 
    result = pal_gethostname(NULL, (socksize_t)k_name_length_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_gethostname passing as name_length argument an invalid size_t value 
// 
TEST_FUNCTION(pal_linux_gethostname__arg_name_length_zero)
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
// Test pal_gethostname unhappy path 
// 
TEST_FUNCTION(pal_linux_gethostname__neg)
{
    static const size_t k_name_length_invalid = 3;
    char* name_valid = UT_MEM;
    int32_t result;

    REGISTER_GLOBAL_MOCK_RETURNS(pal_os_last_error_as_prx_error, er_fault, er_fault);

    // arrange 
    STRICT_EXPECTED_CALL(gethostname(IGNORED_PTR_ARG, (socksize_t)k_name_length_invalid))
        .IgnoreArgument(1)
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(pal_os_last_error_as_prx_error());

    // act 
    result = pal_gethostname(name_valid, k_name_length_invalid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}


#ifdef pal_pton

//
//Test pal_pton happy path 
// 
TEST_FUNCTION(pal_pton__success)
{
    static const const char* k_addr_string_valid;
    static const prx_socket_address_t* k_address_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = pal_pton(k_addr_string_valid, k_address_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test pal_pton passing as addr_string argument an invalid const char* value 
// 
TEST_FUNCTION(pal_pton__arg_addr_string_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_pton();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_pton passing as address argument an invalid prx_socket_address_t* value 
// 
TEST_FUNCTION(pal_pton__arg_address_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_pton();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_pton unhappy path 
// 
TEST_FUNCTION(pal_pton__neg)
{
    static const const char* k_addr_string_valid;
    static const prx_socket_address_t* k_address_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_pton(k_addr_string_valid, k_address_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // pal_pton;

#ifdef pal_ntop

//
//Test pal_ntop happy path 
// 
TEST_FUNCTION(pal_ntop__success)
{
    static const prx_socket_address_t* k_address_valid;
    static const char* k_addr_string_valid;
    static const size_t k_addr_string_size_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = pal_ntop(k_address_valid, k_addr_string_valid, k_addr_string_size_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test pal_ntop passing as address argument an invalid prx_socket_address_t* value 
// 
TEST_FUNCTION(pal_ntop__arg_address_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_ntop();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_ntop passing as addr_string argument an invalid char* value 
// 
TEST_FUNCTION(pal_ntop__arg_addr_string_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_ntop();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_ntop passing as addr_string_size argument an invalid size_t value 
// 
TEST_FUNCTION(pal_ntop__arg_addr_string_size_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_ntop();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_ntop unhappy path 
// 
TEST_FUNCTION(pal_ntop__neg)
{
    static const prx_socket_address_t* k_address_valid;
    static const char* k_addr_string_valid;
    static const size_t k_addr_string_size_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_ntop(k_address_valid, k_addr_string_valid, k_addr_string_size_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // pal_ntop;

#ifdef pal_getifaddrinfo

//
//Test pal_getifaddrinfo happy path 
// 
TEST_FUNCTION(pal_getifaddrinfo__success)
{
    static const const char* k_if_name_valid;
    static const uint32_t k_flags_valid;
    static const prx_ifaddrinfo_t** k_info_valid;
    static const size_t* k_info_count_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = pal_getifaddrinfo(k_if_name_valid, k_flags_valid, k_info_valid, k_info_count_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test pal_getifaddrinfo passing as if_name argument an invalid const char* value 
// 
TEST_FUNCTION(pal_getifaddrinfo__arg_if_name_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_getifaddrinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_getifaddrinfo passing as flags argument an invalid uint32_t value 
// 
TEST_FUNCTION(pal_getifaddrinfo__arg_flags_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_getifaddrinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_getifaddrinfo passing as info argument an invalid prx_ifaddrinfo_t** value 
// 
TEST_FUNCTION(pal_getifaddrinfo__arg_info_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_getifaddrinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_getifaddrinfo passing as info_count argument an invalid size_t* value 
// 
TEST_FUNCTION(pal_getifaddrinfo__arg_info_count_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_getifaddrinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_getifaddrinfo unhappy path 
// 
TEST_FUNCTION(pal_getifaddrinfo__neg)
{
    static const const char* k_if_name_valid;
    static const uint32_t k_flags_valid;
    static const prx_ifaddrinfo_t** k_info_valid;
    static const size_t* k_info_count_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_getifaddrinfo(k_if_name_valid, k_flags_valid, k_info_valid, k_info_count_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // pal_getifaddrinfo;

#ifdef pal_freeifaddrinfo

//
//Test pal_freeifaddrinfo happy path 
// 
TEST_FUNCTION(pal_freeifaddrinfo__success)
{
    static const prx_ifaddrinfo_t* k_info_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = pal_freeifaddrinfo(k_info_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test pal_freeifaddrinfo passing as info argument an invalid prx_ifaddrinfo_t* value 
// 
TEST_FUNCTION(pal_freeifaddrinfo__arg_info_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_freeifaddrinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_freeifaddrinfo unhappy path 
// 
TEST_FUNCTION(pal_freeifaddrinfo__neg)
{
    static const prx_ifaddrinfo_t* k_info_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_freeifaddrinfo(k_info_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // pal_freeifaddrinfo;

#ifdef pal_getifnameinfo

//
//Test pal_getifnameinfo happy path 
// 
TEST_FUNCTION(pal_getifnameinfo__success)
{
    static const prx_socket_address_t* k_if_address_valid;
    static const char* k_if_name_valid;
    static const size_t k_if_name_length_valid;
    static const uint64_t* k_if_index_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = pal_getifnameinfo(k_if_address_valid, k_if_name_valid, k_if_name_length_valid, k_if_index_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test pal_getifnameinfo passing as if_address argument an invalid prx_socket_address_t* value 
// 
TEST_FUNCTION(pal_getifnameinfo__arg_if_address_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_getifnameinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_getifnameinfo passing as if_name argument an invalid char* value 
// 
TEST_FUNCTION(pal_getifnameinfo__arg_if_name_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_getifnameinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_getifnameinfo passing as if_name_length argument an invalid size_t value 
// 
TEST_FUNCTION(pal_getifnameinfo__arg_if_name_length_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_getifnameinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_getifnameinfo passing as if_index argument an invalid uint64_t* value 
// 
TEST_FUNCTION(pal_getifnameinfo__arg_if_index_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_getifnameinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_getifnameinfo unhappy path 
// 
TEST_FUNCTION(pal_getifnameinfo__neg)
{
    static const prx_socket_address_t* k_if_address_valid;
    static const char* k_if_name_valid;
    static const size_t k_if_name_length_valid;
    static const uint64_t* k_if_index_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_getifnameinfo(k_if_address_valid, k_if_name_valid, k_if_name_length_valid, k_if_index_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // pal_getifnameinfo;

#ifdef pal_getaddrinfo

//
//Test pal_getaddrinfo happy path 
// 
TEST_FUNCTION(pal_getaddrinfo__success)
{
    static const const char* k_host_name_valid;
    static const const char* k_service_valid;
    static const prx_address_family_t k_family_valid;
    static const uint32_t k_flags_valid;
    static const prx_addrinfo_t** k_info_valid;
    static const size_t* k_info_count_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = pal_getaddrinfo(k_host_name_valid, k_service_valid, k_family_valid, k_flags_valid, k_info_valid, k_info_count_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test pal_getaddrinfo passing as host_name argument an invalid const char* value 
// 
TEST_FUNCTION(pal_getaddrinfo__arg_host_name_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_getaddrinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_getaddrinfo passing as service argument an invalid const char* value 
// 
TEST_FUNCTION(pal_getaddrinfo__arg_service_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_getaddrinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_getaddrinfo passing as family argument an invalid prx_address_family_t value 
// 
TEST_FUNCTION(pal_getaddrinfo__arg_family_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_getaddrinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_getaddrinfo passing as flags argument an invalid uint32_t value 
// 
TEST_FUNCTION(pal_getaddrinfo__arg_flags_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_getaddrinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_getaddrinfo passing as info argument an invalid prx_addrinfo_t** value 
// 
TEST_FUNCTION(pal_getaddrinfo__arg_info_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_getaddrinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_getaddrinfo passing as info_count argument an invalid size_t* value 
// 
TEST_FUNCTION(pal_getaddrinfo__arg_info_count_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_getaddrinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_getaddrinfo unhappy path 
// 
TEST_FUNCTION(pal_getaddrinfo__neg)
{
    static const const char* k_host_name_valid;
    static const const char* k_service_valid;
    static const prx_address_family_t k_family_valid;
    static const uint32_t k_flags_valid;
    static const prx_addrinfo_t** k_info_valid;
    static const size_t* k_info_count_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_getaddrinfo(k_host_name_valid, k_service_valid, k_family_valid, k_flags_valid, k_info_valid, k_info_count_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // pal_getaddrinfo;

#ifdef pal_freeaddrinfo

//
//Test pal_freeaddrinfo happy path 
// 
TEST_FUNCTION(pal_freeaddrinfo__success)
{
    static const prx_addrinfo_t* k_info_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = pal_freeaddrinfo(k_info_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test pal_freeaddrinfo passing as info argument an invalid prx_addrinfo_t* value 
// 
TEST_FUNCTION(pal_freeaddrinfo__arg_info_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_freeaddrinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_freeaddrinfo unhappy path 
// 
TEST_FUNCTION(pal_freeaddrinfo__neg)
{
    static const prx_addrinfo_t* k_info_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_freeaddrinfo(k_info_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // pal_freeaddrinfo;

#ifdef pal_getnameinfo

//
//Test pal_getnameinfo happy path 
// 
TEST_FUNCTION(pal_getnameinfo__success)
{
    static const prx_socket_address_t* k_address_valid;
    static const char* k_host_valid;
    static const size_t k_host_length_valid;
    static const char* k_service_valid;
    static const size_t k_service_length_valid;
    static const int32_t k_flags_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = pal_getnameinfo(k_address_valid, k_host_valid, k_host_length_valid, k_service_valid, k_service_length_valid, k_flags_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test pal_getnameinfo passing as address argument an invalid prx_socket_address_t* value 
// 
TEST_FUNCTION(pal_getnameinfo__arg_address_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_getnameinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_getnameinfo passing as host argument an invalid char* value 
// 
TEST_FUNCTION(pal_getnameinfo__arg_host_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_getnameinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_getnameinfo passing as host_length argument an invalid size_t value 
// 
TEST_FUNCTION(pal_getnameinfo__arg_host_length_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_getnameinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_getnameinfo passing as service argument an invalid char* value 
// 
TEST_FUNCTION(pal_getnameinfo__arg_service_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_getnameinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_getnameinfo passing as service_length argument an invalid size_t value 
// 
TEST_FUNCTION(pal_getnameinfo__arg_service_length_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_getnameinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_getnameinfo passing as flags argument an invalid int32_t value 
// 
TEST_FUNCTION(pal_getnameinfo__arg_flags_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_getnameinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_getnameinfo unhappy path 
// 
TEST_FUNCTION(pal_getnameinfo__neg)
{
    static const prx_socket_address_t* k_address_valid;
    static const char* k_host_valid;
    static const size_t k_host_length_valid;
    static const char* k_service_valid;
    static const size_t k_service_length_valid;
    static const int32_t k_flags_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_getnameinfo(k_address_valid, k_host_valid, k_host_length_valid, k_service_valid, k_service_length_valid, k_flags_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // pal_getnameinfo;

#ifdef pal_gethostname

//
//Test pal_gethostname happy path 
// 
TEST_FUNCTION(pal_gethostname__success)
{
    static const char* k_name_valid;
    static const size_t k_name_length_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = pal_gethostname(k_name_valid, k_name_length_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test pal_gethostname passing as name argument an invalid char* value 
// 
TEST_FUNCTION(pal_gethostname__arg_name_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_gethostname();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_gethostname passing as name_length argument an invalid size_t value 
// 
TEST_FUNCTION(pal_gethostname__arg_name_length_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_gethostname();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_gethostname unhappy path 
// 
TEST_FUNCTION(pal_gethostname__neg)
{
    static const char* k_name_valid;
    static const size_t k_name_length_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_gethostname(k_name_valid, k_name_length_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // pal_gethostname;

#ifdef pal_os_last_net_error_as_prx_error

//
//Test pal_os_last_net_error_as_prx_error happy path 
// 
TEST_FUNCTION(pal_os_last_net_error_as_prx_error__success)
{
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = pal_os_last_net_error_as_prx_error();

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test pal_os_last_net_error_as_prx_error unhappy path 
// 
TEST_FUNCTION(pal_os_last_net_error_as_prx_error__neg)
{
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_os_last_net_error_as_prx_error();

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // pal_os_last_net_error_as_prx_error;

#ifdef pal_os_set_net_error_as_prx_error

//
//Test pal_os_set_net_error_as_prx_error happy path 
// 
TEST_FUNCTION(pal_os_set_net_error_as_prx_error__success)
{
    static const int32_t k_error_valid;
    void result;

    // arrange 
    // ... 

    // act 
    result = pal_os_set_net_error_as_prx_error(k_error_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ... 
}

// 
// Test pal_os_set_net_error_as_prx_error passing as error argument an invalid int32_t value 
// 
TEST_FUNCTION(pal_os_set_net_error_as_prx_error__arg_error_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_os_set_net_error_as_prx_error();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_os_set_net_error_as_prx_error unhappy path 
// 
TEST_FUNCTION(pal_os_set_net_error_as_prx_error__neg)
{
    static const int32_t k_error_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_os_set_net_error_as_prx_error(k_error_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // pal_os_set_net_error_as_prx_error;


#ifdef pal_net_init 
// 
// Test pal_net_init happy path 
// 
TEST_FUNCTION(pal_net_init__success)
{
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = pal_net_init();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test pal_net_init unhappy path 
// 
TEST_FUNCTION(pal_net_init__neg)
{
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_net_init();

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif //pal_net_init 


#ifdef pal_net_deinit 
// 
// Test pal_net_deinit happy path 
// 
TEST_FUNCTION(pal_net_deinit__success)
{
    void result;

    // arrange 
    // ... 

    // act 
    result = pal_net_deinit();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ... 
}

// 
// Test pal_net_deinit unhappy path 
// 
TEST_FUNCTION(pal_net_deinit__neg)
{
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_net_deinit();

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif //pal_net_deinit 

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()
