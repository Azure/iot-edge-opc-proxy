// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os.h"
#include "util_mem.h"
#include "pal_net.h"
#include "pal_types.h"
#include "pal_err.h"
#include "util_string.h"

//
// Returns a networking stack error as pal error
//
int32_t pal_os_to_prx_net_error(
    int error
)
{
    switch (error)
    {
    case 0:                           return er_ok;
    case WSAEINTR:                    return er_aborted;
    case WSAEBADF:                    return er_arg;
    case WSAEACCES:                   return er_permission;
    case WSAEFAULT:                   return er_fault;
    case WSAEINVAL:                   return er_arg;
    case WSAEMFILE:                   return er_out_of_memory;
    case WSAEWOULDBLOCK:              return er_retry;
    case WSAEINPROGRESS:              return er_retry;
    case WSAEALREADY:                 return er_waiting;
    case WSAENOTSOCK:                 return er_arg;
    case WSAEDESTADDRREQ:             return er_arg;
    case WSAEMSGSIZE:                 return er_arg;
    case WSAEPROTOTYPE:               return er_arg;
    case WSAENOPROTOOPT:              return er_arg;
    case WSAEPROTONOSUPPORT:          return er_not_impl;
    case WSAEOPNOTSUPP:               return er_not_impl;
    case WSAEAFNOSUPPORT:             return er_not_impl;
    case WSAEADDRINUSE:               return er_busy;
    case WSAEADDRNOTAVAIL:            return er_not_found;
    case WSAHOST_NOT_FOUND:           return er_host_unknown;
    case WSAENETDOWN:                 return er_network;
    case WSAENETUNREACH:              return er_undelivered;
    case WSAENETRESET:                return er_network;
    case WSAECONNABORTED:             return er_closed;
    case WSAECONNRESET:               return er_closed;
    case WSAENOBUFS:                  return er_out_of_memory;
    case WSAEISCONN:                  return er_connecting;
    case WSAENOTCONN:                 return er_closed;
    case WSAETIMEDOUT:                return er_timeout;
    case WSAECONNREFUSED:             return er_refused;
    case WSAELOOP:                    return er_arg;
    case WSAENAMETOOLONG:             return er_arg;
    case WSAEHOSTUNREACH:             return er_connecting;
    case WSAENOTEMPTY:                return er_disk_io;
    case WSA_IO_PENDING:              return er_waiting;
    case WSA_IO_INCOMPLETE:           return er_waiting;
    case WSA_INVALID_HANDLE:          return er_arg;
    case WSA_INVALID_PARAMETER:       return er_arg;
    case WSA_NOT_ENOUGH_MEMORY:       return er_out_of_memory;
    case WSA_OPERATION_ABORTED:       return er_aborted;
    case WSAESOCKTNOSUPPORT:          return er_not_impl;
    case WSAEPFNOSUPPORT:             return er_not_impl;
    case WSAESHUTDOWN:                return er_shutdown;
    case WSAETOOMANYREFS:             return er_out_of_memory;
    case WSAEHOSTDOWN:                return er_connecting;
    case WSAEPROCLIM:                 return er_out_of_memory;
    case WSAEUSERS:                   return er_out_of_memory;
    case WSAEDQUOT:                   return er_out_of_memory;
    case WSAESTALE:                   return er_disk_io;
    case WSAEREMOTE:                  return er_disk_io;
    case WSAEDISCON:                  return er_closed;
    case WSAENOMORE:                  return er_nomore;
    case WSAECANCELLED:               return er_aborted;
    case WSAEREFUSED:                 return er_refused;
    case WSANOTINITIALISED:           return er_bad_state;
    case WSASYSCALLFAILURE:           return er_unknown;
    default:
        dbg_assert(0, "Unknown os error %d", error);
    }
    return er_unknown;
}

