// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os.h"
#include "util_mem.h"
#include "pal_net.h"
#include "pal.h"
#include "pal_types.h"
#include "pal_err.h"
#include "util_misc.h"
#include "util_string.h"

#define MAX_SOCKET_ADDRESS_BYTES 127

//
// Convert getnameinfo and getaddrinfo errors to pal
//
int32_t pal_os_to_prx_gai_error(
    int error
)
{
    switch (error)
    {
    case 0:               
        return er_ok;
    case EAI_AGAIN:         
        return er_retry;
    case EAI_BADFLAGS:      
        return er_bad_flags;
    case EAI_FAMILY:       
        return er_address_family;
    case EAI_NONAME:     
        return er_host_unknown;
    default:
        /**/ if (error == EAI_NODATA)
            return er_host_unknown;
        else if (error == EAI_FAIL)
            return er_fatal;
        dbg_assert(0, "Unknown getaddrinfo os error");
    }
    return er_unknown;
}

//
// Convert getnameinfo and getaddrinfo errors
//
int pal_os_from_prx_gai_error(
    int32_t error
)
{
    switch (error)
    {
    case er_ok:             
        return 0;
    case er_retry:          
        return EAI_AGAIN;
    case er_bad_flags:      
        return EAI_BADFLAGS;
    case er_address_family: 
        return EAI_FAMILY;
    case er_fatal:         
        return EAI_FAIL;
    case er_host_unknown:   
        return EAI_NONAME;
    default:
        dbg_assert(0, "Unknown getaddrinfo pi error");
    }

    return EAI_NONAME;
}

// 
// Convert error from gethostname to pal error
//
int32_t pal_os_to_prx_h_error(
    int error
)
{
    switch (error)
    {
    case HOST_NOT_FOUND:  
        return er_no_host;
    case TRY_AGAIN:       
        return er_retry;
    case NO_RECOVERY:     
        return er_fatal;
    case NO_DATA:         
        return er_no_address;
    default:
        dbg_assert(0, "Unknown gethostbyname/gethostbyaddr os error");
    }
    return er_unknown;
}

// 
// Convert error from gethostname
//
int pal_os_from_prx_h_error(
    int32_t error
)
{
    switch (error)
    {
    case er_retry:     
        return TRY_AGAIN;
    case er_fatal:     
        return NO_RECOVERY;
    case er_no_address:   
        return NO_DATA;
    case er_no_host:    
        return HOST_NOT_FOUND;
    default:
        dbg_assert(0, "Unknown gethostbyname/gethostbyaddr pi error");
    }
    return HOST_NOT_FOUND;
}

//
// Convert platform independent getnameinfo flag to OS flags
//
int32_t pal_os_from_prx_client_getnameinfo_flags(
    int32_t flags,
    int *plat_flags
)
{
    if (!plat_flags)
        return er_fault;
    *plat_flags = 0;
    if (flags & ~(prx_ni_flag_namereqd))
        return er_arg;

    if (prx_ni_flag_namereqd == (flags & prx_ni_flag_namereqd))
        *plat_flags |= NI_NAMEREQD;

    return er_ok;
}

//
// Convert OS getnameinfo flag to platform independent flags
//
int32_t pal_os_to_prx_client_getnameinfo_flags(
    int flags,
    int32_t* prx_flags
)
{
    if (!prx_flags)
        return er_fault;
    *prx_flags = 0;

    if (NI_NAMEREQD == (flags & NI_NAMEREQD))
        *prx_flags |= prx_ni_flag_namereqd;

    if (flags & ~(NI_NAMEREQD))
        return er_not_supported;

    return er_ok;
}

