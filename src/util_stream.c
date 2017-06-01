// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "util_misc.h"
#include "util_stream.h"
#include "pal.h"

#include <stdio.h>

//
// Reads from fixed buffer
//
static int32_t io_fixed_buffer_stream_reader(
    io_fixed_buffer_stream_t* mem,
    void *data,
    size_t count,
    size_t* read
)
{
    dbg_assert_ptr(read);
    dbg_assert_ptr(data);
    dbg_assert_ptr(mem);
    *read = min(count, mem->in_len);
    if (*read > 0)
    {
        memcpy(data, mem->in, *read);
        mem->in_len -= *read;
        mem->in += *read;
    }
    return er_ok;
}

//
// Writes to fixed buffer
//
static int32_t io_fixed_buffer_stream_writer(
    io_fixed_buffer_stream_t* mem,
    const void *data,
    size_t count
)
{
    dbg_assert_ptr(data);
    dbg_assert_ptr(mem);
    if (count > mem->out_len)
        return er_writing;
    else if (count > 0)
    {
        memcpy(mem->out, data, count);
        mem->out_len -= count;
        mem->out += count;
    }
    return er_ok;
}

//
// Available in memory to read
//
static size_t io_fixed_buffer_stream_readable(
    io_fixed_buffer_stream_t* mem
)
{
    dbg_assert_ptr(mem);
    return mem->in_len;
}

//
// Available in memory to write
//
static size_t io_fixed_buffer_stream_writeable(
    io_fixed_buffer_stream_t* mem
)
{
    dbg_assert_ptr(mem);
    return mem->out_len;
}

//
// Writes to dynamic buffer
//
static int32_t io_dynamic_buffer_stream_writer(
    io_dynamic_buffer_stream_t* mem,
    const void *data,
    size_t count
)
{
    int32_t result;
    size_t size;
    dbg_assert_ptr(data);
    dbg_assert_ptr(mem);
    dbg_assert_ptr(mem->pool);

    if (!count)
        return er_ok;

    size = prx_buffer_get_size(mem->pool, mem->out);
    if (mem->out_len + count > size)
    {
        // Grow
        size = (((mem->out_len + count) / mem->increment) + 1) * mem->increment;
        result = prx_buffer_set_size(mem->pool, (void**)&mem->out, size);
        if (result != er_ok)
            return result;
    }

    memcpy(&mem->out[mem->out_len], data, count);
    mem->out_len += count;
    return er_ok;
}

//
// Writes to malloc'ed memory
//
static int32_t io_dynamic_memory_stream_writer(
    io_dynamic_buffer_stream_t* mem,
    const void *data,
    size_t count
)
{
    void* tmp;

    dbg_assert_ptr(data);
    dbg_assert_ptr(mem);
    dbg_assert(!mem->pool, "Unexpected");

    if (!count)
        return er_ok;
    while (mem->out_len + count > mem->increment)
    {
        // Grow
        tmp = mem_realloc(mem->out, 2 * mem->increment);
        if (!tmp)
            return er_out_of_memory;

        mem->out = (uint8_t*)tmp;
        mem->increment *= 2;
    }

    memcpy(&mem->out[mem->out_len], data, count);
    mem->out_len += count;
    return er_ok;
}

//
// Available in memory to write
//
static size_t io_dynamic_buffer_stream_writeable(
    io_dynamic_buffer_stream_t* mem
)
{
    (void)mem;
    return (size_t )-1;
}

//
// Reads from file
//
static int32_t io_file_stream_reader(
    io_file_stream_t* fs,
    void *data,
    size_t count,
    size_t* read
)
{
    dbg_assert_ptr(read);
    dbg_assert_ptr(data);
    dbg_assert_ptr(fs);
    dbg_assert_ptr(fs->in_fd);

    *read = fread(data, 1, count, (FILE*)fs->in_fd);
    if (*read < 1)
        return er_reading;
    return er_ok;
}

//
// Writes to file
//
static int32_t io_file_stream_writer(
    io_file_stream_t* fs,
    const void *data,
    size_t count
)
{
    dbg_assert_ptr(data);
    dbg_assert_ptr(fs);
    dbg_assert_ptr(fs->out_fd);

    if (1 > fwrite(data, 1, count, (FILE*)fs->out_fd))
        return er_writing;
    return er_ok;
}

//
// Available in file to read
//
static size_t io_file_stream_readable(
    io_file_stream_t* fs
)
{
    long pos, avail;
    dbg_assert_ptr(fs);
    dbg_assert_ptr(fs->in_fd);

    pos = ftell((FILE*)fs->in_fd);
    fseek((FILE*)fs->in_fd, 0L, SEEK_END);
    avail = ftell((FILE*)fs->in_fd);
    fseek((FILE*)fs->in_fd, pos, SEEK_SET);

    return (size_t)(avail - pos);
}

