// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _aprx_types_h_
#define _aprx_types_h_

#include "common.h"

//
// Platform independent proxy types. As far as reasonable 
// or possible it follows berkley socket semantics.  
//
// Keep in sync with managed layer, in particular order of members!
//

typedef intptr_t prx_fd_t;
typedef uint32_t prx_size_t;

#define prx_invalid_socket ((prx_fd_t)(-1))


#define MAX_HOST_LENGTH 128 // 1025 - TODO: Allocate host buffer
#define MAX_INTERFACE_LENGTH 128
#define MAX_UNIX_PATH_LENGTH 108
#define MAX_PORT_LENGTH 32

//
// Address family
//
typedef enum prx_address_family
{
    // Standard http://www.iana.org assigned numbers
    prx_address_family_unspec = 0,     
    prx_address_family_unix = 1,
    prx_address_family_inet = 2,
    prx_address_family_inet6 = 23,

    // Non standard first come first serve family for proxy
    prx_address_family_proxy = 28165   
}
prx_address_family_t;

//
// ipv4 address
//
typedef struct
{
    union
    {
        uint32_t addr;
        uint8_t u8[4];
    }
	un;
}
prx_inet4_address_t;

//
// ipv6 address
//
typedef struct
{
    union
    {
        uint32_t addr_4;
        uint8_t u8[16];
        uint16_t u16[8];
        uint32_t u32[4];
        uint64_t u64[2];
    }
	un;
    uint32_t scope_id;
}
prx_inet6_address_t;

#define prx_invalid_prefix 0xff

//
// Universal IP address type
//
typedef struct prx_inet_address
{
    union    // Start with address, so it can be used as raw buffer
    {
        prx_inet4_address_t in4;
        prx_inet6_address_t in6;

        char raw[sizeof(prx_inet6_address_t)];
    }
	un;
    uint8_t prefix; // Subnet prefix
}
prx_inet_address_t;

//
// Unix socket address (prx_address_family_unix)
//
typedef struct prx_socket_address_unix
{
    prx_address_family_t family;      // Always first so can be cast
    char path[MAX_UNIX_PATH_LENGTH];                // See man page
}
prx_socket_address_unix_t;

//
// IP Socket address
//
typedef struct prx_socket_address_inet
{
    prx_address_family_t family;      // Always first so can be cast
    uint32_t flow;
    uint16_t port;               // In host byte order, not network
    uint16_t reserved;
    union
    {
        prx_inet4_address_t in4;
        prx_inet6_address_t in6;

        char raw[sizeof(prx_inet6_address_t)];
    }
	un;
}
prx_socket_address_inet_t;

//
// Proxy socket address (prx_address_family_proxy)
//
typedef struct prx_socket_address_proxy
{
    prx_address_family_t family;      // Always first so can be cast
    uint32_t flow;
    uint16_t port;               // In host byte order, not network
    uint16_t reserved;
    char host[MAX_HOST_LENGTH];
}
prx_socket_address_proxy_t;

//
// Inet socket address type
//
typedef struct prx_socket_address
{
    union
    {
        prx_address_family_t family;  // Always first so can be cast

        prx_socket_address_inet_t ip;
        prx_socket_address_unix_t ux;
        prx_socket_address_proxy_t proxy;

        char raw[sizeof(prx_socket_address_proxy_t)];
    }
	un;
}
prx_socket_address_t;

//
// Socket type
//
typedef enum prx_socket_type 
{
    prx_socket_type_stream = 1,        
    prx_socket_type_dgram = 2,         
    prx_socket_type_raw = 3,           
    prx_socket_type_rdm = 4,           
    prx_socket_type_seqpacket = 5      
}
prx_socket_type_t;

//
// Socket protocol
//
typedef enum prx_protocol_type
{
    prx_protocol_type_unspecified = 0, 
    prx_protocol_type_icmp = 1,        
    prx_protocol_type_tcp = 6,         
    prx_protocol_type_udp = 17,        
    prx_protocol_type_icmpv6 = 58      
}
prx_protocol_type_t;

//
// flags used in calls to getnameinfo
//
typedef enum prx_client_getnameinfo_flags
{
    prx_ni_flag_namereqd = 0x1
}
prx_client_getnameinfo_flags_t;

