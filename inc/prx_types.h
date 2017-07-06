// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _prx_types_h_
#define _prx_types_h_

#include "common.h"

//
// Platform independent proxy types. As far as reasonable 
// or possible it follows berkley socket semantics.  
//
// Keep in sync with managed layer, in particular order of
// members!
//

typedef intptr_t prx_fd_t;

#define prx_invalid_socket ((prx_fd_t)(-1))

#define MAX_HOST_LENGTH 1025 
#define MAX_INTERFACE_LENGTH 128
#define MAX_UNIX_PATH_LENGTH 108
#define MAX_PORT_LENGTH 32

//
// Address family (http://www.iana.org assigned numbers)
//
typedef enum prx_address_family
{
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
    union // Start with address, so it can be used as raw buffer
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
    prx_address_family_t family; // Always first so can be cast
    char path[MAX_UNIX_PATH_LENGTH];            // See man page
}
prx_socket_address_unix_t;

//
// IP Socket address
//
typedef struct prx_socket_address_inet
{
    prx_address_family_t family; // Always first so can be cast
    uint16_t port;           // In host byte order, not network
    uint16_t _padding;   
    uint32_t flow;                 // Only valid for in6 family
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
    prx_address_family_t family; // Always first so can be cast
    uint16_t port;           // In host byte order, not network
    uint16_t flags;
    int32_t itf_index;
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
        prx_address_family_t family;   // Always first for cast

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
    prx_socket_address_t address;                    // Address
    int32_t reserved;  // Used to mark whether it needs freeing
    const char* name;             // Canonical name of the host
}
prx_addrinfo_t;

typedef enum prx_ifaddrinfo_flags
{
    prx_ifa_up = 0x1,
    prx_ifa_loopback = 0x2,
    prx_ifa_multicast = 0x4
}
prx_ifaddrinfo_flags_t;

#define prx_itf_index_all ((int32_t)-1)

//
// Platform independent network interface address info
//
typedef struct prx_ifaddrinfo
{
    prx_socket_address_t address;                    // Address
    uint8_t prefix;                       // Subnet mask prefix
    uint8_t flags;
    int16_t reserved;  // Used to mark whether it needs freeing
    char name[128];                        // Name of interface
    int32_t index;                           // Interface index
    prx_socket_address_t broadcast_addr;
}
prx_ifaddrinfo_t;

//
// File types
//
typedef enum prx_file_type
{
    prx_file_type_unknown = 0,
    prx_file_type_file,               // item is a regular file
    prx_file_type_dir,                      // Item is a folder
    prx_file_type_link,                        // Symbolic link
    prx_file_type_bdev,               // Item is a block device
    prx_file_type_cdev,                  // Or character device
    prx_file_type_pipe,
    prx_file_type_sock
}
prx_file_type_t;

//
// Platform independent file stat
//
typedef struct prx_file_info
{
    uint64_t inode_number;
    uint64_t device_id;         // ID of device containing file
    prx_file_type_t type;
    uint64_t total_size;                // total size, in bytes
    uint64_t last_atime;                 // time of last access
    uint64_t last_mtime;                 // time of last change
}
prx_file_info_t;

//
// socket option name for setsockopt and getsockopt
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

    prx_so_ip_multicast_join,
    prx_so_ip_multicast_leave,

    prx_so_props_timeout,

    // ...

    __prx_so_max
}
prx_socket_option_t;

//
// Multicast option
//
typedef struct prx_multicast_option
{
    prx_address_family_t family;   // which address to apply to
    int32_t itf_index;          // Local index of the interface
    union
    {
        prx_inet4_address_t in4;
        prx_inet6_address_t in6;
    } addr;                                // Multicast address
}
prx_multicast_option_t;

//
// Record types
//
typedef enum prx_record_type
{
    prx_record_type_default = 100,                    // Simple
    prx_record_type_txt,                 // txt record property
    prx_record_type_ptr,                 // ptr record property

    // ...

    __prx_record_max
}
prx_record_type_t;

//
// Other property types
//
typedef enum prx_property_type
{
    prx_property_type_file_info = 200,
    prx_property_type_addrinfo,
    prx_property_type_ifaddrinfo,

    // ...

    __prx_property_type_max
}
prx_property_type_t;

//
// Socket property / option value
//
typedef struct prx_property
{
    int32_t type;  // prx_record_type_t, prx_socket_option_t or
    union                                // prx_property_type_t
    {
        uint64_t value;         // Not valid for prx_so_unknown
        prx_multicast_option_t mcast;  // For socket properties

        struct
        {
            size_t size;
            uint8_t* value;
        } bin;                // Free form binary buffer option

        prx_file_info_t file_info;
        prx_addrinfo_t addr_info;
        prx_ifaddrinfo_t itf_info;
    }
    property;
}
prx_property_t;

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
    prx_shutdown_op_read = 0,               // Shutdown reading
    prx_shutdown_op_write = 1,              // Shutdown writing
    prx_shutdown_op_both = 2                // Or shutdown both
}
prx_shutdown_op_t;

//
// Flags for socket properties
//
typedef enum prx_socket_flags
{
    prx_socket_flag_passive = 0x1,         // Socket is passive 
    prx_socket_flag_internal = 0x2, // An internal proxy socket
    prx_socket_flag_persistent = 0x4   // Persistent connection
}
prx_socket_flags_t;

//
// Service ports for internal socket servers
//
typedef enum prx_internal_service_port
{
    prx_internal_service_port_browse = 1,
    // ...
    prx_internal_service_port_invalid = 0
}
prx_internal_service_port_t;

//
// Properties of a socket
//
typedef struct prx_socket_properties
{
    prx_address_family_t family;
    prx_socket_type_t sock_type;
    prx_protocol_type_t proto_type;
    uint32_t flags;                       // prx_socket_flags_t
    uint64_t timeout;    // Timeout of socket after which to gc
    prx_socket_address_t address;      // address of the socket 
    size_t options_len;
    prx_property_t* options;        // options to apply on open
}
prx_socket_properties_t;

#endif // _prx_types_h_
