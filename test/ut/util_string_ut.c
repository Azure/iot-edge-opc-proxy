// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST util_string
#include "util_ut.h"

//
// 1. Required mocks
//
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
REGISTER_UMOCK_ALIAS_TYPE(STRING_HANDLE, void*);
END_DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
DECLARE_TEST_SETUP()


// 
// Test STRING_safe_construct_n happy path 
// 
TEST_FUNCTION(STRING_safe_construct_n__success)
{
    static STRING_HANDLE k_string_handle_valid = (STRING_HANDLE)0x2442;
    static char in[8] = { 't', 'e', 's', 't', 't', 'e', 's', 't' };
    STRING_HANDLE result;

    // arrange
    STRICT_EXPECTED_CALL(c_realloc(3, NULL, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(3).IgnoreArgument(4).IgnoreArgument(5)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(STRING_new_with_memory("te"))
        .SetReturn(k_string_handle_valid);

    // act
    result = STRING_safe_construct_n(in, 2);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, "te", UT_MEM);
    ASSERT_ARE_EQUAL(void_ptr, k_string_handle_valid, result);
}

// 
// Test STRING_safe_construct_n passing as buffer argument an invalid const char* value 
// 
TEST_FUNCTION(STRING_safe_construct_n__arg_buffer_invalid)
{
    STRING_HANDLE result;

    // act
    result = STRING_safe_construct_n(NULL, 4);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void_ptr, NULL, result);
}

//
// STRING_safe_construct_n STRING_new_with_memory fails
//
TEST_FUNCTION(STRING_safe_construct_n__neg__STRING_new_with_memory_fails)
{
    static char in[8] = { 't', 'e', 's', 't', 't', 'e', 's', 't' };
    STRING_HANDLE result;

    // arrange
    STRICT_EXPECTED_CALL(c_realloc(5, NULL, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(3).IgnoreArgument(4).IgnoreArgument(5)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(STRING_new_with_memory("test"))
        .SetReturn((STRING_HANDLE)NULL);
    STRICT_EXPECTED_CALL(c_free((void*)UT_MEM, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act
    result = STRING_safe_construct_n(in, 4);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, "test", UT_MEM);
    ASSERT_ARE_EQUAL(void_ptr, NULL, result);
}

// 
// Test STRING_safe_construct_n unhappy path 
// 
TEST_FUNCTION(STRING_safe_construct_n__neg)
{
    static STRING_HANDLE k_string_handle_valid = (STRING_HANDLE)0x2442;
    static char in[8] = { 't', 'e', 's', 't', 't', 'e', 's', 't' };
    STRING_HANDLE result;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(c_realloc(7, NULL, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(3).IgnoreArgument(4).IgnoreArgument(5)
        .SetReturn((void*)UT_MEM)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(STRING_new_with_memory("te"))
        .SetReturn(k_string_handle_valid)
        .SetFailReturn((STRING_HANDLE)NULL);
    STRICT_EXPECTED_CALL(c_free((void*)UT_MEM, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = STRING_safe_construct_n(in, 6);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, result, NULL);
}

// 
// Test STRING_construct_trim happy path 
// 
TEST_FUNCTION(STRING_construct_trim__success)
{
    static STRING_HANDLE k_string_handle_valid = (STRING_HANDLE)0x252;
    static const char* k_val_valid = "\t ;test ; ;;;;  \t";
    static const char* k_trim_chars_valid = " ;\t";
    STRING_HANDLE result;

    strcpy(UT_MEM, k_val_valid);

    // arrange 
    STRICT_EXPECTED_CALL(STRING_construct(&UT_MEM[3]))
        .SetReturn(k_string_handle_valid);
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_handle_valid))
        .SetReturn(&UT_MEM[3]);

    // act 
    result = STRING_construct_trim(k_val_valid, k_trim_chars_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void_ptr, k_string_handle_valid, result);
}

// 
// Test STRING_construct_trim passing as trim_chars argument a NULL const char* value 
// 
TEST_FUNCTION(STRING_construct_trim__success_trim_chars_null)
{
    static STRING_HANDLE k_string_handle_valid = (STRING_HANDLE)0x2446772;
    static const char* k_val_valid = "\t ;test ; ;;;;  \t";
    STRING_HANDLE result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_construct(k_val_valid))
        .SetReturn(k_string_handle_valid);

    // act 
    result = STRING_construct_trim(k_val_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void_ptr, k_string_handle_valid, result);
}

// 
// Test STRING_construct_trim passing as trim_chars argument a empty const char* value 
// 
TEST_FUNCTION(STRING_construct_trim__success_trim_chars_empty)
{
    static STRING_HANDLE k_string_handle_valid = (STRING_HANDLE)0x2446774;
    static const char* k_val_valid = "\t ;test ; ;;;;  \t";
    static const char* k_trim_chars_valid = "";
    STRING_HANDLE result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_construct(k_val_valid))
        .SetReturn(k_string_handle_valid);

    // act 
    result = STRING_construct_trim(k_val_valid, k_trim_chars_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void_ptr, k_string_handle_valid, result);
}

// 
// Test STRING_construct_trim passing as val argument an invalid const char* value 
// 
TEST_FUNCTION(STRING_construct_trim__arg_val_invalid)
{
    static const char* k_trim_chars_valid = " ;\t";
    STRING_HANDLE result;

    // arrange 

    // act 
    result = STRING_construct_trim(NULL, k_trim_chars_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void_ptr, NULL, result);
}

// 
// Test STRING_construct_trim unhappy path 
// 
TEST_FUNCTION(STRING_construct_trim__neg)
{
    static STRING_HANDLE k_string_handle_valid = (STRING_HANDLE)0x252;
    static const char* k_val_valid = "\t ;test ; ;;;;  \t";
    static const char* k_trim_chars_valid = " ;\t";
    STRING_HANDLE result;

    strcpy(UT_MEM, k_val_valid);

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(STRING_construct(&UT_MEM[3]))
        .SetReturn(k_string_handle_valid)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_handle_valid))
        .SetReturn(&UT_MEM[3])
        .SetFailReturn(&UT_MEM[3]);

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = STRING_construct_trim(k_val_valid, k_trim_chars_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, result, NULL, k_string_handle_valid);
}

