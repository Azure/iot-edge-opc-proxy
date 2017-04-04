// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "util_signal.h"
#include "util_misc.h"
#include "pal_mt.h"
#include "pal_time.h"
#include "azure_c_shared_utility/condition.h"

typedef enum signal_state
{
    signal_state_destroyed = -1,
    signal_state_clear = 0,
    signal_state_set = 1
}
signal_state_t;

//
// Represents a signal or event
//
struct signal
{
    bool manual;
    LOCK_HANDLE lock;
    COND_HANDLE cond;
    signal_state_t state;
};

//
// Create a signal object
//
int32_t signal_create(
    bool manual,
    bool signalled,
    signal_t** created
)
{
    int32_t result = er_out_of_memory;
    signal_t* signal;

    chk_arg_fault_return(created);

    signal = mem_zalloc_type(signal_t);
    if (!signal)
        return er_out_of_memory;
    do
    {
        signal->manual = manual;
        signal->state = signalled ? signal_state_set : signal_state_clear;

        signal->cond = Condition_Init();
        if (!signal->cond)
            break;
        signal->lock = Lock_Init();
        if (!signal->lock)
            break;

        *created = signal;
        return er_ok;

    } while (0);

    signal_free(signal);
    return result;
}

//
// Wait for the signal, returns remaining timeout
//
int32_t signal_wait(
    signal_t* signal,
    int32_t timeout_ms
)
{
    int32_t time_to_wait = timeout_ms;
    return signal_wait_ex(signal, &time_to_wait);
}

//
// Wait for the signal, returns remaining timeout
//
int32_t signal_wait_ex(
    signal_t* signal,
    int32_t* timeout_ms
)
{
    int32_t result = er_fatal;
    int32_t time_to_wait;
    ticks_t time;
    COND_RESULT cond_result;

    chk_arg_fault_return(signal);
    chk_arg_fault_return(timeout_ms);

    if (*timeout_ms == 0) // Immediate
        time_to_wait = 1;
    else if (*timeout_ms == ~0) // Infinite
        time_to_wait = 0;
    else
        time_to_wait = *timeout_ms;

    time = ticks_get();

    Lock(signal->lock);
    if (signal->state == signal_state_destroyed)
        result = er_aborted; 
    else if (signal->state == signal_state_set)
    {
        // Reset event if automatic reset event
        if (!signal->manual)
            signal->state = signal_state_clear;
        result = er_ok;
    }
    else 
    {
        do
        {
            // Wait for signal
            cond_result = Condition_Wait(signal->cond, signal->lock, time_to_wait);

            time = ticks_get() - time;
            *timeout_ms = time_to_wait > (int32_t)time ? time_to_wait - (int32_t)time : 0;

            if (signal->state == signal_state_destroyed)
            {
                result = er_aborted;
                break;
            }

            if (cond_result == COND_TIMEOUT)
            {
                *timeout_ms = 0;
                result = er_timeout;
                break;
            }

            if (cond_result != COND_OK)
            {
                result = er_fatal;
                break;
            }

            if (signal->state == signal_state_set)
            {
                if (!signal->manual)
                    signal->state = signal_state_clear;
                result = er_ok;
                break;
            }

            if (*timeout_ms == 0 && time_to_wait != 0)
            {
                result = er_timeout;
                break;
            }

        } while (signal->state == signal_state_clear);
    }
    Unlock(signal->lock);
    return result;
}

//
// Set signal
//
int32_t signal_set(
    signal_t* signal
)
{
    chk_arg_fault_return(signal);
    Lock(signal->lock);
    signal->state = signal_state_set;
    Condition_Post(signal->cond);
    Unlock(signal->lock);
    return er_ok;
}

//
// Clear signal
//
int32_t signal_clear(
    signal_t* signal
)
{
    chk_arg_fault_return(signal);
    Lock(signal->lock);
    signal->state = signal_state_clear;
    Unlock(signal->lock);
    return er_ok;
}

//
// Frees signal object
//
void signal_free(
    signal_t* signal
)
{
    if (!signal)
        return;

    if (signal->cond)
    {
        // Abort current wait
        Lock(signal->lock);
        signal->state = signal_state_destroyed;
        Unlock(signal->lock);

        Condition_Post(signal->cond);

        // Then block until wait exits completely
        Lock(signal->lock);
        Unlock(signal->lock);
    }

    if (signal->lock)
        Lock_Deinit(signal->lock);
    if (signal->cond)
        Condition_Deinit(signal->cond);

    mem_free_type(signal_t, signal);
}