//
// Convert a platform independent socket address to OS representation
//
int32_t pal_os_from_prx_socket_address(
    const prx_socket_address_t *prx_address,
    struct sockaddr* sa,
    socklen_t* sa_len
)
{
    struct sockaddr_in6* sa_in6;
    struct sockaddr_in* sa_in4;

    if (!sa || !sa_len || !prx_address)
        return er_fault;

    switch (prx_address->un.family)
    {
    case prx_address_family_inet:
        if (*sa_len < sizeof(struct sockaddr_in))
        {
            log_error(NULL, "Not enough buffer for ipv4 address!");
            return er_fault;
        }
        *sa_len = sizeof(struct sockaddr_in);
        sa_in4 = (struct sockaddr_in*)sa;
        memset(sa_in4, 0, *sa_len);
        sa_in4->sin_family = AF_INET;
        sa_in4->sin_port = swap_16(prx_address->un.ip.port);
        sa_in4->sin_addr.s_addr = prx_address->un.ip.un.in4.un.addr;
        return er_ok;
    case prx_address_family_inet6:
        if (*sa_len < sizeof(struct sockaddr_in6))
        {
            log_error(NULL, "Not enough buffer for ipv6 address!");
            return er_fault;
        }
        *sa_len = sizeof(struct sockaddr_in6);
        sa_in6 = (struct sockaddr_in6*)sa;
        memset(sa_in6, 0, *sa_len);
        sa_in6->sin6_family = AF_INET6;
        sa_in6->sin6_port = swap_16(prx_address->un.ip.port);
        sa_in6->sin6_flowinfo = prx_address->un.ip.flow;
        sa_in6->sin6_scope_id = prx_address->un.ip.un.in6.scope_id;
        memcpy(sa_in6->sin6_addr.s6_addr, prx_address->un.ip.un.in6.un.u8,
            sizeof(prx_address->un.ip.un.in6.un.u8));
        return er_ok;
    default:
        log_error(NULL, "Address family %d not yet supported. !", prx_address->un.family);
        break;
    }
    return er_not_supported;
}

//
// Convert a OS socket address to platform independent representation
//
int32_t pal_os_to_prx_socket_address(
    const struct sockaddr* sa,
    socklen_t sa_len,
    prx_socket_address_t *prx_address
)
{
    int result;
    struct sockaddr_in6* sa_in6;
    struct sockaddr_in* sa_in4;

    if (!sa || sa_len < sizeof(struct sockaddr) || !prx_address)
        return er_fault;

    result = pal_os_to_prx_address_family(sa->sa_family, &prx_address->un.family);
    if (result != er_ok)
        return result;

    switch (prx_address->un.family)
    {
    case prx_address_family_inet:
        if (sa_len < sizeof(struct sockaddr_in))
        {
            log_error(NULL, "Not enough buffer for ipv4 address!");
            return er_fault;
        }
        sa_in4 = (struct sockaddr_in*)sa;
        prx_address->un.ip.un.in4.un.addr = sa_in4->sin_addr.s_addr;
        prx_address->un.ip.port = swap_16(sa_in4->sin_port);
        prx_address->un.ip.flow = 0;
        return er_ok;
    case prx_address_family_inet6:
        if (sa_len < sizeof(struct sockaddr_in6))
        {
            log_error(NULL, "Not enough buffer for ipv6 address!");
            return er_fault;
        }
        sa_in6 = (struct sockaddr_in6*)sa;
        memcpy(prx_address->un.ip.un.in6.un.u8, sa_in6->sin6_addr.s6_addr,
            sizeof(prx_address->un.ip.un.in6.un.u8));
        prx_address->un.ip.un.in6.scope_id = sa_in6->sin6_scope_id;
        prx_address->un.ip.port = swap_16(sa_in6->sin6_port);
        prx_address->un.ip.flow = sa_in6->sin6_flowinfo;
        return er_ok;
    default:
        log_error(NULL, "Address family %d not yet supported. !", prx_address->un.family);
        break;
    }
    return er_not_supported;
}

//
// Convert platform independent message flags to os flags
//
int32_t pal_os_from_prx_message_flags(
    int32_t flags,
    int* plat_flags
)
{
    if (!plat_flags)
        return er_fault;

    *plat_flags = 0;
    if (flags & ~(prx_msg_flag_oob | prx_msg_flag_peek | 
        prx_msg_flag_dontroute | prx_msg_flag_trunc | prx_msg_flag_ctrunc))
        return er_arg;

    if (prx_msg_flag_oob == (flags & prx_msg_flag_oob))
        *plat_flags |= MSG_OOB;
    if (prx_msg_flag_peek == (flags & prx_msg_flag_peek))
        *plat_flags |= MSG_PEEK;
    if (prx_msg_flag_dontroute == (flags & prx_msg_flag_dontroute))
        *plat_flags |= MSG_DONTROUTE;
    if (prx_msg_flag_trunc == (flags & prx_msg_flag_trunc))
        *plat_flags |= MSG_TRUNC;
    if (prx_msg_flag_ctrunc == (flags & prx_msg_flag_ctrunc))
        *plat_flags |= MSG_CTRUNC;

    return er_ok;
}

