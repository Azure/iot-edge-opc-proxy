// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "pal.h"
#include "util_log.h"
#include "util_string.h"
#include "util_misc.h"

#include "pal_err.h"
#include "pal_file.h"
#include "pal_ws.h"
#include "pal_net.h"
#include "pal_sd.h"
#include "pal_sk.h"
#include "pal_time.h"
#include "pal_rand.h"
#include "pal_scan.h"
#include "pal_cred.h"

static uint32_t capabilities = pal_not_init;

//
// Initialize the pal layer
//
int32_t pal_init(
    void
)
{
    int32_t result = er_ok;

    if (capabilities != pal_not_init)
        return er_ok;

    result = pal_err_init();
    if (result != er_ok)
        return result;

    result = pal_time_init();
    if (result != er_ok)
    {
        pal_err_deinit();
        return result;
    }

    result = pal_rand_init();
    if (result != er_ok)
    {
        pal_time_deinit();
        pal_err_deinit();
        return result;
    }
    do
    {
        // Initialize secret store
        result = pal_cred_init();
        if (result == er_ok)
            capabilities |= pal_cap_cred;
        else if (result != er_not_supported)
        {
            log_error(NULL, "Failed to init cred pal (%s).",
                prx_err_string(result));
            break;
        }

        // Initialize file
        result = pal_file_init();
        if (result == er_ok)
            capabilities |= pal_cap_file;
        else if (result != er_not_supported)
        {
            log_error(NULL, "Failed to init file pal (%s).",
                prx_err_string(result));
            break;
        }

        // Initialize networking
        result = pal_net_init();
        if (result == er_ok)
            capabilities |= pal_cap_net;
        else
        {
            log_error(NULL, "Failed to init networking pal (%s).",
                prx_err_string(result));
            break;
        }

        // Initialize service discovery
        result = pal_sd_init();
        if (result == er_ok)
            capabilities |= pal_cap_dnssd;
        else if (result != er_not_supported)
        {
            log_error(NULL, "Failed to init name service pal (%s).",
                prx_err_string(result));
            break;
        }

        // Initialize socket
        result = pal_socket_init(&capabilities);
        if (result == er_ok)
            capabilities |= pal_cap_sockets;
        else
        {
            log_error(NULL, "Failed to init socket pal (%s).",
                prx_err_string(result));
            break;
        }

        // Initialize networking scanning
        result = pal_scan_init();
        if (result == er_ok)
            capabilities |= pal_cap_scan;
        else if (result != er_not_supported)
        {
            log_error(NULL, "Failed to init net scanning pal (%s).",
                prx_err_string(result));
            break;
        }

        // Initialize web socket functionality
        result = pal_wsclient_init();
        if (result == er_ok)
            capabilities |= pal_cap_wsclient;
        else if (result != er_not_supported)
        {
            log_error(NULL, "Failed to init wsclient pal (%s).",
                prx_err_string(result));
            break;
        }

        // Success...
        return er_ok;
    }
    while (0);

    if (capabilities == pal_not_init)
    {
        pal_rand_deinit();
        pal_time_deinit();
        pal_err_deinit();
    }
    else
    {
        pal_deinit();
    }
    return result;
}

//
// Post init, returns the capabilties of the pal.
//
uint32_t pal_caps(
    void
)
{
    return capabilities;
}

//
// Deinit pal
//
int32_t pal_deinit(
    void
)
{
    if (capabilities == pal_not_init)
        return er_bad_state;

    if (capabilities & pal_cap_wsclient)
        pal_wsclient_deinit();

    if (capabilities & pal_cap_scan)
        pal_scan_deinit();

    if (capabilities & pal_cap_sockets)
        pal_socket_deinit();

    if (capabilities & pal_cap_dnssd)
        pal_sd_deinit();

    if (capabilities & pal_cap_net)
        pal_net_deinit();

    if (capabilities & pal_cap_file)
        pal_file_deinit();

    if (capabilities & pal_cap_cred)
        pal_cred_deinit();

    pal_rand_deinit();
    pal_time_deinit();
    pal_err_deinit();

    capabilities = pal_not_init;

    return er_ok;
}
