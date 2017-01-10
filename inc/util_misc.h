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

#endif // _util_misc_h_