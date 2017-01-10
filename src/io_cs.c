// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "util_string.h"
#include "io_cs.h"
#include "azure_c_shared_utility/sastoken.h"

//
// Items in spec
//
typedef enum io_cs_entry
{
    io_cs_entry_host_name,
    io_cs_entry_hub_name,
    io_cs_entry_device_id,
    io_cs_entry_shared_access_key_name,
    io_cs_entry_shared_access_key,
    io_cs_entry_endpoint,
    io_cs_entry_consumer_group,
    io_cs_entry_partition_count,
    io_cs_entry_entity,
    io_cs_entry_endpoint_name,
    io_cs_entry_shared_access_token,
    io_cs_entry_max
}
io_cs_entry_t;

//
// connection string
//
typedef struct io_cs
{
    STRING_HANDLE entries[io_cs_entry_max];
}
io_cs_struct_t;

//
// Validate host name
//
static int32_t io_cs_validate_host_name(
    const char* host_name
)
{
    size_t count;
    const char* end;
    bool char_found = false;

    count = strlen(host_name);
    end = strchr(host_name, '.');
    if (!end)
        return er_invalid_format;
    
    count = (size_t)(end - host_name);
    end++;

    // Hub name must be between 3-50 chars long 
    if (count < 3 || count > 50)
        return er_invalid_format;

    // cannot start with "-"
    if (host_name[0] == '-')
        return er_invalid_format;

    for (size_t index = 0; index < count; index++)
    {
        // ... containing only alpha numeric ASCI chars ...
        if (host_name[index] >= '0' && host_name[index] <= '9')
            continue;
        char_found = true;
        if (host_name[index] >= 'a' && host_name[index] <= 'z')
            continue;
        if (host_name[index] >= 'A' && host_name[index] <= 'Z')
            continue;
        if (host_name[index] == '-')
            continue;
        return er_invalid_format;
    }

    // Must not be a number
    if (!char_found)
        return er_invalid_format;

    return er_ok;
}

//
// Validate device id
//
static int32_t io_cs_validate_device_id(
    const char* device_id
)
{
    size_t i, count;

    // Not valid chars: °, ^, §, &, /, [, ], {, }, :, <, >, |, ~, ...
    const char valid_chars[] =
    {
        '-', ':', '.', '+', '%', '_', '#', '*', '?',
        '!', '(', ')', ',', '=', '@', ';', '$', '\''
    };

    count = strlen(device_id);
    // Validate string is < 128 chars and ...
    if (count >= 128)
        return er_invalid_format;
    for (size_t index = 0; index < count; index++)
    {
        // ... containing only alpha numeric ASCI chars ...
        if (device_id[index] >= 'a' && device_id[index] <= 'z')
            continue;
        if (device_id[index] >= 'A' && device_id[index] <= 'Z')
            continue;
        if (device_id[index] >= '0' && device_id[index] <= '9')
            continue;

        // ... or one of the valid chars above
        for (i = 0; i < sizeof(valid_chars); i++)
        {
            if (device_id[index] == valid_chars[i])
                break;
        }
        if (i == sizeof(valid_chars))
            return er_invalid_format;
    }
    return er_ok;
}

//
// Validate fqn
//
static int32_t io_cs_validate(
    io_cs_struct_t* cs
)
{
    int32_t result;
    const char* val;

    dbg_assert_ptr(cs);

    val = io_cs_get_host_name(cs);
    if (!val)
        return er_invalid_format;
    else
    {
        if (!strlen(val))
            return er_invalid_format;
        result = io_cs_validate_host_name(val);
        if (result != er_ok)
            return result;
    }

    if (cs->entries[io_cs_entry_device_id])
    {
        val = STRING_c_str(cs->entries[io_cs_entry_device_id]);
        if (!strlen(val))
            return er_invalid_format;
        result = io_cs_validate_device_id(val);
        if (result != er_ok)
            return result;
    }

    // Currently validation fails due to &skn for non iothub tokens
    else
    {
        // Todo: Validate non device id tokens as well...
        return er_ok;
    }

    if (cs->entries[io_cs_entry_shared_access_token])
    {
        if (!SASToken_Validate(cs->entries[io_cs_entry_shared_access_token]))
            return er_invalid_format;
    }
    return er_ok;
}