// 
// Test STRING_construct_random happy path 
// 
TEST_FUNCTION(STRING_construct_random__success_1)
{
    static const char* k_random_buf_valid = "9E427F630F734843887B941ED76CBE37";
    static STRING_HANDLE k_string_handle_valid = (STRING_HANDLE)0x222233;
    static size_t k_length_valid = 4;
    STRING_HANDLE result;

    // arrange 
    STRICT_EXPECTED_CALL(c_realloc(8, NULL, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(3).IgnoreArgument(4).IgnoreArgument(5)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(pal_rand_fill(IGNORED_PTR_ARG, 2))
        .CopyOutArgumentBuffer_buf(k_random_buf_valid, 2)
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(STRING_new_with_memory("3945"))
        .SetReturn(k_string_handle_valid);

    // act 
    result = STRING_construct_random(k_length_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void_ptr, k_string_handle_valid, result);
}

// 
// Test STRING_construct_random happy path 
// 
TEST_FUNCTION(STRING_construct_random__success_2)
{
    static const char* k_random_buf_valid = "0B064B07D73A4B1C8F7BDA847320E089";
    static STRING_HANDLE k_string_handle_valid = (STRING_HANDLE)0x2254333;
    static size_t k_length_valid = 9;
    STRING_HANDLE result;

    // arrange 
    STRICT_EXPECTED_CALL(c_realloc(16, NULL, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(3).IgnoreArgument(4).IgnoreArgument(5)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(pal_rand_fill(IGNORED_PTR_ARG, 5))
        .CopyOutArgumentBuffer_buf(k_random_buf_valid, 5)
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(STRING_new_with_memory("304230363"))
        .SetReturn(k_string_handle_valid);

    // act 
    result = STRING_construct_random(k_length_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void_ptr, k_string_handle_valid, result);
}

// 
// Test STRING_construct_random with 0 length
// 
TEST_FUNCTION(STRING_construct_random__arg_len_invalid)
{
    STRING_HANDLE result;

    // arrange 

    // act 
    result = STRING_construct_random(0);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_NULL(result);
}

// 
// Test STRING_construct_random unhappy path 
// 
TEST_FUNCTION(STRING_construct_random__neg)
{
    static const char* k_random_buf_valid = "0B064B07D73A4B1C8F7BDA847320E089";
    static STRING_HANDLE k_string_handle_valid = (STRING_HANDLE)0x2254333;
    static size_t k_length_valid = 9;
    STRING_HANDLE result;

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(c_realloc(37, NULL, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(3).IgnoreArgument(4).IgnoreArgument(5)
        .SetReturn((void*)UT_MEM)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(pal_rand_fill(IGNORED_PTR_ARG, 5))
        .CopyOutArgumentBuffer_buf(k_random_buf_valid, 5)
        .SetReturn(er_ok)
        .SetFailReturn(er_fatal);
    STRICT_EXPECTED_CALL(STRING_new_with_memory("304230363"))
        .SetReturn(k_string_handle_valid)
        .SetFailReturn((STRING_HANDLE)NULL);
    STRICT_EXPECTED_CALL(c_free((void*)UT_MEM, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = STRING_construct_random(k_length_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, result, NULL);
}

// 
// Test STRING_construct_utf8 happy path 
// 
TEST_FUNCTION(STRING_construct_utf8__success)
{
    static const uint8_t k_buf_valid[] = { 'a', 127, 'b', 128, 'c', 129, 'd', 180, 'e', 255 };
    static const size_t k_buf_len_valid = sizeof(k_buf_valid);
    static STRING_HANDLE k_string_handle_valid = (STRING_HANDLE)0xabcde0;
    STRING_HANDLE result;

    // arrange 
    STRICT_EXPECTED_CALL(c_realloc(15, NULL, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(3).IgnoreArgument(4).IgnoreArgument(5)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(STRING_new_with_memory(IGNORED_PTR_ARG))
        .IgnoreArgument(1)
        .SetReturn(k_string_handle_valid);

    // act 
    result = STRING_construct_utf8(k_buf_valid, k_buf_len_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void_ptr, k_string_handle_valid, result);
}

// 
// Test STRING_construct_utf8 passing as buf argument an invalid const uint8_t* value 
// 
TEST_FUNCTION(STRING_construct_utf8__arg_buf_invalid)
{
    STRING_HANDLE result;

    // arrange 

    // act 
    result = STRING_construct_utf8(NULL, 14);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void_ptr, NULL, result);
}

// 
// Test STRING_construct_utf8 passing as buf_len argument an invalid size_t value 
// 
TEST_FUNCTION(STRING_construct_utf8__arg_buf_len_invalid)
{
    static const uint8_t k_buf_valid[] = { 'a', 199, 'b', 127, 'c', 99, 'd', 0 };
    STRING_HANDLE result;

    // arrange 

    // act 
    result = STRING_construct_utf8(k_buf_valid, 0);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void_ptr, NULL, result);
}

// 
// Test STRING_construct_utf8 unhappy path 
// 
TEST_FUNCTION(STRING_construct_utf8__neg)
{
    static const uint8_t k_buf_valid[] = { 1, 127, 2, 128, 3, 129, 4, 180, 0, 255 };
    static const size_t k_buf_len_valid = sizeof(k_buf_valid);
    static STRING_HANDLE k_string_handle_valid = (STRING_HANDLE)0xabcd0;
    STRING_HANDLE result;

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(c_realloc(15, NULL, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(3).IgnoreArgument(4).IgnoreArgument(5)
        .SetReturn((void*)UT_MEM)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(STRING_new_with_memory(IGNORED_PTR_ARG))
        .IgnoreArgument(1)
        .SetReturn(k_string_handle_valid)
        .SetFailReturn((STRING_HANDLE)NULL);
    STRICT_EXPECTED_CALL(c_free((void*)UT_MEM, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = STRING_construct_utf8(k_buf_valid, k_buf_len_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, result, NULL, NULL, k_string_handle_valid);
}

// 
// Test STRING_construct_base16 happy path 
// 
TEST_FUNCTION(STRING_construct_base16__success)
{
    static const uint8_t k_buf_valid[] = { 1, 127, 2, 128, 3, 129, 4, 180, 0, 255 };
    static const size_t k_buf_len_valid = sizeof(k_buf_valid);
    static STRING_HANDLE k_string_handle_valid = (STRING_HANDLE)0xabcde0;
    STRING_HANDLE result;

    // arrange 
    STRICT_EXPECTED_CALL(c_realloc(21, NULL, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(3).IgnoreArgument(4).IgnoreArgument(5)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(STRING_new_with_memory(IGNORED_PTR_ARG))
        .IgnoreArgument(1)
        .SetReturn(k_string_handle_valid);

    // act 
    result = STRING_construct_base16(k_buf_valid, k_buf_len_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void_ptr, k_string_handle_valid, result);
}

// 
// Test STRING_construct_base16 passing as buf argument an invalid const uint8_t* value 
// 
TEST_FUNCTION(STRING_construct_base16__arg_buf_invalid)
{
    STRING_HANDLE result;

    // arrange 

    // act 
    result = STRING_construct_base16(NULL, 14);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void_ptr, NULL, result);
}

// 
// Test STRING_construct_base16 passing as buf_len argument an invalid size_t value 
// 
TEST_FUNCTION(STRING_construct_base16__arg_buf_len_invalid)
{
    static const uint8_t k_buf_valid[] = { 1, 127, 2, 128, 3, 129, 4, 180, 0, 255 };
    STRING_HANDLE result;

    // arrange 

    // act 
    result = STRING_construct_base16(k_buf_valid, 0);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void_ptr, NULL, result);
}

// 
// Test STRING_construct_base16 unhappy path 
// 
TEST_FUNCTION(STRING_construct_base16__neg)
{
    static const uint8_t k_buf_valid[] = { 1, 127, 2, 128, 3, 129, 4, 180, 0, 255 };
    static const size_t k_buf_len_valid = sizeof(k_buf_valid);
    static STRING_HANDLE k_string_handle_valid = (STRING_HANDLE)0xabcd0;
    STRING_HANDLE result;

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(c_realloc(21, NULL, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(3).IgnoreArgument(4).IgnoreArgument(5)
        .SetReturn((void*)UT_MEM)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(STRING_new_with_memory(IGNORED_PTR_ARG))
        .IgnoreArgument(1)
        .SetReturn(k_string_handle_valid)
        .SetFailReturn((STRING_HANDLE)NULL);
    STRICT_EXPECTED_CALL(c_free((void*)UT_MEM, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = STRING_construct_base16(k_buf_valid, k_buf_len_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, result, NULL, NULL, k_string_handle_valid);
}

// 
// Test STRING_concat_n happy path 
// 
TEST_FUNCTION(STRING_concat_n__success)
{
    static const STRING_HANDLE k_string_in_valid = (STRING_HANDLE)0x1;
    static const STRING_HANDLE k_string_new_valid = (STRING_HANDLE)0x2;
    static const char* k_buffer_valid = "1234567890123456789012345678901234567890";
    static const size_t k_len_valid = 6;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(c_realloc(7, NULL, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(3).IgnoreArgument(4).IgnoreArgument(5)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(STRING_new_with_memory("123456"))
        .SetReturn(k_string_new_valid);
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(k_string_in_valid, k_string_new_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_delete(k_string_new_valid));

    // act 
    result = STRING_concat_n(k_string_in_valid, k_buffer_valid, k_len_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test STRING_concat_n passing as string argument an invalid STRING_HANDLE value 
// 
TEST_FUNCTION(STRING_concat_n__arg_string_invalid)
{
    int32_t result;

    // arrange 

    // act 
    result = STRING_concat_n(NULL, "testtest", 4);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test STRING_concat_n passing as buffer argument an invalid const char* value 
// 
TEST_FUNCTION(STRING_concat_n__arg_buffer_invalid)
{
    static const STRING_HANDLE k_string_in_valid = (STRING_HANDLE)0x1;
    int32_t result;

    // arrange 

    // act 
    result = STRING_concat_n(k_string_in_valid, NULL, 4);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test STRING_concat_n unhappy path 
// 
TEST_FUNCTION(STRING_concat_n__neg)
{
    static const STRING_HANDLE k_string_in_valid = (STRING_HANDLE)0x1;
    static const STRING_HANDLE k_string_new_valid = (STRING_HANDLE)0x2;
    static const char* k_buffer_valid = "1234567890123456789012345678901234567890";
    static const size_t k_len_valid = 4;
    int32_t result;

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(c_realloc(5, NULL, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(3).IgnoreArgument(4).IgnoreArgument(5)
        .SetReturn((void*)UT_MEM)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(STRING_new_with_memory("1234"))
        .SetReturn(k_string_new_valid)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(k_string_in_valid, k_string_new_valid))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_delete(k_string_new_valid));
    EXPECTED_CALL(c_free((void*)UT_MEM, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = STRING_concat_n(k_string_in_valid, k_buffer_valid, k_len_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_out_of_memory, er_out_of_memory, er_out_of_memory, er_ok);
}

// 
// Test STRING_concat_int happy path 
// 
TEST_FUNCTION(STRING_concat_int__success)
{
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x66;
    static const int32_t k_value_valid = 1234;
    static const int32_t k_radix_valid = 10;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, "1234"))
        .SetReturn(0);

    // act 
    result = STRING_concat_int(k_string_valid, k_value_valid, k_radix_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test STRING_concat_int passing as string argument an invalid STRING_HANDLE value 
// 
TEST_FUNCTION(STRING_concat_int__arg_string_invalid)
{
    int32_t result;

    // arrange 

    // act 
    result = STRING_concat_int(NULL, 10, 10);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test STRING_concat_int passing as radix argument an invalid int32_t value 
// 
TEST_FUNCTION(STRING_concat_int__arg_radix_invalid)
{
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x66;
    int32_t result;

    // arrange 

    // act 
    result = STRING_concat_int(k_string_valid, 10, 0);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_arg, result);
}

// 
// Test STRING_concat_int unhappy path 
// 
TEST_FUNCTION(STRING_concat_int__neg)
{
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x66;
    static const int32_t k_value_valid = 1234;
    static const int32_t k_radix_valid = 10;
    int32_t result;

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, "1234"))
        .SetReturn(0)
        .SetFailReturn(-1);

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = STRING_concat_int(k_string_valid, k_value_valid, k_radix_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_out_of_memory);
}

// 
// Test STRING_update happy path 
// 
TEST_FUNCTION(STRING_update__success)
{
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x55555;
    static const char* k_val_valid = "test";
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn("dddd");
    STRICT_EXPECTED_CALL(STRING_empty(k_string_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, k_val_valid))
        .SetReturn(0);

    // act 
    result = STRING_update(k_string_valid, k_val_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test STRING_update happy path 
// 
TEST_FUNCTION(STRING_update__success_equal_value)
{
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x55555;
    static const char* k_val_valid = "test";
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn("test");

    // act 
    result = STRING_update(k_string_valid, k_val_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test STRING_update passing as string argument an invalid STRING_HANDLE value 
// 
TEST_FUNCTION(STRING_update__arg_string_invalid)
{
    int32_t result;

    // arrange 

    // act 
    result = STRING_update(NULL, "test");

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test STRING_update passing as val argument an invalid const char* value 
// 
TEST_FUNCTION(STRING_update__arg_val_invalid)
{
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0xafafa;
    int32_t result;

    // arrange 

    // act 
    result = STRING_update(k_string_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test STRING_update unhappy path 
// 
TEST_FUNCTION(STRING_update__neg)
{
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x55555;
    static const char* k_val_valid = "test";
    int32_t result;

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn("dddd")
        .SetFailReturn("dddd");
    STRICT_EXPECTED_CALL(STRING_empty(k_string_valid))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(STRING_concat(k_string_valid, k_val_valid))
        .SetReturn(0)
        .SetFailReturn(-1);

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = STRING_update(k_string_valid, k_val_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok, er_out_of_memory);
}

// 
// Test STRING_compare_nocase happy path 
// 
TEST_FUNCTION(STRING_compare_nocase__success_equal_diff_case)
{
    static const STRING_HANDLE k_string1_valid = (STRING_HANDLE)0x5dfa55;
    static const STRING_HANDLE k_string2_valid = (STRING_HANDLE)0x55fa55;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string2_valid))
        .SetReturn("dddD");
    STRICT_EXPECTED_CALL(STRING_c_str(k_string1_valid))
        .SetReturn("DdDd");

    // act 
    result = STRING_compare_nocase(k_string1_valid, k_string2_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, 0, result);
}

// 
// Test STRING_compare_nocase happy path 
// 
TEST_FUNCTION(STRING_compare_nocase__success_equal_same_case)
{
    static const STRING_HANDLE k_string1_valid = (STRING_HANDLE)0x5dfa55;
    static const STRING_HANDLE k_string2_valid = (STRING_HANDLE)0x55fa55;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string2_valid))
        .SetReturn("dddd");
    STRICT_EXPECTED_CALL(STRING_c_str(k_string1_valid))
        .SetReturn("dddd");

    // act 
    result = STRING_compare_nocase(k_string1_valid, k_string2_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, 0, result);
}

// 
// Test STRING_compare_nocase happy path 
// 
TEST_FUNCTION(STRING_compare_nocase__success_equal_empty_string)
{
    static const STRING_HANDLE k_string1_valid = (STRING_HANDLE)0x5dfa55;
    static const STRING_HANDLE k_string2_valid = (STRING_HANDLE)0x55fa55;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string2_valid))
        .SetReturn("");
    STRICT_EXPECTED_CALL(STRING_c_str(k_string1_valid))
        .SetReturn("");

    // act 
    result = STRING_compare_nocase(k_string1_valid, k_string2_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, 0, result);
}

// 
// Test STRING_compare_nocase happy path 
// 
TEST_FUNCTION(STRING_compare_nocase__success_not_equal)
{
    static const STRING_HANDLE k_string1_valid = (STRING_HANDLE)0x5dfa55;
    static const STRING_HANDLE k_string2_valid = (STRING_HANDLE)0x55fa55;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string2_valid))
        .SetReturn("dddd");
    STRICT_EXPECTED_CALL(STRING_c_str(k_string1_valid))
        .SetReturn("aaaa");

    // act 
    result = STRING_compare_nocase(k_string1_valid, k_string2_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_NOT_EQUAL(int32_t, 0, result);
}

// 
// Test STRING_compare_nocase happy path 
// 
TEST_FUNCTION(STRING_compare_nocase__success_not_equal_diff_length_1)
{
    static const STRING_HANDLE k_string1_valid = (STRING_HANDLE)0x5dfa55;
    static const STRING_HANDLE k_string2_valid = (STRING_HANDLE)0x55fa55;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string2_valid))
        .SetReturn("dddd");
    STRICT_EXPECTED_CALL(STRING_c_str(k_string1_valid))
        .SetReturn("ddddddd");

    // act 
    result = STRING_compare_nocase(k_string1_valid, k_string2_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_NOT_EQUAL(int32_t, 0, result);
}

// 
// Test STRING_compare_nocase happy path 
// 
TEST_FUNCTION(STRING_compare_nocase__success_not_equal_diff_length_2)
{
    static const STRING_HANDLE k_string1_valid = (STRING_HANDLE)0x5dfa55;
    static const STRING_HANDLE k_string2_valid = (STRING_HANDLE)0x55fa55;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string2_valid))
        .SetReturn("DDDDDDD");
    STRICT_EXPECTED_CALL(STRING_c_str(k_string1_valid))
        .SetReturn("DDDD");

    // act 
    result = STRING_compare_nocase(k_string1_valid, k_string2_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_NOT_EQUAL(int32_t, 0, result);
}

// 
// Test STRING_compare_nocase passing as string argument an invalid STRING_HANDLE value 
// 
TEST_FUNCTION(STRING_compare_nocase__arg_string_invalid)
{
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0xa55;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn("ABCD");

    // act 
    result = STRING_compare_nocase(NULL, k_string_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_NOT_EQUAL(int32_t, 0, result);
}

// 
// Test STRING_compare_nocase passing as compare_to argument an invalid STRING_HANDLE value 
// 
TEST_FUNCTION(STRING_compare_nocase__arg_compare_to_invalid)
{
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0xa55;
    int32_t result;

    // arrange 

    // act 
    result = STRING_compare_nocase(k_string_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_NOT_EQUAL(int32_t, 0, result);
}

// 
// Test STRING_compare_c_str happy path 
// 
TEST_FUNCTION(STRING_compare_c_str__success_equal)
{
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0xa55;
    static const char* k_compare_to_valid = "test";
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn("test");

    // act 
    result = STRING_compare_c_str(k_string_valid, k_compare_to_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, 0, result);
}

// 
// Test STRING_compare_c_str happy path 
// 
TEST_FUNCTION(STRING_compare_c_str__success_not_equal)
{
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0xa55;
    static const char* k_compare_to_valid = "Test";
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn("test");

    // act 
    result = STRING_compare_c_str(k_string_valid, k_compare_to_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_NOT_EQUAL(int32_t, 0, result);
}

// 
// Test STRING_compare_c_str passing as string argument an invalid STRING_HANDLE value 
// 
TEST_FUNCTION(STRING_compare_c_str__arg_string_invalid)
{
    int32_t result;

    // arrange 

    // act 
    result = STRING_compare_c_str(NULL, "test");

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_NOT_EQUAL(int32_t, 0, result);
}

// 
// Test STRING_compare_c_str passing as compare_to argument an invalid const char* value 
// 
TEST_FUNCTION(STRING_compare_c_str__arg_compare_to_invalid)
{
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0xa55;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn("test");

    // act 
    result = STRING_compare_c_str(k_string_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_NOT_EQUAL(int32_t, 0, result);
}

// 
// Test STRING_compare_c_str_nocase happy path 
// 
TEST_FUNCTION(STRING_compare_c_str_nocase__success_equal_case)
{
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0xa55;
    static const char* k_compare_to_valid = "test";
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn("test");

    // act 
    result = STRING_compare_c_str_nocase(k_string_valid, k_compare_to_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, 0, result);
}

// 
// Test STRING_compare_c_str_nocase happy path 
// 
TEST_FUNCTION(STRING_compare_c_str_nocase__success_equal_nocase)
{
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0xa55;
    static const char* k_compare_to_valid = "test";
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn("Test");

    // act 
    result = STRING_compare_c_str_nocase(k_string_valid, k_compare_to_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, 0, result);
}

// 
// Test STRING_compare_c_str_nocase happy path 
// 
TEST_FUNCTION(STRING_compare_c_str_nocase__success_not_equal_nocase)
{
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0xa55;
    static const char* k_compare_to_valid = "test";
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn("Tester");

    // act 
    result = STRING_compare_c_str_nocase(k_string_valid, k_compare_to_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_NOT_EQUAL(int32_t, 0, result);
}

// 
// Test STRING_compare_c_str_nocase passing as string argument an invalid STRING_HANDLE value 
// 
TEST_FUNCTION(STRING_compare_c_str_nocase__arg_string_invalid)
{
    int32_t result;

    // arrange 

    // act 
    result = STRING_compare_c_str_nocase(NULL, "Test");

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_NOT_EQUAL(int32_t, 0, result);
}

// 
// Test STRING_compare_c_str_nocase passing as compare_to argument an invalid const char* value 
// 
TEST_FUNCTION(STRING_compare_c_str_nocase__arg_compare_to_invalid)
{
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0xa55;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn("test");

    // act 
    result = STRING_compare_c_str_nocase(k_string_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_NOT_EQUAL(int32_t, 0, result);
}

// 
// Test STRING_find happy path 
// 
TEST_FUNCTION(STRING_find__success_found)
{
    static const STRING_HANDLE k_string1_valid = (STRING_HANDLE)0x5dfa55;
    static const STRING_HANDLE k_string2_valid = (STRING_HANDLE)0x55fa55;
    const char* result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string2_valid))
        .SetReturn("ABCD");
    STRICT_EXPECTED_CALL(STRING_c_str(k_string1_valid))
        .SetReturn("     ABCDEFG");

    // act 
    result = STRING_find(k_string1_valid, k_string2_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, "ABCDEFG", result);
}

// 
// Test STRING_find happy path 
// 
TEST_FUNCTION(STRING_find__success_not_found1)
{
    static const STRING_HANDLE k_string1_valid = (STRING_HANDLE)0x5dfa55;
    static const STRING_HANDLE k_string2_valid = (STRING_HANDLE)0x55fa55;
    const char* result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string2_valid))
        .SetReturn("BACD");
    STRICT_EXPECTED_CALL(STRING_c_str(k_string1_valid))
        .SetReturn("     ABCDEFG");

    // act 
    result = STRING_find(k_string1_valid, k_string2_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test STRING_find happy path 
// 
TEST_FUNCTION(STRING_find__success_not_found2)
{
    static const STRING_HANDLE k_string1_valid = (STRING_HANDLE)0x5dfa55;
    static const STRING_HANDLE k_string2_valid = (STRING_HANDLE)0x55fa55;
    const char* result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string2_valid))
        .SetReturn("abcd");
    STRICT_EXPECTED_CALL(STRING_c_str(k_string1_valid))
        .SetReturn("12345ABCDEFG");

    // act 
    result = STRING_find(k_string1_valid, k_string2_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test STRING_find passing as string argument an invalid STRING_HANDLE value 
// 
TEST_FUNCTION(STRING_find__arg_string_invalid)
{
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x5dfa55;
    const char* result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn("ABCD");

    // act 
    result = STRING_find(NULL, k_string_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test STRING_find passing as to_find argument an invalid STRING_HANDLE value 
// 
TEST_FUNCTION(STRING_find__arg_to_find_invalid)
{
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x5dfa55;
    const char* result;

    // arrange 

    // act 
    result = STRING_find(k_string_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test STRING_find_nocase happy path 
// 
TEST_FUNCTION(STRING_find_nocase__success_case)
{
    static const STRING_HANDLE k_string1_valid = (STRING_HANDLE)0x5dfa55;
    static const STRING_HANDLE k_string2_valid = (STRING_HANDLE)0x55fa55;
    const char* result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string2_valid))
        .SetReturn("ABCD");
    STRICT_EXPECTED_CALL(STRING_c_str(k_string1_valid))
        .SetReturn("12345ABCDEFG");

    // act 
    result = STRING_find_nocase(k_string1_valid, k_string2_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, "ABCDEFG", result);
}

// 
// Test STRING_find_nocase happy path 
// 
TEST_FUNCTION(STRING_find_nocase__success_nocase)
{
    static const STRING_HANDLE k_string1_valid = (STRING_HANDLE)0x5dfa55;
    static const STRING_HANDLE k_string2_valid = (STRING_HANDLE)0x55fa55;
    const char* result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string2_valid))
        .SetReturn("abcd");
    STRICT_EXPECTED_CALL(STRING_c_str(k_string1_valid))
        .SetReturn("12345ABCDEFG");

    // act 
    result = STRING_find_nocase(k_string1_valid, k_string2_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, "ABCDEFG", result);
}

// 
// Test STRING_find_nocase happy path 
// 
TEST_FUNCTION(STRING_find_nocase__success_not_found1)
{
    static const STRING_HANDLE k_string1_valid = (STRING_HANDLE)0x5dfa55;
    static const STRING_HANDLE k_string2_valid = (STRING_HANDLE)0x55fa55;
    const char* result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string2_valid))
        .SetReturn("abcdefghijkl");
    STRICT_EXPECTED_CALL(STRING_c_str(k_string1_valid))
        .SetReturn("12345ABCDEFG");

    // act 
    result = STRING_find_nocase(k_string1_valid, k_string2_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test STRING_find_nocase happy path 
// 
TEST_FUNCTION(STRING_find_nocase__success_not_found2)
{
    static const STRING_HANDLE k_string1_valid = (STRING_HANDLE)0x5dfa55;
    static const STRING_HANDLE k_string2_valid = (STRING_HANDLE)0x55fa55;
    const char* result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string2_valid))
        .SetReturn("");
    STRICT_EXPECTED_CALL(STRING_c_str(k_string1_valid))
        .SetReturn("12345ABCDEFG");

    // act 
    result = STRING_find_nocase(k_string1_valid, k_string2_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test STRING_find_nocase passing as string argument an invalid STRING_HANDLE value 
// 
TEST_FUNCTION(STRING_find_nocase__arg_string_invalid)
{
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x5dfa55;
    const char* result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn("ABCD");

    // act 
    result = STRING_find_nocase(NULL, k_string_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test STRING_find_nocase passing as to_find argument an invalid STRING_HANDLE value 
// 
TEST_FUNCTION(STRING_find_nocase__arg_to_find_invalid)
{
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x5dfa55;
    const char* result;

    // arrange 

    // act 
    result = STRING_find_nocase(k_string_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test STRING_find_c_str happy path 
// 
TEST_FUNCTION(STRING_find_c_str__success_found)
{
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x5dfa55;
    const char* result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn("12345abcdefg");

    // act 
    result = STRING_find_c_str(k_string_valid, "ab");

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, "abcdefg", result);
}

// 
// Test STRING_find_c_str happy path 
// 
TEST_FUNCTION(STRING_find_c_str__success_notfound)
{
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x5dfa55;
    const char* result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn("12345abcdefg");

    // act 
    result = STRING_find_c_str(k_string_valid, "AB");

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test STRING_find_c_str passing as string argument an invalid STRING_HANDLE value 
// 
TEST_FUNCTION(STRING_find_c_str__arg_string_invalid)
{
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x5dfa55;
    const char* result;

    // arrange 

    // act 
    result = STRING_find_c_str(NULL, "ab");

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test STRING_find_c_str passing as to_find argument an invalid const char* value 
// 
TEST_FUNCTION(STRING_find_c_str__arg_to_find_invalid)
{
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x5dfa55;
    const char* result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn("12345abcdefg");

    // act 
    result = STRING_find_c_str(k_string_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test STRING_find_c_str_nocase happy path 
// 
TEST_FUNCTION(STRING_find_c_str_nocase__success)
{
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x5dfa55;
    const char* result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn("12345abcdefg");

    // act 
    result = STRING_find_c_str_nocase(k_string_valid, "AB");

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, "abcdefg", result);
}

// 
// Test STRING_find_c_str_nocase happy path 
// 
TEST_FUNCTION(STRING_find_c_str_nocase__success_not_found)
{
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x5dfa55;
    const char* result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn("12345abcdefg");

    // act 
    result = STRING_find_c_str_nocase(k_string_valid, "abdefg");

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test STRING_find_c_str_nocase passing as string argument an invalid STRING_HANDLE value 
// 
TEST_FUNCTION(STRING_find_c_str_nocase__arg_string_invalid)
{
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x5dfa55;
    const char* result;

    // arrange 

    // act 
    result = STRING_find_c_str_nocase(NULL, "ab");

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test STRING_find_c_str_nocase passing as to_find argument an invalid const char* value 
// 
TEST_FUNCTION(STRING_find_c_str_nocase__arg_to_find_invalid)
{
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x5dfa55;
    const char* result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_string_valid))
        .SetReturn("12345abcdefg");

    // act 
    result = STRING_find_c_str_nocase(k_string_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test STRING_clone_c_str happy path 
// 
TEST_FUNCTION(STRING_clone_c_str__success)
{
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x137456;
    static const char* k_string_c_str = "abcdefghijklmnopqrstuvwxyz";
    char* copy_valid;
    int32_t result;

    strcpy(UT_MEM, k_string_c_str);

    // arrange 
    STRICT_EXPECTED_CALL(STRING_length(k_string_valid))
        .SetReturn(10);
    STRICT_EXPECTED_CALL(h_realloc(10 + 1, NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(STRING_copy(k_string_valid, k_string_c_str))
        .SetReturn(0);

    // act 
    result = STRING_clone_c_str(k_string_valid, &copy_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(char_ptr, "abcdefghij", UT_MEM);
}

// 
// Test STRING_clone_c_str passing as string argument an invalid STRING_HANDLE value 
// 
TEST_FUNCTION(STRING_clone_c_str__arg_string_invalid)
{
    char* copy_valid;
    int32_t result;

    // arrange 

    // act 
    result = STRING_clone_c_str(NULL, &copy_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test STRING_clone_c_str passing as copy argument an invalid char** value 
// 
TEST_FUNCTION(STRING_clone_c_str__arg_copy_invalid)
{
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x137456;
    int32_t result;

    // arrange 

    // act 
    result = STRING_clone_c_str(k_string_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test STRING_clone_c_str unhappy path 
// 
TEST_FUNCTION(STRING_clone_c_str__neg)
{
    static const char* k_string_c_str = "abcdefghijklmnopqrstuvwxyz";
    static const STRING_HANDLE k_string_valid = (STRING_HANDLE)0x137456;
    char* copy_valid;
    int32_t result;

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(STRING_length(k_string_valid))
        .SetReturn(10)
        .SetFailReturn(10);
    STRICT_EXPECTED_CALL(h_realloc(10 + 1, NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(STRING_copy(k_string_valid, k_string_c_str))
        .SetReturn(0)
        .SetFailReturn(-1);
    STRICT_EXPECTED_CALL(h_free((void*)UT_MEM, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    strcpy(UT_MEM, k_string_c_str);
    result = STRING_clone_c_str(k_string_valid, &copy_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok, er_out_of_memory, er_fatal, er_ok);
}

// 
// Test string_clone happy path 
// 
TEST_FUNCTION(string_clone__success)
{
    static const char* k_string_c_str = "0123456789";
    char* copy_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(10 + 1, NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);

    // act 
    result = string_clone(k_string_c_str, &copy_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(char_ptr, "0123456789", UT_MEM);
}

// 
// Test string_clone passing as string argument an invalid const char* value 
// 
TEST_FUNCTION(string_clone__arg_string_invalid)
{
    char* copy_valid;
    int32_t result;

    // arrange 

    // act 
    result = string_clone(NULL, &copy_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test string_clone passing as copy argument an invalid char** value 
// 
TEST_FUNCTION(string_clone__arg_copy_invalid)
{
    static const char* k_string_c_str = "0123456789";
    int32_t result;

    // arrange 

    // act 
    result = string_clone(k_string_c_str, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test string_clone unhappy path 
// 
TEST_FUNCTION(string_clone__neg)
{
    static const char* k_string_c_str = "0123456789";
    char* copy_valid;
    int32_t result;

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(h_realloc(10 + 1, NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM)
        .SetFailReturn(NULL);

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = string_clone(k_string_c_str, &copy_valid);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_out_of_memory);
}

// 
// Test string_from_int happy path 
// 
TEST_FUNCTION(string_from_int__success)
{
    static const int32_t k_value_valid = 1234567;
    static const int32_t k_radix_valid = 10;
    char* int_string_valid = UT_MEM;
    size_t int_string_len_valid = sizeof(UT_MEM);
    int32_t result;

    // arrange 

    // act 
    result = string_from_int(k_value_valid, k_radix_valid, int_string_valid, int_string_len_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(char_ptr, "1234567", int_string_valid);
}

// 
// Test string_from_int passing as radix argument an invalid int32_t value 
// 
TEST_FUNCTION(string_from_int__arg_radix_invalid)
{
    static const int32_t k_value_valid = 1234567;
    char* int_string_valid = UT_MEM;
    size_t int_string_len_valid = sizeof(UT_MEM);
    int32_t result;

    // arrange 

    // act 
    result = string_from_int(k_value_valid, 0, int_string_valid, int_string_len_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_arg, result);
}

// 
// Test string_from_int passing as string argument an invalid char* value 
// 
TEST_FUNCTION(string_from_int__arg_string_invalid)
{
    static const int32_t k_value_valid = 1234567;
    static const int32_t k_radix_valid = 10;
    int32_t result;

    // arrange 

    // act 
    result = string_from_int(k_value_valid, k_radix_valid, NULL, 256);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test string_trim_scheme happy path 
// 
TEST_FUNCTION(string_trim_scheme__success)
{
    static const char* k_host_name_valid = "mqtts://testhost.azure.com/https://test";
    size_t name_len_valid = 26;
    const char* result;

    // arrange 

    // act 
    result = string_trim_scheme(k_host_name_valid, &name_len_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, "testhost.azure.com/https://test", result);
    ASSERT_ARE_EQUAL(size_t, 18, name_len_valid);
}

// 
// Test string_trim_scheme passing as name_len argument an invalid size_t* value 
// 
TEST_FUNCTION(string_trim_scheme__success_name_len_invalid)
{
    static const char* k_host_name_valid = "mqtts://testhost.azure.com";
    const char* result;

    // arrange 

    // act 
    result = string_trim_scheme(k_host_name_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, "testhost.azure.com", result);
}

// 
// Test string_trim_scheme passing as host_name argument an invalid const char* value 
// 
TEST_FUNCTION(string_trim_scheme__arg_host_name_invalid)
{
    size_t name_len_valid;
    const char* result;

    // arrange 

    // act 
    result = string_trim_scheme(NULL, &name_len_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test string_trim_back_len happy path 
// 
TEST_FUNCTION(string_trim_back_len__success)
{
    static const char* k_val_valid = "\t ;test ; ;;;;  \t";
    static size_t k_val_len_valid = 12;
    static const char* k_trim_chars_valid = " ;\t";
    size_t result;

    // arrange 

    // act 
    result = string_trim_back_len(k_val_valid, k_val_len_valid, k_trim_chars_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(size_t, 7, result);
}

// 
// Test string_trim_back_len passing as val argument an invalid const char* value 
// 
TEST_FUNCTION(string_trim_back_len__arg_val_invalid)
{
    static size_t k_val_len_valid = 12;
    static const char* k_trim_chars_valid = " ;\t";
    size_t result;

    // arrange 

    // act 
    result = string_trim_back_len(NULL, k_val_len_valid, k_trim_chars_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(size_t, 0, result);
}

// 
// Test string_trim_back_len passing as trim_chars argument an invalid const char* value 
// 
TEST_FUNCTION(string_trim_back_len__arg_trim_chars_invalid)
{
    static size_t k_val_len_valid = 12;
    static const char* k_val_valid = "\t ;test ; ;;;;  \t";
    size_t result;

    // arrange 

    // act 
    result = string_trim_back_len(k_val_valid, k_val_len_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(size_t, k_val_len_valid, result);
}

// 
// Test string_is_equal_nocase happy path 
// 
TEST_FUNCTION(string_is_equal_nocase__success)
{
    static const char* k_val_valid = "test";
    const size_t k_len_valid = 4;
    bool result;
    // arrange 
    // ... 

    // act 
    result = string_is_equal_nocase(k_val_valid, k_len_valid, "test");

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_TRUE(result);
}

// 
// Test string_is_equal_nocase happy path 
// 
TEST_FUNCTION(string_is_equal_nocase__success_nocase)
{
    static const char* k_val_valid = "test";
    const size_t k_len_valid = 4;
    bool result;
    // arrange 
    // ... 

    // act 
    result = string_is_equal_nocase(k_val_valid, k_len_valid, "TEST");

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_TRUE(result);
}

// 
// Test string_is_equal_nocase happy path 
// 
TEST_FUNCTION(string_is_equal_nocase__success_both_null)
{
    bool result;
    // arrange 
    // ... 

    // act 
    result = string_is_equal_nocase(NULL, 0, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_TRUE(result);
}

// 
// Test string_is_equal_nocase happy path 
// 
TEST_FUNCTION(string_is_equal_nocase__success_notequal_nocase)
{
    static const char* k_val_valid = "test";
    const size_t k_len_valid = 4;
    bool result;
    // arrange 
    // ... 

    // act 
    result = string_is_equal_nocase(k_val_valid, k_len_valid, "TEST ");

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_FALSE(result);
}

// 
// Test string_is_equal_nocase happy path 
// 
TEST_FUNCTION(string_is_equal_nocase__success_notequal_case)
{
    static const char* k_val_valid = "test";
    const size_t k_len_valid = 4;
    bool result;
    // arrange 
    // ... 

    // act 
    result = string_is_equal_nocase(k_val_valid, k_len_valid, "test ");

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_FALSE(result);
}

// 
// Test string_is_equal_nocase passing as val argument an invalid const char* value 
// 
TEST_FUNCTION(string_is_equal_nocase__arg_val_invalid)
{
    bool result;

    // arrange 

    // act 
    result = string_is_equal_nocase(NULL, 12, "test");

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_FALSE(result);
}

// 
// Test string_is_equal_nocase passing as to argument an invalid const char* value 
// 
TEST_FUNCTION(string_is_equal_nocase__arg_to_invalid)
{
    bool result;

    // arrange 

    // act 
    result = string_is_equal_nocase("test", 4, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_FALSE(result);
}

// 
// Test string_is_equal happy path 
// 
TEST_FUNCTION(string_is_equal__success)
{
    static const char* k_val_valid = "testtest";
    const size_t k_len_valid = 4;
    bool result;
    
    // arrange 
    
    // act 
    result = string_is_equal_nocase(k_val_valid, k_len_valid, "test");

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_TRUE(result);
}

// 
// Test string_is_equal_nocase happy path 
// 
TEST_FUNCTION(string_is_equal_success_both_null)
{
    bool result;
    // arrange 
    // ... 

    // act 
    result = string_is_equal(NULL, 0, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_TRUE(result);
}

// 
// Test string_is_equal happy path 
// 
TEST_FUNCTION(string_is_equal__success_not_equal)
{
    static const char* k_val_valid = "test";
    const size_t k_len_valid = 4;
    bool result;

    // arrange 

    // act 
    result = string_is_equal(k_val_valid, k_len_valid, "TEST");

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_FALSE(result);
}

// 
// Test string_is_equal happy path 
// 
TEST_FUNCTION(string_is_equal__success_not_equal_empty)
{
    static const char* k_val_valid = "test";
    const size_t k_len_valid = 3;
    bool result;

    // arrange 

    // act 
    result = string_is_equal_nocase(k_val_valid, k_len_valid, "");

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_FALSE(result);
}

// 
// Test string_is_equal passing as val argument an invalid const char* value 
// 
TEST_FUNCTION(string_is_equal__arg_val_invalid)
{
    bool result;

    // arrange 

    // act 
    result = string_is_equal(NULL, 4, "test");

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_FALSE(result);
}


// 
// Test string_is_equal passing as to argument an invalid const char* value 
// 
TEST_FUNCTION(string_is_equal__arg_to_invalid)
{
    bool result;

    // arrange 

    // act 
    result = string_is_equal("test", 4, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_FALSE(result);
}

// 
// Test string_compare happy path 
// 
TEST_FUNCTION(string_compare__success_same)
{
    static const char* k_val_valid = "1";
    static const char* k_to_valid = "1";
    int32_t result;

    // arrange 

    // act 
    result = string_compare(k_val_valid, k_to_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, 0, result);
}

// 
// Test string_compare happy path 
// 
TEST_FUNCTION(string_compare__success_notsame)
{
    static const char* k_val_valid = "1";
    static const char* k_to_valid = "2";
    int32_t result;

    // arrange 

    // act 
    result = string_compare(k_val_valid, k_to_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_NOT_EQUAL(int32_t, 0, result);
}

// 
// Test string_compare passing as val argument an invalid const char* value 
// 
TEST_FUNCTION(string_compare__arg_val_invalid)
{
    int32_t result;

    // arrange 

    // act 
    result = string_compare(NULL, "1");

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_NOT_EQUAL(int32_t, 0, result);
}

// 
// Test string_compare passing as to argument an invalid const char* value 
// 
TEST_FUNCTION(string_compare__arg_to_invalid)
{
    int32_t result;

    // arrange 

    // act 
    result = string_compare("1", NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_NOT_EQUAL(int32_t, 0, result);
}

// 
// Test string_compare_nocase happy path 
// 
TEST_FUNCTION(string_compare_nocase__success)
{
    static const char* k_val_valid = "abcd";
    static const char* k_to_valid = "abcD";
    int32_t result;

    // arrange 

    // act 
    result = string_compare_nocase(k_val_valid, k_to_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, 0, result);
}

// 
// Test string_compare_nocase passing as val argument an invalid const char* value 
// 
TEST_FUNCTION(string_compare_nocase__arg_val_invalid)
{
    int32_t result;

    // arrange 

    // act 
    result = string_compare_nocase(NULL, "abcD");

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_NOT_EQUAL(int32_t, 0, result);
}

// 
// Test string_compare_nocase passing as to argument an invalid const char* value 
// 
TEST_FUNCTION(string_compare_nocase__arg_to_invalid)
{
    int32_t result;

    // arrange 

    // act 
    result = string_compare_nocase("abcD", NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_NOT_EQUAL(int32_t, 0, result);
}

// 
// Test string_starts_with_nocase happy path 
// 
TEST_FUNCTION(string_starts_with_nocase__success_nocase)
{
    static const char* k_val_valid = "aBababababaBab";
    static const char* k_find_valid = "ABaB";
    bool result;

    // arrange 

    // act 
    result = string_starts_with_nocase(k_val_valid, k_find_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_TRUE(result);
}

// 
// Test string_starts_with_nocase happy path 
// 
TEST_FUNCTION(string_starts_with_nocase__success_case)
{
    static const char* k_val_valid = "ababababababab";
    static const char* k_find_valid = "aba";
    bool result;

    // arrange 

    // act 
    result = string_starts_with_nocase(k_val_valid, k_find_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_TRUE(result);
}

// 
// Test string_starts_with_nocase happy path 
// 
TEST_FUNCTION(string_starts_with_nocase__success_case_notfound)
{
    static const char* k_val_valid = "ababababababab";
    static const char* k_find_valid = "baba";
    bool result;

    // arrange 

    // act 
    result = string_starts_with_nocase(k_val_valid, k_find_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_FALSE(result);
}

// 
// Test string_starts_with_nocase happy path 
// 
TEST_FUNCTION(string_starts_with_nocase__success_nocase_notfound)
{
    static const char* k_val_valid = "ababababababab";
    static const char* k_find_valid = "B";
    bool result;

    // arrange 

    // act 
    result = string_starts_with_nocase(k_val_valid, k_find_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_FALSE(result);
}

// 
// Test string_starts_with_nocase passing as val argument an invalid const char* value 
// 
TEST_FUNCTION(string_starts_with_nocase__arg_val_invalid)
{
    static const char* k_find_valid = "B";
    bool result;

    // arrange 

    // act 
    result = string_starts_with_nocase(NULL, k_find_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_FALSE(result);
}

// 
// Test string_starts_with_nocase passing as find argument an invalid const char* value 
// 
TEST_FUNCTION(string_starts_with_nocase__arg_find_invalid)
{
    static const char* k_val_valid = "ababababababab";
    bool result;

    // arrange 

    // act 
    result = string_starts_with_nocase(k_val_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_IS_FALSE(result);
}

// 
// Test string_remove_nocase happy path 
// 
TEST_FUNCTION(string_remove_nocase__success_nocase)
{
    static const char* k_val_valid = "ababababababab";
    static const char* k_find_valid = "B";

    strcpy(UT_MEM, k_val_valid);
    // arrange 

    // act 
    string_remove_nocase(UT_MEM, k_find_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, "aaaaaaa", UT_MEM);
}

// 
// Test string_remove_nocase happy path 
// 
TEST_FUNCTION(string_remove_nocase__success_case)
{
    static const char* k_val_valid = "ababababababab";
    static const char* k_find_valid = "a";

    strcpy(UT_MEM, k_val_valid);
    // arrange 

    // act 
    string_remove_nocase(UT_MEM, k_find_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, "bbbbbbb", UT_MEM);
}

// 
// Test string_remove_nocase happy path 
// 
TEST_FUNCTION(string_remove_nocase__success_notfound)
{
    static const char* k_val_valid = "ababababababab";
    static const char* k_find_valid = "c";

    strcpy(UT_MEM, k_val_valid);
    // arrange 

    // act 
    string_remove_nocase(UT_MEM, k_find_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, k_val_valid, UT_MEM);
}

// 
// Test string_remove_nocase passing as value argument an invalid char* value 
// 
TEST_FUNCTION(string_remove_nocase__arg_value_invalid)
{
    static const char* k_find_valid = "cab";

    // arrange 

    // act 
    string_remove_nocase(NULL, k_find_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test string_remove_nocase passing as to_remove argument an invalid const char* value 
// 
TEST_FUNCTION(string_remove_nocase__arg_to_remove_invalid)
{
    static const char* k_val_valid = "ababababababab";

    strcpy(UT_MEM, k_val_valid);
    // arrange 

    // act 
    string_remove_nocase(UT_MEM, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test string_find happy path 
// 
TEST_FUNCTION(string_find__success)
{
    static const char* k_value_valid = "1234567890";
    static const char* k_to_find_valid = "345";
    const char* result;

    // arrange 

    // act 
    result = string_find(k_value_valid, k_to_find_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, "34567890", result);
}

// 
// Test string_find happy path 
// 
TEST_FUNCTION(string_find__success_notfound)
{
    static const char* k_value_valid = "1234567890";
    static const char* k_to_find_valid = "54";
    const char* result;

    // arrange 

    // act 
    result = string_find(k_value_valid, k_to_find_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test string_find passing as value argument an invalid const char* value 
// 
TEST_FUNCTION(string_find__arg_value_invalid)
{
    static const char* k_to_find_valid = "345";
    const char* result;

    // arrange 

    // act 
    result = string_find(NULL, k_to_find_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test string_find passing as to_find argument an invalid const char* value 
// 
TEST_FUNCTION(string_find__arg_to_find_invalid)
{
    static const char* k_value_valid = "1234567890";
    const char* result;

    // arrange 

    // act 
    result = string_find(k_value_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test string_find_nocase happy path 
// 
TEST_FUNCTION(string_find_nocase__success_case)
{
    static const char* k_value_valid = "abababcdefg";
    static const char* k_to_find_valid = "abc";
    const char* result;

    // arrange 

    // act 
    result = string_find_nocase(k_value_valid, k_to_find_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, "abcdefg", result);
}

// 
// Test string_find_nocase happy path 
// 
TEST_FUNCTION(string_find_nocase__success_nocase)
{
    static const char* k_value_valid = "abababcdefg";
    static const char* k_to_find_valid = "AbC";
    const char* result;

    // arrange 

    // act 
    result = string_find_nocase(k_value_valid, k_to_find_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, "abcdefg", result);
}

// 
// Test string_find_nocase passing as value argument an invalid const char* value 
// 
TEST_FUNCTION(string_find_nocase__arg_value_invalid)
{
    static const char* k_to_find_valid = "AbC";
    const char* result;

    // arrange 

    // act 
    result = string_find_nocase(NULL, k_to_find_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test string_find_nocase passing as to_find argument an invalid const char* value 
// 
TEST_FUNCTION(string_find_nocase__arg_to_find_invalid)
{
    static const char* k_value_valid = "abababcdefg";
    const char* result;

    // arrange 

    // act 
    result = string_find_nocase(k_value_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test string_trim_front happy path 
// 
TEST_FUNCTION(string_trim_front__success)
{
    static const char* k_val_valid = "abcdabcdabcdefgabcd";
    static const char* k_trim_chars_valid = "dcba";
    const char* result;

    // arrange 

    // act 
    result = string_trim_front(k_val_valid, k_trim_chars_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, "efgabcd", result);
}

// 
// Test string_trim_front happy path 
// 
TEST_FUNCTION(string_trim_front__success_notfound)
{
    static const char* k_val_valid = "eabcdabcdabcdefgabcd";
    static const char* k_trim_chars_valid = "dcba";
    const char* result;

    // arrange 

    // act 
    result = string_trim_front(k_val_valid, k_trim_chars_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, k_val_valid, result);
}

// 
// Test string_trim_front passing as val argument an invalid const char* value 
// 
TEST_FUNCTION(string_trim_front__arg_val_invalid)
{
    static const char* k_trim_chars_valid = "dcba";
    const char* result;

    // arrange 

    // act 
    result = string_trim_front(NULL, k_trim_chars_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test string_trim_front passing as trim_chars argument an invalid const char* value 
// 
TEST_FUNCTION(string_trim_front__arg_trim_chars_invalid)
{
    static const char* k_val_valid = "abcdabcdabcdefgabcd";
    const char* result;

    // arrange 

    // act 
    result = string_trim_front(k_val_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, k_val_valid, result);
}

// 
// Test string_trim_back happy path 
// 
TEST_FUNCTION(string_trim_back__success)
{
    static const char* k_val_valid = "eabcdabcdabcdefgabcd";
    static const char* k_trim_chars_valid = "dcba";

    strcpy(UT_MEM, k_val_valid);

    // arrange 

    // act 
    string_trim_back(UT_MEM, k_trim_chars_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, "eabcdabcdabcdefg", UT_MEM);
}

// 
// Test string_trim_back happy path 
// 
TEST_FUNCTION(string_trim_back__success_not_found)
{
    static const char* k_val_valid = "eabcdabcdabcdefgabcde";
    static const char* k_trim_chars_valid = "dcba";

    strcpy(UT_MEM, k_val_valid);

    // arrange 

    // act 
    string_trim_back(UT_MEM, k_trim_chars_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, k_val_valid, UT_MEM);
}

// 
// Test string_trim_back passing as val argument an invalid char* value 
// 
TEST_FUNCTION(string_trim_back__arg_val_invalid)
{
    static const char* k_trim_chars_valid = "dcba";

    // arrange 

    // act 
    string_trim_back(NULL, k_trim_chars_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test string_trim_back passing as trim_chars argument an invalid const char* value 
// 
TEST_FUNCTION(string_trim_back__arg_trim_chars_invalid)
{
    static const char* k_val_valid = "eabcdabcdabcdefgabcde";

    strcpy(UT_MEM, k_val_valid);

    // arrange 

    // act 
    string_trim_back(UT_MEM, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, k_val_valid, UT_MEM);
}

// 
// Test string_trim happy path 
// 
TEST_FUNCTION(string_trim__success)
{
    static const char* k_val_valid = "abcdabcdabcdefgabcd";
    static const char* k_trim_chars_valid = "dcba";
    const char* result;

    strcpy(UT_MEM, k_val_valid);

    // arrange 

    // act 
    result = string_trim(UT_MEM, k_trim_chars_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, "efg", result);
}

// 
// Test string_trim happy path 
// 
TEST_FUNCTION(string_trim__success_not_found)
{
    static const char* k_val_valid = "eabcdabcdabcdefgabcde";
    static const char* k_trim_chars_valid = "dcba";
    const char* result;

    strcpy(UT_MEM, k_val_valid);

    // arrange 

    // act 
    result = string_trim(UT_MEM, k_trim_chars_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, k_val_valid, result);
}

// 
// Test string_trim passing as val argument an invalid char* value 
// 
TEST_FUNCTION(string_trim__arg_val_invalid)
{
    static const char* k_trim_chars_valid = "dcba";
    const char* result;

    // arrange 

    // act 
    result = string_trim(NULL, k_trim_chars_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, NULL, result);
}

// 
// Test string_trim passing as trim_chars argument an invalid const char* value 
// 
TEST_FUNCTION(string_trim__arg_trim_chars_invalid)
{
    static const char* k_val_valid = "abcdabcdabcdefgabcd";
    const char* result;

    strcpy(UT_MEM, k_val_valid);

    // arrange 

    // act 
    result = string_trim(UT_MEM, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, k_val_valid, result);
}

//
// Test helper
//
static int32_t string_key_value_parser__success_accum(
    void* ctx,
    const char* key,
    size_t key_len,
    const char* val,
    size_t val_len
)
{
    (void)key;
    (void)key_len;
    strncat((char*)ctx, val, val_len);
    return er_ok;
}

// 
// Test string_key_value_parser happy path 
// 
TEST_FUNCTION(string_key_value_parser__success_1)
{
    static const char* k_connection_string_valid = "one=1;two=2;three=3";
    static const fn_parse_cb_t k_visitor_valid = string_key_value_parser__success_accum;
    static const char k_delim_valid = ';';
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 

    // act 
    result = string_key_value_parser(k_connection_string_valid, k_visitor_valid, k_delim_valid, UT_MEM);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(char_ptr, "123", UT_MEM);
}

// 
// Test string_key_value_parser happy path 
// 
TEST_FUNCTION(string_key_value_parser__success_2)
{
    static const char* k_connection_string_valid = "one=1&&two=2&three=3&";
    static const fn_parse_cb_t k_visitor_valid = string_key_value_parser__success_accum;
    static const char k_delim_valid = '&';
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 

    // act 
    result = string_key_value_parser(k_connection_string_valid, k_visitor_valid, k_delim_valid, UT_MEM);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(char_ptr, "123", UT_MEM);
}

// 
// Test string_key_value_parser happy path 
// 
TEST_FUNCTION(string_key_value_parser__success_3)
{
    static const char* k_connection_string_valid = "one=1";
    static const fn_parse_cb_t k_visitor_valid = string_key_value_parser__success_accum;
    static const char k_delim_valid = ' ';
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 

    // act 
    result = string_key_value_parser(k_connection_string_valid, k_visitor_valid, k_delim_valid, UT_MEM);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(char_ptr, "1", UT_MEM);
}

// 
// Test string_key_value_parser happy path 
// 
TEST_FUNCTION(string_key_value_parser__success_4)
{
    static const char* k_connection_string_valid = "one=;two=;three=3;";
    static const fn_parse_cb_t k_visitor_valid = string_key_value_parser__success_accum;
    static const char k_delim_valid = ';';
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 

    // act 
    result = string_key_value_parser(k_connection_string_valid, k_visitor_valid, k_delim_valid, UT_MEM);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(char_ptr, "3", UT_MEM);
}

// 
// Test string_key_value_parser happy path 
// 
TEST_FUNCTION(string_key_value_parser__success_5)
{
    static const char* k_connection_string_valid = "one;two;three=3;";
    static const fn_parse_cb_t k_visitor_valid = string_key_value_parser__success_accum;
    static const char k_delim_valid = ';';
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 

    // act 
    result = string_key_value_parser(k_connection_string_valid, k_visitor_valid, k_delim_valid, UT_MEM);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(char_ptr, "3", UT_MEM);
}

// 
// Test string_key_value_parser happy path 
// 
TEST_FUNCTION(string_key_value_parser__success_6)
{
    static const char* k_connection_string_valid = "one=;two=;three=;";
    static const fn_parse_cb_t k_visitor_valid = string_key_value_parser__success_accum;
    static const char k_delim_valid = ';';
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 

    // act 
    result = string_key_value_parser(k_connection_string_valid, k_visitor_valid, k_delim_valid, UT_MEM);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(char_ptr, "", UT_MEM);
}

// 
// Test string_key_value_parser happy path 
// 
TEST_FUNCTION(string_key_value_parser__success_7)
{
    static const char* k_connection_string_valid = "one;two;three=;";
    static const fn_parse_cb_t k_visitor_valid = string_key_value_parser__success_accum;
    static const char k_delim_valid = ';';
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 

    // act 
    result = string_key_value_parser(k_connection_string_valid, k_visitor_valid, k_delim_valid, UT_MEM);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(char_ptr, "", UT_MEM);
}

// 
// Test string_key_value_parser passing as connection_string argument an invalid const char* value 
// 
TEST_FUNCTION(string_key_value_parser__arg_connection_string_invalid)
{
    static const fn_parse_cb_t k_visitor_valid = string_key_value_parser__success_accum;
    static const char k_delim_valid = ';';
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 

    // act 
    result = string_key_value_parser(NULL, k_visitor_valid, k_delim_valid, UT_MEM);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test string_key_value_parser passing as visitor argument an invalid fn_parse_cb_t value 
// 
TEST_FUNCTION(string_key_value_parser__arg_visitor_invalid)
{
    static const char* k_connection_string_valid = "one=1;two=2;three=3";
    static const char k_delim_valid = ';';
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 

    // act 
    result = string_key_value_parser(k_connection_string_valid, NULL, k_delim_valid, UT_MEM);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test string_base16_to_byte_array happy path 
// 
TEST_FUNCTION(string_base16_to_byte_array__success)
{
    static const char* k_val_valid = "7d45f092";
    static const uint8_t k_buffer_valid[] = { 0x7d, 0x45, 0xf0, 0x92 };
    uint8_t* buffer_valid = NULL;
    size_t len_valid = 0;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(4, NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);

    // act 
    result = string_base16_to_byte_array(k_val_valid, &buffer_valid, &len_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(int32_t, (int32_t)len_valid, 4);
    ASSERT_IS_TRUE(0 == memcmp(buffer_valid, k_buffer_valid, sizeof(k_buffer_valid)));
}

// 
// Test string_base16_to_byte_array passing as val argument an invalid const char* value 
// 
TEST_FUNCTION(string_base16_to_byte_array__arg_val_null)
{
    uint8_t* buffer_valid = NULL;
    size_t len_valid = 0;
    int32_t result;

    // arrange 

    // act 
    result = string_base16_to_byte_array(NULL, &buffer_valid, &len_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test string_base16_to_byte_array passing as val argument an invalid const char* value 
// 
TEST_FUNCTION(string_base16_to_byte_array__arg_val_invalid)
{
    static const char* k_val_invalid = "XYZB";
    uint8_t* buffer_valid = NULL;
    size_t len_valid = 0;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(2, NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(h_free((void*)UT_MEM, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    result = string_base16_to_byte_array(k_val_invalid, &buffer_valid, &len_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_invalid_format, result);
}

// 
// Test string_base16_to_byte_array passing as val argument an invalid const char* value 
// 
TEST_FUNCTION(string_base16_to_byte_array__arg_val_empty)
{
    static const char* k_val_invalid = "";
    uint8_t* buffer_valid = NULL;
    size_t len_valid = 0;
    int32_t result;

    // arrange 

    // act 
    result = string_base16_to_byte_array(k_val_invalid, &buffer_valid, &len_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_arg, result);
}

// 
// Test string_base16_to_byte_array passing as buffer argument an invalid uint8_t** value 
// 
TEST_FUNCTION(string_base16_to_byte_array__arg_buffer_invalid)
{
    static const char* k_val_valid = "7d45f092";
    size_t len_valid = 0;
    int32_t result;

    // arrange 

    // act 
    result = string_base16_to_byte_array(k_val_valid, NULL, &len_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test string_base16_to_byte_array passing as len argument an invalid size_t* value 
// 
TEST_FUNCTION(string_base16_to_byte_array__arg_len_invalid)
{
    static const char* k_val_valid = "7d45f092";
    uint8_t* buffer_valid = NULL;
    int32_t result;

    // arrange 

    // act 
    result = string_base16_to_byte_array(k_val_valid, &buffer_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test string_base16_to_byte_array unhappy path 
// 
TEST_FUNCTION(string_base16_to_byte_array__neg)
{
    static const char* k_val_valid = "7d45f092";
    uint8_t* buffer_valid = NULL;
    size_t len_valid = 0;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(4, NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn(NULL);

    // act 
    result = string_base16_to_byte_array(k_val_valid, &buffer_valid, &len_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_out_of_memory, result);
}

// 
// Test string_to_uuid happy path 
// 
TEST_FUNCTION(string_to_uuid__success_1)
{
    static const char* k_string_valid = "73772566-59a3-7d45-92f0-29411b079dec";
    static const uint8_t k_uuid_valid[] = { 0x66, 0x25, 0x77, 0x73, 0xa3, 0x59, 0x45, 0x7d, 0x92, 0xf0, 0x29, 0x41, 0x1b, 0x7, 0x9d, 0xec };
    uint8_t* uuid_valid = (uint8_t*)UT_MEM;
    int32_t result;

    // arrange 

    // act 
    result = string_to_uuid(k_string_valid, uuid_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_IS_TRUE(!memcmp(k_uuid_valid, uuid_valid, 16));
}

// 
// Test string_to_uuid happy path 
// 
TEST_FUNCTION(string_to_uuid__success_2)
{
    static const char* k_string_valid = "73772566-59a3-7d45-92f0-29411b079dec";
    static const uint8_t k_uuid_valid[] = { 0x66, 0x25, 0x77, 0x73, 0xa3, 0x59, 0x45, 0x7d, 0x92, 0xf0, 0x29, 0x41, 0x1b, 0x7, 0x9d, 0xec };
    uint8_t* uuid_valid = (uint8_t*)UT_MEM;
    int32_t result;

    // arrange 

    // act 
    result = string_to_uuid(k_string_valid, uuid_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_IS_TRUE(!memcmp(k_uuid_valid, uuid_valid, 16));
}

// 
// Test string_to_uuid passing as string argument an invalid const char* value 
// 
TEST_FUNCTION(string_to_uuid__arg_string_null)
{
    uint8_t* uuid_valid = (uint8_t*)UT_MEM;
    int32_t result;

    // arrange 

    // act 
    result = string_to_uuid(NULL, uuid_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test string_to_uuid passing as uuid argument an invalid pal_uuid_t value 
// 
TEST_FUNCTION(string_to_uuid__arg_uuid_null)
{
    static const char* k_string_valid = "73772566-59a3-7d45-92f0-29411b079dec";
    int32_t result;

    // arrange 

    // act 
    result = string_to_uuid(k_string_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test string_to_uuid unhappy path 
// 
TEST_FUNCTION(string_to_uuid__neg_1)
{
    static const char* k_string_valid = "6657773-a359-457d-92f0-29411b079deca";
    uint8_t* uuid_valid = (uint8_t*)UT_MEM;
    int32_t result;

    // arrange 

    // act 
    result = string_to_uuid(k_string_valid, uuid_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_invalid_format, result);
}

// 
// Test string_to_uuid unhappy path 
// 
TEST_FUNCTION(string_to_uuid__neg_2)
{
    static const char* k_string_valid = "66257773-a359-457d-92f029411b079deca";
    uint8_t* uuid_valid = (uint8_t*)UT_MEM;
    int32_t result;

    // arrange 

    // act 
    result = string_to_uuid(k_string_valid, uuid_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_invalid_format, result);
}

// 
// Test string_to_uuid unhappy path 
// 
TEST_FUNCTION(string_to_uuid__neg_3)
{
    static const char* k_string_valid = "66257773-a359-457d-92f0-29411b079de";
    uint8_t* uuid_valid = (uint8_t*)UT_MEM;
    int32_t result;

    // arrange 

    // act 
    result = string_to_uuid(k_string_valid, uuid_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_arg, result);
}

// 
// Test string_to_uuid unhappy path 
// 
TEST_FUNCTION(string_to_uuid__neg_4)
{
    static const char* k_string_valid = "";
    uint8_t* uuid_valid = (uint8_t*)UT_MEM;
    int32_t result;

    // arrange 

    // act 
    result = string_to_uuid(k_string_valid, uuid_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_arg, result);
}

// 
// Test string_to_uuid unhappy path 
// 
TEST_FUNCTION(string_to_uuid__neg_5)
{
    static const char* k_string_valid = "66257773a-a359-457d-92f0-29411b079dec";
    uint8_t* uuid_valid = (uint8_t*)UT_MEM;
    int32_t result;

    // arrange 

    // act 
    result = string_to_uuid(k_string_valid, uuid_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_invalid_format, result);
}

// 
// Test string_to_uuid unhappy path 
// 
TEST_FUNCTION(string_to_uuid__neg_6)
{
    static const char* k_string_valid = "66257773-g359-457d-92f0-29411b079dec";
    uint8_t* uuid_valid = (uint8_t*)UT_MEM;
    int32_t result;

    // arrange 

    // act 
    result = string_to_uuid(k_string_valid, uuid_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_invalid_format, result);
}

// 
// Test string_to_uuid unhappy path 
// 
TEST_FUNCTION(string_to_uuid__neg_7)
{
    static const char* k_string_valid = "{73772566-59a3-7d45-92f0-29411b079dec}";
    uint8_t* uuid_valid = (uint8_t*)UT_MEM;
    int32_t result;

    // arrange 

    // act 
    result = string_to_uuid(k_string_valid, uuid_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_invalid_format, result);
}

// 
// Test string_from_uuid happy path 
// 
TEST_FUNCTION(string_from_uuid__success_1)
{
    static const char* k_string_valid = "73772566-59a3-7d45-92f0-29411b079dec";
    static const uint8_t k_uuid_valid[] = { 0x66, 0x25, 0x77, 0x73, 0xa3, 0x59, 0x45, 0x7d, 0x92, 0xf0, 0x29, 0x41, 0x1b, 0x7, 0x9d, 0xec };
    char* string_valid = UT_MEM;
    int32_t result;

    // arrange 

    // act 
    result = string_from_uuid(k_uuid_valid, string_valid, 37);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, k_string_valid, string_valid);
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test string_from_uuid happy path 
// 
TEST_FUNCTION(string_from_uuid__success_2)
{
    static const char* k_string_valid = "73772566-59a3-7d45-92f0-29411b079dec";
    static const uint8_t k_uuid_valid[] = { 0x66, 0x25, 0x77, 0x73, 0xa3, 0x59, 0x45, 0x7d, 0x92, 0xf0, 0x29, 0x41, 0x1b, 0x7, 0x9d, 0xec };
    char* string_valid = UT_MEM;
    int32_t result;

    // arrange 

    // act 
    result = string_from_uuid(k_uuid_valid, string_valid, 100);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(char_ptr, k_string_valid, string_valid);
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test string_from_uuid passing as uuid argument an invalid pal_uuid_t value 
// 
TEST_FUNCTION(string_from_uuid__arg_uuid_null)
{
    char* k_string_valid = UT_MEM;
    int32_t result;

    // arrange 

    // act 
    result = string_from_uuid(NULL, k_string_valid, 88);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test string_from_uuid passing as string argument an invalid char* value 
// 
TEST_FUNCTION(string_from_uuid__arg_string_null)
{
    static const uint8_t k_uuid_valid[] = { 0x66, 0x25, 0x77, 0x73, 0xa3, 0x59, 0x45, 0x7d, 0x92, 0xf0, 0x29, 0x41, 0x1b, 0x7, 0x9d, 0xec };
    int32_t result;

    // arrange 

    // act 
    result = string_from_uuid(k_uuid_valid, NULL, 88);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test string_from_uuid passing invalid len
// 
TEST_FUNCTION(string_from_uuid__arg_len_invalid_1)
{
    static const uint8_t k_uuid_valid[] = { 0x66, 0x25, 0x77, 0x73, 0xa3, 0x59, 0x45, 0x7d, 0x92, 0xf0, 0x29, 0x41, 0x1b, 0x7, 0x9d, 0xec };
    char* k_string_valid = UT_MEM;
    int32_t result;

    // arrange 

    // act 
    result = string_from_uuid(k_uuid_valid, k_string_valid, 0);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_arg, result);
}

// 
// Test string_from_uuid passing invalid len
// 
TEST_FUNCTION(string_from_uuid__arg_len_invalid_2)
{
    static const uint8_t k_uuid_valid[] = { 0x66, 0x25, 0x77, 0x73, 0xa3, 0x59, 0x45, 0x7d, 0x92, 0xf0, 0x29, 0x41, 0x1b, 0x7, 0x9d, 0xec };
    char* k_string_valid = UT_MEM;
    int32_t result;

    // arrange 

    // act 
    result = string_from_uuid(k_uuid_valid, k_string_valid, 36);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_arg, result);
}

// 
// Test string_parse_service_full_name happy path 
// 
TEST_FUNCTION(string_parse_service_full_name__success_0)
{
    char* service_name_valid;
    char* service_type_valid;
    char* domain_valid;
    int32_t result;

    strcpy(UT_MEM, "test._super._tcp._something.domain.com");
    
    // arrange 

    // act 
    result = string_parse_service_full_name(UT_MEM, &service_name_valid, &service_type_valid, &domain_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(char_ptr, "test", service_name_valid);
    ASSERT_ARE_EQUAL(char_ptr, "_super._tcp._something", service_type_valid);
    ASSERT_ARE_EQUAL(char_ptr, "domain.com", domain_valid);
}

// 
// Test string_parse_service_full_name happy path 
// 
TEST_FUNCTION(string_parse_service_full_name__success_1)
{
    char* service_name_valid;
    char* service_type_valid;
    char* domain_valid;
    int32_t result;

    strcpy(UT_MEM, "_super._tcp._something");

    // arrange 

    // act 
    result = string_parse_service_full_name(UT_MEM, &service_name_valid, &service_type_valid, &domain_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_IS_NULL(service_name_valid);
    ASSERT_ARE_EQUAL(char_ptr, "_super._tcp._something", service_type_valid);
    ASSERT_ARE_EQUAL(char_ptr, "local", domain_valid);
}

// 
// Test string_parse_service_full_name happy path 
// 
TEST_FUNCTION(string_parse_service_full_name__success_2)
{
    char* service_name_valid;
    char* service_type_valid;
    char* domain_valid;
    int32_t result;

    strcpy(UT_MEM, "test.._super._tcp._something.local.");

    // arrange 

    // act 
    result = string_parse_service_full_name(UT_MEM, &service_name_valid, &service_type_valid, &domain_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(char_ptr, "test.", service_name_valid);
    ASSERT_ARE_EQUAL(char_ptr, "_super._tcp._something", service_type_valid);
    ASSERT_ARE_EQUAL(char_ptr, "local", domain_valid);
}

// 
// Test string_parse_service_full_name happy path 
// 
TEST_FUNCTION(string_parse_service_full_name__success_3)
{
    char* service_name_valid;
    char* service_type_valid;
    char* domain_valid;
    int32_t result;

    strcpy(UT_MEM, "test.test.._super._tcp._something.");

    // arrange 

    // act 
    result = string_parse_service_full_name(UT_MEM, &service_name_valid, &service_type_valid, &domain_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(char_ptr, "test.test.", service_name_valid);
    ASSERT_ARE_EQUAL(char_ptr, "_super._tcp._something", service_type_valid);
    ASSERT_ARE_EQUAL(char_ptr, "local", domain_valid);
}

// 
// Test string_parse_service_full_name happy path 
// 
TEST_FUNCTION(string_parse_service_full_name__success_4)
{
    char* service_name_valid;
    char* service_type_valid;
    char* domain_valid;
    int32_t result;

    strcpy(UT_MEM, "test.test.something.");

    // arrange 

    // act 
    result = string_parse_service_full_name(UT_MEM, &service_name_valid, &service_type_valid, &domain_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_IS_NULL(service_name_valid);
    ASSERT_IS_NULL(service_type_valid);
    ASSERT_ARE_EQUAL(char_ptr, "test.test.something", domain_valid);
}

// 
// Test string_parse_service_full_name happy path 
// 
TEST_FUNCTION(string_parse_service_full_name__success_5)
{
    char* service_name_valid;
    char* service_type_valid;
    char* domain_valid;
    int32_t result;

    strcpy(UT_MEM, "");

    // arrange 

    // act 
    result = string_parse_service_full_name(UT_MEM, &service_name_valid, &service_type_valid, &domain_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_IS_NULL(service_name_valid);
    ASSERT_IS_NULL(service_type_valid);
    ASSERT_IS_NULL(domain_valid);
}

// 
// Test string_parse_service_full_name happy path 
// 
TEST_FUNCTION(string_parse_service_full_name__success_6)
{
    char* service_name_valid;
    char* service_type_valid;
    char* domain_valid;
    int32_t result;

    strcpy(UT_MEM, "_test.test._something.local");

    // arrange 

    // act 
    result = string_parse_service_full_name(UT_MEM, &service_name_valid, &service_type_valid, &domain_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_IS_NULL(service_name_valid);
    ASSERT_ARE_EQUAL(char_ptr, "_test", service_type_valid);
    ASSERT_ARE_EQUAL(char_ptr, "test._something.local", domain_valid);
}

// 
// Test string_parse_service_full_name happy path 
// 
TEST_FUNCTION(string_parse_service_full_name__success_7)
{
    char* service_name_valid;
    char* service_type_valid;
    char* domain_valid;
    int32_t result;

    strcpy(UT_MEM, "[ some string. . . without 124:240x34:35 ]     ._super._tcp._something");

    // arrange 

    // act 
    result = string_parse_service_full_name(UT_MEM, &service_name_valid, &service_type_valid, &domain_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(char_ptr, "[ some string. . . without 124:240x34:35 ]     ", service_name_valid);
    ASSERT_ARE_EQUAL(char_ptr, "_super._tcp._something", service_type_valid);
    ASSERT_ARE_EQUAL(char_ptr, "local", domain_valid);
}

// 
// Test string_parse_service_full_name passing as full_name argument an invalid char* value 
// 
TEST_FUNCTION(string_parse_service_full_name__arg_full_name_invalid)
{
    char* service_name_valid;
    char* service_type_valid;
    char* domain_valid;
    int32_t result;

    // arrange 

    // act 
    result = string_parse_service_full_name(NULL, &service_name_valid, &service_type_valid, &domain_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test string_parse_service_full_name passing as service_name argument an invalid char** value 
// 
TEST_FUNCTION(string_parse_service_full_name__arg_service_name_invalid)
{
    char* service_type_valid;
    char* domain_valid;
    int32_t result;

    strcpy(UT_MEM, "_test.test._something.local");

    // arrange 

    // act 
    result = string_parse_service_full_name(UT_MEM, NULL, &service_type_valid, &domain_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test string_parse_service_full_name passing as service_type argument an invalid char** value 
// 
TEST_FUNCTION(string_parse_service_full_name__arg_service_type_invalid)
{
    char* service_name_valid;
    char* domain_valid;
    int32_t result;

    strcpy(UT_MEM, "_test.test._something.local");

    // arrange 

    // act 
    result = string_parse_service_full_name(UT_MEM, &service_name_valid, NULL, &domain_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test string_parse_service_full_name passing as domain argument an invalid char** value 
// 
TEST_FUNCTION(string_parse_service_full_name__arg_domain_invalid)
{
    char* service_name_valid;
    char* service_type_valid;
    int32_t result;

    strcpy(UT_MEM, "_test.test._something.local");

    // arrange 

    // act 
    result = string_parse_service_full_name(UT_MEM, &service_name_valid, &service_type_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test string_copy_service_full_name happy path 
// 
TEST_FUNCTION(string_copy_service_full_name__success_0)
{
    static const char* k_service_name_valid = "test with spaces.testwithout [on 123.]";
    static const char* k_service_type_valid = "_xyp._tcp";
    static const char* k_domain_valid = "domain.com";
    static const size_t k_full_size_valid = 256;
    char* full_name_valid = UT_MEM;
    int32_t result;

    // arrange 

    // act 
    result = string_copy_service_full_name(k_service_name_valid, k_service_type_valid, k_domain_valid, full_name_valid, k_full_size_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(char_ptr, "test with spaces.testwithout [on 123.]._xyp._tcp.domain.com", full_name_valid);
}

// 
// Test string_copy_service_full_name happy path 
// 
TEST_FUNCTION(string_copy_service_full_name__success_1)
{
    static const char* k_service_type_valid = "_xyp._tcp";
    static const char* k_domain_valid = "domain.com";
    static const size_t k_full_size_valid = 256;
    char* full_name_valid = UT_MEM;
    int32_t result;

    // arrange 

    // act 
    result = string_copy_service_full_name(NULL, k_service_type_valid, k_domain_valid, full_name_valid, k_full_size_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(char_ptr, "_xyp._tcp.domain.com", full_name_valid);
}

// 
// Test string_copy_service_full_name happy path 
// 
TEST_FUNCTION(string_copy_service_full_name__success_2)
{
    static const char* k_service_name_valid = "test.test";
    static const char* k_service_type_valid = "_xyp._tcp";
    static const size_t k_full_size_valid = 256;
    char* full_name_valid = UT_MEM;
    int32_t result;

    // arrange 

    // act 
    result = string_copy_service_full_name(k_service_name_valid, k_service_type_valid, NULL, full_name_valid, k_full_size_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(char_ptr, "test.test._xyp._tcp.local", full_name_valid);
}

// 
// Test string_copy_service_full_name happy path 
// 
TEST_FUNCTION(string_copy_service_full_name__success_3)
{
    static const char* k_service_type_valid = "_xyp._tcp";
    static const size_t k_full_size_valid = 256;
    char* full_name_valid = UT_MEM;
    int32_t result;

    // arrange 

    // act 
    result = string_copy_service_full_name(NULL, k_service_type_valid, NULL, full_name_valid, k_full_size_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(char_ptr, "_xyp._tcp.local", full_name_valid);
}

// 
// Test string_copy_service_full_name happy path 
// 
TEST_FUNCTION(string_copy_service_full_name__success_4)
{
    static const char* k_service_name_valid = "test..";
    static const char* k_service_type_valid = "_xyp._tcp.";
    static const char* k_domain_valid = "domain.com.";
    static const size_t k_full_size_valid = 256;
    char* full_name_valid = UT_MEM;
    int32_t result;

    // arrange 

    // act 
    result = string_copy_service_full_name(k_service_name_valid, k_service_type_valid, k_domain_valid, full_name_valid, k_full_size_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(char_ptr, "test..._xyp._tcp.domain.com", full_name_valid);
}

// 
// Test string_copy_service_full_name passing as service_type argument an invalid const char* value 
// 
TEST_FUNCTION(string_copy_service_full_name__arg_service_type_invalid)
{
    static const char* k_service_name_valid = "test with spaces.testwithout [on 123.]";
    static const char* k_domain_valid = "domain.com";
    static const size_t k_full_size_valid = 256;
    char* full_name_valid = UT_MEM;
    int32_t result;

    // arrange 

    // act 
    result = string_copy_service_full_name(k_service_name_valid, NULL, k_domain_valid, full_name_valid, k_full_size_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_arg, result);
}

// 
// Test string_copy_service_full_name passing as full_name argument an invalid char* value 
// 
TEST_FUNCTION(string_copy_service_full_name__arg_full_name_invalid)
{
    static const char* k_service_name_valid = "test with spaces.testwithout [on 123.]";
    static const char* k_service_type_valid = "_xyp._tcp";
    static const char* k_domain_valid = "domain.com";
    static const size_t k_full_size_valid = 256;
    int32_t result;

    // arrange 

    // act 
    result = string_copy_service_full_name(k_service_name_valid, k_service_type_valid, k_domain_valid, NULL, k_full_size_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test string_copy_service_full_name passing as full_size argument an invalid size_t value 
// 
TEST_FUNCTION(string_copy_service_full_name__arg_full_size_invalid)
{
    static const char* k_service_name_valid = "test with spaces.testwithout [on 123.]";
    static const char* k_service_type_valid = "_xyp._tcp";
    static const char* k_domain_valid = "domain.com";
    static const size_t k_full_size_invalid = 1;
    char* full_name_valid = UT_MEM;
    int32_t result;

    // arrange 

    // act 
    result = string_copy_service_full_name(k_service_name_valid, k_service_type_valid, k_domain_valid, full_name_valid, k_full_size_invalid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_arg, result);
}

























// 
// Test string_parse_range_list happy path 
// 
TEST_FUNCTION(string_parse_range_list__success_0)
{
    static const char* k_range_list_in = "0-10;11-12;14";
    static const int32_t k_expected[6] = { 0, 10, 11, 12, 14, 14 };
    int32_t* range_tuples_valid;
    size_t range_tuple_count_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(k_expected), NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);

    // act 
    result = string_parse_range_list(k_range_list_in, &range_tuples_valid, &range_tuple_count_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(size_t, _countof(k_expected) / 2, range_tuple_count_valid);
    ASSERT_IS_TRUE(0 == memcmp(k_expected, range_tuples_valid, sizeof(k_expected)));
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test string_parse_range_list happy path 
// 
TEST_FUNCTION(string_parse_range_list__success_1)
{
    static const char* k_range_list_in = "0";
    static const int32_t k_expected[2] = { 0, 0 };
    int32_t* range_tuples_valid;
    size_t range_tuple_count_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(k_expected), NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);

    // act 
    result = string_parse_range_list(k_range_list_in, &range_tuples_valid, &range_tuple_count_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(size_t, _countof(k_expected) / 2, range_tuple_count_valid);
    ASSERT_IS_TRUE(0 == memcmp(k_expected, range_tuples_valid, sizeof(k_expected)));
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test string_parse_range_list happy path 
// 
TEST_FUNCTION(string_parse_range_list__success_2)
{
    static const char* k_range_list_in = "10000;10002;1;2;";
    static const int32_t k_expected[8] = { 10000, 10000, 10002, 10002, 1, 1, 2, 2 };
    int32_t* range_tuples_valid;
    size_t range_tuple_count_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(k_expected), NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);

    // act 
    result = string_parse_range_list(k_range_list_in, &range_tuples_valid, &range_tuple_count_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(size_t, _countof(k_expected) / 2, range_tuple_count_valid);
    ASSERT_IS_TRUE(0 == memcmp(k_expected, range_tuples_valid, sizeof(k_expected)));
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test string_parse_range_list happy path 
// 
TEST_FUNCTION(string_parse_range_list__success_3)
{
    static const char* k_range_list_in = "10000-10002;1-2;";
    static const int32_t k_expected[4] = { 10000, 10002, 1, 2 };
    int32_t* range_tuples_valid;
    size_t range_tuple_count_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(k_expected), NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);

    // act 
    result = string_parse_range_list(k_range_list_in, &range_tuples_valid, &range_tuple_count_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(size_t, _countof(k_expected) / 2, range_tuple_count_valid);
    ASSERT_IS_TRUE(0 == memcmp(k_expected, range_tuples_valid, sizeof(k_expected)));
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test string_parse_range_list happy path 
// 
TEST_FUNCTION(string_parse_range_list__success_4)
{
    static const char* k_range_list_in = "";
    int32_t* range_tuples_valid;
    size_t range_tuple_count_valid;
    int32_t result;

    // arrange 

    // act 
    result = string_parse_range_list(k_range_list_in, &range_tuples_valid, &range_tuple_count_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(size_t, 0, range_tuple_count_valid);
    ASSERT_IS_NULL(range_tuples_valid);
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test string_parse_range_list happy path 
// 
TEST_FUNCTION(string_parse_range_list__success_5)
{
    static const char* k_range_list_in = "   ";
    int32_t* range_tuples_valid;
    size_t range_tuple_count_valid;
    int32_t result;

    // arrange 

    // act 
    result = string_parse_range_list(k_range_list_in, &range_tuples_valid, &range_tuple_count_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(size_t, 0, range_tuple_count_valid);
    ASSERT_IS_NULL(range_tuples_valid);
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test string_parse_range_list happy path 
// 
TEST_FUNCTION(string_parse_range_list__success_6)
{
    static const char* k_range_list_in = "10000-10002;1-2;  ";
    static const int32_t k_expected[4] = { 10000, 10002, 1, 2 };
    int32_t* range_tuples_valid;
    size_t range_tuple_count_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(k_expected), NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);

    // act 
    result = string_parse_range_list(k_range_list_in, &range_tuples_valid, &range_tuple_count_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(size_t, _countof(k_expected) / 2, range_tuple_count_valid);
    ASSERT_IS_TRUE(0 == memcmp(k_expected, range_tuples_valid, sizeof(k_expected)));
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test string_parse_range_list happy path 
// 
TEST_FUNCTION(string_parse_range_list__success_7)
{
    static const char* k_range_list_in = "10000-10002;1-2   ";
    static const int32_t k_expected[4] = { 10000, 10002, 1, 2 };
    int32_t* range_tuples_valid;
    size_t range_tuple_count_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(k_expected), NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);

    // act 
    result = string_parse_range_list(k_range_list_in, &range_tuples_valid, &range_tuple_count_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(size_t, _countof(k_expected) / 2, range_tuple_count_valid);
    ASSERT_IS_TRUE(0 == memcmp(k_expected, range_tuples_valid, sizeof(k_expected)));
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test string_parse_range_list happy path 
// 
TEST_FUNCTION(string_parse_range_list__success_8)
{
    static const char* k_range_list_in = "10002-10000;2-1";
    static const int32_t k_expected[4] = { 10000, 10002, 1, 2 };
    int32_t* range_tuples_valid;
    size_t range_tuple_count_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(k_expected), NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);

    // act 
    result = string_parse_range_list(k_range_list_in, &range_tuples_valid, &range_tuple_count_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(size_t, _countof(k_expected) / 2, range_tuple_count_valid);
    ASSERT_IS_TRUE(0 == memcmp(k_expected, range_tuples_valid, sizeof(k_expected)));
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}


// 
// Test string_parse_range_list happy path 
// 
TEST_FUNCTION(string_parse_range_list__success_validate)
{
    static const char* k_range_list_in = "0-10;11-12;14";
    int32_t result;

    // arrange 

    // act 
    result = string_parse_range_list(k_range_list_in, NULL, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test string_parse_range_list passing as range_string argument an invalid char* value 
// 
TEST_FUNCTION(string_parse_range_list__arg_range_string_invalid_0)
{
    static const char* k_range_list_in = ";;;;";
    int32_t* range_tuples_valid;
    size_t range_tuple_count_valid;
    int32_t result;

    // arrange 

    // act 
    result = string_parse_range_list(k_range_list_in, &range_tuples_valid, &range_tuple_count_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_invalid_format, result);
}

// 
// Test string_parse_range_list passing as range_string argument an invalid char* value 
// 
TEST_FUNCTION(string_parse_range_list__arg_range_string_invalid_1)
{
    static const char* k_range_list_in = ";";
    int32_t* range_tuples_valid;
    size_t range_tuple_count_valid;
    int32_t result;

    // arrange 

    // act 
    result = string_parse_range_list(k_range_list_in, &range_tuples_valid, &range_tuple_count_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_invalid_format, result);
}

// 
// Test string_parse_range_list passing as range_string argument an invalid char* value 
// 
TEST_FUNCTION(string_parse_range_list__arg_range_string_invalid_2)
{
    static const char* k_range_list_in = "fooo";
    int32_t* range_tuples_valid;
    size_t range_tuple_count_valid;
    int32_t result;

    // arrange 

    // act 
    result = string_parse_range_list(k_range_list_in, &range_tuples_valid, &range_tuple_count_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_invalid_format, result);
}

// 
// Test string_parse_range_list passing as range_string argument an invalid char* value 
// 
TEST_FUNCTION(string_parse_range_list__arg_range_string_invalid_3)
{
    static const char* k_range_list_in = "1234;444-;234";
    int32_t* range_tuples_valid;
    size_t range_tuple_count_valid;
    int32_t result;

    // arrange 

    // act 
    result = string_parse_range_list(k_range_list_in, &range_tuples_valid, &range_tuple_count_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_invalid_format, result);
}

// 
// Test string_parse_range_list passing as range_string argument an invalid char* value 
// 
TEST_FUNCTION(string_parse_range_list__arg_range_string_invalid_4)
{
    static const char* k_range_list_in = "1234 ;1-3;4";
    int32_t* range_tuples_valid;
    size_t range_tuple_count_valid;
    int32_t result;

    // arrange 

    // act 
    result = string_parse_range_list(k_range_list_in, &range_tuples_valid, &range_tuple_count_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_invalid_format, result);
}

// 
// Test string_parse_range_list passing as range_string argument a NULL char* value 
// 
TEST_FUNCTION(string_parse_range_list__arg_range_string_null)
{
    int32_t* range_tuples_valid;
    size_t range_tuple_count_valid;
    int32_t result;

    // arrange 

    // act 
    result = string_parse_range_list(NULL, &range_tuples_valid, &range_tuple_count_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test string_parse_range_list passing as range_tuples argument an invalid char** value 
// 
TEST_FUNCTION(string_parse_range_list__arg_range_tuples_invalid)
{
    size_t range_tuple_count_valid;
    int32_t result;

    strcpy(UT_MEM, "1234");

    // arrange 

    // act 
    result = string_parse_range_list(UT_MEM, NULL, &range_tuple_count_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test string_parse_range_list passing as range_tuples_count argument an invalid char** value 
// 
TEST_FUNCTION(string_parse_range_list__arg_range_tuples_count_invalid)
{
    int32_t* range_tuples_valid;
    int32_t result;

    strcpy(UT_MEM, "_test.test._something.local");

    // arrange 

    // act 
    result = string_parse_range_list(NULL, &range_tuples_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

