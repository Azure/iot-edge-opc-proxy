// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST prx_ns
#include "util_ut.h"

//
// 1. Required mocks
//
#include "io_ref.h"
#include "io_cs.h"

#include "parson.h"

MOCKABLE_FUNCTION(, JSON_Value*, json_parse_string, const char*, string);
MOCKABLE_FUNCTION(, size_t, json_serialization_size, const JSON_Value*, value);
MOCKABLE_FUNCTION(, JSON_Status, json_serialize_to_buffer, const JSON_Value*, value, char*, buf, size_t, buf_size_in_bytes);
MOCKABLE_FUNCTION(, const char*, json_object_get_string, const JSON_Object*, object, const char*, name);
MOCKABLE_FUNCTION(, JSON_Array*, json_object_get_array, const JSON_Object*, object, const char*, name);
MOCKABLE_FUNCTION(, const char*, json_object_dotget_string, const JSON_Object*, object, const char*, name);
MOCKABLE_FUNCTION(, double, json_object_dotget_number, const JSON_Object*, object, const char*, name);
MOCKABLE_FUNCTION(, JSON_Status, json_object_set_string, JSON_Object*, object, const char*, name, const char*, string);
MOCKABLE_FUNCTION(, JSON_Status, json_object_dotset_string, JSON_Object*, object, const char*, name, const char*, string);
MOCKABLE_FUNCTION(, JSON_Status, json_object_dotset_number, JSON_Object*, object, const char*, name, double, number);
MOCKABLE_FUNCTION(, JSON_Value*, json_array_get_value, const JSON_Array*, array, size_t, index);
MOCKABLE_FUNCTION(, size_t, json_array_get_count, const JSON_Array*, array);
MOCKABLE_FUNCTION(, JSON_Value*, json_value_init_object);
MOCKABLE_FUNCTION(, JSON_Value*, json_value_deep_copy, const JSON_Value*, value);
MOCKABLE_FUNCTION(, void, json_value_free, JSON_Value*, value);
MOCKABLE_FUNCTION(, JSON_Object*, json_value_get_object, const JSON_Value*, value);
MOCKABLE_FUNCTION(, JSON_Array*, json_value_get_array, const JSON_Value*, value);

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


