// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _prx_host_h_
#define _prx_host_h_

#include "common.h"

typedef struct prx_host prx_host_t;

typedef enum prx_host_type
{
    proxy_type_custom = 0,  
    proxy_type_server,
    proxy_type_client
}
prx_host_type_t;

//
// Init host
//
decl_public_0(int32_t, prx_host_init,
    void
);

//
// Console entry point
//
decl_public_3(int32_t, prx_host_console,
    const char*, config_file,
    int32_t, argc,
    char**, argv
);

//
// Create host instance
//
decl_public_3(int32_t, prx_host_create,
    const char*, config_file,
    prx_host_type_t, type,
    prx_host_t**, host
);

//
// Start host instance
//
decl_public_1(int32_t, prx_host_start,
    prx_host_t*, host
);

//
// Block until break call
//
decl_public_1(int32_t, prx_host_sig_wait,
    prx_host_t*, host
);

//
// Break execution
//
decl_public_1(int32_t, prx_host_sig_break,
    prx_host_t*, host
);

//
// Stop host
//
decl_public_1(int32_t, prx_host_stop,
    prx_host_t*, host
);

// 
// Take a reference on the host
//
decl_public_2(int32_t, prx_host_clone,
    prx_host_t*, host,
    prx_host_t**, cloned
);

//
// Release the host reference
//
decl_public_1(void, prx_host_release,
    prx_host_t*, host
);

//
// Deinit host
//
decl_public_0(void, prx_host_deinit,
    void
);

#endif // _prx_host_h_
