// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _util_misc_h_
#define _util_misc_h_

#include "common.h"

#define __str(n) #n

#define MAX_SHORT_VALUE 32767

//
// Swap and min/max utility macros
//
#define swap_16(s)  \
    ( ( ((s) >>  8) & 0x00FFL ) | ( ((s) <<  8) & 0xFF00L ) ) \

#define swap_32(l)  \
    ( ( ((l) >> 24) & 0x000000FFL ) | ( ((l) >>  8) & 0x0000FF00L ) | \
      ( ((l) <<  8) & 0x00FF0000L ) | ( ((l) << 24) & 0xFF000000L ) )

#define swap_64(l)  \
    ( ( ((l) >> 56) & 0x00000000000000FFLL ) | ( ((l) >> 40) & 0x000000000000FF00LL ) | \
      ( ((l) >> 24) & 0x0000000000FF0000LL ) | ( ((l) >>  8) & 0x00000000FF000000LL ) | \
      ( ((l) <<  8) & 0x000000FF00000000LL ) | ( ((l) << 24) & 0x0000FF0000000000LL ) | \
      ( ((l) << 40) & 0x00FF000000000000LL ) | ( ((l) << 56) & 0xFF00000000000000LL ) )


#if !defined(max)
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#if !defined(min)
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#if !defined(_MSC_VER) || !defined(_countof)
#define _countof(a) (sizeof(a) / sizeof((a)[0]))
#endif

//
// Count leading ones
//
decl_internal_1(size_t, count_leading_ones,
    uint8_t, b
);

//
// Count leading ones in a contiguous buffer
//
decl_internal_2(size_t, count_leading_ones_in_buf,
    const uint8_t*, buf,
    size_t, len
);

//
// Calculate checksum
//
decl_internal_2(uint32_t, crc32,
    const void*, buf,
    size_t, len
);


//
// Posix socket address utilities
//

#define __sa_base(sa) \
    ((struct sockaddr*)(sa))
#define __sa_size(sa) \
    __sa_is_in4(sa) ? sizeof(struct sockaddr_in) :  \
    (__sa_is_in6(sa) ? sizeof(struct sockaddr_in6) : 0) \

#define __sa_as_in4(sa) \
    ((struct sockaddr_in*)(sa))
#define __sa_is_in4(sa) \
    (__sa_base(sa)->sa_family == AF_INET)
#define __sa_as_in6(sa) \
    ((struct sockaddr_in6*)(sa))
#define __sa_is_in6(sa) \
    (__sa_base(sa)->sa_family == AF_INET6)

#define __sa_in6_fmt \
    "[%x:%x:%x:%x:%x:%x:%x:%x]:%d"
#define __sa_in4_fmt \
    "%d.%d.%d.%d:%d"

#define __sa_in6_args(sa) \
    ((uint16_t*)&(__sa_as_in6(sa)->sin6_addr.s6_addr))[0], \
    ((uint16_t*)&(__sa_as_in6(sa)->sin6_addr.s6_addr))[1], \
    ((uint16_t*)&(__sa_as_in6(sa)->sin6_addr.s6_addr))[2], \
    ((uint16_t*)&(__sa_as_in6(sa)->sin6_addr.s6_addr))[3], \
    ((uint16_t*)&(__sa_as_in6(sa)->sin6_addr.s6_addr))[4], \
    ((uint16_t*)&(__sa_as_in6(sa)->sin6_addr.s6_addr))[5], \
    ((uint16_t*)&(__sa_as_in6(sa)->sin6_addr.s6_addr))[6], \
    ((uint16_t*)&(__sa_as_in6(sa)->sin6_addr.s6_addr))[7], \
    swap_16(__sa_as_in6(sa)->sin6_port)

#define __sa_in4_args(sa) \
    ((uint8_t*)&(__sa_as_in4(sa)->sin_addr.s_addr))[0], \
    ((uint8_t*)&(__sa_as_in4(sa)->sin_addr.s_addr))[1], \
    ((uint8_t*)&(__sa_as_in4(sa)->sin_addr.s_addr))[2], \
    ((uint8_t*)&(__sa_as_in4(sa)->sin_addr.s_addr))[3], \
    swap_16(__sa_as_in4(sa)->sin_port)


#endif // _util_misc_h_