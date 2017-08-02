// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os.h"
#include "util_mem.h"
#include "pal_net.h"
#include "pal_types.h"
#include "pal_err.h"
#include "util_string.h"
#include "util_misc.h"

//
// Returns a networking stack error as pal error
//
int32_t pal_os_to_prx_net_error(
    int error
)
{
    return pal_os_to_prx_error(error);
}

//
// Returns a networking stack error as pal error
//
int pal_os_from_prx_net_error(
    int32_t error
)
{
    return pal_os_from_prx_error(error);
}

//
// Sets last networking stack error
//
void pal_os_set_net_error_as_prx_error(
    int32_t error
)
{
    pal_os_set_error_as_prx_error(error);
}

//
// Returns a networking stack error as pal error
//
int32_t pal_os_last_net_error_as_prx_error(
    void
)
{
    return pal_os_last_error_as_prx_error();
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
    struct sockaddr* ifaddr = (struct sockaddr*)ifa;
    struct ifaddrs* ifinfo = (struct ifaddrs*)ifi;

    memset(prx_ifa, 0, sizeof(prx_ifaddrinfo_t));
    //
    // Note that we use sizeof(struct sockaddr_in6), assuming here the family
    // in the socket buffer denotes the real struct size and os is returning
    // an address with the right length.
    //
    result = pal_os_to_prx_socket_address(
        ifaddr, sizeof(struct sockaddr_in6), &prx_ifa->address);
    if (result != er_ok)
        return result;

    if (ifinfo->ifa_broadaddr != NULL)
    {
        result = pal_os_to_prx_socket_address(ifinfo->ifa_broadaddr,
            sizeof(struct sockaddr_in6), &prx_ifa->broadcast_addr);
        if (result != er_ok)
            return result;
    }

    if (ifinfo->ifa_flags & IFF_UP)
        prx_ifa->flags |= prx_ifa_up;
    if (ifinfo->ifa_flags & IFF_LOOPBACK)
        prx_ifa->flags |= prx_ifa_loopback;
    if (ifinfo->ifa_flags & IFF_MULTICAST)
        prx_ifa->flags |= prx_ifa_multicast;

    prx_ifa->prefix = prx_invalid_prefix;
    if (ifinfo->ifa_netmask != NULL)
    {
        // Count network prefix bits in the provided netmask
        switch (ifinfo->ifa_netmask->sa_family)
        {
        case AF_INET:
            prx_ifa->prefix = (uint8_t)count_leading_ones_in_buf(
                (uint8_t*)&((struct sockaddr_in*)ifinfo->ifa_netmask)->sin_addr.s_addr, 4);
            break;
        case AF_INET6:
            prx_ifa->prefix = (uint8_t)count_leading_ones_in_buf(
                (uint8_t*)((struct sockaddr_in6*)ifinfo->ifa_netmask)->sin6_addr.s6_addr, 16);
            break;
        default:
            log_error(NULL, "Address family %d not supported. !",
                ifinfo->ifa_netmask->sa_family);
            return er_not_supported;
        }
    }
    strncpy(prx_ifa->name, ifinfo->ifa_name, sizeof(prx_ifa->name));
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
    (void)prx_ifa;
    (void)ifinfo;
    (void)iaddr;
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
    struct ifaddrs *ifaddr = NULL, *ifa;

    (void)flags;

    if (!prx_ifa || !prx_ifa_count)
        return er_fault;

    *prx_ifa_count = 0;
    *prx_ifa = NULL;

    // Get all interface addresses
    result = getifaddrs(&ifaddr);
    do
    {
        if (result != 0)
        {
            result = pal_os_last_net_error_as_prx_error();
            break;
        }

        // First calculate length of returned structs
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
        {
            if (if_name &&
                0 != string_compare(ifa->ifa_name, if_name))
                continue;
            // One address per interface structure
            if (ifa->ifa_addr != NULL)
            {
                if (AF_INET != ifa->ifa_addr->sa_family &&
                    AF_INET6 != ifa->ifa_addr->sa_family)
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
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
        {
            prx_ifaddrinfo_t* prx_ifa_cur = &(*prx_ifa)[*prx_ifa_count];
            if (if_name &&
                0 != string_compare(ifa->ifa_name, if_name))
                continue;
            if (ifa->ifa_addr != NULL)
            {
                if (AF_INET != ifa->ifa_addr->sa_family &&
                    AF_INET6 != ifa->ifa_addr->sa_family)
                    continue;

                result = pal_os_to_prx_ifaddrinfo(ifa, ifa->ifa_addr, prx_ifa_cur);
                if (result != er_ok)
                    continue;

                prx_ifa_cur->index = if_nametoindex(ifa->ifa_name);
                if (prx_ifa_cur->index != 0)  // Returns index 0 on error
                    (*prx_ifa_count)++;
                else
                    result = pal_os_last_net_error_as_prx_error();
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
    while (0);

    if (ifaddr)
        freeifaddrs(ifaddr);
    return result;
}

//
// Frees interface address info
//
int32_t pal_freeifaddrinfo(
    prx_ifaddrinfo_t* info
)
{
    if (!info)
        return er_fault;
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
    (void)if_address;
    (void)if_name;
    (void)if_name_length;
    (void)if_index;
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
    if (!name || namelen <= 0)
        return er_fault;

    result = gethostname((char*)name, (uint32_t)namelen);
    if (result < 0)
    {
        return pal_os_last_net_error_as_prx_error();
    }
    return er_ok;
}

//
// Called before using networking layer
//
int32_t pal_net_init(
    void
)
{
    return er_ok;
}

//
// Free networking layer
//
void pal_net_deinit(
    void
)
{
    // No op
}