//
// Convert os message flags to platform independent flags
//
int32_t pal_os_to_prx_message_flags(
    int flags,
    int32_t* prx_flags
)
{
    if (!prx_flags)
        return er_fault;

    if (flags & ~(MSG_OOB | MSG_PEEK | MSG_DONTROUTE | MSG_TRUNC | MSG_CTRUNC))
        return er_not_supported;

    *prx_flags = 0;

    if (MSG_OOB == (flags & MSG_OOB))
        *prx_flags |= prx_msg_flag_oob;
    if (MSG_PEEK == (flags & MSG_PEEK))
        *prx_flags |= prx_msg_flag_peek;
    if (MSG_DONTROUTE == (flags & MSG_DONTROUTE))
        *prx_flags |= prx_msg_flag_dontroute;
    if (MSG_TRUNC == (flags & MSG_TRUNC))
        *prx_flags |= prx_msg_flag_trunc;
    if (MSG_CTRUNC == (flags & MSG_CTRUNC))
        *prx_flags |= prx_msg_flag_ctrunc;

    return er_ok;
}

//
// Convert os socket option to platform independent option
//
int32_t pal_os_to_prx_socket_option(
    int opt_lvl,
    int opt_name,
    prx_socket_option_t* option
)
{
    if (!option)
        return er_fault;

    switch (opt_lvl)
    {
    case SOL_SOCKET:
        switch (opt_name)
        {
        case SO_DEBUG:             
            *option = prx_so_debug;               
            break;
        case SO_ACCEPTCONN:         
            *option = prx_so_acceptconn;       
            break;
        case SO_REUSEADDR:         
            *option = prx_so_reuseaddr;          
            break;
        case SO_KEEPALIVE:         
            *option = prx_so_keepalive;           
            break;
        case SO_DONTROUTE:         
            *option = prx_so_dontroute;           
            break;
        case SO_BROADCAST:         
            *option = prx_so_broadcast;           
            break;
        case SO_LINGER:            
            *option = prx_so_linger;             
            break;
        case SO_OOBINLINE:         
            *option = prx_so_oobinline;          
            break;
        case SO_SNDBUF:             
            *option = prx_so_sndbuf;              
            break;
        case SO_RCVBUF:       
            *option = prx_so_rcvbuf;             
            break;
        case SO_SNDLOWAT:      
            *option = prx_so_sndlowat;          
            break;
        case SO_RCVLOWAT:     
            *option = prx_so_rcvlowat;
            break;
        case SO_SNDTIMEO:     
            *option = prx_so_sndtimeo; 
            break;
        case SO_RCVTIMEO:    
            *option = prx_so_rcvtimeo;
            break;
        case SO_ERROR:       
            *option = prx_so_error; 
            break;
        case SO_TYPE:        
            *option = prx_so_type;   
            break;
        default:
            return er_not_supported;
        }
        break;
    case IPPROTO_IP:
        switch (opt_name)
        {
        case IP_OPTIONS:         
            *option = prx_so_ip_options; 
            break;
        case IP_HDRINCL:        
            *option = prx_so_ip_hdrincl;
            break;
        case IP_TOS:            
            *option = prx_so_ip_tos;  
            break;
        case IP_TTL:            
            *option = prx_so_ip_ttl;  
            break;
        case IP_MULTICAST_TTL:    
            *option = prx_so_ip_multicast_ttl;
            break;
        case IP_MULTICAST_LOOP:    
            *option = prx_so_ip_multicast_loop;  
            break;
        case IP_PKTINFO:         
            *option = prx_so_ip_pktinfo;  
            break;

        default:
            return er_not_supported;
        }
        break;
    case IPPROTO_IPV6:
        switch (opt_name)
        {
        case IPV6_HOPLIMIT:   
            *option = prx_so_ipv6_hoplimit;
            break;
        case IPV6_V6ONLY:      
            *option = prx_so_ipv6_v6only; 
            break;
        default:
            return er_not_supported;
        }
        break;
    case IPPROTO_TCP:
        switch (opt_name)
        {
        case TCP_NODELAY:      
            *option = prx_so_tcp_nodelay;  
            break;
        default:
            return er_not_supported;
        }
        break;
    case IPPROTO_UDP:
    default:
        return er_not_supported;
    }
    return er_ok;
}