//
// Callback for connections string parser
//
static int32_t io_cs_parser_callback(
    void* ctx,
    const char* key,
    size_t key_len,
    const char* val,
    size_t val_len
)
{
    io_cs_struct_t* cs = (io_cs_struct_t*)ctx;
#define __io_cs_add_entry(t) { \
        if (cs->entries[t]) return er_invalid_format; \
        cs->entries[t] = STRING_safe_construct_n(val, val_len); \
        if (!cs->entries[t]) return er_out_of_memory; \
    }

    /**/ if (string_is_equal_nocase(key, key_len, "HostName"))
        __io_cs_add_entry(io_cs_entry_host_name)
    else if (string_is_equal_nocase(key, key_len, "DeviceId"))
        __io_cs_add_entry(io_cs_entry_device_id)
    else if (string_is_equal_nocase(key, key_len, "SharedAccessKeyName"))
        __io_cs_add_entry(io_cs_entry_shared_access_key_name)
    else if (string_is_equal_nocase(key, key_len, "SharedAccessKey"))
        __io_cs_add_entry(io_cs_entry_shared_access_key)
    else if (string_is_equal_nocase(key, key_len, "ConsumerGroup"))
        __io_cs_add_entry(io_cs_entry_consumer_group)
    else if (string_is_equal_nocase(key, key_len, "Partitions"))
        __io_cs_add_entry(io_cs_entry_partition_count)
    else if (string_is_equal_nocase(key, key_len, "Endpoint"))
        __io_cs_add_entry(io_cs_entry_endpoint)
    else if (string_is_equal_nocase(key, key_len, "EntityPath"))
        __io_cs_add_entry(io_cs_entry_entity)
    else if (string_is_equal_nocase(key, key_len, "EndpointName"))
        __io_cs_add_entry(io_cs_entry_endpoint_name)
    else if (string_is_equal_nocase(key, key_len, "SharedAccessToken"))
        __io_cs_add_entry(io_cs_entry_shared_access_token)
    else if (string_is_equal_nocase(key, key_len, "GatewayHostName"))
        return er_ok; // Not used
	else
        return er_invalid_format;
    return er_ok;
}

//
// Append an entry to the connection string string
//
static int32_t io_cs_append_entry_to_STRING(
    io_cs_t* cs,
    io_cs_entry_t entry,
    const char* name,
    STRING_HANDLE c_string
)
{
    if (!cs->entries[entry])
        return er_ok;

    if (STRING_length(c_string) > 0 &&
        0 != STRING_concat(c_string, ";"))
        return er_out_of_memory;

    if (0 != STRING_concat(c_string, name) ||
        0 != STRING_concat_with_STRING(
            c_string, cs->entries[entry]))
        return er_out_of_memory;

    return er_ok;
}

//
// Create connection string from cs
//
STRING_HANDLE io_cs_to_STRING(
    io_cs_t* cs
)
{
    STRING_HANDLE result;
    if (!cs)
        return NULL;
    result = STRING_new();
    if (!result)
        return NULL;
    if (er_ok == io_cs_append_to_STRING(cs, result))
        return result;
    STRING_delete(result);
    return NULL;
}

