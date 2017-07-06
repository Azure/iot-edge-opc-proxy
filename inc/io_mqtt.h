// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _io_mqtt_h_
#define _io_mqtt_h_

#include "common.h"
#include "io_url.h"
#include "io_token.h"
#include "prx_sched.h"

//
// Mqtt connection
//
typedef struct io_mqtt_connection io_mqtt_connection_t;

//
// Mqtt subscription 
//
typedef struct io_mqtt_subscription io_mqtt_subscription_t;

//
// Properties handle
//
typedef struct io_mqtt_properties io_mqtt_properties_t;

//
// Receive callback
//
typedef void (*io_mqtt_subscription_receiver_t)(
    void* context,
    io_mqtt_properties_t* properties,
    const uint8_t* body,
    size_t body_len
    );

//
// Create a connection to an endpoint
//
decl_internal_4(int32_t, io_mqtt_connection_create,
    io_url_t*, address,
    const char*, client_id,
    prx_scheduler_t*, scheduler,
    io_mqtt_connection_t**, created
);

//
// Subscribe to a subscription
//
decl_internal_5(int32_t, io_mqtt_connection_subscribe,
    io_mqtt_connection_t*, connection,
    const char*, uri,
    io_mqtt_subscription_receiver_t, cb,
    void*, context,
    io_mqtt_subscription_t**, subscription
);

//
// Reconnect callback
//
typedef bool (*io_mqtt_connection_reconnect_t)(
    void* context,
    int32_t last_error
    );

//
// and call connect on the connection 
//
decl_internal_3(int32_t, io_mqtt_connection_connect,
    io_mqtt_connection_t*, connection,
    io_mqtt_connection_reconnect_t, reconnect_cb,
    void*, reconnect_ctx
);

//
// Create properties bag
//
decl_internal_1(int32_t, io_mqtt_properties_create,
    io_mqtt_properties_t**, properties
);

//
// Add property to bag
//
decl_internal_3(int32_t, io_mqtt_properties_add,
    io_mqtt_properties_t*, properties,
    const char*, key,
    const char*, value
);

//
// Get property from bag
//
decl_internal_4(int32_t, io_mqtt_properties_get,
    io_mqtt_properties_t*, properties,
    const char*, key,
    char*, value_buf,
    size_t, value_len
);

//
// Free properties bag
//
decl_internal_1(void, io_mqtt_properties_free,
    io_mqtt_properties_t*, properties
);

//
// Publish complete callback
//
typedef void (*io_mqtt_publish_complete_t)(
    void* context,
    int32_t result
    );

//
// Publish data
//
decl_internal_7(int32_t, io_mqtt_connection_publish,
    io_mqtt_connection_t*, connection,
    const char*, uri,
    io_mqtt_properties_t*, properties,
    const uint8_t*, body,
    size_t, body_len,
    io_mqtt_publish_complete_t, cb,
    void*, context
);

//
// Unsubscribe when done with subscription 
//
decl_internal_1(void, io_mqtt_subscription_release,
    io_mqtt_subscription_t*, subscription
);

//
// Close connection to disconnect and free all subscriptions
//
decl_internal_1(void, io_mqtt_connection_close,
    io_mqtt_connection_t*, connection
);

#endif // _io_mqtt_h_
