// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os.h"
#include "util_mem.h"
#include "pal_sd.h"
#include "pal.h"
#include "pal_types.h"
#include "pal_err.h"
#include "pal_mt.h"
#include "util_misc.h"
#include "util_string.h"

#include "azure_c_shared_utility/doublylinkedlist.h"

#if !defined(UNIT_TEST)
#include <avahi-common/defs.h>
#include <avahi-common/error.h>
#include <avahi-common/domain.h>
#include <avahi-common/thread-watch.h>
#if !defined(_AVAHI_EMBEDDED)
#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#else
#include <avahi-core/core.h>
#include <avahi-core/lookup.h>
#define AvahiClient \
    AvahiServer
#define AvahiClientState \
    AvahiServerState
#define avahi_client_free \
    avahi_server_free
#define avahi_client_get_domain_name \
    avahi_server_get_domain_name
#define avahi_client_errno \
    avahi_server_errno
#define AvahiServiceResolver \
    AvahiSServiceResolver
#define avahi_service_resolver_new \
    avahi_s_service_resolver_new
#define avahi_service_resolver_free \
    avahi_s_service_resolver_free
#define AvahiHostNameResolver \
    AvahiSHostNameResolver
#define avahi_host_name_resolver_new \
    avahi_s_host_name_resolver_new
#define avahi_host_name_resolver_free \
    avahi_s_host_name_resolver_free
#define AvahiServiceBrowser \
    AvahiSServiceBrowser
#define avahi_service_browser_new \
    avahi_s_service_browser_new
#define avahi_service_browser_free \
    avahi_s_service_browser_free
#define AvahiServiceTypeBrowser \
    AvahiSServiceTypeBrowser
#define avahi_service_type_browser_new \
    avahi_s_service_type_browser_new
#define avahi_service_type_browser_free \
    avahi_s_service_type_browser_free
#define AvahiDomainBrowser \
    AvahiSDomainBrowser
#define avahi_domain_browser_new \
    avahi_s_domain_browser_new
#define avahi_domain_browser_free \
    avahi_s_domain_browser_free
#endif // !_AVAHI_EMBEDDED
#else
#if defined (_AVAHI_EMBEDDED)
#undef _AVAHI_EMBEDDED
#endif
#endif

//
// Avahi client/server wrapper
//
struct pal_sdclient
{
    AvahiClient* client;     // Avahi browser client - connected to daemon
    AvahiThreadedPoll* polling;                       // Main polling loop
    pal_sdclient_error_cb_t cb;
    void* context;
    log_t log;
};

//
// Browser instance object - wraps a browsing session on a client
//
struct pal_sdbrowser
{
    pal_sdclient_t* client;                               // Client handle
    DLIST_ENTRY handles;                 // List of active browser handles
                                          // Always access under poll lock
    size_t items_found;
    pal_sd_result_cb_t cb;                          // Result callback ...
    void* context;                                // ... with user context
    log_t log;
};

//
// Avahi handle wrapper
//
typedef struct pal_sdhandle
{
    AvahiServiceResolver* resolver;                   // Service resolvers
    AvahiHostNameResolver* address;       // Host name to address resolver
    AvahiServiceTypeBrowser* service_types;          // or type enumerator
    AvahiDomainBrowser* domains;                      // Or domain browser
    AvahiServiceBrowser* services;            // and / or services browser

    pal_sdbrowser_t* browser;                       // Owner of the handle
    char* service_name;                          // For resolve operations
    uint16_t port;
    DLIST_ENTRY link;           // Link to queue browser clients in server
}
pal_sdhandle_t;

