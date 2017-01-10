// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _util_handle_h_
#define _util_handle_h_

#include "common.h"

#define handle_map_invalid_handle (int32_t)0

//
// Create a global handle map
//
decl_internal_0(int32_t, handle_map_init,
    void
);

//
// Returns a handle for a pointer
//
decl_internal_1(int32_t, handle_map_get_handle,
    const void*, pointer
);

//
// Returns a pointer for handle
//
decl_internal_1(const void*, handle_map_get_pointer,
    int32_t, handle
);

//
// Closes a handle
//
decl_internal_1(const void*, handle_map_remove_handle,
    int32_t, handle
);

//
// Frees global handle table
//
decl_internal_0(void, handle_map_deinit,
    void
);

#endif // _util_handle_h_