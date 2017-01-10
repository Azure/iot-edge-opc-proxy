// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef _WIN32
#define __WSAFDIsSet __WSAFDIsSet_override
#define WINSOCK_API_LINKAGE __declspec(dllexport)
#endif

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "api_proxy.h"
#include "api_mocket.h"
#include "os.h"
#include "pal_errors.h"
#include "pal_types.h"
#include "util_misc.h"

#ifdef _WIN32
#define _std_call __stdcall
#else
#define _std_call 
#endif

//
// Use pal to set error, but avoid including entire pal
//
decl_public_1(void, pal_os_set_net_error_as_pi_error,
    int32_t, error
);

//
// Helper to set error code and return various results
//
#define _set_error_and_return(e) \
    { \
        pal_os_set_net_error_as_pi_error(e); \
        return e == er_ok ? 0 : -1; \
    } \

#define _set_error_and_return_null(e) \
    { \
        pal_os_set_net_error_as_pi_error(e); \
        return NULL; \
    } \

#define _set_error_and_return_invalid_fd(e) \
    { \
        pal_os_set_net_error_as_pi_error(e); \
        return _invalid_fd; \
    } \

#if 0

//
// Free the address info struct
//
void _std_call freeaddrinfo(
    struct addrinfo* ai
)
{
    struct addrinfo* ptr_to_free;
    while (ai)
    {
        ptr_to_free = ai;
        ai = ai->ai_next;

        free(ptr_to_free);
    }
}

//
// Returns an info struct for a socket and connect call
//
int _std_call getaddrinfo(
    const char* name,
    const char* svc,
    const struct addrinfo* hints,
    struct addrinfo** ai
)
{
    int result;
    pi_size_t pi_ai_count = 0;
    pi_addrinfo_t* pi_ai = NULL;
    pi_address_family_t family;
    struct addrinfo** ai_cur;
    int32_t flags = 0;

    result = pal_os_to_pi_address_family(hints->ai_family, &family);
    if (result != er_ok)
        _set_error_and_return(result);

    result = pal_os_to_pi_getaddrinfo_flags(hints->ai_flags, &flags);
    if (result != er_ok)
        _set_error_and_return(result);

    result = pi_getaddrinfo(name, svc, family, flags, &pi_ai, &pi_ai_count);
    if (result != er_ok)
        _set_error_and_return(result);

    ai_cur = ai;
    for (pi_size_t i = 0; i < pi_ai_count; i++)
    {
#define GAI_ADDR_LEN 32
        *ai_cur = malloc(sizeof(struct addrinfo) + GAI_ADDR_LEN);
        memset(*ai_cur, 0, sizeof(sizeof(struct addrinfo)));

        (*ai_cur)->ai_addr = (struct sockaddr*)&(*ai_cur)[1];
        (*ai_cur)->ai_addrlen = GAI_ADDR_LEN;
        (*ai_cur)->ai_protocol = hints->ai_protocol;
        (*ai_cur)->ai_socktype = hints->ai_socktype;

        result = pal_os_from_pi_socket_address(&pi_ai[i].address,
            (*ai_cur)->ai_addr, (socklen_t*)&(*ai_cur)->ai_addrlen);
        if (result != er_ok)
            break;

        result = pal_os_from_pi_address_family(
            pi_ai[i].address.un.family, &(*ai_cur)->ai_family);
        if (result != er_ok)
            break;

        ai_cur = &(*ai_cur)->ai_next;
    }
    *ai_cur = NULL;

    pi_freeaddrinfo(pi_ai);

    if (result != er_ok)
    {
        freeaddrinfo(*ai);
        *ai = NULL;
    }
    _set_error_and_return(result);
}

//
// Returns a 32 bit address from a ip string
//
saddr_t _std_call inet_addr(
    const char * cp
)
{
    saddr_t ip;
    if (0 != inet_pton(AF_INET, cp, &ip))
        return INADDR_NONE;
    return ip;
}

//
// Uses the fake ip table to get a string host name from ipv4 address
//
char* _std_call inet_ntoa(
    struct in_addr address
)
{
    static char buf[INET_ADDRSTRLEN];
    if (0 != inet_ntop(AF_INET, &address, buf, sizeof(buf)))
        return NULL;
    return buf;
}

//
// Convert an address string into an address structure
//
int _std_call inet_pton(
    int family,
    const char* addr_string,
    void* address
)
{
    int result;
    pi_socket_address_t sa;

    result = pal_os_to_pi_address_family(family, &sa.un.family);
    if (result != er_ok)
        _set_error_and_return(result);

    result = pi_pton(addr_string, &sa);
    if (result == er_ok)
    {
        if (sa.un.family == pi_address_family_inet && family == AF_INET)
            memcpy(address, sa.un.ip.un.in4.un.u8, sizeof(sa.un.ip.un.in4.un.u8));
        else if (sa.un.family == pi_address_family_inet6 && family == AF_INET6)
            memcpy(address, sa.un.ip.un.in6.un.u8, sizeof(sa.un.ip.un.in6.un.u8));
        else
            result = er_arg;
    }
    _set_error_and_return(result);
}