//
// Translate error numbers from avahi sd to prx
//
static int32_t pal_sd_avahi_error_to_prx_error(
    int32_t error
)
{
    switch (error) 
    {
    case AVAHI_OK:
        return er_ok;
    case AVAHI_ERR_FAILURE:
        // Generic error code
        return er_fatal;
    case AVAHI_ERR_BAD_STATE:
        // Object was in a bad state
        return er_bad_state;
    case AVAHI_ERR_INVALID_HOST_NAME:
        // Invalid host name
        return er_invalid_format;
    case AVAHI_ERR_INVALID_DOMAIN_NAME:
        // Invalid domain name
        return er_invalid_format;
    case AVAHI_ERR_NO_NETWORK:
        // No suitable network protocol available
        return er_network;
    case AVAHI_ERR_INVALID_TTL:
        // Invalid DNS TTL
        return er_arg;
    case AVAHI_ERR_IS_PATTERN:
        // RR key is pattern
        return er_invalid_format;
    case AVAHI_ERR_COLLISION:
        // Name collision
        return er_already_exists;
    case AVAHI_ERR_INVALID_RECORD:
        // Invalid RR
        return er_invalid_format;
    case AVAHI_ERR_INVALID_SERVICE_NAME:
        // Invalid service name
        return er_invalid_format;
    case AVAHI_ERR_INVALID_SERVICE_TYPE:
        // Invalid service type
        return er_invalid_format;
    case AVAHI_ERR_INVALID_PORT:
        // Invalid port number
        return er_invalid_format;
    case AVAHI_ERR_INVALID_KEY:
        // Invalid key
        return er_crypto;
    case AVAHI_ERR_INVALID_ADDRESS:
        // Invalid address
        return er_no_address;
    case AVAHI_ERR_TIMEOUT:
        // Timeout reached
        return er_timeout;
    case AVAHI_ERR_TOO_MANY_CLIENTS:
        // Too many clients
        return er_refused;
    case AVAHI_ERR_TOO_MANY_OBJECTS:
        // Too many objects
        return er_refused;
    case AVAHI_ERR_TOO_MANY_ENTRIES:
        // Too many entries
        return er_refused;
    case AVAHI_ERR_OS:
        // OS error
        return er_fatal;
    case AVAHI_ERR_ACCESS_DENIED:
        // Access denied
        return er_permission;
    case AVAHI_ERR_INVALID_OPERATION:
        // Invalid operation
        return er_not_supported;
    case AVAHI_ERR_DBUS_ERROR:
        // An unexpected D-Bus error occurred
        return er_comm;
    case AVAHI_ERR_DISCONNECTED:
        // Daemon connection failed
        return er_connecting;
    case AVAHI_ERR_NO_MEMORY:
        // Memory exhausted
        return er_out_of_memory;
    case AVAHI_ERR_INVALID_OBJECT:
        // The object passed to this function was invalid
        return er_arg;
    case AVAHI_ERR_NO_DAEMON:
        // Daemon not running
        return er_bad_state;
    case AVAHI_ERR_INVALID_INTERFACE:
        // Invalid interface
        return er_no_address;
    case AVAHI_ERR_INVALID_PROTOCOL:
        // Invalid protocol
        return er_not_supported;
    case AVAHI_ERR_INVALID_FLAGS:
        // Invalid flags
        return er_bad_flags;
    case AVAHI_ERR_NOT_FOUND:
        // Not found
        return er_not_found;
    case AVAHI_ERR_INVALID_CONFIG:
        // Configuration error
        return er_prop_get;
    case AVAHI_ERR_VERSION_MISMATCH:
        // Verson mismatch
        return er_invalid_format;
    case AVAHI_ERR_INVALID_SERVICE_SUBTYPE:
        // Invalid service subtype
        return er_invalid_format;
    case AVAHI_ERR_INVALID_PACKET:
        // Invalid packet
        return er_invalid_format;
    case AVAHI_ERR_INVALID_DNS_ERROR:
        // Invalid DNS return code
        return er_invalid_format;
    case AVAHI_ERR_DNS_FORMERR:
        // DNS Error: Form error
        return er_invalid_format;
    case AVAHI_ERR_DNS_SERVFAIL:
        // DNS Error: Server Failure
        return er_comm;
    case AVAHI_ERR_DNS_NXDOMAIN:
        // DNS Error: No such domain
        return er_not_found;
    case AVAHI_ERR_DNS_NOTIMP:
        // DNS Error: Not implemented
        return er_not_impl;
    case AVAHI_ERR_DNS_REFUSED:
        // DNS Error: Operation refused
        return er_refused;
    case AVAHI_ERR_DNS_YXDOMAIN:
    case AVAHI_ERR_DNS_YXRRSET:
    case AVAHI_ERR_DNS_NXRRSET:
        return er_bad_state;
    case AVAHI_ERR_DNS_NOTAUTH:
        // DNS Error: Not authorized
        return er_permission;
    case AVAHI_ERR_DNS_NOTZONE:
        return er_invalid_format;
    case AVAHI_ERR_INVALID_RDATA:
        // Invalid RDATA
        return er_invalid_format;
    case AVAHI_ERR_INVALID_DNS_CLASS:
        // Invalid DNS class
        return er_invalid_format;
    case AVAHI_ERR_INVALID_DNS_TYPE:
        // Invalid DNS type
        return er_invalid_format;
    case AVAHI_ERR_NOT_SUPPORTED:
        // Not supported
        return er_not_supported;
    case AVAHI_ERR_NOT_PERMITTED:
        // Operation not permitted
        return er_permission;
    case AVAHI_ERR_INVALID_ARGUMENT:
        // Invalid argument
        return er_arg;
    case AVAHI_ERR_IS_EMPTY:
        // Is empty
        return er_nomore;
    case AVAHI_ERR_NO_CHANGE:
        // The requested operation is invalid because it is redundant
        return er_bad_state;
    default:
        return er_unknown;
    }
}

