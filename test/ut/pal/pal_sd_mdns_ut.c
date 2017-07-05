// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST pal_sd_mdns
#include "util_ut.h"

//
// 1. Required mocks
//
#include "os.h"
#include "prx_types.h"

// dns_sd.h
MOCKABLE_FUNCTION(, void, DNSServiceRefDeallocate,
    DNSServiceRef, sdRef);
MOCKABLE_FUNCTION(, dnssd_sock_t, DNSServiceRefSockFD,
    DNSServiceRef, sdRef);
MOCKABLE_FUNCTION(, DNSServiceErrorType, DNSServiceProcessResult,
    DNSServiceRef, sdRef);
MOCKABLE_FUNCTION(, DNSServiceErrorType, DNSServiceEnumerateDomains,
    DNSServiceRef*, sdRef, DNSServiceFlags, flags, uint32_t, interfaceIndex, DNSServiceDomainEnumReply, callBack, void*, context);
MOCKABLE_FUNCTION(, DNSServiceErrorType, DNSServiceBrowse,
    DNSServiceRef*, sdRef, DNSServiceFlags, flags, uint32_t, interfaceIndex, const char*, regtype, const char*, domain, DNSServiceBrowseReply, callBack, void*, context);
MOCKABLE_FUNCTION(, DNSServiceErrorType, DNSServiceResolve,
    DNSServiceRef*, sdRef, DNSServiceFlags, flags, uint32_t, interfaceIndex, const char*, name, const char*, regtype, const char*, domain, DNSServiceResolveReply, callBack, void*, context);
MOCKABLE_FUNCTION(, DNSServiceErrorType, DNSServiceGetAddrInfo,
    DNSServiceRef*, sdRef, DNSServiceFlags, flags, uint32_t, interfaceIndex, DNSServiceProtocol, protocol, const char*, hostname, DNSServiceGetAddrInfoReply, callBack, void*, context);

MOCKABLE_FUNCTION(, DNSServiceErrorType, DNSServiceCreateConnection,
    DNSServiceRef*, sdRef);

//
// 2. Include unit under test
//
#undef ENABLE_MOCKS
#include "pal_sd.h"
#define ENABLE_MOCKS
#include UNIT_C

//
// 3. Setup test suite
//
BEGIN_DECLARE_TEST_SUITE()
REGISTER_UMOCK_ALIAS_TYPE(dnssd_sock_t, int);
REGISTER_UMOCK_ALIAS_TYPE(DNSServiceErrorType, int);
REGISTER_UMOCK_ALIAS_TYPE(DNSServiceFlags, int);
REGISTER_UMOCK_ALIAS_TYPE(DNSServiceProtocol, int);
REGISTER_UMOCK_ALIAS_TYPE(DNSServiceRef, void*);
REGISTER_UMOCK_ALIAS_TYPE(DNSServiceDomainEnumReply, void*);
REGISTER_UMOCK_ALIAS_TYPE(DNSServiceBrowseReply, void*);
REGISTER_UMOCK_ALIAS_TYPE(DNSServiceResolveReply, void*);
REGISTER_UMOCK_ALIAS_TYPE(DNSServiceGetAddrInfoReply, void*);
END_DECLARE_TEST_SUITE()
// -or- DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
DECLARE_TEST_SETUP()


