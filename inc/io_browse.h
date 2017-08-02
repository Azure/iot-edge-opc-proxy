// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _io_browse_h_
#define _io_browse_h_

#include "common.h"
#include "io_codec.h"
#include "io_ref.h"

//
// Browse protocol is implemented on top of proxy link protocol
// and works as a simple request / response protocol. A command
// sequence is identified by a reference id.  A command is send
// and responses are received as messages, until eof or command
// is cancelled.
//

//
// Type of request - service, address resolver, folders
//
typedef enum io_browse_request_type
{
    io_browse_request_cancel = 0, // Cancel request with handle

    io_browse_request_resolve,     // Resolve string to address
    io_browse_request_service,               // Browse services
    io_browse_request_dirpath,            // Recurse a dir path
    io_browse_request_ipscan,                   // Scan network
    io_browse_request_portscan      // Scan host for open ports

    // ...
}
io_browse_request_type_t;

//
// Flags determine aspects of request
//
typedef enum io_browse_request_flags
{
    io_browse_response_cache_only = 0x1    // Only browse cache
}
io_browse_request_flags_t;

//
// Browse service request
//
typedef struct io_browse_request
{
    io_ref_t handle;                   // browse session handle
#define PRX_BROWSE_VERSION 1
    uint8_t version;         // Version of the command protocol
    io_browse_request_type_t type;    // Type of browse request
    uint32_t flags;
    prx_socket_address_t item;                // Item to browse
}
io_browse_request_t;

//
// Flags determine aspects of response
//
typedef enum io_browse_response_flags
{
    io_browse_response_eos = 0x1,      // End of stream reached
    io_browse_response_removed = 0x2,           // Item removed
    io_browse_response_allfornow = 0x4,      // Cache exhausted
    io_browse_response_empty = 0x8            // Empty response
}
io_browse_response_flags_t;

//
// Browse service response
//
typedef struct io_browse_response
{
    io_ref_t handle;                   // browse session handle
    uint32_t flags;
    int32_t error_code;                            // Exception
    prx_socket_address_t item;                 // Item returned
    size_t props_size;           // Optional meta data for item
    prx_property_t* props;
}
io_browse_response_t;

//
// Encode browse request
//
decl_public_2(int32_t, io_encode_browse_request,
    io_codec_ctx_t*, ctx,
    const io_browse_request_t*, prx_br
);

//
// Decode browse request
//
decl_public_2(int32_t, io_decode_browse_request,
    io_codec_ctx_t*, ctx,
    io_browse_request_t*, prx_br
);

//
// Encode browse response
//
decl_public_2(int32_t, io_encode_browse_response,
    io_codec_ctx_t*, ctx,
    const io_browse_response_t*, prx_br
);

//
// Decode browse response
//
decl_public_2(int32_t, io_decode_browse_response,
    io_codec_ctx_t*, ctx,
    io_browse_response_t*, prx_br
);


#endif // _io_browse_h_