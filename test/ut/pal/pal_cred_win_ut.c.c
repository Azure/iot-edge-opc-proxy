// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.


#define UNIT_UNDER_TEST pal_cred_win
#include "util_ut.h"

//
// 1. Required mocks
//
#include "os.h"
#include "util_string.h"

// advapi.h
MOCKABLE_FUNCTION(WINAPI, BOOL, CryptProtectMemory,
    LPVOID, pDataIn, DWORD, cbDataIn, DWORD, dwFlags);
MOCKABLE_FUNCTION(WINAPI, BOOL, CryptUnprotectMemory,
    LPVOID, pDataIn, DWORD, cbDataIn, DWORD, dwFlags);
// winnt.h
MOCKABLE_FUNCTION(, PVOID, RtlSecureZeroMemory,
    PVOID, ptr, SIZE_T, cnt);
// wincred.h
MOCKABLE_FUNCTION(WINAPI, BOOL, CredWriteA,
    CREDENTIALA*, Credential, DWORD, Flags);
MOCKABLE_FUNCTION(WINAPI, BOOL, CredReadA,
    LPCSTR, TargetName, DWORD, Type, DWORD, Flags, CREDENTIALA**, Credential);
MOCKABLE_FUNCTION(WINAPI, void, CredFree,
    PVOID, Buffer);