//
// Convert platform independent option to os socket option 
//
int32_t pal_os_from_prx_socket_option(
    prx_socket_option_t socket_option,
    int* opt_lvl,
    int* opt_name
)
{
    int result;
    if (!opt_lvl || !opt_name)
        return er_fault;

    result = er_ok;
    switch (socket_option)
    {
    case prx_so_debug:              
        *opt_name = SO_DEBUG;                 
        break;
    case prx_so_acceptconn:          
        *opt_name = SO_ACCEPTCONN;           
        break;
    case prx_so_reuseaddr:           
        *opt_name = SO_REUSEADDR;           
        break;
    case prx_so_keepalive:       
        *opt_name = SO_KEEPALIVE;          
        break;
    case prx_so_dontroute:       
        *opt_name = SO_DONTROUTE;         
        break;
    case prx_so_broadcast:          
        *opt_name = SO_BROADCAST;            
        break;
    case prx_so_linger:          
        *opt_name = SO_LINGER;            
        break;
    case prx_so_oobinline:       
        *opt_name = SO_OOBINLINE;          
        break;
    case prx_so_sndbuf:            
        *opt_name = SO_SNDBUF;           
        break;
    case prx_so_rcvbuf:           
        *opt_name = SO_RCVBUF;           
        break;
    case prx_so_sndlowat:         
        *opt_name = SO_SNDLOWAT;        
        break;
    case prx_so_rcvlowat:        
        *opt_name = SO_RCVLOWAT;        
        break;
    case prx_so_sndtimeo:          
        *opt_name = SO_SNDTIMEO;        
        break;
    case prx_so_rcvtimeo:          
        *opt_name = SO_RCVTIMEO;   
        break;
    case prx_so_error:           
        *opt_name = SO_ERROR;    
        break;
    case prx_so_type:                
        *opt_name = SO_TYPE;     
        break;
    default:
        result = er_not_supported;
        break;
    }
    if (result == er_ok)
    {
        *opt_lvl = SOL_SOCKET;
        return result;
    }

    result = er_ok;
    switch (socket_option)
    {
    case prx_so_ip_options:          
        *opt_name = IP_OPTIONS;                
        break;
    case prx_so_ip_hdrincl:          
        *opt_name = IP_HDRINCL;                
        break;
    case prx_so_ip_tos:            
        *opt_name = IP_TOS;                   
        break;
    case prx_so_ip_ttl:             
        *opt_name = IP_TTL;                   
        break;
    case prx_so_ip_multicast_ttl:    
        *opt_name = IP_MULTICAST_TTL;          
        break;
    case prx_so_ip_multicast_loop:  
        *opt_name = IP_MULTICAST_LOOP;         
        break;
    case prx_so_ip_pktinfo:          
        *opt_name = IP_PKTINFO;              
        break;
    default:
        result = er_not_supported;
        break;
    }

    if (result == er_ok)
    {
        *opt_lvl = IPPROTO_IP;
        return result;
    }

    result = er_ok;
    switch (socket_option)
    {
    case prx_so_ipv6_hoplimit:  
        *opt_name = IPV6_HOPLIMIT;            
        break;
    case prx_so_ipv6_v6only:     
        *opt_name = IPV6_V6ONLY;            
        break;
    default:
        result = er_not_supported;
        break;
    }

    if (result == er_ok)
    {
        *opt_lvl = IPPROTO_IPV6;
        return result;
    }

    result = er_ok;
    switch (socket_option)
    {
    case prx_so_tcp_nodelay:        
        *opt_name = TCP_NODELAY;               
        break;
    default:
        result = er_not_supported;
        break;
    }

    if (result == er_ok)
    {
        *opt_lvl = IPPROTO_TCP;
        return result;
    }

    return result;
}

//
// Convert platform independent shutdown op from OS option
//
int32_t pal_os_to_prx_shutdown_op(
    int platform_shutdown,
    prx_shutdown_op_t* prx_shutdown
)
{
    if (!prx_shutdown)
        return er_fault;
    switch (platform_shutdown)
    {
    case SHUT_RD:         
        *prx_shutdown = prx_shutdown_op_read;   
        break;
    case SHUT_WR:           
        *prx_shutdown = prx_shutdown_op_write;   
        break;
    case SHUT_RDWR:        
        *prx_shutdown = prx_shutdown_op_both;     
        break;
    default:
        return er_arg;
    }
    return er_ok;
}

