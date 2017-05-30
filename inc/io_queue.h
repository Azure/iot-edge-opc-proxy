// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _io_queue_h_
#define _io_queue_h_

#include "common.h"
#include "io_stream.h"
#include "azure_c_shared_utility/doublylinkedlist.h"

//
// A queue of buffers for I/O buffering
//
typedef struct io_queue io_queue_t;

//
// Header of a queued buffer
//
typedef struct io_queue_buffer
{
    DLIST_ENTRY qlink;             // Used to link buffer into queue
    io_queue_t* queue;                // Queue the buffer is part of

    void (*cb_ptr)(void*, int32_t);   // A space to store a callback
    void* ctx;                     // An opaque context for callback

    io_stream_t itf;         // Stream interface for reading/writing
    int32_t flags;        // Flags, e.g. to support fragment buffers
    int32_t code;            // buffer or error code for convinience
    size_t write_offset;    // Tracks a write offset into the buffer
    size_t read_offset;      // Tracks a read offset into the buffer
    size_t length;              // length of the buffer that follows

 // int8_t ptr[]
}
io_queue_buffer_t;

//
// allocate a queue
//
decl_internal_2(int32_t, io_queue_create,
    const char*, name,
    io_queue_t**, queue
    );

//
// free the queue
//
decl_internal_1(void, io_queue_free,
    io_queue_t*, queue
    );

//
// Whether ready buffers are present
//
decl_internal_1(bool, io_queue_has_ready,
    io_queue_t*, queue
);

//
// Retrieve the first ready buffer from the queue
//
decl_internal_1(io_queue_buffer_t*, io_queue_pop_ready,
    io_queue_t*, queue
    );

//
// Whether in progress buffers are present
//
decl_internal_1(bool, io_queue_has_inprogress,
    io_queue_t*, queue
);

//
// Retrieve the first in progress buffer from the queue
//
decl_internal_1(io_queue_buffer_t*, io_queue_pop_inprogress,
    io_queue_t*, queue
    );

//
// Whether completed buffers are present
//
decl_internal_1(bool, io_queue_has_done,
    io_queue_t*, queue
);

//
// Retrieve the first completed buffer from the queue
//
decl_internal_1(io_queue_buffer_t*, io_queue_pop_done,
    io_queue_t*, queue
);

//
// Move all in progress messages back to ready state
//
decl_internal_1(void, io_queue_rollback,
    io_queue_t*, queue
);

//
// Abort all messages in the queue
//
decl_internal_1(void, io_queue_abort,
    io_queue_t*, queue
);

//
// Creates a new buffer with given content
//
decl_internal_4(int32_t, io_queue_create_buffer,
    io_queue_t*, queue,
    const void*, payload,
    size_t, size,
    io_queue_buffer_t**, buffer
);

//
// move the buffer into in "done" state
//
decl_internal_1(void, io_queue_buffer_set_done,
    io_queue_buffer_t*, buffer
);

//
// move the buffer into "in progress" state
//
decl_internal_1(void, io_queue_buffer_set_inprogress,
    io_queue_buffer_t*, buffer
);

//
// move the buffer into in "ready" state
//
decl_internal_1(void, io_queue_buffer_set_ready,
    io_queue_buffer_t*, buffer
);

//
// Get the raw buffer from buffer handle
//
decl_internal_1(uint8_t*, io_queue_buffer_to_ptr,
    io_queue_buffer_t*, buffer
);

//
// Get the buffer handle from raw potr
//
decl_internal_1(io_queue_buffer_t*, io_queue_buffer_from_ptr,
    uint8_t*, payload
);

//
// Get stream from buffer
//
decl_internal_1(io_stream_t*, io_queue_buffer_as_stream,
    io_queue_buffer_t*, buffer
);

//
// Release buffer and invoke any registered callback
//
decl_internal_1(void, io_queue_buffer_release,
    io_queue_buffer_t*, buffer
);

#endif // _io_queue_h_