//
// Get last browser error
//
static int32_t pal_sdclient_last_avahi_error(
    pal_sdclient_t* client
)
{
    int error;
    chk_arg_fault_return(client);
    chk_arg_fault_return(client->client);
    error = avahi_client_errno(client->client);
    log_error(client->log, "Error %s", avahi_strerror(error));
    return pal_sd_avahi_error_to_prx_error(error);
}

//
// Client callback
//
static void pal_sdclient_callback(
    AvahiClient *avahi_client,
    AvahiClientState state, 
    void* context
) 
{
    int error;
    pal_sdclient_t* client = (pal_sdclient_t*)context;

    switch (state) 
    {
#if !defined(_AVAHI_EMBEDDED)
    case AVAHI_CLIENT_CONNECTING:
        log_trace(client->log, "Avahi connecting...");
        break;
    case AVAHI_CLIENT_FAILURE:
#endif
    case AVAHI_SERVER_FAILURE:
        error = avahi_client_errno(avahi_client);
        log_error(client->log, "Avahi client failure %s!", avahi_strerror(error));
        if (client->cb)
        {
            client->cb(client->context, pal_sd_avahi_error_to_prx_error(error));
            client->cb = NULL;
        }
        break;
    case AVAHI_SERVER_COLLISION:
    case AVAHI_SERVER_REGISTERING:
        break;
    case AVAHI_SERVER_RUNNING:
        log_trace(client->log, "Avahi connected, server running!");
        break;
    default:
        dbg_assert(0, "Unexpected avahi client/server state %d", state);
        break;
    }
}

//
// Create and add wrapped handle to browser
//
static void pal_sdbrowser_remove_handle(
    pal_sdhandle_t* handle
)
{
    dbg_assert_ptr(handle);
    dbg_assert_ptr(handle->browser);

    // Assumption is that this is under polling lock
    if (handle->resolver)
        avahi_service_resolver_free(handle->resolver);
    if (handle->address)
        avahi_host_name_resolver_free(handle->address);
    if (handle->service_types)
        avahi_service_type_browser_free(handle->service_types);
    if (handle->domains)
        avahi_domain_browser_free(handle->domains);
    if (handle->services)
        avahi_service_browser_free(handle->services);

    if (handle->service_name)
        mem_free(handle->service_name);

    DList_RemoveEntryList(&handle->link);
    mem_free_type(pal_sdhandle_t, handle);
}

//
// Create and add wrapped handle to browser
//
static int32_t pal_sdbrowser_add_handle(
    pal_sdbrowser_t* browser,
    pal_sdhandle_t** created
)
{
    pal_sdhandle_t* handle;

    // Assumption is that this is under polling lock

    dbg_assert_ptr(browser);
    dbg_assert_ptr(created);

    handle = (pal_sdhandle_t*)mem_zalloc_type(pal_sdhandle_t);
    if (!handle)
        return er_out_of_memory;

    DList_InitializeListHead(&handle->link);
    DList_InsertTailList(&browser->handles, &handle->link);
    handle->browser = browser;

    *created = handle;
    return er_ok;
}

//
// Convert event to flags or error
//
static int32_t pal_sdbrowser_translate_avahi_event(
    pal_sdbrowser_t* browser,
    AvahiBrowserEvent ev,
    bool* null_result,
    int32_t* pal_flags
)
{
    dbg_assert_ptr(browser);
    dbg_assert_ptr(pal_flags);
    switch (ev)
    {
    case AVAHI_BROWSER_NEW:
        *null_result = false;
        break;
    case AVAHI_BROWSER_REMOVE:
        *null_result = false;
        (*pal_flags) |= (pal_sd_result_removed);
        break;
    case AVAHI_BROWSER_CACHE_EXHAUSTED:
    case AVAHI_BROWSER_ALL_FOR_NOW:
        *null_result = true;
        if (browser->items_found > 0)
            (*pal_flags) |= (pal_sd_result_all_for_now);
        break;
    case AVAHI_BROWSER_FAILURE:
        *null_result = true;
        return pal_sdclient_last_avahi_error(browser->client);
    default:
        dbg_assert(0, "Unexpected event %d", ev);
        return er_fault;
    }
    return er_ok;
}

