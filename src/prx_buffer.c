// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// #define DBG_MEM

#include "util_mem.h"
#include "util_misc.h"
#include "prx_buffer.h"
#include "pal_mt.h"

#include "azure_c_shared_utility/doublylinkedlist.h"
#include "azure_c_shared_utility/refcount.h"

//
// Universal buffer header
//
typedef struct prx_buffer
{
#ifdef DBG_MEM
    uint64_t sentinel;
#endif
    size_t length;
    COUNT_TYPE refs;
    DLIST_ENTRY link;
}
prx_buffer_t;

//
// Universal buffer pool structure
//
typedef struct prx_buffer_pool
{
    const char* name;
    lock_t lock;
    DLIST_ENTRY free_list;                      // buffers that are free
    size_t free_count;             // Number of free buffers in the pool
    DLIST_ENTRY checked_out;             // buffers that are checked out
    size_t item_size;                             // default buffer size
    prx_pool_config_t config;
    size_t count;                   // Number of items made by this pool
    bool empty;     // true = dipped below low, false = pushed past high
}
prx_buffer_pool_t;

//
// Dynamic allocation
//
typedef struct prx_dynamic_pool
{
    prx_buffer_factory_t funcs;      // Interface (first so we can cast)
    prx_buffer_pool_t pool;                     // Universal pool struct
}
prx_dynamic_pool_t;

//
// Allocates from blocks
//
typedef struct prx_fixed_pool
{
    prx_buffer_factory_t funcs;      // Interface (first so we can cast)
    prx_buffer_pool_t pool;                     // Universal pool struct
    DLIST_ENTRY blocks;      // Allocated blocks (prx_buffer) of buffers
}
prx_fixed_pool_t;

//
// Grow policy
//
typedef int32_t (*prx_buffer_pool_grow_t)(
    void* context
    );


#define __prx_buffer(p) \
    (prx_buffer_t*)(((char*)(p)) - sizeof(prx_buffer_t))

#define __raw_buffer(b) \
    (void*)(((char*)(b)) + sizeof(prx_buffer_t))


//
// Debug buffer corruption
//
#ifdef DBG_MEM
#define _sentinel 0x98112124444ULL
#define dbg_assert_buf(buf) \
    dbg_assert(buf->sentinel == _sentinel, "Bad sentinel in %p (%llx).", \
        buf, buf->sentinel)
#else
#define dbg_assert_buf(buf)
#endif

//
// Dummy if user does not provide callback
//
static void prx_buffer_available_dummy_callback(
    void* context,
    bool empty
)
{
    (void)context;
    (void)empty;
}

//
// Init buffer pool struct
//
int32_t prx_buffer_pool_init(
    prx_buffer_pool_t* pool,
    const char* name,
    size_t item_size,
    prx_pool_config_t* config
)
{
    int32_t result;

    DList_InitializeListHead(&pool->checked_out);
    DList_InitializeListHead(&pool->free_list);

    result = lock_create(&pool->lock);
    if (result != er_ok)
        return result;

    pool->name = name;
    pool->item_size = item_size;
    pool->count = 0;

    if (config)
        memcpy(&pool->config, config, sizeof(prx_pool_config_t));
    else
        memset(&pool->config, 0, sizeof(prx_pool_config_t));

    if (!pool->config.cb)
        pool->config.cb = prx_buffer_available_dummy_callback;
    if (!pool->config.initial_count)
        pool->config.initial_count = 10;

    if (pool->config.low_watermark > pool->config.high_watermark)
        pool->config.low_watermark = 0;
    if (pool->config.max_count != 0 && 
        pool->config.high_watermark > pool->config.max_count)
        pool->config.high_watermark = pool->config.max_count;
    return er_ok;
}

//
// Deinit buffer pool struct
//
void prx_buffer_pool_deinit(
    prx_buffer_pool_t* pool
)
{
    if (pool->lock)
        lock_free(pool->lock);

    pool->config.cb = prx_buffer_available_dummy_callback;
}

//
// Prototype of function to create buffer 
//
static void* prx_buffer_pool_alloc_buffer(
    prx_buffer_pool_t* pool,
    void* original,
    prx_buffer_pool_grow_t grow,
    void* context
)
{
    void* result;
    prx_buffer_t* out, *in = NULL;
    bool signal = false;

    dbg_assert_ptr(pool);

    if (original)
    {
        in = __prx_buffer(original);
        dbg_assert_buf(in);
        atomic_inc(in->refs);
        return original;
    }

    lock_enter(pool->lock);
    do
    {
        if (DList_IsListEmpty(&pool->free_list))
        {
            result = NULL;
            break;
        }

        out = containingRecord(
            DList_RemoveHeadList(&pool->free_list), prx_buffer_t, link);
        DList_InitializeListHead(&out->link);
        dbg_assert_buf(out);

        --pool->free_count;
        if (pool->free_count == 0) // attempt to grow if we are empty
            grow(context);
        if (pool->free_count <= pool->config.low_watermark && !pool->empty)
            pool->empty = signal = true;

        // Take a reference on the pool buf
        atomic_inc(out->refs);
        DList_InsertTailList(&pool->checked_out, &out->link);
        dbg_assert_buf(out);
        result = __raw_buffer(out);
    } 
    while (0);
    lock_exit(pool->lock);
    
    if (signal)
        pool->config.cb(pool->config.context, true);  // Low mem

    return result;
}

