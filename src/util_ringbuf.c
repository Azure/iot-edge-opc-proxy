// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "util_ringbuf.h"

//
// Ring buffer head
//
struct ring_buffer
{
    uint8_t* start;    // points to the first item in the buffer
    uint8_t* end;               // a pointer one past the buffer
    uint8_t* read_pos;             // The first item in the ring
    uint8_t* write_pos;   // Points to the last item in the ring
};

//          
// Create ring buffer
//
int32_t ring_buffer_create(
    size_t size,
    ring_buffer_t** created
)
{
    int32_t result;
    ring_buffer_t *rb;
    
    rb = mem_zalloc_type(ring_buffer_t);
    if (!rb)
         return er_out_of_memory;
    do
    {
        rb->start = (uint8_t*)mem_zalloc(size + 1);
        if (!rb->start)
        {
            result = er_out_of_memory;
            break;
        }

        rb->read_pos = rb->start;
        rb->write_pos = rb->start;
        rb->end = &(rb->start[size + 1]);   // point past buffer

        *created = rb;
        return er_ok;
    } 
    while (0);

    ring_buffer_free(rb);
    return result;
}

//
// Free ring buffer
//
void ring_buffer_free(
    ring_buffer_t* rb
)
{
    if (!rb)
        return;

    if (rb->start)
        mem_free(rb->start);

    mem_free_type(ring_buffer_t, rb);
}

//
// Write to ring buffer
//
size_t ring_buffer_write(
    ring_buffer_t* rb, 
    const uint8_t* buf, 
    size_t len
)
{
    size_t written = 0;
    size_t avail;
    const volatile uint8_t* read_pos = rb->read_pos;

    if (rb->write_pos >= read_pos)
    {
        // The write pointer is either at or ahead of the read 
        // pointer.  If they are equal, the buffer is empty.
        avail = rb->end - rb->write_pos;
        
        // Edge case: Do not catch up to the read pointer
        // or we are empty again
        if (read_pos == rb->start) 
        {
            --avail;
            if (len > avail)
                len = avail;
        }

        if (len < avail)
            avail = len;
        len -= avail;

        if (avail > 0)
        {
            memcpy(rb->write_pos, buf, avail);
            rb->write_pos += avail;

            written += avail;
        }

        // Wrap if needed
        if (rb->write_pos == rb->end)
            rb->write_pos = rb->start;
    }
    
    if (len > 0)
    {
        // Make sure not to catch up with the read pointer, 
        // or else we are empty
        avail = read_pos - rb->write_pos - 1;
        if (len < avail)
            avail = len;
        len -= avail;

        if (avail > 0)
        {
            memcpy(rb->write_pos, &(buf[written]), avail);
            rb->write_pos += avail;

            written += avail;
        }
    }
    return written;
}

//
// Check if buffer is empty
//
bool ring_buffer_is_empty(
    ring_buffer_t* rb
)
{
    //
    // Rather than tracking full or empty state in the struct
    // end expose us to race conditions, the buffer is full if 
    // the write pointer is 1 behind the read pointer.  
    //
    // Consequently the buffer is empty if write pointer and  
    // read pointer are the same.
    //
    return rb->write_pos == rb->read_pos;
}

//
// Reset ring buffer
//
void ring_buffer_clear(
    ring_buffer_t* rb
)
{
    rb->write_pos = rb->read_pos = rb->start;
}

//
// Return capacity, i.e. what can be written
//
size_t ring_buffer_capacity(
    ring_buffer_t *rb
)
{
    const volatile uint8_t* read_pos = rb->read_pos;
    if (rb->write_pos >= read_pos)
        return (rb->end - rb->write_pos) + (read_pos - rb->start) - 1;
    return read_pos - rb->write_pos - 1;
}

//
// Read from ring buffer
//
size_t ring_buffer_read(
    ring_buffer_t* rb,
    uint8_t* buf,
    size_t len
)
{
    size_t read = 0;
    size_t avail;

    const volatile uint8_t * write_pos = rb->write_pos;

    if (rb->read_pos > write_pos)
    {
        // Read up to the end, and whatever else is remaining
        avail = rb->end - rb->read_pos;
        if (len < avail)
            avail = len;
        len -= avail;

        if (avail > 0)
        {
            memcpy(buf, rb->read_pos, avail);
            rb->read_pos += avail;

            read += avail;
        }

        // Wrap if needed
        if (rb->read_pos == rb->end)
            rb->read_pos = rb->start;
    }

    if (len > 0)
    {
        // Make sure not to catch up with the read pointer, or else
        // we are empty
        avail = (write_pos - rb->read_pos);
        if (len < avail)
            avail = len;
        len -= avail;

        if (avail > 0)
        {
            memcpy(&(buf[read]), rb->read_pos, avail);
            rb->read_pos += avail;

            read += avail;
        }
    }
    return read;
}

//
// Return how much is available
//
size_t ring_buffer_available(
    ring_buffer_t *rb
)
{
    const volatile uint8_t* write_pos = rb->write_pos;

    if (rb->read_pos > write_pos)
        return (rb->end - rb->read_pos) + (write_pos - rb->start);
    return write_pos - rb->read_pos;
}