//
// Callback
//
static void pal_sdbrowser_service_resolve_callback (
    AvahiServiceResolver *avahi_handle,
    AvahiIfIndex itf_index,
    AvahiProtocol protocol, 
    AvahiResolverEvent ev,
    const char *name, 
    const char *type,
    const char *domain, 
    const char *host,
    const AvahiAddress *address, 
    uint16_t port,
    AvahiStringList *txt, 
    AvahiLookupResultFlags flags, 
    void *context
)
{
    int32_t result;
    pal_sdhandle_t* handle = (pal_sdhandle_t*)context;
    pal_sdbrowser_t* browser;
    pal_sd_service_entry_t resolve_result;
    prx_socket_address_proxy_t addr;
    AvahiStringList* iter;
    int32_t pal_flags = 0;

    (void)avahi_handle;
    (void)address;
    (void)flags;
    (void)domain;
    (void)type;
    (void)name;
    (void)protocol;

    dbg_assert_ptr(handle);
    browser = handle->browser;
    dbg_assert_ptr(browser);

    if (ev == AVAHI_RESOLVER_FAILURE)
    {
        result = browser->cb(browser->context, itf_index, 
            pal_sdclient_last_avahi_error(browser->client),
            pal_sd_result_entry, NULL, 0);
    }
    else
    {
        result = er_ok;

        addr.family = prx_address_family_proxy;
        addr.flags = 0;
        addr.itf_index = itf_index;
        addr.port = port;
        addr.host_dyn = host ? host : "";

        resolve_result.addr = &addr;
        resolve_result.records_len = 0;
        resolve_result.records = NULL;

        // Validate and count number of txt records
        for (iter = txt; iter; iter = iter->next)
            resolve_result.records_len++;
        if (resolve_result.records_len > 0)
        {
            // Allocate variable length prop record set (todo: cache in browser)
            resolve_result.records = (prx_property_t*)mem_zalloc(
                resolve_result.records_len * sizeof(prx_property_t));
            if (!resolve_result.records)
                result = er_out_of_memory;
            else
            {
                iter = txt;
                for (int i = 0; iter; iter = iter->next, i++)
                {
                    resolve_result.records[i].type = prx_record_type_txt;
                    resolve_result.records[i].property.bin.size = iter->size;
                    resolve_result.records[i].property.bin.value = iter->text;
                }
            }
        }
        result = browser->cb(browser->context, itf_index, result,
            pal_sd_result_entry, &resolve_result, pal_flags);

        if (resolve_result.records)
            mem_free(resolve_result.records);
    }

    if (result == er_ok)
        return;

    pal_sdbrowser_remove_handle(handle);
    if (result != er_aborted)
    {
        log_error(browser->log, "Resolve callback returned %s...",
            prx_err_string(result));
    }
}

//
// Callback
//
static void pal_sdbrowser_enumerate_services_and_resolve_callback(
    AvahiServiceBrowser *avahi_handle,
    AvahiIfIndex itf_index,
    AvahiProtocol protocol,
    AvahiBrowserEvent ev,
    const char *service_name,
    const char *service_type,
    const char *domain,
    AvahiLookupResultFlags flags,
    void *context
)
{
    int32_t result;
    pal_sdhandle_t* handle = (pal_sdhandle_t*)context;
    pal_sdbrowser_t* browser;
    int32_t pal_flags = 0;
    bool null_result = false;

    (void)avahi_handle;
    (void)flags;
    (void)protocol;

    dbg_assert_ptr(handle);
    browser = handle->browser;
    dbg_assert_ptr(browser);

    result = pal_sdbrowser_translate_avahi_event(browser, ev, &null_result, &pal_flags);
    if (null_result)
    {
        if (result == er_ok && !pal_flags)
            return;
        result = browser->cb(browser->context, itf_index, result,
            pal_sd_result_entry, NULL, 0);
    }
    else
    {
        // Todo: Handle remove
        // Todo: Handle errors better

        dbg_assert_ptr(service_name);
        if (0 != string_compare(service_name, handle->service_name))
            return;

        // Now do the real resolve with the good interface index and protocol info
        result = pal_sdbrowser_add_handle(browser, &handle);
        if (result == er_ok)
        {
            handle->resolver = avahi_service_resolver_new(browser->client->client,
                itf_index, protocol, service_name, service_type, domain,
                AVAHI_PROTO_UNSPEC, 0, pal_sdbrowser_service_resolve_callback, handle);
            if (handle->resolver)
                return; // done

            pal_sdbrowser_remove_handle(handle);
            result = pal_sdclient_last_avahi_error(browser->client);
        }

        // Pass on error info
        result = browser->cb(browser->context, itf_index, result,
            pal_sd_result_entry, NULL, 0);
    }
    if (result == er_ok)
        return;

    pal_sdbrowser_remove_handle(handle);
    if (result != er_aborted)
    {
        log_error(browser->log, "Browse callback returned %s...",
            prx_err_string(result));
    }
}

