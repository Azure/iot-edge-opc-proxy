// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _pal_mt_h
#define _pal_mt_h

#include "common.h"

#if !defined(__PAL_ST)
//
// Atomic set and compare using intrinsics.  These are here not in
// os_xxx.h so that they can be used in unit tests, since they are
// effectively compiler emitted, not operating system dependent.
//
#if defined(_WIN32) 
typedef volatile long atomic_t;

#include <intrin.h>
#define set_atomic(var, val) \
    _InterlockedExchange((atomic_t*)&(var), (atomic_t)(val))
#define set_atomic_ptr(ptr, val) \
    _InterlockedExchangePointer((volatile PVOID*)&(ptr), (PVOID)(val))
#define atomic_inc(var) \
    _InterlockedIncrement((atomic_t*)&(var))
#define atomic_dec(var) \
    _InterlockedDecrement((atomic_t*)&(var))
#define atomic_bit_set(var, index) \
    _interlockedbittestandset((atomic_t*)&(var), index)
#define atomic_bit_clear(var, index) \
    _interlockedbittestandreset((atomic_t*)&(var), index)

#elif defined(__GNUC__)
typedef volatile long atomic_t;

#if (__GNUC__ >= 4) && (__GNUC_MINOR__ >= 7) // Use C'11 atomics

#define set_atomic(var, val) \
    __atomic_exchange_n(&(var), val, __ATOMIC_ACQ_REL)
#define set_atomic_ptr(ptr, val) \
    __atomic_exchange_n(&(ptr), val, __ATOMIC_ACQ_REL)
#define atomic_inc(var) \
    __atomic_fetch_add(&(var), 1, __ATOMIC_ACQ_REL)
#define atomic_dec(var) \
    __atomic_sub_fetch(&(var), 1,__ATOMIC_ACQ_REL)
#define atomic_bit_set(var, index) \
    __atomic_fetch_or(&(var), (1 << index), __ATOMIC_ACQ_REL) & (1 << index)
#define atomic_bit_clear(var, index) \
    __atomic_fetch_and(&(var), ~(1 << index), __ATOMIC_ACQ_REL) & (1 << index)

#else // Use legacy atomic ops

#define set_atomic(var, val) \
    __sync_lock_test_and_set(&(var), val); __sync_lock_release (&(var))
#define set_atomic_ptr(ptr, val) \
    __sync_lock_test_and_set(&(ptr), val); __sync_lock_release (&(ptr))
#define atomic_inc(var) \
    __sync_fetch_and_add (&(var), 1)
#define atomic_dec(var) \
    __sync_sub_and_fetch (&(var), 1)
#define atomic_bit_set(var, index) \
    __sync_fetch_and_or(&(var), (1 << index)) & (1 << index)
#define atomic_bit_clear(var, index) \
    __sync_fetch_and_and(&(var), ~(1 << index)) & (1 << index)

#endif // __GNUC > 4.7.0

#elif !defined (UNIT_TEST) // pal
typedef volatile long atomic_t;

#define set_atomic(var, val) \
    pal_atomic_exchange((atomic_t*)(&var), (atomic_t)(val))
#define set_atomic_ptr(ptr, val) \
    pal_atomic_exchange_ptr((volatile void**)(&ptr), (void*)(val))
#define atomic_inc(var) \
    pal_atomic_inc((atomic_t*)&(var))
#define atomic_dec(var) \
    pal_atomic_dec((atomic_t*)&(var))
#define atomic_bit_set(var, index) \
    pal_atomic_bit_set((atomic_t*)&(var), index)
#define atomic_bit_clear(var, index) \
    pal_atomic_bit_clear((atomic_t*)&(var), index)

#else // UNIT_TEST
#define __PAL_ST 1
#endif 
#endif // !__PAL_ST

#if defined(__PAL_ST)
//
// Single threaded - no need for atomics
//
typedef volatile int atomic_t;

#define set_atomic(var, val) \
    var = val
#define set_atomic_ptr(ptr, val) \
    ptr = val
#define atomic_inc(var) \
    var++
#define atomic_dec(var) \
    var--
#define atomic_bit_set(var, index) \
    ((var) & (1 << index)); (var) |= (1 << index)
#define atomic_bit_clear(var, index) \
    ((var) & (1 << index)); (var) &= ~(1 << index)

#endif // !__PAL_ST

// 
// Thread id
//
typedef intptr_t tid_t;

//
// Reader/Writer locks
//
typedef void* rw_lock_t;

// 
// Simple, non-reentrant mutexes
//
typedef void* lock_t;

//
// Returns current thread id
//
decl_internal_0(tid_t, tid_self,
    void
);

//
// Returns whether 2 thread ids are equal
//
decl_internal_2(bool, tid_equal,
    tid_t, left,
    tid_t, right
);

//
// Create mutex lock
//
decl_internal_1(int32_t, lock_create,
    lock_t*, created
);

//
// Lock mutex lock
//
decl_internal_1(void, lock_enter,
    lock_t, lock
);

//
// Unlock mutex lock
//
decl_internal_1(void, lock_exit,
    lock_t, lock
);

//
// Destroy mutex lock
//
decl_internal_1(void, lock_free,
    lock_t, lock
);

//
// Create reader/writer lock
//
decl_internal_1(int32_t, rw_lock_create,
    rw_lock_t*, created
);

//
// Lock reader/writer lock
//
decl_internal_1(void, rw_lock_enter,
    rw_lock_t, lock
);

//
// Unlock reader/writer lock
//
decl_internal_1(void, rw_lock_exit,
    rw_lock_t, lock
);

//
// Lock reader/writer lock for writing
//
decl_internal_1(void, rw_lock_enter_w,
    rw_lock_t, lock
);

//
// Unlock reader/writer lock as writer
//
decl_internal_1(void, rw_lock_exit_w,
    rw_lock_t, lock
);

//
// Destroy reader/writer lock
//
decl_internal_1(void, rw_lock_free,
    rw_lock_t, lock
);

#endif // _pal_mt_h