//
// Convert OS shutdown op to platform independent option
//
int32_t pal_os_from_prx_shutdown_op(
    prx_shutdown_op_t prx_shutdown,
    int* platform_shutdown
)
{
    if (!platform_shutdown)
        return er_fault;
    switch (prx_shutdown)
    {
    case prx_shutdown_op_read:  
        *platform_shutdown = SHUT_RD;          
        break;
    case prx_shutdown_op_write: 
        *platform_shutdown = SHUT_WR;         
        break;
    case prx_shutdown_op_both:  
        *platform_shutdown = SHUT_RDWR;      
        break;
    default:
        return er_arg;
    }
    return er_ok;
}

//
// Convert OS address family into platform independent
//
int32_t pal_os_to_prx_address_family(
    int platform_af,
    prx_address_family_t* prx_af
)
{
    if (!prx_af)
        return er_fault;
    switch (platform_af)
    {
    case AF_UNSPEC:   
        *prx_af = prx_address_family_unspec;     
        break;
    case AF_UNIX:       
        *prx_af = prx_address_family_unix;       
        break;
    case AF_INET:        
        *prx_af = prx_address_family_inet;      
        break;
    case AF_INET6:        
        *prx_af = prx_address_family_inet6;      
        break;
    default:
        return er_not_supported;
    }
    return er_ok;
}

//
// Convert platform independent address family into OS
//
int32_t pal_os_from_prx_address_family(
    prx_address_family_t prx_af,
    int* platform_af
)
{
    if (!platform_af)
        return er_fault;

    switch (prx_af)
    {
    case prx_address_family_unspec:    
        *platform_af = AF_UNSPEC;           
        break;
    case prx_address_family_unix:       
        *platform_af = AF_UNIX;             
        break;
    case prx_address_family_inet:       
        *platform_af = AF_INET;
        break;
    case prx_address_family_inet6:      
        *platform_af = AF_INET6;
        break;
    default:
        return er_arg;
    }
    return er_ok;
}

//
// Convert OS protocol type into platform independent
//
int32_t pal_os_to_prx_protocol_type(
    int platform_proto,
    prx_protocol_type_t* prx_proto
)
{
    if (!prx_proto)
        return er_fault;
    switch (platform_proto)
    {
    case IPPROTO_TCP:           
        *prx_proto = prx_protocol_type_tcp;
        break;
    case IPPROTO_UDP:           
        *prx_proto = prx_protocol_type_udp;
        break;
    case IPPROTO_ICMP:          
        *prx_proto = prx_protocol_type_icmp;
        break;
    case IPPROTO_ICMPV6:        
        *prx_proto = prx_protocol_type_icmpv6; 
        break;
    case 0:                     
        *prx_proto = prx_protocol_type_unspecified;
        break;
    default:
        return er_not_supported;
    }
    return er_ok;
}

//
// Convert platform independent protocol type into OS
//
int32_t pal_os_from_prx_protocol_type(
    prx_protocol_type_t prx_proto,
    int* platform_proto
)
{
    if (!platform_proto)
        return er_fault;
    switch (prx_proto)
    {
    case prx_protocol_type_tcp:         
        *platform_proto = IPPROTO_TCP; 
        break;
    case prx_protocol_type_udp:         
        *platform_proto = IPPROTO_UDP;    
        break;
    case prx_protocol_type_icmp:        
        *platform_proto = IPPROTO_ICMP;    
        break;
    case prx_protocol_type_icmpv6:      
        *platform_proto = IPPROTO_ICMPV6;   
        break;
    case prx_protocol_type_unspecified: 
        *platform_proto = 0;              
        break;
    default:                           
        *platform_proto = (int)prx_proto;
        return er_arg;
    }
    return er_ok;
}

