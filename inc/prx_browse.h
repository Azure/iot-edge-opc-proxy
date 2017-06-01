// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _prx_browse_h_
#define _prx_browse_h_

#include "common.h"
#include "io_codec.h"
#include "prx_sched.h"
#include "pal_sk.h"

// 
// Browse server instance
//
typedef struct prx_browse_server prx_browse_server_t;

// 
// An instance of a client socket connected to browse server
//
typedef struct prx_browse_session prx_browse_session_t;

//
// Create browse server instance
//
decl_public_2(int32_t, prx_browse_server_create,
    prx_scheduler_t*, scheduler,
    prx_browse_server_t**, created
);

//
// Adds a session in the server and returns socket interface
//
decl_public_3(int32_t, prx_browse_server_accept,
    prx_browse_server_t*, server,
    io_codec_id_t, codec_id,
    pal_socket_client_itf_t*, itf
);

//
// Free browse server and release all associated resources
//
decl_public_1(void, prx_browse_server_free,
    prx_browse_server_t*, server
);

#endif // _prx_browse_h_