//
// Return buffer to pool
//
static void prx_buffer_pool_free_buffer(
    prx_buffer_pool_t* pool,
    void* buffer
)
{
    prx_buffer_t* pool_buf = __prx_buffer(buffer);
    bool no_signal = true;
    dbg_assert_ptr(pool);
    dbg_assert_buf(pool_buf);

    if (!atomic_dec(pool_buf->refs))
    {
        // Return to free list
        lock_enter(pool->lock);
        DList_RemoveEntryList(&pool_buf->link);
        DList_InsertTailList(&pool->free_list, &pool_buf->link);
        
        ++pool->free_count;

        if (pool->free_count > pool->config.high_watermark && pool->empty)
            pool->empty = no_signal = false;
        lock_exit(pool->lock);

        if (!no_signal)
            pool->config.cb(pool->config.context, false); // High mem
    }
}

//
// Returns how many buffers can be created
//
static size_t prx_dynamic_pool_available(
    prx_dynamic_pool_t* pool
)
{
    return pool->pool.free_count;
}

//
// Release dynamic pool
//
static void prx_dynamic_pool_free(
    prx_dynamic_pool_t* pool
)
{
    prx_buffer_t* next;
    dbg_assert_ptr(pool);

    if (pool->pool.lock)
    {
        lock_enter(pool->pool.lock);

#if defined(DBG_MEM)
        // Free all checked_out buffers, and let owner crash
        while (!DList_IsListEmpty(&pool->pool.checked_out))
        {
            next = containingRecord(
                DList_RemoveHeadList(&pool->pool.checked_out), prx_buffer_t, link);
            dbg_assert_buf(next);
            mem_free(next);
        }
#else
        dbg_assert(DList_IsListEmpty(&pool->pool.checked_out),
            "Leaking buffer that was not returned to pool!");
#endif
        while (!DList_IsListEmpty(&pool->pool.free_list))
        {
            next = containingRecord(
                DList_RemoveHeadList(&pool->pool.free_list), prx_buffer_t, link);
            dbg_assert_buf(next);
            mem_free(next);
        }

        lock_exit(pool->pool.lock);
    }

    prx_buffer_pool_deinit(&pool->pool);
    mem_free_type(prx_dynamic_pool_t, pool);
}

//
// Grow pool to specified size of dynamic buffers
//
static int32_t prx_dynamic_pool_grow_no_lock(
    prx_dynamic_pool_t* pool
)
{
    int32_t result;
    prx_buffer_t* buf;
    size_t items;

    items = pool->pool.config.initial_count;
    if (pool->pool.config.max_count)
        items = min(pool->pool.config.max_count - pool->pool.count, items);

    result = er_out_of_memory;
    for (size_t i = 0; i < items; i++)
    {
        buf = (prx_buffer_t*)mem_alloc(sizeof(prx_buffer_t) + pool->pool.item_size);
        if (!buf)
        {
            result = er_out_of_memory;
            break;
        }

        DList_InitializeListHead(&buf->link);
#ifdef DBG_MEM
        buf->sentinel = _sentinel;
#endif
        buf->refs = 0;  // Starts at 0, increased on checkout.
        buf->length = pool->pool.item_size;

        DList_InsertTailList(&pool->pool.free_list, &buf->link);
        pool->pool.count++;
        pool->pool.free_count++;
    }
    
    // log_debug(NULL, "Dynamic pool grown to %d", pool->pool.free_count);
    return pool->pool.free_count ? er_ok : result;
}

//
// Create dynamic buffer using dynamic allocator
//
static void* prx_dynamic_buffer_alloc(
    prx_dynamic_pool_t* pool,
    void* original
)
{
    return prx_buffer_pool_alloc_buffer(&pool->pool, original, 
        (prx_buffer_pool_grow_t)prx_dynamic_pool_grow_no_lock, pool);
}

//
// Return buffer to pool
//
static void prx_dynamic_buffer_release(
    prx_dynamic_pool_t* pool,
    void* buffer
)
{
    prx_buffer_pool_free_buffer(&pool->pool, buffer);
}