//
// Resolve services
//
static int32_t pal_sdbrowser_resolve_service(
    pal_sdbrowser_t* browser,
    const char* service_name,
    const char* service_type,
    const char* domain,
    int32_t itf_index
)
{
    int32_t result;
    pal_sdhandle_t* handle = NULL;

    dbg_assert_ptr(browser);
    dbg_assert_ptr(browser->client);
    dbg_assert_ptr(service_name);
    dbg_assert_ptr(service_type);

    //
    // Resolve requires the exact interface and protocol passed from 
    // service enumeration. This means to allow resolve to be called
    // on a name that was published on all interfaces we must 
    // re-enumerate the service to find all services with matching name, 
    // and then resolve them.
    //
    avahi_threaded_poll_lock(browser->client->polling);
    do
    {
        result = pal_sdbrowser_add_handle(browser, &handle);
        if (result != er_ok)
            break;

        result = string_clone(service_name, &handle->service_name);
        if (result != er_ok)
            break;

        handle->services = avahi_service_browser_new(browser->client->client,
            itf_index == prx_itf_index_all ? AVAHI_IF_UNSPEC : (uint32_t)itf_index,
            AVAHI_PROTO_UNSPEC, service_type, domain,
            0, pal_sdbrowser_enumerate_services_and_resolve_callback, handle);
        if (!handle->services)
        {
            result = pal_sdclient_last_avahi_error(browser->client);
            break;
        }
        // Done
        result = er_ok;
        handle = NULL;
        break;
    } 
    while (0);
    avahi_threaded_poll_unlock(browser->client->polling);
    if (handle)
        pal_sdbrowser_remove_handle(handle);
    return result;
}

//
// Callback
//
static void pal_sdbrowser_enumerate_services_callback (
    AvahiServiceBrowser *avahi_handle,
    AvahiIfIndex itf_index,
    AvahiProtocol protocol, 
    AvahiBrowserEvent ev,
    const char *service_name,
    const char *service_type,
    const char *domain, 
    AvahiLookupResultFlags flags, 
    void *context
)
{
    int32_t result;
    pal_sdhandle_t* handle = (pal_sdhandle_t*)context;
    pal_sdbrowser_t* browser;
    pal_sd_browse_result_t browse_result;
    int32_t pal_flags = 0;
    bool null_result = false;

    (void)avahi_handle;
    (void)flags;
    (void)protocol;

    dbg_assert_ptr(handle);
    browser = handle->browser;
    dbg_assert_ptr(browser);

    result = pal_sdbrowser_translate_avahi_event(browser, ev, &null_result, &pal_flags);
    if (null_result)
    {
        if (result == er_ok && !pal_flags)
            return;
        result = browser->cb(browser->context, itf_index, result,
            pal_sd_result_service, NULL, pal_flags);
    }
    else
    {
        browse_result.domain = domain;
        browse_result.service_name = service_name;
        browse_result.service_type = service_type;

        result = browser->cb(browser->context, itf_index, result,
            pal_sd_result_service, &browse_result, pal_flags);
    }

    if (result == er_ok)
        return;
    
    pal_sdbrowser_remove_handle(handle);
    if (result != er_aborted)
    {
        log_error(browser->log, "Browse callback returned %s...",
            prx_err_string(result));
    }
}

//
// Browse services
//
static int32_t pal_sdbrowser_enumerate_services(
    pal_sdbrowser_t* browser,
    const char* service_type,
    const char* domain,      
    int32_t itf_index
)
{
    int32_t result;
    pal_sdhandle_t* handle = NULL;

    dbg_assert_ptr(browser);
    dbg_assert_ptr(service_type);
    dbg_assert_ptr(browser->client);

    avahi_threaded_poll_lock(browser->client->polling);
    do
    {
        result = pal_sdbrowser_add_handle(browser, &handle);
        if (result != er_ok)
            break;

        handle->services = avahi_service_browser_new(browser->client->client,
            itf_index == prx_itf_index_all ? AVAHI_IF_UNSPEC : (uint32_t)itf_index,
            AVAHI_PROTO_UNSPEC, service_type, domain,
            0, pal_sdbrowser_enumerate_services_callback, handle);
        if (!handle->services)
        {
            result = pal_sdclient_last_avahi_error(browser->client);
            break;
        }
        // Done
        handle = NULL;
        result = er_ok;
        break;
    }
    while (0);
    avahi_threaded_poll_unlock(browser->client->polling);

    if (handle)
        pal_sdbrowser_remove_handle(handle);
    return result;
}