#ifdef prx_ns_iot_hub_create_from_cs //
// Test prx_ns_iot_hub_create_from_cs happy path
//
TEST_FUNCTION(prx_ns_iot_hub_create_from_cs__success)
{
    static const io_cs_t* k_hub_cs_valid;
    static const prx_ns_t** k_created_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = prx_ns_iot_hub_create_from_cs(k_hub_cs_valid, k_created_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test prx_ns_iot_hub_create_from_cs passing as hub_cs argument an invalid io_cs_t* value
//
TEST_FUNCTION(prx_ns_iot_hub_create_from_cs__arg_hub_cs_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_iot_hub_create_from_cs();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_iot_hub_create_from_cs passing as created argument an invalid prx_ns_t** value
//
TEST_FUNCTION(prx_ns_iot_hub_create_from_cs__arg_created_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_iot_hub_create_from_cs();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_iot_hub_create_from_cs unhappy path
//
TEST_FUNCTION(prx_ns_iot_hub_create_from_cs__neg)
{
    static const io_cs_t* k_hub_cs_valid;
    static const prx_ns_t** k_created_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_ns_iot_hub_create_from_cs(k_hub_cs_valid, k_created_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_ns_iot_hub_create_from_cs;

#ifdef prx_ns_iot_hub_create //
// Test prx_ns_iot_hub_create happy path
//
TEST_FUNCTION(prx_ns_iot_hub_create__success)
{
    static const const char* k_config_valid;
    static const prx_ns_t** k_created_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = prx_ns_iot_hub_create(k_config_valid, k_created_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test prx_ns_iot_hub_create passing as config argument an invalid const char* value
//
TEST_FUNCTION(prx_ns_iot_hub_create__arg_config_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_iot_hub_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_iot_hub_create passing as created argument an invalid prx_ns_t** value
//
TEST_FUNCTION(prx_ns_iot_hub_create__arg_created_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_iot_hub_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_iot_hub_create unhappy path
//
TEST_FUNCTION(prx_ns_iot_hub_create__neg)
{
    static const const char* k_config_valid;
    static const prx_ns_t** k_created_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_ns_iot_hub_create(k_config_valid, k_created_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_ns_iot_hub_create;

#ifdef prx_ns_generic_create //
// Test prx_ns_generic_create happy path
//
TEST_FUNCTION(prx_ns_generic_create__success)
{
    static const const char* k_file_name_valid;
    static const prx_ns_t** k_created_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = prx_ns_generic_create(k_file_name_valid, k_created_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test prx_ns_generic_create passing as file_name argument an invalid const char* value
//
TEST_FUNCTION(prx_ns_generic_create__arg_file_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_generic_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_generic_create passing as created argument an invalid prx_ns_t** value
//
TEST_FUNCTION(prx_ns_generic_create__arg_created_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_generic_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_generic_create unhappy path
//
TEST_FUNCTION(prx_ns_generic_create__neg)
{
    static const const char* k_file_name_valid;
    static const prx_ns_t** k_created_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_ns_generic_create(k_file_name_valid, k_created_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_ns_generic_create;

#ifdef prx_ns_get_entry_by_addr //
// Test prx_ns_get_entry_by_addr happy path
//
TEST_FUNCTION(prx_ns_get_entry_by_addr__success)
{
    static const prx_ns_t* k_ns_valid;
    static const io_ref_t* k_address_valid;
    static const prx_ns_entry_t** k_entry_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = prx_ns_get_entry_by_addr(k_ns_valid, k_address_valid, k_entry_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test prx_ns_get_entry_by_addr passing as ns argument an invalid prx_ns_t* value
//
TEST_FUNCTION(prx_ns_get_entry_by_addr__arg_ns_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_get_entry_by_addr();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_get_entry_by_addr passing as address argument an invalid io_ref_t* value
//
TEST_FUNCTION(prx_ns_get_entry_by_addr__arg_address_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_get_entry_by_addr();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_get_entry_by_addr passing as entry argument an invalid prx_ns_entry_t** value
//
TEST_FUNCTION(prx_ns_get_entry_by_addr__arg_entry_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_get_entry_by_addr();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_get_entry_by_addr unhappy path
//
TEST_FUNCTION(prx_ns_get_entry_by_addr__neg)
{
    static const prx_ns_t* k_ns_valid;
    static const io_ref_t* k_address_valid;
    static const prx_ns_entry_t** k_entry_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_ns_get_entry_by_addr(k_ns_valid, k_address_valid, k_entry_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_ns_get_entry_by_addr

#ifdef prx_ns_get_entry_by_name //
// Test prx_ns_get_entry_by_name happy path
//
TEST_FUNCTION(prx_ns_get_entry_by_name__success)
{
    static const prx_ns_t* k_ns_valid;
    static const const char* k_name_valid;
    static const prx_ns_result_t** k_results_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = prx_ns_get_entry_by_name(k_ns_valid, k_name_valid, k_results_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test prx_ns_get_entry_by_name passing as ns argument an invalid prx_ns_t* value
//
TEST_FUNCTION(prx_ns_get_entry_by_name__arg_ns_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_get_entry_by_name();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_get_entry_by_name passing as name argument an invalid const char* value
//
TEST_FUNCTION(prx_ns_get_entry_by_name__arg_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_get_entry_by_name();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_get_entry_by_name passing as results argument an invalid prx_ns_result_t** value
//
TEST_FUNCTION(prx_ns_get_entry_by_name__arg_results_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_get_entry_by_name();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_get_entry_by_name unhappy path
//
TEST_FUNCTION(prx_ns_get_entry_by_name__neg)
{
    static const prx_ns_t* k_ns_valid;
    static const const char* k_name_valid;
    static const prx_ns_result_t** k_results_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_ns_get_entry_by_name(k_ns_valid, k_name_valid, k_results_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_ns_get_entry_by_name

#ifdef prx_ns_get_entry_by_type //
// Test prx_ns_get_entry_by_type happy path
//
TEST_FUNCTION(prx_ns_get_entry_by_type__success)
{
    static const prx_ns_t* k_ns_valid;
    static const uint32_t k_type_valid;
    static const prx_ns_result_t** k_results_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = prx_ns_get_entry_by_type(k_ns_valid, k_type_valid, k_results_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test prx_ns_get_entry_by_type passing as ns argument an invalid prx_ns_t* value
//
TEST_FUNCTION(prx_ns_get_entry_by_type__arg_ns_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_get_entry_by_type();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_get_entry_by_type passing as type argument an invalid uint32_t value
//
TEST_FUNCTION(prx_ns_get_entry_by_type__arg_type_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_get_entry_by_type();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_get_entry_by_type passing as results argument an invalid prx_ns_result_t** value
//
TEST_FUNCTION(prx_ns_get_entry_by_type__arg_results_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_get_entry_by_type();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_get_entry_by_type unhappy path
//
TEST_FUNCTION(prx_ns_get_entry_by_type__neg)
{
    static const prx_ns_t* k_ns_valid;
    static const uint32_t k_type_valid;
    static const prx_ns_result_t** k_results_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_ns_get_entry_by_type(k_ns_valid, k_type_valid, k_results_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_ns_get_entry_by_type

#ifdef prx_ns_create_entry //
// Test prx_ns_create_entry happy path
//
TEST_FUNCTION(prx_ns_create_entry__success)
{
    static const prx_ns_t* k_ns_valid;
    static const prx_ns_entry_t* k_entry_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = prx_ns_create_entry(k_ns_valid, k_entry_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test prx_ns_create_entry passing as ns argument an invalid prx_ns_t* value
//
TEST_FUNCTION(prx_ns_create_entry__arg_ns_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_create_entry();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_create_entry passing as entry argument an invalid prx_ns_entry_t* value
//
TEST_FUNCTION(prx_ns_create_entry__arg_entry_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_create_entry();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_create_entry unhappy path
//
TEST_FUNCTION(prx_ns_create_entry__neg)
{
    static const prx_ns_t* k_ns_valid;
    static const prx_ns_entry_t* k_entry_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_ns_create_entry(k_ns_valid, k_entry_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_ns_create_entry

#ifdef prx_ns_update_entry //
// Test prx_ns_update_entry happy path
//
TEST_FUNCTION(prx_ns_update_entry__success)
{
    static const prx_ns_t* k_ns_valid;
    static const prx_ns_entry_t* k_entry_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = prx_ns_update_entry(k_ns_valid, k_entry_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test prx_ns_update_entry passing as ns argument an invalid prx_ns_t* value
//
TEST_FUNCTION(prx_ns_update_entry__arg_ns_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_update_entry();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_update_entry passing as entry argument an invalid prx_ns_entry_t* value
//
TEST_FUNCTION(prx_ns_update_entry__arg_entry_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_update_entry();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_update_entry unhappy path
//
TEST_FUNCTION(prx_ns_update_entry__neg)
{
    static const prx_ns_t* k_ns_valid;
    static const prx_ns_entry_t* k_entry_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_ns_update_entry(k_ns_valid, k_entry_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_ns_update_entry

#ifdef prx_ns_remove_entry //
// Test prx_ns_remove_entry happy path
//
TEST_FUNCTION(prx_ns_remove_entry__success)
{
    static const prx_ns_t* k_ns_valid;
    static const prx_ns_entry_t* k_entry_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = prx_ns_remove_entry(k_ns_valid, k_entry_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test prx_ns_remove_entry passing as ns argument an invalid prx_ns_t* value
//
TEST_FUNCTION(prx_ns_remove_entry__arg_ns_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_remove_entry();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_remove_entry passing as entry argument an invalid prx_ns_entry_t* value
//
TEST_FUNCTION(prx_ns_remove_entry__arg_entry_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_remove_entry();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_remove_entry unhappy path
//
TEST_FUNCTION(prx_ns_remove_entry__neg)
{
    static const prx_ns_t* k_ns_valid;
    static const prx_ns_entry_t* k_entry_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_ns_remove_entry(k_ns_valid, k_entry_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_ns_remove_entry

#ifdef prx_ns_close //
// Test prx_ns_close happy path
//
TEST_FUNCTION(prx_ns_close__success)
{
    static const prx_ns_t* k_ns_valid;
    void result;

    // arrange
    // ...

    // act
    result = prx_ns_close(k_ns_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ...
}

//
// Test prx_ns_close passing as ns argument an invalid prx_ns_t* value
//
TEST_FUNCTION(prx_ns_close__arg_ns_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_close();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_close unhappy path
//
TEST_FUNCTION(prx_ns_close__neg)
{
    static const prx_ns_t* k_ns_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_ns_close(k_ns_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // prx_ns_close

#ifdef prx_ns_result_size //
// Test prx_ns_result_size happy path
//
TEST_FUNCTION(prx_ns_result_size__success)
{
    static const prx_ns_result_t* k_results_valid;
    size_t result;

    // arrange
    // ...

    // act
    result = prx_ns_result_size(k_results_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(size_t, er_ok, result);
    // ...
}

//
// Test prx_ns_result_size passing as results argument an invalid prx_ns_result_t* value
//
TEST_FUNCTION(prx_ns_result_size__arg_results_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_result_size();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_result_size unhappy path
//
TEST_FUNCTION(prx_ns_result_size__neg)
{
    static const prx_ns_result_t* k_results_valid;
    size_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_ns_result_size(k_results_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(size_t, result, er_ok);
}

#endif // prx_ns_result_size

#ifdef prx_ns_result_pop //
// Test prx_ns_result_pop happy path
//
TEST_FUNCTION(prx_ns_result_pop__success)
{
    static const prx_ns_result_t* k_results_valid;
    prx_ns_entry_t* result;

    // arrange
    // ...

    // act
    result = prx_ns_result_pop(k_results_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(prx_ns_entry_t*, er_ok, result);
    // ...
}

//
// Test prx_ns_result_pop passing as results argument an invalid prx_ns_result_t* value
//
TEST_FUNCTION(prx_ns_result_pop__arg_results_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_result_pop();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_result_pop unhappy path
//
TEST_FUNCTION(prx_ns_result_pop__neg)
{
    static const prx_ns_result_t* k_results_valid;
    prx_ns_entry_t* result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_ns_result_pop(k_results_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(prx_ns_entry_t*, result, er_ok);
}

#endif // prx_ns_result_pop

#ifdef prx_ns_result_release //
// Test prx_ns_result_release happy path
//
TEST_FUNCTION(prx_ns_result_release__success)
{
    static const prx_ns_result_t* k_results_valid;
    void result;

    // arrange
    // ...

    // act
    result = prx_ns_result_release(k_results_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ...
}

//
// Test prx_ns_result_release passing as results argument an invalid prx_ns_result_t* value
//
TEST_FUNCTION(prx_ns_result_release__arg_results_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_result_release();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_result_release unhappy path
//
TEST_FUNCTION(prx_ns_result_release__neg)
{
    static const prx_ns_result_t* k_results_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_ns_result_release(k_results_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // prx_ns_result_release

#ifdef prx_ns_entry_clone

//
//Test prx_ns_entry_clone happy path
//
TEST_FUNCTION(prx_ns_entry_clone__success)
{
    static const prx_ns_entry_t* k_entry_valid;
    static const prx_ns_entry_t** k_clone_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = prx_ns_entry_clone(k_entry_valid, k_clone_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test prx_ns_entry_clone passing as entry argument an invalid prx_ns_entry_t* value
//
TEST_FUNCTION(prx_ns_entry_clone__arg_entry_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_entry_clone();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_entry_clone passing as clone argument an invalid prx_ns_entry_t** value
//
TEST_FUNCTION(prx_ns_entry_clone__arg_clone_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_entry_clone();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_entry_clone unhappy path
//
TEST_FUNCTION(prx_ns_entry_clone__neg)
{
    static const prx_ns_entry_t* k_entry_valid;
    static const prx_ns_entry_t** k_clone_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_ns_entry_clone(k_entry_valid, k_clone_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_ns_entry_clone

#ifdef prx_ns_entry_get_cs
//
//Test prx_ns_entry_get_cs happy path
//
TEST_FUNCTION(prx_ns_entry_get_cs__success)
{
    static const prx_ns_entry_t* k_entry_valid;
    static const io_cs_t** k_cs_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = prx_ns_entry_get_cs(k_entry_valid, k_cs_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test prx_ns_entry_get_cs passing as entry argument an invalid prx_ns_entry_t* value
//
TEST_FUNCTION(prx_ns_entry_get_cs__arg_entry_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_entry_get_cs();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_entry_get_cs passing as cs argument an invalid io_cs_t** value
//
TEST_FUNCTION(prx_ns_entry_get_cs__arg_cs_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_entry_get_cs();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_entry_get_cs unhappy path
//
TEST_FUNCTION(prx_ns_entry_get_cs__neg)
{
    static const prx_ns_entry_t* k_entry_valid;
    static const io_cs_t** k_cs_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_ns_entry_get_cs(k_entry_valid, k_cs_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}
#endif // prx_ns_entry_get_cs

#ifdef prx_ns_entry_get_id

//
//Test prx_ns_entry_get_id happy path
//
TEST_FUNCTION(prx_ns_entry_get_id__success)
{
    static const prx_ns_entry_t* k_entry_valid;
    const char* result;

    // arrange
    // ...

    // act
    result = prx_ns_entry_get_id(k_entry_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(const char*, er_ok, result);
    // ...
}

//
// Test prx_ns_entry_get_id passing as entry argument an invalid prx_ns_entry_t* value
//
TEST_FUNCTION(prx_ns_entry_get_id__arg_entry_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_entry_get_id();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_entry_get_id unhappy path
//
TEST_FUNCTION(prx_ns_entry_get_id__neg)
{
    static const prx_ns_entry_t* k_entry_valid;
    const char* result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_ns_entry_get_id(k_entry_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(const char*, result, er_ok);
}

#endif

#ifdef prx_ns_entry_get_name

//
//Test prx_ns_entry_get_name happy path
//
TEST_FUNCTION(prx_ns_entry_get_name__success)
{
    static const prx_ns_entry_t* k_entry_valid;
    const char* result;

    // arrange
    // ...

    // act
    result = prx_ns_entry_get_name(k_entry_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(const char*, er_ok, result);
    // ...
}

//
// Test prx_ns_entry_get_name passing as entry argument an invalid prx_ns_entry_t* value
//
TEST_FUNCTION(prx_ns_entry_get_name__arg_entry_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_entry_get_name();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_entry_get_name unhappy path
//
TEST_FUNCTION(prx_ns_entry_get_name__neg)
{
    static const prx_ns_entry_t* k_entry_valid;
    const char* result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_ns_entry_get_name(k_entry_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(const char*, result, er_ok);
}

#endif

#ifdef prx_ns_entry_get_type

//
//Test prx_ns_entry_get_type happy path
//
TEST_FUNCTION(prx_ns_entry_get_type__success)
{
    static const prx_ns_entry_t* k_entry_valid;
    uint32_t result;

    // arrange
    // ...

    // act
    result = prx_ns_entry_get_type(k_entry_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(uint32_t, er_ok, result);
    // ...
}

//
// Test prx_ns_entry_get_type passing as entry argument an invalid prx_ns_entry_t* value
//
TEST_FUNCTION(prx_ns_entry_get_type__arg_entry_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_entry_get_type();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_entry_get_type unhappy path
//
TEST_FUNCTION(prx_ns_entry_get_type__neg)
{
    static const prx_ns_entry_t* k_entry_valid;
    uint32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_ns_entry_get_type(k_entry_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(uint32_t, result, er_ok);
}
#endif


#ifdef prx_ns_entry_get_index

//
//Test prx_ns_entry_get_index happy path
//
TEST_FUNCTION(prx_ns_entry_get_index__success)
{
    static const prx_ns_entry_t* k_entry_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = prx_ns_entry_get_index(k_entry_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test prx_ns_entry_get_index passing as entry argument an invalid prx_ns_entry_t* value
//
TEST_FUNCTION(prx_ns_entry_get_index__arg_entry_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_entry_get_index();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_entry_get_index unhappy path
//
TEST_FUNCTION(prx_ns_entry_get_index__neg)
{
    static const prx_ns_entry_t* k_entry_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_ns_entry_get_index(k_entry_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}
#endif



#ifdef prx_ns_entry_get_addr

//
//Test prx_ns_entry_get_addr happy path
//
TEST_FUNCTION(prx_ns_entry_get_addr__success)
{
    static const prx_ns_entry_t* k_entry_valid;
    static const io_ref_t* k_address_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = prx_ns_entry_get_addr(k_entry_valid, k_address_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test prx_ns_entry_get_addr passing as entry argument an invalid prx_ns_entry_t* value
//
TEST_FUNCTION(prx_ns_entry_get_addr__arg_entry_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_entry_get_addr();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_entry_get_addr passing as address argument an invalid io_ref_t* value
//
TEST_FUNCTION(prx_ns_entry_get_addr__arg_address_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_entry_get_addr();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_entry_get_addr unhappy path
//
TEST_FUNCTION(prx_ns_entry_get_addr__neg)
{
    static const prx_ns_entry_t* k_entry_valid;
    static const io_ref_t* k_address_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_ns_entry_get_addr(k_entry_valid, k_address_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}
#endif



#ifdef prx_ns_entry_get_routes

//
//Test prx_ns_entry_get_routes happy path
//
TEST_FUNCTION(prx_ns_entry_get_routes__success)
{
    static const prx_ns_entry_t* k_entry_valid;
    static const prx_ns_result_t** k_results_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = prx_ns_entry_get_routes(k_entry_valid, k_results_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test prx_ns_entry_get_routes passing as entry argument an invalid prx_ns_entry_t* value
//
TEST_FUNCTION(prx_ns_entry_get_routes__arg_entry_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_entry_get_routes();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_entry_get_routes passing as results argument an invalid prx_ns_result_t** value
//
TEST_FUNCTION(prx_ns_entry_get_routes__arg_results_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_entry_get_routes();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_entry_get_routes unhappy path
//
TEST_FUNCTION(prx_ns_entry_get_routes__neg)
{
    static const prx_ns_entry_t* k_entry_valid;
    static const prx_ns_result_t** k_results_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_ns_entry_get_routes(k_entry_valid, k_results_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}
#endif


#ifdef prx_ns_entry_add_route

//
//Test prx_ns_entry_add_route happy path
//
TEST_FUNCTION(prx_ns_entry_add_route__success)
{
    static const prx_ns_entry_t* k_entry_valid;
    static const prx_ns_entry_t* k_proxy_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = prx_ns_entry_add_route(k_entry_valid, k_proxy_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test prx_ns_entry_add_route passing as entry argument an invalid prx_ns_entry_t* value
//
TEST_FUNCTION(prx_ns_entry_add_route__arg_entry_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_entry_add_route();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_entry_add_route passing as proxy argument an invalid prx_ns_entry_t* value
//
TEST_FUNCTION(prx_ns_entry_add_route__arg_proxy_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_entry_add_route();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_entry_add_route unhappy path
//
TEST_FUNCTION(prx_ns_entry_add_route__neg)
{
    static const prx_ns_entry_t* k_entry_valid;
    static const prx_ns_entry_t* k_proxy_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_ns_entry_add_route(k_entry_valid, k_proxy_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif

#ifdef prx_ns_entry_get_links

//
//Test prx_ns_entry_get_links happy path
//
TEST_FUNCTION(prx_ns_entry_get_links__success)
{
    static const prx_ns_entry_t* k_entry_valid;
    static const prx_ns_result_t** k_results_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = prx_ns_entry_get_links(k_entry_valid, k_results_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test prx_ns_entry_get_links passing as entry argument an invalid prx_ns_entry_t* value
//
TEST_FUNCTION(prx_ns_entry_get_links__arg_entry_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_entry_get_links();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_entry_get_links passing as results argument an invalid prx_ns_result_t** value
//
TEST_FUNCTION(prx_ns_entry_get_links__arg_results_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_entry_get_links();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_entry_get_links unhappy path
//
TEST_FUNCTION(prx_ns_entry_get_links__neg)
{
    static const prx_ns_entry_t* k_entry_valid;
    static const prx_ns_result_t** k_results_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_ns_entry_get_links(k_entry_valid, k_results_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}
#endif

#ifdef prx_ns_entry_to_prx_socket_address

//
//Test prx_ns_entry_to_prx_socket_address happy path
//
TEST_FUNCTION(prx_ns_entry_to_prx_socket_address__success)
{
    static const prx_ns_entry_t* k_entry_valid;
    static const prx_address_family_t k_family_valid;
    static const prx_socket_address_t* k_socket_address_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = prx_ns_entry_to_prx_socket_address(k_entry_valid, k_family_valid, k_socket_address_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test prx_ns_entry_to_prx_socket_address passing as entry argument an invalid prx_ns_entry_t* value
//
TEST_FUNCTION(prx_ns_entry_to_prx_socket_address__arg_entry_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_entry_to_prx_socket_address();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_entry_to_prx_socket_address passing as family argument an invalid prx_address_family_t value
//
TEST_FUNCTION(prx_ns_entry_to_prx_socket_address__arg_family_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_entry_to_prx_socket_address();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_entry_to_prx_socket_address passing as socket_address argument an invalid prx_socket_address_t* value
//
TEST_FUNCTION(prx_ns_entry_to_prx_socket_address__arg_socket_address_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_entry_to_prx_socket_address();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_entry_to_prx_socket_address unhappy path
//
TEST_FUNCTION(prx_ns_entry_to_prx_socket_address__neg)
{
    static const prx_ns_entry_t* k_entry_valid;
    static const prx_address_family_t k_family_valid;
    static const prx_socket_address_t* k_socket_address_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_ns_entry_to_prx_socket_address(k_entry_valid, k_family_valid, k_socket_address_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_ns_entry_to_prx_socket_address;

#ifdef prx_ns_entry_create

//
//Test prx_ns_entry_create happy path
//
TEST_FUNCTION(prx_ns_entry_create__success)
{
    static const uint32_t k_type_valid;
    static const const char* k_id_valid;
    static const const char* k_name_valid;
    static const prx_ns_entry_t** k_entry_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = prx_ns_entry_create(k_type_valid, k_id_valid, k_name_valid, k_entry_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test prx_ns_entry_create passing as type argument an invalid uint32_t value
//
TEST_FUNCTION(prx_ns_entry_create__arg_type_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_entry_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_entry_create passing as id argument an invalid const char* value
//
TEST_FUNCTION(prx_ns_entry_create__arg_id_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_entry_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_entry_create passing as name argument an invalid const char* value
//
TEST_FUNCTION(prx_ns_entry_create__arg_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_entry_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_entry_create passing as entry argument an invalid prx_ns_entry_t** value
//
TEST_FUNCTION(prx_ns_entry_create__arg_entry_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_entry_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_entry_create unhappy path
//
TEST_FUNCTION(prx_ns_entry_create__neg)
{
    static const uint32_t k_type_valid;
    static const const char* k_id_valid;
    static const const char* k_name_valid;
    static const prx_ns_entry_t** k_entry_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_ns_entry_create(k_type_valid, k_id_valid, k_name_valid, k_entry_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_ns_entry_create;

#ifdef prx_ns_entry_create_from_cs

//
//Test prx_ns_entry_create_from_cs happy path
//
TEST_FUNCTION(prx_ns_entry_create_from_cs__success)
{
    static const uint32_t k_type_valid;
    static const io_ref_t* k_address_valid;
    static const io_cs_t* k_cs_valid;
    static const prx_ns_entry_t** k_entry_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = prx_ns_entry_create_from_cs(k_type_valid, k_address_valid, k_cs_valid, k_entry_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test prx_ns_entry_create_from_cs passing as type argument an invalid uint32_t value
//
TEST_FUNCTION(prx_ns_entry_create_from_cs__arg_type_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_entry_create_from_cs();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_entry_create_from_cs passing as address argument an invalid io_ref_t* value
//
TEST_FUNCTION(prx_ns_entry_create_from_cs__arg_address_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_entry_create_from_cs();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_entry_create_from_cs passing as cs argument an invalid io_cs_t* value
//
TEST_FUNCTION(prx_ns_entry_create_from_cs__arg_cs_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_entry_create_from_cs();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_entry_create_from_cs passing as entry argument an invalid prx_ns_entry_t** value
//
TEST_FUNCTION(prx_ns_entry_create_from_cs__arg_entry_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_entry_create_from_cs();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_entry_create_from_cs unhappy path
//
TEST_FUNCTION(prx_ns_entry_create_from_cs__neg)
{
    static const uint32_t k_type_valid;
    static const io_ref_t* k_address_valid;
    static const io_cs_t* k_cs_valid;
    static const prx_ns_entry_t** k_entry_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_ns_entry_create_from_cs(k_type_valid, k_address_valid, k_cs_valid, k_entry_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_ns_entry_create_from_cs;

#ifdef prx_ns_entry_release

//
//Test prx_ns_entry_release happy path
//
TEST_FUNCTION(prx_ns_entry_release__success)
{
    static const prx_ns_entry_t* k_entry_valid;
    void result;

    // arrange
    // ...

    // act
    result = prx_ns_entry_release(k_entry_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ...
}

//
// Test prx_ns_entry_release passing as entry argument an invalid prx_ns_entry_t* value
//
TEST_FUNCTION(prx_ns_entry_release__arg_entry_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_ns_entry_release();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_ns_entry_release unhappy path
//
TEST_FUNCTION(prx_ns_entry_release__neg)
{
    static const prx_ns_entry_t* k_entry_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_ns_entry_release(k_entry_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // prx_ns_entry_release



//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