//
// Create connection string from cs
//
int32_t io_cs_append_to_STRING(
    io_cs_t* cs,
    STRING_HANDLE c_string
)
{
    int32_t result;

    if (!cs || !c_string)
        return er_fault;
    
    result = io_cs_append_entry_to_STRING(
        cs, io_cs_entry_host_name, "HostName=", c_string);
    if (result != er_ok)
        return result;
    result = io_cs_append_entry_to_STRING(
        cs, io_cs_entry_device_id, "DeviceId=", c_string);
    if (result != er_ok)
        return result;
	result = io_cs_append_entry_to_STRING(
		cs, io_cs_entry_endpoint, "Endpoint=", c_string);
	if (result != er_ok)
		return result;
	result = io_cs_append_entry_to_STRING(
        cs, io_cs_entry_entity, "EntityPath=", c_string);
    if (result != er_ok)
        return result;
    result = io_cs_append_entry_to_STRING(
        cs, io_cs_entry_shared_access_key_name, "SharedAccessKeyName=", c_string);
    if (result != er_ok)
        return result;
    result = io_cs_append_entry_to_STRING(
        cs, io_cs_entry_shared_access_key, "SharedAccessKey=", c_string);
    if (result != er_ok)
        return result;
    result = io_cs_append_entry_to_STRING(
        cs, io_cs_entry_consumer_group, "ConsumerGroup=", c_string);
    if (result != er_ok)
        return result;
    result = io_cs_append_entry_to_STRING(
        cs, io_cs_entry_partition_count, "Partitions=", c_string);
    if (result != er_ok)
        return result;
    result = io_cs_append_entry_to_STRING(
        cs, io_cs_entry_endpoint_name, "EndpointName=", c_string);
    if (result != er_ok)
        return result;
    result = io_cs_append_entry_to_STRING(
        cs, io_cs_entry_shared_access_token, "SharedAccessToken=", c_string);
    if (result != er_ok)
        return result;

    return result;
}

//
// Free connection cs
//
void io_cs_free(
    io_cs_t* cs
)
{
    if (!cs)
        return;
    for (int32_t i = 0; i < io_cs_entry_max; i++)
    {
        if (cs->entries[i])
            STRING_delete(cs->entries[i]);
    }
    mem_free_type(io_cs_struct_t, cs);
}

//
// Create connection cs from string
//
int32_t io_cs_create_from_string(
    const char* c_string,
    io_cs_t** created
)
{
    int32_t result;
    io_cs_struct_t* cs;
    if (!c_string || !created)
        return er_fault;

    cs = mem_zalloc_type(io_cs_struct_t);
    if (!cs)
        return er_out_of_memory;
    do
    {
        result = string_key_value_parser(
            c_string, io_cs_parser_callback, ';', cs);
        if (result != er_ok)
            break;

        result = io_cs_validate(cs);
        if (result != er_ok)
            break;

        *created = cs;
        return er_ok;

    } while (0);

    io_cs_free(cs);
    return result;
}

//
// Create new connection cs
//
int32_t io_cs_create(
    const char* host_name,
    const char* device_id,
    const char* shared_access_key_name,
    const char* shared_access_key,
    io_cs_t** created
)
{
    int32_t result = er_out_of_memory;
    io_cs_struct_t* cs;
    if (!host_name || !created)
        return er_fault;

    cs = mem_zalloc_type(io_cs_struct_t);
    if (!cs)
        return er_out_of_memory;
    do
    {
        cs->entries[io_cs_entry_host_name] = 
            STRING_construct(host_name);
        if (!cs->entries[io_cs_entry_host_name])
            break;

        if (device_id)
        {
            cs->entries[io_cs_entry_device_id] = 
                STRING_construct(device_id);
            if (!cs->entries[io_cs_entry_device_id])
                break;
        }

        if (shared_access_key_name)
        {
            cs->entries[io_cs_entry_shared_access_key_name] = 
                STRING_construct(shared_access_key_name);
            if (!cs->entries[io_cs_entry_shared_access_key_name])
                break;
        }

        if (shared_access_key)
        {
            cs->entries[io_cs_entry_shared_access_key] = 
                STRING_construct(shared_access_key);
            if (!cs->entries[io_cs_entry_shared_access_key])
                break;
        }

        result = io_cs_validate(cs);
        if (result != er_ok)
            break;

        *created = cs;
        return er_ok;

    } while (0);

    io_cs_free(cs);
    return result;
}