//
// Callback
//
static void pal_sdbrowser_enumerate_service_types_callback(
    AvahiServiceTypeBrowser *avahi_handle,
    AvahiIfIndex itf_index,
    AvahiProtocol protocol,
    AvahiBrowserEvent ev,
    const char *service_type,
    const char *domain,
    AvahiLookupResultFlags flags,
    void *context
)
{
    int32_t result;
    pal_sdhandle_t* handle = (pal_sdhandle_t*)context;
    pal_sdbrowser_t* browser;
    pal_sd_browse_result_t browse_result;
    int32_t pal_flags = 0;
    bool null_result = false;

    (void)avahi_handle;
    (void)flags;
    (void)protocol;

    dbg_assert_ptr(handle);
    browser = handle->browser;
    dbg_assert_ptr(browser);

    result = pal_sdbrowser_translate_avahi_event(browser, ev, &null_result, &pal_flags);
    if (null_result)
    {
        if (result == er_ok && !pal_flags)
            return;
        result = browser->cb(browser->context, itf_index, result,
            pal_sd_result_type, NULL, pal_flags);
    }
    else
    {
        browse_result.domain = domain;
        browse_result.service_type = service_type;
        browse_result.service_name = NULL;

        result = browser->cb(browser->context, itf_index, result,
            pal_sd_result_type, &browse_result, pal_flags);
    }

    if (result == er_ok)
        return;
    pal_sdbrowser_remove_handle(handle);
    if (result != er_aborted)
    {
        log_error(browser->log, "Browse callback returned %s...",
            prx_err_string(result));
    }
}

//
// Browse service types
//
static int32_t pal_sdbrowser_enumerate_service_types(
    pal_sdbrowser_t* browser,
    const char* domain,
    int32_t itf_index
)
{
    int32_t result;
    pal_sdhandle_t* handle = NULL;
    dbg_assert_ptr(browser);
    dbg_assert_ptr(browser->client);

    avahi_threaded_poll_lock(browser->client->polling);
    do
    {
        result = pal_sdbrowser_add_handle(browser, &handle);
        if (result != er_ok)
            break;

        handle->service_types = avahi_service_type_browser_new(browser->client->client,
            itf_index == prx_itf_index_all ? AVAHI_IF_UNSPEC : (uint32_t)itf_index,
            AVAHI_PROTO_UNSPEC, domain, 0,
            pal_sdbrowser_enumerate_service_types_callback, handle);
        if (!handle->service_types)
        {
            result = pal_sdclient_last_avahi_error(browser->client);
            break;
        }
        // Done
        result = er_ok;
        handle = NULL;
        break;
    } 
    while (0);
    avahi_threaded_poll_unlock(browser->client->polling);
    if (handle)
        pal_sdbrowser_remove_handle(handle);
    return result;
}

//
// Callback
//
static void pal_sdbrowser_enumerate_domains_callback(
    AvahiDomainBrowser *avahi_handle,
    AvahiIfIndex itf_index,
    AvahiProtocol protocol, 
    AvahiBrowserEvent ev,
    const char *domain, 
    AvahiLookupResultFlags flags, 
    void *context
)
{
    int32_t result;
    pal_sdhandle_t* handle = (pal_sdhandle_t*)context;
    pal_sdbrowser_t* browser;
    pal_sd_browse_result_t browse_result;
    int32_t pal_flags = 0;
    bool null_result = false;

    (void)avahi_handle;
    (void)flags;
    (void)protocol;

    dbg_assert_ptr(handle);
    browser = handle->browser;
    dbg_assert_ptr(browser);

    result = pal_sdbrowser_translate_avahi_event(browser, ev, &null_result, &pal_flags);
    if (null_result)
    {
        if (result == er_ok && !pal_flags)
            return;
        result = browser->cb(browser->context, itf_index, result,
            pal_sd_result_domain, NULL, pal_flags);
    }
    else
    {
        browse_result.domain = domain;
        browse_result.service_name = NULL;
        browse_result.service_type = NULL;

        result = browser->cb(browser->context, itf_index, result,
            pal_sd_result_domain, &browse_result, pal_flags);
    }

    if (result != er_ok)
    {
        if (result != er_aborted)
        {
            log_error(browser->log, "Domain callback returned %s...",
                prx_err_string(result));
        }
        pal_sdbrowser_remove_handle(handle);
    }
}

//
// Browse domains
//
static int32_t pal_sdbrowser_enumerate_domains(
    pal_sdbrowser_t* browser,
    const char* root,
    int32_t itf_index
)
{
    int32_t result;
    pal_sdhandle_t* handle;
    const char* default_domain;
    dbg_assert_ptr(browser);
    dbg_assert_ptr(browser->client);

    avahi_threaded_poll_lock(browser->client->polling);
    do
    {
        result = pal_sdbrowser_add_handle(browser, &handle);
        if (result != er_ok)
            break;

        handle->domains = avahi_domain_browser_new(browser->client->client,
            itf_index == prx_itf_index_all ? AVAHI_IF_UNSPEC : (uint32_t)itf_index,
            AVAHI_PROTO_UNSPEC, root, AVAHI_DOMAIN_BROWSER_BROWSE, 0,
            pal_sdbrowser_enumerate_domains_callback, handle);
        if (!handle->domains)
        {
            result = pal_sdclient_last_avahi_error(browser->client);
            break;
        }
        
        // Send default domain inline...
        default_domain = avahi_client_get_domain_name(browser->client->client);
        pal_sdbrowser_enumerate_domains_callback(
            handle->domains, itf_index, AVAHI_PROTO_UNSPEC,
            default_domain ? AVAHI_BROWSER_NEW : AVAHI_BROWSER_FAILURE,
            default_domain, 0, handle);

        // Done
        result = er_ok;
        handle = NULL;
        break;
    } 
    while (0);
    avahi_threaded_poll_unlock(browser->client->polling);

    if (handle)
        pal_sdbrowser_remove_handle(handle);
    return result;
}

