// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os.h"
#include "util_mem.h"
#include "pal_sd.h"
#include "pal.h"
#include "pal_ev.h"
#include "pal_types.h"
#include "pal_err.h"
#include "util_misc.h"
#include "util_string.h"

#if !defined(UNIT_TEST)
#include "dns_sd.h"
#endif

#include "azure_c_shared_utility/refcount.h"

//
// Dns client wrapper - wraps service ref
//
struct pal_sdclient
{
    DNSServiceRef ref;           // Master connection for browsing
    uintptr_t event_handle;
    pal_sdclient_error_cb_t cb;
    void* context;
    log_t log;
};

DEFINE_REFCOUNT_TYPE(pal_sdclient_t);

//
// Client instance object - wraps a ref counted dns_sd connection
//
struct pal_sdbrowser
{
    DNSServiceRef ref;           // Shared connection for browsing
    pal_sdclient_t* client;          // Reference to client object
    pal_sd_result_cb_t cb;                  // Result callback ...
    void* context;                         // ... and user context
    uint16_t port;
    log_t log;
};

static uintptr_t event_port = 0;

//
// Translate error numbers from dns sd to prx
//
static int32_t pal_sd_mdns_error_to_prx_error(
    DNSServiceErrorType error
)
{
    switch (error)
    {
    case kDNSServiceErr_NoError:
        return er_ok;
    case kDNSServiceErr_NoSuchName:
        return er_not_found;
    case kDNSServiceErr_NoMemory:
        return er_out_of_memory;
    case kDNSServiceErr_BadParam:
        return er_arg;
    case kDNSServiceErr_BadReference:
        return er_fault;
    case kDNSServiceErr_BadState:
        return er_bad_state;
    case kDNSServiceErr_BadFlags:
        return er_bad_flags;
    case kDNSServiceErr_Unsupported:
        return er_not_supported;
    case kDNSServiceErr_NotInitialized:
        return er_fatal;
    case kDNSServiceErr_AlreadyRegistered:
        return er_already_exists;
    case kDNSServiceErr_NameConflict:
        return er_already_exists;
    case kDNSServiceErr_Invalid:
        return er_invalid_format;
    case kDNSServiceErr_Firewall:
        return er_refused;
    case kDNSServiceErr_Incompatible:
        return er_fatal;
    case kDNSServiceErr_BadInterfaceIndex:
        return er_closed;
    case kDNSServiceErr_Refused:
        return er_refused;
    case kDNSServiceErr_NoSuchRecord:
        return er_not_found;
    case kDNSServiceErr_NoAuth:
        return er_permission;
    case kDNSServiceErr_NoSuchKey:
        return er_not_found;
    case kDNSServiceErr_NATTraversal:
        return er_comm;
    case kDNSServiceErr_DoubleNAT:
        return er_bad_state;
    case kDNSServiceErr_BadTime:
        return er_fatal;
    case kDNSServiceErr_BadSig:
        return er_crypto;
    case kDNSServiceErr_BadKey:
        return er_crypto;
    case kDNSServiceErr_Transient:
        return er_retry;
    case kDNSServiceErr_ServiceNotRunning:
        return er_bad_state;
    case kDNSServiceErr_NATPortMappingUnsupported:
        return er_not_supported;
    case kDNSServiceErr_NATPortMappingDisabled:
        return er_bad_state;
    case kDNSServiceErr_NoRouter:
        return er_connecting;
    case kDNSServiceErr_PollingMode:
        return er_comm;
    case kDNSServiceErr_Timeout:
        return er_timeout;
    case kDNSServiceErr_Unknown:
        return er_unknown;
    default:
        return er_fatal;
    }
}

//
// Convert flags
//
static int32_t pal_sd_mdns_flag_to_pal_flags(
    int plat_flags
)
{
    int32_t pal_flags = 0;

    if (0 == (plat_flags & kDNSServiceFlagsAdd))
        pal_flags |= pal_sd_result_removed;

    return pal_flags;
}

//
// Release client
//
static void pal_sdclient_release(
    pal_sdclient_t* client
)
{
    if (!client)
        return;
    if (DEC_REF(pal_sdclient_t, client) == DEC_RETURN_ZERO)
    {
        // Deregister from event port and wait for close complete
        if (client->event_handle != 0)
            pal_event_close(client->event_handle, false);
        else
            pal_sdclient_free(client);
    }
}

