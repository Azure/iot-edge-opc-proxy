// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "prx_module.h"

//
// Parse module configuration
//
void* prx_module_parse(
    const char* configuration
)
{
    (void)configuration;
    return (void*)0x1;
}

//
// Frees module configuration
//
void prx_module_free(
    void* configuration
)
{
    (void)configuration;
}

//
// Creates a proxy host instance to be managed by field gateway
//
static prx_host_t* prx_module_create(
    broker_handle_t* broker,
    const void* configuration
)
{
    int32_t result;
    prx_host_t* host = NULL;
    (void)configuration, broker;

    // Always init - todo: ref count..., pass configuration, save broker, etc...
    (void)prx_host_init();

    // Create host
    result = prx_host_create(NULL, proxy_type_server, &host);
    if (result != er_ok)
    {
        log_error(NULL, "Failed to create host (%s)!", prx_err_string(result));
        return NULL;
    }

    // Success
    log_info(NULL, "Proxy module created in GW Host!");
    return host;
}

//
// Destroys proxy host instance
//
static void prx_module_start(
    prx_host_t* host
)
{
    int32_t result;
    if (!host)
        return;
    result = prx_host_start(host);
    if (result != er_ok)
    {
        log_error(NULL, "Failed to start host (%s)!", prx_err_string(result));
    }
}

//
// Callback when field gateway received message 
//
static void prx_module_receive(
    prx_host_t* host,
    message_handle_t* message
)
{
    (void)host, message;
    // No-op at this point since we do not know how to crack the message
}

//
// Destroys host instance
//
static void prx_module_destroy(
    prx_host_t* host
)
{
    int32_t result;
    if (!host)
        return;
    result = prx_host_stop(host);
    if (result != er_ok)
    {
        log_error(NULL, "Failed to stop host (%s)!", prx_err_string(result));
    }
    prx_host_release(host);

    // Always deinit, todo: ref count before deinit
    prx_host_deinit();
}

//
// Export our host module callbacks
//
prx_module_aprx_t* Module_GetAPIS(
    int gw_version
)
{
    static prx_module_aprx_t api = {
        0,  // v1
        prx_module_parse,
        prx_module_free,
        prx_module_create,
        prx_module_destroy,
        prx_module_receive,
        prx_module_start
    };
    return gw_version == 0 ? &api : NULL;
}
