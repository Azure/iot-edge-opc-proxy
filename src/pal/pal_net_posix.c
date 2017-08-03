// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os.h"
#include "util_mem.h"
#include "pal_net.h"
#include "pal.h"
#include "pal_sd.h"
#include "pal_types.h"
#include "pal_err.h"

#include "util_misc.h"
#include "util_string.h"
#include "util_signal.h"

// #define RESOLVE_USING_MDNS_ONLY

//
// Logging helper tracing addresses that have been resolved...
//
_inl__ void pal_net_trace_resolved(
    const char* address,
    const char* service,
    prx_addrinfo_t* prx_ai
)
{
    (void)address;
    (void)service;

    switch (prx_ai->address.un.family)
    {
    case prx_address_family_inet:
        log_trace(NULL, "  ... %s:%s -> " __sa_in4_fmt " (%s)", address, service,
            prx_ai->address.un.ip.un.in4.un.u8[0], prx_ai->address.un.ip.un.in4.un.u8[1],
            prx_ai->address.un.ip.un.in4.un.u8[2], prx_ai->address.un.ip.un.in4.un.u8[3],
            prx_ai->address.un.ip.port, prx_ai->name ? prx_ai->name : "");
        break;
    case prx_address_family_inet6:
        log_trace(NULL, "  ... %s:%s -> " __sa_in6_fmt " (%s)", address, service,
            prx_ai->address.un.ip.un.in6.un.u16[0], prx_ai->address.un.ip.un.in6.un.u16[1],
            prx_ai->address.un.ip.un.in6.un.u16[2], prx_ai->address.un.ip.un.in6.un.u16[3],
            prx_ai->address.un.ip.un.in6.un.u16[4], prx_ai->address.un.ip.un.in6.un.u16[5],
            prx_ai->address.un.ip.un.in6.un.u16[6], prx_ai->address.un.ip.un.in6.un.u16[7],
            prx_ai->address.un.ip.port, prx_ai->name ? prx_ai->name : "");
        break;
    default:
        dbg_assert(0, "Unexpected family %d resolved", prx_ai->address.un.family);
    }
}

//
// Callback context
//
typedef struct pal_net_sd_ctx
{
    const char* address;
    const char* service;
    signal_t* signal;
    size_t result_len;
    prx_addrinfo_t* result;
    int32_t error;
}
pal_net_sd_ctx_t;

//
// Results callback
//
static int32_t pal_sd_getaddrinfo_cb(
    void *context,
    int32_t itf_index,
    int32_t error,
    pal_sd_result_type_t type,
    void *result,
    int32_t flags
)
{
    pal_net_sd_ctx_t* ctx = (pal_net_sd_ctx_t*)context;
    prx_addrinfo_t* ptr, *prx_ai = (prx_addrinfo_t*)result;

    (void)flags;
    (void)itf_index;

    if (error != er_ok)
    {
        log_trace(NULL, "Error %s in resolve cb ...", prx_err_string(error));
        ctx->error = error;
    }
    else if (type == pal_sd_result_addrinfo)
    {
        pal_net_trace_resolved(ctx->address, ctx->service, prx_ai);

        ptr = (prx_addrinfo_t*)mem_realloc(ctx->result, (ctx->result_len + 2) *
            sizeof(prx_addrinfo_t));
        if (!ptr)
            return er_out_of_memory;

        memcpy(&ptr[ctx->result_len], prx_ai, sizeof(prx_addrinfo_t));
        ptr[ctx->result_len].reserved = 0;
        ptr[ctx->result_len].name = NULL;

        ctx->error = er_ok;
        ctx->result = ptr;

        if (prx_ai->name)
        {
            error = string_clone(
                prx_ai->name, (char**)&ptr[ctx->result_len].name);
            if (error != er_ok)
                return error;
        }

        ctx->result_len++;
        ctx->result[ctx->result_len].reserved = 1;
    }
    else
    {
        dbg_assert(0, "Unexpected result type %d", type);
    }
    return er_ok;
}

