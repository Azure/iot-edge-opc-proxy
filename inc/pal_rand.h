// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _pal_rand_h_
#define _pal_rand_h_

#include "common.h"

//
// Initialize rand
//
decl_public_0(int32_t, pal_rand_init,
    void
);

//
// Fill buffer with random data
//
decl_public_2(int32_t, pal_rand_fill,
    void*, buf,
    size_t, len
);

//
// Destroy rand
//
decl_public_0(void, pal_rand_deinit,
    void
);

#endif // _pal_rand_h_