//
// Convert an address structure to a string
//
#ifdef _WIN32
typedef void _ntop_addr_t;
typedef size_t _ntop_socklen_t;
#else
typedef const void _ntop_addr_t;
typedef socklen_t _ntop_socklen_t;
#endif
const char* _std_call inet_ntop(
    int family,
    _ntop_addr_t* address,
    char* addr_string,
    _ntop_socklen_t addr_string_size
)
{
    int result;
    pi_socket_address_t sa;

    result = pal_os_to_pi_address_family(family, &sa.un.family);
    if (result != er_ok)
        _set_error_and_return_null(result);

    switch (sa.un.family)
    {
    case pi_address_family_inet:
        memcpy(sa.un.ip.un.in4.un.u8, address, sizeof(sa.un.ip.un.in4.un.u8));
        break;
    case pi_address_family_inet6:
        memcpy(sa.un.ip.un.in6.un.u8, address, sizeof(sa.un.ip.un.in6.un.u8));
        break;
    default:
        _set_error_and_return_null(er_not_supported);
    }
    result = pi_ntop(&sa, addr_string, (pi_size_t)addr_string_size);
    if (result != er_ok)
        _set_error_and_return_null(result);
    return addr_string;
}

//
// Return a fake hostent structure from an ipv4 address
//
struct hostent* _std_call gethostbyaddr(
    const sockbuf_t* address,
    socklen_t len,
    int family
)
{
    static struct {
        struct hostent h;
        char sa[32];
        char* lst[2];
        char name[MAX_HOST_LENGTH];
        char svc[MAX_PORT_LENGTH];
    } hent;

    int result;
    pi_socket_address_t sa;

    result = pal_os_to_pi_address_family(family, &sa.un.family);
    if (result != er_ok)
        _set_error_and_return_null(result);

    switch (sa.un.family)
    {
    case pi_address_family_inet:
        if (len < sizeof(sa.un.ip.un.in4.un.u8))
            _set_error_and_return_null(er_arg);
        memcpy(sa.un.ip.un.in4.un.u8, address, sizeof(sa.un.ip.un.in4.un.u8));
        memcpy(hent.sa, address, sizeof(sa.un.ip.un.in4.un.u8));
        break;
    case pi_address_family_inet6:
        if (len < sizeof(sa.un.ip.un.in6.un.u8))
            _set_error_and_return_null(er_arg);
        memcpy(sa.un.ip.un.in6.un.u8, address, sizeof(sa.un.ip.un.in6.un.u8));
        memcpy(hent.sa, address, sizeof(sa.un.ip.un.in6.un.u8));
        break;
    default:
        _set_error_and_return_null(er_not_supported);
    }

    memset(&hent, 0, sizeof(hent));

    result = pi_getnameinfo(&sa, hent.name, sizeof(hent.name),
        hent.svc, sizeof(hent.svc), pi_ni_flag_namereqd);
    if (result != er_ok)
        _set_error_and_return_null(result);

    hent.h.h_name = hent.name;
    hent.h.h_addrtype = (short)family;
    *hent.lst = hent.sa;
    hent.h.h_addr_list = hent.lst;
    return &hent.h;
}

//
// Return a fake hostent structure from a host name
//
struct hostent* _std_call gethostbyname(
    const char* name
)
{
    static struct {
        struct hostent h;
        char sa[32];
        socklen_t sa_len;
        char* lst[2];
    } hent;

    int result;
    pi_addrinfo_t* pi_ai = NULL;
    pi_size_t pi_ai_count = 0;
    int family;

    result = pi_getaddrinfo(
        name, NULL, pi_address_family_unspec, 0, &pi_ai, &pi_ai_count);
    if (result != er_ok || !pi_ai_count)
        _set_error_and_return_null(result);

    memset(&hent, 0, sizeof(hent));
    hent.h.h_name = NULL;
    result = pal_os_from_pi_address_family(pi_ai->address.un.family, &family);
    if (result == er_ok)
    {
        hent.h.h_addrtype = (short)family;
        result = pal_os_from_pi_socket_address(
            &pi_ai->address, (struct sockaddr*)hent.sa, &hent.sa_len);
        if (result != er_ok)
        {
            *hent.lst = hent.sa;
            hent.h.h_addr_list = hent.lst;

            pi_freeaddrinfo(pi_ai);
            pal_os_set_net_error_as_pi_error(er_ok);
            return &hent.h;
        }
    }

    pi_freeaddrinfo(pi_ai);
    _set_error_and_return_null(result);
}

//
// Returns a host name, which in our case is faked
//
int _std_call gethostname(
    char * name,
    socksize_t namelen
)
{
#ifdef __linux__
    struct utsname buf;
#endif
    if (!name || !namelen)
        _set_error_and_return(er_arg);

#ifdef _WIN32
    if (!GetComputerName(name, (LPDWORD)&namelen))
        _set_error_and_return(er_fault);
#else
    if (0 != uname(&buf))
        _set_error_and_return(er_fault);
    strncpy(name, buf.nodename, namelen);
#endif
    _set_error_and_return(er_ok);
}

