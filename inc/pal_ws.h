// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _pal_wsclient_h_
#define _pal_wsclient_h_

#include "common.h"
#include "prx_types.h"

//
// Platform specific websocket client 
//
typedef struct pal_wsclient pal_wsclient_t;

//
// Websocket client events
//
typedef enum pal_wsclient_event
{
    pal_wsclient_event_unknown = 0,
    pal_wsclient_event_connected,       // Connection established
    pal_wsclient_event_begin_recv,    // Requests an empty buffer
    pal_wsclient_event_end_recv,   // Sent when buffer was filled
    pal_wsclient_event_begin_send,    // Requests a filled buffer
    pal_wsclient_event_end_send,        // Filled buffer was sent
    pal_wsclient_event_disconnected,              // Disconnected
    pal_wsclient_event_closed                           // Closed
}
pal_wsclient_event_t;

//
// Buffer type for send and receive
//
typedef enum pal_wsclient_buffer_type
{
    pal_wsclient_buffer_type_unknown = 0,
    pal_wsclient_buffer_type_binary_msg,
    pal_wsclient_buffer_type_binary_fragment,
    pal_wsclient_buffer_type_utf8_msg,
    pal_wsclient_buffer_type_utf8_fragment
}
pal_wsclient_buffer_type_t;

//
// Called by when event on websocket occurred to service protocol  
//
typedef void (*pal_wsclient_event_handler_t)(
    void* context,
    pal_wsclient_event_t ev,
    uint8_t** buffer,         // The buffer for the event or null
    size_t* size,        // Size of buffer or amount read/written
    pal_wsclient_buffer_type_t* type,  // Flags describing buffer
    int32_t error    // Error that occurred, e.g. as result of op
    );

//
// Create free threaded websocket client
//
decl_public_8(int32_t, pal_wsclient_create,
    const char*, protocol_name,
    const char*, host,
    uint16_t, port,
    const char*, path,
    bool, secure,
    pal_wsclient_event_handler_t, callback,
    void*, callback_context,
    pal_wsclient_t**, wsclient
);

//
// Add key value to header
//
decl_internal_3(int32_t, pal_wsclient_add_header,
    pal_wsclient_t*, wsclient,
    const char*, key,
    const char*, value
);

//
// Connect client - will cause connected event to be sent
//
decl_public_1(int32_t, pal_wsclient_connect,
    pal_wsclient_t*, wsclient
);

//
// While on, will receive as many buffers as possible
//
decl_public_2(int32_t, pal_wsclient_can_recv,
    pal_wsclient_t*, wsclient,
    bool, enable
);

//
// While on, will send as many buffers as possible until disconnect
//
decl_public_2(int32_t, pal_wsclient_can_send,
    pal_wsclient_t*, wsclient,
    bool, enable
);

//
// Disconnect client - will cause disconnect event to be sent
//
decl_public_1(int32_t, pal_wsclient_disconnect,
    pal_wsclient_t*, wsclient
);

//
// Close websocket client - will cause closed event to be sent
//
decl_public_1(void, pal_wsclient_close,
    pal_wsclient_t*, wsclient
);

//
// Initialize websocket client pal
//
decl_public_0(int32_t, pal_wsclient_init,
    void
);

//
// Destroy websocket client pal
//
decl_public_0(void, pal_wsclient_deinit,
    void
);

#endif // _pal_wsclient_h_
