// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "util_string.h"
#include "prx_config.h"

//
// Configuration map
//
struct prx_config
{
    STRING_HANDLE values[prx_config_key_max];
};

//
// Global config storage
//
static prx_config_t* _config;

//
// Returns the name of the key
//
static const char* prx_config_key_name(
    prx_config_key_t key
)
{
    switch (key)
    {
    case prx_config_key_proxy_host:
        return "proxy_host";
    case prx_config_key_proxy_user:
        return "proxy_user";
    case prx_config_key_proxy_pwd:
        return "proxy_pwd";
    case prx_config_key_token_ttl:
        return "token_ttl";
    case prx_config_key_policy_import:
        return "policy_import";
    case prx_config_key_log_telemetry:
        return "log_telemetry";
    case prx_config_key_browse_fs:
        return "browse_fs";
    case prx_config_key_restricted_ports:
        return "restricted_ports";
    case prx_config_key_max:
    default:
        return NULL;
    }
}

//
// Free connection config
//
void prx_config_free(
    prx_config_t* config
)
{
    if (!config)
        return;
    for (int32_t i = 0; i < prx_config_key_max; i++)
    {
        if (config->values[i])
            STRING_delete(config->values[i]);
    }
    mem_free_type(prx_config_t, config);
}

//
// Create new config
//
int32_t prx_config_create(
    prx_config_t** created
)
{
    prx_config_t* config;
    chk_arg_fault_return(created);

    config = mem_zalloc_type(prx_config_t);
    if (!config)
        return er_out_of_memory;
    *created = config;
    return er_ok;
}

//
// Returns a string configuration entry
//
const char* prx_config_get_string(
    prx_config_t* config,
    prx_config_key_t key,
    const char* def
)
{
    if (key >= prx_config_key_max || !config || !config->values[key])
        return def;
    return STRING_c_str(config->values[key]);
}

//
// Returns integer configuration entry
//
int32_t prx_config_get_int(
    prx_config_t* config,
    prx_config_key_t key,
    int32_t def
)
{
    const char* result = prx_config_get_string(config, key, NULL);
    if (!result)
        return def;
    return atoi(result);
}

//
// Sets a string configuration entry
//
int32_t prx_config_set_string(
    prx_config_t* config,
    prx_config_key_t key,
    const char* val
)
{
    chk_arg_fault_return(config);
    if (key >= prx_config_key_max)
        return er_arg;
    if (config->values[key])
        STRING_delete(config->values[key]);
    if (!val)
        config->values[key] = NULL;
    else
    {
        config->values[key] = STRING_construct(val);
        if (!config->values[key])
            return er_out_of_memory;
    }
    return er_ok;
}

//
// Set integer configuration entry
//
int32_t prx_config_set_int(
    prx_config_t* config,
    prx_config_key_t key,
    int32_t val
)
{
    int32_t result;
    char buf[32];
    if (val)
    {
        result = string_from_int(val, 10, buf, sizeof(buf));
        if (result != er_ok)
            return result;
    }
    return prx_config_set_string(config, key, val ? buf : NULL);
}

//
// Encode the configuration
//
int32_t io_encode_prx_config(
    io_codec_ctx_t* ctx,
    prx_config_t* config
)
{
    int32_t result;

    dbg_assert_ptr(ctx);
    dbg_assert_ptr(config);

    __io_encode_type_begin(ctx, config, prx_config_key_max);
    for (int32_t i = 0; i < prx_config_key_max; i++)
    {
        if (!config->values[i])
        {
            continue;
        }
        result = io_encode_STRING_HANDLE(ctx, 
            prx_config_key_name((prx_config_key_t)i), config->values[i]);
        if (result != er_ok)
            return result;
    }
    __io_encode_type_end(ctx);
    return result;
}

//
// Decode the configuration
//
int32_t io_decode_prx_config(
    io_codec_ctx_t *ctx,
    prx_config_t* config
)
{
    int32_t result;

    dbg_assert_ptr(ctx);
    dbg_assert_ptr(config);

    __io_decode_type_begin(ctx, config, prx_config_key_max);
    for (int32_t i = 0; i < prx_config_key_max; i++)
    {
        result = io_decode_STRING_HANDLE(ctx,
            prx_config_key_name((prx_config_key_t)i), &config->values[i]);
       // if (result != er_ok && result != er_not_found)
       //     return result;
    }
    __io_decode_type_end(ctx);
    return result;
}

//
// Get global process config 
//
prx_config_t* _prx_config(
    void
)
{
    return _config;
}

//
// Initialize config
//
int32_t prx_config_init(
    void
)
{
    if (_config)
        return er_bad_state;
    return prx_config_create(&_config);
}

//
// Deinit config
//
void prx_config_deinit(
    void
)
{
    if (!_config)
        return;
    prx_config_free(_config);
    _config = NULL;
}