//
// Returns a networking stack error as pal error
//
int pal_os_from_prx_net_error(
    int32_t error
)
{
    switch (error)
    {
    case er_ok:                       return 0;
    case er_out_of_memory:            return WSA_NOT_ENOUGH_MEMORY;
    case er_permission:               return WSAEACCES;
    case er_fault:                    return WSAEFAULT;
    case er_arg:                      return WSAEINVAL;
    case er_retry:                    return WSAEWOULDBLOCK;
    case er_waiting:                  return WSAEALREADY;
    case er_not_impl:                 return WSAEOPNOTSUPP;
    case er_not_supported:            return WSAEOPNOTSUPP;
    case er_busy:                     return WSAEADDRINUSE;
    case er_not_found:                return WSAEADDRNOTAVAIL;
    case er_host_unknown:             return WSAHOST_NOT_FOUND;
    case er_network:                  return WSAENETDOWN;
    case er_connecting:               return WSAECONNABORTED;
    case er_closed:                   return WSAENOTCONN;
    case er_shutdown:                 return WSAESHUTDOWN;
    case er_refused:                  return WSAEREFUSED;
    case er_timeout:                  return WSAETIMEDOUT;
    case er_disk_io:                  return WSAENOTEMPTY;
    case er_bad_state:                return WSANOTINITIALISED;
    case er_nomore:                   return WSAENOMORE;
    case er_aborted:                  return WSAECANCELLED;
    case er_unknown:                  return WSASYSCALLFAILURE;
    case er_undelivered:              return WSAENETUNREACH;
    default:
        dbg_assert(0, "Unknown pi error %d", error);
    }
    return -1;
}

//
// Sets last networking stack error
//
void pal_os_set_net_error_as_prx_error(
    int32_t error
)
{
    int32_t result;

    if (error != er_ok)
    {
        log_debug(NULL, "Error occurred: %s", prx_err_string(error));
    }

    result = pal_os_from_prx_net_error(error);

    WSASetLastError(result);
}

//
// Returns a networking stack error as pal error
//
int32_t pal_os_last_net_error_as_prx_error(
    void
)
{
    int32_t error;
    char* message = NULL;

    error = WSAGetLastError();
    if (error != 0 &&
        error != WSAEWOULDBLOCK &&
        error != WSA_IO_PENDING)
    {
        FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (char*)&message, 0, NULL);
        if (message)
            string_trim_back(message, "\r\n\t ");
        log_info(NULL, "Networking error code %d (%s)",
            error, message ? message : "<unknown>");
        LocalFree(message);
    }
    return pal_os_to_prx_net_error(error);
}

//
// Return platform independent interface address from ifaddrinfo
//
int32_t pal_os_to_prx_ifaddrinfo(
    ifinfo_t* ifi,
    ifaddr_t* ifa,
    prx_ifaddrinfo_t* prx_ifa
)
{
    int32_t result;

    IP_ADAPTER_UNICAST_ADDRESS* ifaddr = (IP_ADAPTER_UNICAST_ADDRESS*)ifa;
    IP_ADAPTER_ADDRESSES* ifinfo = (IP_ADAPTER_ADDRESSES*)ifi;

    memset(prx_ifa, 0, sizeof(prx_ifaddrinfo_t));
    result = pal_os_to_prx_socket_address(
        ifaddr->Address.lpSockaddr, ifaddr->Address.iSockaddrLength, &prx_ifa->address);
    if (result != er_ok)
        return result;

    if (IfOperStatusUp == ifinfo->OperStatus)
        prx_ifa->flags |= prx_ifa_up;
    if (IF_TYPE_SOFTWARE_LOOPBACK == ifinfo->IfType)
        prx_ifa->flags |= prx_ifa_loopback;
    if (0 == (ifinfo->Flags & IP_ADAPTER_NO_MULTICAST))
        prx_ifa->flags |= prx_ifa_multicast;

    prx_ifa->prefix = ifaddr->OnLinkPrefixLength;
    prx_ifa->index = ifinfo->IfIndex;
    strncpy(prx_ifa->name, ifinfo->AdapterName, sizeof(prx_ifa->name));
    return er_ok;
}

//
// Return OS interface address from platform independent ifaddr
//
int32_t pal_os_from_prx_ifaddrinfo(
    prx_ifaddrinfo_t* prx_ifa,
    ifinfo_t* ifinfo,
    ifaddr_t* iaddr
)
{
    chk_arg_fault_return(prx_ifa);
    chk_arg_fault_return(ifinfo);
    chk_arg_fault_return(iaddr);
    return er_not_impl;
}

