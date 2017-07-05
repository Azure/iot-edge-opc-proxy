// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "pal_sd.h"


//
// Create client
//
int32_t pal_sdclient_create(
    pal_sdclient_error_cb_t cb,
    void* context,
    pal_sdclient_t** client
)
{
    (void)cb;
    (void)context;
    (void)client;

    dbg_assert(0, "Unexpected");

    return er_not_impl;
}

//
// Free client
//
void pal_sdclient_free(
    pal_sdclient_t* client
)
{
    (void)client;

    dbg_assert(0, "Unexpected");
}

//
// Create a browser instance for name service query
//
int32_t pal_sdbrowser_create(
    pal_sdclient_t* browser,
    pal_sd_result_cb_t cb,
    void* context,
    pal_sdbrowser_t** created
)
{
    (void)browser;
    (void)cb;
    (void)context;
    (void)created;

    dbg_assert(0, "Unexpected");

    return er_not_impl;
}


//
// Resolve service entries into addrinfo objects
//
int32_t pal_sdbrowser_resolve(
    pal_sdbrowser_t* browser,
    prx_socket_address_proxy_t* addr,
    int32_t itf_index
)
{
    (void)browser;
    (void)addr;
    (void)itf_index;

    dbg_assert(0, "Unexpected");

    return er_not_impl;
}

//
// Resolve services, or browse services and domains
//
int32_t pal_sdbrowser_browse(
    pal_sdbrowser_t* browser,
    const char* service_name, 
    const char* service_type, 
    const char* domain,       
    int32_t itf_index
)
{
    (void)browser;
    (void)service_name;
    (void)service_type;
    (void)domain;
    (void)itf_index;

    dbg_assert(0, "Unexpected");

    return er_not_impl;
}

//
// Release client
//
void pal_sdbrowser_free(
    pal_sdbrowser_t* browser
)
{
    (void)browser;

    dbg_assert(0, "Unexpected");
}

//
// Returns not supported
//
int32_t pal_sd_init(
    void
)
{
    return er_not_supported;
}

//
// Free name service layer
//
void pal_sd_deinit(
    void
)
{
    // No op
}
