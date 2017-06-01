// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _io_stream_h_
#define _io_stream_h_

#include "common.h"
#include "util_dbg.h"

//
// Represents an open streamthat can be read from or written to.
//
typedef struct io_stream io_stream_t;

//
// Returns amount of data readable or writeable
//
typedef size_t (*io_stream_available_t)(
    void* ctx
    );

//
// Read encoded data 
//
typedef int32_t (*io_stream_reader_t)(
    void* ctx,
    void* buf, 
    size_t len,
    size_t* read
    );

//
// Reset stream
//
typedef int32_t(*io_stream_reset_t)(
    void* ctx
    );

//
// Write encoded data
//
typedef int32_t (*io_stream_writer_t)(
    void* ctx, 
    const void *buf,
    size_t len
    );

//
// Close stream
//
typedef void (*io_stream_close_t)(
    void* ctx
    );

//
// Stream interface
//
struct io_stream
{
    void* context;
    io_stream_available_t readable;
    io_stream_reader_t reader;
    io_stream_reset_t reset;
    io_stream_available_t writeable;
    io_stream_writer_t writer;
    io_stream_close_t close;
};

//
// Get whether stream is readable and how much can be read
//
decl_inline_1(size_t, io_stream_readable,
    io_stream_t*, stream
)
{
    if (!stream)
        return 0;
    if (!stream->readable)
        return 0;  // Cannot read from stream
    return stream->readable(stream->context);
}

//
// Reset stream
//
decl_inline_1(int32_t, io_stream_reset,
    io_stream_t*, stream
)
{
    chk_arg_fault_return(stream);
    if (!stream->reset)
        return er_ok;
    return stream->reset(stream->context);
}

//
// Read from stream
//
decl_inline_4(int32_t, io_stream_read,
    io_stream_t*, stream,
    void*, buf,
    size_t, len,
    size_t*, read
)
{
    chk_arg_fault_return(stream);
    if (!stream->reader)
        return er_not_supported;  // Cannot read from stream
    return stream->reader(stream->context, buf, len, read);
}

//
// Get whether stream is writable and how much can be written
//
decl_inline_1(size_t, io_stream_writeable,
    io_stream_t*, stream
)
{
    if (!stream)
        return 0;
    if (!stream->writeable)
        return 0;  // Cannot write to stream
    return stream->writeable(stream->context);
}

//
// Write to stream
//
decl_inline_3(int32_t, io_stream_write,
    io_stream_t*, stream,
    const void*, buf,
    size_t, len
)
{
    chk_arg_fault_return(stream);
    if (!stream->writer)
        return er_not_supported;  // Cannot write to stream
    return stream->writer(stream->context, buf, len);
}

//
// Close stream
//
decl_inline_1(void, io_stream_close,
    io_stream_t*, stream
)
{
    if (!stream || !stream->close)
        return;
    stream->close(stream->context);
}

//
// Provides a stream to a data consumer or producer
//
typedef int32_t (*io_stream_handler_t)(
    void* context,
    io_stream_t* stream // Use this to read or write message
    );


#endif  // _io_stream_h_