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
// Free url
//
void io_url_free(
    io_url_t* url
)
{
    if (!url)
        return;

    if (url->scheme)
        STRING_delete(url->scheme);
    if (url->host_name)
        STRING_delete(url->host_name);
    if (url->path)
        STRING_delete(url->path);
    if (url->user_name)
        STRING_delete(url->user_name);
    if (url->password)
        STRING_delete(url->password);
    if (url->token_provider)
        io_token_provider_release(url->token_provider);

    mem_free_type(io_url_t, url);
}

//
// Creates an url from a url string
//
int32_t io_url_parse(
    const char* string,
    io_url_t** created
)
{
    int32_t result;
    char* copy, *ptr;
    const char* scheme, *host_name, *path = NULL, *port_str = NULL;
    const char* user_name = NULL, *password = NULL;

    chk_arg_fault_return(string);
    chk_arg_fault_return(created);

    // Make a working copy
    result = string_clone(string, &copy);
    if (result != er_ok)
        return result;
    result = er_invalid_format;
    do
    {
        // Todo: Support parsing username:password@hostname:port

        // Parse scheme
        scheme = ptr = copy;
        while (*ptr && *ptr != ':') ptr++;
        if (!*ptr) break;
        *ptr++ = 0;
        if (*ptr++ != '/' || *ptr++ != '/') break;

        // Parse host name
        host_name = ptr;
        while (*ptr && *ptr != ':' && *ptr != '/') ptr++;
        /**/ if (*ptr == ':')
        {
            *ptr++ = 0;
            // Parse port
            port_str = ptr;
            while (*ptr && *ptr != '/') ptr++;
        }
        else if (*ptr == '/')
            ;
        else
            break;
        *ptr++ = 0;
        if (*ptr)
        {
            // Remainder is path
            path = ptr;
        }
        result = io_url_create(scheme, host_name, port_str ? 
            (uint16_t)atoi(port_str) : 0, path, user_name, password, created);
    } 
    while (0);
    mem_free(copy);
    return result;
}

//
// Creates a url 
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
    io_url_t* url;

    chk_arg_fault_return(host_name);
    chk_arg_fault_return(created);

    url = mem_zalloc_type(io_url_t);
    if (!url)
        return er_out_of_memory;
    do
    {
        url->host_name = STRING_construct(host_name);
        url->port = port;

        if (!url->host_name)
            break;

        if (scheme)
        {
            if (!url->port)
            {
                /**/ if (0 == string_compare_nocase(scheme, "wss"))
                    url->port = 443;
                else if (0 == string_compare_nocase(scheme, "amqps"))
                    url->port = 5671;
                else if (0 == string_compare_nocase(scheme, "https"))
                    url->port = 443;
                else if (0 == string_compare_nocase(scheme, "mqtts"))
                    url->port = 8883;
                else
                {
                    result = er_invalid_format;
                    break;
                }
            }
            url->scheme = STRING_construct(scheme);
            if (!url->scheme)
                break;
        }

        if (path)
        {
            url->path = STRING_construct(path);
            if (!url->path)
                break;
        }

        if (user_name)
        {
            url->user_name = STRING_construct(user_name);
            if (!url->user_name)
                break;
        }

        if (password)
        {
            url->password = STRING_construct(password);
            if (!url->password)
                break;
        }

        *created = url;
        return er_ok;

    } while (0);
    
    io_url_free(url);
    return result;
}

//
// Make a clone of the url configuration
//
int32_t io_url_clone(
    io_url_t* url,
    io_url_t** cloned
)
{
    int32_t result;
    io_url_t* copy;

    chk_arg_fault_return(url);
    chk_arg_fault_return(cloned);

    copy = mem_zalloc_type(io_url_t);
    if (!copy)
        return er_out_of_memory;
    do
    {
        result = er_out_of_memory;
        copy->port = url->port;

        if (url->scheme)
            copy->scheme = STRING_clone(url->scheme);
        if (url->host_name)
            copy->host_name = STRING_clone(url->host_name);
        if (url->path)
            copy->path = STRING_clone(url->path);
        if (url->user_name)
            copy->user_name = STRING_clone(url->user_name);
        if (url->password)
            copy->password = STRING_clone(url->password);

        if ((url->scheme && !copy->scheme) ||
            (url->host_name && !copy->host_name) ||
            (url->path && !copy->path) ||
            (url->user_name && !copy->user_name) ||
            (url->password && !copy->password))
            break;

        if (url->token_provider)
        {
            result = io_token_provider_clone(
                url->token_provider, &copy->token_provider);
            if (result != er_ok)
                break;
        }

        *cloned = copy;
        return er_ok;

    } while (0);

    io_url_free(copy);
    return result;
}
