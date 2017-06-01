// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os.h"
#include "util_mem.h"
#include "pal_time.h"

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
    return GetTickCount64();
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
