// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "util_stream.h"
#include "pal.h"

#include <stdio.h>

//
// Reads from fixed buffer
//
static int32_t io_fixed_buffer_stream_reader(
    void *context,
    void *data,
    size_t count,
    size_t* read
)
{
    io_fixed_buffer_stream_t* mem = (io_fixed_buffer_stream_t*)context;
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
    void *context,
    const void *data,
    size_t count
)
{
    io_fixed_buffer_stream_t* mem = (io_fixed_buffer_stream_t*)context;
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
    void *context
)
{
    dbg_assert_ptr(context);
    return ((io_fixed_buffer_stream_t*)context)->in_len;
}

//
// Available in memory to write
//
static size_t io_fixed_buffer_stream_writeable(
    void *context
)
{
    dbg_assert_ptr(context);
    return ((io_fixed_buffer_stream_t*)context)->out_len;
}

//
// Writes to dynamic buffer
//
static int32_t io_dynamic_buffer_stream_writer(
    void *context,
    const void *data,
    size_t count
)
{
    int32_t result;
    size_t size;
    io_dynamic_buffer_stream_t* mem = (io_dynamic_buffer_stream_t*)context;
    dbg_assert_ptr(data);
    dbg_assert_ptr(mem);

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
// Available in memory to write
//
static size_t io_dynamic_buffer_stream_writeable(
    void *context
)
{
    (void)context;
    return (size_t )-1;
}

//
// Reads from file
//
static int32_t io_file_stream_reader(
    void *context,
    void *data,
    size_t count,
    size_t* read
)
{
    io_file_stream_t* fs = (io_file_stream_t*)context;
    dbg_assert_ptr(read);
    dbg_assert_ptr(data);
    dbg_assert_ptr(fs);

    if (!fs->fd)
        return er_disk_io;
    *read = fread(data, 1, count, (FILE*)fs->fd);
    if (*read < 1)
        return er_reading;
    return er_ok;
}

//
// Writes to file
//
static int32_t io_file_stream_writer(
    void *context,
    const void *data,
    size_t count
)
{
    io_file_stream_t* fs = (io_file_stream_t*)context;
    dbg_assert_ptr(data);
    dbg_assert_ptr(fs);
    if (!fs->fd)
        return er_disk_io;
    if (1 > fwrite(data, 1, count, (FILE*)fs->fd))
        return er_writing;
    return er_ok;
}

//
// Available in file to read
//
static size_t io_file_stream_readable(
    void *context
)
{
    io_file_stream_t* fs = (io_file_stream_t*)context;
    long pos, avail;
    dbg_assert_ptr(fs);
    if (!fs->fd)
        return 0;
    pos = ftell((FILE*)fs->fd);
    fseek((FILE*)fs->fd, 0L, SEEK_END);
    avail = ftell((FILE*)fs->fd);
    fseek((FILE*)fs->fd, pos, SEEK_SET);

    return (size_t)(avail - pos);
}

//
// Available in file to write
//
static size_t io_file_stream_writeable(
    void *context
)
{
    (void)context;
    return (size_t)-1;
}

//
// Reset stream
//
static int32_t io_file_stream_reset(
    void *context
)
{
    io_file_stream_t* fs = (io_file_stream_t*)context;
    dbg_assert_ptr(fs);

    if (0 == ftell((FILE*)fs->fd))
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
    void *context
)
{
    io_file_stream_t* fs = (io_file_stream_t*)context;
    dbg_assert_ptr(fs);
    fclose((FILE*)fs->fd);
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
    mem->itf.read = 
        io_fixed_buffer_stream_reader;
    mem->itf.write = 
        io_fixed_buffer_stream_writer;
    mem->itf.readable =
        io_fixed_buffer_stream_readable;
    mem->itf.writeable =
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
    mem->out = (uint8_t*)prx_buffer_new(pool, mem->increment);
    if (!mem->out)
        return NULL;

    mem->itf.context =
        mem;
    mem->itf.readable =
        NULL;
    mem->itf.read =
        NULL;
    mem->itf.write =
        io_dynamic_buffer_stream_writer;
    mem->itf.writeable =
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
    const char* file_name,
    const char* mode
)
{
    fs->fd = fopen(file_name, mode);
    if (!fs->fd)
        return NULL;
    fs->itf.context =
        fs;
    fs->itf.read =
        io_file_stream_reader;
    fs->itf.readable =
        io_file_stream_readable;
    fs->itf.write =
        io_file_stream_writer;
    fs->itf.writeable =
        io_file_stream_writeable;
    fs->itf.reset =
        io_file_stream_reset;
    fs->itf.close =
        io_file_stream_close;
    return &fs->itf;
}