//
// Returns the name of the socket peer 
//
int _std_call getpeername(
    fd_t s,
    struct sockaddr * name,
    socklen_t * namelen
)
{
    int result;
    pi_socket_address_t sa;
    result = pi_getpeername((pi_fd_t)s, &sa);
    if (result == er_ok)
        result = pal_os_from_pi_socket_address(&sa, name, namelen);
    _set_error_and_return(result);
}

//
// Returns host and service name strings from sock address
//
#ifdef _WIN32
typedef unsigned long __gnilen_t;
#else
typedef socklen_t __gnilen_t;
#endif
int _std_call getnameinfo(
    const struct sockaddr* address,
    socklen_t addr_len,
    char* buffer,
    __gnilen_t buf_size,
    char* svcbuffer,
    __gnilen_t svc_buf_size,
    int flags
)
{
    int result;
    int32_t pi_flags;
    pi_socket_address_t sa;

    result = pal_os_to_pi_socket_address(address, addr_len, &sa);
    if (result != er_ok)
        _set_error_and_return(result);

    result = pal_os_to_pi_getnameinfo_flags(flags, &pi_flags);
    if (result != er_ok)
        _set_error_and_return(result);

    result = pi_getnameinfo(&sa, buffer, buf_size, svcbuffer, svc_buf_size, pi_flags);
    _set_error_and_return(result);
}

//
// Service and protocol information not supported
//
struct servent * _std_call getservbyport(
    int port,
    const char * proto
)
{
	(void)port, proto;
    return NULL;
}

struct servent * _std_call getservbyname(
    const char * name,
    const char * proto
)
{
	(void)name, proto;
    return NULL;
}

struct protoent * _std_call getprotobynumber(
    int number
)
{
	(void)number;
    return NULL;
}

struct protoent * _std_call getprotobyname(
    const char * name
)
{
	(void)name;
    return NULL;
}


saddr_t _std_call htonl(
    saddr_t hostlong
)
{
    const saddr_t value = swap_32(hostlong);
    return value;
}

unsigned short _std_call htons(
    unsigned short hostshort
)
{
    const unsigned short value = swap_16(hostshort);
    return value;
}

saddr_t _std_call ntohl(
    saddr_t netlong
)
{
    const saddr_t value = swap_32(netlong);
    return value;
}

unsigned short _std_call ntohs(
    unsigned short netshort
)
{
    const unsigned short value = swap_16(netshort);
    return value;
}

#if 0 
uint64_t htonll(
    uint64_t Value
)
{
    const uint64_t Retval = __SWAP_LONGLONG(Value);
    return Retval;
}

uint64_t ntohll(
    uint64_t Value
)
{
    const uint64_t Retval = __SWAP_LONGLONG(Value);
    return Retval;
}

uint32_t htonf(
    float Value
)
{
    uint32_t Tempval;
    uint32_t Retval;
    Tempval = *(uint32_t*)(&Value);
    Retval = __SWAP_LONG(Tempval);
    return Retval;
}

float ntohf(
    uint32_t Value
)
{
    const uint32_t Tempval = __SWAP_LONG(Value);
    float Retval;
    *((uint32_t*)&Retval) = Tempval;
    return Retval;
}

uint64_t htond(
    double Value
)
{
    uint64_t Tempval;
    uint64_t Retval;
    Tempval = *(uint64_t*)(&Value);
    Retval = __SWAP_LONGLONG(Tempval);
    return Retval;
}

double ntohd(
    uint64_t Value
)
{
    const uint64_t Tempval = __SWAP_LONGLONG(Value);
    double Retval;
    *((uint64_t*)&Retval) = Tempval;
    return Retval;
}
#endif

#ifdef _WIN32
//
// FD_ISSET macro in winsock uses __WSAFDIsSet to test fd, simpl impl
//
#undef __WSAFDIsSet
__declspec(dllexport) int _std_call __WSAFDIsSet(
    SOCKET sock,
    fd_set* set
)
{
    for (unsigned int i = 0; i < set->fd_count; i++)
    {
        if (set->fd_array[i] == sock)
            return i + 1;
    }
    return 0;
}
#endif

//
// Creates a new socket object
//
fd_t _std_call socket(
    int family,
    int type,
    int protocol
)
{
    int result;
    pi_fd_t pi_fd;
    pi_address_family_t pi_af;
    pi_protocol_type_t pi_protocol;
    pi_socket_type_t pi_type;

    result = pal_os_to_pi_address_family(family, &pi_af);
    if (result != er_ok)
        _set_error_and_return_invalid_fd(result);

    result = pal_os_to_pi_protocol_type(protocol, &pi_protocol);
    if (result != er_ok)
		_set_error_and_return_invalid_fd(result);

    result = pal_os_to_pi_socket_type(type, &pi_type);
    if (result != er_ok)
		_set_error_and_return_invalid_fd(result);

    result = pi_socket(pi_af, pi_type, pi_protocol, &pi_fd);
    if (result != er_ok)
		_set_error_and_return_invalid_fd(result);

    // The id is our handle now
	pal_os_set_net_error_as_pi_error(er_ok);
    return (fd_t)pi_fd;
}

