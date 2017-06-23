// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _prx_ns_h_
#define _prx_ns_h_

#include "common.h"
#include "io_ref.h"
#include "io_cs.h"

//
// Represents a location where hosts and proxies are looked up from
// 
typedef struct prx_ns prx_ns_t;

//
// Represents an iterable result set
//
typedef struct prx_ns_result prx_ns_result_t;

// 
// Type of registry entry
//
typedef enum prx_ns_entry_type
{
    prx_ns_entry_type_hub = 0x0,
    prx_ns_entry_type_host = 0x1,
    prx_ns_entry_type_proxy = 0x2,
    prx_ns_entry_type_startup = 0x4,
    prx_ns_entry_type_link = 0x8,
    prx_ns_entry_query_all = 0xf
}
prx_ns_entry_type_t;

//
// Represents an entry in the registry
//
typedef struct prx_ns_entry prx_ns_entry_t;

//
// Create hub registry with connection string
//
decl_public_2(int32_t, prx_ns_iot_hub_create_from_cs,
    io_cs_t*, hub_cs,
    prx_ns_t**, created
);

//
// Create registry from config file or connection string
//
decl_public_2(int32_t, prx_ns_iot_hub_create,
    const char*, config,
    prx_ns_t**, created
);

//
// Create generic registry, optionally populated from file
//
decl_public_2(int32_t, prx_ns_generic_create,
    const char*, file_name,
    prx_ns_t**, created
);

//
// Provider to lookup entry by unique address
//
typedef int32_t (*prx_ns_get_entry_by_addr_t)(
    void* context,
    io_ref_t* address,
    prx_ns_entry_t** entry
    );

//
// Provider to lookup entries with a particular name
//
typedef int32_t (*prx_ns_get_entry_by_name_t)(
    void* context,
    const char* name,
    prx_ns_result_t** list
    );

//
// Provider to lookup entries of particular type
//
typedef int32_t (*prx_ns_get_entry_by_type_t)(
    void* context,
    uint32_t type,
    prx_ns_result_t** list
    );

//
// Create a new entry in the database
//
typedef int32_t (*prx_ns_create_entry_t)(
    void* context,
    prx_ns_entry_t* entry  
    );

//
// Update existing entry in database from entry
//
typedef int32_t (*prx_ns_update_entry_t)(
    void* context,
    prx_ns_entry_t* entry
    );

//
// Remove entry from database
//
typedef int32_t(*prx_ns_remove_entry_t)(
    void* context,
    prx_ns_entry_t* entry
    );

//
// Closes name service database
//
typedef void (*prx_ns_close_t)(
    void* context
    );

//
// Database interface
// 
struct prx_ns
{
    void* context;
    prx_ns_create_entry_t create;
    prx_ns_get_entry_by_addr_t get_by_addr;
    prx_ns_get_entry_by_name_t get_by_name;
    prx_ns_get_entry_by_type_t get_by_type;
    prx_ns_update_entry_t update;
    prx_ns_remove_entry_t remove;
    prx_ns_close_t close;
};

//
// Get entry by address
//
decl_inline_3(int32_t, prx_ns_get_entry_by_addr,
    prx_ns_t*, ns,
    io_ref_t*, address,
    prx_ns_entry_t**, entry
)
{
    chk_arg_fault_return(ns);
    chk_arg_fault_return(address);
    chk_arg_fault_return(entry);
    dbg_assert_ptr(ns->get_by_addr);
    return ns->get_by_addr(ns->context, address, entry);
}

//
// Get entry by name
//
decl_inline_3(int32_t, prx_ns_get_entry_by_name,
    prx_ns_t*, ns,
    const char*, name,
    prx_ns_result_t**, results
)
{
    chk_arg_fault_return(ns);
    chk_arg_fault_return(name);
    chk_arg_fault_return(results);
    dbg_assert_ptr(ns->get_by_name);
    return ns->get_by_name(ns->context, name, results);
}

//
// Get entry by type
//
decl_inline_3(int32_t, prx_ns_get_entry_by_type,
    prx_ns_t*, ns,
    uint32_t, type,
    prx_ns_result_t**, results
)
{
    chk_arg_fault_return(ns);
    chk_arg_fault_return(results);
    dbg_assert_ptr(ns->get_by_type);
    return ns->get_by_type(ns->context, type, results);
}