//
// Callback
//
static void DNSSD_API pal_sdbrowser_resolve_reply(
    DNSServiceRef ref,
    const DNSServiceFlags flags,
    uint32_t itf_index,
    DNSServiceErrorType error,
    const char *fullname,
    const char *host,
    uint16_t port,
    uint16_t txt_len,
    const unsigned char *txt,
    void *context
)
{
    int32_t result;
    pal_sdbrowser_t* browser = (pal_sdbrowser_t*)context;
    pal_sd_service_entry_t resolve_result;
    prx_socket_address_proxy_t addr;
    const unsigned char *txt_ptr, *txt_max = txt + txt_len;

    (void)flags;
    (void)fullname;
    (void)ref;

    dbg_assert_ptr(browser);
    dbg_assert(browser->ref == ref, "Unexpected ref not equal");

    log_info(browser->log, "Resolved %s", fullname);
    addr.family = prx_address_family_proxy;
    addr.itf_index = itf_index;
    addr.flags = 0;
    addr.port = swap_16(port);
    addr.host_dyn = NULL;
    strncpy(addr.host_fix, host, sizeof(addr.host_fix));
    string_trim_back(addr.host_fix, ".");

    resolve_result.addr = &addr;
    resolve_result.records_len = 0;
    resolve_result.records = NULL;

    // Validate and count number of txt records
    if (error == 0 && txt_len > 1)
    {
        txt_ptr = txt;
        while (txt_ptr < txt_max)
        {
            txt_ptr = txt_ptr + 1 + txt_ptr[0];
            if (txt_ptr > txt_max)
            {
                log_error(browser->log, "bad data in txt record!");
                error = kDNSServiceErr_BadReference;
                break;
            }
            resolve_result.records_len++;
        }
    }

    if (error == 0 && resolve_result.records_len > 0)
    {
        // Allocate variable length record set (todo: cache in browser)
        resolve_result.records = (prx_property_t*)mem_zalloc(
            resolve_result.records_len * sizeof(prx_property_t));
        if (!resolve_result.records)
            error = kDNSServiceErr_NoMemory;
        else
        {
            txt_ptr = txt;
            for (size_t i = 0; i < resolve_result.records_len; i++)
            {
                resolve_result.records[i].type =
                    prx_record_type_txt;
                resolve_result.records[i].property.bin.size =
                    (size_t)txt_ptr[0];
                resolve_result.records[i].property.bin.value =
                    (uint8_t*)txt_ptr + 1;

                txt_ptr = resolve_result.records[i].property.bin.value +
                    resolve_result.records[i].property.bin.size;
            }
        }
    }

    result = browser->cb(browser->context, (int32_t)itf_index,
        pal_sd_mdns_error_to_prx_error(error), pal_sd_result_entry,
        &resolve_result, pal_sd_mdns_flag_to_pal_flags(flags | kDNSServiceFlagsAdd));
    if (result != er_ok)
    {
        if (result != er_aborted)
        {
            log_error(browser->log, "Resolve callback returned %s...",
                prx_err_string(result));
        }
        // Cancel ref
        DNSServiceRefDeallocate(browser->ref);
        browser->ref = NULL;
    }
    if (resolve_result.records)
        mem_free(resolve_result.records);
}

//
// Resolve services, or browse services and domains
//
static int32_t pal_sdbrowser_resolve_service(
    pal_sdbrowser_t* browser,
    const char* service_name,
    const char* service_type,
    const char* domain,
    uint32_t itf_index
)
{
    int32_t result;
    DNSServiceErrorType error;

    chk_arg_fault_return(service_type);

    dbg_assert_ptr(browser);
    dbg_assert_ptr(service_name);

    log_info(browser->log, "Resolving %s.%s.%s", service_name, service_type,
        domain ? domain : "(local)");

    if (browser->ref)
        DNSServiceRefDeallocate(browser->ref);
    browser->ref = browser->client->ref;
    error = DNSServiceResolve(&browser->ref, kDNSServiceFlagsShareConnection,
        itf_index == prx_itf_index_all ? kDNSServiceInterfaceIndexAny :
        (uint32_t)itf_index, service_name, service_type, domain,
        pal_sdbrowser_resolve_reply, browser);
    if (!error)
        return er_ok;

    result = pal_sd_mdns_error_to_prx_error(error);
    log_error(browser->log, "Resolve call failed with %d (%s).", error,
        prx_err_string(result));
    return result;
}

