// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "io_url.h"
#include "util_string.h"

//
// Returns true if both urls are logically equal
//
bool io_url_equals(
    io_url_t* that,
    io_url_t* other
)
{
    if (!that && !other)
        return true;
    if (!that || !other)
        return false;
    if (that->port != other->port)
        return false;
    if (that->scheme && other->scheme)
    {
        if (0 != STRING_compare_nocase(that->scheme, other->scheme))
            return false;
    }
    else if (that->scheme != other->scheme)
        return false;
    if (that->host_name && other->host_name)
    {
        if (0 != STRING_compare_nocase(that->host_name, other->host_name))
            return false;
    }
    else if (that->host_name != other->host_name)
        return false;
    if (that->path && other->path)
    {
        if (0 != STRING_compare_nocase(that->path, other->path))
            return false;
    }
    else if (that->path != other->path)
        return false;
    if (that->user_name && other->user_name)
    {
        if (0 != STRING_compare_nocase(that->user_name, other->user_name))
            return false;
    }
    else if (that->user_name != other->user_name)
        return false;
    if (that->password && other->password)
    {
        if (0 != STRING_compare_nocase(that->password, other->password))
            return false;
    }
    else if (that->password != other->password)
        return false;
    return io_token_provider_is_equivalent(that->token_provider, other->token_provider);
}

//
// Free address
//
void io_url_free(
    io_url_t* address
)
{
    if (!address)
        return;

    if (address->scheme)
        STRING_delete(address->scheme);
    if (address->host_name)
        STRING_delete(address->host_name);
    if (address->path)
        STRING_delete(address->path);
    if (address->user_name)
        STRING_delete(address->user_name);
    if (address->password)
        STRING_delete(address->password);
    if (address->token_provider)
        io_token_provider_release(address->token_provider);

    mem_free_type(io_url_t, address);
}

//
// Creates an address 
//
int32_t io_url_create(
    const char* scheme,
    const char* host_name,
    uint16_t port,
    const char* path,
    const char* user_name,
    const char* password,
    io_url_t** created
)
{
    int32_t result = er_out_of_memory;
    io_url_t* address;

    chk_arg_fault_return(host_name);
    chk_arg_fault_return(created);

    address = mem_zalloc_type(io_url_t);
    if (!address)
        return er_out_of_memory;
    do
    {
        address->host_name = STRING_construct(host_name);
        address->port = port;

        if (!address->host_name)
            break;

        if (scheme)
        {
            if (!address->port)
            {
                /**/ if (0 == string_compare_nocase(scheme, "wss"))
                    address->port = 443;
                else if (0 == string_compare_nocase(scheme, "amqps"))
                    address->port = 5671;
                else if (0 == string_compare_nocase(scheme, "https"))
                    address->port = 443;
                else if (0 == string_compare_nocase(scheme, "mqtts"))
                    address->port = 8883;
                else
                {
                    result = er_invalid_format;
                    break;
                }
            }
            address->scheme = STRING_construct(scheme);
            if (!address->scheme)
                break;
        }

        if (path)
        {
            address->path = STRING_construct(path);
            if (!address->path)
                break;
        }

        if (user_name)
        {
            address->user_name = STRING_construct(user_name);
            if (!address->user_name)
                break;
        }

        if (password)
        {
            address->password = STRING_construct(password);
            if (!address->password)
                break;
        }

        *created = address;
        return er_ok;

    } while (0);
    
    io_url_free(address);
    return result;
}

//
// Make a clone of the address configuration
//
int32_t io_url_clone(
    io_url_t* address,
    io_url_t** cloned
)
{
    int32_t result;
    io_url_t* copy;

    chk_arg_fault_return(address);
    chk_arg_fault_return(cloned);

    copy = mem_zalloc_type(io_url_t);
    if (!copy)
        return er_out_of_memory;
    do
    {
        result = er_out_of_memory;
        copy->port = address->port;

        if (address->scheme)
            copy->scheme = STRING_clone(address->scheme);
        if (address->host_name)
            copy->host_name = STRING_clone(address->host_name);
        if (address->path)
            copy->path = STRING_clone(address->path);
        if (address->user_name)
            copy->user_name = STRING_clone(address->user_name);
        if (address->password)
            copy->password = STRING_clone(address->password);

        if ((address->scheme && !copy->scheme) ||
            (address->host_name && !copy->host_name) ||
            (address->path && !copy->path) ||
            (address->user_name && !copy->user_name) ||
            (address->password && !copy->password))
            break;

        if (address->token_provider)
        {
            result = io_token_provider_clone(
                address->token_provider, &copy->token_provider);
            if (result != er_ok)
                break;
        }

        *cloned = copy;
        return er_ok;

    } while (0);

    io_url_free(copy);
    return result;
}
