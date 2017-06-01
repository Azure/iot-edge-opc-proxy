// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os.h"
#include "util_mem.h"
#include "pal_time.h"

#if !defined(UNIT_TEST)
#include <time.h>
#endif

//
// Initialize global tick counter
//
int32_t pal_time_init(
    void
)
{
    return er_ok;
}

//
// Get current tick count in ms
//
ticks_t ticks_get(
    void
)
{
    int result;
    struct timespec ts;

    result = clock_gettime(CLOCK_MONOTONIC, &ts);
    dbg_assert(!result, "Failed to read tick");
    return (ts.tv_sec * 1000LL) + (ts.tv_nsec / 1000000);
}

//
// Deinit tick counter
//
void pal_time_deinit(
    void
)
{
    // No op
}
