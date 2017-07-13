// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _io_proto_h_
#define _io_proto_h_

#include "common.h"
#include "prx_types.h"

#include "io_ref.h"
#include "io_codec.h"
#include "prx_buffer.h"

#include "azure_c_shared_utility/doublylinkedlist.h"

//
// A message factory creates ...
//
typedef struct io_message_factory io_message_factory_t;

//
// ... protocol messages
//
typedef struct io_message io_message_t;

//
// Create message factory - a pool of reusable messages
//
decl_internal_8(int32_t, io_message_factory_create,
    const char*, name,
    size_t, initial_pool_size,
    size_t, max_pool_size,
    size_t, low_watermark,
    size_t, high_watermark,
    prx_buffer_pool_cb_t, cb,
    void*, context,
    io_message_factory_t**, factory
);

//
// Free message factory
//
decl_internal_1(void, io_message_factory_free,
    io_message_factory_t*, factory
);

//
// Create new  message
//
decl_internal_5(int32_t, io_message_create,
    io_message_factory_t*, factory,
    int32_t, type,
    io_ref_t*, source,
    io_ref_t*, target,
    io_message_t**, message
);

//
// Create empty message for decoding
//
decl_internal_2(int32_t, io_message_create_empty,
    io_message_factory_t*, factory,
    io_message_t**, message
);

//
// Clone message
//
decl_internal_2(int32_t, io_message_clone,
    io_message_t*, original,
    io_message_t**, created
);

//
// Encode message to stream
//
decl_internal_2(int32_t, io_encode_message,
    io_codec_ctx_t*, ctx,
    const io_message_t*, message
);

//
// Decode a message from stream
//
decl_internal_2(int32_t, io_decode_message,
    io_codec_ctx_t*, ctx,
    io_message_t*, message
);

//
// Make response message out of request message
//
decl_internal_1(void, io_message_as_response,
    io_message_t*, message
);

//
// Allocate memory from the message object for dynamic payload
//
decl_internal_3(int32_t, io_message_allocate_buffer,
    io_message_t*, message,
    size_t, size,
    void**, mem
);

//
// Release message
//
decl_internal_1(void, io_message_release,
    io_message_t*, message
);

//
// Proxy protocol, defines content sent to rpc socket communication.
// More explanation per content item below.
//
// Keep in sync with managed layer, in particular order of members!
//

decl_g(const uint32_t, io_message_type_ping,                    10);
decl_g(const uint32_t, io_message_type_link,                    12);
decl_g(const uint32_t, io_message_type_setopt,                  13);
decl_g(const uint32_t, io_message_type_getopt,                  14);
decl_g(const uint32_t, io_message_type_open,                    20);
decl_g(const uint32_t, io_message_type_close,                   21);
decl_g(const uint32_t, io_message_type_data,                    30);
decl_g(const uint32_t, io_message_type_poll,                    31);

// ... more socket server messages

//
// Returns the name for the type
//
decl_internal_1(const char*, io_message_type_as_string,
    uint32_t, type
);

//
// Ping messages, are used to ping hosts on proxys. Ping requests 
// are broadcasts containing a proxy address to ping. Responses 
// are only sent when host was reached. Responses include MAC 
// address to uniquely identify the target, and the latency.
//

//
// Ping request
//
typedef struct io_ping_request
{
    prx_socket_address_t address;
}
io_ping_request_t;

//
// Ping response
//
typedef struct io_ping_response
{
    prx_socket_address_t address;
    uint8_t physical_address[8];
    uint32_t time_ms;
}
io_ping_response_t;

//
// Link requests are used to create a link between caller and 
// host:port in the form of a proxied socket.  Sockets are 
// opened according to the socket properties passed.  Future
// versions of link request messages will contain a connection
// string that binds the link to a unique device identity.
// Link responses contain the address of the link (for open,
// setopt, getopt, and close requests), as well as the proxy
// side local and peer addresses (only valid on proxy)
//

