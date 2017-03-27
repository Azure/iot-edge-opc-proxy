// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.


#define UNIT_UNDER_TEST pal_cred_gnome
#include "util_ut.h"

//
// 1. Required mocks
//
#include "os.h"
#include "util_string.h"

MOCKABLE_FUNCTION(, void, secret_password_free,
    gchar*, password);
MOCKABLE_FUNCTION(, void, g_error_free,
    GError*, error);
MOCKABLE_FUNCTION(, gboolean, secret_password_clear_sync,
    const SecretSchema*, schema, GCancellable*, cancellable,
    GError**, error, const void*, key, const void*, value, void*, x);
MOCKABLE_FUNCTION(, gboolean, secret_password_store_sync,
    const SecretSchema*, schema, const gchar*, collection,
    const gchar*, label, const gchar*, password,
    GCancellable*, cancellable, GError**, error,
    const void*, key, const void*, value, void*, x);
MOCKABLE_FUNCTION(, gchar*, secret_password_lookup_sync,
    const SecretSchema*, schema, GCancellable*, cancellable,
    GError**, error,
    const void*, key, const void*, value, void*, x);

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
REGISTER_UMOCK_ALIAS_TYPE(g_error_t, void*);
REGISTER_UMOCK_ALIAS_TYPE(gboolean, bool);
REGISTER_UMOCK_ALIAS_TYPE(gchar, char);
REGISTER_UMOCK_ALIAS_TYPE(gchar*, char*);
REGISTER_UMOCK_ALIAS_TYPE(STRING_HANDLE, void*);
REGISTER_UMOCK_ALIAS_TYPE(BUFFER_HANDLE, void*);
REGISTER_UMOCK_ALIAS_TYPE(HMACSHA256_RESULT, int);
END_DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
DECLARE_TEST_SETUP()

