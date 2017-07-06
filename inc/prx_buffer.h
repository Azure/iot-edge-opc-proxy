// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _prx_buffer_h_
#define _prx_buffer_h_

#include "common.h"
#include "util_dbg.h"

//
// Returns how many buffers can be created
//
typedef size_t (*prx_buffer_factory_available_t) (
    void* context
    );

//
// Allocate buffer 
//
typedef void* (*prx_buffer_alloc_t) (
    void* context,
    void* original
    );

//
// Resize the buffer to length, returns new buffer if needed
//
typedef int32_t (*prx_buffer_set_size_t) (
    void* context,
    void** buffer,
    size_t length
    );

//
// Get size of buffer
//
typedef size_t (*prx_buffer_get_size_t) (
    void* context,
    void* buffer
    );

//
// Release buffer - pool returns memory to pool, etc.
//
typedef void (*prx_buffer_free_t) (
    void* context,
    void* buffer
    );

//
// Release buffer buffer_factory
//
typedef void (*prx_buffer_factory_free_t) (
    void* context
    );

//
// Buffer factory interface, provides different implementations, e.g. pool
//
typedef struct prx_buffer_factory
{
    void* context;                     // Buffer buffer_factory implementation 
    prx_buffer_alloc_t on_alloc;                           // Allocate a buffer
    prx_buffer_get_size_t on_get_size;        // Returns the size of the buffer
    prx_buffer_set_size_t on_set_size;           // Resize the buffer to length
    prx_buffer_free_t on_release;                             // Release buffer

    prx_buffer_factory_free_t on_free;                   // Free buffer factory
    prx_buffer_factory_available_t on_available;     // Items available in pool
}
prx_buffer_factory_t;

//
// If available returns 0, er_out_of_memory is returned by create
//
decl_inline_1(size_t, prx_buffer_factory_get_available,
    prx_buffer_factory_t*, buffer_factory
)
{
    if (!buffer_factory)
        return 0;
    dbg_assert_ptr(buffer_factory->on_available);
    return buffer_factory->on_available(buffer_factory->context);
}

//
// Allocate a buffer from the buffer factory
//
decl_inline_2(void*, prx_buffer_alloc,
    prx_buffer_factory_t*, buffer_factory,
    void*, original
)
{
    if (!buffer_factory)
        return NULL;
    dbg_assert_ptr(buffer_factory->on_alloc);
    return buffer_factory->on_alloc(buffer_factory->context, original);
}

//
// Allocate buffer with size - factory should allow dynamic resizing
//
decl_internal_2(void*, prx_buffer_new,
    prx_buffer_factory_t*, buffer_factory,
    size_t, length
);

//
// Get access to raw memory
//
decl_inline_2(size_t, prx_buffer_get_size,
    prx_buffer_factory_t*, buffer_factory,
    const void*, buffer
)
{
    if (!buffer_factory)
        return 0;
    dbg_assert_ptr(buffer_factory->on_get_size);
    return buffer_factory->on_get_size(buffer_factory->context, (void*)buffer);
}

//
// Resize buffer, does not shrink in all pool case
//
decl_inline_3(int32_t, prx_buffer_set_size,
    prx_buffer_factory_t*, buffer_factory,
    void**, buffer,
    size_t, length
)
{
    chk_arg_fault_return(buffer_factory);
    dbg_assert_ptr(buffer_factory->on_set_size);
    return buffer_factory->on_set_size(buffer_factory->context, buffer, length);
}

//
// Safely call release on buffer
//
decl_inline_2(void, prx_buffer_release,
    prx_buffer_factory_t*, buffer_factory,
    void*, buffer
)
{
    if (!buffer_factory)
        return;
    dbg_assert_ptr(buffer_factory->on_release);
    buffer_factory->on_release(buffer_factory->context, buffer);
}

//
// Safely call release on buffer buffer_factory
//
decl_inline_1(void, prx_buffer_factory_free,
    prx_buffer_factory_t*, buffer_factory
)
{
    if (!buffer_factory)
        return;
    dbg_assert_ptr(buffer_factory->on_free);
    buffer_factory->on_free(buffer_factory->context);
}

//
// Callback notifying about state of pool
//
typedef void (*prx_buffer_pool_cb_t)(
    void* context,
    bool empty
    );

//
// Pool configuration
//
typedef struct prx_pool_config
{
    size_t initial_count;                            // 1 == allocate per item
    size_t max_count;                             // 0 == grows pool on demand
    size_t low_watermark;                    // if >= high_watermark, set to 0
    size_t high_watermark;                // if >= max_count, set to max_count
    prx_buffer_pool_cb_t cb;    // called with true if dip below low_watermark
    void* context;
}
prx_pool_config_t;

//
// Creates a buffer pool buffer_factory which allows growing items
//
decl_internal_4(int32_t, prx_dynamic_pool_create,
    const char*, name,
    size_t, initial_item_size,
    prx_pool_config_t*, config,
    prx_buffer_factory_t**, pool
);

//
// Creates a buffer pool buffer_factory for fixed sized items
//
decl_internal_4(int32_t, prx_fixed_pool_create,
    const char*, name,
    size_t, fixed_item_size, 
    prx_pool_config_t*, config,          
    prx_buffer_factory_t**, pool
);

#endif // _prx_buffer_h_