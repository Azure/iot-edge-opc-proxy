// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _io_cs_h_
#define _io_cs_h_

#include "common.h"
#include "io_codec.h"
#include "io_token.h"

//
// connection connection string
//
typedef struct io_cs io_cs_t;

//
// Create new connection connection string from string
//
decl_internal_2(int32_t, io_cs_create_from_string,
    const char*, connection_string,
    io_cs_t**, created
);

//
// Create connection cs from raw content of file
//
decl_internal_2(int32_t, io_cs_create_from_raw_file,
    const char*, file_name,
    io_cs_t**, created
);

//
// Create new connection connection string
//
decl_internal_5(int32_t, io_cs_create,
    const char*, host_name,
    const char*, device_id,
    const char*, shared_access_key_name,
    const char*, shared_access_key,
    io_cs_t**, created
);

//
// Clone connection connection string
//
decl_internal_2(int32_t, io_cs_clone,
    io_cs_t*, orig,
    io_cs_t**, cloned
);

//
// Create connection string from connection string
//
decl_internal_2(int32_t, io_cs_append_to_STRING,
    io_cs_t*, connection_string,
    STRING_HANDLE, string
);

//
// Create connection string from connection string
//
decl_internal_1(STRING_HANDLE, io_cs_to_STRING,
    io_cs_t*, connection_string
);

//
// Returns the host name from the connection string
//
decl_internal_1(const char*, io_cs_get_host_name,
    io_cs_t*, connection_string
);

//
// Set the host name in the connection string
//
decl_internal_2(int32_t, io_cs_set_host_name,
    io_cs_t*, connection_string,
    const char*, host_name
);

//
// Returns the name of the hub
//
decl_internal_1(const char*, io_cs_get_hub_name,
    io_cs_t*, connection_string
);

//
// Returns the device id from the connection string
//
decl_internal_1(const char*, io_cs_get_device_id,
    io_cs_t*, connection_string
);

//
// Set the device id in the connection string
//
decl_internal_2(int32_t, io_cs_set_device_id,
    io_cs_t*, connection_string,
    const char*, device_id
);

//
// Returns the service bus endpoint from the string
//
decl_internal_1(const char*, io_cs_get_endpoint,
    io_cs_t*, connection_string
);

//
// Set the service bus endpoint in the connection string
//
decl_internal_2(int32_t, io_cs_set_endpoint,
    io_cs_t*, connection_string,
    const char*, endpoint
);

//
// Returns the endpoint path from the string
//
decl_internal_1(const char*, io_cs_get_endpoint_name,
    io_cs_t*, connection_string
);

//
// Set the service bus endpoint path in the connection string
//
decl_internal_2(int32_t, io_cs_set_endpoint_name,
    io_cs_t*, connection_string,
    const char*, endpoint_name
);

//
// Returns the event hub consumer group
//
decl_internal_1(const char*, io_cs_get_consumer_group,
    io_cs_t*, connection_string
);

//
// Set the event hub consumer group
//
decl_internal_2(int32_t, io_cs_set_consumer_group,
    io_cs_t*, connection_string,
    const char*, consumer_group
);

//
// Returns the event partition count
//
decl_internal_1(int32_t, io_cs_get_partition_count,
    io_cs_t*, connection_string
);

//
// Set partition count in the connection string
//
decl_internal_2(int32_t, io_cs_set_partition_count,
    io_cs_t*, connection_string,
    int32_t, count
);

//
// Returns the key name from the connection string
//
decl_internal_1(const char*, io_cs_get_shared_access_key_name,
    io_cs_t*, connection_string
);

//
// Returns the token from the connection string
//
decl_internal_1(const char*, io_cs_get_shared_access_token,
    io_cs_t*, connection_string
);

//
// Returns the entity path, ie. queue
//
decl_internal_1(const char*, io_cs_get_entity,
    io_cs_t*, connection_string
);

//
// Set the entity path, ie. queue name
//
decl_internal_2(int32_t, io_cs_set_entity,
    io_cs_t*, connection_string,
    const char*, entity
);

//
// Create token provider from connection string
//
decl_internal_2(int32_t, io_cs_create_token_provider,
    io_cs_t*, cs,
    io_token_provider_t**, provider
);

//
// Releases any persistently stored key handle
//
decl_internal_1(void, io_cs_remove_keys,
    io_cs_t*, connection_string
);

//
// Free connection string
//
decl_internal_1(void, io_cs_free,
    io_cs_t*, connection_string
);

//
// Encode a connection string
//
decl_internal_2(int32_t, io_encode_cs,
    io_codec_ctx_t*, ctx,
    io_cs_t**, connection_string
);

//
// Decode connection string
//
decl_internal_2(int32_t, io_decode_cs,
    io_codec_ctx_t*, ctx,
    io_cs_t**, connection_string
);

#endif // _io_cs_h_