//
// Callback
//
static void pal_sdbrowser_hostname_resolver_callback(
    AvahiHostNameResolver *avahi_handle,
    AvahiIfIndex itf_index,
    AvahiProtocol protocol,
    AvahiResolverEvent ev,
    const char *name,
    const AvahiAddress *address,
    AvahiLookupResultFlags flags,
    void *context
)
{
    int32_t result;
    pal_sdhandle_t* handle = (pal_sdhandle_t*)context;
    pal_sdbrowser_t* browser;
    prx_addrinfo_t addrinfo_result;
    int32_t pal_flags = 0;

    (void)avahi_handle;
    (void)protocol;
    (void)flags;

    dbg_assert_ptr(handle);
    browser = handle->browser;
    dbg_assert_ptr(browser);

    if (ev == AVAHI_RESOLVER_FAILURE)
    {
        result = browser->cb(browser->context, itf_index,
            pal_sdclient_last_avahi_error(browser->client),
            pal_sd_result_addrinfo, NULL, 0);
    }
    else
    {
        result = er_ok;
        memset(&addrinfo_result, 0, sizeof(addrinfo_result));
        addrinfo_result.name = name;

        // Convert address to prx_socket_address_t
        dbg_assert_ptr(address);
        switch (address->proto)
        {
        case AVAHI_PROTO_INET6:
            addrinfo_result.address.un.family = prx_address_family_inet6;
            addrinfo_result.address.un.ip.port = handle->port;
            memcpy(addrinfo_result.address.un.ip.un.in6.un.u8,
                address->data.ipv6.address, sizeof(addrinfo_result.address.un.ip.un.in6.un.u8));
            break;
        case AVAHI_PROTO_INET:
            addrinfo_result.address.un.family = prx_address_family_inet;
            addrinfo_result.address.un.ip.port = handle->port;
            addrinfo_result.address.un.ip.un.in4.un.addr = address->data.ipv4.address;
            break;
        default:
            dbg_assert(0, "Bad family returned");
            addrinfo_result.address.un.family = prx_address_family_unspec;
            result = er_invalid_format;
            break;
        }
        result = browser->cb(browser->context, itf_index, result,
            pal_sd_result_addrinfo, &addrinfo_result, pal_flags);
    }

    if (result == er_ok)
        return;

    pal_sdbrowser_remove_handle(handle);
    if (result != er_aborted)
    {
        log_error(browser->log, "getaddrinfo callback returned %s...",
            prx_err_string(result));
    }
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
    int32_t result;
    pal_sdhandle_t* handle;

    chk_arg_fault_return(browser);
    chk_arg_fault_return(addr);
    dbg_assert_ptr(browser->client);

    avahi_threaded_poll_lock(browser->client->polling);
    do
    {
        result = pal_sdbrowser_add_handle(browser, &handle);
        if (result != er_ok)
            break;

        handle->port = addr->port;
        handle->address = avahi_host_name_resolver_new(browser->client->client,
            itf_index == prx_itf_index_all ? AVAHI_IF_UNSPEC : (uint32_t)itf_index,
            AVAHI_PROTO_UNSPEC, prx_socket_address_proxy_get_host(addr), 
            AVAHI_PROTO_UNSPEC, 0, pal_sdbrowser_hostname_resolver_callback, handle);
        if (!handle->address)
        {
            result = pal_sdclient_last_avahi_error(browser->client);
            break;
        }
      
        // Done
        result = er_ok;
        handle = NULL;
        break;
    } 
    while (0);

    avahi_threaded_poll_unlock(browser->client->polling);

    if (handle)
        pal_sdbrowser_remove_handle(handle);
    return result;
}

