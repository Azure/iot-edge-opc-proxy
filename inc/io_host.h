// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _io_host_h_
#define _io_host_h_

#include "common.h"
#include "prx_host.h"
#include "prx_sched.h"
#include "prx_ns.h"

//
// Returns the host address
//
decl_internal_1(io_ref_t*, prx_host_get_id,
    prx_host_t*, host
);

//
// returns a pointer to the name service
//
decl_internal_1(prx_ns_t*, prx_host_get_ns,
    prx_host_t*, host
);

//
// returns a pointer to the scheduler
//
decl_internal_1(prx_scheduler_t*, prx_host_get_scheduler,
    prx_host_t*, host
);

#endif // _io_host_h_
