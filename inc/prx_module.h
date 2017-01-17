// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _prx_module_h_
#define _prx_module_h_

#include "common.h"

//
// Proxy server module
//
typedef struct prx_module prx_module_t;

//
// Opaque Broker handle...
//
typedef void broker_handle_t;

//
// Opaque Message handle...
//
typedef void message_handle_t;

//
// Parse module configuration
//
typedef void* (*prx_module_parse_t)(
    const char* configuration
    );

//
// Frees module configuration
//
typedef void (*prx_module_free_t)(
    void* configuration
    );

//
// Creates a host instance to be managed by field gateway
//
typedef prx_module_t* (*prx_module_create_t)(
    broker_handle_t* broker, 
    const void* configuration
    );

//
// Start module
//
typedef void (*prx_module_start_t)(
    prx_module_t* module
    );

//
// Destroys host instance
//
typedef void (*prx_module_destroy_t)(
    prx_module_t* module
    );

//
// Callback when field gateway received message - stub only at this point
//
typedef void (*prx_module_receive_t)(
    prx_module_t* module,
    message_handle_t* message 
    );

//
// Represents v1 module api, exported to field gateway - !!!fixed order!!!
//
typedef struct prx_module_api
{
    int version;
    prx_module_parse_t on_parse;
    prx_module_free_t on_free;
    prx_module_create_t on_create;
    prx_module_destroy_t on_destroy;
    prx_module_receive_t on_receive;
    prx_module_start_t on_start;
}
prx_module_api_t;

//
// Mandatory export for field gateway to load host
//
decl_public_1(prx_module_api_t*, Module_GetAPIS,
    int, gateway_aprx_version
);

#endif // _prx_module_h_