//
// Resolve services, or browse services and domains
//
int32_t pal_sdbrowser_browse(
    pal_sdbrowser_t* browser,
    const char* service_name,      // Pass null to browse services
    const char* service_type, // Pass null to browse service types
    const char* domain,           // Pass null for domain browsing
    int32_t itf_index
)
{
    chk_arg_fault_return(browser);

    if (service_name)
        // Resolve service name to host
        return pal_sdbrowser_resolve_service(browser, 
                    service_name, service_type, domain, itf_index);
    if (service_type)
        // Browse
        return pal_sdbrowser_enumerate_services(browser,
                                  service_type, domain, itf_index);
    if (domain)
        // Enumerate service types in domain
        return pal_sdbrowser_enumerate_service_types(browser,
                                                domain, itf_index);

        // Enumerate domains
        return pal_sdbrowser_enumerate_domains(browser,
                                                  NULL, itf_index);
}

//
// Create a browser instance for name service query
//
int32_t pal_sdbrowser_create(
    pal_sdclient_t* client,
    pal_sd_result_cb_t cb,
    void* context,
    pal_sdbrowser_t** created
)
{
    pal_sdbrowser_t* browser;

    chk_arg_fault_return(cb);
    chk_arg_fault_return(created);

    browser = (pal_sdbrowser_t*)mem_zalloc_type(pal_sdbrowser_t);
    if (!browser)
        return er_out_of_memory;

    browser->log = log_get("pal_sd.sdclient");
    browser->cb = cb;
    browser->context = context;
    browser->client = client;
    DList_InitializeListHead(&browser->handles);

    *created = browser;
    return er_ok;
}

//
// Free browser
//
void pal_sdbrowser_free(
    pal_sdbrowser_t* browser
)
{
    if (!browser)
        return;

    dbg_assert_ptr(browser->client);
    if (browser->client->polling)
        avahi_threaded_poll_lock(browser->client->polling);

    // Delete all handles
    while (!DList_IsListEmpty(&browser->handles))
    {
        pal_sdbrowser_remove_handle(containingRecord(
            browser->handles.Flink, pal_sdhandle_t, link));
    }

    if (browser->client->polling)
        avahi_threaded_poll_unlock(browser->client->polling);
    mem_free_type(pal_sdbrowser_t, browser);
}

//
// Create client
//
int32_t pal_sdclient_create(
    pal_sdclient_error_cb_t cb,
    void* context,
    pal_sdclient_t** created
)
{
    int32_t result;
    pal_sdclient_t* client;
    int error;
#if defined(_AVAHI_EMBEDDED)
    AvahiServerConfig config;
#endif

    chk_arg_fault_return(created);
    client = (pal_sdclient_t*)mem_zalloc_type(pal_sdclient_t);
    if (!client)
        return er_out_of_memory;
    do
    {
        client->log = log_get("sd");
        client->cb = cb;
        client->context = context;

        client->polling = avahi_threaded_poll_new();
        if (!client->polling)
        {
            log_error(client->log, "Failed to create simple poll object.");
            result = er_out_of_memory;
            break;
        }

#if defined(_AVAHI_EMBEDDED)
        memset(&config, 0, sizeof(AvahiServerConfig));
        avahi_server_config_init(&config);

        // Unicast DNS server to use for wide area lookup
        avahi_address_parse("192.168.50.1", AVAHI_PROTO_UNSPEC,
            &config.wide_area_servers[0]);

        // See avahi-daemon.conf for help
        config.n_wide_area_servers = 1;
        config.enable_wide_area = 1;
        config.publish_hinfo = 1;
        config.publish_addresses = 1;
        config.publish_workstation = 1;
        config.publish_domain = 1;
        config.publish_a_on_ipv6 = 1;
        config.publish_aaaa_on_ipv4 = 1;
        config.enable_reflector = 1;
        config.reflect_ipv = 1; 

        client->client = avahi_server_new(avahi_threaded_poll_get(client->polling), 
            &config, pal_sdclient_callback, client, &error);
        avahi_server_config_free(&config);
#else
        client->client = avahi_client_new(avahi_threaded_poll_get(client->polling),
            AVAHI_CLIENT_NO_FAIL, pal_sdclient_callback, client, &error);
#endif
        if (!client->client)
        {
            log_error(client->log, "Failed to create client object (%s).",
                avahi_strerror(error));
            result = pal_sd_avahi_error_to_prx_error(error);
            break;
        }

        // Start threaded poll
        if (0 != avahi_threaded_poll_start(client->polling))
        {
            result = er_fatal;
            break;
        }

        *created = client;
        return er_ok;
    } while (0);

    pal_sdclient_free(client);
    return result;
}

//
// Free client
//
void pal_sdclient_free(
    pal_sdclient_t* client
)
{
    if (!client)
        return;

    if (client->polling)
        (void)avahi_threaded_poll_stop(client->polling);
    if (client->client)
        avahi_client_free(client->client);
    if (client->polling)
        avahi_threaded_poll_free(client->polling);

    mem_free_type(pal_sdclient_t, client);
}

//
// Call before using name service layer
//
int32_t pal_sd_init(
    void
)
{
    // No op

    return er_ok;
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
