// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _prx_config_h_
#define _prx_config_h_

#include "common.h"
#include "io_codec.h"

//
// Proxy configuration
//
typedef struct prx_config prx_config_t;

//
// Keys in configuration
//
typedef enum prx_config_key
{
    prx_config_key_proxy_host,       // Host name of the proxy server
    prx_config_key_proxy_user,                   // User name and ...
    prx_config_key_proxy_pwd,       // ... password to use to connect
    prx_config_key_connect_flag,      // Controls connection behavior
    prx_config_key_token_ttl,       // Default ttl for created tokens
    prx_config_key_policy_import,   // Whether to allow policy import
    prx_config_key_log_telemetry, // Whether to send log to event hub
    prx_config_key_browse_fs, // Whether to allow browsing filesystem
    prx_config_key_restricted_ports,   // Ports allowed to connect to
    prx_config_key_bind_device,       // Device to attempt to bind to
    prx_config_key_max
}
prx_config_key_t;

//
// Store string in configuration
//
decl_internal_3(int32_t, prx_config_set_string,
    prx_config_t*, config,
    prx_config_key_t, key,
    const char*, value
);

//
// Get string from configuration
//
decl_internal_3(const char*, prx_config_get_string,
    prx_config_t*, config,
    prx_config_key_t, key,
    const char*, def
);

//
// Store int in configuration
//
decl_internal_3(int32_t, prx_config_set_int,
    prx_config_t*, config,
    prx_config_key_t, key,
    int32_t, value
);

//
// Get int from configuration
//
decl_internal_3(int, prx_config_get_int,
    prx_config_t*, config,
    prx_config_key_t, key,
    int32_t, def
);

//
// Create new config
//
decl_internal_1(int32_t, prx_config_create,
    prx_config_t**, created
);

//
// Free configuration object
//
decl_internal_1(void, prx_config_free,
    prx_config_t*, config
);

//
// Encode configuration object
//
decl_internal_2(int32_t, io_encode_prx_config,
    io_codec_ctx_t*, ctx,
    prx_config_t*, config
);

//
// Decode configuration object
//
decl_internal_2(int32_t, io_decode_prx_config,
    io_codec_ctx_t*, ctx,
    prx_config_t*, config
);

//
// Get global process config
//
decl_internal_0(prx_config_t*, _prx_config,
    void
);

//
// Returns a global string value
//
#define __prx_config_get(k, d) \
    prx_config_get_string(_prx_config(), k, d)

//
// Updates a global string value
//
#define __prx_config_set(k, v) \
    prx_config_set_string(_prx_config(), k, v)

//
// Returns a global int value
//
#define __prx_config_get_int(k, d) \
    prx_config_get_int(_prx_config(), k, d)

//
// Updates a global int value
//
#define __prx_config_set_int(k, v) \
    prx_config_set_int(_prx_config(), k, v)

//
// Initialize config
//
decl_internal_0(int32_t, prx_config_init,
    void
);

//
// Deinit config
//
decl_internal_0(void, prx_config_deinit,
    void
);


#endif