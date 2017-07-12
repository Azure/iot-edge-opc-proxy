// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST pal_mt_win
#include "util_ut.h"

//
// 1. Required mocks
//
#include "os.h"

#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/condition.h"

// synchapi.h
MOCKABLE_FUNCTION(WINBASEAPI, void, InitializeSRWLock,
    PSRWLOCK, SRWLock);
MOCKABLE_FUNCTION(WINBASEAPI, void, ReleaseSRWLockExclusive,
    PSRWLOCK, SRWLock);
MOCKABLE_FUNCTION(WINBASEAPI, void, ReleaseSRWLockShared,
    PSRWLOCK, SRWLock);
MOCKABLE_FUNCTION(WINBASEAPI, void, AcquireSRWLockExclusive,
    PSRWLOCK, SRWLock);
MOCKABLE_FUNCTION(WINBASEAPI, void, AcquireSRWLockShared,
    PSRWLOCK, SRWLock);

// processthreadsapi.h
MOCKABLE_FUNCTION(WINBASEAPI, DWORD, GetCurrentThreadId);

//
// 2. Include unit under test
//
#undef ENABLE_MOCKS
#include "pal_mt.h"
#define ENABLE_MOCKS
#include UNIT_C

//
// 3. Setup test suite
//
BEGIN_DECLARE_TEST_SUITE()
REGISTER_UMOCK_ALIAS_TYPE(DWORD, int);
REGISTER_UMOCK_ALIAS_TYPE(lock_t, void*);
REGISTER_UMOCK_ALIAS_TYPE(rwlock_t, void*);
REGISTER_UMOCK_ALIAS_TYPE(tid_t, long);
REGISTER_UMOCK_ALIAS_TYPE(LOCK_HANDLE, void*);
REGISTER_UMOCK_ALIAS_TYPE(LOCK_RESULT, int);
REGISTER_UMOCK_ALIAS_TYPE(PSRWLOCK, void*);
END_DECLARE_TEST_SUITE()
// -or- DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
DECLARE_TEST_SETUP()

// 
// Test lock_create happy path 
// 
TEST_FUNCTION(pal_win_lock_create__success)
{
    static const LOCK_HANDLE k_lock_handle_valid = (LOCK_HANDLE)0x26;
    lock_t created_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(Lock_Init())
        .SetReturn(k_lock_handle_valid);

    // act 
    result = lock_create(&created_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(void_ptr, k_lock_handle_valid, created_valid);
}

// 
// Test lock_create passing as created argument an invalid lock_t* value 
// 
TEST_FUNCTION(pal_win_lock_create__arg_created_null)
{
    int32_t result;

    // arrange 

    // act 
    result = lock_create(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test lock_create unhappy path 
// 
TEST_FUNCTION(pal_win_lock_create__neg)
{
    lock_t created_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(Lock_Init())
        .SetReturn(NULL);

    // act 
    result = lock_create(&created_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_out_of_memory, result);
}

// 
// Test lock_enter happy path 
// 
TEST_FUNCTION(pal_win_lock_enter__success)
{
    static const lock_t k_lock_valid = (lock_t)0x23423;

    // arrange 
    STRICT_EXPECTED_CALL(Lock((LOCK_HANDLE)k_lock_valid))
        .SetReturn(LOCK_OK);

    // act 
    lock_enter(k_lock_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test lock_exit happy path 
// 
TEST_FUNCTION(pal_win_lock_exit__success)
{
    static const lock_t k_lock_valid = (lock_t)0x83456;

    // arrange 
    STRICT_EXPECTED_CALL(Unlock((LOCK_HANDLE)k_lock_valid))
        .SetReturn(LOCK_OK);

    // act 
    lock_exit(k_lock_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test lock_free happy path 
// 
TEST_FUNCTION(pal_win_lock_free__success)
{
    static const lock_t k_lock_valid = (lock_t)0x7423;

    // arrange 
    STRICT_EXPECTED_CALL(Lock_Deinit((LOCK_HANDLE)k_lock_valid))
        .SetReturn(LOCK_OK);

    // act 
    lock_free(k_lock_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test rw_lock_create happy path 
// 
TEST_FUNCTION(pal_win_rw_lock_create__success)
{
    rw_lock_t created_valid;
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(SRWLOCK), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(InitializeSRWLock((SRWLOCK*)UT_MEM));

    // act 
    result = rw_lock_create(&created_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(void_ptr, UT_MEM, created_valid);
}

// 
// Test rw_lock_create passing as created argument an invalid rw_lock_t* value 
// 
TEST_FUNCTION(pal_win_rw_lock_create__arg_created_null)
{
    int32_t result;

    // arrange 

    // act 
    result = rw_lock_create(NULL);

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
}

// 
// Test rw_lock_create unhappy path 
// 
TEST_FUNCTION(pal_win_rw_lock_create__neg)
{
    rw_lock_t created_valid;
    int32_t result;

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(h_realloc(sizeof(SRWLOCK), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(InitializeSRWLock((SRWLOCK*)UT_MEM));

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = rw_lock_create(&created_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_out_of_memory, er_ok);
}

// 
// Test rw_lock_enter happy path 
// 
TEST_FUNCTION(pal_win_rw_lock_enter__success)
{
    static const rw_lock_t k_lock_valid = (rw_lock_t)0x26234;

    // arrange 
    STRICT_EXPECTED_CALL(AcquireSRWLockShared((SRWLOCK*)k_lock_valid));

    // act 
    rw_lock_enter(k_lock_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test rw_lock_exit happy path 
// 
TEST_FUNCTION(pal_win_rw_lock_exit__success)
{
    static const rw_lock_t k_lock_valid = (rw_lock_t)0x26234;

    // arrange 
    STRICT_EXPECTED_CALL(ReleaseSRWLockShared((SRWLOCK*)k_lock_valid));

    // act 
    rw_lock_exit(k_lock_valid);

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test rw_lock_enter_w happy path 
// 
TEST_FUNCTION(pal_win_rw_lock_enter_w__success)
{
    static const rw_lock_t k_lock_valid = (rw_lock_t)0x266344;

    // arrange 
    STRICT_EXPECTED_CALL(AcquireSRWLockExclusive((SRWLOCK*)k_lock_valid));

    // act 
    rw_lock_enter_w(k_lock_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test rw_lock_exit_w happy path 
// 
TEST_FUNCTION(pal_win_rw_lock_exit_w__success)
{
    static const rw_lock_t k_lock_valid = (rw_lock_t)0x266344;

    // arrange 
    STRICT_EXPECTED_CALL(ReleaseSRWLockExclusive((SRWLOCK*)k_lock_valid));

    // act 
    rw_lock_exit_w(k_lock_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test rw_lock_free happy path 
// 
TEST_FUNCTION(pal_win_rw_lock_free__success)
{
    static const rw_lock_t k_lock_valid = (rw_lock_t)0x266344;

    // arrange 
    STRICT_EXPECTED_CALL(h_free((void*)k_lock_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    rw_lock_free(k_lock_valid);

    // assert 
    ASSERT_EXPECTED_CALLS();
}


//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

