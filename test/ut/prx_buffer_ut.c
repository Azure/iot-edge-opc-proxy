// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST prx_buffer
#include "util_ut.h"

//
// 1. Required mocks
//
#include "azure_c_shared_utility/strings.h"
#include "util_misc.h"

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


#ifdef prx_buffer_factory_get_available //
// Test prx_buffer_factory_get_available happy path
//
TEST_FUNCTION(prx_buffer_factory_get_available__success)
{
    static const prx_buffer_factory_t* k_buffer_factory_valid;
    size_t result;

    // arrange
    // ...

    // act
    result = prx_buffer_factory_get_available(k_buffer_factory_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(size_t, er_ok, result);
    // ...
}

//
// Test prx_buffer_factory_get_available passing as buffer_factory argument an invalid prx_buffer_factory_t* value
//
TEST_FUNCTION(prx_buffer_factory_get_available__arg_buffer_factory_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_buffer_factory_get_available();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_buffer_factory_get_available unhappy path
//
TEST_FUNCTION(prx_buffer_factory_get_available__neg)
{
    static const prx_buffer_factory_t* k_buffer_factory_valid;
    size_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_buffer_factory_get_available(k_buffer_factory_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(size_t, result, er_ok);
}

#endif // prx_buffer_factory_get_available

#ifdef prx_buffer_alloc //
// Test prx_buffer_alloc happy path
//
TEST_FUNCTION(prx_buffer_alloc__success)
{
    static const prx_buffer_factory_t* k_buffer_factory_valid;
    static const void* k_original_valid;
    void* result;

    // arrange
    // ...

    // act
    result = prx_buffer_alloc(k_buffer_factory_valid, k_original_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void*, er_ok, result);
    // ...
}

//
// Test prx_buffer_alloc passing as buffer_factory argument an invalid prx_buffer_factory_t* value
//
TEST_FUNCTION(prx_buffer_alloc__arg_buffer_factory_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_buffer_alloc();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_buffer_alloc passing as original argument an invalid void* value
//
TEST_FUNCTION(prx_buffer_alloc__arg_original_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_buffer_alloc();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_buffer_alloc unhappy path
//
TEST_FUNCTION(prx_buffer_alloc__neg)
{
    static const prx_buffer_factory_t* k_buffer_factory_valid;
    static const void* k_original_valid;
    void* result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_buffer_alloc(k_buffer_factory_valid, k_original_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void*, result, er_ok);
}

#endif // prx_buffer_alloc

#ifdef prx_buffer_new //
// Test prx_buffer_new happy path
//
TEST_FUNCTION(prx_buffer_new__success)
{
    static const prx_buffer_factory_t* k_buffer_factory_valid;
    static const size_t k_length_valid;
    void* result;

    // arrange
    // ...

    // act
    result = prx_buffer_new(k_buffer_factory_valid, k_length_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void*, er_ok, result);
    // ...
}

//
// Test prx_buffer_new passing as buffer_factory argument an invalid prx_buffer_factory_t* value
//
TEST_FUNCTION(prx_buffer_new__arg_buffer_factory_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_buffer_new();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_buffer_new passing as length argument an invalid size_t value
//
TEST_FUNCTION(prx_buffer_new__arg_length_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_buffer_new();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_buffer_new unhappy path
//
TEST_FUNCTION(prx_buffer_new__neg)
{
    static const prx_buffer_factory_t* k_buffer_factory_valid;
    static const size_t k_length_valid;
    void* result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_buffer_new(k_buffer_factory_valid, k_length_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void*, result, er_ok);
}

#endif // prx_buffer_new;

#ifdef prx_buffer_get_size //
// Test prx_buffer_get_size happy path
//
TEST_FUNCTION(prx_buffer_get_size__success)
{
    static const prx_buffer_factory_t* k_buffer_factory_valid;
    static const void* k_buffer_valid;
    size_t result;

    // arrange
    // ...

    // act
    result = prx_buffer_get_size(k_buffer_factory_valid, k_buffer_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(size_t, er_ok, result);
    // ...
}

//
// Test prx_buffer_get_size passing as buffer_factory argument an invalid prx_buffer_factory_t* value
//
TEST_FUNCTION(prx_buffer_get_size__arg_buffer_factory_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_buffer_get_size();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_buffer_get_size passing as buffer argument an invalid void* value
//
TEST_FUNCTION(prx_buffer_get_size__arg_buffer_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_buffer_get_size();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_buffer_get_size unhappy path
//
TEST_FUNCTION(prx_buffer_get_size__neg)
{
    static const prx_buffer_factory_t* k_buffer_factory_valid;
    static const void* k_buffer_valid;
    size_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_buffer_get_size(k_buffer_factory_valid, k_buffer_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(size_t, result, er_ok);
}

#endif // prx_buffer_get_size

#ifdef prx_buffer_set_size //
// Test prx_buffer_set_size happy path
//
TEST_FUNCTION(prx_buffer_set_size__success)
{
    static const prx_buffer_factory_t* k_buffer_factory_valid;
    static const void** k_buffer_valid;
    static const size_t k_length_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = prx_buffer_set_size(k_buffer_factory_valid, k_buffer_valid, k_length_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test prx_buffer_set_size passing as buffer_factory argument an invalid prx_buffer_factory_t* value
//
TEST_FUNCTION(prx_buffer_set_size__arg_buffer_factory_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_buffer_set_size();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_buffer_set_size passing as buffer argument an invalid void** value
//
TEST_FUNCTION(prx_buffer_set_size__arg_buffer_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_buffer_set_size();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_buffer_set_size passing as length argument an invalid size_t value
//
TEST_FUNCTION(prx_buffer_set_size__arg_length_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_buffer_set_size();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_buffer_set_size unhappy path
//
TEST_FUNCTION(prx_buffer_set_size__neg)
{
    static const prx_buffer_factory_t* k_buffer_factory_valid;
    static const void** k_buffer_valid;
    static const size_t k_length_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_buffer_set_size(k_buffer_factory_valid, k_buffer_valid, k_length_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_buffer_set_size

#ifdef prx_buffer_release //
// Test prx_buffer_release happy path
//
TEST_FUNCTION(prx_buffer_release__success)
{
    static const prx_buffer_factory_t* k_buffer_factory_valid;
    static const void* k_buffer_valid;
    void result;

    // arrange
    // ...

    // act
    result = prx_buffer_release(k_buffer_factory_valid, k_buffer_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ...
}

//
// Test prx_buffer_release passing as buffer_factory argument an invalid prx_buffer_factory_t* value
//
TEST_FUNCTION(prx_buffer_release__arg_buffer_factory_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_buffer_release();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_buffer_release passing as buffer argument an invalid void* value
//
TEST_FUNCTION(prx_buffer_release__arg_buffer_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_buffer_release();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_buffer_release unhappy path
//
TEST_FUNCTION(prx_buffer_release__neg)
{
    static const prx_buffer_factory_t* k_buffer_factory_valid;
    static const void* k_buffer_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_buffer_release(k_buffer_factory_valid, k_buffer_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // prx_buffer_release

#ifdef prx_buffer_factory_free //
// Test prx_buffer_factory_free happy path
//
TEST_FUNCTION(prx_buffer_factory_free__success)
{
    static const prx_buffer_factory_t* k_buffer_factory_valid;
    void result;

    // arrange
    // ...

    // act
    result = prx_buffer_factory_free(k_buffer_factory_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ...
}

//
// Test prx_buffer_factory_free passing as buffer_factory argument an invalid prx_buffer_factory_t* value
//
TEST_FUNCTION(prx_buffer_factory_free__arg_buffer_factory_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_buffer_factory_free();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_buffer_factory_free unhappy path
//
TEST_FUNCTION(prx_buffer_factory_free__neg)
{
    static const prx_buffer_factory_t* k_buffer_factory_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_buffer_factory_free(k_buffer_factory_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // prx_buffer_factory_free

#ifdef prx_dynamic_pool_create //
// Test prx_dynamic_pool_create happy path
//
TEST_FUNCTION(io_dynamic_pool_create__success)
{
    static const const char* k_name_valid;
    static const size_t k_initial_item_size_valid;
    static const prx_pool_config_t* k_config_valid;
    static const prx_buffer_factory_t** k_pool_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = prx_dynamic_pool_create(k_name_valid, k_initial_item_size_valid, k_config_valid, k_pool_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test prx_dynamic_pool_create passing as name argument an invalid const char* value
//
TEST_FUNCTION(io_dynamic_pool_create__arg_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_dynamic_pool_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_dynamic_pool_create passing as initial_item_size argument an invalid size_t value
//
TEST_FUNCTION(io_dynamic_pool_create__arg_initial_item_size_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_dynamic_pool_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_dynamic_pool_create passing as config argument an invalid prx_pool_config_t* value
//
TEST_FUNCTION(io_dynamic_pool_create__arg_config_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_dynamic_pool_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_dynamic_pool_create passing as pool argument an invalid prx_buffer_factory_t** value
//
TEST_FUNCTION(io_dynamic_pool_create__arg_pool_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_dynamic_pool_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_dynamic_pool_create unhappy path
//
TEST_FUNCTION(io_dynamic_pool_create__neg)
{
    static const const char* k_name_valid;
    static const size_t k_initial_item_size_valid;
    static const prx_pool_config_t* k_config_valid;
    static const prx_buffer_factory_t** k_pool_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_dynamic_pool_create(k_name_valid, k_initial_item_size_valid, k_config_valid, k_pool_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_dynamic_pool_create;

#ifdef prx_fixed_pool_create //
// Test prx_fixed_pool_create happy path
//
TEST_FUNCTION(prx_fixed_pool_create__success)
{
    static const const char* k_name_valid;
    static const size_t k_fixed_item_size_valid;
    static const prx_pool_config_t* k_config_valid;
    static const prx_buffer_factory_t** k_pool_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = prx_fixed_pool_create(k_name_valid, k_fixed_item_size_valid, k_config_valid, k_pool_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test prx_fixed_pool_create passing as name argument an invalid const char* value
//
TEST_FUNCTION(prx_fixed_pool_create__arg_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_fixed_pool_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_fixed_pool_create passing as fixed_item_size argument an invalid size_t value
//
TEST_FUNCTION(prx_fixed_pool_create__arg_fixed_item_size_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_fixed_pool_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_fixed_pool_create passing as config argument an invalid prx_pool_config_t* value
//
TEST_FUNCTION(prx_fixed_pool_create__arg_config_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_fixed_pool_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_fixed_pool_create passing as pool argument an invalid prx_buffer_factory_t** value
//
TEST_FUNCTION(prx_fixed_pool_create__arg_pool_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_fixed_pool_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_fixed_pool_create unhappy path
//
TEST_FUNCTION(prx_fixed_pool_create__neg)
{
    static const const char* k_name_valid;
    static const size_t k_fixed_item_size_valid;
    static const prx_pool_config_t* k_config_valid;
    static const prx_buffer_factory_t** k_pool_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_fixed_pool_create(k_name_valid, k_fixed_item_size_valid, k_config_valid, k_pool_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_fixed_pool_create;

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