//
// Callback
//
static void DNSSD_API pal_sdbrowser_enumerate_services_reply(
    DNSServiceRef ref,
    const DNSServiceFlags flags,
    uint32_t itf_index,
    DNSServiceErrorType error,
    const char* service_name,
    const char* service_type,
    const char* domain,
    void *context
)
{
    int32_t result;
    pal_sdbrowser_t* browser = (pal_sdbrowser_t*)context;
    pal_sd_browse_result_t browse_result;

    (void)ref;

    dbg_assert_ptr(browser);
    dbg_assert(browser->ref == ref, "Unexpected ref not equal");

    // Clients have to remove any trailing . for efficiency
    browse_result.domain = domain;
    browse_result.service_name = service_name;
    browse_result.service_type = service_type;

    result = browser->cb(browser->context, (int32_t)itf_index,
        pal_sd_mdns_error_to_prx_error(error), pal_sd_result_service,
        &browse_result, pal_sd_mdns_flag_to_pal_flags(flags));
    if (result != er_ok)
    {
        if (result != er_aborted)
        {
            log_error(browser->log, "Browse callback returned %s...",
                prx_err_string(result));
        }
        // Cancel ref
        DNSServiceRefDeallocate(browser->ref);
        browser->ref = NULL;
    }
}

//
// Resolve services, or browse services and domains
//
static int32_t pal_sdbrowser_enumerate_services(
    pal_sdbrowser_t* browser,
    const char* service_type,
    const char* domain,
    uint32_t itf_index
)
{
    int32_t result;
    DNSServiceErrorType error;

    dbg_assert_ptr(browser);
    dbg_assert_ptr(service_type);

    if (browser->ref)
        DNSServiceRefDeallocate(browser->ref);
    browser->ref = browser->client->ref;
    error = DNSServiceBrowse(&browser->ref, kDNSServiceFlagsShareConnection,
        itf_index == prx_itf_index_all ? kDNSServiceInterfaceIndexAny :
        (uint32_t)itf_index, service_type, domain,
        pal_sdbrowser_enumerate_services_reply, browser);
    if (!error)
        return er_ok;

    result = pal_sd_mdns_error_to_prx_error(error);
    log_error(browser->log, "Browse call failed with %d (%s).", error,
        prx_err_string(result));
    return result;
}

//
// Callback
//
static void DNSSD_API pal_sdbrowser_enumerate_service_types_reply(
    DNSServiceRef ref,
    const DNSServiceFlags flags,
    uint32_t itf_index,
    DNSServiceErrorType error,
    const char* service_name,
    const char* service_type,
    const char* domain,
    void *context
)
{
    int32_t result;
    pal_sdbrowser_t* browser = (pal_sdbrowser_t*)context;
    pal_sd_browse_result_t browse_result;
    size_t len;
    char* buf = NULL;

    (void)ref;

    dbg_assert_ptr(browser);
    dbg_assert(browser->ref == ref, "Unexpected ref not equal");

    browse_result.domain = domain;
    browse_result.service_name = NULL;
    browse_result.service_type = service_name;

    if (error == 0)
    {
        //
        // Service type registry responses look like this:
        // Type         Name
        // _tcp.local.  _printer
        // _tcp.local.  _ipp
        // _tcp.local.  _scanner
        // _tcp.local.  _http
        // Make full name and parse it
        //
        len = strlen(service_name) + strlen(service_type) + 2;
        buf = (char*)mem_zalloc(len);
        if (!buf)
            error = kDNSServiceErr_NoMemory;
        else
        {
            // Make a full name and parse it into components
            strcpy(buf, service_name);
            strcat(buf, ".");
            strcat(buf, service_type);
            // Parse resulting string.
            result = string_parse_service_full_name(buf,
                (char**)&browse_result.service_name,
                (char**)&browse_result.service_type,
                (char**)&browse_result.domain);

            dbg_assert(!browse_result.service_name,
                "%s should not contain instance name", buf);
        }
    }

    result = browser->cb(browser->context, (int32_t)itf_index,
        pal_sd_mdns_error_to_prx_error(error), pal_sd_result_type,
        &browse_result, pal_sd_mdns_flag_to_pal_flags(flags));
    if (result != er_ok)
    {
        if (result != er_aborted)
        {
            log_error(browser->log, "Browse callback returned %s...",
                prx_err_string(result));
        }
        // Cancel ref
        DNSServiceRefDeallocate(browser->ref);
        browser->ref = NULL;
    }

    if (buf)
        mem_free(buf);
}

