// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _io_transport_h_
#define _io_transport_h_

#include "common.h"
#include "util_dbg.h"
#include "prx_sched.h"
#include "prx_ns.h"
#include "io_proto.h"

//
// Transport interface is used to create ...
//
typedef struct io_transport io_transport_t;

//
// ... connections
//
typedef struct io_connection io_connection_t;

//
// Send message over connection
//
typedef int32_t (*io_connection_send_t)(
    void* context,
    io_message_t* message
);

//
// Close connection
//
typedef void (*io_connection_close_t)(
    void* context
    );

//
// Free connection
//
typedef void (*io_connection_free_t)(
    void* context
    );

//
// Connection interface
//
struct io_connection
{
    io_connection_send_t on_send;       // Send message over connection
    io_connection_close_t on_close;                 // Close connection
    io_connection_free_t on_free;                    // Free connection
    void* context;                  // Actual connection implementation
};

//
// Send message over connection interface
//
decl_inline_2(int32_t, io_connection_send,
    io_connection_t*, connection,
    io_message_t*, message
)
{
    chk_arg_fault_return(connection);
    dbg_assert_ptr(connection->on_send);
    return connection->on_send(connection->context, message);
}

//
// Close connection interface
//
decl_inline_1(void, io_connection_close,
    io_connection_t*, connection
)
{
    if (!connection)
        return;
    dbg_assert_ptr(connection->on_close);
    connection->on_close(connection->context);
}

//
// Free connection - call only after closed event received...
//
decl_inline_1(void, io_connection_free,
    io_connection_t*, connection
)
{
    if (!connection)
        return;
    dbg_assert_ptr(connection->on_free);
    connection->on_free(connection->context);
}

//
// Connection events
// 
typedef enum io_connection_event_t
{
    io_connection_received,               // Send when message received
    io_connection_reconnecting,    // Sent when connection reconnecting
    io_connection_closed    // Send when connection successfully closed
}
io_connection_event_t;

//
// Event callback for connection
//
typedef int32_t (*io_connection_cb_t)(
    void* context,
    io_connection_event_t ev,
    io_message_t* message,
    int32_t last_error,
    uint32_t* delay
    );

// 
// Create connection on transport
//
typedef int32_t (*io_transport_create_connection_t)(
    void* context,
    prx_ns_entry_t* entry,
    io_codec_id_t codec_id,
    io_connection_cb_t handler_cb,
    void* handler_ctx,
    prx_scheduler_t* scheduler,
    io_connection_t** connection
);

//
// Transport interface
//
struct io_transport
{
    io_transport_create_connection_t on_create;    // Create connection
    void* context;                   // Actual transport implementation
};

//
// Create connection from transport interface
//
decl_inline_7(int32_t, io_transport_create,
    io_transport_t*, transport,
    prx_ns_entry_t*, entry,
    io_codec_id_t, codec_id,
    io_connection_cb_t, cb,
    void*, context,
    prx_scheduler_t*, scheduler,
    io_connection_t**, connection
)
{
    chk_arg_fault_return(transport);
    dbg_assert_ptr(transport->on_create);
    return transport->on_create(transport->context, 
        entry, codec_id, cb, context, scheduler, connection);
}

//
// Get transport for transport type
//
decl_internal_1(io_transport_t*, io_transport_get,
    prx_transport_type_t, type
);


#endif // _io_transport_h_