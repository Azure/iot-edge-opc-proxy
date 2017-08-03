// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _pal_h_
#define _pal_h_

#include "common.h"
#include "prx_types.h"

//
// Pal capabilities
//
typedef enum pal_capabilities
{
    pal_not_init = 0x0,

    pal_cap_file = 0x1,          // Pal has file r/w support
    pal_cap_net = 0x2,     // Pal has network support (must)
    pal_cap_sockets = 0x4,  // Pal has socket support (must)

    pal_cap_wsclient = 0x8,      // Pal has websocket client
    pal_cap_cred = 0x10, // Pal has credential store support
    pal_cap_ev = 0x20,         // Pal supports async sockets

    pal_cap_dnssd = 0x40,   // Pal supports service browsing
    pal_cap_dirs = 0x80,        // Pal supports dir browsing
    pal_cap_scan = 0x100,   // Pal supports network scanning

    pal_cap_all = 0x1ff
}
pal_capabilities_t;

//
// Initialize pal
//
decl_public_0(int32_t, pal_init,
    void
);

//
// Post init, returns the capabilties of the pal.
//
decl_public_0(uint32_t, pal_caps,
    void
);

//
// Deinit pal
//
decl_public_0(int32_t, pal_deinit,
    void
);

#endif // _pal_h_