// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _prx_log_h_
#define _prx_log_h_

#include "common.h"
#include "io_stream.h"

//
// Initialize logging facilities
//
decl_public_0(int32_t, log_init,
    void
);

//
// Get a log stream to read logged messages from
//
decl_public_0(io_stream_t*, log_stream_get,
    void
);

//
// Deinitialize logging facilities
//
decl_public_0(void, log_deinit,
    void
);

#endif // _util_log_h_
