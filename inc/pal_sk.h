// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _pal_socket_h_
#define _pal_socket_h_

#include "common.h"
#include "prx_types.h"

//
// Represents a socket
//
typedef struct pal_socket pal_socket_t;

//
// Socket client events
//
typedef enum pal_socket_event
{
    pal_socket_event_unknown = 0,
    pal_socket_event_opened,                           // Socket opened
    pal_socket_event_begin_accept,         // Requests client interface
    pal_socket_event_end_accept,          // Sends newly created socket
    pal_socket_event_begin_recv,            // Requests an empty buffer
    pal_socket_event_end_recv,           // Sent when buffer was filled
    pal_socket_event_begin_send,            // Requests a filled buffer
    pal_socket_event_end_send,                // Filled buffer was sent
    pal_socket_event_closed                            // Socket closed
}
pal_socket_event_t;

//
// Called by when event on socket occurred
//
typedef void (*pal_socket_event_handler_t)(
    void* context,
    pal_socket_event_t ev,
    uint8_t** buffer,               // The buffer for the event or null
    size_t* size,              // Size of buffer or amount read/written
    prx_socket_address_t* addr,         // Socket address if applicable
    int32_t* flags,                       // Flags describing operation
    int32_t error,         // Error that occurred, e.g. as result of op
    void** op_context              // A user provided operation context
    );

//
// Client interface for socket passed in begin_accept cb or create
//
typedef struct pal_socket_client_itf
{
    prx_socket_properties_t props;
    pal_socket_event_handler_t cb;
    void* context;
}
pal_socket_client_itf_t;

//
// Call before using socket layer
//
decl_public_1(int32_t, pal_socket_init,
    uint32_t*, caps
);

//
// Create socket and bind to passed in client interface
//
decl_internal_2(int32_t, pal_socket_create,
    pal_socket_client_itf_t*, itf,
    pal_socket_t**, sock
);

//
// Open a socket - wait for opened event
//
decl_internal_2(int32_t, pal_socket_open,
    pal_socket_t*, sock,
    const char*, itf_name
);

//
// Create an opened / connected pair of local sockets
//
decl_internal_4(int32_t, pal_socket_pair,
    pal_socket_client_itf_t*, itf1,
    pal_socket_t**, sock1,
    pal_socket_client_itf_t*, itf2,
    pal_socket_t**, sock2
);

//
// Get socket properties
//
decl_internal_2(int32_t, pal_socket_get_properties,
    pal_socket_t*, sock,
    prx_socket_properties_t*, props
);

//
// Get peer address
//
decl_internal_2(int32_t, pal_socket_getpeername,
    pal_socket_t*, sock,
    prx_socket_address_t*, socket_address
);

//
// Get local address
//
decl_internal_2(int32_t, pal_socket_getsockname,
    pal_socket_t*, sock,
    prx_socket_address_t*, socket_address
);

//
// Notify socket that caller is ready to send
//
decl_internal_2(int32_t, pal_socket_can_send,
    pal_socket_t*, sock,
    bool, ready
);

//
// Notify socket that caller is ready to recv
//
decl_internal_2(int32_t, pal_socket_can_recv,
    pal_socket_t*, sock,
    bool, ready
);

//
// Set socket option
//
decl_internal_3(int32_t, pal_socket_setsockopt,
    pal_socket_t*, sock,
    prx_socket_option_t, socket_option,
    uint64_t, value
);

//
// Get socket option
//
decl_internal_3(int32_t, pal_socket_getsockopt,
    pal_socket_t*, sock,
    prx_socket_option_t, socket_option,
    uint64_t*, value
);

//
// Join multicast group
//
decl_public_2(int32_t, pal_socket_join_multicast_group,
    pal_socket_t*, sock,
    const prx_multicast_option_t*, option
);

//
// Leave multicast group
//
decl_public_2(int32_t, pal_socket_leave_multicast_group,
    pal_socket_t*, sock,
    const prx_multicast_option_t*, option
);

//
// Close socket - wait for close event before calling free
//
decl_internal_1(void, pal_socket_close,
    pal_socket_t*, socket
);

//
// Free the socket
//
decl_internal_1(void, pal_socket_free,
    pal_socket_t*, sock
);

//
// Free the global socket layer
//
decl_public_0(void, pal_socket_deinit,
    void
);

#endif // _pal_socket_h_