//
// Browse service types
//
static int32_t pal_sdbrowser_enumerate_service_types(
    pal_sdbrowser_t* browser,
    const char* domain,
    uint32_t itf_index
)
{
    int32_t result;
    DNSServiceErrorType error;

    dbg_assert_ptr(browser);
    dbg_assert_ptr(domain);

    if (browser->ref)
        DNSServiceRefDeallocate(browser->ref);
    browser->ref = browser->client->ref;
    error = DNSServiceBrowse(&browser->ref, kDNSServiceFlagsShareConnection,
        itf_index == prx_itf_index_all ? kDNSServiceInterfaceIndexAny :
        (uint32_t)itf_index, "_services._dns-sd._udp", domain,
        pal_sdbrowser_enumerate_service_types_reply, browser);
    if (!error)
        return er_ok;

    result = pal_sd_mdns_error_to_prx_error(error);
    log_error(browser->log, "Browse call failed with %d (%s).", error,
        prx_err_string(result));
    return result;
}

//
// Callback
//
static void DNSSD_API pal_sdbrowser_enumerate_domains_reply(
    DNSServiceRef ref,
    const DNSServiceFlags flags,
    uint32_t itf_index,
    DNSServiceErrorType error,
    const char *domain,
    void *context
)
{
    int32_t result;
    pal_sdbrowser_t* browser = (pal_sdbrowser_t*)context;
    pal_sd_browse_result_t browse_result;

    (void)ref;

    dbg_assert_ptr(browser);
    dbg_assert(browser->ref == ref, "Unexpected ref not equal");

    // Clients have to remove any trailing . for efficiency
    browse_result.domain = domain;
    browse_result.service_name = NULL;
    browse_result.service_type = NULL;

    result = browser->cb(browser->context, (int32_t)itf_index,
        pal_sd_mdns_error_to_prx_error(error), pal_sd_result_domain,
        &browse_result, pal_sd_mdns_flag_to_pal_flags(flags));
    if (result != er_ok)
    {
        if (result != er_aborted)
        {
            log_error(browser->log, "Domain callback returned %s...",
                prx_err_string(result));
        }
        // Cancel ref
        DNSServiceRefDeallocate(browser->ref);
        browser->ref = NULL;
    }
}

//
// Resolve services, or browse services and domains
//
static int32_t pal_sdbrowser_enumerate_domains(
    pal_sdbrowser_t* browser,
    uint32_t itf_index
)
{
    int32_t result;
    DNSServiceErrorType error;

    dbg_assert_ptr(browser);

    if (browser->ref)
        DNSServiceRefDeallocate(browser->ref);
    browser->ref = browser->client->ref;
    error = DNSServiceEnumerateDomains(&browser->ref,
        kDNSServiceFlagsShareConnection | kDNSServiceFlagsBrowseDomains,
        itf_index == prx_itf_index_all ? kDNSServiceInterfaceIndexAny :
        (uint32_t)itf_index, pal_sdbrowser_enumerate_domains_reply, browser);
    if (!error)
        return er_ok;

    result = pal_sd_mdns_error_to_prx_error(error);
    log_error(browser->log, "Domain enum call failed with %d (%s).", error,
        prx_err_string(result));
    return result;
}

//
// Finally deallocate client when socket is destroyed.
//
static void pal_sdclient_on_destroy(
    pal_sdclient_t* client
)
{
    if (!client)
        return;
    if (client->ref)
        DNSServiceRefDeallocate(client->ref);

    mem_free_type(pal_sdclient_t, client);
}

