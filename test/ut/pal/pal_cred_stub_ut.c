// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.


#define UNIT_UNDER_TEST pal_cred_stub
#include "util_ut.h"

//
// 1. Required mocks
//
#include "os.h"
#include "util_string.h"

//
// 2. Include unit under test
//
#undef ENABLE_MOCKS
#include "pal_cred.h"
#define ENABLE_MOCKS
#include UNIT_C

//
// 3. Setup test suite
//
BEGIN_DECLARE_TEST_SUITE()
REGISTER_UMOCK_ALIAS_TYPE(STRING_HANDLE, void*);
REGISTER_UMOCK_ALIAS_TYPE(BUFFER_HANDLE, void*);
REGISTER_UMOCK_ALIAS_TYPE(HMACSHA256_RESULT, int);
END_DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
DECLARE_TEST_SETUP()

// 
// Test pal_cred_hmac_sha256 happy path 
// 
TEST_FUNCTION(pal_stub_cred_hmac_sha256__success)
{
    static STRING_HANDLE k_valid_handle = (STRING_HANDLE)0x234;
    static char* k_valid_b64_key = "a1s34";
    static unsigned char* k_valid_key = (unsigned char*)"a12355";
    static size_t k_valid_key_len = 32;

    static unsigned char* k_valid_buf = (unsigned char*)"0x2355";
    static size_t k_valid_buf_len = 123;

    static void* k_valid_sig = UT_MEM;
    static size_t k_valid_sig_len = 32;

    static void* k_valid_hash = "12345678901234567890123456789012";
    static BUFFER_HANDLE k_valid_buffer_handle = (BUFFER_HANDLE)0x53535;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 
    STRICT_EXPECTED_CALL(BUFFER_new())
        .SetReturn(k_valid_buffer_handle);
    STRICT_EXPECTED_CALL(STRING_c_str(k_valid_handle))
        .SetReturn(k_valid_b64_key);
    STRICT_EXPECTED_CALL(string_base64_to_byte_array(k_valid_b64_key, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_buffer(&k_valid_key, sizeof(k_valid_key))
        .CopyOutArgumentBuffer_len(&k_valid_key_len, sizeof(k_valid_key_len))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(HMACSHA256_ComputeHash(k_valid_key, k_valid_key_len, k_valid_buf, k_valid_buf_len, k_valid_buffer_handle))
        .SetReturn(HMACSHA256_OK);
    STRICT_EXPECTED_CALL(BUFFER_length(k_valid_buffer_handle))
        .SetReturn(32);
    STRICT_EXPECTED_CALL(BUFFER_u_char(k_valid_buffer_handle))
        .SetReturn((unsigned char*)k_valid_hash);
    STRICT_EXPECTED_CALL(BUFFER_delete(k_valid_buffer_handle));
    STRICT_EXPECTED_CALL(h_free((void*)k_valid_key, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    result = pal_cred_hmac_sha256(k_valid_handle, k_valid_buf, k_valid_buf_len, k_valid_sig, k_valid_sig_len);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_IS_TRUE(!memcmp(UT_MEM, k_valid_hash, 32));
}

// 
// Test pal_cred_hmac_sha256 passing as buf argument an invalid handle
// 
TEST_FUNCTION(pal_stub_cred_hmac_sha256__arg_handle_null)
{
    static unsigned char* k_valid_buf = (unsigned char*)"0x2355";
    static size_t k_valid_buf_len = 123;
    static void* k_valid_sig = UT_MEM;
    static size_t k_valid_sig_len = 32;
    int32_t result;

    // arrange 

    // act 
    result = pal_cred_hmac_sha256(NULL, k_valid_buf, k_valid_buf_len, k_valid_sig, k_valid_sig_len);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_cred_hmac_sha256 passing as buf argument an invalid ptr
// 
TEST_FUNCTION(pal_stub_cred_hmac_sha256__arg_buf_null)
{
    static STRING_HANDLE k_valid_handle = (STRING_HANDLE)0x234;
    static size_t k_valid_buf_len = 123;
    static void* k_valid_sig = UT_MEM;
    static size_t k_valid_sig_len = 32;
    int32_t result;

    // arrange 

    // act 
    result = pal_cred_hmac_sha256(k_valid_handle, NULL, k_valid_buf_len, k_valid_sig, k_valid_sig_len);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_cred_hmac_sha256 passing as buf_len 0
// 
TEST_FUNCTION(pal_stub_cred_hmac_sha256__arg_buf_len_invalid)
{
    static STRING_HANDLE k_valid_handle = (STRING_HANDLE)0x234;
    static unsigned char* k_valid_buf = (unsigned char*)"0x2355";
    static void* k_valid_sig = UT_MEM;
    static size_t k_valid_sig_len = 32;
    int32_t result;

    // arrange 

    // act 
    result = pal_cred_hmac_sha256(k_valid_handle, k_valid_buf, 0, k_valid_sig, k_valid_sig_len);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_arg, result);
}

// 
// Test pal_cred_hmac_sha256 passing as sig argument an invalid ptr
// 
TEST_FUNCTION(pal_stub_cred_hmac_sha256__arg_sig_null)
{
    static STRING_HANDLE k_valid_handle = (STRING_HANDLE)0x234;
    static unsigned char* k_valid_buf = (unsigned char*)"0x2355";
    static size_t k_valid_buf_len = 123;
    static size_t k_valid_sig_len = 32;
    int32_t result;

    // arrange 

    // act 
    result = pal_cred_hmac_sha256(k_valid_handle, k_valid_buf, k_valid_buf_len, NULL, k_valid_sig_len);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_cred_hmac_sha256 passing as sig_len and invalid value
// 
TEST_FUNCTION(pal_stub_cred_hmac_sha256__arg_sig_len_invalid)
{
    static STRING_HANDLE k_valid_handle = (STRING_HANDLE)0x234;
    static unsigned char* k_valid_buf = (unsigned char*)"0x2355";
    static size_t k_valid_buf_len = 123;
    static void* k_valid_sig = UT_MEM;
    int32_t result;

    // arrange 

    // act 
    result = pal_cred_hmac_sha256(k_valid_handle, k_valid_buf, k_valid_buf_len, k_valid_sig, 31);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_arg, result);
}

// 
// Test pal_cred_hmac_sha256 unhappy path 
// 
TEST_FUNCTION(pal_stub_cred_hmac_sha256__neg_0)
{
    static STRING_HANDLE k_valid_handle = (STRING_HANDLE)0x234;
    static char* k_valid_b64_key = "a1s34";
    static unsigned char* k_valid_buf = (unsigned char*)"0x2355";
    static size_t k_valid_buf_len = 123;

    static void* k_valid_sig = UT_MEM;
    static size_t k_valid_sig_len = 32;

    static BUFFER_HANDLE k_valid_buffer_handle = (BUFFER_HANDLE)0x53535;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 
    STRICT_EXPECTED_CALL(BUFFER_new())
        .SetReturn(k_valid_buffer_handle);
    STRICT_EXPECTED_CALL(STRING_c_str(k_valid_handle))
        .SetReturn(k_valid_b64_key);
    STRICT_EXPECTED_CALL(string_base64_to_byte_array(k_valid_b64_key, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2).IgnoreArgument(3)
        .SetReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL(BUFFER_delete(k_valid_buffer_handle));

    // act 
    result = pal_cred_hmac_sha256(k_valid_handle, k_valid_buf, k_valid_buf_len, k_valid_sig, k_valid_sig_len);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_out_of_memory, result);
}

// 
// Test pal_cred_hmac_sha256 unhappy path 
// 
TEST_FUNCTION(pal_stub_cred_hmac_sha256__neg_1)
{
    static STRING_HANDLE k_valid_handle = (STRING_HANDLE)0x234;
    static char* k_valid_b64_key = "a1s34";
    static unsigned char* k_valid_key = (unsigned char*)"0x2d5";
    static size_t k_valid_key_len = 32;

    static unsigned char* k_valid_buf = (unsigned char*)"0x2355";
    static size_t k_valid_buf_len = 123;

    static void* k_valid_sig = UT_MEM;
    static size_t k_valid_sig_len = 32;

    static BUFFER_HANDLE k_valid_buffer_handle = (BUFFER_HANDLE)0x53535;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 
    STRICT_EXPECTED_CALL(BUFFER_new())
        .SetReturn(k_valid_buffer_handle);
    STRICT_EXPECTED_CALL(STRING_c_str(k_valid_handle))
        .SetReturn(k_valid_b64_key);
    STRICT_EXPECTED_CALL(string_base64_to_byte_array(k_valid_b64_key, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_buffer(&k_valid_key, sizeof(k_valid_key))
        .CopyOutArgumentBuffer_len(&k_valid_key_len, sizeof(k_valid_key_len))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(HMACSHA256_ComputeHash(k_valid_key, k_valid_key_len, k_valid_buf, k_valid_buf_len, k_valid_buffer_handle))
        .SetReturn(HMACSHA256_ERROR);
    STRICT_EXPECTED_CALL(BUFFER_delete(k_valid_buffer_handle));
    STRICT_EXPECTED_CALL(h_free((void*)k_valid_key, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    result = pal_cred_hmac_sha256(k_valid_handle, k_valid_buf, k_valid_buf_len, k_valid_sig, k_valid_sig_len);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_invalid_format, result);
}


// 
// Test pal_cred_hmac_sha256 unhappy path 
// 
TEST_FUNCTION(pal_stub_cred_hmac_sha256__neg_2)
{
    static STRING_HANDLE k_valid_handle = (STRING_HANDLE)0x234;
    static char* k_valid_b64_key = "a1s34";
    static unsigned char* k_valid_key = (unsigned char*)"0x2d5";
    static size_t k_valid_key_len = 32;

    static unsigned char* k_valid_buf = (unsigned char*)"0x2355";
    static size_t k_valid_buf_len = 123;

    static void* k_valid_sig = UT_MEM;
    static size_t k_valid_sig_len = 32;

    static void* k_valid_hash = "12345678901234567890123456789012";
    static BUFFER_HANDLE k_valid_buffer_handle = (BUFFER_HANDLE)0x53535;
    int32_t result;

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(BUFFER_new())
        .SetReturn(k_valid_buffer_handle)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(STRING_c_str(k_valid_handle))
        .SetReturn(k_valid_b64_key)
        .SetFailReturn(k_valid_b64_key);
    STRICT_EXPECTED_CALL(string_base64_to_byte_array(k_valid_b64_key, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_buffer(&k_valid_key, sizeof(k_valid_key))
        .CopyOutArgumentBuffer_len(&k_valid_key_len, sizeof(k_valid_key_len))
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL(HMACSHA256_ComputeHash(k_valid_key, k_valid_key_len, k_valid_buf, k_valid_buf_len, k_valid_buffer_handle))
        .SetReturn(HMACSHA256_OK)
        .SetFailReturn(HMACSHA256_INVALID_ARG);
    STRICT_EXPECTED_CALL(BUFFER_length(k_valid_buffer_handle))
        .SetReturn(32)
        .SetFailReturn(11);
    STRICT_EXPECTED_CALL(BUFFER_u_char(k_valid_buffer_handle))
        .SetReturn((unsigned char*)k_valid_hash)
        .SetFailReturn((unsigned char*)k_valid_hash);
    STRICT_EXPECTED_CALL(BUFFER_delete(k_valid_buffer_handle));
    STRICT_EXPECTED_CALL(h_free((void*)k_valid_key, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    memset(UT_MEM, 0, sizeof(UT_MEM));
    result = pal_cred_hmac_sha256(k_valid_handle, k_valid_buf, k_valid_buf_len, k_valid_sig, k_valid_sig_len);

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_out_of_memory, er_ok, er_out_of_memory, er_invalid_format, er_ok);
}

// 
// Test pal_cred_hmac_sha256 unhappy path 
// 
TEST_FUNCTION(pal_stub_cred_hmac_sha256__neg_3)
{
    static STRING_HANDLE k_valid_handle = (STRING_HANDLE)0x234;
    static unsigned char* k_valid_buf = (unsigned char*)"0x2355";
    static const size_t k_valid_buf_len = 123;
    static void* k_valid_sig = UT_MEM + 128;
    static const size_t k_valid_sig_len = 32;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(BUFFER_new())
        .SetReturn(NULL);

    // act 
    result = pal_cred_hmac_sha256(k_valid_handle, k_valid_buf, k_valid_buf_len, k_valid_sig, k_valid_sig_len);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_out_of_memory, result);
}

// 
// Test pal_cred_deinit happy path
// 
TEST_FUNCTION(pal_stub_cred_deinit__success)
{
    // arrange 

    // act 
    pal_cred_deinit();

    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_cred_init happy path 
// 
TEST_FUNCTION(pal_stub_cred_init__success)
{
    int32_t result;

    // arrange 

    // act 
    result = pal_cred_init();

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_not_supported, result);
}

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

