// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _pal_types_h_
#define _pal_types_h_

#include "common.h"
#include "prx_types.h"

//
// Convert platform independent addressinfo flags to OS flags
//
decl_public_2(int32_t, pal_os_from_prx_client_getaddrinfo_flags,
    int32_t, flags,
    int*, platform_flags
);

//
// Convert OS flags to platform independent address info flags
//
decl_public_2(int32_t, pal_os_to_prx_client_getaddrinfo_flags,
    int, flags,
    int32_t*, prx_flags
);

//
// Return platform independent address info from os addrinfo
//
decl_public_2(int32_t, pal_os_to_prx_addrinfo,
    struct addrinfo*, ai,
    prx_addrinfo_t*, prx_ai
);

//
// Return OS address info from platform independent addrinfo
//
decl_public_2(int32_t, pal_os_from_prx_addrinfo,
    prx_addrinfo_t*, prx_ai,
    struct addrinfo*, ai
);

//
// Return OS interface address from platform independent ifaddr
//
decl_public_3(int32_t, pal_os_from_prx_ifaddrinfo,
    prx_ifaddrinfo_t*, prx_ifa,
    ifinfo_t*, ifinfo,
    ifaddr_t*, ifaddr
);

//
// Return platform independent interface address from ifaddrinfo
//
decl_public_3(int32_t, pal_os_to_prx_ifaddrinfo,
    ifinfo_t*, ifinfo,
    ifaddr_t*, ifaddr,
    prx_ifaddrinfo_t*, prx_ifa
);

//
// Convert platform independent getnameinfo flag to OS flags
//
decl_public_2(int32_t, pal_os_from_prx_client_getnameinfo_flags,
    int32_t, flags,
    int*, plat_flags
);

//
// Convert OS getnameinfo flag to platform independent flags
//
decl_public_2(int32_t, pal_os_to_prx_client_getnameinfo_flags,
    int, flags,
    int32_t*, prx_flags
);

//
// Convert a platform independent socket address to OS
//
decl_public_3(int32_t, pal_os_from_prx_socket_address,
    const prx_socket_address_t*, prx_address,
    struct sockaddr*, sa,
    socklen_t*, sa_len
);

//
// Convert a OS socket address to platform independent
//
decl_public_3(int32_t, pal_os_to_prx_socket_address,
    const struct sockaddr*, sa,
    socklen_t, sa_len,
    prx_socket_address_t*, prx_address
);

//
// Convert platform independent message flags to os flags
//
decl_public_2(int32_t, pal_os_from_prx_message_flags,
    int32_t, flags,
    int*, platf_flags
);

//
// Convert os message flags to platform independent flags
//
decl_public_2(int32_t, pal_os_to_prx_message_flags,
    int, flags,
    int32_t*, prx_flags
);

//
// Convert os socket option to platform independent
//
decl_public_3(int32_t, pal_os_to_prx_socket_option,
    int, opt_lvl,
    int, opt_name,
    prx_socket_option_t*, socket_option
);

//
// Convert platform independent to os socket option
//
decl_public_3(int32_t, pal_os_from_prx_socket_option,
    prx_socket_option_t, socket_option,
    int*, opt_lvl,
    int*, opt_name
);

//
// Convert platform independent shutdown op from OS option
//
decl_public_2(int32_t, pal_os_to_prx_shutdown_op,
    int, platform_shutdown,
    prx_shutdown_op_t*, prx_shutdown
);

//
// Convert OS shutdown op to platform independent option
//
decl_public_2(int32_t, pal_os_from_prx_shutdown_op,
    prx_shutdown_op_t, prx_shutdown,
    int*, platform_shutdown
);

//
// Convert OS address family into platform independent
//
decl_public_2(int32_t, pal_os_to_prx_address_family,
    int, platform_af,
    prx_address_family_t*, prx_af
);

//
// Convert platform independent address family into OS
//
decl_public_2(int32_t, pal_os_from_prx_address_family,
    prx_address_family_t, prx_af,
    int*, platform_af
);

//
// Convert platform independent protocol type into OS
//
decl_public_2(int32_t, pal_os_from_prx_protocol_type,
    prx_protocol_type_t, prx_proto,
    int*, platform_proto
);

//
// Convert OS protocol type into platform independent
//
decl_public_2(int32_t, pal_os_to_prx_protocol_type,
    int, platform_proto,
    prx_protocol_type_t*, prx_proto
);

//
// Convert platform independent protocol type into OS
//
decl_public_2(int32_t, pal_os_from_prx_socket_type,
    prx_socket_type_t, prx_socktype,
    int*, platform_socktype
);

//
// Convert OS socket type into platform independent
//
decl_public_2(int32_t, pal_os_to_prx_socket_type,
    int, platform_socktype,
    prx_socket_type_t*, prx_socktype
);

//
// Convert a networking stack error to pal error
//
decl_public_1(int32_t, pal_os_to_prx_net_error,
    int, error
);

//
// Convert a pal error to networking stack error
//
decl_public_1(int, pal_os_from_prx_net_error,
    int32_t, error
);

//
// Convert getnameinfo and getaddrinfo errors
//
decl_public_1(int32_t, pal_os_to_prx_gai_error,
    int, error
);

//
// Convert getnameinfo and getaddrinfo errors
//
decl_public_1(int, pal_os_from_prx_gai_error,
    int32_t, error
);

//
// Convert error from gethostname
//
decl_public_1(int32_t, pal_os_to_prx_h_error,
    int, error
);

//
// Convert error from gethostname
//
decl_public_1(int, pal_os_from_prx_h_error,
    int32_t, error
);


#endif // _pal_types_h_