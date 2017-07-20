// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST util_signal
#include "util_ut.h"

//
// 1. Required mocks
//

//
// 2. Include unit under test
//
#undef ENABLE_MOCKS
#include UNIT_H
#define ENABLE_MOCKS
#include UNIT_C

//
// Helper to modify signal state to set during wait
//
static COND_RESULT Condition_Wait__set_hook(
    COND_HANDLE handle,
    LOCK_HANDLE lock,
    int timeout_milliseconds
)
{
    (void)timeout_milliseconds;
    (void)lock;
    ((signal_t*)handle)->state = signal_state_set;
    return COND_OK;
}

//
// Helper to modify signal state to clear during wait
//
static COND_RESULT Condition_Wait__clear_hook(
    COND_HANDLE handle,
    LOCK_HANDLE lock,
    int timeout_milliseconds
)
{
    (void)timeout_milliseconds;
    (void)lock;
    ((signal_t*)handle)->state = signal_state_clear;
    return COND_TIMEOUT;
}

//
// 3. Setup test suite
//
BEGIN_DECLARE_TEST_SUITE()
REGISTER_UMOCK_ALIAS_TYPE(COND_HANDLE, void*)
REGISTER_UMOCK_ALIAS_TYPE(COND_RESULT, int)
REGISTER_UMOCK_ALIAS_TYPE(LOCK_HANDLE, void*)
REGISTER_UMOCK_ALIAS_TYPE(LOCK_RESULT, int)
REGISTER_UMOCK_ALIAS_TYPE(ticks_t, long long)
END_DECLARE_TEST_SUITE()
// -or- DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
DECLARE_TEST_SETUP()

