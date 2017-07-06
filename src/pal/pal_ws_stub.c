// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "pal_ws.h"

//
// Init stub - Websocket capability will not be set
//
int32_t pal_wsclient_init(
    void
)
{
    return er_not_supported;
}

//
// Deinit stub
//
void pal_wsclient_deinit(
    void
)
{
    // no op
}

//
// Linker stub
//
int32_t pal_wsclient_create(
    const char* protocol_name,
    const char* host,
    uint16_t port,
    const char* path,
    bool secure,
    pal_wsclient_event_handler_t callback,
    void* callback_context,
    pal_wsclient_t** wsclient
)
{
    (void)protocol_name;
    (void)host;
    (void)port;
    (void)path;
    (void)secure;
    (void)callback;
    (void)callback_context;
    (void)wsclient;
    dbg_assert(0, "Not supported");
    return er_not_supported;
}

//
// Linker stub
//
int32_t pal_wsclient_add_header(
    pal_wsclient_t* wsclient,
    const char* key,
    const char* value
)
{
    (void)wsclient;
    (void)key;
    (void)value;
    dbg_assert(0, "Not supported");
    return er_not_supported;
}

//
// Linker stub
//
int32_t pal_wsclient_connect(
    pal_wsclient_t* wsclient
)
{
    (void)wsclient;
    dbg_assert(0, "Not supported");
    return er_not_supported;
}

//
// Linker stub
//
int32_t pal_wsclient_can_recv(
    pal_wsclient_t* wsclient,
    bool enable
)
{
    (void)wsclient;
    (void)enable;
    dbg_assert(0, "Not supported");
    return er_not_supported;
}

//
// Linker stub
//
int32_t pal_wsclient_can_send(
    pal_wsclient_t* wsclient,
    bool enable
)
{
    (void)wsclient;
    (void)enable;
    dbg_assert(0, "Not supported");
    return er_not_supported;
}

//
// Linker stub
//
int32_t pal_wsclient_disconnect(
    pal_wsclient_t* wsclient
)
{
    (void)wsclient;
    dbg_assert(0, "Not supported");
    return er_not_supported;
}

//
// Linker stub
//
void pal_wsclient_close(
    pal_wsclient_t* wsclient
)
{
    (void)wsclient;
    dbg_assert(0, "Not supported");
}
