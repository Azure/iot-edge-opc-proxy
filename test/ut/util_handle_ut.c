// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST util_handle
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
// 3. Setup test suite
//
BEGIN_DECLARE_TEST_SUITE()
REGISTER_UMOCK_ALIAS_TYPE(PDLIST_ENTRY, void*);
REGISTER_UMOCK_ALIAS_TYPE(const PDLIST_ENTRY, const void*);
REGISTER_UMOCK_ALIAS_TYPE(lock_t, void*);
END_DECLARE_TEST_SUITE()
// -or- DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
BEGIN_DECLARE_TEST_SETUP()
handle_map = NULL;
END_DECLARE_TEST_SETUP()


// 
// Test handle_map_init happy path 
// 
TEST_FUNCTION(handle_map_init__success)
{
    int32_t result;

    // arrange 
    STRICT_EXPECTED_CALL(h_realloc(sizeof(handle_map_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM);
    EXPECTED_CALL(DList_InitializeListHead(IGNORED_PTR_ARG));
    EXPECTED_CALL(lock_create(IGNORED_PTR_ARG))
        .SetReturn(er_ok);

    // act 
    result = handle_map_init();

    // assert 
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void_ptr, handle_map, (void*)UT_MEM);
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
}

// 
// Test handle_map_init unhappy path 
// 
TEST_FUNCTION(handle_map_init__neg)
{
    int32_t result;

    // arrange 
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    memset(UT_MEM, 0, sizeof(UT_MEM));
    STRICT_EXPECTED_CALL(h_realloc(sizeof(handle_map_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)UT_MEM)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(DList_InitializeListHead(IGNORED_PTR_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(lock_create(IGNORED_PTR_ARG))
        .IgnoreArgument(1)
        .SetReturn(er_ok)
        .SetFailReturn(er_out_of_memory);
    STRICT_EXPECTED_CALL(DList_IsListEmpty(IGNORED_PTR_ARG))
        .IgnoreArgument(1)
        .SetReturn(true)
        .SetFailReturn(true);
    STRICT_EXPECTED_CALL(h_free((void*)UT_MEM, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    UMOCK_C_NEGATIVE_TESTS_ACT();
    handle_map = NULL;
    result = handle_map_init();

    // assert 
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_out_of_memory, er_ok, er_out_of_memory, er_ok);
}

// 
// Test handle_map_get_handle happy path 
// 
TEST_FUNCTION(handle_map_get_handle__success)
{
    static const int32_t k_handle_valid = 34;
    static const void* k_pointer_valid = (void*)43;
    static const lock_t k_lock_valid = (lock_t)88;
    handle_map_t handle_map_valid;
    handle_t handle_valid;
    int32_t result;

    handle_map = &handle_map_valid;
    handle_map->last_id = k_handle_valid - 1;
    handle_map->lock = k_lock_valid;

    // arrange 
    STRICT_EXPECTED_CALL(lock_enter(k_lock_valid));
    STRICT_EXPECTED_CALL(h_realloc(sizeof(handle_valid), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn((void*)&handle_valid);
    STRICT_EXPECTED_CALL(DList_InsertTailList(&handle_map_valid.handles, &handle_valid.link));
    STRICT_EXPECTED_CALL(lock_exit(k_lock_valid));

    // act 
    result = handle_map_get_handle(k_pointer_valid);

    // assert 
    ASSERT_ARE_EQUAL(int32_t, result, k_handle_valid);
    ASSERT_ARE_EQUAL(int32_t, handle_valid.id, k_handle_valid);
    ASSERT_ARE_EQUAL(void_ptr, handle_valid.pointer, k_pointer_valid);
    ASSERT_EXPECTED_CALLS();
}

// 
// Test handle_map_get_handle passing as pointer argument an invalid void* value 
// 
TEST_FUNCTION(handle_map_get_handle__arg_pointer_invalid)
{
    static const void* k_pointer_invalid = NULL;
    static const lock_t k_lock_valid = (lock_t)3323456;

    handle_map_t handle_map_valid;
    int32_t result;

    handle_map = &handle_map_valid;
    handle_map->lock = k_lock_valid;

    // arrange 

    // act 
    result = handle_map_get_handle(k_pointer_invalid);

    // assert 
    ASSERT_ARE_EQUAL(int32_t, handle_map_invalid_handle, result);
    ASSERT_EXPECTED_CALLS();
}

// 
// Test handle_map_get_handle unhappy path 
// 
TEST_FUNCTION(handle_map_get_handle__neg_alloc_fail)
{
    static const int32_t k_handle_valid = 32354223;
    static const void* k_pointer_valid = (void*)23445;
    static const lock_t k_lock_valid = (lock_t)88;
    handle_map_t handle_map_valid;
    int32_t result;

    handle_map = &handle_map_valid;
    handle_map->last_id = k_handle_valid - 1;
    handle_map->lock = k_lock_valid;

    // arrange 
    STRICT_EXPECTED_CALL(lock_enter(k_lock_valid));
    STRICT_EXPECTED_CALL(h_realloc(sizeof(handle_t), NULL, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(4).IgnoreArgument(5).IgnoreArgument(6)
        .SetReturn(NULL);
    STRICT_EXPECTED_CALL(lock_exit(k_lock_valid));

    // act 
    result = handle_map_get_handle(k_pointer_valid);

    // assert 
    ASSERT_ARE_EQUAL(int32_t, handle_map_invalid_handle, result);
    ASSERT_EXPECTED_CALLS();
}

// 
// Test handle_map_get_handle unhappy path 
// 
TEST_FUNCTION(handle_map_get_handle__neg_not_init)
{
    static const void* k_pointer_valid = (void*)254367;
    int32_t result;

    // act 
    result = handle_map_get_handle(k_pointer_valid);

    // assert 
    ASSERT_ARE_EQUAL(int32_t, handle_map_invalid_handle, result);
    ASSERT_EXPECTED_CALLS();
}

// 
// Test handle_map_get_pointer happy path 
// 
TEST_FUNCTION(handle_map_get_pointer__success)
{
    static const int32_t k_handle_valid = 23334;
    static const void* k_pointer_valid = (void*)2345;
    static const lock_t k_lock_valid = (lock_t)3323456;

    handle_map_t handle_map_valid;
    handle_t handle_valid;
    const void* result;

    handle_map = &handle_map_valid;
    handle_map->lock = k_lock_valid;
    handle_valid.id = k_handle_valid;
    handle_valid.pointer = k_pointer_valid;
    handle_map->handles.Flink = handle_map->handles.Blink = &handle_valid.link;
    handle_valid.link.Flink = handle_valid.link.Blink = &handle_map->handles;

    // arrange 
    STRICT_EXPECTED_CALL(lock_enter(k_lock_valid));
    STRICT_EXPECTED_CALL(lock_exit(k_lock_valid));

    // act 
    result = handle_map_get_pointer(k_handle_valid);

    // assert 
    ASSERT_ARE_EQUAL(void_ptr, result, k_pointer_valid);
    ASSERT_EXPECTED_CALLS();
}

// 
// Test handle_map_get_pointer passing as handle argument an invalid int32_t value 
// 
TEST_FUNCTION(handle_map_get_pointer__arg_handle_invalid)
{
    static const int32_t k_handle_invalid = handle_map_invalid_handle;
    static const lock_t k_lock_valid = (lock_t)3323456;

    handle_map_t handle_map_valid;
    const void* result;

    handle_map = &handle_map_valid;
    handle_map->lock = k_lock_valid;

    // arrange 

    // act 
    result = handle_map_get_pointer(k_handle_invalid);

    // assert 
    ASSERT_ARE_EQUAL(void_ptr, NULL, result);
    ASSERT_EXPECTED_CALLS();
}

// 
// Test handle_map_get_pointer unhappy path 
// 
TEST_FUNCTION(handle_map_get_pointer__neg_not_init)
{
    static const int32_t k_handle_valid = 44;
    const void* result;

    // arrange 

    // act 
    result = handle_map_get_pointer(k_handle_valid);

    // assert 
    ASSERT_ARE_EQUAL(void_ptr, NULL, result);
    ASSERT_EXPECTED_CALLS();
}

// 
// Test handle_map_remove_handle happy path 
// 
TEST_FUNCTION(handle_map_remove_handle__success)
{
    static const int32_t k_handle_valid = 445;
    static const void* k_pointer_valid = (void*)112;
    static const lock_t k_lock_valid = (lock_t)334;

    handle_map_t handle_map_valid;
    handle_t handle_valid1;
    handle_t handle_valid2;
    const void* result;

    memset(&handle_map_valid, 0xff, sizeof(handle_map_valid));
    memset(&handle_valid1, 0xff, sizeof(handle_valid1));
    memset(&handle_valid2, 0xff, sizeof(handle_valid2));

    handle_map = &handle_map_valid;
    handle_map->lock = k_lock_valid;
    handle_valid2.id = k_handle_valid;
    handle_valid2.pointer = k_pointer_valid;

    handle_map->handles.Flink = &handle_valid1.link;
    handle_valid1.link.Blink = &handle_map->handles;
    handle_valid1.link.Flink = &handle_valid2.link;
    handle_valid2.link.Blink = &handle_valid1.link;
    handle_valid2.link.Flink = &handle_map->handles;
    handle_map->handles.Blink = &handle_valid2.link;

    // arrange 
    STRICT_EXPECTED_CALL(lock_enter(k_lock_valid));
    STRICT_EXPECTED_CALL(DList_RemoveEntryList(&handle_valid2.link));
    STRICT_EXPECTED_CALL(lock_exit(k_lock_valid));
    STRICT_EXPECTED_CALL(h_free((void*)&handle_valid2, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .IgnoreArgument(2).IgnoreArgument(3).IgnoreArgument(4);

    // act 
    result = handle_map_remove_handle(k_handle_valid);

    // assert 
    ASSERT_ARE_EQUAL(void_ptr, k_pointer_valid, result);
    ASSERT_EXPECTED_CALLS();
}

// 
// Test handle_map_remove_handle passing as handle argument an invalid int32_t value 
// 
TEST_FUNCTION(handle_map_remove_handle__arg_handle_invalid)
{
    static const int32_t k_handle_invalid = handle_map_invalid_handle;
    static const lock_t k_lock_valid = (lock_t)434;

    handle_map_t handle_map_valid;
    const void* result;

    handle_map = &handle_map_valid;
    handle_map->lock = k_lock_valid;

    // arrange 

    // act 
    result = handle_map_remove_handle(k_handle_invalid);

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test handle_map_remove_handle unhappy path 
// 
TEST_FUNCTION(handle_map_remove_handle__not_init)
{
    static const int32_t k_handle_valid = 77;
    const void* result;

    // arrange 

    // act 
    result = handle_map_remove_handle(k_handle_valid);

    // assert 
    ASSERT_ARE_EQUAL(void_ptr, 0, result);
    ASSERT_EXPECTED_CALLS();
}

// 
// Test handle_map_deinit happy path 
// 
TEST_FUNCTION(handle_map_deinit__success)
{
    // arrange 
    // ... 

    // act 
    handle_map_deinit();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}


//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