//
// Fake an ip out of the device io pointer
//
int _std_call getsockname(
    fd_t s,
    struct sockaddr * name,
    socklen_t * namelen
)
{
    int result;
    pi_socket_address_t sa;

    if (!s || !name || !namelen)
        _set_error_and_return(er_arg);

    result = pi_getsockname((pi_fd_t)s, &sa);
    if (result == er_ok)
        result = pal_os_from_pi_socket_address(&sa, name, namelen);
    _set_error_and_return(result);
}

//
// Connect synchronously
//
int _std_call connect(
    fd_t s,
    const struct sockaddr* name,
    socklen_t namelen
)
{
    int result;
    pi_socket_address_t sa;

    if (!s || !name || !namelen)
        _set_error_and_return(er_arg);

    result = pal_os_to_pi_socket_address(name, namelen, &sa);
    if (result == er_ok)
        result = pi_connect((pi_fd_t)s, &sa);
    _set_error_and_return(result);
}

//
// Binds the socket to a network interface address
//
int _std_call bind(
    fd_t s,
    const struct sockaddr * name,
    socklen_t namelen
)
{
    int result;
    pi_socket_address_t sa;

    if (!s || !name || !namelen)
        _set_error_and_return(er_arg);

    result = pal_os_to_pi_socket_address(name, namelen, &sa);
    if (result == er_ok)
        result = pi_bind((pi_fd_t)s, &sa);
    _set_error_and_return(result);
}

//
// Listen on bound socket
//
int _std_call listen(
    fd_t s,
    int backlog
)
{
    int result = pi_setsockopt((pi_fd_t)s, pi_so_acceptconn, backlog);
    _set_error_and_return(result);
}

//
// Get the next socket accepted on the passive socket
//
fd_t _std_call accept(
    fd_t s,
    struct sockaddr* name,
    socklen_t* namelen
)
{
    int result;
    pi_fd_t accepted = pi_invalid_socket;
    pi_socket_address_t sa;

    if (!s || !name || !namelen)
		_set_error_and_return_invalid_fd(er_arg);

    result = pi_accept((pi_fd_t)s, 0, &sa, &accepted);
    if (result != er_ok)
		_set_error_and_return_invalid_fd(result);

    result = pal_os_from_pi_socket_address(&sa, name, namelen);
    if (result != er_ok)
    {
        pi_close(accepted);
		_set_error_and_return_invalid_fd(result);
    }
	pal_os_set_net_error_as_pi_error(er_ok);
    return (fd_t)accepted;
}