// 
// Test pal_cred_import happy path 
// 
TEST_FUNCTION(pal_gnome_cred_import__success_0)
{
    static const char* k_key_name_valid = "testkeyname";
    static const size_t k_key_len_valid = 32;
    static STRING_HANDLE k_encoded_string = (STRING_HANDLE)0x8982;
    static const char* k_key_encoded_valid = "3452345234578";
    static STRING_HANDLE k_handle_valid = (STRING_HANDLE)0x234243;
    static const bool k_persist_valid = true;
    void* k_key_val_valid = (UT_MEM + 128);
    STRING_HANDLE handle_valid;
    int32_t result;

    memcpy(UT_MEM, k_key_encoded_valid, strlen(k_key_encoded_valid) + 1);

    // arrange 
    STRICT_EXPECTED_CALL(STRING_construct(k_key_name_valid))
        .SetReturn(k_handle_valid);
    STRICT_EXPECTED_CALL(STRING_construct_base16((uint8_t*)k_key_val_valid, k_key_len_valid))
        .SetReturn(k_encoded_string);
    STRICT_EXPECTED_CALL(STRING_c_str(k_encoded_string))
        .SetReturn(UT_MEM);
    STRICT_EXPECTED_CALL(secret_password_store_sync(IGNORED_PTR_ARG, SECRET_COLLECTION_DEFAULT, k_key_name_valid, k_key_encoded_valid, NULL, IGNORED_PTR_ARG, PASSWORD_TARGET, k_key_name_valid, NULL))
        .IgnoreAllArguments()
        .SetReturn(true);
    STRICT_EXPECTED_CALL(STRING_c_str(k_encoded_string))
        .SetReturn(UT_MEM);
    STRICT_EXPECTED_CALL(STRING_length(k_encoded_string))
        .SetReturn(strlen(k_key_encoded_valid));
    STRICT_EXPECTED_CALL(STRING_delete(k_encoded_string));

    // act 
    result = pal_cred_import(k_key_name_valid, k_key_val_valid, k_key_len_valid, k_persist_valid, &handle_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_cred_import happy path 
// 
TEST_FUNCTION(pal_gnome_cred_import__success_1)
{
    static const char* k_key_name_valid = "testkeyname";
    static const size_t k_key_len_valid = 32;
    static STRING_HANDLE k_encoded_string = (STRING_HANDLE)0x8982;
    static const char* k_key_encoded_valid = "3452345234578";
    static STRING_HANDLE k_handle_valid = (STRING_HANDLE)0x234243;
    static const bool k_persist_valid = false;
    void* k_key_val_valid = (UT_MEM + 128);
    STRING_HANDLE handle_valid;
    int32_t result;

    memcpy(UT_MEM, k_key_encoded_valid, strlen(k_key_encoded_valid) + 1);

    // arrange 
    STRICT_EXPECTED_CALL(STRING_construct(k_key_name_valid))
        .SetReturn(k_handle_valid);
    STRICT_EXPECTED_CALL(STRING_construct_base16((uint8_t*)k_key_val_valid, k_key_len_valid))
        .SetReturn(k_encoded_string);
    STRICT_EXPECTED_CALL(STRING_c_str(k_encoded_string))
        .SetReturn(UT_MEM);
    STRICT_EXPECTED_CALL(secret_password_store_sync(IGNORED_PTR_ARG, SECRET_COLLECTION_SESSION, k_key_name_valid, k_key_encoded_valid, NULL, IGNORED_PTR_ARG, PASSWORD_TARGET, k_key_name_valid, NULL))
        .IgnoreAllArguments()
        .SetReturn(true);
    STRICT_EXPECTED_CALL(STRING_c_str(k_encoded_string))
        .SetReturn(UT_MEM);
    STRICT_EXPECTED_CALL(STRING_length(k_encoded_string))
        .SetReturn(strlen(k_key_encoded_valid));
    STRICT_EXPECTED_CALL(STRING_delete(k_encoded_string));

    // act 
    result = pal_cred_import(k_key_name_valid, k_key_val_valid, k_key_len_valid, k_persist_valid, &handle_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_cred_import passing as key_val argument an invalid void* value 
// 
TEST_FUNCTION(pal_gnome_cred_import__arg_key_val_invalid)
{
    static const char* k_key_name_valid = "testkeyname";
    static const size_t k_key_len_valid = 32;
    static const bool k_persist_valid = false;
    STRING_HANDLE handle_valid;
    int32_t result;

    // arrange 

    // act 
    result = pal_cred_import(k_key_name_valid, NULL, k_key_len_valid, k_persist_valid, &handle_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_cred_import passing as key_len argument an invalid size_t value 
// 
TEST_FUNCTION(pal_gnome_cred_import__arg_key_len_invalid)
{
    static const char* k_key_name_valid = "testkeyname";
    static void* k_key_val_valid = (void*)0x2432;
    static const bool k_persist_valid = false;
    STRING_HANDLE handle_valid;
    int32_t result;

    // arrange 

    // act 
    result = pal_cred_import(k_key_name_valid, k_key_val_valid, 0, k_persist_valid, &handle_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_arg, result);
}

// 
// Test pal_cred_import passing as handle argument an invalid STRING_HANDLE* value 
// 
TEST_FUNCTION(pal_gnome_cred_import__arg_handle_invalid)
{
    static const char* k_key_name_valid = "testkeyname";
    static void* k_key_val_valid = (void*)0x2432;
    static const size_t k_key_len_valid = 32;
    static const bool k_persist_valid = false;
    int32_t result;

    // arrange 

    // act 
    result = pal_cred_import(k_key_name_valid, k_key_val_valid, k_key_len_valid, k_persist_valid, NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test pal_cred_import unhappy path 
// 
TEST_FUNCTION(pal_gnome_cred_import__neg_0)
{
    static const char* k_key_name_valid = "testkeyname";
    static const size_t k_key_len_valid = 32;
    static const bool k_persist_valid = true;
    void* k_key_val_valid = (UT_MEM + 128);
    STRING_HANDLE handle_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_construct(k_key_name_valid))
        .SetReturn(NULL);

    // act 
    result = pal_cred_import(k_key_name_valid, k_key_val_valid, k_key_len_valid, k_persist_valid, &handle_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_out_of_memory, result);
}

// 
// Test pal_cred_import unhappy path 
// 
TEST_FUNCTION(pal_gnome_cred_import__neg_1)
{
    static const char* k_key_name_valid = "testkeyname";
    static const size_t k_key_len_valid = 32;
    static STRING_HANDLE k_encoded_string = (STRING_HANDLE)0x8982;
    static const char* k_key_encoded_valid = "3452345234578";
    static STRING_HANDLE k_handle_valid = (STRING_HANDLE)0x234243;
    static const bool k_persist_valid = true;
    void* k_key_val_valid = (UT_MEM + 128);
    STRING_HANDLE handle_valid;
    int32_t result;

    memcpy(UT_MEM, k_key_encoded_valid, strlen(k_key_encoded_valid) + 1);

    // arrange 
    STRICT_EXPECTED_CALL(STRING_construct(k_key_name_valid))
        .SetReturn(k_handle_valid);
    STRICT_EXPECTED_CALL(STRING_construct_base16((uint8_t*)k_key_val_valid, k_key_len_valid))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(STRING_delete(k_handle_valid));

    // act 
    result = pal_cred_import(k_key_name_valid, k_key_val_valid, k_key_len_valid, k_persist_valid, &handle_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_out_of_memory, result);
}

// 
// Test pal_cred_import unhappy path 
// 
TEST_FUNCTION(pal_gnome_cred_import__neg_2)
{
    static const char* k_key_name_valid = "testkeyname";
    static const size_t k_key_len_valid = 32;
    static STRING_HANDLE k_encoded_string = (STRING_HANDLE)0x8982;
    static const char* k_key_encoded_valid = "3452345234578";
    static STRING_HANDLE k_handle_valid = (STRING_HANDLE)0x234243;
    static const bool k_persist_valid = true;
    void* k_key_val_valid = (UT_MEM + 128);
    STRING_HANDLE handle_valid;
    int32_t result;
    GError error_valid, *k_error_valid_ptr = &error_valid;

    memcpy(UT_MEM, k_key_encoded_valid, strlen(k_key_encoded_valid) + 1);
    error_valid.message = "Error";

    // arrange 
    STRICT_EXPECTED_CALL(STRING_construct(k_key_name_valid))
        .SetReturn(k_handle_valid);
    STRICT_EXPECTED_CALL(STRING_construct_base16((uint8_t*)k_key_val_valid, k_key_len_valid))
        .SetReturn(k_encoded_string);
    STRICT_EXPECTED_CALL(STRING_c_str(k_encoded_string))
        .SetReturn(UT_MEM);
    STRICT_EXPECTED_CALL(secret_password_store_sync(IGNORED_PTR_ARG, SECRET_COLLECTION_DEFAULT, k_key_name_valid, k_key_encoded_valid, NULL, IGNORED_PTR_ARG, PASSWORD_TARGET, k_key_name_valid, NULL))
        .IgnoreAllArguments()
        .CopyOutArgumentBuffer_error(&k_error_valid_ptr, sizeof(k_error_valid_ptr))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(g_error_free(k_error_valid_ptr));
    STRICT_EXPECTED_CALL(STRING_delete(k_handle_valid));
    STRICT_EXPECTED_CALL(STRING_c_str(k_encoded_string))
        .SetReturn(UT_MEM);
    STRICT_EXPECTED_CALL(STRING_length(k_encoded_string))
        .SetReturn(strlen(k_key_encoded_valid));
    STRICT_EXPECTED_CALL(STRING_delete(k_encoded_string));

    // act 
    result = pal_cred_import(k_key_name_valid, k_key_val_valid, k_key_len_valid, k_persist_valid, &handle_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fatal, result);
}

// 
// Test pal_cred_hmac_sha256 happy path 
// 
TEST_FUNCTION(pal_gnome_cred_hmac_sha256__success)
{
    static STRING_HANDLE k_valid_handle = (STRING_HANDLE)0x234;
    static char* k_valid_target = "device:x@y.com";
    static const char* k_protected_key = "10323423942334";
    static unsigned char* k_valid_key = (unsigned char*)"2382734928332423";
    static const size_t k_valid_key_len = 32;
    static unsigned char* k_valid_buf = (unsigned char*)"0x2355";
    static const size_t k_valid_buf_len = 123;
    static void* k_valid_sig = UT_MEM + 128;
    static const size_t k_valid_sig_len = 32;
    static void* k_valid_hash = "12345678901234567890123456789012";
    static BUFFER_HANDLE k_valid_buffer_handle = (BUFFER_HANDLE)0x53535;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(BUFFER_new())
        .SetReturn(k_valid_buffer_handle);
    STRICT_EXPECTED_CALL(STRING_c_str(k_valid_handle))
        .SetReturn(k_valid_target);
    STRICT_EXPECTED_CALL(secret_password_lookup_sync(IGNORED_PTR_ARG, NULL, IGNORED_PTR_ARG, PASSWORD_TARGET, k_valid_target, NULL))
        .IgnoreAllArguments()
        .SetReturn((gchar*)k_protected_key);
    STRICT_EXPECTED_CALL(h_realloc(7, NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)k_valid_key);
    STRICT_EXPECTED_CALL(string_base16_to_byte_array(k_protected_key, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .CopyOutArgumentBuffer_len(&k_valid_key_len, sizeof(k_valid_key_len))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(HMACSHA256_ComputeHash(k_valid_key, k_valid_key_len, k_valid_buf, k_valid_buf_len, k_valid_buffer_handle))
        .SetReturn(HMACSHA256_OK);
    STRICT_EXPECTED_CALL(BUFFER_length(k_valid_buffer_handle))
        .SetReturn(32);
    STRICT_EXPECTED_CALL(BUFFER_u_char(k_valid_buffer_handle))
        .SetReturn((unsigned char*)k_valid_hash);
    STRICT_EXPECTED_CALL(BUFFER_delete(k_valid_buffer_handle));
    STRICT_EXPECTED_CALL(secret_password_free((gchar*)k_protected_key));
    STRICT_EXPECTED_CALL(h_free((void*)k_valid_key, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    result = pal_cred_hmac_sha256(k_valid_handle, k_valid_buf, k_valid_buf_len, k_valid_sig, k_valid_sig_len);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_IS_TRUE(!memcmp(k_valid_sig, k_valid_hash, k_valid_sig_len));
}

// 
// Test pal_cred_hmac_sha256 passing as buf argument an invalid handle
// 
TEST_FUNCTION(pal_gnome_cred_hmac_sha256__arg_handle_null)
{
    static unsigned char* k_valid_buf = (unsigned char*)"0x2355";
    static const size_t k_valid_buf_len = 123;
    static void* k_valid_sig = UT_MEM;
    static const size_t k_valid_sig_len = 32;
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
TEST_FUNCTION(pal_gnome_cred_hmac_sha256__arg_buf_null)
{
    static STRING_HANDLE k_valid_handle = (STRING_HANDLE)0x234;
    static const size_t k_valid_buf_len = 123;
    static void* k_valid_sig = UT_MEM;
    static const size_t k_valid_sig_len = 32;
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
TEST_FUNCTION(pal_gnome_cred_hmac_sha256__arg_buf_len_invalid)
{
    static STRING_HANDLE k_valid_handle = (STRING_HANDLE)0x234;
    static unsigned char* k_valid_buf = (unsigned char*)"0x2355";
    static void* k_valid_sig = UT_MEM;
    static const size_t k_valid_sig_len = 32;
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
TEST_FUNCTION(pal_gnome_cred_hmac_sha256__arg_sig_null)
{
    static STRING_HANDLE k_valid_handle = (STRING_HANDLE)0x234;
    static unsigned char* k_valid_buf = (unsigned char*)"0x2355";
    static const size_t k_valid_buf_len = 123;
    static const size_t k_valid_sig_len = 32;
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
TEST_FUNCTION(pal_gnome_cred_hmac_sha256__arg_sig_len_invalid)
{
    static STRING_HANDLE k_valid_handle = (STRING_HANDLE)0x234;
    static unsigned char* k_valid_buf = (unsigned char*)"0x2355";
    static const size_t k_valid_buf_len = 123;
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
TEST_FUNCTION(pal_gnome_cred_hmac_sha256__neg_0)
{
    static STRING_HANDLE k_valid_handle = (STRING_HANDLE)0x234;
    static char* k_valid_target = "device:x@y.com";
    static unsigned char* k_valid_buf = (unsigned char*)"0x2355";
    static const size_t k_valid_buf_len = 123;
    static void* k_valid_sig = UT_MEM + 128;
    static const size_t k_valid_sig_len = 32;
    static BUFFER_HANDLE k_valid_buffer_handle = (BUFFER_HANDLE)0x53535;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(BUFFER_new())
        .SetReturn(k_valid_buffer_handle);
    STRICT_EXPECTED_CALL(STRING_c_str(k_valid_handle))
        .SetReturn(k_valid_target);
    STRICT_EXPECTED_CALL(secret_password_lookup_sync(IGNORED_PTR_ARG, NULL, IGNORED_PTR_ARG, PASSWORD_TARGET, k_valid_target, NULL))
        .IgnoreAllArguments()
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(BUFFER_delete(k_valid_buffer_handle));

    // act 
    result = pal_cred_hmac_sha256(k_valid_handle, k_valid_buf, k_valid_buf_len, k_valid_sig, k_valid_sig_len);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_not_found, result);
}

// 
// Test pal_cred_hmac_sha256 unhappy path 
// 
TEST_FUNCTION(pal_gnome_cred_hmac_sha256__neg_1)
{
    static STRING_HANDLE k_valid_handle = (STRING_HANDLE)0x234;
    static char* k_valid_target = "device:x@y.com";
    static unsigned char* k_valid_buf = (unsigned char*)"0x2355";
    static const size_t k_valid_buf_len = 123;
    static void* k_valid_sig = UT_MEM + 128;
    static const size_t k_valid_sig_len = 32;
    static BUFFER_HANDLE k_valid_buffer_handle = (BUFFER_HANDLE)0x53535;
    GError error_valid, *k_error_valid_ptr = &error_valid;
    int32_t result;

    error_valid.message = "Error";

    // arrange 
    STRICT_EXPECTED_CALL(BUFFER_new())
        .SetReturn(k_valid_buffer_handle);
    STRICT_EXPECTED_CALL(STRING_c_str(k_valid_handle))
        .SetReturn(k_valid_target);
    STRICT_EXPECTED_CALL(secret_password_lookup_sync(IGNORED_PTR_ARG, NULL, IGNORED_PTR_ARG, PASSWORD_TARGET, k_valid_target, NULL))
        .IgnoreAllArguments()
        .CopyOutArgumentBuffer_error(&k_error_valid_ptr, sizeof(k_error_valid_ptr))
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(g_error_free(k_error_valid_ptr));
    STRICT_EXPECTED_CALL(BUFFER_delete(k_valid_buffer_handle));

    // act 
    result = pal_cred_hmac_sha256(k_valid_handle, k_valid_buf, k_valid_buf_len, k_valid_sig, k_valid_sig_len);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fatal, result);
}

// 
// Test pal_cred_hmac_sha256 unhappy path 
// 
TEST_FUNCTION(pal_gnome_cred_hmac_sha256__neg_2)
{
    static STRING_HANDLE k_valid_handle = (STRING_HANDLE)0x234;
    static char* k_valid_target = "device:x@y.com";
    static const char* k_protected_key = "10323423942334";
    static unsigned char* k_valid_buf = (unsigned char*)"0x2355";
    static const size_t k_valid_buf_len = 123;
    static void* k_valid_sig = UT_MEM + 128;
    static const size_t k_valid_sig_len = 32;
    static BUFFER_HANDLE k_valid_buffer_handle = (BUFFER_HANDLE)0x53535;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(BUFFER_new())
        .SetReturn(k_valid_buffer_handle);
    STRICT_EXPECTED_CALL(STRING_c_str(k_valid_handle))
        .SetReturn(k_valid_target);
    STRICT_EXPECTED_CALL(secret_password_lookup_sync(IGNORED_PTR_ARG, NULL, IGNORED_PTR_ARG, PASSWORD_TARGET, k_valid_target, NULL))
        .IgnoreAllArguments()
        .SetReturn((gchar*)k_protected_key);
    STRICT_EXPECTED_CALL(h_realloc(7, NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(BUFFER_delete(k_valid_buffer_handle));
    STRICT_EXPECTED_CALL(secret_password_free((gchar*)k_protected_key));

    // act 
    result = pal_cred_hmac_sha256(k_valid_handle, k_valid_buf, k_valid_buf_len, k_valid_sig, k_valid_sig_len);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_out_of_memory, result);
}

// 
// Test pal_cred_hmac_sha256 unhappy path 
// 
TEST_FUNCTION(pal_gnome_cred_hmac_sha256__neg_3)
{
    static STRING_HANDLE k_valid_handle = (STRING_HANDLE)0x234;
    static char* k_valid_target = "device:x@y.com";
    static const char* k_protected_key = "10323423942334";
    static unsigned char* k_valid_key = (unsigned char*)"2382734928332423";
    static const size_t k_valid_key_len = 32;
    static unsigned char* k_valid_buf = (unsigned char*)"0x2355";
    static const size_t k_valid_buf_len = 123;
    static void* k_valid_sig = UT_MEM + 128;
    static const size_t k_valid_sig_len = 32;
    static BUFFER_HANDLE k_valid_buffer_handle = (BUFFER_HANDLE)0x53535;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(BUFFER_new())
        .SetReturn(k_valid_buffer_handle);
    STRICT_EXPECTED_CALL(STRING_c_str(k_valid_handle))
        .SetReturn(k_valid_target);
    STRICT_EXPECTED_CALL(secret_password_lookup_sync(IGNORED_PTR_ARG, NULL, IGNORED_PTR_ARG, PASSWORD_TARGET, k_valid_target, NULL))
        .IgnoreAllArguments()
        .SetReturn((gchar*)k_protected_key);
    STRICT_EXPECTED_CALL(h_realloc(7, NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)k_valid_key);
    STRICT_EXPECTED_CALL(string_base16_to_byte_array(k_protected_key, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .CopyOutArgumentBuffer_len(&k_valid_key_len, sizeof(k_valid_key_len))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(HMACSHA256_ComputeHash(k_valid_key, k_valid_key_len, k_valid_buf, k_valid_buf_len, k_valid_buffer_handle))
        .SetReturn(HMACSHA256_ERROR);
    STRICT_EXPECTED_CALL(BUFFER_delete(k_valid_buffer_handle));
    STRICT_EXPECTED_CALL(secret_password_free((gchar*)k_protected_key));
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
TEST_FUNCTION(pal_gnome_cred_hmac_sha256__neg_4)
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
// Test pal_cred_remove happy path 
// 
TEST_FUNCTION(pal_gnome_cred_remove__success)
{
    static STRING_HANDLE k_handle_valid = (STRING_HANDLE)0x23423;
    static const char* k_valid_target = "abcdefg";

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_handle_valid))
        .SetReturn(k_valid_target);
    STRICT_EXPECTED_CALL(secret_password_clear_sync(IGNORED_PTR_ARG, NULL, IGNORED_PTR_ARG, PASSWORD_TARGET, k_valid_target, NULL))
        .IgnoreAllArguments()
        .SetReturn(true);

    // act 
    pal_cred_remove(k_handle_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_cred_remove passing as handle argument an invalid STRING_HANDLE value 
// 
TEST_FUNCTION(pal_gnome_cred_remove__arg_handle_invalid)
{
    // arrange 

    // act 
    pal_cred_remove(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_cred_remove unhappy path 
// 
TEST_FUNCTION(pal_gnome_cred_remove__neg)
{
    static STRING_HANDLE k_handle_valid = (STRING_HANDLE)0x23423;
    static const char* k_valid_target = "abcdefg";
    GError error_valid, *k_error_valid_ptr = &error_valid;

    error_valid.message = "Error";

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_handle_valid))
        .SetReturn(k_valid_target);
    STRICT_EXPECTED_CALL(secret_password_clear_sync(IGNORED_PTR_ARG, NULL, IGNORED_PTR_ARG, PASSWORD_TARGET, k_valid_target, NULL))
        .IgnoreAllArguments()
        .CopyOutArgumentBuffer_error(&k_error_valid_ptr, sizeof(k_error_valid_ptr))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(g_error_free(k_error_valid_ptr));

    // act 
    pal_cred_remove(k_handle_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_cred_deinit happy path
// 
TEST_FUNCTION(pal_gnome_cred_deinit__success)
{
    // arrange 

    // act 
    pal_cred_deinit();

    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_cred_init happy path 
// 
TEST_FUNCTION(pal_gnome_cred_init__success)
{
    int32_t result;

    // arrange 

    // act 
    result = pal_cred_init();

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

