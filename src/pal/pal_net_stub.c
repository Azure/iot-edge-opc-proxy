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
    (void)ifi;
    (void)ifa;
    (void)prx_ifa;
    return er_not_supported;
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
    return er_not_supported;
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
    (void)if_name;
    (void)flags;
    (void)prx_ifa;
    (void)prx_ifa_count;
    return er_not_supported;
}

//
// Frees interface address info
//
int32_t pal_freeifaddrinfo(
    prx_ifaddrinfo_t* info
)
{
    (void)info;
    return er_not_supported;
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
    return er_not_supported;
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