//
// Clone connection cs
//
int32_t io_cs_clone(
    io_cs_t* orig,
    io_cs_t** cloned
)
{
    int32_t result = er_ok;
    io_cs_struct_t* cs;
    if (!orig || !cloned)
        return er_fault;

    cs = mem_zalloc_type(io_cs_struct_t);
    if (!cs)
        return er_out_of_memory;
    for (int32_t i = 0; i < io_cs_entry_max; i++)
    {
        if (!orig->entries[i])
            continue;
            
        cs->entries[i] = STRING_clone(orig->entries[i]);
        if (!cs->entries[i])
        {
            result = er_out_of_memory;
            break;
        }
    }
    if (result != er_ok)
        io_cs_free(cs);
    else
        *cloned = cs;
    return result;
}

//
// Returns an entry in the connection string
//
const char* io_cs_get_entry(
    io_cs_t* cs,
    io_cs_entry_t id
)
{
    if (id < 0 || id >= io_cs_entry_max || !cs || !cs->entries[id])
        return NULL;
    return STRING_c_str(cs->entries[id]);
}

//
// Returns the host name from the cs
//
const char* io_cs_get_host_name(
    io_cs_t* cs
)
{
    size_t len;
    const char* val;

    if (!cs)
        return NULL;
    if (!cs->entries[io_cs_entry_host_name])
    {
        val = io_cs_get_endpoint(cs);  
        if (!val)
            return NULL;

        len = strlen(val);
        val = string_trim_scheme(val, &len);
        for (size_t i = 0; i < len; i++)
        {
            if (val[i] == '/' || !val[i])
            {
                cs->entries[io_cs_entry_host_name] =
                    STRING_safe_construct_n(val, i);
                break;
            }
        }
    }
    return io_cs_get_entry(cs, io_cs_entry_host_name);
}

//
// Returns the device id from the cs
//
const char* io_cs_get_device_id(
    io_cs_t* cs
)
{
    return io_cs_get_entry(cs, io_cs_entry_device_id);
}

//
// Returns the entity, ie. queue
//
const char* io_cs_get_entity(
    io_cs_t* cs
)
{
    return io_cs_get_entry(cs, io_cs_entry_entity);
}

//
// Returns the endpoint from the connection string
//
const char* io_cs_get_endpoint(
    io_cs_t* cs
)
{
    return io_cs_get_entry(cs, io_cs_entry_endpoint);
}

//
// Returns the endpoint name from the connection string
//
const char* io_cs_get_endpoint_name(
    io_cs_t* cs
)
{
    const char* result = io_cs_get_entry(cs, io_cs_entry_endpoint_name);
    if (!result)
        result = io_cs_get_hub_name(cs);
    return result;
}

//
// Returns the consumer group info from the connection string
//
const char* io_cs_get_consumer_group(
    io_cs_t* cs
)
{
    const char* result = io_cs_get_entry(cs, io_cs_entry_consumer_group);
    if (!result && io_cs_get_endpoint(cs))
        return "$default";
    return result;
}

//
// Returns the partition count from connection string
//
int32_t io_cs_get_partition_count(
    io_cs_t* cs
)
{
    const char* result = io_cs_get_entry(cs, io_cs_entry_partition_count);
    if (!result)
        return io_cs_get_endpoint(cs) ? 4 : 0;
    return atoi(result);
}

//
// Returns the shared access token from cs
//
const char* io_cs_get_shared_access_token(
    io_cs_t* cs
)
{
    return io_cs_get_entry(cs, io_cs_entry_shared_access_token);
}

//
// Returns the key name from the cs
//
const char* io_cs_get_shared_access_key_name(
    io_cs_t* cs
)
{
    return io_cs_get_entry(cs, io_cs_entry_shared_access_key_name);
}