MOCKABLE_FUNCTION(WINAPI, BOOL, CredDeleteA,
    LPCSTR, TargetName, DWORD, Type, DWORD, Flags);


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
REGISTER_UMOCK_ALIAS_TYPE(BOOL, bool);
REGISTER_UMOCK_ALIAS_TYPE(DWORD, int);
REGISTER_UMOCK_ALIAS_TYPE(LPVOID, void*);
REGISTER_UMOCK_ALIAS_TYPE(PVOID, void*);
REGISTER_UMOCK_ALIAS_TYPE(LPCSTR, const char*);
REGISTER_UMOCK_ALIAS_TYPE(SIZE_T, unsigned long);
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
TEST_FUNCTION(pal_win_cred_import__success_0)
{
    static const char* k_key_name_valid = "testkeyname";
    static const size_t k_key_len_valid = 32;
    static STRING_HANDLE k_handle_valid = (STRING_HANDLE)0x234243;
    static const bool k_persist_valid = true;
    void* k_key_val_valid = (UT_MEM + 128);
    STRING_HANDLE handle_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_construct(k_key_name_valid))
        .SetReturn(k_handle_valid);
    STRICT_EXPECTED_CALL(CredWriteA(IGNORED_PTR_ARG, 0))
        .IgnoreArgument(1)
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(RtlSecureZeroMemory(k_key_val_valid, k_key_len_valid));

    // act 
    result = pal_cred_import(k_key_name_valid, k_key_val_valid, k_key_len_valid, k_persist_valid, &handle_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_cred_import happy path 
// 
TEST_FUNCTION(pal_win_cred_import__success_1)
{
    static const char* k_key_name_valid = "testkeyname";
    static const size_t k_key_len_valid = 32;
    static STRING_HANDLE k_handle_valid1 = (STRING_HANDLE)0x234243;
    static STRING_HANDLE k_handle_valid2 = (STRING_HANDLE)0x65343;
    static const bool k_persist_valid = true;
    void* k_key_val_valid = (UT_MEM + 128);
    STRING_HANDLE handle_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_construct(k_key_name_valid))
        .SetReturn(k_handle_valid1);
    STRICT_EXPECTED_CALL(CredWriteA(IGNORED_PTR_ARG, 0))
        .IgnoreArgument(1)
        .SetReturn(FALSE);
    STRICT_EXPECTED_CALL(STRING_delete(k_handle_valid1));

    STRICT_EXPECTED_CALL(h_realloc(48, NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(CryptProtectMemory((void*)UT_MEM, 48, CRYPTPROTECTMEMORY_SAME_LOGON))
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(STRING_construct_base64((unsigned char*)UT_MEM, 48))
        .SetReturn(k_handle_valid2);
    STRICT_EXPECTED_CALL(STRING_construct(HANDLE_PREFIX))
        .SetReturn(k_handle_valid1);
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(k_handle_valid1, k_handle_valid2))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(RtlSecureZeroMemory((void*)UT_MEM, 48));
    STRICT_EXPECTED_CALL(h_free((void*)UT_MEM, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);
    STRICT_EXPECTED_CALL(STRING_delete(k_handle_valid2));
    STRICT_EXPECTED_CALL(RtlSecureZeroMemory(k_key_val_valid, k_key_len_valid));

    // act 
    result = pal_cred_import(k_key_name_valid, k_key_val_valid, k_key_len_valid, k_persist_valid, &handle_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_cred_import happy path 
// 
TEST_FUNCTION(pal_win_cred_import__success_2)
{
    static const size_t k_key_len_valid = 32;
    static STRING_HANDLE k_handle_valid1 = (STRING_HANDLE)0x234243;
    static STRING_HANDLE k_handle_valid2 = (STRING_HANDLE)0x65343;
    static const bool k_persist_valid = true;
    void* k_key_val_valid = (void*)(UT_MEM + 128);
    STRING_HANDLE handle_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(48, NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(CryptProtectMemory((void*)UT_MEM, 48, CRYPTPROTECTMEMORY_SAME_LOGON))
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(STRING_construct_base64((unsigned char*)UT_MEM, 48))
        .SetReturn(k_handle_valid2);
    STRICT_EXPECTED_CALL(STRING_construct(HANDLE_PREFIX))
        .SetReturn(k_handle_valid1);
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(k_handle_valid1, k_handle_valid2))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(RtlSecureZeroMemory((void*)UT_MEM, 48));
    STRICT_EXPECTED_CALL(h_free((void*)UT_MEM, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);
    STRICT_EXPECTED_CALL(STRING_delete(k_handle_valid2));
    STRICT_EXPECTED_CALL(RtlSecureZeroMemory(k_key_val_valid, k_key_len_valid));

    // act 
    result = pal_cred_import(NULL, k_key_val_valid, k_key_len_valid, k_persist_valid, &handle_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_cred_import happy path 
// 
TEST_FUNCTION(pal_win_cred_import__success_3)
{
    static const char* k_key_name_valid = "testkeyname";
    static const size_t k_key_len_valid = 32;
    static STRING_HANDLE k_handle_valid1 = (STRING_HANDLE)0x234243;
    static STRING_HANDLE k_handle_valid2 = (STRING_HANDLE)0x65343;
    static const bool k_persist_valid = false;
    STRING_HANDLE handle_valid;
    void* k_key_val_valid = (void*)(UT_MEM + 128);
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(48, NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(CryptProtectMemory((void*)UT_MEM, 48, CRYPTPROTECTMEMORY_SAME_LOGON))
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(STRING_construct_base64((unsigned char*)UT_MEM, 48))
        .SetReturn(k_handle_valid2);
    STRICT_EXPECTED_CALL(STRING_construct(HANDLE_PREFIX))
        .SetReturn(k_handle_valid1);
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(k_handle_valid1, k_handle_valid2))
        .SetReturn(0);
    STRICT_EXPECTED_CALL(RtlSecureZeroMemory((void*)UT_MEM, 48));
    STRICT_EXPECTED_CALL(h_free((void*)UT_MEM, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);
    STRICT_EXPECTED_CALL(STRING_delete(k_handle_valid2));
    STRICT_EXPECTED_CALL(RtlSecureZeroMemory(k_key_val_valid, k_key_len_valid));

    // act 
    result = pal_cred_import(k_key_name_valid, k_key_val_valid, k_key_len_valid, k_persist_valid, &handle_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test pal_cred_import passing as key_val argument an invalid void* value 
// 
TEST_FUNCTION(pal_win_cred_import__arg_key_val_invalid)
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
TEST_FUNCTION(pal_win_cred_import__arg_key_len_invalid)
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
TEST_FUNCTION(pal_win_cred_import__arg_handle_invalid)
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
TEST_FUNCTION(pal_win_cred_import__neg_0)
{
    static const char* k_key_name_valid = "testkeyname";
    static const size_t k_key_len_valid = 32;
    static const bool k_persist_valid = true;
    STRING_HANDLE handle_valid;
    void* k_key_val_valid = (void*)(UT_MEM + 128);
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
TEST_FUNCTION(pal_win_cred_import__neg_1)
{
    static const char* k_key_name_valid = "testkeyname";
    static const size_t k_key_len_valid = 32;
    static STRING_HANDLE k_handle_valid1 = (STRING_HANDLE)0x234243;
    static const bool k_persist_valid = true;
    STRING_HANDLE handle_valid;
    void* k_key_val_valid = (void*)(UT_MEM + 128);
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_construct(k_key_name_valid))
        .SetReturn(k_handle_valid1);
    STRICT_EXPECTED_CALL(CredWriteA(IGNORED_PTR_ARG, 0))
        .IgnoreArgument(1)
        .SetReturn(FALSE);
    STRICT_EXPECTED_CALL(STRING_delete(k_handle_valid1));

    STRICT_EXPECTED_CALL(h_realloc(48, NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(CryptProtectMemory((void*)UT_MEM, 48, CRYPTPROTECTMEMORY_SAME_LOGON))
        .SetReturn(FALSE);
    STRICT_EXPECTED_CALL(pal_os_last_error_as_prx_error())
        .SetReturn(er_fatal);
    STRICT_EXPECTED_CALL(RtlSecureZeroMemory((void*)UT_MEM, 48));
    STRICT_EXPECTED_CALL(h_free((void*)UT_MEM, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);
    STRICT_EXPECTED_CALL(RtlSecureZeroMemory(k_key_val_valid, k_key_len_valid));

    // act 
    result = pal_cred_import(k_key_name_valid, k_key_val_valid, k_key_len_valid, k_persist_valid, &handle_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fatal, result);
}

// 
// Test pal_cred_import unhappy path 
// 
TEST_FUNCTION(pal_win_cred_import__neg_2)
{
    static const char* k_key_name_valid = "testkeyname";
    static const size_t k_key_len_valid = 32;
    static STRING_HANDLE k_handle_valid1 = (STRING_HANDLE)0x234243;
    static STRING_HANDLE k_handle_valid2 = (STRING_HANDLE)0x65343;
    static const bool k_persist_valid = true;
    void* k_key_val_valid = (void*)(UT_MEM + 128);
    STRING_HANDLE handle_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(STRING_construct(k_key_name_valid))
        .SetReturn(k_handle_valid1);
    STRICT_EXPECTED_CALL(CredWriteA(IGNORED_PTR_ARG, 0))
        .IgnoreArgument(1)
        .SetReturn(FALSE);
    STRICT_EXPECTED_CALL(STRING_delete(k_handle_valid1));

    STRICT_EXPECTED_CALL(h_realloc(48, NULL, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(CryptProtectMemory((void*)UT_MEM, 48, CRYPTPROTECTMEMORY_SAME_LOGON))
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(STRING_construct_base64((unsigned char*)UT_MEM, 48))
        .SetReturn(k_handle_valid2);
    STRICT_EXPECTED_CALL(STRING_construct(HANDLE_PREFIX))
        .SetReturn(k_handle_valid1);
    STRICT_EXPECTED_CALL(STRING_concat_with_STRING(k_handle_valid1, k_handle_valid2))
        .SetReturn(-1);
    STRICT_EXPECTED_CALL(STRING_delete(k_handle_valid1));
    STRICT_EXPECTED_CALL(RtlSecureZeroMemory((void*)UT_MEM, 48));
    STRICT_EXPECTED_CALL(h_free((void*)UT_MEM, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);
    STRICT_EXPECTED_CALL(STRING_delete(k_handle_valid2));
    STRICT_EXPECTED_CALL(RtlSecureZeroMemory(k_key_val_valid, k_key_len_valid));

    // act 
    result = pal_cred_import(k_key_name_valid, k_key_val_valid, k_key_len_valid, k_persist_valid, &handle_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_out_of_memory, result);
}

// 
// Test pal_cred_hmac_sha256 happy path 
// 
TEST_FUNCTION(pal_win_cred_hmac_sha256__success_0)
{
    static STRING_HANDLE k_valid_handle = (STRING_HANDLE)0x234;
    static char* k_valid_target = "b64:a1s343534580a8324=";
    static unsigned char* k_protected_key = (unsigned char*)UT_MEM;
    static const size_t k_protected_key_len = 48;
    static const size_t k_valid_key_len = 32;
    static unsigned char* k_valid_buf = (unsigned char*)"0x2355";
    static const size_t k_valid_buf_len = 123;
    static void* k_valid_sig = UT_MEM + 128;
    static const size_t k_valid_sig_len = 32;
    static void* k_valid_hash = "12345678901234567890123456789012";
    static BUFFER_HANDLE k_valid_buffer_handle = (BUFFER_HANDLE)0x53535;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));
    *(size_t*)UT_MEM = k_valid_key_len;

    // arrange 
    STRICT_EXPECTED_CALL(BUFFER_new())
        .SetReturn(k_valid_buffer_handle);
    STRICT_EXPECTED_CALL(STRING_c_str(k_valid_handle))
        .SetReturn(k_valid_target);
    STRICT_EXPECTED_CALL(string_starts_with_nocase(k_valid_target, HANDLE_PREFIX))
        .SetReturn(true);

    STRICT_EXPECTED_CALL(string_base64_to_byte_array(k_valid_target + 4, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_buffer(&k_protected_key, sizeof(k_protected_key))
        .CopyOutArgumentBuffer_len(&k_protected_key_len, sizeof(k_protected_key_len))
        .SetReturn(er_ok);
    STRICT_EXPECTED_CALL(CryptUnprotectMemory(k_protected_key, (DWORD)k_protected_key_len, CRYPTPROTECTMEMORY_SAME_LOGON))
        .SetReturn(TRUE);

    STRICT_EXPECTED_CALL(HMACSHA256_ComputeHash(k_protected_key + sizeof(size_t), k_valid_key_len, k_valid_buf, k_valid_buf_len, k_valid_buffer_handle))
        .SetReturn(HMACSHA256_OK);
    STRICT_EXPECTED_CALL(BUFFER_length(k_valid_buffer_handle))
        .SetReturn(32);
    STRICT_EXPECTED_CALL(BUFFER_u_char(k_valid_buffer_handle))
        .SetReturn((unsigned char*)k_valid_hash);
    STRICT_EXPECTED_CALL(BUFFER_delete(k_valid_buffer_handle));
    STRICT_EXPECTED_CALL(RtlSecureZeroMemory((void*)(k_protected_key + sizeof(size_t)), k_valid_key_len));
    STRICT_EXPECTED_CALL(h_free((void*)k_protected_key, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    result = pal_cred_hmac_sha256(k_valid_handle, k_valid_buf, k_valid_buf_len, k_valid_sig, k_valid_sig_len);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_IS_TRUE(!memcmp(k_valid_sig, k_valid_hash, k_valid_sig_len));
}

// 
// Test pal_cred_hmac_sha256 happy path 
// 
TEST_FUNCTION(pal_win_cred_hmac_sha256__success_1)
{
    static STRING_HANDLE k_valid_handle = (STRING_HANDLE)0x234;
    static char* k_valid_target = "devíce:a1s34@something.com";

    static unsigned char* k_protected_key = (unsigned char*)UT_MEM;
    static const size_t k_valid_key_len = 32;
    static unsigned char* k_valid_buf = (unsigned char*)"0x2355";
    static const size_t k_valid_buf_len = 123;
    static void* k_valid_sig = UT_MEM + 128;
    static const size_t k_valid_sig_len = 32;
    static void* k_valid_hash = "12345678901234567890123456789012";
    static BUFFER_HANDLE k_valid_buffer_handle = (BUFFER_HANDLE)0x53535;
    CREDENTIALA valid_cred, *k_valid_cred_ptr = &valid_cred;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));
    *(size_t*)UT_MEM = k_valid_key_len;
    valid_cred.CredentialBlob = (LPBYTE)k_protected_key;
    valid_cred.CredentialBlobSize = (DWORD)k_valid_key_len;

    // arrange 
    STRICT_EXPECTED_CALL(BUFFER_new())
        .SetReturn(k_valid_buffer_handle);
    STRICT_EXPECTED_CALL(STRING_c_str(k_valid_handle))
        .SetReturn(k_valid_target);
    STRICT_EXPECTED_CALL(string_starts_with_nocase(k_valid_target, HANDLE_PREFIX))
        .SetReturn(false);

    STRICT_EXPECTED_CALL(CredReadA(k_valid_target, CRED_TYPE_GENERIC, 0, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_Credential(&k_valid_cred_ptr, sizeof(k_valid_cred_ptr))
        .SetReturn(TRUE);

    STRICT_EXPECTED_CALL(HMACSHA256_ComputeHash(k_protected_key, k_valid_key_len, k_valid_buf, k_valid_buf_len, k_valid_buffer_handle))
        .SetReturn(HMACSHA256_OK);
    STRICT_EXPECTED_CALL(BUFFER_length(k_valid_buffer_handle))
        .SetReturn(32);
    STRICT_EXPECTED_CALL(BUFFER_u_char(k_valid_buffer_handle))
        .SetReturn((unsigned char*)k_valid_hash);
    STRICT_EXPECTED_CALL(BUFFER_delete(k_valid_buffer_handle));
    STRICT_EXPECTED_CALL(RtlSecureZeroMemory((void*)k_protected_key, k_valid_key_len));
    STRICT_EXPECTED_CALL(CredFree((void*)k_valid_cred_ptr));

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
TEST_FUNCTION(pal_win_cred_hmac_sha256__arg_handle_null)
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
TEST_FUNCTION(pal_win_cred_hmac_sha256__arg_buf_null)
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
TEST_FUNCTION(pal_win_cred_hmac_sha256__arg_buf_len_invalid)
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
TEST_FUNCTION(pal_win_cred_hmac_sha256__arg_sig_null)
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
TEST_FUNCTION(pal_win_cred_hmac_sha256__arg_sig_len_invalid)
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
TEST_FUNCTION(pal_win_cred_hmac_sha256__neg_1)
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
// Test pal_cred_hmac_sha256 unhappy path 
// 
TEST_FUNCTION(pal_win_cred_hmac_sha256__neg_2)
{
    static STRING_HANDLE k_valid_handle = (STRING_HANDLE)0x234;
    static char* k_valid_target = "a1s34";
    static unsigned char* k_valid_buf = (unsigned char*)"0x2355";
    static const size_t k_valid_buf_len = 123;

    static void* k_valid_sig = UT_MEM;
    static const size_t k_valid_sig_len = 32;

    static BUFFER_HANDLE k_valid_buffer_handle = (BUFFER_HANDLE)0x53535;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange 
    STRICT_EXPECTED_CALL(BUFFER_new())
        .SetReturn(k_valid_buffer_handle);
    STRICT_EXPECTED_CALL(STRING_c_str(k_valid_handle))
        .SetReturn(k_valid_target);
    STRICT_EXPECTED_CALL(string_starts_with_nocase(k_valid_target, HANDLE_PREFIX))
        .SetReturn(false);

    STRICT_EXPECTED_CALL(CredReadA(k_valid_target, CRED_TYPE_GENERIC, 0, IGNORED_PTR_ARG))
        .IgnoreArgument(4)
        .SetReturn(FALSE);
    STRICT_EXPECTED_CALL(pal_os_last_error_as_prx_error())
        .SetReturn(er_fatal);
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
TEST_FUNCTION(pal_win_cred_hmac_sha256__neg_3)
{
    static STRING_HANDLE k_valid_handle = (STRING_HANDLE)0x234;
    static char* k_valid_target = "a1s34";
    static unsigned char* k_valid_key = (unsigned char*)"0x2d5";
    static const size_t k_valid_key_len = 32;
    static unsigned char* k_valid_buf = (unsigned char*)"0x2355";
    static const size_t k_valid_buf_len = 123;
    static void* k_valid_sig = UT_MEM;
    static const size_t k_valid_sig_len = 32;
    static BUFFER_HANDLE k_valid_buffer_handle = (BUFFER_HANDLE)0x53535;
    CREDENTIALA valid_cred, *k_valid_cred_ptr = &valid_cred;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));
    *(size_t*)UT_MEM = k_valid_key_len;
    valid_cred.CredentialBlob = (LPBYTE)k_valid_key;
    valid_cred.CredentialBlobSize = (DWORD)k_valid_key_len;

    // arrange 
    STRICT_EXPECTED_CALL(BUFFER_new())
        .SetReturn(k_valid_buffer_handle);
    STRICT_EXPECTED_CALL(STRING_c_str(k_valid_handle))
        .SetReturn(k_valid_target);
    STRICT_EXPECTED_CALL(string_starts_with_nocase(k_valid_target, HANDLE_PREFIX))
        .SetReturn(false);

    STRICT_EXPECTED_CALL(CredReadA(k_valid_target, CRED_TYPE_GENERIC, 0, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer_Credential(&k_valid_cred_ptr, sizeof(k_valid_cred_ptr))
        .SetReturn(TRUE);
    STRICT_EXPECTED_CALL(HMACSHA256_ComputeHash(k_valid_key, k_valid_key_len, k_valid_buf, k_valid_buf_len, k_valid_buffer_handle))
        .SetReturn(HMACSHA256_ERROR);
    STRICT_EXPECTED_CALL(BUFFER_delete(k_valid_buffer_handle));
    STRICT_EXPECTED_CALL(RtlSecureZeroMemory((void*)k_valid_key, k_valid_key_len));
    STRICT_EXPECTED_CALL(CredFree((void*)k_valid_cred_ptr));

    // act 
    result = pal_cred_hmac_sha256(k_valid_handle, k_valid_buf, k_valid_buf_len, k_valid_sig, k_valid_sig_len);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_invalid_format, result);
}

// 
// Test pal_cred_remove happy path 
// 
TEST_FUNCTION(pal_win_cred_remove__success_0)
{
    static STRING_HANDLE k_handle_valid = (STRING_HANDLE)0x23423;
    static const char* k_valid_target = "abcdefg";

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_handle_valid))
        .SetReturn(k_valid_target);
    STRICT_EXPECTED_CALL(string_starts_with_nocase(k_valid_target, HANDLE_PREFIX))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(CredDeleteA(k_valid_target, CRED_TYPE_GENERIC, 0))
        .SetReturn(TRUE);

    // act 
    pal_cred_remove(k_handle_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_cred_remove happy path 
// 
TEST_FUNCTION(pal_win_cred_remove__success_1)
{
    static STRING_HANDLE k_handle_valid = (STRING_HANDLE)0x23423;
    static const char* k_valid_target = "abcdefg";

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_handle_valid))
        .SetReturn(k_valid_target);
    STRICT_EXPECTED_CALL(string_starts_with_nocase(k_valid_target, HANDLE_PREFIX))
        .SetReturn(true);

    // act 
    pal_cred_remove(k_handle_valid);

    // assert
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_cred_remove passing as handle argument an invalid STRING_HANDLE value 
// 
TEST_FUNCTION(pal_win_cred_remove__arg_handle_invalid)
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
TEST_FUNCTION(pal_win_cred_remove__neg)
{
    static STRING_HANDLE k_handle_valid = (STRING_HANDLE)0x23423;
    static const char* k_valid_target = "abcdefg";

    // arrange 
    STRICT_EXPECTED_CALL(STRING_c_str(k_handle_valid))
        .SetReturn(k_valid_target);
    STRICT_EXPECTED_CALL(string_starts_with_nocase(k_valid_target, HANDLE_PREFIX))
        .SetReturn(false);
    STRICT_EXPECTED_CALL(CredDeleteA(k_valid_target, CRED_TYPE_GENERIC, 0))
        .SetReturn(FALSE);

    // act 
    pal_cred_remove(k_handle_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_cred_deinit happy path
// 
TEST_FUNCTION(pal_win_cred_deinit__success)
{
    // arrange 

    // act 
    pal_cred_deinit();

    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_cred_init happy path 
// 
TEST_FUNCTION(pal_win_cred_init__success)
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

