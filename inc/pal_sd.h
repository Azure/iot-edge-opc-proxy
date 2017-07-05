// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _pal_sd_h_
#define _pal_sd_h_

#include "common.h"
#include "prx_types.h"

//
// Service discovery and nameservice query client
//
typedef struct pal_sdclient pal_sdclient_t;

//
// Service discovery query and browsing session
//
typedef struct pal_sdbrowser pal_sdbrowser_t;

//
// Types of results
//
typedef enum pal_sd_result_type
{
    pal_sd_result_domain,
    pal_sd_result_type,
    pal_sd_result_service,
    pal_sd_result_entry,
    pal_sd_result_addrinfo
}
pal_sd_result_type_t;

typedef enum pal_sd_result_flag
{
    pal_sd_result_removed = 0x1,
    pal_sd_result_all_for_now = 0x2
}
pal_sd_result_flag_t;

//
// Called for each result found / event received
//
typedef int32_t (*pal_sd_result_cb_t)(
    void *context,
    int32_t itf_index,
    int32_t error,
    pal_sd_result_type_t type,
    void *result,
    int32_t flags
    );

//
// Intermediate browse result
//
typedef struct pal_sd_browse_result
{
    const char* service_name;
    const char* service_type;  
    const char* domain; 
}
pal_sd_browse_result_t;

//
// Service entry result
//
typedef struct pal_sd_service_entry
{
    prx_socket_address_proxy_t* addr;     // The host:port address
    size_t records_len;                   // Number of txt records
    prx_property_t* records;                       // in blob form 
}
pal_sd_service_entry_t;

//
// Called for client events
//
typedef void (*pal_sdclient_error_cb_t)(
    void *context,
    int32_t error
    );

//
// Called before any of the following functions are used
//
decl_public_0(int32_t, pal_sd_init,
    void
);

//
// Create a client to create browsers (browse sessions)
//
decl_public_3(int32_t, pal_sdclient_create,
    pal_sdclient_error_cb_t, cb,
    void*, context,
    pal_sdclient_t**, client
);

//
// Create a browser instance to browse objects
//
decl_public_4(int32_t, pal_sdbrowser_create,
    pal_sdclient_t*, client,
    pal_sd_result_cb_t, cb,
    void*, context,
    pal_sdbrowser_t**, browser
);

//
// Browse domains, service types, or services
//
decl_public_5(int32_t, pal_sdbrowser_browse,
    pal_sdbrowser_t*, browser,
    const char*, service_name,     // Pass null to browse services
    const char*, service_type,  // Pass null to browse all domains
    const char*, domain,           // Pass null for default domain
    int32_t, itf_index
);

//
// Resolve service entries into addrinfo objects
//
decl_public_3(int32_t, pal_sdbrowser_resolve,
    pal_sdbrowser_t*, browser,
    prx_socket_address_proxy_t*, addr,       // from service entry
    int32_t, itf_index
);

//
// Cancel browsing and release browser reference to client
//
decl_public_1(void, pal_sdbrowser_free,
    pal_sdbrowser_t*, browser
);

//
// Release handle to client
//
decl_public_1(void, pal_sdclient_free,
    pal_sdclient_t*, client
);

//
// Called when done using above functions
//
decl_public_0(void, pal_sd_deinit,
    void
);

#endif // _pal_sd_h_