// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef prx_client_h_
#define prx_client_h_

#include "common.h"
#include "prx_types.h"

//
// Client api to create and communicate with remote socket
//

//
// Convert an address string into an address
//
decl_public_2(int32_t, prx_client_pton,
    const char*, addr_string,
    prx_socket_address_t*, address
);

//
// Convert an address to a string
//
decl_public_3(int32_t, prx_client_ntop,
    prx_socket_address_t*, address,
    char*, addr_string,
    size_t, addr_string_size
);

//
// Look up interface addresses
//
decl_public_4(int32_t, prx_client_getifaddrinfo,
    const char*, if_name,
    uint32_t, flags,
    prx_ifaddrinfo_t**, info,
    size_t*, info_count
);

//
// Frees interface address info
//
decl_public_1(int32_t, prx_client_freeifaddrinfo,
    prx_ifaddrinfo_t*, info
);

//
// Inverse of getifaddrinfo, converts interface address
//
decl_public_4(int32_t, prx_client_getifnameinfo,
    prx_socket_address_t*, if_address,
    char*, if_name,
    size_t, if_name_length,
    uint64_t*, if_index
);

//
// Look up socket address info
//
decl_public_6(int32_t, prx_client_getaddrinfo,
    const char*, host_name,
    const char*, service,
    prx_address_family_t, family,
    uint32_t, flags,
    prx_addrinfo_t**, info,
    size_t*, info_count
);

//
// Frees address info
//
decl_public_1(int32_t, prx_client_freeaddrinfo,
    prx_addrinfo_t*, info
);

//
// Inverse of getaddrinfo, from socket address to host
//
decl_public_6(int32_t, prx_client_getnameinfo,
    prx_socket_address_t*, address,
    char*, host,
    size_t, host_length,
    char*, service,
    size_t, service_length,
    int32_t, flags
);

//
// Create a socket
//
decl_public_4(int32_t, prx_client_socket,
    prx_address_family_t, address_family,
    prx_socket_type_t, socket_type,
    prx_protocol_type_t, protocol_type,
    prx_fd_t*, fd
);

//
// Start to listen
//
decl_public_2(int32_t, prx_client_listen,
    prx_fd_t, fd,
    int32_t, backlog
);

//
// Bind the socket
//
decl_public_2(int32_t, prx_client_bind,
    prx_fd_t, fd,
    prx_socket_address_t*, socket_address
);

//
// Connect the socket
//
decl_public_2(int32_t, prx_client_connect,
    prx_fd_t, fd,
    prx_socket_address_t*, socket_address
);

//
// getpeername
//
decl_public_2(int32_t, prx_client_getpeername,
    prx_fd_t, fd,
    prx_socket_address_t*, socket_address
);

//
// Get socket address info
//
decl_public_2(int32_t, prx_client_getsockname,
    prx_fd_t, fd,
    prx_socket_address_t*, socket_address
);

//
// Accepts a new socket
//
decl_public_4(int32_t, prx_client_accept,
    prx_fd_t, fd,
    uintptr_t, key,
    prx_socket_address_t*, socket_address,
    prx_fd_t*, accepted_socket
);

//
// Receive
//
decl_public_7(int32_t, prx_client_recv,
    prx_fd_t, fd,
    uintptr_t, key,
    int32_t, flags,
    uint8_t*, buffer,
    size_t, offset,
    size_t, length,
    size_t*, received
);

//
// Receive from 
//
decl_public_8(int32_t, prx_client_recvfrom,
    prx_fd_t, fd,
    uintptr_t, key,
    int32_t, flags,
    uint8_t*, buffer,
    size_t, offset,
    size_t, length,
    prx_socket_address_t*, socket_address,
    size_t*, received
);

//
// Send
//
decl_public_7(int32_t, prx_client_send,
    prx_fd_t, fd,
    uintptr_t, key,
    int32_t, flags,
    uint8_t*, buffer,
    size_t, offset,
    size_t, length,
    size_t*, sent
);

//
// Send to
//
decl_public_8(int32_t, prx_client_sendto,
    prx_fd_t, fd,
    uintptr_t, key,
    int32_t, flags,
    uint8_t*, buffer,
    size_t, offset,
    size_t, length,
    prx_socket_address_t*, address,
    size_t*, sent
);

//
// Get socket option
//
decl_public_3(int32_t, prx_client_getsockopt,
    prx_fd_t, fd,
    prx_socket_option_t, option,
    uint64_t*, value
);

//
// Set socket option
//
decl_public_3(int32_t, prx_client_setsockopt,
    prx_fd_t, fd,
    prx_socket_option_t, option,
    uint64_t, value
);

//
// Wait for an activity on any of the passed in sockets
//
decl_public_3(int32_t, prx_client_poll,
    size_t, num,
    prx_fd_t*, sockets,
    int32_t*, timeout_ms
);

//
// True if data is available to receive
//
decl_public_1(bool, prx_client_can_recv,
    prx_fd_t, fd
);

//
// True if ready to send data
//
decl_public_1(bool, prx_client_can_send,
    prx_fd_t, fd
);

//
// returns true if fd is in error or closed condition
//
decl_public_1(bool, prx_client_has_error,
    prx_fd_t, fd
);

//
// returns true if fd is in error or closed condition
//
decl_public_1(bool, prx_client_is_disconnected,
    prx_fd_t, fd
);

//
// Closes the socket
//
decl_public_1(int32_t, prx_client_close,
    prx_fd_t, fd
);

//
// Initialize
//
decl_public_1(int32_t, prx_client_init,
    const char*, config
);

//
// De-initialize
//
decl_public_0(int32_t, prx_client_deinit,
    void
);

#endif // prx_client_h_
