// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "pal_time.h"
#include "azure_c_shared_utility/tickcounter.h"

//
// Pal wrapper
//
TICK_COUNTER_HANDLE tickcounter_create(
    void
)
{
    return (TICK_COUNTER_HANDLE)0x1;
}

//
// Pal wrapper
//
void tickcounter_destroy(
    TICK_COUNTER_HANDLE tick_counter
)
{
    (void)tick_counter;
}

//
// Pal wrapper
//
int tickcounter_get_current_ms(
    TICK_COUNTER_HANDLE tick_counter,
    tickcounter_ms_t* current_ms
)
{
    (void)tick_counter;
    *current_ms = (tickcounter_ms_t)ticks_get();
    return 0;
}
