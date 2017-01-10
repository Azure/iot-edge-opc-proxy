// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "util_handle.h"
#include "pal_mt.h"
#include "azure_c_shared_utility/doublylinkedlist.h"

//
// Storage of handles
//
typedef struct handle
{
    int32_t id;
    const void* pointer;
    DLIST_ENTRY link;
}
handle_t;

//
// Handle map
//
typedef struct handle_map
{
    lock_t lock;
    DLIST_ENTRY handles;
    int32_t last_id;
}
handle_map_t;


static handle_map_t* handle_map;


//
// Free handle
//
static void handle_free(
    handle_t* handle
)
{
    if (!handle)
        return;

    mem_free_type(handle_t, handle);
}

//
// Create handle
//
static int32_t handle_create(
    int32_t id,
    const void* pointer,
    handle_t** created
)
{
    handle_t* handle;
    dbg_assert(id != handle_map_invalid_handle, "Invalid Argument");

    handle = mem_zalloc_type(handle_t);
    if (!handle)
        return er_out_of_memory;

    handle->id = id;
    handle->pointer = pointer;
    *created = handle;
    return er_ok;
}

//
// Frees global handle table
//
void handle_map_deinit(
    void
)
{
    handle_t* handle = NULL;
    if (!handle_map)
        return;

    if (handle_map->lock)
        lock_enter(handle_map->lock);

    while (!DList_IsListEmpty(&handle_map->handles))
    {
        handle = containingRecord(DList_RemoveHeadList(&handle_map->handles), handle_t, link);
		dbg_assert(0, "Open handle %d (%p) not freed", handle->id, handle->pointer);
    }

    if (handle_map->lock)
    {
        lock_exit(handle_map->lock);
        lock_free(handle_map->lock);
    }

    mem_free_type(handle_map_t, handle_map);
    handle_map = NULL;
}

//
// Create a global handle map
//
int32_t handle_map_init(
    void
)
{
    int32_t result;

    if (handle_map)
        return er_ok;

    handle_map = mem_zalloc_type(handle_map_t);
    if (!handle_map)
        return er_out_of_memory;
    do
    {
        DList_InitializeListHead(&handle_map->handles);
        result = lock_create(&handle_map->lock);
        if (result != er_ok)
            break;
        return er_ok;
    } while (0);

    handle_map_deinit();
    return result;
}

//
// Returns a new handle for a pointer
//
int32_t handle_map_get_handle(
    const void* pointer
)
{
    int32_t result;
    handle_t* handle;
    int32_t id;

    if (!pointer || !handle_map)
        return handle_map_invalid_handle;

    lock_enter(handle_map->lock);
    do
    { 
        id = ++handle_map->last_id;
    } 
    while (id == handle_map_invalid_handle);

    result = handle_create(id, pointer, &handle);
    if (result == er_ok)
        DList_InsertTailList(&handle_map->handles, &handle->link);
    lock_exit(handle_map->lock);

    return result == er_ok ? handle->id : handle_map_invalid_handle;
}

//
// Returns a pointer for handle
//
const void* handle_map_get_pointer(
    int32_t id
)
{
    handle_t* handle;

    if (!handle_map || id == handle_map_invalid_handle)
        return NULL;

    lock_enter(handle_map->lock);
    for (PDLIST_ENTRY p = handle_map->handles.Flink; p != &handle_map->handles; p = p->Flink)
    {
        handle = containingRecord(p, handle_t, link);
        if (id == handle->id)
        {
            lock_exit(handle_map->lock);
            return handle->pointer;
        }
    }
    lock_exit(handle_map->lock);
    return NULL;
}

//
// Removes a handle from the map
//
const void* handle_map_remove_handle(
    int32_t id
)
{
    handle_t* handle;
    const void* pointer;

    if (!handle_map || id == handle_map_invalid_handle)
        return NULL;

    lock_enter(handle_map->lock);
    for (PDLIST_ENTRY p = handle_map->handles.Flink; p != &handle_map->handles; p = p->Flink)
    {
        handle = containingRecord(p, handle_t, link);
        if (id == handle->id)
        {
            DList_RemoveEntryList(&handle->link);
            lock_exit(handle_map->lock);

            pointer = handle->pointer;
            handle_free(handle);
            return pointer;
        }
    }
    lock_exit(handle_map->lock);
    return NULL;
}