//
// Resolve host name using sd but synchronously waiting for results for timeout ms.
//
static int32_t pal_sd_getaddrinfo(
    const char* address,
    const char* service,
    int32_t timeout,
    prx_addrinfo_t** ai_info,
    size_t* ai_size
)
{
    int32_t result;
    prx_socket_address_proxy_t host_addr;
    pal_net_sd_ctx_t ctx;
    pal_sdclient_t* client;
    pal_sdbrowser_t* browser = NULL;

    dbg_assert_ptr(address);
    dbg_assert_ptr(ai_info);
    dbg_assert_ptr(ai_size);

    result = pal_sdclient_create(NULL, NULL, &client);
    if (result != er_ok)
        return result;
    do
    {
        memset(&ctx, 0, sizeof(pal_net_sd_ctx_t));
        ctx.address = address;
        ctx.service = service;

        result = signal_create(true, false, &ctx.signal);
        if (result != er_ok)
            break;
        result = pal_sdbrowser_create(client, pal_sd_getaddrinfo_cb, &ctx,
            &browser);
        if (result != er_ok)
            break;

        memset(&host_addr, 0, sizeof(host_addr));
        host_addr.family = prx_address_family_proxy;
        if (service)
            host_addr.port = (uint16_t)atoi(service);
        host_addr.host_dyn = address;
        result = pal_sdbrowser_resolve(browser, &host_addr, prx_itf_index_all);
        if (result != er_ok)
        {
            log_error(NULL, "Warning: Failed to resolve on sd browser (%s)",
                prx_err_string(result));
            break;
        }

        // Wait for signal or timeout...
        result = signal_wait_ex(ctx.signal, &timeout);

        // Collect result
        if (!ctx.result)
        {
            if (ctx.error == er_ok)
                result = er_not_found;
            else
                result = ctx.error;
            break;
        }

        // Mark last element so free works correctly
        *ai_info = ctx.result;
        ctx.result = NULL;
        *ai_size = ctx.result_len;
        ctx.result_len = 0;
        result = er_ok;
        break;
    }
    while (0);
    if (browser)
        pal_sdbrowser_free(browser);
    pal_sdclient_free(client);
    if (ctx.result)
        pal_freeaddrinfo(ctx.result);
    if (ctx.signal)
        signal_free(ctx.signal);
    return result;
}

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
        else if (error == EAI_ADDRFAMILY)
            return er_address_family;
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
    chk_arg_fault_return(plat_flags);
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
    chk_arg_fault_return(prx_flags);
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
    struct sockaddr_un* sa_un;

    chk_arg_fault_return(sa);
    chk_arg_fault_return(sa_len);
    chk_arg_fault_return(prx_address);

    switch (prx_address->un.family)
    {
    case prx_address_family_inet:
        if (*sa_len < sizeof(struct sockaddr_in))
        {
            log_error(NULL, "Not enough buffer for ipv4 address struct!");
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
            log_error(NULL, "Not enough buffer for ipv6 address struct!");
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
    case prx_address_family_unix:
        sa_un = (struct sockaddr_un*)sa;
        if (*sa_len == sizeof(sa->sa_family) && !prx_address->un.ux.path[0])
        {
            sa_un->sun_family = AF_UNIX;
            return er_ok;
        }
        if (*sa_len < sizeof(struct sockaddr_un))
        {
            log_error(NULL, "Not enough buffer for unix address struct!");
            return er_fault;
        }
        *sa_len = sizeof(struct sockaddr_un);
        memset(sa_un, 0, *sa_len);
        sa_un->sun_family = AF_UNIX;
        if (strlen(prx_address->un.ux.path) >= sizeof(sa_un->sun_path))
        {
            log_error(NULL, "Not enough buffer to copy unix path!");
            return er_arg;
        }
        strcpy(sa_un->sun_path, prx_address->un.ux.path);
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
    struct sockaddr_un* sa_un;

    chk_arg_fault_return(sa);
    chk_arg_fault_return(prx_address);

    if (sa_len < sizeof(sa->sa_family))
        return er_fault;

    result = pal_os_to_prx_address_family(sa->sa_family, &prx_address->un.family);
    if (result != er_ok)
        return result;

    switch (prx_address->un.family)
    {
    case prx_address_family_inet:
        if (sa_len < sizeof(struct sockaddr_in))
        {
            log_error(NULL, "Ipv4 address is too small!");
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
            log_error(NULL, "Ipv6 address is too small!");
            return er_fault;
        }
        sa_in6 = (struct sockaddr_in6*)sa;
        memcpy(prx_address->un.ip.un.in6.un.u8, sa_in6->sin6_addr.s6_addr,
            sizeof(prx_address->un.ip.un.in6.un.u8));
        prx_address->un.ip.un.in6.scope_id = sa_in6->sin6_scope_id;
        prx_address->un.ip.port = swap_16(sa_in6->sin6_port);
        prx_address->un.ip.flow = sa_in6->sin6_flowinfo;
        return er_ok;
    case prx_address_family_unix:
        if (sa_len == sizeof(sa->sa_family))
        {
            // Special case for socket pair addresses
            prx_address->un.ux.path[0] = 0;
            return er_ok;
        }
        if (sa_len < sizeof(struct sockaddr_un))
        {
            log_error(NULL, "Unix address is too small");
            return er_fault;
        }
        sa_un = (struct sockaddr_un*)sa;
        if (strlen(sa_un->sun_path) >= sizeof(prx_address->un.ux.path))
        {
            log_error(NULL, "Unix address is too long");
            return er_arg;
        }
        strcpy(prx_address->un.ux.path, sa_un->sun_path);
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
    chk_arg_fault_return(plat_flags);

    *plat_flags = 0;
    if (flags & ~(prx_msg_flag_oob | prx_msg_flag_peek | prx_msg_flag_more |
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
    if (prx_msg_flag_more == (flags & prx_msg_flag_more))
        *plat_flags |= MSG_MORE;

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
    chk_arg_fault_return(prx_flags);

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
    if (MSG_MORE == (flags & MSG_MORE))
        *prx_flags |= prx_msg_flag_more;

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
    chk_arg_fault_return(option);

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
    chk_arg_fault_return(opt_lvl);
    chk_arg_fault_return(opt_name);

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
    chk_arg_fault_return(prx_shutdown);
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
    chk_arg_fault_return(platform_shutdown);
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
    chk_arg_fault_return(prx_af);
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
    chk_arg_fault_return(platform_af);
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
    chk_arg_fault_return(prx_proto);
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
    chk_arg_fault_return(platform_proto);
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
    chk_arg_fault_return(prx_socktype);
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
    chk_arg_fault_return(platform_socktype);
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
    chk_arg_fault_return(prx_flags);
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
    chk_arg_fault_return(platform_flags);
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

    chk_arg_fault_return(ai);
    chk_arg_fault_return(prx_ai);

    result = pal_os_to_prx_socket_address(
        ai->ai_addr, (socklen_t)ai->ai_addrlen, &prx_ai->address);
    if (result != er_ok)
        return result;

    if (ai->ai_canonname)
        result = string_clone(ai->ai_canonname, (char**)&prx_ai->name);
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
    chk_arg_fault_return(ai);
    chk_arg_fault_return(prx_ai);
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
    prx_addrinfo_t** prx_ai_result,
    size_t* prx_ai_result_count
)
{
    struct addrinfo hint, *info, *ai_next = NULL;
    int32_t result, error;
    int attempt = 0;
    prx_addrinfo_t *prx_ai = NULL;
    size_t alloc_count, prx_ai_count = 0;

    chk_arg_fault_return(prx_ai_result_count);
    chk_arg_fault_return(prx_ai_result);

    log_info(NULL, "Resolving %s:%s (family: %d)...", address, service,
        family);

    // Get all address families and the canonical name
    memset(&hint, 0, sizeof(hint));

    result = pal_os_from_prx_address_family(family, &hint.ai_family);
    if (result != er_ok)
        return result;

    result = pal_os_from_prx_client_getaddrinfo_flags(flags, &hint.ai_flags);
    if (result != er_ok)
        return result;

    hint.ai_flags |= AI_CANONNAME;
    while (true)
    {
        info = NULL;
#if defined(RESOLVE_USING_MDNS_ONLY)
        error = 0;
#else
        error = getaddrinfo(address, service, &hint, &info);
#endif
        if (error == 0)
        {
            result = er_ok;
            break;
        }

        // Workaround for issue #32 when host wants AF_INET6
        if (error == EAI_ADDRFAMILY || error == EAI_FAMILY)
        {
            if (hint.ai_family == AF_INET)
            {
                hint.ai_family = AF_INET6;
                continue; // try again with IPv6, if IPv4 has failed
            }
        }

        // Intermittent dns outages can result in E_AGAIN, try 3 times
#define GAI_MAX_ATTEMPTS 3
        if (error != EAI_AGAIN || attempt++ >= GAI_MAX_ATTEMPTS)
        {
            log_error(NULL, "getaddrinfo returned error '%s' (%d)",
                gai_strerror(error), error);
            result = pal_os_to_prx_gai_error(error);
            break;
        }

        log_info(NULL, "getaddrinfo returned error '%s' (%d) - try again...",
            gai_strerror(error), error);

        if (!info)
            continue;
        freeaddrinfo(info);
    }
    do
    {
        if (result != er_ok)
            break;

        alloc_count = 0;
        for (ai_next = info; ai_next != NULL; ai_next = ai_next->ai_next)
            alloc_count++;
        if (alloc_count == 0)
        {
            result = er_not_found;
            break;
        }

        // Alloc a flat buffer of addrinfo structures
        prx_ai = (prx_addrinfo_t*)mem_zalloc(
            (alloc_count + 1) * sizeof(prx_addrinfo_t));
        if (!prx_ai)
        {
            result = er_out_of_memory;
            break;
        }

        // Copy os addrinfo over post existing set of address infos if any
        for (ai_next = info; ai_next != NULL; ai_next = ai_next->ai_next)
        {
            result = pal_os_to_prx_addrinfo(ai_next, &prx_ai[prx_ai_count]);
            if (result != er_ok)
            {
                log_error(NULL, "Warning: Failed to convert addrinfo "
                    "to prx_addrinfo_t (%s)", prx_err_string(result));
                continue;
            }
            pal_net_trace_resolved(address, service, &prx_ai[prx_ai_count]);
            prx_ai_count++;
        }

        if (prx_ai_count == 0)
        {
            log_error(NULL, "Error: %d addrinfo(s) available, but failed "
                "to convert any! (%s)", alloc_count, prx_err_string(result));
            mem_free(prx_ai);
            prx_ai = NULL;
            if (result == er_ok)
                result = er_not_found;
            break;
        }

        // Mark last item so that pal_freeaddrinfo can free names.
        prx_ai[prx_ai_count].reserved = 1;
        *prx_ai_result = prx_ai;
        *prx_ai_result_count = prx_ai_count;
        result = er_ok;
        break;
    }
    while (0);

    if (info)
        freeaddrinfo(info);

    //
    // We do getaddrinfo first since doing mdns lookup is more expansive due to the
    // allocation and closing of sd clients. Todo: consider to cache a client instance
    // and re-create it only in case of errors.
    //
    if (result != er_ok && address &&
        0 == (flags & prx_ai_passive) && (pal_caps() & pal_cap_dnssd))
        return pal_sd_getaddrinfo(address, service, 100, prx_ai_result, prx_ai_result_count);
    else
        return result;
}

//
// Free host entry structure
//
int32_t pal_freeaddrinfo(
    prx_addrinfo_t* info
)
{
    chk_arg_fault_return(info);

    for (int32_t i = 0; !info[i].reserved; i++)
    {
        if (info[i].name)
            mem_free((char*)info[i].name);
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
    size_t host_length,
    char* service,
    size_t service_length,
    int32_t flags
)
{
    int32_t result;
    int32_t plat_flags = 0;
#define MAX_SOCKET_ADDRESS_BYTES 127
    uint8_t sa_in[MAX_SOCKET_ADDRESS_BYTES];
    socklen_t sa_len = sizeof(sa_in);

    chk_arg_fault_return(address);
    chk_arg_fault_return(host);
    if (!host_length)
        return er_arg;
    if ((service && !service_length) || (service_length && !service))
        return er_arg;

    result = pal_os_from_prx_socket_address(address, (struct sockaddr*)sa_in, &sa_len);
    if (result != er_ok)
        return result;

    if (flags == prx_ni_flag_numeric)
    {
        plat_flags |= NI_NUMERICHOST;
        if (service)
            plat_flags |= NI_NUMERICSERV;
    }
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
    chk_arg_fault_return(address);
    chk_arg_fault_return(addr_string);

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
    size_t addr_string_size
)
{
    char svc[NI_MAXSERV];
    return pal_getnameinfo(
        address, addr_string, addr_string_size, svc, NI_MAXSERV, prx_ni_flag_numeric);
}
