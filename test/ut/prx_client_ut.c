// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST prx_client
#include "util_ut.h"

//
// 1. Required mocks
//
#include "prx_types.h"

//
// 2. Include unit under test
//
#undef ENABLE_MOCKS
#include UNIT_H
#define ENABLE_MOCKS
#include UNIT_C

//
// 3. Setup test suite
//
BEGIN_DECLARE_TEST_SUITE()
END_DECLARE_TEST_SUITE()
// -or- DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
DECLARE_TEST_SETUP()


#ifdef prx_client_pton // 
// Test prx_client_pton happy path 
// 
TEST_FUNCTION(prx_client_pton__success)
{
    static const const char* k_addr_string_valid;
    static const prx_socket_address_t* k_address_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = prx_client_pton(k_addr_string_valid, k_address_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test prx_client_pton passing as addr_string argument an invalid const char* value 
// 
TEST_FUNCTION(prx_client_pton__arg_addr_string_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_pton();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_pton passing as address argument an invalid prx_socket_address_t* value 
// 
TEST_FUNCTION(prx_client_pton__arg_address_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_pton();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_pton unhappy path 
// 
TEST_FUNCTION(prx_client_pton__neg)
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
    result = prx_client_pton(k_addr_string_valid, k_address_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_client_pton;

#ifdef prx_client_ntop // 
// Test prx_client_ntop happy path 
// 
TEST_FUNCTION(prx_client_ntop__success)
{
    static const prx_socket_address_t* k_address_valid;
    static const char* k_addr_string_valid;
    static const size_t k_addr_string_size_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = prx_client_ntop(k_address_valid, k_addr_string_valid, k_addr_string_size_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test prx_client_ntop passing as address argument an invalid prx_socket_address_t* value 
// 
TEST_FUNCTION(prx_client_ntop__arg_address_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_ntop();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_ntop passing as addr_string argument an invalid char* value 
// 
TEST_FUNCTION(prx_client_ntop__arg_addr_string_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_ntop();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_ntop passing as addr_string_size argument an invalid size_t value 
// 
TEST_FUNCTION(prx_client_ntop__arg_addr_string_size_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_ntop();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_ntop unhappy path 
// 
TEST_FUNCTION(prx_client_ntop__neg)
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
    result = prx_client_ntop(k_address_valid, k_addr_string_valid, k_addr_string_size_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_client_ntop;

#ifdef prx_client_getifaddrinfo // 
// Test prx_client_getifaddrinfo happy path 
// 
TEST_FUNCTION(prx_client_getifaddrinfo__success)
{
    static const const char* k_if_name_valid;
    static const uint32_t k_flags_valid;
    static const prx_ifaddrinfo_t** k_info_valid;
    static const size_t* k_info_count_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = prx_client_getifaddrinfo(k_if_name_valid, k_flags_valid, k_info_valid, k_info_count_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test prx_client_getifaddrinfo passing as if_name argument an invalid const char* value 
// 
TEST_FUNCTION(prx_client_getifaddrinfo__arg_if_name_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_getifaddrinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_getifaddrinfo passing as flags argument an invalid uint32_t value 
// 
TEST_FUNCTION(prx_client_getifaddrinfo__arg_flags_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_getifaddrinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_getifaddrinfo passing as info argument an invalid prx_ifaddrinfo_t** value 
// 
TEST_FUNCTION(prx_client_getifaddrinfo__arg_info_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_getifaddrinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_getifaddrinfo passing as info_count argument an invalid size_t* value 
// 
TEST_FUNCTION(prx_client_getifaddrinfo__arg_info_count_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_getifaddrinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_getifaddrinfo unhappy path 
// 
TEST_FUNCTION(prx_client_getifaddrinfo__neg)
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
    result = prx_client_getifaddrinfo(k_if_name_valid, k_flags_valid, k_info_valid, k_info_count_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_client_getifaddrinfo;

#ifdef prx_client_freeifaddrinfo // 
// Test prx_client_freeifaddrinfo happy path 
// 
TEST_FUNCTION(prx_client_freeifaddrinfo__success)
{
    static const prx_ifaddrinfo_t* k_info_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = prx_client_freeifaddrinfo(k_info_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test prx_client_freeifaddrinfo passing as info argument an invalid prx_ifaddrinfo_t* value 
// 
TEST_FUNCTION(prx_client_freeifaddrinfo__arg_info_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_freeifaddrinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_freeifaddrinfo unhappy path 
// 
TEST_FUNCTION(prx_client_freeifaddrinfo__neg)
{
    static const prx_ifaddrinfo_t* k_info_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_client_freeifaddrinfo(k_info_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_client_freeifaddrinfo;

#ifdef prx_client_getifnameinfo // 
// Test prx_client_getifnameinfo happy path 
// 
TEST_FUNCTION(prx_client_getifnameinfo__success)
{
    static const prx_socket_address_t* k_if_address_valid;
    static const char* k_if_name_valid;
    static const size_t k_if_name_length_valid;
    static const uint64_t* k_if_index_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = prx_client_getifnameinfo(k_if_address_valid, k_if_name_valid, k_if_name_length_valid, k_if_index_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test prx_client_getifnameinfo passing as if_address argument an invalid prx_socket_address_t* value 
// 
TEST_FUNCTION(prx_client_getifnameinfo__arg_if_address_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_getifnameinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_getifnameinfo passing as if_name argument an invalid char* value 
// 
TEST_FUNCTION(prx_client_getifnameinfo__arg_if_name_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_getifnameinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_getifnameinfo passing as if_name_length argument an invalid size_t value 
// 
TEST_FUNCTION(prx_client_getifnameinfo__arg_if_name_length_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_getifnameinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_getifnameinfo passing as if_index argument an invalid uint64_t* value 
// 
TEST_FUNCTION(prx_client_getifnameinfo__arg_if_index_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_getifnameinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_getifnameinfo unhappy path 
// 
TEST_FUNCTION(prx_client_getifnameinfo__neg)
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
    result = prx_client_getifnameinfo(k_if_address_valid, k_if_name_valid, k_if_name_length_valid, k_if_index_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_client_getifnameinfo;

#ifdef prx_client_getaddrinfo // 
// Test prx_client_getaddrinfo happy path 
// 
TEST_FUNCTION(prx_client_getaddrinfo__success)
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
    result = prx_client_getaddrinfo(k_host_name_valid, k_service_valid, k_family_valid, k_flags_valid, k_info_valid, k_info_count_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test prx_client_getaddrinfo passing as host_name argument an invalid const char* value 
// 
TEST_FUNCTION(prx_client_getaddrinfo__arg_host_name_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_getaddrinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_getaddrinfo passing as service argument an invalid const char* value 
// 
TEST_FUNCTION(prx_client_getaddrinfo__arg_service_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_getaddrinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_getaddrinfo passing as family argument an invalid prx_address_family_t value 
// 
TEST_FUNCTION(prx_client_getaddrinfo__arg_family_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_getaddrinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_getaddrinfo passing as flags argument an invalid uint32_t value 
// 
TEST_FUNCTION(prx_client_getaddrinfo__arg_flags_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_getaddrinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_getaddrinfo passing as info argument an invalid prx_addrinfo_t** value 
// 
TEST_FUNCTION(prx_client_getaddrinfo__arg_info_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_getaddrinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_getaddrinfo passing as info_count argument an invalid size_t* value 
// 
TEST_FUNCTION(prx_client_getaddrinfo__arg_info_count_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_getaddrinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_getaddrinfo unhappy path 
// 
TEST_FUNCTION(prx_client_getaddrinfo__neg)
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
    result = prx_client_getaddrinfo(k_host_name_valid, k_service_valid, k_family_valid, k_flags_valid, k_info_valid, k_info_count_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_client_getaddrinfo;

#ifdef prx_client_freeaddrinfo // 
// Test prx_client_freeaddrinfo happy path 
// 
TEST_FUNCTION(prx_client_freeaddrinfo__success)
{
    static const prx_addrinfo_t* k_info_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = prx_client_freeaddrinfo(k_info_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test prx_client_freeaddrinfo passing as info argument an invalid prx_addrinfo_t* value 
// 
TEST_FUNCTION(prx_client_freeaddrinfo__arg_info_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_freeaddrinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_freeaddrinfo unhappy path 
// 
TEST_FUNCTION(prx_client_freeaddrinfo__neg)
{
    static const prx_addrinfo_t* k_info_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_client_freeaddrinfo(k_info_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_client_freeaddrinfo;

#ifdef prx_client_getnameinfo // 
// Test prx_client_getnameinfo happy path 
// 
TEST_FUNCTION(prx_client_getnameinfo__success)
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
    result = prx_client_getnameinfo(k_address_valid, k_host_valid, k_host_length_valid, k_service_valid, k_service_length_valid, k_flags_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test prx_client_getnameinfo passing as address argument an invalid prx_socket_address_t* value 
// 
TEST_FUNCTION(prx_client_getnameinfo__arg_address_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_getnameinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_getnameinfo passing as host argument an invalid char* value 
// 
TEST_FUNCTION(prx_client_getnameinfo__arg_host_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_getnameinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_getnameinfo passing as host_length argument an invalid size_t value 
// 
TEST_FUNCTION(prx_client_getnameinfo__arg_host_length_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_getnameinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_getnameinfo passing as service argument an invalid char* value 
// 
TEST_FUNCTION(prx_client_getnameinfo__arg_service_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_getnameinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_getnameinfo passing as service_length argument an invalid size_t value 
// 
TEST_FUNCTION(prx_client_getnameinfo__arg_service_length_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_getnameinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_getnameinfo passing as flags argument an invalid int32_t value 
// 
TEST_FUNCTION(prx_client_getnameinfo__arg_flags_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_getnameinfo();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_getnameinfo unhappy path 
// 
TEST_FUNCTION(prx_client_getnameinfo__neg)
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
    result = prx_client_getnameinfo(k_address_valid, k_host_valid, k_host_length_valid, k_service_valid, k_service_length_valid, k_flags_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_client_getnameinfo;

#ifdef prx_gethostname // 
// Test prx_gethostname happy path 
// 
TEST_FUNCTION(prx_gethostname__success)
{
    static const char* k_name_valid;
    static const size_t k_name_length_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = prx_gethostname(k_name_valid, k_name_length_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test prx_gethostname passing as name argument an invalid char* value 
// 
TEST_FUNCTION(prx_gethostname__arg_name_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_gethostname();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_gethostname passing as name_length argument an invalid size_t value 
// 
TEST_FUNCTION(prx_gethostname__arg_name_length_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_gethostname();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_gethostname unhappy path 
// 
TEST_FUNCTION(prx_gethostname__neg)
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
    result = prx_gethostname(k_name_valid, k_name_length_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_gethostname;

#ifdef prx_client_socket // 
// Test prx_client_socket happy path 
// 
TEST_FUNCTION(prx_client_socket__success)
{
    static const prx_address_family_t k_address_family_valid;
    static const prx_socket_type_t k_socket_type_valid;
    static const prx_protocol_type_t k_protocol_type_valid;
    static const prx_fd_t* k_fd_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = prx_client_socket(k_address_family_valid, k_socket_type_valid, k_protocol_type_valid, k_fd_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test prx_client_socket passing as address_family argument an invalid prx_address_family_t value 
// 
TEST_FUNCTION(prx_client_socket__arg_address_family_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_socket();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_socket passing as socket_type argument an invalid prx_socket_type_t value 
// 
TEST_FUNCTION(prx_client_socket__arg_socket_type_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_socket();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_socket passing as protocol_type argument an invalid prx_protocol_type_t value 
// 
TEST_FUNCTION(prx_client_socket__arg_protocol_type_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_socket();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_socket passing as fd argument an invalid prx_fd_t* value 
// 
TEST_FUNCTION(prx_client_socket__arg_fd_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_socket();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_socket unhappy path 
// 
TEST_FUNCTION(prx_client_socket__neg)
{
    static const prx_address_family_t k_address_family_valid;
    static const prx_socket_type_t k_socket_type_valid;
    static const prx_protocol_type_t k_protocol_type_valid;
    static const prx_fd_t* k_fd_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_client_socket(k_address_family_valid, k_socket_type_valid, k_protocol_type_valid, k_fd_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_client_socket;

#ifdef prx_client_listen // 
// Test prx_client_listen happy path 
// 
TEST_FUNCTION(prx_client_listen__success)
{
    static const prx_fd_t k_fd_valid;
    static const int32_t k_backlog_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = prx_client_listen(k_fd_valid, k_backlog_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test prx_client_listen passing as fd argument an invalid prx_fd_t value 
// 
TEST_FUNCTION(prx_client_listen__arg_fd_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_listen();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_listen passing as backlog argument an invalid int32_t value 
// 
TEST_FUNCTION(prx_client_listen__arg_backlog_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_listen();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_listen unhappy path 
// 
TEST_FUNCTION(prx_client_listen__neg)
{
    static const prx_fd_t k_fd_valid;
    static const int32_t k_backlog_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_client_listen(k_fd_valid, k_backlog_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_client_listen;

#ifdef prx_client_bind // 
// Test prx_client_bind happy path 
// 
TEST_FUNCTION(prx_client_bind__success)
{
    static const prx_fd_t k_fd_valid;
    static const prx_socket_address_t* k_socket_address_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = prx_client_bind(k_fd_valid, k_socket_address_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test prx_client_bind passing as fd argument an invalid prx_fd_t value 
// 
TEST_FUNCTION(prx_client_bind__arg_fd_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_bind();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_bind passing as socket_address argument an invalid prx_socket_address_t* value 
// 
TEST_FUNCTION(prx_client_bind__arg_socket_address_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_bind();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_bind unhappy path 
// 
TEST_FUNCTION(prx_client_bind__neg)
{
    static const prx_fd_t k_fd_valid;
    static const prx_socket_address_t* k_socket_address_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_client_bind(k_fd_valid, k_socket_address_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_client_bind;

#ifdef prx_client_connect // 
// Test prx_client_connect happy path 
// 
TEST_FUNCTION(prx_client_connect__success)
{
    static const prx_fd_t k_fd_valid;
    static const prx_socket_address_t* k_socket_address_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = prx_client_connect(k_fd_valid, k_socket_address_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test prx_client_connect passing as fd argument an invalid prx_fd_t value 
// 
TEST_FUNCTION(prx_client_connect__arg_fd_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_connect();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_connect passing as socket_address argument an invalid prx_socket_address_t* value 
// 
TEST_FUNCTION(prx_client_connect__arg_socket_address_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_connect();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_connect unhappy path 
// 
TEST_FUNCTION(prx_client_connect__neg)
{
    static const prx_fd_t k_fd_valid;
    static const prx_socket_address_t* k_socket_address_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_client_connect(k_fd_valid, k_socket_address_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_client_connect;

#ifdef prx_client_getpeername // 
// Test prx_client_getpeername happy path 
// 
TEST_FUNCTION(prx_client_getpeername__success)
{
    static const prx_fd_t k_fd_valid;
    static const prx_socket_address_t* k_socket_address_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = prx_client_getpeername(k_fd_valid, k_socket_address_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test prx_client_getpeername passing as fd argument an invalid prx_fd_t value 
// 
TEST_FUNCTION(prx_client_getpeername__arg_fd_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_getpeername();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_getpeername passing as socket_address argument an invalid prx_socket_address_t* value 
// 
TEST_FUNCTION(prx_client_getpeername__arg_socket_address_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_getpeername();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_getpeername unhappy path 
// 
TEST_FUNCTION(prx_client_getpeername__neg)
{
    static const prx_fd_t k_fd_valid;
    static const prx_socket_address_t* k_socket_address_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_client_getpeername(k_fd_valid, k_socket_address_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_client_getpeername;

#ifdef prx_client_getsockname // 
// Test prx_client_getsockname happy path 
// 
TEST_FUNCTION(prx_client_getsockname__success)
{
    static const prx_fd_t k_fd_valid;
    static const prx_socket_address_t* k_socket_address_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = prx_client_getsockname(k_fd_valid, k_socket_address_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test prx_client_getsockname passing as fd argument an invalid prx_fd_t value 
// 
TEST_FUNCTION(prx_client_getsockname__arg_fd_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_getsockname();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_getsockname passing as socket_address argument an invalid prx_socket_address_t* value 
// 
TEST_FUNCTION(prx_client_getsockname__arg_socket_address_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_getsockname();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_getsockname unhappy path 
// 
TEST_FUNCTION(prx_client_getsockname__neg)
{
    static const prx_fd_t k_fd_valid;
    static const prx_socket_address_t* k_socket_address_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_client_getsockname(k_fd_valid, k_socket_address_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_client_getsockname;

#ifdef prx_client_accept // 
// Test prx_client_accept happy path 
// 
TEST_FUNCTION(prx_client_accept__success)
{
    static const prx_fd_t k_fd_valid;
    static const uintptr_t k_key_valid;
    static const prx_socket_address_t* k_socket_address_valid;
    static const prx_fd_t* k_accepted_socket_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = prx_client_accept(k_fd_valid, k_key_valid, k_socket_address_valid, k_accepted_socket_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test prx_client_accept passing as fd argument an invalid prx_fd_t value 
// 
TEST_FUNCTION(prx_client_accept__arg_fd_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_accept();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_accept passing as key argument an invalid uintptr_t value 
// 
TEST_FUNCTION(prx_client_accept__arg_key_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_accept();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_accept passing as socket_address argument an invalid prx_socket_address_t* value 
// 
TEST_FUNCTION(prx_client_accept__arg_socket_address_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_accept();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_accept passing as accepted_socket argument an invalid prx_fd_t* value 
// 
TEST_FUNCTION(prx_client_accept__arg_accepted_socket_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_accept();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_accept unhappy path 
// 
TEST_FUNCTION(prx_client_accept__neg)
{
    static const prx_fd_t k_fd_valid;
    static const uintptr_t k_key_valid;
    static const prx_socket_address_t* k_socket_address_valid;
    static const prx_fd_t* k_accepted_socket_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_client_accept(k_fd_valid, k_key_valid, k_socket_address_valid, k_accepted_socket_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_client_accept;

#ifdef prx_client_recv // 
// Test prx_client_recv happy path 
// 
TEST_FUNCTION(prx_client_recv__success)
{
    static const prx_fd_t k_fd_valid;
    static const uintptr_t k_key_valid;
    static const int32_t k_flags_valid;
    static const uint8_t* k_buffer_valid;
    static const size_t k_offset_valid;
    static const size_t k_length_valid;
    static const size_t* k_received_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = prx_client_recv(k_fd_valid, k_key_valid, k_flags_valid, k_buffer_valid, k_offset_valid, k_length_valid, k_received_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test prx_client_recv passing as fd argument an invalid prx_fd_t value 
// 
TEST_FUNCTION(prx_client_recv__arg_fd_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_recv();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_recv passing as key argument an invalid uintptr_t value 
// 
TEST_FUNCTION(prx_client_recv__arg_key_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_recv();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_recv passing as flags argument an invalid int32_t value 
// 
TEST_FUNCTION(prx_client_recv__arg_flags_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_recv();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_recv passing as buffer argument an invalid uint8_t* value 
// 
TEST_FUNCTION(prx_client_recv__arg_buffer_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_recv();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_recv passing as offset argument an invalid size_t value 
// 
TEST_FUNCTION(prx_client_recv__arg_offset_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_recv();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_recv passing as length argument an invalid size_t value 
// 
TEST_FUNCTION(prx_client_recv__arg_length_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_recv();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_recv passing as received argument an invalid size_t* value 
// 
TEST_FUNCTION(prx_client_recv__arg_received_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_recv();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_recv unhappy path 
// 
TEST_FUNCTION(prx_client_recv__neg)
{
    static const prx_fd_t k_fd_valid;
    static const uintptr_t k_key_valid;
    static const int32_t k_flags_valid;
    static const uint8_t* k_buffer_valid;
    static const size_t k_offset_valid;
    static const size_t k_length_valid;
    static const size_t* k_received_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_client_recv(k_fd_valid, k_key_valid, k_flags_valid, k_buffer_valid, k_offset_valid, k_length_valid, k_received_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_client_recv;

#ifdef prx_client_recvfrom // 
// Test prx_client_recvfrom happy path 
// 
TEST_FUNCTION(prx_client_recvfrom__success)
{
    static const prx_fd_t k_fd_valid;
    static const uintptr_t k_key_valid;
    static const int32_t k_flags_valid;
    static const uint8_t* k_buffer_valid;
    static const size_t k_offset_valid;
    static const size_t k_length_valid;
    static const prx_socket_address_t* k_socket_address_valid;
    static const size_t* k_received_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = prx_client_recvfrom(k_fd_valid, k_key_valid, k_flags_valid, k_buffer_valid, k_offset_valid, k_length_valid, k_socket_address_valid, k_received_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test prx_client_recvfrom passing as fd argument an invalid prx_fd_t value 
// 
TEST_FUNCTION(prx_client_recvfrom__arg_fd_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_recvfrom();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_recvfrom passing as key argument an invalid uintptr_t value 
// 
TEST_FUNCTION(prx_client_recvfrom__arg_key_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_recvfrom();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_recvfrom passing as flags argument an invalid int32_t value 
// 
TEST_FUNCTION(prx_client_recvfrom__arg_flags_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_recvfrom();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_recvfrom passing as buffer argument an invalid uint8_t* value 
// 
TEST_FUNCTION(prx_client_recvfrom__arg_buffer_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_recvfrom();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_recvfrom passing as offset argument an invalid size_t value 
// 
TEST_FUNCTION(prx_client_recvfrom__arg_offset_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_recvfrom();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_recvfrom passing as length argument an invalid size_t value 
// 
TEST_FUNCTION(prx_client_recvfrom__arg_length_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_recvfrom();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_recvfrom passing as socket_address argument an invalid prx_socket_address_t* value 
// 
TEST_FUNCTION(prx_client_recvfrom__arg_socket_address_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_recvfrom();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_recvfrom passing as received argument an invalid size_t* value 
// 
TEST_FUNCTION(prx_client_recvfrom__arg_received_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_recvfrom();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_recvfrom unhappy path 
// 
TEST_FUNCTION(prx_client_recvfrom__neg)
{
    static const prx_fd_t k_fd_valid;
    static const uintptr_t k_key_valid;
    static const int32_t k_flags_valid;
    static const uint8_t* k_buffer_valid;
    static const size_t k_offset_valid;
    static const size_t k_length_valid;
    static const prx_socket_address_t* k_socket_address_valid;
    static const size_t* k_received_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_client_recvfrom(k_fd_valid, k_key_valid, k_flags_valid, k_buffer_valid, k_offset_valid, k_length_valid, k_socket_address_valid, k_received_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_client_recvfrom;

#ifdef prx_client_send // 
// Test prx_client_send happy path 
// 
TEST_FUNCTION(prx_client_send__success)
{
    static const prx_fd_t k_fd_valid;
    static const uintptr_t k_key_valid;
    static const int32_t k_flags_valid;
    static const uint8_t* k_buffer_valid;
    static const size_t k_offset_valid;
    static const size_t k_length_valid;
    static const size_t* k_sent_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = prx_client_send(k_fd_valid, k_key_valid, k_flags_valid, k_buffer_valid, k_offset_valid, k_length_valid, k_sent_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test prx_client_send passing as fd argument an invalid prx_fd_t value 
// 
TEST_FUNCTION(prx_client_send__arg_fd_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_send();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_send passing as key argument an invalid uintptr_t value 
// 
TEST_FUNCTION(prx_client_send__arg_key_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_send();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_send passing as flags argument an invalid int32_t value 
// 
TEST_FUNCTION(prx_client_send__arg_flags_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_send();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_send passing as buffer argument an invalid uint8_t* value 
// 
TEST_FUNCTION(prx_client_send__arg_buffer_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_send();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_send passing as offset argument an invalid size_t value 
// 
TEST_FUNCTION(prx_client_send__arg_offset_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_send();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_send passing as length argument an invalid size_t value 
// 
TEST_FUNCTION(prx_client_send__arg_length_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_send();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_send passing as sent argument an invalid size_t* value 
// 
TEST_FUNCTION(prx_client_send__arg_sent_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_send();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_send unhappy path 
// 
TEST_FUNCTION(prx_client_send__neg)
{
    static const prx_fd_t k_fd_valid;
    static const uintptr_t k_key_valid;
    static const int32_t k_flags_valid;
    static const uint8_t* k_buffer_valid;
    static const size_t k_offset_valid;
    static const size_t k_length_valid;
    static const size_t* k_sent_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_client_send(k_fd_valid, k_key_valid, k_flags_valid, k_buffer_valid, k_offset_valid, k_length_valid, k_sent_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_client_send;

#ifdef prx_client_sendto // 
// Test prx_client_sendto happy path 
// 
TEST_FUNCTION(prx_client_sendto__success)
{
    static const prx_fd_t k_fd_valid;
    static const uintptr_t k_key_valid;
    static const int32_t k_flags_valid;
    static const uint8_t* k_buffer_valid;
    static const size_t k_offset_valid;
    static const size_t k_length_valid;
    static const prx_socket_address_t* k_address_valid;
    static const size_t* k_sent_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = prx_client_sendto(k_fd_valid, k_key_valid, k_flags_valid, k_buffer_valid, k_offset_valid, k_length_valid, k_address_valid, k_sent_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test prx_client_sendto passing as fd argument an invalid prx_fd_t value 
// 
TEST_FUNCTION(prx_client_sendto__arg_fd_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_sendto();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_sendto passing as key argument an invalid uintptr_t value 
// 
TEST_FUNCTION(prx_client_sendto__arg_key_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_sendto();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_sendto passing as flags argument an invalid int32_t value 
// 
TEST_FUNCTION(prx_client_sendto__arg_flags_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_sendto();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_sendto passing as buffer argument an invalid uint8_t* value 
// 
TEST_FUNCTION(prx_client_sendto__arg_buffer_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_sendto();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_sendto passing as offset argument an invalid size_t value 
// 
TEST_FUNCTION(prx_client_sendto__arg_offset_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_sendto();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_sendto passing as length argument an invalid size_t value 
// 
TEST_FUNCTION(prx_client_sendto__arg_length_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_sendto();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_sendto passing as address argument an invalid prx_socket_address_t* value 
// 
TEST_FUNCTION(prx_client_sendto__arg_address_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_sendto();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_sendto passing as sent argument an invalid size_t* value 
// 
TEST_FUNCTION(prx_client_sendto__arg_sent_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_sendto();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_sendto unhappy path 
// 
TEST_FUNCTION(prx_client_sendto__neg)
{
    static const prx_fd_t k_fd_valid;
    static const uintptr_t k_key_valid;
    static const int32_t k_flags_valid;
    static const uint8_t* k_buffer_valid;
    static const size_t k_offset_valid;
    static const size_t k_length_valid;
    static const prx_socket_address_t* k_address_valid;
    static const size_t* k_sent_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_client_sendto(k_fd_valid, k_key_valid, k_flags_valid, k_buffer_valid, k_offset_valid, k_length_valid, k_address_valid, k_sent_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_client_sendto;

#ifdef prx_client_getsockopt // 
// Test prx_client_getsockopt happy path 
// 
TEST_FUNCTION(prx_client_getsockopt__success)
{
    static const prx_fd_t k_fd_valid;
    static const prx_socket_option_t k_option_valid;
    static const uint64_t* k_value_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = prx_client_getsockopt(k_fd_valid, k_option_valid, k_value_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test prx_client_getsockopt passing as fd argument an invalid prx_fd_t value 
// 
TEST_FUNCTION(prx_client_getsockopt__arg_fd_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_getsockopt();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_getsockopt passing as option argument an invalid prx_socket_option_t value 
// 
TEST_FUNCTION(prx_client_getsockopt__arg_option_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_getsockopt();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_getsockopt passing as value argument an invalid uint64_t* value 
// 
TEST_FUNCTION(prx_client_getsockopt__arg_value_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_getsockopt();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_getsockopt unhappy path 
// 
TEST_FUNCTION(prx_client_getsockopt__neg)
{
    static const prx_fd_t k_fd_valid;
    static const prx_socket_option_t k_option_valid;
    static const uint64_t* k_value_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_client_getsockopt(k_fd_valid, k_option_valid, k_value_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_client_getsockopt;

#ifdef prx_client_setsockopt // 
// Test prx_client_setsockopt happy path 
// 
TEST_FUNCTION(prx_client_setsockopt__success)
{
    static const prx_fd_t k_fd_valid;
    static const prx_socket_option_t k_option_valid;
    static const uint64_t k_value_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = prx_client_setsockopt(k_fd_valid, k_option_valid, k_value_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test prx_client_setsockopt passing as fd argument an invalid prx_fd_t value 
// 
TEST_FUNCTION(prx_client_setsockopt__arg_fd_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_setsockopt();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_setsockopt passing as option argument an invalid prx_socket_option_t value 
// 
TEST_FUNCTION(prx_client_setsockopt__arg_option_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_setsockopt();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_setsockopt passing as value argument an invalid uint64_t value 
// 
TEST_FUNCTION(prx_client_setsockopt__arg_value_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_setsockopt();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_setsockopt unhappy path 
// 
TEST_FUNCTION(prx_client_setsockopt__neg)
{
    static const prx_fd_t k_fd_valid;
    static const prx_socket_option_t k_option_valid;
    static const uint64_t k_value_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_client_setsockopt(k_fd_valid, k_option_valid, k_value_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_client_setsockopt;

#ifdef prx_join_multicast_group // 
// Test prx_join_multicast_group happy path 
// 
TEST_FUNCTION(prx_join_multicast_group__success)
{
    static const prx_fd_t k_fd_valid;
    static const prx_multicast_option_t* k_option_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = prx_join_multicast_group(k_fd_valid, k_option_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test prx_join_multicast_group passing as fd argument an invalid prx_fd_t value 
// 
TEST_FUNCTION(prx_join_multicast_group__arg_fd_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_join_multicast_group();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_join_multicast_group passing as option argument an invalid prx_multicast_option_t* value 
// 
TEST_FUNCTION(prx_join_multicast_group__arg_option_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_join_multicast_group();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_join_multicast_group unhappy path 
// 
TEST_FUNCTION(prx_join_multicast_group__neg)
{
    static const prx_fd_t k_fd_valid;
    static const prx_multicast_option_t* k_option_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_join_multicast_group(k_fd_valid, k_option_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_join_multicast_group;

#ifdef prx_leave_multicast_group // 
// Test prx_leave_multicast_group happy path 
// 
TEST_FUNCTION(prx_leave_multicast_group__success)
{
    static const prx_fd_t k_fd_valid;
    static const prx_multicast_option_t* k_option_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = prx_leave_multicast_group(k_fd_valid, k_option_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test prx_leave_multicast_group passing as fd argument an invalid prx_fd_t value 
// 
TEST_FUNCTION(prx_leave_multicast_group__arg_fd_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_leave_multicast_group();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_leave_multicast_group passing as option argument an invalid prx_multicast_option_t* value 
// 
TEST_FUNCTION(prx_leave_multicast_group__arg_option_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_leave_multicast_group();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_leave_multicast_group unhappy path 
// 
TEST_FUNCTION(prx_leave_multicast_group__neg)
{
    static const prx_fd_t k_fd_valid;
    static const prx_multicast_option_t* k_option_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_leave_multicast_group(k_fd_valid, k_option_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_leave_multicast_group;

#ifdef prx_client_poll // 
// Test prx_client_poll happy path 
// 
TEST_FUNCTION(prx_client_poll__success)
{
    static const size_t k_num_valid;
    static const prx_fd_t* k_sockets_valid;
    static const int32_t* k_timeout_ms_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = prx_client_poll(k_num_valid, k_sockets_valid, k_timeout_ms_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test prx_client_poll passing as num argument an invalid size_t value 
// 
TEST_FUNCTION(prx_client_poll__arg_num_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_poll();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_poll passing as sockets argument an invalid prx_fd_t* value 
// 
TEST_FUNCTION(prx_client_poll__arg_sockets_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_poll();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_poll passing as timeout_ms argument an invalid int32_t* value 
// 
TEST_FUNCTION(prx_client_poll__arg_timeout_ms_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_poll();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_poll unhappy path 
// 
TEST_FUNCTION(prx_client_poll__neg)
{
    static const size_t k_num_valid;
    static const prx_fd_t* k_sockets_valid;
    static const int32_t* k_timeout_ms_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_client_poll(k_num_valid, k_sockets_valid, k_timeout_ms_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_client_poll;

#ifdef prx_client_can_recv // 
// Test prx_client_can_recv happy path 
// 
TEST_FUNCTION(prx_client_can_recv__success)
{
    static const prx_fd_t k_fd_valid;
    bool result;

    // arrange 
    // ... 

    // act 
    result = prx_client_can_recv(k_fd_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(bool, er_ok, result);
    // ... 
}

// 
// Test prx_client_can_recv passing as fd argument an invalid prx_fd_t value 
// 
TEST_FUNCTION(prx_client_can_recv__arg_fd_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_can_recv();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_can_recv unhappy path 
// 
TEST_FUNCTION(prx_client_can_recv__neg)
{
    static const prx_fd_t k_fd_valid;
    bool result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_client_can_recv(k_fd_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(bool, result, er_ok);
}

#endif // prx_client_can_recv;

#ifdef prx_client_can_send // 
// Test prx_client_can_send happy path 
// 
TEST_FUNCTION(prx_client_can_send__success)
{
    static const prx_fd_t k_fd_valid;
    bool result;

    // arrange 
    // ... 

    // act 
    result = prx_client_can_send(k_fd_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(bool, er_ok, result);
    // ... 
}

// 
// Test prx_client_can_send passing as fd argument an invalid prx_fd_t value 
// 
TEST_FUNCTION(prx_client_can_send__arg_fd_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_can_send();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_can_send unhappy path 
// 
TEST_FUNCTION(prx_client_can_send__neg)
{
    static const prx_fd_t k_fd_valid;
    bool result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_client_can_send(k_fd_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(bool, result, er_ok);
}

#endif // prx_client_can_send;

#ifdef prx_client_has_error // 
// Test prx_client_has_error happy path 
// 
TEST_FUNCTION(prx_client_has_error__success)
{
    static const prx_fd_t k_fd_valid;
    bool result;

    // arrange 
    // ... 

    // act 
    result = prx_client_has_error(k_fd_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(bool, er_ok, result);
    // ... 
}

// 
// Test prx_client_has_error passing as fd argument an invalid prx_fd_t value 
// 
TEST_FUNCTION(prx_client_has_error__arg_fd_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_has_error();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_has_error unhappy path 
// 
TEST_FUNCTION(prx_client_has_error__neg)
{
    static const prx_fd_t k_fd_valid;
    bool result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_client_has_error(k_fd_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(bool, result, er_ok);
}

#endif // prx_client_has_error;

#ifdef prx_client_is_disconnected // 
// Test prx_client_is_disconnected happy path 
// 
TEST_FUNCTION(prx_client_is_disconnected__success)
{
    static const prx_fd_t k_fd_valid;
    bool result;

    // arrange 
    // ... 

    // act 
    result = prx_client_is_disconnected(k_fd_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(bool, er_ok, result);
    // ... 
}

// 
// Test prx_client_is_disconnected passing as fd argument an invalid prx_fd_t value 
// 
TEST_FUNCTION(prx_client_is_disconnected__arg_fd_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_is_disconnected();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_is_disconnected unhappy path 
// 
TEST_FUNCTION(prx_client_is_disconnected__neg)
{
    static const prx_fd_t k_fd_valid;
    bool result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_client_is_disconnected(k_fd_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(bool, result, er_ok);
}

#endif // prx_client_is_disconnected;

#ifdef prx_client_init // 
// Test prx_client_init happy path 
// 
TEST_FUNCTION(prx_client_init__success)
{
    static const const char* k_config_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = prx_client_init(k_config_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test prx_client_init passing as config argument an invalid const char* value 
// 
TEST_FUNCTION(prx_client_init__arg_config_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = prx_client_init();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test prx_client_init unhappy path 
// 
TEST_FUNCTION(prx_client_init__neg)
{
    static const const char* k_config_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_client_init(k_config_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_client_init;

#ifdef prx_client_deinit // 
// Test prx_client_deinit happy path 
// 
TEST_FUNCTION(prx_client_deinit__success)
{
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = prx_client_deinit();

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test prx_client_deinit unhappy path 
// 
TEST_FUNCTION(prx_client_deinit__neg)
{
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_client_deinit();

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_client_deinit;

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