//
// Test signal_create happy path
//
TEST_FUNCTION(signal_create__success)
{
    static const bool k_manual_valid = true;
    static const bool k_signalled_valid = true;
    static const COND_HANDLE k_condition_handle_valid = (COND_HANDLE)0x666;
    static const LOCK_HANDLE k_lock_handle_valid = (LOCK_HANDLE)0x333;
    signal_t* signal_valid;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange
    STRICT_EXPECTED_CALL(h_realloc(sizeof(signal_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    STRICT_EXPECTED_CALL(Condition_Init())
        .SetReturn(k_condition_handle_valid);
    STRICT_EXPECTED_CALL(Lock_Init())
        .SetReturn(k_lock_handle_valid);

    // act
    result = signal_create(k_manual_valid, k_signalled_valid, &signal_valid);

    // assert
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(void_ptr, (void*)signal_valid, (void*)UT_MEM);
    ASSERT_ARE_EQUAL(void_ptr, signal_valid->lock, k_lock_handle_valid);
    ASSERT_ARE_EQUAL(void_ptr, signal_valid->cond, k_condition_handle_valid);
    ASSERT_IS_TRUE(signal_valid->manual);
    ASSERT_IS_TRUE(signal_valid->state == signal_state_set);
    ASSERT_EXPECTED_CALLS();
}

//
// Test signal_create unhappy path
//
TEST_FUNCTION(signal_create__neg)
{
    static const bool k_manual_valid = false;
    static const bool k_signalled_valid = false;
    static const COND_HANDLE k_condition_handle_valid = (COND_HANDLE)0x1234;
    static const LOCK_HANDLE k_lock_handle_valid = (LOCK_HANDLE)0x4321;
    signal_t* signal_valid;
    int32_t result;

    memset(UT_MEM, 0, sizeof(UT_MEM));

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(h_realloc(sizeof(signal_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(Condition_Init())
        .SetReturn(k_condition_handle_valid)
        .SetFailReturn((COND_HANDLE)0);
    STRICT_EXPECTED_CALL(Lock_Init())
        .SetReturn(k_lock_handle_valid)
        .SetFailReturn((LOCK_HANDLE)0);

    // act
        UMOCK_C_NEGATIVE_TESTS_ACT();
        result = signal_create(k_manual_valid, k_signalled_valid, &signal_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_out_of_memory);
}

//
// Test signal_wait happy path
//
TEST_FUNCTION(signal_wait__success_10000_ms)
{
    const int32_t k_timeout_ms_valid = 10000;
    static const LOCK_HANDLE k_lock_handle_valid = (LOCK_HANDLE)0x4321;
    signal_t signal_valid;
    COND_HANDLE condition_handle_valid = (COND_HANDLE)&signal_valid;
    int32_t result;

    signal_valid.cond = condition_handle_valid;
    signal_valid.lock = k_lock_handle_valid;
    signal_valid.state = signal_state_clear;
    signal_valid.manual = false;

    REGISTER_GLOBAL_MOCK_HOOK(Condition_Wait, Condition_Wait__set_hook);
    REGISTER_GLOBAL_MOCK_RETURNS(Condition_Wait, COND_OK, COND_ERROR);

    // arrange
    STRICT_EXPECTED_CALL(ticks_get())
        .SetReturn((ticks_t)100);
    STRICT_EXPECTED_CALL(Lock(k_lock_handle_valid))
        .SetReturn(LOCK_OK);
    STRICT_EXPECTED_CALL(Condition_Wait(condition_handle_valid, k_lock_handle_valid, k_timeout_ms_valid));
    STRICT_EXPECTED_CALL(ticks_get())
        .SetReturn((ticks_t)300);
    STRICT_EXPECTED_CALL(Unlock(k_lock_handle_valid))
        .SetReturn(LOCK_OK);

    // act
    result = signal_wait(&signal_valid, k_timeout_ms_valid);

    // assert
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_EXPECTED_CALLS();
}

//
// Test signal_wait happy path
//
TEST_FUNCTION(signal_wait__success_infinite_ms)
{
    const int32_t k_timeout_ms_valid = -1;
    static const LOCK_HANDLE k_lock_handle_valid = (LOCK_HANDLE)0x6321;
    signal_t signal_valid;
    COND_HANDLE condition_handle_valid = (COND_HANDLE)&signal_valid;
    int32_t result;

    signal_valid.cond = condition_handle_valid;
    signal_valid.lock = k_lock_handle_valid;
    signal_valid.state = signal_state_clear;
    signal_valid.manual = false;

    REGISTER_GLOBAL_MOCK_HOOK(Condition_Wait, Condition_Wait__set_hook);
    REGISTER_GLOBAL_MOCK_RETURNS(Condition_Wait, COND_OK, COND_ERROR);

    // arrange
    STRICT_EXPECTED_CALL(ticks_get())
        .SetReturn((ticks_t)2222);
    STRICT_EXPECTED_CALL(Lock(k_lock_handle_valid))
        .SetReturn(LOCK_OK);
    STRICT_EXPECTED_CALL(Condition_Wait(condition_handle_valid, k_lock_handle_valid, 0));
    STRICT_EXPECTED_CALL(ticks_get())
        .SetReturn((ticks_t)2222);
    STRICT_EXPECTED_CALL(Unlock(k_lock_handle_valid))
        .SetReturn(LOCK_OK);

    // act
    result = signal_wait(&signal_valid, k_timeout_ms_valid);

    // assert
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_EXPECTED_CALLS();
}

//
// Test signal_wait happy path
//
TEST_FUNCTION(signal_wait__success_0_ms)
{
    const int32_t k_timeout_ms_valid = 0;
    static const LOCK_HANDLE k_lock_handle_valid = (LOCK_HANDLE)0x8321;
    signal_t signal_valid;
    COND_HANDLE condition_handle_valid = (COND_HANDLE)&signal_valid;
    int32_t result;

    signal_valid.cond = condition_handle_valid;
    signal_valid.lock = k_lock_handle_valid;
    signal_valid.state = signal_state_clear;
    signal_valid.manual = false;

    REGISTER_GLOBAL_MOCK_HOOK(Condition_Wait, Condition_Wait__set_hook);
    REGISTER_GLOBAL_MOCK_RETURNS(Condition_Wait, COND_OK, COND_ERROR);

    // arrange
    STRICT_EXPECTED_CALL(ticks_get())
        .SetReturn((ticks_t)2222);
    STRICT_EXPECTED_CALL(Lock(k_lock_handle_valid))
        .SetReturn(LOCK_OK);
    STRICT_EXPECTED_CALL(Condition_Wait(condition_handle_valid, k_lock_handle_valid, 1));
    STRICT_EXPECTED_CALL(ticks_get())
        .SetReturn((ticks_t)2224);
    STRICT_EXPECTED_CALL(Unlock(k_lock_handle_valid))
        .SetReturn(LOCK_OK);

    // act
    result = signal_wait(&signal_valid, k_timeout_ms_valid);

    // assert
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_EXPECTED_CALLS();
}

//
// Test signal_wait happy path
//
TEST_FUNCTION(signal_wait__success_10_ms_timeout)
{
    const int32_t k_timeout_ms_valid = 10;
    static const LOCK_HANDLE k_lock_handle_valid = (LOCK_HANDLE)0x1321;
    signal_t signal_valid;
    COND_HANDLE condition_handle_valid = (COND_HANDLE)&signal_valid;
    int32_t result;

    signal_valid.cond = condition_handle_valid;
    signal_valid.lock = k_lock_handle_valid;
    signal_valid.state = signal_state_clear;
    signal_valid.manual = false;

    REGISTER_GLOBAL_MOCK_HOOK(Condition_Wait, Condition_Wait__clear_hook);
    REGISTER_GLOBAL_MOCK_RETURNS(Condition_Wait, COND_OK, COND_ERROR);

    // arrange
    STRICT_EXPECTED_CALL(ticks_get())
        .SetReturn((ticks_t)2222);
    STRICT_EXPECTED_CALL(Lock(k_lock_handle_valid))
        .SetReturn(LOCK_OK);
    STRICT_EXPECTED_CALL(Condition_Wait(condition_handle_valid, k_lock_handle_valid, 10))
        .SetReturn(COND_TIMEOUT);
    STRICT_EXPECTED_CALL(ticks_get())
        .SetReturn((ticks_t)2222);
    STRICT_EXPECTED_CALL(Unlock(k_lock_handle_valid))
        .SetReturn(LOCK_OK);

    // act
    result = signal_wait(&signal_valid, k_timeout_ms_valid);

    // assert
    ASSERT_ARE_EQUAL(int32_t, er_timeout, result);
    ASSERT_EXPECTED_CALLS();
}

//
// Test signal_wait passing as signal argument an invalid signal_t* value
//
TEST_FUNCTION(signal_wait__arg_signal_invalid)
{
    int32_t result;

    // arrange

    // act
    result = signal_wait(NULL, 1000);

    // assert
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    ASSERT_EXPECTED_CALLS();
}

//
// Test signal_wait unhappy path
//
TEST_FUNCTION(signal_wait__neg)
{
    const int32_t k_timeout_ms_valid = 10;
    static const LOCK_HANDLE k_lock_handle_valid = (LOCK_HANDLE)0x9999;
    signal_t signal_valid;
    COND_HANDLE condition_handle_valid = (COND_HANDLE)&signal_valid;
    int32_t result;

    signal_valid.cond = condition_handle_valid;
    signal_valid.lock = k_lock_handle_valid;
    signal_valid.state = signal_state_clear;
    signal_valid.manual = false;

    REGISTER_GLOBAL_MOCK_HOOK(Condition_Wait, Condition_Wait__set_hook);
    REGISTER_GLOBAL_MOCK_RETURNS(Condition_Wait, COND_OK, COND_ERROR);

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(ticks_get())
        .SetReturn((ticks_t)2222)
        .SetFailReturn((ticks_t)0);
    STRICT_EXPECTED_CALL(Lock(k_lock_handle_valid))
        .SetReturn(LOCK_OK)
        .SetFailReturn(LOCK_ERROR);
    STRICT_EXPECTED_CALL(Condition_Wait(condition_handle_valid, k_lock_handle_valid, 10));
    STRICT_EXPECTED_CALL(ticks_get())
        .SetReturn((ticks_t)2222)
        .SetFailReturn((ticks_t)0);
    STRICT_EXPECTED_CALL(Unlock(k_lock_handle_valid))
        .SetReturn(LOCK_OK)
        .SetFailReturn(LOCK_ERROR);

    // act
        UMOCK_C_NEGATIVE_TESTS_ACT();
        result = signal_wait(&signal_valid, k_timeout_ms_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok, er_ok, er_fatal, er_ok);
}

//
// Test signal_wait_ex happy path
//
TEST_FUNCTION(signal_wait_ex__success_400_ms)
{
    const int32_t k_timeout_ms_valid = 400;
    static const LOCK_HANDLE k_lock_handle_valid = (LOCK_HANDLE)0x585858;
    signal_t signal_valid;
    COND_HANDLE condition_handle_valid = (COND_HANDLE)&signal_valid;
    int32_t timeout_ms_valid = k_timeout_ms_valid;
    int32_t result;

    signal_valid.cond = condition_handle_valid;
    signal_valid.lock = k_lock_handle_valid;
    signal_valid.state = signal_state_clear;
    signal_valid.manual = false;

    REGISTER_GLOBAL_MOCK_HOOK(Condition_Wait, Condition_Wait__set_hook);
    REGISTER_GLOBAL_MOCK_RETURNS(Condition_Wait, COND_OK, COND_ERROR);

    // arrange
    STRICT_EXPECTED_CALL(ticks_get())
        .SetReturn((ticks_t)100);
    STRICT_EXPECTED_CALL(Lock(k_lock_handle_valid))
        .SetReturn(LOCK_OK);
    STRICT_EXPECTED_CALL(Condition_Wait(condition_handle_valid, k_lock_handle_valid, k_timeout_ms_valid));
    STRICT_EXPECTED_CALL(ticks_get())
        .SetReturn((ticks_t)300);
    STRICT_EXPECTED_CALL(Unlock(k_lock_handle_valid))
        .SetReturn(LOCK_OK);

    // act
    result = signal_wait_ex(&signal_valid, &timeout_ms_valid);

    // assert
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    ASSERT_ARE_EQUAL(int32_t, 200, timeout_ms_valid);
    ASSERT_EXPECTED_CALLS();
}

//
// Test signal_wait_ex happy path
//
TEST_FUNCTION(signal_wait_ex__success_200_ms_timeout)
{
    const int32_t k_timeout_ms_valid = 400;
    static const LOCK_HANDLE k_lock_handle_valid = (LOCK_HANDLE)0x1222;
    signal_t signal_valid;
    COND_HANDLE condition_handle_valid = (COND_HANDLE)&signal_valid;
    int32_t timeout_ms_valid = k_timeout_ms_valid;
    int32_t result;

    signal_valid.cond = condition_handle_valid;
    signal_valid.lock = k_lock_handle_valid;
    signal_valid.state = signal_state_clear;
    signal_valid.manual = false;

    REGISTER_GLOBAL_MOCK_HOOK(Condition_Wait, Condition_Wait__clear_hook);
    REGISTER_GLOBAL_MOCK_RETURNS(Condition_Wait, COND_TIMEOUT, COND_ERROR);

    // arrange
    STRICT_EXPECTED_CALL(ticks_get())
        .SetReturn((ticks_t)100);
    STRICT_EXPECTED_CALL(Lock(k_lock_handle_valid))
        .SetReturn(LOCK_OK);
    STRICT_EXPECTED_CALL(Condition_Wait(condition_handle_valid, k_lock_handle_valid, k_timeout_ms_valid));
    STRICT_EXPECTED_CALL(ticks_get())
        .SetReturn((ticks_t)500);
    STRICT_EXPECTED_CALL(Unlock(k_lock_handle_valid))
        .SetReturn(LOCK_OK);

    // act
    result = signal_wait_ex(&signal_valid, &timeout_ms_valid);

    // assert
    ASSERT_ARE_EQUAL(int32_t, er_timeout, result);
    ASSERT_ARE_EQUAL(int32_t, 0, timeout_ms_valid);
    ASSERT_EXPECTED_CALLS();
}

//
// Test signal_wait_ex passing as signal argument an invalid signal_t* value
//
TEST_FUNCTION(signal_wait_ex__arg_signal_invalid)
{
    int32_t result;

    // arrange

    // act
    result = signal_wait(NULL, 1000);

    // assert
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    ASSERT_EXPECTED_CALLS();
}

//
// Test signal_wait_ex passing as timeout_ms argument an invalid int32_t* value
//
TEST_FUNCTION(signal_wait_ex__arg_timeout_ms_invalid)
{
    static const COND_HANDLE k_condition_handle_valid = (COND_HANDLE)0xbeef;
    static const LOCK_HANDLE k_lock_handle_valid = (LOCK_HANDLE)0x252000;
    signal_t signal_valid;
    int32_t result;

    signal_valid.cond = k_condition_handle_valid;
    signal_valid.lock = k_lock_handle_valid;
    signal_valid.state = signal_state_clear;
    signal_valid.manual = false;

    // arrange

    // act
    result = signal_wait_ex(&signal_valid, NULL);

    // assert
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    ASSERT_EXPECTED_CALLS();
}

//
// Test signal_wait_ex unhappy path
//
TEST_FUNCTION(signal_wait_ex__neg)
{
    const int32_t k_timeout_ms_valid = 10;
    static const LOCK_HANDLE k_lock_handle_valid = (LOCK_HANDLE)0x1110;
    signal_t signal_valid;
    COND_HANDLE condition_handle_valid = (COND_HANDLE)&signal_valid;
    int32_t timeout_ms_valid;
    int32_t result;

    signal_valid.cond = condition_handle_valid;
    signal_valid.lock = k_lock_handle_valid;
    signal_valid.state = signal_state_clear;
    signal_valid.manual = false;

    REGISTER_GLOBAL_MOCK_HOOK(Condition_Wait, Condition_Wait__clear_hook);
    REGISTER_GLOBAL_MOCK_RETURNS(Condition_Wait, COND_TIMEOUT, COND_ERROR);

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(ticks_get())
        .SetReturn((ticks_t)2222)
        .SetFailReturn((ticks_t)0);
    STRICT_EXPECTED_CALL(Lock(k_lock_handle_valid))
        .SetReturn(LOCK_OK)
        .SetFailReturn(LOCK_ERROR);
    STRICT_EXPECTED_CALL(Condition_Wait(condition_handle_valid, k_lock_handle_valid, 10));
    STRICT_EXPECTED_CALL(ticks_get())
        .SetReturn((ticks_t)2232)
        .SetFailReturn((ticks_t)0);
    STRICT_EXPECTED_CALL(Unlock(k_lock_handle_valid))
        .SetReturn(LOCK_OK)
        .SetFailReturn(LOCK_ERROR);

    // act
        UMOCK_C_NEGATIVE_TESTS_ACT();
        timeout_ms_valid = k_timeout_ms_valid;
    result = signal_wait_ex(&signal_valid, &timeout_ms_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_timeout, er_timeout, er_fatal, er_timeout);
}

//
// Test signal_set happy path
//
TEST_FUNCTION(signal_set__success)
{
    static const COND_HANDLE k_condition_handle_valid = (COND_HANDLE)0x27;
    static const LOCK_HANDLE k_lock_handle_valid = (LOCK_HANDLE)0x27;
    signal_t signal_valid;
    int32_t result;

    signal_valid.cond = k_condition_handle_valid;
    signal_valid.lock = k_lock_handle_valid;
    signal_valid.state = signal_state_clear;
    signal_valid.manual = false;

    // arrange
    STRICT_EXPECTED_CALL(Lock(k_lock_handle_valid))
        .SetReturn(LOCK_OK);
    STRICT_EXPECTED_CALL(Condition_Post(k_condition_handle_valid))
        .SetReturn(COND_OK);
    STRICT_EXPECTED_CALL(Unlock(k_lock_handle_valid))
        .SetReturn(LOCK_OK);

    // act
    result = signal_set(&signal_valid);

    // assert
    ASSERT_ARE_EQUAL(int32_t, result, er_ok);
    ASSERT_IS_TRUE(signal_valid.state == signal_state_set);
    ASSERT_EXPECTED_CALLS();
}

//
// Test signal_set passing as signal argument an invalid signal_t* value
//
TEST_FUNCTION(signal_set__arg_signal_invalid)
{
    int32_t result;

    // arrange

    // act
    result = signal_set(NULL);

    // assert
    ASSERT_ARE_EQUAL(int32_t, result, er_fault);
    ASSERT_EXPECTED_CALLS();
}

//
// Test signal_set unhappy path
//
TEST_FUNCTION(signal_set__neg)
{
    static const COND_HANDLE k_condition_handle_valid = (COND_HANDLE)0xffffff;
    static const LOCK_HANDLE k_lock_handle_valid = (LOCK_HANDLE)0xeeeeee;
    signal_t signal_valid;
    int32_t result;

    signal_valid.cond = k_condition_handle_valid;
    signal_valid.lock = k_lock_handle_valid;
    signal_valid.state = signal_state_clear;
    signal_valid.manual = false;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    STRICT_EXPECTED_CALL(Lock(k_lock_handle_valid))
        .SetReturn(LOCK_OK)
        .SetFailReturn(LOCK_ERROR);
    STRICT_EXPECTED_CALL(Condition_Post(k_condition_handle_valid))
        .SetReturn(COND_OK)
        .SetFailReturn(COND_ERROR);
    STRICT_EXPECTED_CALL(Unlock(k_lock_handle_valid))
        .SetReturn(LOCK_OK)
        .SetFailReturn(LOCK_ERROR);

    // act
        UMOCK_C_NEGATIVE_TESTS_ACT();
        result = signal_set(&signal_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

//
// Test signal_clear happy path
//
TEST_FUNCTION(signal_clear__success)
{
    static const COND_HANDLE k_condition_handle_valid = (COND_HANDLE)0x28;
    static const LOCK_HANDLE k_lock_handle_valid = (LOCK_HANDLE)0x28;
    signal_t signal_valid;
    int32_t result;

    signal_valid.cond = k_condition_handle_valid;
    signal_valid.lock = k_lock_handle_valid;
    signal_valid.state = signal_state_set;
    signal_valid.manual = false;

    // arrange
    STRICT_EXPECTED_CALL(Lock(k_lock_handle_valid))
        .SetReturn(LOCK_OK);
    STRICT_EXPECTED_CALL(Unlock(k_lock_handle_valid))
        .SetReturn(LOCK_OK);

    // act
    result = signal_clear(&signal_valid);

    // assert
    ASSERT_ARE_EQUAL(int32_t, result, er_ok);
    ASSERT_IS_TRUE(signal_valid.state == signal_state_clear);
    ASSERT_EXPECTED_CALLS();
}

//
// Test signal_clear passing as signal argument an invalid signal_t* value
//
TEST_FUNCTION(signal_clear__arg_signal_invalid)
{
    int32_t result;

    // arrange

    // act
    result = signal_clear(NULL);

    // assert
    ASSERT_ARE_EQUAL(int32_t, result, er_fault);
    ASSERT_EXPECTED_CALLS();
}

//
// Test signal_free happy path
//
TEST_FUNCTION(signal_free__success)
{
    static const COND_HANDLE k_condition_handle_valid = (COND_HANDLE)0xfee;
    static const LOCK_HANDLE k_lock_handle_valid = (LOCK_HANDLE)0xeef;
    signal_t signal_valid;

    signal_valid.cond = k_condition_handle_valid;
    signal_valid.lock = k_lock_handle_valid;
    signal_valid.state = signal_state_set;
    signal_valid.manual = false;

    // arrange
    STRICT_EXPECTED_CALL(Lock(k_lock_handle_valid))
        .SetReturn(LOCK_OK);
    STRICT_EXPECTED_CALL(Unlock(k_lock_handle_valid))
        .SetReturn(LOCK_OK);
    STRICT_EXPECTED_CALL(Condition_Post(k_condition_handle_valid))
        .SetReturn(COND_OK);
    STRICT_EXPECTED_CALL(Lock(k_lock_handle_valid))
        .SetReturn(LOCK_OK);
    STRICT_EXPECTED_CALL(Unlock(k_lock_handle_valid))
        .SetReturn(LOCK_OK);
    STRICT_EXPECTED_CALL(Lock_Deinit(k_lock_handle_valid))
        .SetReturn(LOCK_OK);
    STRICT_EXPECTED_CALL(Condition_Deinit(k_condition_handle_valid));
    STRICT_EXPECTED_CALL(h_free((void*)&signal_valid, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act
    signal_free(&signal_valid);

    // assert
    ASSERT_IS_TRUE(signal_valid.state == signal_state_destroyed);
    ASSERT_EXPECTED_CALLS();
}

//
// Test signal_free passing as signal argument an invalid signal_t* value
//
TEST_FUNCTION(signal_free__arg_signal_invalid)
{
    // arrange

    // act
    signal_free(NULL);

    // assert
    ASSERT_EXPECTED_CALLS();
}


//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