//
// Convert OS socket type into platform independent
//
int32_t pal_os_to_prx_socket_type(
    int platform_socktype,
    prx_socket_type_t* prx_socktype
)
{
    if (!prx_socktype)
        return er_fault;

    switch (platform_socktype)
    {
    case SOCK_DGRAM:       
        *prx_socktype = prx_socket_type_dgram;
        break;
    case SOCK_STREAM:      
        *prx_socktype = prx_socket_type_stream; 
        break;
    case SOCK_RAW:         
        *prx_socktype = prx_socket_type_raw;    
        break;
    case SOCK_RDM:          
        *prx_socktype = prx_socket_type_rdm;      
        break;
    case SOCK_SEQPACKET:    
        *prx_socktype = prx_socket_type_seqpacket;   
        break;
    default:
        return er_not_supported;
    }
    return er_ok;
}

//
// Convert platform independent protocol type into OS
//
int32_t pal_os_from_prx_socket_type(
    prx_socket_type_t prx_socktype,
    int* platform_socktype
)
{
    if (!platform_socktype)
        return er_fault;
    switch (prx_socktype)
    {
    case prx_socket_type_dgram:
        *platform_socktype = SOCK_DGRAM;
        break;
    case prx_socket_type_stream:
        *platform_socktype = SOCK_STREAM;
        break;
    case prx_socket_type_raw:
        *platform_socktype = SOCK_RAW;
        break;
    case prx_socket_type_rdm:
        *platform_socktype = SOCK_RDM;
        break;
    case prx_socket_type_seqpacket:
        *platform_socktype = SOCK_SEQPACKET;
        break;
    default:
        return er_arg;
    }
    return er_ok;
}

//
// Convert OS flags to platform independent address info flags
//
int32_t pal_os_to_prx_client_getaddrinfo_flags(
    int flags,
    int32_t* prx_flags
)
{
    if (!prx_flags)
        return er_fault;

    *prx_flags = 0;

    if (AI_PASSIVE == (flags & AI_PASSIVE))
        *prx_flags |= prx_ai_passive;

    if (flags & ~(AI_CANONNAME | AI_PASSIVE))
        return er_not_supported;

    return er_ok;
}

//
// Convert platform independent address info flags to OS flags
//
int32_t pal_os_from_prx_client_getaddrinfo_flags(
    int32_t flags,
    int* platform_flags
)
{
    if (!platform_flags)
        return er_fault;

    *platform_flags = 0;

    if (flags & ~(prx_ai_passive))
        return er_arg;

    if (prx_ai_passive == (flags & prx_ai_passive))
        *platform_flags |= AI_PASSIVE;

    return er_ok;
}

//
// Return platform independent address info from os addrinfo
//
int32_t pal_os_to_prx_addrinfo(
    struct addrinfo* ai,
    prx_addrinfo_t* prx_ai
)
{
    int result;

    if (!ai || !prx_ai)
        return er_fault;

    result = pal_os_to_prx_socket_address(
        ai->ai_addr, (socklen_t)ai->ai_addrlen, &prx_ai->address);
    if (result != er_ok)
        return result;

    if (ai->ai_canonname)
        result = string_clone(ai->ai_canonname, &prx_ai->name);
    else
        prx_ai->name = NULL;
    return result;
}

//
// Return OS address info from platform independent addrinfo
//
int32_t pal_os_from_prx_addrinfo(
    prx_addrinfo_t* prx_ai,
    struct addrinfo* ai
)
{
    (void)prx_ai;
    (void)ai;
    return er_not_impl;
}

