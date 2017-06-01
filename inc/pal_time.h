// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _pal_time_h
#define _pal_time_h

#include "os.h"  // for ticks_t
#include "common.h"

//
// Initialize global tick counter
//
decl_internal_0(int32_t, pal_time_init,
    void
);

//
// Get current tick count in ms
//
decl_internal_0(ticks_t, ticks_get,
    void
);

//
// Deinit tick counter
//
decl_internal_0(void, pal_time_deinit,
    void
);

#endif // _pal_time_h