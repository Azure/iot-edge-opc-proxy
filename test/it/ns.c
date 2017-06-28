// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "prx_log.h"
#include "prx_ns.h"
#include "util_string.h"
#include <stdio.h>

static log_t cat;

//
// Name service registry add
//
int32_t ns_add(
    prx_ns_t* ns,
    uint32_t type,
    const char* id,
    const char* name
)
{
    int32_t result;
    prx_ns_entry_t* entry;
    
    result = prx_ns_entry_create(type, id, name, MODULE_VER_NUM, &entry);
    if (result != er_ok)
        return result;
    result = prx_ns_create_entry(ns, entry);
    prx_ns_entry_release(entry);
    return result;
}

//
// List entries of given type or types
//
int32_t ns_list_by_type(
    prx_ns_t* ns,
    uint32_t type
)
{
    int32_t result;
    int32_t index = 0;
    prx_ns_entry_t* entry;
    prx_ns_result_t* resultset;

    result = prx_ns_get_entry_by_type(ns, type, &resultset);
    if (result != er_ok)
        return result;

    while ((entry = prx_ns_result_pop(resultset)) != NULL)
    {
        printf("Entry %d - Id: %s Name: %s Type: %d Index: %d\n", ++index, 
            prx_ns_entry_get_id(entry), prx_ns_entry_get_name(entry),
            prx_ns_entry_get_type(entry), prx_ns_entry_get_index(entry));
        prx_ns_entry_release(entry);
    }
    prx_ns_result_release(resultset);
    return result;
}

//
// Print all entries with given name
//
int32_t ns_list_by_name(
    prx_ns_t* ns,
    const char* name
)
{
    int32_t result;
    int32_t index = 0;
    prx_ns_entry_t* entry;
    prx_ns_result_t* resultset;

    result = prx_ns_get_entry_by_name(ns, name, &resultset);
    if (result != er_ok)
        return result;

    while ((entry = prx_ns_result_pop(resultset)) != NULL)
    {
        printf("Entry %d - Id: %s Name: %s Type: %d Index: %d\n", ++index,
            prx_ns_entry_get_id(entry), prx_ns_entry_get_name(entry),
            prx_ns_entry_get_type(entry), prx_ns_entry_get_index(entry));
        prx_ns_entry_release(entry);
    }
    prx_ns_result_release(resultset);
    return result;
}

//
// Name service registry remove
//
int32_t ns_remove(
    prx_ns_t* ns,
    const char* name
)
{
    int32_t result;
    prx_ns_entry_t* entry;
    prx_ns_result_t* resultset;

    result = prx_ns_get_entry_by_name(ns, name, &resultset);
    if (result != er_ok)
        return result;
    dbg_assert(prx_ns_result_size(resultset) == 1, "");
    entry = prx_ns_result_pop(resultset);
    result = prx_ns_remove_entry(ns, entry);
    prx_ns_entry_release(entry);
    return result;
}

//
// Name service registry empty all items
//
int32_t ns_clean(
    prx_ns_t* ns
)
{
    int32_t result;
    prx_ns_entry_t* entry;
    prx_ns_result_t* resultset;
    const char* name;

    result = prx_ns_get_entry_by_type(ns, 0xf, &resultset);
    if (result != er_ok)
        return result;
    while ((entry = prx_ns_result_pop(resultset)) != NULL)
    {
        name = prx_ns_entry_get_name(entry);
        if (strcmp(name, "host") && strcmp(name, "proxy") && strcmp(name, "link"))
            continue;

        result = prx_ns_remove_entry(ns, entry);
        if (result != er_ok)
        {
            log_error(cat, "Failed to remove entry %s (%s)",
                prx_ns_entry_get_id(entry), prx_err_string(result));
        }
        prx_ns_entry_release(entry);
    }
    prx_ns_result_release(resultset);
    return result;
}

//
// Name service registry test utility
//
int test_ns(
    void
)
{
    int32_t result;
    prx_ns_t* ns;
    io_cs_t* cs;
    char buffer[64];

    result = io_cs_create_from_string(getenv("_HUB_CS"), &cs);
    if (result != er_ok)
        return result;
    do
    {
        result = prx_ns_iot_hub_create_from_cs(cs, &ns);
        if (result != er_ok)
            break;

        // Clean database
        ns_clean(ns);

#define NUM_PROXYS 34
        log_trace(cat, "Creating %d proxy entries", NUM_PROXYS);
        for (int i = 0; i < NUM_PROXYS; i++)
        {
            strcpy(buffer, "proxy_");
            string_from_int(i, 10, buffer + 6, sizeof(buffer) - 6);

            result = ns_add(ns, prx_ns_entry_type_proxy, buffer, "proxy");
            if (result != er_ok)
            {
                log_error(cat, "Failed to add proxy entry %s (%s)",
                    buffer, prx_err_string(result));
                break;
            }
        }
#define NUM_HOSTS 46
        log_trace(cat, "Creating %d host entries", NUM_HOSTS);
        for (int i = 0; i < NUM_HOSTS; i++)
        {
            strcpy(buffer, "host_");
            string_from_int(i, 10, buffer + 5, sizeof(buffer) - 5);

            result = ns_add(ns, prx_ns_entry_type_host, buffer, "host");
            if (result != er_ok)
            {
                log_error(cat, "Failed to add proxy entry %s (%s)",
                    buffer, prx_err_string(result));
                break;
            }
        }
#define NUM_LINKS 65
        log_trace(cat, "Creating %d link entries", NUM_LINKS);
        for (int i = 0; i < NUM_LINKS; i++)
        {
            strcpy(buffer, "link_");
            string_from_int(i, 10, buffer + 5, sizeof(buffer) - 5);

            result = ns_add(ns, prx_ns_entry_type_link, buffer, "link");
            if (result != er_ok)
            {
                log_error(cat, "Failed to add proxy entry %s (%s)",
                    buffer, prx_err_string(result));
                break;
            }
        }

        // List all proxys && links
        result = ns_list_by_type(ns, prx_ns_entry_type_proxy | prx_ns_entry_type_link);
        if (result != er_ok)
        {
            log_error(cat, "Failed to list proxy entries (%s)",
                prx_err_string(result));
            break;
        }

        // List all hosts
        result = ns_list_by_name(ns, "host");
        if (result != er_ok)
        {
            log_error(cat, "Failed to list link entries (%s)",
                prx_err_string(result));
            break;
        }

    } while (0);

    // Clean database
    if (ns)
    {
        ns_clean(ns);
        prx_ns_close(ns);
    }

    io_cs_free(cs);

    return result;
}

//
// Name service registry test utility
//
int main_ns(int argc, char *argv[])
{
    int32_t result;

    (void)argc;
    (void)argv;

    cat = log_get("test.ns");

    result = test_ns();

    return result;
}