//
// In case of daemon communication error notify client to reconnect.
//
static void pal_sdclient_on_error(
    pal_sdclient_t* client,
    int32_t error_code
)
{
    if (client->cb)
    {
        client->cb(client->context, error_code);
        client->cb = NULL; // One shot callback
    }
}

//
// Called by event port when event occurred on registered socket
//
static int32_t pal_sdclient_socket_callback(
    void* context,
    pal_event_type_t event_type,
    int32_t error_code
)
{
    DNSServiceErrorType error;
    pal_sdclient_t* client = (pal_sdclient_t*)context;

    dbg_assert_ptr(client);
    switch (event_type)
    {
    case pal_event_type_read:
        error = DNSServiceProcessResult(client->ref);
        if (error == kDNSServiceErr_NoError)
            break;
        error_code = pal_sd_mdns_error_to_prx_error(error);
        pal_sdclient_on_error(client, error_code);
        break;
    case pal_event_type_write:
        break;
    case pal_event_type_error:
        pal_sdclient_on_error(client, error_code);
        break;
    case pal_event_type_close:
        pal_sdclient_on_error(client, er_closed);
        break;
    case pal_event_type_destroy:
        pal_sdclient_on_destroy(client);
        break;
    case pal_event_type_unknown:
    default:
        dbg_assert(0, "Unknown event type %d", event_type);
        return er_bad_state;
    }
    return er_ok;
}