//
// Resize dynamic buffer to length
//
static int32_t prx_dynamic_buffer_set_size(
    prx_dynamic_pool_t* pool,
    void** buffer,
    size_t size
)
{
    int32_t result;
    prx_buffer_t* buf, *orig;
    chk_arg_fault_return(buffer);
    chk_arg_fault_return(*buffer);

    orig = __prx_buffer(*buffer);
    dbg_assert_buf(orig);

    // Pointers will change after realloc, so remove from checked out list
    lock_enter(pool->pool.lock);
    DList_RemoveEntryList(&orig->link);
    dbg_assert_buf(orig);

    buf = (prx_buffer_t*)mem_realloc(orig, sizeof(prx_buffer_t) + size);
    if (buf)
    {
        // Then return to checked out list with new pointers
        DList_InsertTailList(&pool->pool.checked_out, &buf->link);
        buf->length = size;
        *buffer = __raw_buffer(buf);
        dbg_assert_buf(buf);
        result = er_ok;
    }
    else
    {
        // In case of error always return original to checked out list
        DList_InsertTailList(&pool->pool.checked_out, &orig->link);
        dbg_assert_buf(orig);
        result = er_out_of_memory;
    }
    lock_exit(pool->pool.lock);
    return result;
}

//
// Returns current size of dynamic buffer
//
static size_t prx_dynamic_buffer_get_size(
    prx_dynamic_pool_t* pool,
    void* buffer
)
{
    (void)pool;
    prx_buffer_t* pool_buf = __prx_buffer(buffer);
    dbg_assert_buf(pool_buf);
    return pool_buf->length;
}

//
// Returns how many buffers can be created
//
static size_t prx_fixed_pool_available(
    prx_fixed_pool_t* pool
)
{
    return pool->pool.free_count; // TODO
}

//
// Release fixed pool and free all block allocations
//
static void prx_fixed_pool_free(
    prx_fixed_pool_t* pool
)
{
    prx_buffer_t* next;
    dbg_assert_ptr(pool);

    if (pool->pool.lock)
    {
        lock_enter(pool->pool.lock);

        dbg_assert(DList_IsListEmpty(&pool->pool.checked_out),
            "Leaking buffer that was not returned to pool!");

        DList_InitializeListHead(&pool->pool.checked_out);
        DList_InitializeListHead(&pool->pool.free_list);
        while (!DList_IsListEmpty(&pool->blocks))
        {
            next = containingRecord(
                DList_RemoveHeadList(&pool->blocks), prx_buffer_t, link);
            dbg_assert_buf(next);
            mem_free(next);
        }

        lock_exit(pool->pool.lock);
    }

    prx_buffer_pool_deinit(&pool->pool);
    mem_free_type(prx_fixed_pool_t, pool);
}

//
// Grow fixed pool by adding another block
//
static int32_t prx_fixed_pool_grow_no_lock(
    prx_fixed_pool_t* pool
)
{
    prx_buffer_t* buf;
    size_t items, size;

    items = pool->pool.config.initial_count;
    if (pool->pool.config.max_count)
        items = min(pool->pool.config.max_count - pool->pool.count, items);
    if (!items)
        return er_out_of_memory;  // Reached our max

    // Allocate block
    size = sizeof(prx_buffer_t) + pool->pool.item_size;
    buf = (prx_buffer_t*)mem_alloc(
        sizeof(prx_buffer_t) + (items * size));
    if (!buf)
        return er_out_of_memory;

    // Add block to block list
    memset(buf, 0, sizeof(prx_buffer_t));
    buf->length = sizeof(prx_buffer_t) + (items * size);
#ifdef DBG_MEM
    buf->sentinel = _sentinel;
#endif
    DList_InsertTailList(&pool->blocks, &buf->link);
   
    // Initialize all buffers in block
    buf = (prx_buffer_t*)__raw_buffer(buf);

    for (size_t i = 0; i < items; i++)
    {
        DList_InitializeListHead(&buf->link);
#ifdef DBG_MEM
        buf->sentinel = _sentinel;
#endif
        buf->refs = 0;  // Starts at 0, increased on checkout.
        buf->length = pool->pool.item_size;

        DList_InsertTailList(&pool->pool.free_list, &buf->link);
        pool->pool.count++;
        pool->pool.free_count++;

        // next...
        buf = (prx_buffer_t*)(((int8_t*)buf) + size);
    }
    
    // log_debug(NULL, "Fixed pool grown to %d", pool->pool.free_count);
    return er_ok;
}