#ifdef pal_sd_init 
// 
// Test pal_sd_init happy path 
// 
TEST_FUNCTION(pal_mdns_sd_init__success)
{
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = pal_sd_init();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test pal_sd_init unhappy path 
// 
TEST_FUNCTION(pal_mdns_sd_init__neg)
{
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_sd_init();

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // pal_sd_init 

#ifdef pal_sdclient_create 
// 
// Test pal_sdclient_create happy path 
// 
TEST_FUNCTION(pal_mdns_sdclient_create__success)
{
    static const pal_sdclient_t** k_client_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = pal_sdclient_create(k_client_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test pal_sdclient_create passing as client argument an invalid pal_sdclient_t** value 
// 
TEST_FUNCTION(pal_mdns_sdclient_create__arg_client_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_sdclient_create();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_sdclient_create unhappy path 
// 
TEST_FUNCTION(pal_mdns_sdclient_create__neg)
{
    static const pal_sdclient_t** k_client_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_sdclient_create(k_client_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // pal_sdclient_create 

#ifdef pal_sdbrowser_create 
// 
// Test pal_sdbrowser_create happy path 
// 
TEST_FUNCTION(pal_mdns_sdbrowser_create__success)
{
    static const pal_sdclient_t* k_client_valid;
    static const pal_sd_result_cb_t k_cb_valid;
    static const void* k_context_valid;
    static const pal_sdbrowser_t** k_browser_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = pal_sdbrowser_create(k_client_valid, k_cb_valid, k_context_valid, k_browser_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test pal_sdbrowser_create passing as client argument an invalid pal_sdclient_t* value 
// 
TEST_FUNCTION(pal_mdns_sdbrowser_create__arg_client_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_sdbrowser_create();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_sdbrowser_create passing as cb argument an invalid pal_sd_result_cb_t value 
// 
TEST_FUNCTION(pal_mdns_sdbrowser_create__arg_cb_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_sdbrowser_create();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_sdbrowser_create passing as context argument an invalid void* value 
// 
TEST_FUNCTION(pal_mdns_sdbrowser_create__arg_context_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_sdbrowser_create();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_sdbrowser_create passing as browser argument an invalid pal_sdbrowser_t** value 
// 
TEST_FUNCTION(pal_mdns_sdbrowser_create__arg_browser_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_sdbrowser_create();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_sdbrowser_create unhappy path 
// 
TEST_FUNCTION(pal_mdns_sdbrowser_create__neg)
{
    static const pal_sdclient_t* k_client_valid;
    static const pal_sd_result_cb_t k_cb_valid;
    static const void* k_context_valid;
    static const pal_sdbrowser_t** k_browser_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_sdbrowser_create(k_client_valid, k_cb_valid, k_context_valid, k_browser_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // pal_sdbrowser_create 

#ifdef pal_sdbrowser_browse 
// 
// Test pal_sdbrowser_browse happy path 
// 
TEST_FUNCTION(pal_mdns_sdbrowser_browse__success)
{
    static const pal_sdbrowser_t* k_browser_valid;
    static const const char* k_service_name_valid;
    static const const char* k_service_type_valid;
    static const const char* k_domain_valid;
    static const int32_t k_itf_index_valid;
    int32_t result;

    // arrange 
    // ... 

    // act 
    result = pal_sdbrowser_browse(k_browser_valid, k_service_name_valid, k_service_type_valid, k_domain_valid, k_itf_index_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ... 
}

// 
// Test pal_sdbrowser_browse passing as browser argument an invalid pal_sdbrowser_t* value 
// 
TEST_FUNCTION(pal_mdns_sdbrowser_browse__arg_browser_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_sdbrowser_browse();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_sdbrowser_browse passing as service_name argument an invalid const char* value 
// 
TEST_FUNCTION(pal_mdns_sdbrowser_browse__arg_service_name_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_sdbrowser_browse();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_sdbrowser_browse passing as service_type argument an invalid const char* value 
// 
TEST_FUNCTION(pal_mdns_sdbrowser_browse__arg_service_type_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_sdbrowser_browse();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_sdbrowser_browse passing as domain argument an invalid const char* value 
// 
TEST_FUNCTION(pal_mdns_sdbrowser_browse__arg_domain_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_sdbrowser_browse();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_sdbrowser_browse passing as itf_index argument an invalid int32_t value 
// 
TEST_FUNCTION(pal_mdns_sdbrowser_browse__arg_itf_index_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_sdbrowser_browse();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_sdbrowser_browse unhappy path 
// 
TEST_FUNCTION(pal_mdns_sdbrowser_browse__neg)
{
    static const pal_sdbrowser_t* k_browser_valid;
    static const const char* k_service_name_valid;
    static const const char* k_service_type_valid;
    static const const char* k_domain_valid;
    static const int32_t k_itf_index_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_sdbrowser_browse(k_browser_valid, k_service_name_valid, k_service_type_valid, k_domain_valid, k_itf_index_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // pal_sdbrowser_browse 

#ifdef pal_sdbrowser_free 
// 
// Test pal_sdbrowser_free happy path 
// 
TEST_FUNCTION(pal_mdns_sdbrowser_free__success)
{
    static const pal_sdbrowser_t* k_browser_valid;
    void result;

    // arrange 
    // ... 

    // act 
    result = pal_sdbrowser_free(k_browser_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ... 
}

// 
// Test pal_sdbrowser_free passing as browser argument an invalid pal_sdbrowser_t* value 
// 
TEST_FUNCTION(pal_mdns_sdbrowser_free__arg_browser_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_sdbrowser_free();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_sdbrowser_free unhappy path 
// 
TEST_FUNCTION(pal_mdns_sdbrowser_free__neg)
{
    static const pal_sdbrowser_t* k_browser_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_sdbrowser_free(k_browser_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // pal_sdbrowser_free 

#ifdef pal_sdclient_free 
// 
// Test pal_sdclient_free happy path 
// 
TEST_FUNCTION(pal_mdns_sdclient_release__success)
{
    static const pal_sdclient_t* k_client_valid;
    void result;

    // arrange 
    // ... 

    // act 
    result = pal_sdclient_free(k_client_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ... 
}

// 
// Test pal_sdclient_free passing as client argument an invalid pal_sdclient_t* value 
// 
TEST_FUNCTION(pal_mdns_sdclient_release__arg_client_invalid)
{
    // ... 
    int32_t result;

    // arrange 
    // ... 

    // act 
    handle = pal_sdclient_free();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ... 
}

// 
// Test pal_sdclient_free unhappy path 
// 
TEST_FUNCTION(pal_mdns_sdclient_release__neg)
{
    static const pal_sdclient_t* k_client_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_sdclient_free(k_client_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // pal_sdclient_free 

#ifdef pal_sd_deinit 
// 
// Test pal_sd_deinit happy path 
// 
TEST_FUNCTION(pal_mdns_sd_deinit__success)
{
    void result;

    // arrange 
    // ... 

    // act 
    result = pal_sd_deinit();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ... 
}

// 
// Test pal_sd_deinit unhappy path 
// 
TEST_FUNCTION(pal_mdns_sd_deinit__neg)
{
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ... 

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_sd_deinit();

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // pal_sd_deinit 

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

