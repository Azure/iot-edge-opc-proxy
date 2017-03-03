// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _pal_file_h_
#define _pal_file_h_

#include "common.h"
#include "prx_types.h"

//
// Initialize file
//
decl_public_0(int32_t, pal_file_init,
    void
);

//
// Returns whether file exists
//
decl_public_1(bool, pal_file_exists,
    const char*, file_name
);

//
// Get real/absolute path of a file name
//
decl_public_2(int32_t, pal_get_real_path,
    const char*, file_name,
    const char**, path
);

//
// Free created path
//
decl_public_1(void, pal_free_path,
    const char*, path
);

//
// Deinit pal 
//
decl_public_0(void, pal_file_deinit,
    void
);

#endif // _pal_file_h_