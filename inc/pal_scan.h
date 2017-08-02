// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _pal_scan_h_
#define _pal_scan_h_

#include "common.h"
#include "prx_types.h"


//
// Represents a scan of the network or a machine
//
typedef struct pal_scan pal_scan_t;

//
// Called for each found address
//
typedef void(*pal_scan_cb_t)(
    void *context,
    uint64_t itf_index,       // Interface the address was found on
    int32_t error,                    // er_nomore if no more items
    prx_socket_address_t *addr,  // Ip address of found host / port
    const char* host_name       // Host name if possible to look up
    );

//
// Hint flags for scan functions
//
typedef enum pal_scan_flags
{
    pal_scan_cache_only = 0x1,    // Do not wait for changes in net
    pal_scan_no_name_lookup = 0x2   // Do not do name lookup for ip
}
pal_scan_flags_t;

//
// Called before the following functions are used
//
decl_public_0(int32_t, pal_scan_init,
    void
);

//
// Scan for addresses in all reachable subnets
//
decl_public_5(int32_t, pal_scan_net,
    uint16_t, port,
    int32_t, flags,
    pal_scan_cb_t, cb,
    void*, context,
    pal_scan_t**, scan
);

//
// Scan for ports on address
//
decl_public_7(int32_t, pal_scan_ports,
    const prx_socket_address_t*, addr,
    uint16_t, port_range_low,
    uint16_t, port_range_high,
    int32_t, flags,
    pal_scan_cb_t, cb,
    void*, context,
    pal_scan_t**, scan
);

//
// Release and abort in progress scan
//
decl_public_1(void, pal_scan_close,
    pal_scan_t*, scan
);

//
// Called when done using above functions
//
decl_public_0(void, pal_scan_deinit,
    void
);

#endif // _pal_scan_h_