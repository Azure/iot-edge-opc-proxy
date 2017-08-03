// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "prx_log.h"
#include "pal.h"
#include "pal_net.h"
#include "pal_time.h"
#include "pal_scan.h"
#include "util_string.h"
#include "util_signal.h"
#include <stdio.h>


typedef struct scan_ctx
{
    signal_t* signal;
    pal_scan_t* scan;
    log_t log;
}
scan_ctx_t;

//
// Callback
//
void test_scan_cb(
    void *context,
    uint64_t itf_index,
    int32_t error,
    prx_socket_address_t *addr,
    const char* host_name
)
{
    scan_ctx_t* scanner = (scan_ctx_t*)context;

    (void)itf_index;

    if (error == er_ok)
    {
        if (addr->un.family == prx_address_family_inet6)
        {
            log_info(scanner->log, "Found: " __prx_sa_in6_fmt " (%s)",
                __prx_sa_in6_args(addr), host_name ? host_name : "");
        }
        else
        {
            dbg_assert(addr->un.family == prx_address_family_inet, "af wrong");
            log_info(scanner->log, "Found: " __prx_sa_in4_fmt " (%s)",
                __prx_sa_in4_args(addr), host_name ? host_name : "");
        }
    }
    else
    {
        if (error != er_nomore)
        {
            log_error(scanner->log, "Error scanning (%s)", prx_err_string(error));
        }
        signal_set(scanner->signal);
    }
}

//
// Subnet scan test utility
//
int main_scan(int argc, char *argv[])
{
    int32_t result;
    const char* port = NULL;
    scan_ctx_t ctx, *scanner = &ctx;
    ticks_t now;

    if (argc < 1)
        return er_arg;

    while (argc > 1)
    {
        argv++;
        argc--;

        /**/ if (!port)
            port = argv[0];
    }

    if (!port)
        port = "0";

    result = pal_init();
    if (result != er_ok)
        return result;
    do
    {
        memset(scanner, 0, sizeof(scan_ctx_t));
        scanner->log = log_get("test.scan");

        result = signal_create(true, false, &scanner->signal);
        if (result != er_ok)
            break;

        now = ticks_get();
        result = pal_scan_net((uint16_t)atoi(port), 0, test_scan_cb, scanner, &scanner->scan);
        if (result != er_ok)
            break;
        signal_wait(scanner->signal, -1);
        log_info(scanner->log, "Scan took %u ms", (uint32_t)(ticks_get() - now));
    }
    while (0);

    if (scanner->signal)
        signal_free(scanner->signal);
    if (scanner->scan)
        pal_scan_close(scanner->scan);
    pal_deinit();
    return result;
}

//
// Host scan utility
//
int main_pscan(int argc, char *argv[])
{
    int32_t result;
    const char* port_low = NULL, *port_high = NULL, *host = NULL;
    scan_ctx_t ctx, *scanner = &ctx;
    prx_addrinfo_t* info = NULL;
    size_t info_count;
    ticks_t now;

    if (argc < 1)
        return er_arg;

    while (argc > 1)
    {
        argv++;
        argc--;

        /**/ if (!host)
            host = argv[0];
        else if (!port_low)
            port_low = argv[0];
        else if (!port_high)
            port_high = argv[0];
    }

    if (!host)
        return er_arg;
    if (!port_low)
        port_low = "0";
    if (!port_high)
        port_high = "0";

    result = pal_init();
    if (result != er_ok)
        return result;
    do
    {
        memset(scanner, 0, sizeof(scan_ctx_t));
        scanner->log = log_get("test.pscan");

        result = signal_create(true, false, &scanner->signal);
        if (result != er_ok)
            break;

        result = pal_getaddrinfo(host, NULL, prx_address_family_unspec, 0, &info, &info_count);
        if (result != er_ok)
            break;
        if (info_count == 0)
        {
            result = er_not_found;
            break;
        }

        now = ticks_get();
        result = pal_scan_ports(&info[0].address, 0, (uint16_t)atoi(port_low),
            (uint16_t)atoi(port_high), test_scan_cb, scanner, &scanner->scan);
        if (result != er_ok)
            break;

        signal_wait(scanner->signal, -1);
        log_info(scanner->log, "Scan took %u ms", (uint32_t)(ticks_get() - now));
    }
    while (0);

    if (info)
        pal_freeaddrinfo(info);
    if (scanner->signal)
        signal_free(scanner->signal);
    if (scanner->scan)
        pal_scan_close(scanner->scan);
    pal_deinit();
    return result;
}
