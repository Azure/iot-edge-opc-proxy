// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _io_ws_h_
#define _io_ws_h_

#include "common.h"
#include "prx_types.h"
#include "io_stream.h"
#include "io_token.h"
#include "prx_sched.h"
#include "io_url.h"

//
// Websocket stream based connection bound to scheduler
//
typedef struct io_ws_connection io_ws_connection_t;

//
// Create websocket connection
//
decl_public_7(int32_t, io_ws_connection_create,
    io_url_t*, address,
    const char*, user_header_key,
    const char*, pwd_header_key,
    prx_scheduler_t*, scheduler,
    io_stream_handler_t, receiver_cb,
    void*, receiver_ctx,
    io_ws_connection_t**, connection
);

//
// Reconnect callback
//
typedef bool (*io_ws_connection_reconnect_t)(
    void* context,
    int32_t last_error
);

//
// Connect -- must be called from scheduler
//
decl_public_3(int32_t, io_ws_connection_connect,
    io_ws_connection_t*, connection,
    io_ws_connection_reconnect_t, reconnect_cb,
    void*, reconnect_ctx
);

//
// Send complete callback
//
typedef void (*io_ws_connection_send_complete_t)(
    void* context,
    int32_t result
    );

//
// Write to connection -- must be called from scheduler
//
decl_public_5(int32_t, io_ws_connection_send,
    io_ws_connection_t*, connection,
    io_stream_handler_t, sender_cb,
    void*, sender_ctx,
    io_ws_connection_send_complete_t, complete_cb,
    void*, complete_ctx
);

//
// Close connection -- must be called from scheduler
//
decl_public_1(int32_t, io_ws_connection_close,
    io_ws_connection_t*, connection
);

#endif // _io_ws_h_
