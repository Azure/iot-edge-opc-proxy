// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os.h"
#include "util_mem.h"
#include "pal_scan.h"
#include "pal_net.h"
#include "pal_types.h"
#include "pal_err.h"
#include "util_string.h"
#include "util_misc.h"

//
// Scan for addresses with open port in subnet
//
int32_t pal_scan_net(
    uint16_t port,
    int32_t flags,
    pal_scan_cb_t cb,
    void* context,
    pal_scan_t** scan
)
{
    (void)flags;
    (void)port;
    (void)cb;
    (void)context;
    (void)scan;

    return er_not_supported;
}

//
// Scan for ports on address
//
int32_t pal_scan_ports(
    const prx_socket_address_t* addr,
    uint16_t port_range_low,
    uint16_t port_range_high,
    int32_t flags,
    pal_scan_cb_t cb,
    void* context,
    pal_scan_t** scan
)
{
    (void)addr;
    (void)port_range_low;
    (void)port_range_high;
    (void)flags;
    (void)cb;
    (void)context;
    (void)scan;

    return er_not_supported;
}

//
// Release and abort in progress scan
//
void pal_scan_close(
    pal_scan_t* scan
)
{
    (void)scan;
}

//
// Called before using networking layer
//
int32_t pal_scan_init(
    void
)
{
    return er_not_supported;
}

//
// Free networking layer
//
void pal_scan_deinit(
    void
)
{
    // No op
}