//
// Create entry in database
//
decl_inline_2(int32_t, prx_ns_create_entry,
    prx_ns_t*, ns,
    prx_ns_entry_t*, entry
)
{
    chk_arg_fault_return(ns);
    chk_arg_fault_return(entry);
    dbg_assert_ptr(ns->create);
    return ns->create(ns->context, entry);
}

//
// Update entry in database
//
decl_inline_2(int32_t, prx_ns_update_entry,
    prx_ns_t*, ns,
    prx_ns_entry_t*, entry
)
{
    chk_arg_fault_return(ns);
    chk_arg_fault_return(entry);
    dbg_assert_ptr(ns->update);
    return ns->update(ns->context, entry);
}

//
// Remove entry from database
//
decl_inline_2(int32_t, prx_ns_remove_entry,
    prx_ns_t*, ns,
    prx_ns_entry_t*, entry
)
{
    chk_arg_fault_return(ns);
    chk_arg_fault_return(entry);
    dbg_assert_ptr(ns->remove);
    return ns->remove(ns->context, entry);
}

//
// Free database object
//
decl_inline_1(void, prx_ns_close,
    prx_ns_t*, ns
)
{
    if (!ns)
        return;
    dbg_assert_ptr(ns->close);
    ns->close(ns->context);
}

//
// Return size of result set
//
typedef size_t(*prx_ns_result_size_t)(
    void* context
    );

//
// Get next entry in the result set
//
typedef prx_ns_entry_t* (*prx_ns_result_pop_t)(
    void* context
    );

//
// Release the result set
//
typedef void(*prx_ns_result_release_t)(
    void* context
    );

//
// Result set interface for query results
// 
struct prx_ns_result
{
    void* context;
    prx_ns_result_size_t size;
    prx_ns_result_pop_t pop;
    prx_ns_result_release_t release;
};

//
// Return size of the returned result set
//
decl_inline_1(size_t, prx_ns_result_size,
    prx_ns_result_t*, results
)
{
    if (!results)
        return 0;
    dbg_assert_ptr(results->size);
    return results->size(results->context);
}

//
// Get next entry in the result set
//
decl_inline_1(prx_ns_entry_t*, prx_ns_result_pop,
    prx_ns_result_t*, results
)
{
    if (!results)
        return NULL;
    dbg_assert_ptr(results->pop);
    return results->pop(results->context);
}

//
// Free the result set
//
decl_inline_1(void, prx_ns_result_release,
    prx_ns_result_t*, results
)
{
    if (!results)
        return;
    dbg_assert_ptr(results->release);
    results->release(results->context);
}

//
// Provider to clone entry
//
typedef int32_t (*prx_ns_entry_clone_t)(
    void* context,
    prx_ns_entry_t**
    );

//
// Provider to supply primary key in entry's database
//
typedef const char* (*prx_ns_entry_get_id_t)(
    void* context
    );

//
// Provider to return name of entry
//
typedef const char* (*prx_ns_entry_get_name_t)(
    void* context
    );

//
// Provider to return type of entry
//
typedef uint32_t (*prx_ns_entry_get_type_t)(
    void* context
    );

//
// Provider to return index of entry
//
typedef int32_t (*prx_ns_entry_get_index_t)(
    void* context
    );

//
// Provider to return version of the entry
//
typedef uint32_t (*prx_ns_entry_get_version_t)(
    void* context
    );

//
// Provider to return unique address of entry
//
typedef int32_t (*prx_ns_entry_get_addr_t)(
    void* context,
    io_ref_t*
    );

//
// Returns a connection string from an entry
//
typedef int32_t(*prx_ns_entry_get_cs_t)(
    void* context,
    io_cs_t** cs
    );

//
// Provider to return links for this entry
//
typedef int32_t (*prx_ns_entry_get_links_t)(
    void* context,
    prx_ns_result_t** results
    );

//
// Provider to release entry
//
typedef void (*prx_ns_entry_release_t)(
    void* context
    );

//
// Provider interface for a database entry or record
//
struct prx_ns_entry
{
    void* context;
    prx_ns_entry_clone_t clone;
    prx_ns_entry_get_cs_t get_cs;
    prx_ns_entry_get_type_t get_type;
    prx_ns_entry_get_id_t get_id;
    prx_ns_entry_get_version_t get_version;
    prx_ns_entry_get_name_t get_name;
    prx_ns_entry_get_index_t get_index;
    prx_ns_entry_get_addr_t get_addr;
    prx_ns_entry_get_links_t get_links;
    prx_ns_entry_release_t release;
};

