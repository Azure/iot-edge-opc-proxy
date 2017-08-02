// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _os_posix_h_
#define _os_posix_h_

//
// OS apis, used mostly by pal implementation
//

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/utsname.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <poll.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <err.h>

//
// OS type adapters
//
typedef int fd_t;
#define _invalid_fd -1

typedef ssize_t sockssize_t;
typedef size_t socksize_t;
typedef void sockbuf_t;
typedef uint32_t saddr_t;
typedef struct ifaddrs ifinfo_t;
typedef struct sockaddr ifaddr_t;
typedef long long ticks_t;

#if !defined(EAI_FAIL)
#define EAI_FAIL -1
#endif

#if !defined(EAI_ADDRFAMILY)
#define EAI_ADDRFAMILY EAI_NODATA
#endif

#include <fcntl.h>

static inline int _fd_nonblock(fd_t fd)
{
    int r = fcntl(fd, F_GETFL, 0);
    if (r != -1) {
        r |= O_NONBLOCK;
        r = fcntl(fd, F_SETFL, r);
    }
}

#define closesocket close

#endif // _os_posix_h_