//
// Returns the key from the cs
//
const char* io_cs_get_shared_access_key(
    io_cs_t* cs
)
{
    return io_cs_get_entry(cs, io_cs_entry_shared_access_key);
}

//
// Returns the name of the hub
//
const char* io_cs_get_hub_name(
    io_cs_t* cs
)
{
    size_t len;
    const char* val;

    if (!cs)
        return NULL;
    if (!cs->entries[io_cs_entry_hub_name])
    {
        val = io_cs_get_host_name(cs);
        dbg_assert_ptr(val);
        len = strlen(val);
        for (size_t i = 0; i < len; i++)
        {
            if (val[i] == '.')
            {
                cs->entries[io_cs_entry_hub_name] =
                    STRING_safe_construct_n(val, i);
                break;
            }
        }
    }
    return io_cs_get_entry(cs, io_cs_entry_hub_name);
}

//
// Sets an entry in the connection string
//
int32_t io_cs_set_entry(
    io_cs_t* cs,
    io_cs_entry_t id,
    const char* entry
)
{
    if (!cs)
        return er_fault;
    if (id < 0 || id >= io_cs_entry_max)
        return er_arg;
    if (cs->entries[id])
        STRING_delete(cs->entries[id]);
    if (!entry)
        cs->entries[id] = NULL;
    else
    {
        cs->entries[id] = STRING_construct(entry);
        if (!cs->entries[id])
            return er_out_of_memory;
    }
    return er_ok;
}

//
// Set the host name in the cs
//
int32_t io_cs_set_host_name(
    io_cs_t* cs,
    const char* host_name
)
{
    int32_t result;
    if (!cs || !host_name)
        return er_fault;

    result = io_cs_validate_host_name(host_name);
    if (result != er_ok)
        return result;

    if (cs->entries[io_cs_entry_host_name])
        STRING_delete(cs->entries[io_cs_entry_host_name]);
    cs->entries[io_cs_entry_host_name] = STRING_construct_trim(host_name, "/\\ ");
    if (!cs->entries[io_cs_entry_host_name])
        return er_out_of_memory;
    return er_ok;
}

//
// Set the device id in the cs
//
int32_t io_cs_set_device_id(
    io_cs_t* cs,
    const char* device_id
)
{
    int32_t result;
    if (!cs)
        return er_fault;

    if (device_id)
    {
        result = io_cs_validate_device_id(device_id);
        if (result != er_ok)
            return result;
    }
    return io_cs_set_entry(cs, io_cs_entry_device_id, device_id);
}

//
// Set the event hub endpoint in the cs
//
int32_t io_cs_set_endpoint(
    io_cs_t* cs,
    const char* endpoint
)
{
    size_t val_len;
    if (!cs)
        return er_fault;
    if (!endpoint)
        return io_cs_set_entry(cs, io_cs_entry_endpoint, NULL);
    if (cs->entries[io_cs_entry_endpoint])
        STRING_delete(cs->entries[io_cs_entry_endpoint]);
    // Skip scheme and remove any trailing slash
    val_len = strlen(endpoint);
    endpoint = string_trim_scheme(endpoint, &val_len);
    val_len = string_trim_back_len(endpoint, val_len, "/\\ ");
    cs->entries[io_cs_entry_endpoint] =
        STRING_safe_construct_n(endpoint, val_len);
    if (!cs->entries[io_cs_entry_endpoint])
        return er_out_of_memory;
    return er_ok;
}

//
// Set the event hub endpoint name
//
int32_t io_cs_set_endpoint_name(
    io_cs_t* cs,
    const char* endpoint_name
)
{
    return io_cs_set_entry(cs, io_cs_entry_endpoint_name, endpoint_name);
}

//
// Set the event hub consumer group
//
int32_t io_cs_set_consumer_group(
    io_cs_t* cs,
    const char* consumer_group
)
{
    return io_cs_set_entry(cs, io_cs_entry_consumer_group, consumer_group);
}

