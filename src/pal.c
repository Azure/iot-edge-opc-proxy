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
#include "pal_sk.h"
#include "pal_time.h"
#include "pal_rand.h"
#include "pal_cred.h"

static pal_diag_callback_t diag_callback = NULL;
static uint32_t capabilities = pal_not_init;

//
// Internal registered callback
//
static void pal_diag_callback(
    log_entry_t* msg
)
{
#if defined(NO_ZLOG)
    (void)msg;
#else
    pal_diag_callback_t _cb = diag_callback;
    if (!_cb || !msg)
        return;
    _cb(msg->target, msg->msg);
#endif
}

//
// Hook for diagnostic callbacks
//
int32_t pal_set_diag_callback(
    pal_diag_callback_t callback
)
{
    diag_callback = callback;
    if (capabilities == pal_not_init)
        return er_ok;

    return log_register("pal", pal_diag_callback);
}

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
        // Register diagnostic callback
        if (diag_callback)
        {
            result = log_register("pal", pal_diag_callback);
            if (result != er_ok)
                break;
        }

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

        // Initialize socket 
        result = pal_socket_init();
        if (result == er_ok)
            capabilities |= (pal_cap_sockets | pal_cap_ev);
        else 
        {
            log_error(NULL, "Failed to init socket pal (%s).",
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
    diag_callback = NULL;
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

    if (capabilities & pal_cap_sockets)
        pal_socket_deinit();

    if (capabilities & pal_cap_file)
        pal_file_deinit();

    if (capabilities & pal_cap_cred)
        pal_cred_deinit();

    pal_rand_deinit();
    pal_time_deinit();
    pal_err_deinit();

    capabilities = pal_not_init;
    diag_callback = NULL;

    return er_ok;
}