//
// Get host entry for address
//
int32_t pal_getaddrinfo(
    const char* address,
    const char* service,
    prx_address_family_t family,
    uint32_t flags,
    prx_addrinfo_t** prx_ai,
    prx_size_t* prx_ai_count
)
{
    struct addrinfo hint, *info, *ai = NULL;
    int32_t result;
    prx_size_t alloc_count = 0;

    if (!prx_ai || !prx_ai_count)
        return er_fault;

    *prx_ai_count = 0;
    *prx_ai = NULL;

    // Get all address families and the canonical name
    memset(&hint, 0, sizeof(hint));

    result = pal_os_from_prx_address_family(family, &hint.ai_family);
    if (result != er_ok)
        return result;

    result = pal_os_from_prx_client_getaddrinfo_flags(flags, &hint.ai_flags);
    if (result != er_ok)
        return result;

    hint.ai_flags |= AI_CANONNAME;

    result = getaddrinfo(address, service, &hint, &info);
    if (result != 0)
    {
#ifndef _WIN32
        log_error(NULL, "getaddrinfo returned error '%s' (%d)", 
            gai_strerror(result), result);
#endif
        return pal_os_to_prx_gai_error(result);
    }

    do
    {
        for (ai = info; ai != NULL; ai = ai->ai_next)
            alloc_count++;
        if (alloc_count == 0)
        {
            result = er_not_found;
            break;
        }

        // Alloc a flat buffer of addrinfo structures
        *prx_ai = (prx_addrinfo_t*)mem_zalloc(
            (alloc_count + 1) * sizeof(prx_addrinfo_t));
        if (!*prx_ai)
        {
            result = er_out_of_memory;
            break;
        }

        // Copy os addrinfo over
        for (ai = info; ai != NULL; ai = ai->ai_next)
        {
            prx_addrinfo_t* prx_ai_cur = &(*prx_ai)[*prx_ai_count];
            result = pal_os_to_prx_addrinfo(ai, prx_ai_cur);
            if (result == er_ok)
            {
                prx_ai_cur->reserved = 1;
                (*prx_ai_count)++;
            }
            else
            {
                log_info(NULL, "Warning: Failed to convert 1 addrinfo to prx_addrinfo_t (%s)",
                    prx_err_string(result));
            }
        }

        if (*prx_ai_count == 0)
        {
            log_error(NULL, "Error: %d addrinfo(s) available, but failed to convert any! (%s)",
                alloc_count, prx_err_string(result));

            mem_free(*prx_ai);
            *prx_ai = NULL;

            if (result != er_ok)
                break;
            result = er_not_found;
            break;
        }

        result = er_ok;
        break;
    } 
    while (0);

    freeaddrinfo(info);
    return result;
}

//
// Free host entry structure
//
int32_t pal_freeaddrinfo(
    prx_addrinfo_t* info
)
{
    if (!info)
        return er_fault;

    for (int32_t i = 0; info[i].reserved != 0; i++)
    {
        if (info[i].name)
            mem_free(info[i].name);
    }

    mem_free(info);
    return er_ok;
}

#define prx_ni_flag_numeric 0x10

//
// Get name info
//
int32_t pal_getnameinfo(
    prx_socket_address_t* address,
    char* host,
    prx_size_t host_length,
    char* service,
    prx_size_t service_length,
    int32_t flags
)
{
    int32_t result;
    int32_t plat_flags;
    uint8_t sa_in[MAX_SOCKET_ADDRESS_BYTES];
    socklen_t sa_len = sizeof(sa_in);

    if (!address || !host || !service || host_length <= 0 || service_length <= 0)
        return er_fault;

    result = pal_os_from_prx_socket_address(address, (struct sockaddr*)sa_in, &sa_len);
    if (result != er_ok)
        return result;

    if (flags == prx_ni_flag_numeric)
        plat_flags = (NI_NUMERICHOST | NI_NUMERICSERV);
    else
    {
        result = pal_os_from_prx_client_getnameinfo_flags(flags, &plat_flags);
        if (result != er_ok)
            return result;
    }
    result = getnameinfo((const struct sockaddr*)sa_in, (socklen_t)sa_len, host,
        (socklen_t)host_length, service, (socklen_t)service_length, plat_flags);
    if (result != 0)
        return pal_os_to_prx_gai_error(result);
    return er_ok;
}

//
// Convert an address string into an address
//
int32_t pal_pton(
    const char* addr_string,
    prx_socket_address_t* address
)
{
    struct addrinfo* info = NULL;
    struct addrinfo hint;
    int32_t result;
    if (!address || !addr_string)
        return er_fault;

    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_UNSPEC;
    hint.ai_flags = AI_NUMERICHOST;
    result = getaddrinfo(addr_string, NULL, &hint, &info);
    if (result != 0)
        return pal_os_to_prx_gai_error(result);
    result = pal_os_to_prx_socket_address(
        info->ai_addr, (socklen_t)info->ai_addrlen, address);
    freeaddrinfo(info);
    return result;
}

//
// Convert an address to a string
//
int32_t pal_ntop(
    prx_socket_address_t* address,
    char* addr_string,
    prx_size_t addr_string_size
)
{
    char svc[NI_MAXSERV];
    return pal_getnameinfo(
        address, addr_string, addr_string_size, svc, NI_MAXSERV, prx_ni_flag_numeric);
}
