// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _os_win_h_
#define _os_win_h_

#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

//
// Windows OS layer api, used by mostly pal, and c client
//
#define WIN32_LEAN_AND_MEAN
#define NONAMELESSUNION 1

#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS

#include <WinSock2.h>
#include <MSWSock.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#include <Wincrypt.h>
#include <winternl.h>
#include <ntstatus.h>

//
// OS type adapters
//
typedef SOCKET fd_t;
#define _invalid_fd INVALID_SOCKET
#define poll WSAPoll
typedef int socklen_t;
typedef int socksize_t;
typedef int sockssize_t;
typedef char sockbuf_t;
typedef unsigned long saddr_t;
typedef IP_ADAPTER_ADDRESSES ifinfo_t;
typedef IP_ADAPTER_UNICAST_ADDRESS ifaddr_t;
typedef long long ticks_t;


#define SHUT_RD SD_RECEIVE
#define SHUT_WR SD_SEND
#define SHUT_RDWR SD_BOTH

struct sockaddr_un 
{
    ADDRESS_FAMILY sun_family; 
    CHAR sun_path[108]; 
};

// Implemented in pal_sk_win
int socketpair(int, int, int, fd_t sv[2]);

__inline int _fd_nonblock(fd_t fd, int r)
{
    u_long on = 1;
    (void)r;
    return ioctlsocket(fd, FIONBIO, &on);
}

#if !defined(EAI_NODATA)
#define EAI_NODATA WSANO_DATA
#endif

#if !defined(EAI_ADDRFAMILY)
#define EAI_ADDRFAMILY WSANO_DATA
#endif

#endif // _os_win_h_