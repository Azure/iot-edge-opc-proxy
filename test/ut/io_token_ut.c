// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST io_token
#include "util_ut.h"

//
// 1. Required mocks
//
#include "util_misc.h"
#include "azure_c_shared_utility/strings.h"

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

#ifdef io_token_provider_clone //
// Test io_token_provider_clone happy path
//
TEST_FUNCTION(io_token_provider_clone__success)
{
    static const io_token_provider_t* k_provider_valid;
    static const io_token_provider_t** k_clone_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_token_provider_clone(k_provider_valid, k_clone_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_token_provider_clone passing as provider argument an invalid io_token_provider_t* value
//
TEST_FUNCTION(io_token_provider_clone__arg_provider_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_token_provider_clone();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_token_provider_clone passing as clone argument an invalid io_token_provider_t** value
//
TEST_FUNCTION(io_token_provider_clone__arg_clone_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_token_provider_clone();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_token_provider_clone unhappy path
//
TEST_FUNCTION(io_token_provider_clone__neg)
{
    static const io_token_provider_t* k_provider_valid;
    static const io_token_provider_t** k_clone_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_token_provider_clone(k_provider_valid, k_clone_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_token_provider_clone

#ifdef io_token_provider_is_equivalent //
// Test io_token_provider_is_equivalent happy path
//
TEST_FUNCTION(io_token_provider_is_equivalent__success)
{
    static const io_token_provider_t* k_that_valid;
    static const io_token_provider_t* k_other_valid;
    bool result;

    // arrange
    // ...

    // act
    result = io_token_provider_is_equivalent(k_that_valid, k_other_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(bool, er_ok, result);
    // ...
}

//
// Test io_token_provider_is_equivalent passing as that argument an invalid io_token_provider_t* value
//
TEST_FUNCTION(io_token_provider_is_equivalent__arg_that_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_token_provider_is_equivalent();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_token_provider_is_equivalent passing as other argument an invalid io_token_provider_t* value
//
TEST_FUNCTION(io_token_provider_is_equivalent__arg_other_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_token_provider_is_equivalent();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_token_provider_is_equivalent unhappy path
//
TEST_FUNCTION(io_token_provider_is_equivalent__neg)
{
    static const io_token_provider_t* k_that_valid;
    static const io_token_provider_t* k_other_valid;
    bool result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_token_provider_is_equivalent(k_that_valid, k_other_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(bool, result, er_ok);
}

#endif // io_token_provider_is_equivalent;

#ifdef io_token_provider_get_property //
// Test io_token_provider_get_property happy path
//
TEST_FUNCTION(io_token_provider_get_property__success)
{
    static const io_token_provider_t* k_provider_valid;
    static const io_token_property_id_t k_id_valid;
    const char* result;

    // arrange
    // ...

    // act
    result = io_token_provider_get_property(k_provider_valid, k_id_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(const char*, er_ok, result);
    // ...
}

//
// Test io_token_provider_get_property passing as provider argument an invalid io_token_provider_t* value
//
TEST_FUNCTION(io_token_provider_get_property__arg_provider_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_token_provider_get_property();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_token_provider_get_property passing as id argument an invalid io_token_property_id_t value
//
TEST_FUNCTION(io_token_provider_get_property__arg_id_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_token_provider_get_property();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_token_provider_get_property unhappy path
//
TEST_FUNCTION(io_token_provider_get_property__neg)
{
    static const io_token_provider_t* k_provider_valid;
    static const io_token_property_id_t k_id_valid;
    const char* result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_token_provider_get_property(k_provider_valid, k_id_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(const char*, result, er_ok);
}

#endif // io_token_provider_get_property

#ifdef io_token_provider_new_token //
// Test io_token_provider_new_token happy path
//
TEST_FUNCTION(io_token_provider_new_token__success)
{
    static const io_token_provider_t* k_provider_valid;
    static const STRING_HANDLE* k_token_valid;
    static const ticks_t* k_expiry_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_token_provider_new_token(k_provider_valid, k_token_valid, k_expiry_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_token_provider_new_token passing as provider argument an invalid io_token_provider_t* value
//
TEST_FUNCTION(io_token_provider_new_token__arg_provider_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_token_provider_new_token();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_token_provider_new_token passing as token argument an invalid STRING_HANDLE* value
//
TEST_FUNCTION(io_token_provider_new_token__arg_token_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_token_provider_new_token();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_token_provider_new_token passing as expiry argument an invalid ticks_t* value
//
TEST_FUNCTION(io_token_provider_new_token__arg_expiry_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_token_provider_new_token();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_token_provider_new_token unhappy path
//
TEST_FUNCTION(io_token_provider_new_token__neg)
{
    static const io_token_provider_t* k_provider_valid;
    static const STRING_HANDLE* k_token_valid;
    static const ticks_t* k_expiry_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_token_provider_new_token(k_provider_valid, k_token_valid, k_expiry_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_token_provider_new_token

#ifdef io_token_provider_release //
// Test io_token_provider_release happy path
//
TEST_FUNCTION(io_token_provider_release__success)
{
    static const io_token_provider_t* k_provider_valid;
    void result;

    // arrange
    // ...

    // act
    result = io_token_provider_release(k_provider_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ...
}

//
// Test io_token_provider_release passing as provider argument an invalid io_token_provider_t* value
//
TEST_FUNCTION(io_token_provider_release__arg_provider_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_token_provider_release();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_token_provider_release unhappy path
//
TEST_FUNCTION(io_token_provider_release__neg)
{
    static const io_token_provider_t* k_provider_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_token_provider_release(k_provider_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // io_token_provider_release

#ifdef io_passthru_token_provider_create //
// Test io_passthru_token_provider_create happy path
//
TEST_FUNCTION(io_passthru_token_provider_create__success)
{
    static const const char* k_shared_access_token_valid;
    static const io_token_provider_t** k_provider_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_passthru_token_provider_create(k_shared_access_token_valid, k_provider_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_passthru_token_provider_create passing as shared_access_token argument an invalid const char* value
//
TEST_FUNCTION(io_passthru_token_provider_create__arg_shared_access_token_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_passthru_token_provider_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_passthru_token_provider_create passing as provider argument an invalid io_token_provider_t** value
//
TEST_FUNCTION(io_passthru_token_provider_create__arg_provider_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_passthru_token_provider_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_passthru_token_provider_create unhappy path
//
TEST_FUNCTION(io_passthru_token_provider_create__neg)
{
    static const const char* k_shared_access_token_valid;
    static const io_token_provider_t** k_provider_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_passthru_token_provider_create(k_shared_access_token_valid, k_provider_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_passthru_token_provider_create;

#ifdef io_iothub_token_provider_create //
// Test io_iothub_token_provider_create happy path
//
TEST_FUNCTION(io_iothub_token_provider_create__success)
{
    static const const char* k_policy_valid;
    static const const char* k_shared_access_key_valid;
    static const const char* k_scope_valid;
    static const io_token_provider_t** k_provider_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_iothub_token_provider_create(k_policy_valid, k_shared_access_key_valid, k_scope_valid, k_provider_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_iothub_token_provider_create passing as policy argument an invalid const char* value
//
TEST_FUNCTION(io_iothub_token_provider_create__arg_policy_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_iothub_token_provider_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_iothub_token_provider_create passing as shared_access_key argument an invalid const char* value
//
TEST_FUNCTION(io_iothub_token_provider_create__arg_shared_access_key_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_iothub_token_provider_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_iothub_token_provider_create passing as scope argument an invalid const char* value
//
TEST_FUNCTION(io_iothub_token_provider_create__arg_scope_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_iothub_token_provider_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_iothub_token_provider_create passing as provider argument an invalid io_token_provider_t** value
//
TEST_FUNCTION(io_iothub_token_provider_create__arg_provider_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_iothub_token_provider_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_iothub_token_provider_create unhappy path
//
TEST_FUNCTION(io_iothub_token_provider_create__neg)
{
    static const const char* k_policy_valid;
    static const const char* k_shared_access_key_valid;
    static const const char* k_scope_valid;
    static const io_token_provider_t** k_provider_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_iothub_token_provider_create(k_policy_valid, k_shared_access_key_valid, k_scope_valid, k_provider_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_iothub_token_provider_create;

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

