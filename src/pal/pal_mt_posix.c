// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os.h"
#include "util_mem.h"
#include "pal_mt.h"
#include "util_string.h"
#include "azure_c_shared_utility/lock.h"

#include <stdio.h>
#if !defined(UNIT_TEST)
#include "pthread.h"
#endif

//
// Returns current thread id
//
tid_t tid_self(
    void
)
{
    return (tid_t)pthread_self();
}

//
// Returns whether 2 thread ids are equal
//
bool tid_equal(
    tid_t left,
    tid_t right
)
{
    return 0 != pthread_equal((pthread_t)left, (pthread_t)right);
}

//
// Create reader/writer lock
//
int32_t rw_lock_create(
    rw_lock_t* created
)
{
    int rc;
    if (!created)
        return er_fault;
    pthread_rwlock_t* lock = mem_zalloc_type(pthread_rwlock_t);
    if (!lock)
        return er_out_of_memory;
    rc = pthread_rwlock_init(lock, NULL);
    if (rc != 0)
    {
        mem_free_type(pthread_rwlock_t, lock);
        return er_fatal;
    }
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
    if (0 != pthread_rwlock_rdlock((pthread_rwlock_t*)lock))
    {
        dbg_assert(0, "Failed to lock");
    }
}

//
// Unlock reader/writer lock
//
void rw_lock_exit(
    rw_lock_t lock
)
{
    dbg_assert_ptr(lock);
    if (0 != pthread_rwlock_unlock((pthread_rwlock_t*)lock))
    {
        dbg_assert(0, "Failed to unlock");
    }
}

//
// Lock reader/writer lock for writing
//
void rw_lock_enter_w(
    rw_lock_t lock
)
{
    dbg_assert_ptr(lock);
    if (0 != pthread_rwlock_wrlock((pthread_rwlock_t*)lock))
    {
        dbg_assert(0, "Failed to lock writer lock");
    }
}

//
// Unlock reader/writer lock as writer
//
void rw_lock_exit_w(
    rw_lock_t lock
)
{
    dbg_assert_ptr(lock);
    if (0 != pthread_rwlock_unlock((pthread_rwlock_t*)lock))
    {
        dbg_assert(0, "Failed to unlock writer lock");
    }
}

//
// Destroy reader/writer lock
//
void rw_lock_free(
    rw_lock_t lock
)
{
    dbg_assert_ptr(lock);
    pthread_rwlock_destroy((pthread_rwlock_t*)lock);
    mem_free_type(pthread_rwlock_t, lock);
}

//
// Create non-reentrant lock
//
int32_t lock_create(
    lock_t* created
)
{
    int32_t result;
    LOCK_HANDLE lock;
    if (!created)
        return er_fault;
    do
    {
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
}

//
// Lock reentrant lock
//
void lock_enter(
    lock_t lock
)
{
    dbg_assert_ptr(lock);
    (void)Lock((LOCK_HANDLE)lock);
}

//
// Unlock reentrant lock
//
void lock_exit(
    lock_t lock
)
{
    dbg_assert_ptr(lock);
    (void)Unlock((LOCK_HANDLE)lock);
}

//
// Destroy reentrant lock
//
void lock_free(
    lock_t lock
)
{
    dbg_assert_ptr(lock);
    Lock_Deinit((LOCK_HANDLE)lock);
}