//
// Available in file to write
//
static size_t io_file_stream_writeable(
    io_file_stream_t* fs
)
{
    (void)fs;
    return (size_t)-1;
}

//
// Reset stream
//
static int32_t io_file_stream_reset(
    io_file_stream_t* fs
)
{
    dbg_assert_ptr(fs);

    if (fs->in_fd)
        (void)fseek((FILE*)fs->in_fd, 0, SEEK_SET);
    if (!fs->out_fd || 0 == ftell((FILE*)fs->out_fd))
        return er_ok;

    //
    // Truncate not implemented, better to return an error than
    // to write to the end or beginning of the same file
    //
    return er_not_impl;
}

//
// Close stream
//
static void io_file_stream_close(
    io_file_stream_t* fs
)
{
    dbg_assert_ptr(fs);
    if (fs->out_fd)
        fclose((FILE*)fs->out_fd);
    if (fs->in_fd)
        fclose((FILE*)fs->in_fd);
}

//
// Initialize a fixed in/out buffer stream on stack
//
io_stream_t* io_fixed_buffer_stream_init(
    io_fixed_buffer_stream_t* mem,
    const uint8_t* in,
    size_t in_len,
    uint8_t* out,
    size_t out_len
)
{
    mem->in = in;
    mem->in_len = in_len;
    mem->out = out;
    mem->out_len = out_len;

    mem->itf.context = 
        mem;
    mem->itf.reader = (io_stream_reader_t)
        io_fixed_buffer_stream_reader;
    mem->itf.writer = (io_stream_writer_t)
        io_fixed_buffer_stream_writer;
    mem->itf.readable = (io_stream_available_t)
        io_fixed_buffer_stream_readable;
    mem->itf.writeable = (io_stream_available_t)
        io_fixed_buffer_stream_writeable;
    mem->itf.reset =
        NULL;
    mem->itf.close =
        NULL;
    return &mem->itf;
}

//
// Initialize a dynamic out buffer memory stream 
//
io_stream_t* io_dynamic_buffer_stream_init(
    io_dynamic_buffer_stream_t* mem,
    prx_buffer_factory_t* pool,
    size_t increment
)
{
    mem->pool = pool;
    mem->increment = increment ? increment : 0x400;
    mem->out_len = 0;
    if (!pool)
        mem->out = (uint8_t*)mem_alloc(mem->increment);
    else
        mem->out = (uint8_t*)prx_buffer_new(pool, mem->increment);
    if (!mem->out)
        return NULL;

    mem->itf.context =
        mem;
    mem->itf.readable =
        NULL;
    mem->itf.reader =
        NULL;
    mem->itf.writer = (io_stream_writer_t)(
        pool ? io_dynamic_buffer_stream_writer : io_dynamic_memory_stream_writer);
    mem->itf.writeable = (io_stream_available_t)
        io_dynamic_buffer_stream_writeable;
    mem->itf.reset =
        NULL;
    mem->itf.close =
        NULL;
    return &mem->itf;
}

//
// Initialize a file stream object on stack
//
io_stream_t* io_file_stream_init(
    io_file_stream_t* fs,
    const char* in_file,
    const char* out_file
)
{
    if (!in_file && !out_file)
        return NULL;

    fs->itf.context =
        fs;
    fs->itf.reset = (io_stream_reset_t)
        io_file_stream_reset;
    fs->itf.close = (io_stream_close_t)
        io_file_stream_close;
    fs->itf.reader =
        NULL;
    fs->itf.readable =
        NULL;
    fs->in_fd = 
        NULL;
    fs->out_fd = 
        NULL;
    fs->itf.writer =
        NULL;
    fs->itf.writeable =
        NULL;

    if (in_file)
    {
        fs->in_fd = fopen(in_file, "r");
        if (!fs->in_fd)
        {
            io_file_stream_close(fs);
            return NULL;
        }
        fs->itf.reader = (io_stream_reader_t)
            io_file_stream_reader;
        fs->itf.readable = (io_stream_available_t)
            io_file_stream_readable;
    }

    if (out_file)
    {
        fs->out_fd = fopen(out_file, "w");
        if (!fs->out_fd)
        {
            io_file_stream_close(fs);
            return NULL;
        }
        fs->itf.writer = (io_stream_writer_t)
            io_file_stream_writer;
        fs->itf.writeable = (io_stream_available_t)
            io_file_stream_writeable;
    }

    return &fs->itf;
}