//
// Create fixed buffer using block allocator
//
static void* prx_fixed_buffer_alloc(
    prx_fixed_pool_t* pool,
    void* original
)
{
    return prx_buffer_pool_alloc_buffer(&pool->pool, original, 
        (prx_buffer_pool_grow_t)prx_fixed_pool_grow_no_lock, pool);
}

//
// Return buffer to pool
//
static void prx_fixed_buffer_release(
    prx_fixed_pool_t* pool,
    void* buffer
)
{
    prx_buffer_pool_free_buffer(&pool->pool, buffer);

    // TODO: Check to see if we can compact by removing blocks
}

//
// Fixed buffers cannot be resized
//
static int32_t prx_fixed_buffer_set_size(
    prx_fixed_pool_t* pool,
    void** buffer,
    size_t size
)
{
    (void)pool;
    (void)buffer;
    (void)size;

    dbg_assert(0, "Unsupported");
    return er_not_supported;
}

//
// Returns raw memory pointer 
//
static size_t prx_fixed_buffer_get_size(
    prx_fixed_pool_t* pool,
    void* buffer
)
{
    (void)buffer;
    return pool->pool.item_size;
}

//
// Creates a dynamic buffer pool 
//
int32_t prx_dynamic_pool_create(
    const char* name,
    size_t item_size,
    prx_pool_config_t* config,
    prx_buffer_factory_t** buffer_factory
)
{
    prx_dynamic_pool_t* pool;
    int32_t result;

    dbg_assert_ptr(buffer_factory);

    pool = mem_zalloc_type(prx_dynamic_pool_t);
    if (!pool)
        return er_out_of_memory;
    do
    {
        result = prx_buffer_pool_init(
            &pool->pool, name, item_size, config);
        if (result != er_ok)
            break;

        result = prx_dynamic_pool_grow_no_lock(pool);
        if (result != er_ok)
            break;

        pool->funcs.context = 
            pool;
        pool->funcs.on_available = (prx_buffer_factory_available_t)
            prx_dynamic_pool_available;
        pool->funcs.on_free = (prx_buffer_factory_free_t)
            prx_dynamic_pool_free;
        pool->funcs.on_alloc = (prx_buffer_alloc_t)
            prx_dynamic_buffer_alloc;
        pool->funcs.on_set_size = (prx_buffer_set_size_t)
            prx_dynamic_buffer_set_size;
        pool->funcs.on_get_size = (prx_buffer_get_size_t)
            prx_dynamic_buffer_get_size;
        pool->funcs.on_release = (prx_buffer_free_t)
            prx_dynamic_buffer_release;

        *buffer_factory = &pool->funcs;
        return er_ok;
    } while (0);

    prx_dynamic_pool_free(pool);
    return result;
}

//
// Creates a fixed buffer pool 
//
int32_t prx_fixed_pool_create(
    const char* name,
    size_t item_size,
    prx_pool_config_t* config,
    prx_buffer_factory_t** buffer_factory
)
{
    prx_fixed_pool_t* pool;
    int32_t result;

    dbg_assert_ptr(buffer_factory);

    pool = mem_zalloc_type(prx_fixed_pool_t);
    if (!pool)
        return er_out_of_memory;
    do
    {
        DList_InitializeListHead(&pool->blocks);
        result = prx_buffer_pool_init(
            &pool->pool, name, item_size, config);
        if (result != er_ok)
            break;

        result = prx_fixed_pool_grow_no_lock(pool);
        if (result != er_ok)
            break;

        pool->funcs.context =
            pool;
        pool->funcs.on_available = (prx_buffer_factory_available_t)
            prx_fixed_pool_available;
        pool->funcs.on_free = (prx_buffer_factory_free_t)
            prx_fixed_pool_free;
        pool->funcs.on_alloc = (prx_buffer_alloc_t)
            prx_fixed_buffer_alloc;
        pool->funcs.on_set_size = (prx_buffer_set_size_t)
            prx_fixed_buffer_set_size;
        pool->funcs.on_get_size = (prx_buffer_get_size_t)
            prx_fixed_buffer_get_size;
        pool->funcs.on_release = (prx_buffer_free_t)
            prx_fixed_buffer_release;

        *buffer_factory = &pool->funcs;
        return er_ok;
    } while (0);

    prx_fixed_pool_free(pool);
    return result;
}

//
// Allocate buffer with size
//
void* prx_buffer_new(
    prx_buffer_factory_t* buffer_factory,
    size_t length
)
{
    int32_t result;
    void* buffer = prx_buffer_alloc(buffer_factory, NULL);
    if (buffer)
    {
        result = prx_buffer_set_size(buffer_factory, &buffer, length);
        if (result == er_ok)
            return buffer;

        // Probably not a dynamic pool...
        prx_buffer_release(buffer_factory, buffer);
    }
    return NULL;
}