//
// Set partition count
//
int32_t io_cs_set_partition_count(
    io_cs_t* cs,
    int32_t count
)
{
    int32_t result;
    char buf[32];
    if (count)
    {
        result = string_from_int(count, 10, buf, sizeof(buf));
        if (result != er_ok)
            return result;
    }
    return io_cs_set_entry(cs, io_cs_entry_partition_count, count ? buf : NULL);
}

//
// Set the entity, ie. queue name
//
int32_t io_cs_set_entity(
    io_cs_t* cs,
    const char* entity
)
{
    return io_cs_set_entry(cs, io_cs_entry_entity, entity);
}

//
// Create token provider from connection string
//
int32_t io_cs_create_token_provider(
    io_cs_t* cs,
    io_token_provider_t** provider
)
{
    int32_t result;
    const char *key, *val, *delim = "/";
    STRING_HANDLE scope = NULL, b64_key = NULL;

    key = io_cs_get_shared_access_token(cs);
    if (key)
        return io_passthru_token_provider_create(key, provider);
    do
    {
        result = er_invalid_format;
        key = io_cs_get_shared_access_key(cs);
        if (!key)
            break;

        val = io_cs_get_endpoint(cs);
        if (!val)
        {
            val = io_cs_get_host_name(cs);
            if (!val)
                break;
        }

        result = er_out_of_memory;
        scope = STRING_construct(val);
        if (!scope)
            break;

        val = io_cs_get_device_id(cs);
        if (val)
        {
            // scope is {HostName}/devices/{DeviceId}
            delim = "/devices/";
            // Iot hub connection string is alread base 64
            b64_key = STRING_construct(key);
        }
        else
        {
            val = io_cs_get_entity(cs);
            if (!val)
                val = io_cs_get_endpoint_name(cs);
            // Convert utf-8 key to base 64
            b64_key = STRING_construct_base64((const unsigned char*)key, strlen(key));
        }

        if (!b64_key)
            break;
        if (val)
        {
            if (0 != STRING_concat(scope, delim) ||
                0 != STRING_concat(scope, val))
                break;
        }
        result = io_iothub_token_provider_create(io_cs_get_shared_access_key_name(cs), 
            STRING_c_str(b64_key), STRING_c_str(scope), provider);
        if (result != er_ok)
            break;
    } while (0);

    if (scope)
        STRING_delete(scope);
    if (b64_key)
        STRING_delete(b64_key);
    return result;
}

//
// Encode the connection string
//
int32_t io_encode_cs(
    io_codec_ctx_t *ctx,
    io_cs_t** cs
)
{
    int32_t result;
    STRING_HANDLE connection_string;

    dbg_assert_ptr(ctx);
    dbg_assert_ptr(cs);
    dbg_assert_ptr(*cs);

    __io_encode_type_begin(ctx, cs, 1);
    connection_string = io_cs_to_STRING(*cs);
    if (!connection_string)
        return er_out_of_memory;
    result = io_encode_STRING_HANDLE(ctx, "connection-string", connection_string);
    STRING_delete(connection_string);
    if (result != er_ok)
        return result;
    __io_encode_type_end(ctx);
    return result;
}

//
// Decode the connection string
//
int32_t io_decode_cs(
    io_codec_ctx_t *ctx,
    io_cs_t** cs
)
{
    int32_t result;
    STRING_HANDLE connection_string;

    dbg_assert_ptr(ctx);
    dbg_assert_ptr(cs);

    __io_decode_type_begin(ctx, cs, 1);
    result = io_decode_STRING_HANDLE(ctx, "connection-string", &connection_string);
    if (result != er_ok)
        return result;
    result = io_cs_create_from_string(STRING_c_str(connection_string), cs);
    STRING_delete(connection_string);
    if (result != er_ok)
        return result;
    __io_decode_type_end(ctx);
    return result;
}
