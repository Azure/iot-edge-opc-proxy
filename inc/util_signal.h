// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _util_signal_h_
#define _util_signal_h_

#include "common.h"

typedef struct signal signal_t;

//
// Create a signal object
//
decl_internal_3(int32_t, signal_create,
    bool, manual,
    bool, signalled,
    signal_t**, signal
);

//
// Wait for the signal
//
decl_internal_2(int32_t, signal_wait,
    signal_t*, signal,
    int32_t, timeout_ms
);

//
// Wait for the signal, returns remaining
//
decl_internal_2(int32_t, signal_wait_ex,
    signal_t*, signal,
    int32_t*, timeout_ms
);

//
// Set signal
//
decl_internal_1(int32_t, signal_set,
    signal_t*, signal
);

//
// Clear signal
//
decl_internal_1(int32_t, signal_clear,
    signal_t*, signal
);

//
// Frees signal object
//
decl_internal_1(void, signal_free,
    signal_t*, signal
);

#endif // _util_signal_h_