//
// flags used in calls to getaddrinfo
//
typedef enum prx_client_getaddrinfo_flags
{
    prx_ai_passive = 0x1
}
prx_client_getaddrinfo_flags_t;

//
// prx_addrinfo provides a platform independent host resolution
//
typedef struct prx_addrinfo
{
    prx_socket_address_t address;                // Address
    int64_t reserved;
    char* name;              // Canonical name of the host
}
prx_addrinfo_t;

typedef enum prx_ifaddrinfo_flags
{
    prx_ifa_up = 0x1,
    prx_ifa_loopback = 0x2,
    prx_ifa_multicast = 0x4
}
prx_ifaddrinfo_flags_t;

//
// Platform independent network interface address info
//
typedef struct prx_ifaddrinfo
{
    prx_socket_address_t address;                // Address
    uint8_t prefix;                  // Subnet mask prefix
    uint8_t flags;
    uint16_t reserved;
    char name[128];                   // Name of interface
    int32_t index;                      // Interface index
    prx_socket_address_t broadcast_addr;
}
prx_ifaddrinfo_t;

//
// socket option name for prx_client_setsockopt and prx_client_getsockopt
//
typedef enum prx_socket_option
{
    prx_so_unknown,
    prx_so_nonblocking,
    prx_so_available,
    prx_so_shutdown,
    prx_so_debug,
    prx_so_acceptconn,
    prx_so_reuseaddr,
    prx_so_keepalive,
    prx_so_dontroute,
    prx_so_broadcast,
    prx_so_linger,
    prx_so_oobinline,
    prx_so_sndbuf,
    prx_so_rcvbuf,
    prx_so_sndlowat,
    prx_so_rcvlowat,
    prx_so_sndtimeo,
    prx_so_rcvtimeo,
    prx_so_error,
    prx_so_type,
    prx_so_ip_options,
    prx_so_ip_hdrincl,
    prx_so_ip_tos,
    prx_so_ip_ttl,
    prx_so_ip_multicast_ttl,
    prx_so_ip_multicast_loop,
    prx_so_ip_pktinfo,
    prx_so_ipv6_hoplimit,
    prx_so_ipv6_protection_level,
    prx_so_ipv6_v6only,
    prx_so_tcp_nodelay,
    // ...
    __prx_so_max
}
prx_socket_option_t;

//
// Socket option value
//
typedef struct prx_socket_option_value
{
    prx_socket_option_t option; 
    uint64_t value;             // Not valid if option is prx_so_unknown
}
prx_socket_option_value_t;

//
// Flags for socket properties
//
typedef enum prx_client_socket_flags
{
    socket_flag_passive = 0x1       // Whether socket is passive socket
}
prx_client_socket_flags_t;

//
// Multicast option
//
typedef struct prx_multicast_option
{
    prx_address_family_t family;            // which address to apply to
    union
    {
        prx_inet4_address_t in4;  
        prx_inet6_address_t in6;  
    } address;                                     // Multicast address
    int32_t interface_index;            // Local ondex of the interface
}
prx_multicast_option_t;

// 
// message flags for calls to send and recv
// 
typedef enum prx_message_flags
{
    prx_msg_flag_oob = 0x0001,       
    prx_msg_flag_peek = 0x0002,      
    prx_msg_flag_dontroute = 0x0004, 
    prx_msg_flag_trunc = 0x0100,     
    prx_msg_flag_ctrunc = 0x0200     
}
prx_message_flags_t;

//
// Shutdown operation
//
typedef enum prx_shutdown_op
{
    prx_shutdown_op_read = 0,                        // Shutdown reading
    prx_shutdown_op_write = 1,                     // Shutdown writing
    prx_shutdown_op_both = 2                         // Or shutdown both
}
prx_shutdown_op_t;

//
// Properties of a socket
//
typedef struct prx_socket_properties
{
    prx_address_family_t family;
    prx_socket_type_t sock_type;
    prx_protocol_type_t proto_type;
    uint32_t flags;                                  // socket_flags_t
    prx_socket_address_t address;
    prx_socket_option_value_t options[__prx_so_max]; // options to apply
}
prx_socket_properties_t;


#endif // _aprx_types_h_
