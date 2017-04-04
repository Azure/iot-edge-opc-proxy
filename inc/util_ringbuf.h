// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _util_ringbuf_h
#define _util_ringbuf_h

#include "common.h"

typedef struct ring_buffer ring_buffer_t;

// 
// Creates a ring buffer of a certain size
//
decl_internal_2(int32_t, ring_buffer_create,
    size_t, size,
    ring_buffer_t**, rb
);

//
// Tests whether the buffer is empty
//
decl_internal_1(bool, ring_buffer_is_empty,
    ring_buffer_t*, rb
);

//
// Clears the ring buffer
//
decl_internal_1(void, ring_buffer_clear,
    ring_buffer_t*, rb
);

//
// How many bytes are guaranteed to be free to write to?
//
decl_internal_1(size_t, ring_buffer_capacity,
    ring_buffer_t*, rb
);

//
// Write len bytes of data to a ring buffer rb
//
decl_internal_3(size_t, ring_buffer_write,
    ring_buffer_t*, rb, 
    const uint8_t*, b, 
    size_t, len
);

//
// How many bytes are guaranteed to be available to read?
//
decl_internal_1(size_t, ring_buffer_available,
    ring_buffer_t*, rb
);

//
// Read len bytes of data from a ring buffer rb
//
decl_internal_3(size_t, ring_buffer_read,
    ring_buffer_t*, rb, 
    uint8_t*, b, 
    size_t, len
);

// 
// This function frees the ring buffer memory
// 
decl_internal_1(void, ring_buffer_free,
    ring_buffer_t*, rb
);


#endif // _util_ringbuf_h
