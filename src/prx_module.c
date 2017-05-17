// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "prx_module.h"
#include "prx_server.h"
#include "io_host.h"

//
// Proxy module
//
struct prx_module
{
    prx_host_t* host;
    prx_server_t* server;
    broker_handle_t* broker;
    log_t log;
};

//
// Server transport (methods)
//
extern io_transport_t* io_iot_hub_mqtt_server_transport(
    void
);

//
// Parse module configuration
//
void* prx_module_parse(
    const char* configuration
)
{
    int32_t result;
    prx_ns_entry_t* entry;

    // Parse proxy entry
    result = prx_ns_entry_create_from_string(configuration, &entry);
    if (result != er_ok)
        return NULL;

    return entry;
}

//
// Frees module configuration
//
void prx_module_free(
    void* configuration
)
{
    prx_ns_entry_t* entry = (prx_ns_entry_t*)configuration;
    if (!entry)
        return;

    prx_ns_entry_release(entry);
}

//
// Destroys module instance
//
static void prx_module_destroy(
    prx_module_t* module
)
{
    if (!module)
        return;

    if (module->server)
        prx_server_release(module->server);
    if (module->host)
        prx_host_release(module->host);

    mem_free_type(prx_module_t, module);
}

//
// Creates a proxy module instance to be managed by edge
//
static prx_module_t* prx_module_create(
    broker_handle_t* broker,
    const void* configuration
)
{
    int32_t result;
    prx_module_t* module;
    prx_ns_entry_t* entry = (prx_ns_entry_t*)configuration;
    (void)broker;

    module = mem_zalloc_type(prx_module_t);
    if (!module)
        return NULL;
    do
    {
        module->log = log_get("module");
        module->broker = broker;

        // Aquire a host
        result = prx_host_get(&module->host);
        if (result != er_ok)
        {
            log_error(module->log, "Failed to get host reference (%s)!",
                prx_err_string(result));
            break;
        }

        // Create and start the server
        result = prx_server_create(io_iot_hub_mqtt_server_transport(),
            entry, prx_host_get_scheduler(module->host), &module->server);
        if (result != er_ok)
        {
            log_error(module->log, "Failed to create server (%s)!", 
                prx_err_string(result));
            break;
        }
        // Success
        log_trace(NULL, "Proxy module created in GW Host!");
        return module;
    }
    while (0);

    prx_module_destroy(module);
    return NULL;
}

//
// Start module
//
static void prx_module_start(
    prx_module_t* module
)
{
    (void)module;
    // No-op, we already started the server
}

//
// Callback when edge received message 
//
static void prx_module_receive(
    prx_module_t* module,
    message_handle_t* message
)
{
    (void)module;
    (void)message;
    // No-op at this point since we do not know how to crack the message
}

//
// Export our module module callbacks
//
prx_module_api_t* Module_GetAPIS(
    int gw_version
)
{
    static prx_module_api_t api = {
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