//
// Select is similar to poll
//
#if _WIN32
typedef const struct timeval _select_timeval_t;
#else
typedef struct timeval _select_timeval_t;
#endif
int _std_call select(
    int nfds,
    fd_set * readfds,
    fd_set * writefds,
    fd_set * exceptfds,
    _select_timeval_t * timeout
)
{
    int result = 0;
    uint64_t timeout_ms;
    uint32_t i;
    uint32_t fd;

	(void)nfds;

    fd_set readfds_set;
    memset(&readfds_set, 0, sizeof(fd_set));
    fd_set writefds_set;
    memset(&writefds_set, 0, sizeof(fd_set));
    fd_set exceptfds_set;
    memset(&exceptfds_set, 0, sizeof(fd_set));

    timeout_ms = timeout ? timeout->tv_sec * 1000 + timeout->tv_usec / 1000 : -1;
    do
    {
#if _WIN32
#define select_fds(fds, cond) \
        for (i = 0; fds && i < fds->fd_count; i++) { \
            fd = (uint32_t)fds->fd_array[i]; \
            if ((fd_t)fd == -1 || !(cond)) continue; \
            result++; FD_SET((fd_t)fd, &##fds##_set); \
        }
#else
#define select_fds(fds, cond) 
#endif
        select_fds(readfds, pi_can_recv((pi_fd_t)fd));
        select_fds(writefds, pi_can_send((pi_fd_t)fd));
        select_fds(exceptfds, pi_has_error((pi_fd_t)fd));

        if (result || !timeout_ms)
        {
#if _WIN32
#define copy_fds(fds) \
            if (fds) { \
                memcpy(fds->fd_array, fds##_set.fd_array, \
                    fds->fd_count * sizeof(fds->fd_array[0])); \
                fds->fd_count = fds##_set.fd_count; \
            }
#else
#define copy_fds(fds) \

#endif

            copy_fds(readfds);
            copy_fds(writefds);
            copy_fds(exceptfds);

            return result;
        }

        pi_wait_activity(0, NULL, &timeout_ms);
        if (timeout_ms < 0)
            continue;

    } while (1);

    _set_error_and_return(er_bad_state);
}

//
// Poll for status on the poll array, similar to select
//
int _std_call poll(
    struct pollfd* fd_array,
    unsigned long fds,
    int timeout
)
{
    int result = 0;
    uint64_t timeout_ms = timeout;

    do
    {
        for (uint32_t i = 0; i < fds; i++)
        {
            uint32_t fd = (uint32_t)fd_array[i].fd;
            fd_array[i].revents = 0;
            if ((fd_t)fd == -1)
                continue;
            if (pi_is_disconnected((pi_fd_t)fd))
                fd_array[i].revents |= POLLHUP;
            if (pi_has_error((pi_fd_t)fd))
                fd_array[i].revents |= POLLERR;
            if (pi_can_recv((pi_fd_t)fd))
                fd_array[i].revents |= (fd_array[i].events & POLLRDNORM);
            if (pi_can_send((pi_fd_t)fd))
                fd_array[i].revents |= (fd_array[i].events & POLLWRNORM);
            if (fd_array[i].revents)
                result++;
        }

        if (result || !timeout_ms)
            return result;

        pi_wait_activity(0, NULL, &timeout_ms);
    } while (1);

    _set_error_and_return(er_bad_state);
}

//
// Send data to specified socket
//
sockssize_t _std_call sendto(
    fd_t s,
    const sockbuf_t * buf,
    socksize_t len,
    int flags,
    const struct sockaddr * name,
    socklen_t namelen
)
{
    int result;
    pi_size_t bytes_written;
    pi_socket_address_t sa;
    int32_t pi_flags;
    if (!s || !buf)
        _set_error_and_return(er_fault);

    result = pal_os_to_pi_socket_address(name, namelen, &sa);
    if (result != er_ok)
        _set_error_and_return(result);

    result = pal_os_to_pi_message_flags(flags, &pi_flags);
    if (result != er_ok)
        _set_error_and_return(result);

    result = pi_sendto(
        (pi_fd_t)s, 0, pi_flags, (uint8_t*)buf, 0, len, &sa, &bytes_written);
    if (result != er_ok)
        _set_error_and_return(result);

    return (sockssize_t)bytes_written;
}

//
// Send data on socket
//
sockssize_t _std_call send(
    fd_t s,
    const sockbuf_t *buf,
    socksize_t len,
    int flags
)
{
    int result;
    pi_size_t bytes_written;
    int32_t pi_flags;
    if (!s || !buf)
        _set_error_and_return(er_fault);

    result = pal_os_to_pi_message_flags(flags, &pi_flags);
    if (result != er_ok)
        _set_error_and_return(result);

    result = pi_send(
        (pi_fd_t)s, 0, pi_flags, (uint8_t*)buf, 0, len, &bytes_written);
    if (result != er_ok)
        _set_error_and_return(result);
    return (sockssize_t)bytes_written;
}

//
// Receive data on a bound socket
//
sockssize_t _std_call recvfrom(
    fd_t s,
    sockbuf_t* buf,
    socksize_t len,
    int flags,
    struct sockaddr * name,
    socklen_t * namelen
)
{
    int result;
    pi_size_t bytes_read;
    int32_t pi_flags;
    pi_socket_address_t sa;
    if (!s || !buf)
        _set_error_and_return(er_fault);

    result = pal_os_to_pi_message_flags(flags, &pi_flags);
    if (result != er_ok)
        _set_error_and_return(result);

    result = pi_recvfrom(
		(pi_fd_t)s, 0, pi_flags, (uint8_t*)buf, 0, len, &sa, &bytes_read);
    if (result != er_ok)
        _set_error_and_return(result);

    result = pal_os_from_pi_socket_address(&sa, name, namelen);
    if (result != er_ok)
        _set_error_and_return(result);

    return (sockssize_t)bytes_read;
}

//
// Receive data from socket
//
sockssize_t _std_call recv(
    fd_t s,
    sockbuf_t *buf,
    socksize_t len,
    int flags
)
{
    int result;
    pi_size_t bytes_read;
    int32_t pi_flags;

    if (!s || !buf)
        _set_error_and_return(er_fault);

    result = pal_os_to_pi_message_flags(flags, &pi_flags);
    if (result != er_ok)
        _set_error_and_return(result);

    result = pi_recv(
		(pi_fd_t)s, 0, pi_flags, (uint8_t*)buf, 0, len, &bytes_read);
    if (result == er_closed)
        return 0;
    if (result != er_ok)
        _set_error_and_return(result);

    return (sockssize_t)bytes_read;
}

//
// Shutdown sending and receiving
//
int _std_call shutdown(
    fd_t s,
    int how
)
{
    int result;
    pi_shutdown_op_t pi_how;
    if (!s)
        _set_error_and_return(er_fault);

    result = pal_os_to_pi_shutdown_op(how, &pi_how);
    if (result != er_ok)
        _set_error_and_return(result);

    result = pi_setsockopt((pi_fd_t)s, pi_so_shutdown, pi_how);
    _set_error_and_return(result);
}

//
// Close the socket
//
#if _WIN32
int _std_call closesocket(
#else
int _std_call close(
#endif
    fd_t s
)
{
    int result;
    if (!s)
        _set_error_and_return(er_arg);
    result = pi_close((pi_fd_t)s);
    _set_error_and_return(result);
}

//
// Handle non blocking setting only...
//
int _std_call ioctlsocket(
    fd_t s,
#if _WIN32
    long cmd,
    unsigned long * argp
#else
    unsigned long cmd,
    ...
#endif
)
{
    int result;
    uint64_t opt_val;
    if (!s)
        _set_error_and_return(er_arg);
    switch (cmd)
    {
#if _WIN32
    case FIONBIO:
        opt_val = *argp;
        result = pi_setsockopt((pi_fd_t)s, pi_so_nonblocking, opt_val);
        break;
    case FIONREAD:
        result = pi_getsockopt((pi_fd_t)s, pi_so_available, &opt_val);
        if (result == er_ok)
            *argp = (unsigned long)opt_val;
        break;
#endif
    default:
        result = er_not_impl;
        break;
    }
    _set_error_and_return(result);
}

//
// Returns socket options 
//
int _std_call getsockopt(
    fd_t s,
    int optlevel,
    int optname,
    sockbuf_t* optval,
    socklen_t* optlen
)
{
    int result = er_unknown;
    pi_socket_option_t option;
    uint64_t value;

    if (!s || !optlen || *optlen <= 0 || !optval)
        _set_error_and_return(er_fault);

    result = pal_os_to_pi_socket_option(optlevel, optname, &option);
    if (result != er_ok)
        _set_error_and_return(result);

    result = pi_getsockopt((pi_fd_t)s, option, &value);
    if (result != er_ok)
        _set_error_and_return(result);

    // Conversion
    /**/ if (option == pi_so_linger)
    {
        ((struct linger*)optval)->l_onoff = !!value;
        ((struct linger*)optval)->l_linger = (unsigned short)value;
        *optlen = sizeof(struct linger);
    }
    else if (option == pi_so_error)
    {
        *((int*)optval) = pal_os_from_pi_error((int32_t)value);
        *optlen = sizeof(int);
    }
    else
    {
        *((int*)optval) = (int)value;
        *optlen = sizeof(int);
    }
    _set_error_and_return(result);
}

//
// Set socket options for the unconnected socket
//
int _std_call setsockopt(
    fd_t s,
    int optlevel,
    int optname,
    const sockbuf_t* optval,
    socklen_t optlen
)
{
    int result = er_unknown;
    pi_socket_option_t option;
    uint64_t value;

    if (!s || optlen <= 0 || !optval)
        _set_error_and_return(er_fault);

    result = pal_os_to_pi_socket_option(optlevel, optname, &option);
    if (result != er_ok)
        _set_error_and_return(result);

    // Conversion
    /**/ if (option == pi_so_linger)
    {
        struct linger* opt = (struct linger*)optval;
        if (optlen < sizeof(struct linger))
            _set_error_and_return(er_fault);
        value = opt->l_onoff ? opt->l_linger : 0;
    }
    else
    {
        if (optlen < sizeof(int))
            _set_error_and_return(er_fault);
        value = *((int*)optval);
    }

    result = pi_setsockopt((pi_fd_t)s, option, value);
    _set_error_and_return(result);
}

#ifndef _WIN32

//
// Shared lib constructor
//
__attribute__((constructor)) void so_init(
    void
)
{
    pi_init(NULL);
}

//
// Hook to pass config in
//
_ext__ void so_init2(
    const char* config
)
{
    pi_deinit();
    pi_init(config);
}

//
// Shared lib destructor
//
__attribute__((destructor)) void so_fini(
    void
)
{
    pi_deinit();
}

#else
//
// Winsock startup stub, calls global init
//
int _std_call WSAStartup(
    WORD wVersionRequested,
    LPWSADATA lpWSAData
)
{
    int32_t result;
    //
    // Hook to pass config to us
    //
    if (wVersionRequested == 0xc5)
    {
        result = pi_init(lpWSAData->lpVendorInfo);
    }
    else
    {
        memset(lpWSAData, 0, sizeof(WSADATA));
        result = pi_init(NULL);
    }
    return pal_os_from_pi_error(result);
}

//
// Winsock cleanup stub, calls global deinit
//
int _std_call WSACleanup(
    void
)
{
    return pal_os_from_pi_error(pi_deinit());
}

//
// Winsock last error support
//
void _std_call WSASetLastError(
    int error
)
{
    SetLastError(error);
}

//
// Winsock last error support
//
int _std_call WSAGetLastError(
    void
)
{
    return GetLastError();
}

//
// Winsock specific api stubbed out for linking
//
int _std_call WSAConnect(
    SOCKET s,
    const struct sockaddr * name,
    int namelen,
    LPWSABUF lpCallerData,
    LPWSABUF lpCalleeData,
    LPQOS lpSQOS,
    LPQOS lpGQOS
)
{
	(void)s, name, namelen, lpCallerData, lpCalleeData;
	(void)lpSQOS, lpGQOS;
	return SOCKET_ERROR;
}

int _std_call WSAHtonl(
    SOCKET s,
    unsigned long hostlong,
    unsigned long * lpnetlong
)
{
	(void)s;
    *lpnetlong = htonl(hostlong);
    return 0;
}

int _std_call WSAHtons(
    SOCKET s,
    unsigned short hostshort,
    unsigned short * lpnetshort
)
{
	(void)s;
	*lpnetshort = htons(hostshort);
    return 0;
}

int _std_call WSACancelBlockingCall(
    void
)
{
    return SOCKET_ERROR;
}

HANDLE _std_call WSAAsyncGetServByName(
    HWND hWnd,
    unsigned int wMsg,
    const char * name,
    const char * proto,
    char * buf,
    int buflen
)
{
	(void)hWnd, wMsg, name, proto, buf, buflen;
	return NULL;
}

HANDLE _std_call WSAAsyncGetServByPort(
    HWND hWnd,
    unsigned int wMsg,
    int port,
    const char * proto,
    char * buf,
    int buflen
)
{
	(void)hWnd, wMsg, port, proto, buf, buflen;
	return NULL;
}

HANDLE _std_call WSAAsyncGetProtoByName(
    HWND hWnd,
    unsigned int wMsg,
    const char * name,
    char * buf,
    int buflen
)
{
	(void)hWnd, wMsg, name, buf, buflen;
	return NULL;
}

HANDLE _std_call WSAAsyncGetProtoByNumber(
    HWND hWnd,
    unsigned int wMsg,
    int number,
    char * buf,
    int buflen
)
{
	(void)hWnd, wMsg, number, buf, buflen;
	return NULL;
}

HANDLE _std_call WSAAsyncGetHostByName(
    HWND hWnd,
    unsigned int wMsg,
    const char * name,
    char * buf,
    int buflen
)
{
	(void)hWnd, wMsg, name, buf, buflen;
	return NULL;
}

HANDLE _std_call WSAAsyncGetHostByAddr(
    HWND hWnd,
    unsigned int wMsg,
    const char * address,
    int len,
    int type,
    char * buf,
    int buflen
)
{
	(void)hWnd, wMsg, address, len, type, buf, buflen;
    return NULL;
}

int _std_call WSACancelAsyncRequest(
    HANDLE hAsyncTaskHandle
)
{
	(void)hAsyncTaskHandle;
    return SOCKET_ERROR;
}

int _std_call WSAAsyncSelect(
    SOCKET s,
    HWND hWnd,
    unsigned int wMsg,
    long lEvent
)
{
	(void)s, hWnd, wMsg, lEvent;
    return SOCKET_ERROR;
}

SOCKET _std_call WSAAccept(
    SOCKET s,
    struct sockaddr * address,
    int* addrlen,
    LPCONDITIONPROC lpfnCondition,
    DWORD_PTR dwCallbackData
)
{
	(void)s, address, addrlen, lpfnCondition, dwCallbackData;
    return INVALID_SOCKET;
}

int _std_call WSACloseEvent(
    WSAEVENT hEvent
)
{
	return CloseHandle((HANDLE)hEvent);
}

int _std_call WSAConnectByNameA(
    SOCKET s,
    LPCSTR nodename,
    LPCSTR servicename,
    LPDWORD LocalAddressLength,
    LPSOCKADDR LocalAddress,
    LPDWORD RemoteAddressLength,
    LPSOCKADDR RemoteAddress,
    const struct timeval * timeout,
    LPWSAOVERLAPPED Reserved
)
{
	(void)s, nodename, servicename, LocalAddressLength, LocalAddress;
	(void)RemoteAddressLength, RemoteAddress, timeout, Reserved;
	return FALSE;
}

int _std_call WSAConnectByList(
    SOCKET s,
    PSOCKET_ADDRESS_LIST address,
    LPDWORD LocalAddressLength,
    LPSOCKADDR LocalAddress,
    LPDWORD RemoteAddressLength,
    LPSOCKADDR RemoteAddress,
    const struct timeval * timeout,
    LPWSAOVERLAPPED Reserved
)
{
	(void)s, address, LocalAddressLength, LocalAddress, RemoteAddressLength;
	(void)RemoteAddress, timeout, Reserved;
    return FALSE;
}

WSAEVENT _std_call WSACreateEvent(
    void
)
{
	return CreateEvent(NULL, FALSE, FALSE, NULL);
}

int _std_call WSADuplicateSocketA(
    SOCKET s,
    DWORD dwProcessId,
    LPWSAPROTOCOL_INFOA lpProtocolInfo
)
{
	(void)s, dwProcessId, lpProtocolInfo;
    return SOCKET_ERROR;
}

int _std_call WSAEnumNetworkEvents(
    SOCKET s,
    WSAEVENT hEventObject,
    LPWSANETWORKEVENTS lpNetworkEvents
)
{
	(void)s, hEventObject, lpNetworkEvents;
    return SOCKET_ERROR;
}

int _std_call WSAEnumProtocolsA(
    int* lpiProtocols,
    LPWSAPROTOCOL_INFOA lpProtocolBuffer,
    LPDWORD lpdwBufferLength
)
{
	(void)lpiProtocols, lpProtocolBuffer, lpdwBufferLength;
    return SOCKET_ERROR;
}

int _std_call WSAEventSelect(
    SOCKET s,
    WSAEVENT hEventObject,
    long lNetworkEvents
)
{
	(void)s, hEventObject, lNetworkEvents;
    return SOCKET_ERROR;
}

int _std_call WSAGetOverlappedResult(
    SOCKET s,
    LPWSAOVERLAPPED lpOverlapped,
    LPDWORD lpcbTransfer,
    int fWait,
    LPDWORD lpdwFlags
)
{
	(void)s, lpOverlapped, lpcbTransfer, fWait, lpdwFlags;
    return FALSE;
}

int _std_call WSAGetQOSByName(
    SOCKET s,
    LPWSABUF lpQOSName,
    LPQOS lpQOS
)
{
	(void)s, lpQOSName, lpQOS;
    return FALSE;
}

int _std_call WSAIoctl(
    SOCKET s,
    DWORD dwIoControlCode,
    LPVOID lpvInBuffer,
    DWORD cbInBuffer,
    LPVOID lpvOutBuffer,
    DWORD cbOutBuffer,
    LPDWORD lpcbBytesReturned,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
)
{
	(void)s, dwIoControlCode, lpvInBuffer, cbInBuffer, lpcbBytesReturned;
	(void)lpvOutBuffer, cbOutBuffer, lpOverlapped, lpCompletionRoutine;
    return SOCKET_ERROR;
}

SOCKET _std_call WSAJoinLeaf(
    SOCKET s,
    const struct sockaddr * name,
    int namelen,
    LPWSABUF lpCallerData,
    LPWSABUF lpCalleeData,
    LPQOS lpSQOS,
    LPQOS lpGQOS,
    DWORD dwFlags
)
{
	(void)s, name, namelen, lpCallerData, lpCalleeData;
	(void)lpSQOS, lpGQOS, dwFlags;
    return INVALID_SOCKET;
}

int _std_call WSARecv(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesRecvd,
    LPDWORD lpFlags,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
)
{
	(void)s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd;
	(void)lpOverlapped, lpCompletionRoutine, lpFlags;
	return SOCKET_ERROR;
}

int _std_call WSARecvDisconnect(
    SOCKET s,
    LPWSABUF lpInboundDisconnectData
)
{
	(void)s, lpInboundDisconnectData;
    return SOCKET_ERROR;
}

int _std_call WSARecvFrom(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesRecvd,
    LPDWORD lpFlags,
    struct sockaddr * lpFrom,
    int* lpFromlen,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
)
{
	(void)s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd;
	(void)lpOverlapped, lpCompletionRoutine, lpFlags, lpFrom, lpFromlen;
	return SOCKET_ERROR;
}

int _std_call WSARecvMsg(
    SOCKET s,
    LPWSAMSG lpMsg,
    LPDWORD lpdwNumberOfBytesRecvd,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
)
{
	(void)s, lpMsg, lpdwNumberOfBytesRecvd;
	(void)lpOverlapped, lpCompletionRoutine;
	return SOCKET_ERROR;
}

int _std_call WSAResetEvent(
    WSAEVENT hEvent
)
{
	return ResetEvent((HANDLE)hEvent);
}

int _std_call WSASend(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesSent,
    DWORD dwFlags,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
)
{
	(void)s, lpBuffers, dwBufferCount, lpNumberOfBytesSent;
	(void)lpOverlapped, lpCompletionRoutine, dwFlags;
	return SOCKET_ERROR;
}

int _std_call WSASendMsg(
    SOCKET s,
    LPWSAMSG lpMsg,
    DWORD dwFlags,
    LPDWORD lpNumberOfBytesSent,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
)
{
	(void)s, lpMsg, dwFlags, lpNumberOfBytesSent;
	(void)lpOverlapped, lpCompletionRoutine;
	return SOCKET_ERROR;
}

int _std_call WSASendDisconnect(
    SOCKET s,
    LPWSABUF lpOutboundDisconnectData
)
{
	(void)s, lpOutboundDisconnectData;
    return SOCKET_ERROR;
}

int _std_call WSASendTo(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesSent,
    DWORD dwFlags,
    const struct sockaddr * lpTo,
    int iTolen,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
)
{
	(void)s, lpBuffers, dwBufferCount, lpNumberOfBytesSent;
	(void)lpOverlapped, lpCompletionRoutine, dwFlags, lpTo, iTolen;
	return SOCKET_ERROR;
}

int _std_call WSASetEvent(
    WSAEVENT hEvent
)
{
    return SetEvent((HANDLE)hEvent);
}

SOCKET _std_call WSASocketA(
    int family,
    int type,
    int protocol,
    LPWSAPROTOCOL_INFOA lpProtocolInfo,
    GROUP g,
    DWORD dwFlags
)
{
	(void)lpProtocolInfo, g, dwFlags;
    return socket(family, type, protocol);
}

DWORD _std_call WSAWaitForMultipleEvents(
    DWORD cEvents,
    const WSAEVENT * lphEvents,
    int fWaitAll,
    DWORD dwTimeout,
    int fAlertable
)
{
    return WaitForMultipleObjectsEx(
		cEvents, (const HANDLE*)lphEvents, fWaitAll, dwTimeout, fAlertable);
}

int _std_call WSAAddressToStringA(
    LPSOCKADDR lpsaAddress,
    DWORD dwAddressLength,
    LPWSAPROTOCOL_INFOA lpProtocolInfo,
    LPSTR lpszAddressString,
    LPDWORD lpdwAddressStringLength
)
{
	(void)lpsaAddress, dwAddressLength, lpProtocolInfo;
	(void)lpszAddressString, lpdwAddressStringLength;
	return SOCKET_ERROR;
}

int _std_call WSAStringToAddressA(
    LPSTR AddressString,
    int AddressFamily,
    LPWSAPROTOCOL_INFOA lpProtocolInfo,
    LPSOCKADDR lpAddress,
    int* lpAddressLength
)
{
	(void)AddressString, AddressFamily, lpProtocolInfo;
	(void)lpAddress, lpAddressLength;
	return SOCKET_ERROR;
}

#endif
#endif