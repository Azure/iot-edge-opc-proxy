// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _pal_errors_h_
#define _pal_errors_h_

#include "common.h"
#include "prx_err.h"

//
// Initialize err
//
decl_public_0(int32_t, pal_err_init,
    void
);

//
// Convert an error code
//
decl_public_1(int32_t, pal_os_to_prx_error,
    int, error
);

//
// Convert an error code
//
decl_public_1(int, pal_os_from_prx_error,
    int32_t, error
);

//
// Return general last error
//
decl_public_0(int32_t, pal_os_last_error_as_prx_error,
    void
);

//
// Set general last error
//
decl_public_1(void, pal_os_set_error_as_prx_error,
    int32_t, error
);

//
// Destroy err
//
decl_public_0(void, pal_err_deinit,
    void
);


#endif // _pal_errors_h_