//
// Link request
//
typedef struct io_link_request
{
#define LINK_VERSION 7
    uint8_t version;
    prx_socket_properties_t props;
}
io_link_request_t;

//
// Link response
//
typedef struct io_link_response
{
    io_ref_t link_id;
    prx_socket_address_t local_address;
    prx_socket_address_t peer_address;
}
io_link_response_t;

//
// Allows to set options on a link, which correspond to local
// socket options which have a value of less than 64 bit (not 
// multicast join/leave, which are currently not supported).
// Responses are exception responses only (i.e. The error_code
// member of message is set to something other than er_ok.
//

//
// Set option request
//
typedef struct io_setopt_request
{
    prx_property_t so_val;
}
io_setopt_request_t;

//
// Allows to retrieve an option value from the proxied link. Not 
// all options can be retrieved.  Responses contain the option 
// value.
//

//
// Get option request
//
typedef struct io_getopt_request
{
    prx_socket_option_t so_opt;
}
io_getopt_request_t;

//
// Get option response
//
typedef struct io_getopt_response
{
    prx_property_t so_val;
}
io_getopt_response_t;

//
// Open requests open a channel for asynchronous messages on the 
// link, which mainly include data messages.  For this the open
// request carries a connection string that points to an endpoint.
// Responses are exception responses only (i.e. error_code
// are set to something other than er_ok, e.g. in the case the
// connection could not be established to the web socket endpoint.
//

//
// Open request - opens the link
//
typedef struct io_open_request
{
    io_ref_t stream_id;
    int32_t encoding;
    int32_t type;
    const char* connection_string;
    bool polled;
    uint32_t max_recv;
}
io_open_request_t;

//
// Poll messages allow streaming in polled mode (see open). A poll
// request specifies how long to wait for data to arrive. A response 
// does not contain a timeout.
//
typedef struct io_poll_message
{
    uint64_t sequence_number;
    uint64_t timeout;
}
io_poll_message_t;

//
// Stream data messages are sent either as a request to write, a
// response to a poll request, or unsolicited on an asynchronous 
// stream. They contain the last successful read of the underlying 
// proxied socket.
//

//
// Stream data message payload
//
typedef struct io_data_message
{
    uint64_t sequence_number;
    prx_socket_address_t source_address;
    size_t control_buffer_length;
    uint8_t* control_buffer;
    size_t buffer_length;
    uint8_t* buffer;
}
io_data_message_t;

//
// Close requests request closing of the channel and underlying 
// link i.e. there is no link close message. The response error 
// code has a socket error code, e.g. if the socket was closed 
// from remote side.
//

//
// close response
//
typedef struct io_close_response
{
    uint64_t time_open;
    uint64_t bytes_sent;
    uint64_t bytes_received;
    int32_t error_code;
}
io_close_response_t;


//
// A proxy protocol message is a union of all protocol messages
//
struct io_message 
{
    io_ref_t source_id;             // Source proxy address and
    io_ref_t proxy_id;     // Proxy the message is sent through
    io_ref_t target_id;            // Target address of message
    uint32_t seq_id;         // A Sequence number for debugging
    int32_t error_code;                            // Exception
    bool is_response;           // Request or response message?
    uint32_t type;               // Identifies the content type
    union {                        // Message content structure
    io_ping_request_t ping_request;
    io_ping_response_t ping_response;
    io_link_request_t link_request;
    io_link_response_t link_response;
    io_setopt_request_t setopt_request;
    io_getopt_request_t getopt_request;
    io_getopt_response_t getopt_response;
    io_open_request_t open_request;
    io_poll_message_t poll_message;
    io_data_message_t data_message;
    io_close_response_t close_response;
    } content;                                 // Internal: ...
    io_message_factory_t* owner;              // Owning factory
    void* buffer;       // Memory for dynamic allocated content
    int correlation_id;      // The request response session id
    void* context;                            // Opaque storage
    DLIST_ENTRY link;        // Link to queue protocol messages
};

#endif // _io_proto_h_