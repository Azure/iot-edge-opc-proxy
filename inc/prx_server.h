// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _prx_server_h_
#define _prx_server_h_

#include "common.h"
#include "prx_sched.h"
#include "io_transport.h"

//
// Handles link protocol, i.e. remote sockets
//
typedef struct prx_server prx_server_t;

//
// Create a socket server with a listener
//
decl_public_4(int32_t, prx_server_create,
    io_transport_t*, transport,
    prx_ns_entry_t*, entry,
    prx_scheduler_t*, scheduler,
    prx_server_t**, server
);

//
// Free socket server 
//
decl_public_1(void, prx_server_release,
    prx_server_t*, server
);

#endif // _prx_server_h_