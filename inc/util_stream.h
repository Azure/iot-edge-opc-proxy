// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _util_stream_h_
#define _util_stream_h_

#include "common.h"
#include "io_stream.h"
#include "prx_buffer.h"

//
// Stream with preallocated input and output buffers
//
typedef struct io_fixed_buffer_stream
{
    io_stream_t itf;                 // codec stream interface
    const uint8_t* in;
    size_t in_len;
    uint8_t* out;
    size_t out_len;
}
io_fixed_buffer_stream_t;

//
// Stream for pool allocated dynamic output buffers
//
typedef struct io_dynamic_buffer_stream
{
    io_stream_t itf;                 // codec stream interface
    prx_buffer_factory_t* pool;                // dynamic pool
    size_t increment;   // Grow buffer by this number of bytes
    uint8_t* out;                           // != null on fini
    size_t out_len;                                    // Size 
}
io_dynamic_buffer_stream_t;

//
// File stream for input and output
//
typedef struct io_file_stream
{
    io_stream_t itf;           // codec stream interface
    void* in_fd;
    void* out_fd;
}
io_file_stream_t;

//
// Initialize a fixed in/out buffer memory stream 
//
decl_internal_5(io_stream_t*, io_fixed_buffer_stream_init,
    io_fixed_buffer_stream_t*, mem,
    const uint8_t*, in,
    size_t, in_len,
    uint8_t*, out,
    size_t, out_len
);

//
// Initialize a dynamic out buffer memory stream 
//
decl_internal_3(io_stream_t*, io_dynamic_buffer_stream_init,
    io_dynamic_buffer_stream_t*, mem,
    prx_buffer_factory_t*, pool,
    size_t, increment
);

//
// Initialize a file stream 
//
decl_internal_3(io_stream_t*, io_file_stream_init,
    io_file_stream_t*, fs,
    const char*, in_file,
    const char*, out_file
);

#endif // _util_stream_h_