// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os.h"
#include "util_mem.h"
#include "pal_mt.h"

#include "azure_c_shared_utility/lock.h"

//
// Returns current thread id
//
tid_t tid_self(
    void
)
{
    return (tid_t)GetCurrentThreadId();
}

//
// Returns whether 2 thread ids are equal
//
bool tid_equal(
    tid_t left,
    tid_t right
)
{
    return left == right;
}

//
// Create reader/writer lock
//
int32_t rw_lock_create(
    rw_lock_t* created
)
{
    SRWLOCK* lock;
    chk_arg_fault_return(created);
    lock = mem_zalloc_type(SRWLOCK);
    if (!lock)
        return er_out_of_memory;
    InitializeSRWLock(lock);
    *created = lock;
    return er_ok;
}

//
// Lock reader/writer lock
//
void rw_lock_enter(
    rw_lock_t lock
)
{
    dbg_assert_ptr(lock);
    AcquireSRWLockShared((SRWLOCK*)lock);
}

//
// Unlock reader/writer lock
//
void rw_lock_exit(
    rw_lock_t lock
)
{
    dbg_assert_ptr(lock);
    ReleaseSRWLockShared((SRWLOCK*)lock);
}

//
// Lock reader/writer lock for writing
//
void rw_lock_enter_w(
    rw_lock_t lock
)
{
    dbg_assert_ptr(lock);
    AcquireSRWLockExclusive((SRWLOCK*)lock);
}

//
// Unlock reader/writer lock as writer
//
void rw_lock_exit_w(
    rw_lock_t lock
)
{
    dbg_assert_ptr(lock);
    __analysis_suppress(26110)
    ReleaseSRWLockExclusive((SRWLOCK*)lock);
}

//
// Destroy reader/writer lock
//
void rw_lock_free(
    rw_lock_t lock
)
{
    dbg_assert_ptr(lock);
    mem_free_type(SRWLOCK, lock);
}

//
// Create non-reentrant lock
//
int32_t lock_create(
    lock_t* created
)
{
#if defined(DEBUG) && !defined(UNIT_TEST)
    //
    // In debug use rw lock to catch reentrancy issues since
    // critical section lock implementation is reentrant. Use
    // cs in non debug builds for speed.
    //
    return rw_lock_create((rw_lock_t*)created);
#else
    int32_t result;
    LOCK_HANDLE lock;
    chk_arg_fault_return(created);
    do
    {
        /* EnterCriticalSection lock is reentrant */
        lock = Lock_Init();
        if (!lock)
        {
            result = er_out_of_memory;
            break;
        }
        *created = (lock_t)lock;
        return er_ok;
    }
    while (0);
    return result;
#endif
}

//
// Lock reentrant lock
//
void lock_enter(
    lock_t lock
)
{
    dbg_assert_ptr(lock);
#if defined(DEBUG) && !defined(UNIT_TEST)
    rw_lock_enter_w((rw_lock_t)lock);
#else
    (void)Lock((LOCK_HANDLE)lock);
#endif
}

//
// Unlock reentrant lock
//
void lock_exit(
    lock_t lock
)
{
    dbg_assert_ptr(lock);
#if defined(DEBUG) && !defined(UNIT_TEST)
    rw_lock_exit_w((rw_lock_t)lock);
#else
    (void)Unlock((LOCK_HANDLE)lock);
#endif
}

//
// Destroy reentrant lock
//
void lock_free(
    lock_t lock
)
{
    dbg_assert_ptr(lock);
#if defined(DEBUG) && !defined(UNIT_TEST)
    rw_lock_free((rw_lock_t)lock);
#else
    Lock_Deinit((LOCK_HANDLE)lock);
#endif
}

