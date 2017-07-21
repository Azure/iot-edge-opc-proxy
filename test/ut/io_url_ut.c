// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST io_url
#include "util_ut.h"

//
// 1. Required mocks
//
#include "azure_c_shared_utility/strings.h"
#include "io_token.h"

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
REGISTER_UMOCK_ALIAS_TYPE(io_token_provider_t*, void*);
END_DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
DECLARE_TEST_SETUP()

//
// Test io_url_create happy path
//
TEST_FUNCTION(io_url_create__success)
{
    static const char* k_scheme_valid = "mqtts";
    static const char* k_host_name_valid = "testhost.com";
    static const uint16_t k_port_valid = 0;
    static const char* k_path_valid = "foobar";
    static const char* k_user_name_valid = "user1";
    static const char* k_password_valid = "pword";

    io_url_t* url_valid;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_url_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(STRING_construct(k_host_name_valid))
        .SetReturn((STRING_HANDLE)1);
    STRICT_EXPECTED_CALL(string_compare_nocase(k_scheme_valid, "wss"))
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(string_compare_nocase(k_scheme_valid, "amqps"))
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(string_compare_nocase(k_scheme_valid, "https"))
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(string_compare_nocase(k_scheme_valid, k_scheme_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_construct(k_scheme_valid))
        .SetReturn((STRING_HANDLE)2);
    STRICT_EXPECTED_CALL(STRING_construct(k_path_valid))
        .SetReturn((STRING_HANDLE)3);
    STRICT_EXPECTED_CALL(STRING_construct(k_user_name_valid))
        .SetReturn((STRING_HANDLE)4);
    STRICT_EXPECTED_CALL(STRING_construct(k_password_valid))
        .SetReturn((STRING_HANDLE)5);

    // act
    result = io_url_create(k_scheme_valid, k_host_name_valid, k_port_valid,
        k_path_valid, k_user_name_valid, k_password_valid, &url_valid);

    // assert
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(void_ptr, (void*)1, url_valid->host_name);
    ASSERT_ARE_EQUAL(void_ptr, (void*)2, url_valid->scheme);
    ASSERT_ARE_EQUAL(void_ptr, (void*)3, url_valid->path);
    ASSERT_ARE_EQUAL(void_ptr, (void*)4, url_valid->user_name);
    ASSERT_ARE_EQUAL(void_ptr, (void*)5, url_valid->password);
    ASSERT_ARE_EQUAL(void_ptr, NULL, url_valid->token_provider);
    ASSERT_ARE_EQUAL(int16_t, 8883, (int16_t)url_valid->port);
    ASSERT_EXPECTED_CALLS();
}

//
// Test io_url_create passing as scheme argument an invalid const char* value
//
TEST_FUNCTION(io_url_create__arg_scheme_invalid)
{
    static const char* k_host_name_valid = "testhost.com";
    static const uint16_t k_port_valid = 18;
    static const char* k_path_valid = "foobar";
    static const char* k_user_name_valid = "user1";
    static const char* k_password_valid = "pword";

    io_url_t* url_valid;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_url_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(STRING_construct(k_host_name_valid))
        .SetReturn((STRING_HANDLE)1);
    STRICT_EXPECTED_CALL(STRING_construct(k_path_valid))
        .SetReturn((STRING_HANDLE)3);
    STRICT_EXPECTED_CALL(STRING_construct(k_user_name_valid))
        .SetReturn((STRING_HANDLE)4);
    STRICT_EXPECTED_CALL(STRING_construct(k_password_valid))
        .SetReturn((STRING_HANDLE)5);

    // act
    result = io_url_create(NULL, k_host_name_valid, k_port_valid,
        k_path_valid, k_user_name_valid, k_password_valid, &url_valid);

    // assert
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(void_ptr, (void*)1, url_valid->host_name);
    ASSERT_ARE_EQUAL(void_ptr, NULL, url_valid->scheme);
    ASSERT_ARE_EQUAL(void_ptr, (void*)3, url_valid->path);
    ASSERT_ARE_EQUAL(void_ptr, (void*)4, url_valid->user_name);
    ASSERT_ARE_EQUAL(void_ptr, (void*)5, url_valid->password);
    ASSERT_ARE_EQUAL(void_ptr, NULL, url_valid->token_provider);
    ASSERT_ARE_EQUAL(int16_t, k_port_valid, (int16_t)url_valid->port);
    ASSERT_EXPECTED_CALLS();
}

//
// Test io_url_create passing as host_name argument an invalid const char* value
//
TEST_FUNCTION(io_url_create__arg_host_name_invalid)
{
    static const char* k_scheme_valid = "mqtts";
    static const uint16_t k_port_valid = 0;
    static const char* k_path_valid = "foobar";
    static const char* k_user_name_valid = "user1";
    static const char* k_password_valid = "pword";
    io_url_t* url_valid;
    int32_t result;

    // arrange

    // act
    result = io_url_create(k_scheme_valid, NULL, k_port_valid,
        k_path_valid, k_user_name_valid, k_password_valid, &url_valid);

    // assert
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    ASSERT_EXPECTED_CALLS();
}

//
// Test io_url_create passing as port argument an invalid uint16_t value
//
TEST_FUNCTION(io_url_create__arg_port_invalid)
{
    static const char* k_scheme_valid = "test";
    static const char* k_host_name_valid = "testhost.com";
    static const uint16_t k_port_valid = 0;
    static const char* k_path_valid = "foobar";
    static const char* k_user_name_valid = "user1";
    static const char* k_password_valid = "pword";
    io_url_t* url_valid;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_url_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(STRING_construct(k_host_name_valid))
        .SetReturn((STRING_HANDLE)1);
    STRICT_EXPECTED_CALL(string_compare_nocase(k_scheme_valid, "wss"))
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(string_compare_nocase(k_scheme_valid, "amqps"))
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(string_compare_nocase(k_scheme_valid, "https"))
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(string_compare_nocase(k_scheme_valid, "mqtts"))
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(STRING_delete((STRING_HANDLE)1));
    STRICT_EXPECTED_CALL(h_free((void*)UT_MEM, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act
    result = io_url_create(k_scheme_valid, k_host_name_valid, k_port_valid,
        k_path_valid, k_user_name_valid, k_password_valid, &url_valid);

    // assert
    ASSERT_ARE_EQUAL(int32_t, er_invalid_format, result);
    ASSERT_EXPECTED_CALLS();
}

//
// Test io_url_create passing as path argument an invalid const char* value
//
TEST_FUNCTION(io_url_create__arg_path_invalid)
{
    static const char* k_scheme_valid = "amqps";
    static const char* k_host_name_valid = "testhost.com";
    static const uint16_t k_port_valid = 0;
    static const char* k_user_name_valid = "user1";
    static const char* k_password_valid = "pword";
    io_url_t* url_valid;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_url_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(STRING_construct(k_host_name_valid))
        .SetReturn((STRING_HANDLE)1);
    STRICT_EXPECTED_CALL(string_compare_nocase(k_scheme_valid, "wss"))
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(string_compare_nocase(k_scheme_valid, k_scheme_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_construct(k_scheme_valid))
        .SetReturn((STRING_HANDLE)2);
    STRICT_EXPECTED_CALL(STRING_construct(k_user_name_valid))
        .SetReturn((STRING_HANDLE)4);
    STRICT_EXPECTED_CALL(STRING_construct(k_password_valid))
        .SetReturn((STRING_HANDLE)5);

    // act
    result = io_url_create(k_scheme_valid, k_host_name_valid, k_port_valid,
        NULL, k_user_name_valid, k_password_valid, &url_valid);

    // assert
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(void_ptr, (void*)1, url_valid->host_name);
    ASSERT_ARE_EQUAL(void_ptr, (void*)2, url_valid->scheme);
    ASSERT_ARE_EQUAL(void_ptr, NULL, url_valid->path);
    ASSERT_ARE_EQUAL(void_ptr, (void*)4, url_valid->user_name);
    ASSERT_ARE_EQUAL(void_ptr, (void*)5, url_valid->password);
    ASSERT_ARE_EQUAL(void_ptr, NULL, url_valid->token_provider);
    ASSERT_ARE_EQUAL(int16_t, 5671, (int16_t)url_valid->port);
    ASSERT_EXPECTED_CALLS();
}

//
// Test io_url_create passing as user_name argument an invalid const char* value
//
TEST_FUNCTION(io_url_create__arg_user_name_invalid)
{
    static const char* k_scheme_valid = "wss";
    static const char* k_host_name_valid = "testhost.com";
    static const uint16_t k_port_valid = 0;
    static const char* k_path_valid = "foobar";
    static const char* k_password_valid = "pword";
    io_url_t* url_valid;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_url_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(STRING_construct(k_host_name_valid))
        .SetReturn((STRING_HANDLE)1);
    STRICT_EXPECTED_CALL(string_compare_nocase(k_scheme_valid, k_scheme_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_construct(k_scheme_valid))
        .SetReturn((STRING_HANDLE)2);
    STRICT_EXPECTED_CALL(STRING_construct(k_path_valid))
        .SetReturn((STRING_HANDLE)3);
    STRICT_EXPECTED_CALL(STRING_construct(k_password_valid))
        .SetReturn((STRING_HANDLE)5);

    // act
    result = io_url_create(k_scheme_valid, k_host_name_valid, k_port_valid,
        k_path_valid, NULL, k_password_valid, &url_valid);

    // assert
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(void_ptr, (void*)1, url_valid->host_name);
    ASSERT_ARE_EQUAL(void_ptr, (void*)2, url_valid->scheme);
    ASSERT_ARE_EQUAL(void_ptr, (void*)3, url_valid->path);
    ASSERT_ARE_EQUAL(void_ptr, NULL, url_valid->user_name);
    ASSERT_ARE_EQUAL(void_ptr, (void*)5, url_valid->password);
    ASSERT_ARE_EQUAL(void_ptr, NULL, url_valid->token_provider);
    ASSERT_ARE_EQUAL(int16_t, 443, (int16_t)url_valid->port);
    ASSERT_EXPECTED_CALLS();
}

//
// Test io_url_create passing as password argument an invalid const char* value
//
TEST_FUNCTION(io_url_create__arg_password_invalid)
{
    static const char* k_scheme_valid = "https";
    static const char* k_host_name_valid = "testhost.com";
    static const uint16_t k_port_valid = 0;
    static const char* k_path_valid = "foobar";
    static const char* k_user_name_valid = "user1";
    io_url_t* url_valid;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_url_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(STRING_construct(k_host_name_valid))
        .SetReturn((STRING_HANDLE)1);
    STRICT_EXPECTED_CALL(string_compare_nocase(k_scheme_valid, "wss"))
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(string_compare_nocase(k_scheme_valid, "amqps"))
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(string_compare_nocase(k_scheme_valid, k_scheme_valid))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_construct(k_scheme_valid))
        .SetReturn((STRING_HANDLE)2);
    STRICT_EXPECTED_CALL(STRING_construct(k_path_valid))
        .SetReturn((STRING_HANDLE)3);
    STRICT_EXPECTED_CALL(STRING_construct(k_user_name_valid))
        .SetReturn((STRING_HANDLE)4);

    // act
    result = io_url_create(k_scheme_valid, k_host_name_valid, k_port_valid,
        k_path_valid, k_user_name_valid, NULL, &url_valid);

    // assert
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(void_ptr, (void*)1, url_valid->host_name);
    ASSERT_ARE_EQUAL(void_ptr, (void*)2, url_valid->scheme);
    ASSERT_ARE_EQUAL(void_ptr, (void*)3, url_valid->path);
    ASSERT_ARE_EQUAL(void_ptr, (void*)4, url_valid->user_name);
    ASSERT_ARE_EQUAL(void_ptr, NULL, url_valid->password);
    ASSERT_ARE_EQUAL(void_ptr, NULL, url_valid->token_provider);
    ASSERT_ARE_EQUAL(int16_t, 443, (int16_t)url_valid->port);
    ASSERT_EXPECTED_CALLS();
}

//
// Test io_url_create passing as url argument an invalid io_url_t** value
//
TEST_FUNCTION(io_url_create__arg_url_invalid)
{
    static const char* k_scheme_valid = "mqtts";
    static const char* k_host_name_valid = "testhost.com";
    static const uint16_t k_port_valid = 0;
    static const char* k_path_valid = "foobar";
    static const char* k_user_name_valid = "user1";
    static const char* k_password_valid = "pword";
    int32_t result;

    // arrange

    // act
    result = io_url_create(k_scheme_valid, k_host_name_valid, k_port_valid,
        k_path_valid, k_user_name_valid, k_password_valid, NULL);

    // assert
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    ASSERT_EXPECTED_CALLS();
}

//
// Test io_url_create unhappy path
//
TEST_FUNCTION(io_url_create__neg)
{
    static const char* k_scheme_valid = "wss";
    static const char* k_host_name_valid = "testhost.com";
    static const uint16_t k_port_valid = 0;
    static const char* k_path_valid = "foobar";
    static const char* k_user_name_valid = "user1";
    static const char* k_password_valid = "pword";
    io_url_t* url_valid;
    int32_t result;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_url_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(STRING_construct(k_host_name_valid))
        .SetReturn((STRING_HANDLE)1)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(STRING_construct(k_scheme_valid))
        .SetReturn((STRING_HANDLE)2)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(STRING_construct(k_path_valid))
        .SetReturn((STRING_HANDLE)3)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(STRING_construct(k_user_name_valid))
        .SetReturn((STRING_HANDLE)4)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(STRING_construct(k_password_valid))
        .SetReturn((STRING_HANDLE)5)
        .SetFailReturn(NULL);

    // act
        UMOCK_C_NEGATIVE_TESTS_ACT();
        memset(UT_MEM, 0, sizeof(UT_MEM));
    result = io_url_create(k_scheme_valid, k_host_name_valid, k_port_valid,
        k_path_valid, k_user_name_valid, k_password_valid, &url_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_out_of_memory);
}

//
// Test io_url_clone happy path
//
TEST_FUNCTION(io_url_clone__success)
{
    static io_url_t url_valid = {
        (STRING_HANDLE)1,
        (STRING_HANDLE)2,
        3,
        (STRING_HANDLE)4,
        (STRING_HANDLE)5,
        (STRING_HANDLE)6,
        NULL
    };
    io_url_t* cloned_valid;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_url_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(STRING_clone((STRING_HANDLE)1))
        .SetReturn((STRING_HANDLE)1);
    STRICT_EXPECTED_CALL(STRING_clone((STRING_HANDLE)2))
        .SetReturn((STRING_HANDLE)2);
    STRICT_EXPECTED_CALL(STRING_clone((STRING_HANDLE)4))
        .SetReturn((STRING_HANDLE)4);
    STRICT_EXPECTED_CALL(STRING_clone((STRING_HANDLE)5))
        .SetReturn((STRING_HANDLE)5);
    STRICT_EXPECTED_CALL(STRING_clone((STRING_HANDLE)6))
        .SetReturn((STRING_HANDLE)6);

    // act
    result = io_url_clone(&url_valid, &cloned_valid);

    // assert
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(void_ptr, (void*)1, cloned_valid->scheme);
    ASSERT_ARE_EQUAL(void_ptr, (void*)2, cloned_valid->host_name);
    ASSERT_ARE_EQUAL(int16_t, 3, (int16_t)cloned_valid->port);
    ASSERT_ARE_EQUAL(void_ptr, (void*)4, cloned_valid->path);
    ASSERT_ARE_EQUAL(void_ptr, (void*)5, cloned_valid->user_name);
    ASSERT_ARE_EQUAL(void_ptr, (void*)6, cloned_valid->password);
    ASSERT_ARE_EQUAL(void_ptr, NULL, cloned_valid->token_provider);
    ASSERT_EXPECTED_CALLS();
}

//
// Test io_url_clone passing as url argument an invalid io_url_t* value
//
TEST_FUNCTION(io_url_clone__arg_url_invalid)
{
    io_url_t* cloned_valid;
    int32_t result;

    // arrange

    // act
    result = io_url_clone(NULL, &cloned_valid);

    // assert
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    ASSERT_EXPECTED_CALLS();
}

//
// Test io_url_clone passing as cloned argument an invalid io_url_t** value
//
TEST_FUNCTION(io_url_clone__arg_cloned_invalid)
{
    static io_url_t url_valid = {
        (STRING_HANDLE)1,
        (STRING_HANDLE)2,
        3,
        (STRING_HANDLE)4,
        (STRING_HANDLE)5,
        (STRING_HANDLE)6,
        NULL
    };
    int32_t result;

    // arrange

    // act
    result = io_url_clone(&url_valid, NULL);

    // assert
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    ASSERT_EXPECTED_CALLS();
}

//
// Test io_url_clone unhappy path
//
TEST_FUNCTION(io_url_clone__neg)
{
    static io_url_t url_valid = {
        (STRING_HANDLE)1,
        (STRING_HANDLE)2,
        3,
        (STRING_HANDLE)4,
        (STRING_HANDLE)5,
        (STRING_HANDLE)6,
        NULL
    };
    io_url_t* cloned_valid;
    int32_t result;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(h_realloc(sizeof(io_url_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(STRING_clone((STRING_HANDLE)1))
        .SetReturn((STRING_HANDLE)1);
    STRICT_EXPECTED_CALL(STRING_clone((STRING_HANDLE)2))
        .SetReturn((STRING_HANDLE)2);
    STRICT_EXPECTED_CALL(STRING_clone((STRING_HANDLE)4))
        .SetReturn((STRING_HANDLE)4);
    STRICT_EXPECTED_CALL(STRING_clone((STRING_HANDLE)5))
        .SetReturn((STRING_HANDLE)5);
    STRICT_EXPECTED_CALL(STRING_clone((STRING_HANDLE)6))
        .SetReturn((STRING_HANDLE)6);

    // act
        UMOCK_C_NEGATIVE_TESTS_ACT();
        memset(UT_MEM, 0, sizeof(UT_MEM));
    result = io_url_clone(&url_valid, &cloned_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_out_of_memory);
}

//
// Test io_url_equals happy path
//
TEST_FUNCTION(io_url_equals__success)
{
    static io_url_t url_that_valid = {
        (STRING_HANDLE)1,
        (STRING_HANDLE)2,
        3,
        (STRING_HANDLE)4,
        (STRING_HANDLE)5,
        (STRING_HANDLE)6,
        (io_token_provider_t*)7
    };
    static io_url_t url_other_valid = {
        (STRING_HANDLE)11,
        (STRING_HANDLE)12,
        3,
        (STRING_HANDLE)14,
        (STRING_HANDLE)15,
        (STRING_HANDLE)16,
        (io_token_provider_t*)17
    };
    bool result;

    // arrange
    STRICT_EXPECTED_CALL(STRING_compare_nocase((STRING_HANDLE)1, (STRING_HANDLE)11))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_compare_nocase((STRING_HANDLE)2, (STRING_HANDLE)12))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_compare_nocase((STRING_HANDLE)4, (STRING_HANDLE)14))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_compare_nocase((STRING_HANDLE)5, (STRING_HANDLE)15))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(STRING_compare_nocase((STRING_HANDLE)6, (STRING_HANDLE)16))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(io_token_provider_is_equivalent((io_token_provider_t*)7, (io_token_provider_t*)17))
        .SetReturn(true);

    // act
    result = io_url_equals(&url_that_valid, &url_other_valid);

    // assert
    ASSERT_IS_TRUE(result);
    ASSERT_EXPECTED_CALLS();
}

//
// Test io_url_equals passing as other argument an invalid io_url_t* value
//
TEST_FUNCTION(io_url_equals__success_both_invalid)
{
    bool result;

    // arrange

    // act
    result = io_url_equals(NULL, NULL);

    // assert
    ASSERT_IS_TRUE(result);
    ASSERT_EXPECTED_CALLS();
}

//
// Test io_url_equals passing as that argument an invalid io_url_t* value
//
TEST_FUNCTION(io_url_equals__arg_that_invalid)
{
    static io_url_t url_other_valid = {
        (STRING_HANDLE)11,
        (STRING_HANDLE)12,
        3,
        (STRING_HANDLE)14,
        (STRING_HANDLE)15,
        (STRING_HANDLE)16,
        NULL
    };
    bool result;

    // arrange

    // act
    result = io_url_equals(NULL, &url_other_valid);

    // assert
    ASSERT_IS_FALSE(result);
    ASSERT_EXPECTED_CALLS();
}

//
// Test io_url_equals passing as other argument an invalid io_url_t* value
//
TEST_FUNCTION(io_url_equals__arg_other_invalid)
{
    static io_url_t url_that_valid = {
        (STRING_HANDLE)1,
        (STRING_HANDLE)2,
        3,
        (STRING_HANDLE)4,
        (STRING_HANDLE)5,
        (STRING_HANDLE)6,
        NULL
    };
    bool result;

    // arrange

    // act
    result = io_url_equals(&url_that_valid, NULL);

    // assert
    ASSERT_IS_FALSE(result);
    ASSERT_EXPECTED_CALLS();
}

//
// Test io_url_free happy path
//
TEST_FUNCTION(io_url_free__success)
{
    static io_url_t url_valid = {
        (STRING_HANDLE)1,
        (STRING_HANDLE)2,
        3,
        (STRING_HANDLE)4,
        (STRING_HANDLE)5,
        (STRING_HANDLE)6,
        NULL
    };

    // arrange
    STRICT_EXPECTED_CALL(STRING_delete((STRING_HANDLE)1));
    STRICT_EXPECTED_CALL(STRING_delete((STRING_HANDLE)2));
    STRICT_EXPECTED_CALL(STRING_delete((STRING_HANDLE)4));
    STRICT_EXPECTED_CALL(STRING_delete((STRING_HANDLE)5));
    STRICT_EXPECTED_CALL(STRING_delete((STRING_HANDLE)6));

    STRICT_EXPECTED_CALL(h_free((void*)&url_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act
    io_url_free(&url_valid);

    // assert
    // ...
    ASSERT_EXPECTED_CALLS();
}

//
// Test io_url_free passing as url argument an invalid io_url_t* value
//
TEST_FUNCTION(io_url_free__arg_url_invalid)
{
    // arrange

    // act
    io_url_free(NULL);

    // assert
    ASSERT_EXPECTED_CALLS();
}

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