//
// Look up interface addresses
//
int32_t pal_getifaddrinfo(
    const char* if_name,
    uint32_t flags,
    prx_ifaddrinfo_t** prx_ifa,
    size_t* prx_ifa_count
)
{
    int32_t result;
    size_t alloc_count = 0;
    PIP_ADAPTER_ADDRESSES ifaddr = NULL, ifa;
    IP_ADAPTER_UNICAST_ADDRESS *uai;
    ULONG alloc_size = 15000;
    (void)flags;

    chk_arg_fault_return(prx_ifa);
    chk_arg_fault_return(prx_ifa_count);

    *prx_ifa_count = 0;
    *prx_ifa = NULL;

    // Get all interface addresses
    while (true)
    {
        ifa = (PIP_ADAPTER_ADDRESSES)mem_realloc(ifaddr, alloc_size);
        if (!ifa)
        {
            result = er_out_of_memory;
            break;
        }

        ifaddr = ifa;
        result = GetAdaptersAddresses(AF_UNSPEC,
            GAA_FLAG_INCLUDE_PREFIX |
            GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_FRIENDLY_NAME |
            GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST,
            NULL, ifaddr, &alloc_size);

        if (result == ERROR_BUFFER_OVERFLOW)
            continue;
        else if (result != 0)
        {
            result = pal_os_to_prx_error(result);
            break;
        }

        // First calculate length of returned structs
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->Next)
        {
            if (if_name &&
                0 != string_compare(ifa->AdapterName, if_name))
                continue;
            // Multiple addresses per adapter possible
            for (uai = ifa->FirstUnicastAddress; uai; uai = uai->Next)
            {
                if (AF_INET != uai->Address.lpSockaddr->sa_family &&
                    AF_INET6 != uai->Address.lpSockaddr->sa_family)
                    continue;
                ++alloc_count;
            }
        }

        if (alloc_count == 0)
        {
            result = er_not_found;
            break;
        }

        // then alloc a flat buffer of ifaddrinfo structures
        *prx_ifa = (prx_ifaddrinfo_t*)mem_zalloc(
            (alloc_count + 1) * sizeof(prx_ifaddrinfo_t));
        if (!*prx_ifa)
        {
            result = er_out_of_memory;
            break;
        }

        // and copy os ifaddr into flat buffer
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->Next)
        {
            prx_ifaddrinfo_t* prx_ifa_cur = &(*prx_ifa)[*prx_ifa_count];
            if (if_name &&
                0 != string_compare(ifa->AdapterName, if_name))
                continue;
            for (uai = ifa->FirstUnicastAddress; uai; uai = uai->Next)
            {
                if (AF_INET != uai->Address.lpSockaddr->sa_family &&
                    AF_INET6 != uai->Address.lpSockaddr->sa_family)
                    continue;

                result = pal_os_to_prx_ifaddrinfo(ifa, uai, prx_ifa_cur);
                if (result == er_ok)
                    (*prx_ifa_count)++;
            }
        }

        // If none were copied even though we had found some, return error
        if (*prx_ifa_count == 0)
        {
            log_error(NULL, "Error: %d ifaddrinfo(s) available, but failed to convert any! (%s)",
                alloc_count, prx_err_string(result));

            mem_free(*prx_ifa);
            *prx_ifa = NULL;

            if (result != er_ok)
                break;
            result = er_not_found;
            break;
        }

        // Otherwise success
        result = er_ok;
        break;
    }

    if (ifaddr)
        mem_free(ifaddr);
    return result;
}

//
// Frees interface address info
//
int32_t pal_freeifaddrinfo(
    prx_ifaddrinfo_t* info
)
{
    chk_arg_fault_return(info);
    mem_free(info);
    return er_ok;
}

//
// Inverse of getifaddrinfo, converts interface address
//
int32_t pal_getifnameinfo(
    prx_socket_address_t* if_address,
    char* if_name,
    size_t if_name_length,
    uint64_t *if_index
)
{
    chk_arg_fault_return(if_address);
    chk_arg_fault_return(if_name);
    chk_arg_fault_return(if_index);
    chk_arg_fault_return(if_name_length);
    return er_not_impl;
}

//
// Return host name
//
int32_t pal_gethostname(
    char* name,
    size_t namelen
)
{
    int32_t result;
    chk_arg_fault_return(name);
    chk_arg_fault_return(namelen);

    result = gethostname((char*)name, (uint32_t)namelen);
    if (result >= 0)
        result = er_ok;
    else
    {
        result = pal_os_last_net_error_as_prx_error();
        if (GetComputerNameA((LPSTR)name, (LPDWORD)&namelen))
            result = er_ok;
    }
    return result;
}

//
// Called before using networking layer
//
int32_t pal_net_init(
    void
)
{
    int error;
    WSADATA wsd;
    error = WSAStartup(MAKEWORD(2, 2), &wsd);
    if (error != 0)
        return pal_os_to_prx_net_error(error);
    return er_ok;
}

//
// Free networking layer
//
void pal_net_deinit(
    void
)
{
    (void)WSACleanup();
}
