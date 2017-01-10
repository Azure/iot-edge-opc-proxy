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
// Get Full path of a file name
//
decl_public_1(const char*, pal_create_full_path,
    const char*, file_name
);

//
// Free created path
//
decl_public_1(void, pal_free_path,
    const char*, path
);

//
// Change working dir
//
decl_public_1(int32_t, pal_set_working_dir,
    const char*, dir
);

//
// Get working dir
//
decl_public_0(const char*, pal_get_working_dir,
    void
);

//
// Deinit pal 
//
decl_public_0(void, pal_file_deinit,
    void
);

#endif // _pal_file_h_