//
// Callback
//
static void DNSSD_API pal_sdbrowser_getaddrinfo_reply(
    DNSServiceRef ref,
    DNSServiceFlags flags,
    uint32_t itf_index,
    DNSServiceErrorType error,
    const char *hostname,
    const struct sockaddr *address,
    uint32_t ttl,
    void *context
)
{
    int32_t result;
    pal_sdbrowser_t* browser = (pal_sdbrowser_t*)context;
    prx_addrinfo_t addrinfo_result;
    socklen_t address_len;

    (void)ref;
    (void)ttl;

    dbg_assert_ptr(browser);
    dbg_assert(browser->ref == ref, "Unexpected ref not equal");

    memset(&addrinfo_result, 0, sizeof(addrinfo_result));
    addrinfo_result.name = hostname;

    if (error != 0)
        result = pal_sd_mdns_error_to_prx_error(error);
    else
    {
        dbg_assert_ptr(address);
        switch (address->sa_family)
        {
        case AF_INET6:
            address_len = sizeof(struct sockaddr_in6);
            break;
        case AF_INET:
            address_len = sizeof(struct sockaddr_in);
            break;
        default:
            dbg_assert(0, "Bad family returned");
            address_len = 0;
            break;
        }
        result = pal_os_to_prx_socket_address(address, address_len,
            &addrinfo_result.address);
        addrinfo_result.address.un.ip.port = browser->port;
    }
    result = browser->cb(browser->context, (int32_t)itf_index,
        result, pal_sd_result_addrinfo, &addrinfo_result,
        pal_sd_mdns_flag_to_pal_flags(flags));
    if (result != er_ok)
    {
        if (result != er_aborted)
        {
            log_error(browser->log, "getaddrinfo callback returned %s...",
                prx_err_string(result));
        }
        // Cancel ref
        DNSServiceRefDeallocate(browser->ref);
        browser->ref = NULL;
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
    DNSServiceErrorType error;
    DNSServiceProtocol plat_proto;

    chk_arg_fault_return(browser);
    chk_arg_fault_return(addr);

    plat_proto = kDNSServiceProtocol_IPv4 | kDNSServiceProtocol_IPv6;

    if (browser->ref)
        DNSServiceRefDeallocate(browser->ref);
    browser->ref = browser->client->ref;
    browser->port = addr->port;
    error = DNSServiceGetAddrInfo(&browser->ref, kDNSServiceFlagsShareConnection,
        itf_index == prx_itf_index_all ? kDNSServiceInterfaceIndexAny :
        (uint32_t)itf_index, plat_proto, prx_socket_address_proxy_get_host(addr),
        pal_sdbrowser_getaddrinfo_reply, browser);
    if (!error)
        return er_ok;

    result = pal_sd_mdns_error_to_prx_error(error);
    log_error(browser->log, "getaddrino call failed with %d (%s).", error,
        prx_err_string(result));
    return result;
}

//
// Resolve services, or browse services and domains
//
int32_t pal_sdbrowser_browse(
    pal_sdbrowser_t* browser,
    const char* service_name,       // Pass null to browse services
    const char* service_type,  // Pass null to browse service types
    const char* domain,            // Pass null for domain browsing
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
        return pal_sdbrowser_enumerate_domains(browser, itf_index);
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
    int32_t result;
    pal_sdbrowser_t* browser;

    chk_arg_fault_return(cb);
    chk_arg_fault_return(created);

    browser = (pal_sdbrowser_t*)mem_zalloc_type(pal_sdbrowser_t);
    if (!browser)
        return er_out_of_memory;
    do
    {
        browser->log = log_get("pal_sd.sdclient");
        browser->cb = cb;
        browser->context = context;

        result = pal_event_select(client->event_handle, pal_event_type_read);
        if (result != er_ok)
            break;

        browser->client = client;
        INC_REF(pal_sdclient_t, client);
        *created = browser;
        return er_ok;
    }
    while (0);

    pal_sdbrowser_free(browser);
    return result;
}

//
// Release browser
//
void pal_sdbrowser_free(
    pal_sdbrowser_t* browser
)
{
    if (!browser)
        return;
    if (browser->ref)
        DNSServiceRefDeallocate(browser->ref);
    if (browser->client)
        pal_sdclient_release(browser->client);

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
    DNSServiceErrorType error;
    pal_sdclient_t* client;
    dnssd_sock_t sock;

    chk_arg_fault_return(created);
    if (!event_port)
        return er_bad_state;

    client = (pal_sdclient_t*)REFCOUNT_TYPE_CREATE(pal_sdclient_t);
    if (!client)
        return er_out_of_memory;
    memset(client, 0, sizeof(pal_sdclient_t));
    do
    {
        client->log = log_get("sd");
        client->cb = cb;
        client->context = context;

        error = DNSServiceCreateConnection(&client->ref);
        if (error != 0)
        {
            result = pal_sd_mdns_error_to_prx_error(error);
            log_error(client->log,
                "Failed creating connection to service: %d (%s)",
                error, prx_err_string(result));
            break;
        }

        // Register ref with event port
        sock = DNSServiceRefSockFD(client->ref);
        if (sock == _invalid_fd)
        {
            log_error(client->log, "Failed to get ref to socket!");
            result = er_fatal;
            break;
        }

        result = pal_event_port_register(event_port, sock,
            pal_sdclient_socket_callback, client, &client->event_handle);
        if (result != er_ok)
            break;
        result = pal_event_select(client->event_handle, pal_event_type_close);
        if (result != er_ok)
            break;
        *created = client;
        return er_ok;
    } while (0);

    pal_sdclient_release(client);
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

    client->cb = NULL;
    client->context = NULL;

    pal_sdclient_release(client);
}

//
// Call before using name service layer
//
int32_t pal_sd_init(
    void
)
{
    DNSServiceErrorType error;
    uint32_t version;
    uint32_t size = sizeof(version);
    int32_t result;
    int attempt = 0;

    result = pal_event_port_create(NULL, NULL, &event_port);
    if (result != er_ok)
    {
        log_error(NULL, "FATAL: Failed creating event port.");
        return result;
    }
    
    while (true)
    {
        error = DNSServiceGetProperty(kDNSServiceProperty_DaemonVersion,
            &version, &size);
        if (error == 0)
        {
            log_info(NULL, "Using Bonjour (Version %d.%d)\n",
                version / 10000, version / 100 % 100);
            return er_ok;
        }

        log_error(NULL, "%d: Cannot connect to local Bonjour Service. (Error %d)"
            "Ensure the service is installed and running...", attempt, error);
        
        if (++attempt > 10)
        {
            if (error == kDNSServiceErr_ServiceNotRunning)
            {
                log_error(NULL, "  ... Giving up to connect to Bonjour -"
                    " service discovery not supported on this platform.");
                result = er_not_supported;  
            }
            else
                result = pal_sd_mdns_error_to_prx_error(error);
        }
    }
    
    pal_event_port_close(event_port);
    event_port = 0;
    return result;
}

//
// Free name service layer
//
void pal_sd_deinit(
    void
)
{
    if (event_port)
        pal_event_port_close(event_port);
    event_port = 0;
}