//
// Clones entry
//
decl_inline_2(int32_t, prx_ns_entry_clone,
    prx_ns_entry_t*, entry,
    prx_ns_entry_t**, clone
)
{
    chk_arg_fault_return(entry);
    dbg_assert_ptr(entry->clone);
    return entry->clone(entry->context, clone);
}

//
// Returns a connection string from an entry
//
decl_inline_2(int32_t, prx_ns_entry_get_cs,
    prx_ns_entry_t*, entry,
    io_cs_t**, cs
)
{
    chk_arg_fault_return(entry);
    dbg_assert_ptr(entry->get_cs);
    return entry->get_cs(entry->context, cs);
}

//
// Returns primary key of entry
//
decl_inline_1(const char*, prx_ns_entry_get_id,
    prx_ns_entry_t*, entry
)
{
    if (!entry)
        return NULL;
    dbg_assert_ptr(entry->get_id);
    return entry->get_id(entry->context);
}

//
// Returns name of entry
//
decl_inline_1(const char*, prx_ns_entry_get_name,
    prx_ns_entry_t*, entry
)
{
    if (!entry)
        return NULL;
    dbg_assert_ptr(entry->get_name);
    return entry->get_name(entry->context);
}

//
// Returns type of entry
//
decl_inline_1(uint32_t, prx_ns_entry_get_type,
    prx_ns_entry_t*, entry
)
{
    if (!entry)
        return 0;
    dbg_assert_ptr(entry->get_type);
    return entry->get_type(entry->context);
}

//
// Returns index of entry
//
decl_inline_1(int32_t, prx_ns_entry_get_index,
    prx_ns_entry_t*, entry
)
{
    if (!entry)
        return 0;
    dbg_assert_ptr(entry->get_index);
    return entry->get_index(entry->context);
}

//
// Returns version of entry
//
decl_inline_1(uint32_t, prx_ns_entry_get_version,
    prx_ns_entry_t*, entry
)
{
    if (!entry)
        return 0;
    dbg_assert_ptr(entry->get_version);
    return entry->get_version(entry->context);
}

//
// Returns unique address of entry
//
decl_inline_2(int32_t, prx_ns_entry_get_addr,
    prx_ns_entry_t*, entry,
    io_ref_t*, address
)
{
    chk_arg_fault_return(entry);
    chk_arg_fault_return(address);
    dbg_assert_ptr(entry->get_addr);
    return entry->get_addr(entry->context, address);
}

//
// Get link entries
//
decl_inline_2(int32_t, prx_ns_entry_get_links,
    prx_ns_entry_t*, entry,
    prx_ns_result_t**, results
)
{
    chk_arg_fault_return(entry);
    dbg_assert_ptr(entry->get_links);
    return entry->get_links(entry->context, results);
}

//
// Converts an entry to socket address
//
decl_internal_3(int32_t, prx_ns_entry_to_prx_socket_address,
    prx_ns_entry_t*, entry,
    prx_address_family_t, family,
    prx_socket_address_t*, socket_address
);

//
// Create in memory entry 
//
decl_internal_5(int32_t, prx_ns_entry_create,
    uint32_t, type,
    const char*, id,
    const char*, name,
    uint32_t, version,
    prx_ns_entry_t**, entry
);

//
// Create in memory entry from connection string
//
decl_internal_4(int32_t, prx_ns_entry_create_from_cs,
    uint32_t, type,
    io_ref_t*, address,
    io_cs_t*, cs,
    prx_ns_entry_t**, entry
);

//
// Create entry from a json configuration string
//
decl_internal_2(int32_t, prx_ns_entry_create_from_string,
    const char*, string,
    prx_ns_entry_t**, entry
);

//
// Serialize entry to json configuration string
//
decl_internal_2(int32_t, prx_ns_entry_to_STRING,
    prx_ns_entry_t*, entry,
    STRING_HANDLE*, string
);

//
// Free entry
//
decl_inline_1(void, prx_ns_entry_release,
    prx_ns_entry_t*, entry
)
{
    dbg_assert_ptr(entry);
    dbg_assert_ptr(entry->release);
    entry->release(entry->context);
}


#endif // _prx_ns_h_

