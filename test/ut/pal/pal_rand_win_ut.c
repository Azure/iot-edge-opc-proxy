// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.


#define UNIT_UNDER_TEST pal_rand_win
#include "util_ut.h"

//
// 1. Required mocks
//
#include "os.h"
#include "prx_types.h"

// Wincrypt.h
MOCKABLE_FUNCTION(WINAPI, BOOL, CryptAcquireContextA,
    HCRYPTPROV*, phProv, LPCSTR, szContainer, LPCSTR, szProvider, DWORD, dwProvType, DWORD, dwFlags);
MOCKABLE_FUNCTION(WINAPI, BOOL, CryptGenRandom,
    HCRYPTPROV, hProv, DWORD, dwLen, BYTE*, pbBuffer);
MOCKABLE_FUNCTION(WINAPI, BOOL, CryptReleaseContext,
    HCRYPTPROV, hProv, DWORD, dwFlags);

//
// 2. Include unit under test
//
#undef ENABLE_MOCKS
#include "pal_rand.h"
#define ENABLE_MOCKS
#include UNIT_C

//
// 3. Setup test suite
//
BEGIN_DECLARE_TEST_SUITE()
REGISTER_UMOCK_ALIAS_TYPE(HCRYPTPROV, void*);
REGISTER_UMOCK_ALIAS_TYPE(BOOL, bool);
REGISTER_UMOCK_ALIAS_TYPE(BYTE, unsigned char);
REGISTER_UMOCK_ALIAS_TYPE(DWORD, int);
REGISTER_UMOCK_ALIAS_TYPE(LPCSTR, const char*);
END_DECLARE_TEST_SUITE()
// -or- DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
DECLARE_TEST_SETUP()

//
// Test pal_rand_fill happy path
//
TEST_FUNCTION(pal_win_rand_fill__success)
{
    static const HCRYPTPROV k_ctx_valid = (HCRYPTPROV)0x2342;
    int32_t result;

    h_prov = k_ctx_valid;

    // arrange
    STRICT_EXPECTED_CALL(CryptGenRandom(k_ctx_valid, 100, (BYTE*)UT_MEM))
        .SetReturn(TRUE);

    // act
    result = pal_rand_fill(UT_MEM, 100);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

//
// Test pal_rand_fill passing as uuid argument an invalid pal_uuid_t value
//
TEST_FUNCTION(pal_win_rand_fill__arg_context_null)
{
    static const HCRYPTPROV k_ctx_valid = (HCRYPTPROV)0x2342;
    int32_t result;

    h_prov = NULL;

    // arrange
    STRICT_EXPECTED_CALL(CryptGenRandom(NULL, 100, (BYTE*)UT_MEM))
        .SetReturn(FALSE);
    STRICT_EXPECTED_CALL(pal_os_last_error_as_prx_error())
        .SetReturn(er_fault);

    // act
    result = pal_rand_fill(UT_MEM, 100);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

//
// Test pal_rand_fill passing as uuid argument an invalid pal_uuid_t value
//
TEST_FUNCTION(pal_win_rand_fill__arg_buf_null)
{
    static const HCRYPTPROV k_ctx_valid = (HCRYPTPROV)0x2342;
    int32_t result;

    h_prov = k_ctx_valid;

    // arrange
    STRICT_EXPECTED_CALL(CryptGenRandom(k_ctx_valid, 100, NULL))
        .SetReturn(FALSE);
    STRICT_EXPECTED_CALL(pal_os_last_error_as_prx_error())
        .SetReturn(er_fault);

    // act
    result = pal_rand_fill(NULL, 100);

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

//
// Test pal_rand_deinit happy path
//
TEST_FUNCTION(pal_win_rand_deinit__success_1)
{
    static const HCRYPTPROV k_ctx_valid = (HCRYPTPROV)0x2342;

    h_prov = k_ctx_valid;

    // arrange
    STRICT_EXPECTED_CALL(CryptReleaseContext(k_ctx_valid, 0))
        .SetReturn(TRUE);

    // act
    pal_rand_deinit();

    ASSERT_EXPECTED_CALLS();
}

//
// Test pal_rand_deinit happy path
//
TEST_FUNCTION(pal_win_rand_deinit__success_2)
{
    h_prov = NULL;

    // arrange

    // act
    pal_rand_deinit();

    ASSERT_EXPECTED_CALLS();
}

//
// Test pal_rand_init happy path
//
TEST_FUNCTION(pal_win_rand_init__success)
{
    static const HCRYPTPROV k_ctx_valid = (HCRYPTPROV)0x2342;
    int32_t result;

    h_prov = NULL;

    // arrange
    STRICT_EXPECTED_CALL(CryptAcquireContextA(&h_prov, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT))
        .CopyOutArgumentBuffer_phProv(&k_ctx_valid, sizeof(k_ctx_valid))
        .SetReturn(TRUE);

    // act
    result = pal_rand_init();

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(void_ptr, k_ctx_valid, h_prov);
}

//
// Test pal_rand_init unhappy path
//
TEST_FUNCTION(pal_win_rand_init__neg_1)
{
    int32_t result;

    h_prov = NULL;

    // arrange
    STRICT_EXPECTED_CALL(CryptAcquireContextA(&h_prov, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT))
        .SetReturn(FALSE);
    STRICT_EXPECTED_CALL(pal_os_last_error_as_prx_error())
        .SetReturn(er_fatal);

    // act
    result = pal_rand_init();

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fatal, result);
}

//
// Test pal_rand_init unhappy path
//
TEST_FUNCTION(pal_win_rand_init__neg_2)
{
    static const HCRYPTPROV k_ctx_valid = (HCRYPTPROV)0x2342;
    int32_t result;

    h_prov = k_ctx_valid;

    // arrange

    // act
    result = pal_rand_init();

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_bad_state, result);
}


//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

