// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "prx_client.h"
#include "prx_types.h"
#include "util_misc.h"
#include "util_string.h"
#include "io_proto.h"
#include "io_host.h"

int ____i = 0;

#if 0

#include "azure_c_shared_utility/refcount.h"

//
// An object representing a virtual socket
//
typedef struct prx_client_socket
{
    io_ref_t l_addr;                                 // Local address
    io_ref_t c_addr;                    // Address sock is bound to
    io_ref_t r_addr;                         // Remote target address

    int32_t sock_fd;             
    prx_socket_properties_t props;          // Properties of the socket
    prx_socket_address_t r_name;         // Remote bound socket address
    prx_socket_address_t r_peer;          // Remote peer socket address

    bool connected;                                // socket is linked
  //  io_cm_interface_t* itf;                        // Loaded interface
  //  io_channel_t* stream;  // A channel here means socket is streaming

    uint8_t* recv_buffer;
    size_t recv_buffer_size;
    size_t recv_buffer_offset;
    uint32_t recv_credit;
    uint32_t recv_count;
    uint32_t send_credit;
    uint32_t send_count;

    int32_t control_timeout;        // Default control message timeout
    int32_t timeout;                  // Default read or write timeout
    prx_host_t* host;
    log_t log;
}
prx_client_socket_t;

DEFINE_REFCOUNT_TYPE(prx_client_socket_t);

// Note: Handle map must have been inited

//
// Global state for prx_client_socket
//
static struct
{
    prx_host_t* host;
    bool init;
}
global_state = { 0 };

//
// Logging helper macro
//
#define prx_client_socket_log_error(s, FMT, ...) \
    log_error(s->log, "V-Socket " IO_REF_FMT " (c: %p, if: " IO_REF_FMT ") " FMT, \
        IO_REF_PRINT(&s->r_addr), NULL/*s->stream*/, IO_REF_PRINT(&s->c_addr), __VA_ARGS__ );
#define prx_client_socket_log_trace(s, FMT, ...) \
    log_trace(s->log, "V-Socket " IO_REF_FMT " (c: %p, if: " IO_REF_FMT ") " FMT, \
        IO_REF_PRINT(&s->r_addr), NULL/*s->stream*/, IO_REF_PRINT(&s->c_addr), __VA_ARGS__ );

//
// Release the socket 
//
static void prx_client_socket_release(
    prx_client_socket_t* sock
)
{
    if (!sock)
        return;

    if (DEC_REF(prx_client_socket_t, sock) == DEC_RETURN_ZERO)
    {
        if (sock->host)
            prx_host_release(sock->host);

      //  if (sock->stream)
      //      io_channel_close(sock->stream);
      //
      //  if (sock->itf)
      //      io_cm_interface_release(sock->itf);

        if (sock->recv_buffer)
            mem_free(sock->recv_buffer);

        REFCOUNT_TYPE_FREE(prx_client_socket_t, sock);
    }
}

//
// Allocate a socket object
//
static int32_t prx_client_socket_create(
    prx_host_t* host,
    prx_address_family_t family,
    prx_socket_type_t sock_type,
    prx_protocol_type_t proto_type,
    prx_client_socket_t** created
)
{
    const uint32_t k_default_recv_credit = 16;
    const uint32_t k_default_send_credit = (uint32_t)~0;

    int32_t result;
    prx_client_socket_t* sock;

    sock = REFCOUNT_TYPE_CREATE(prx_client_socket_t);
    if (!sock)
        return er_out_of_memory;
    memset(sock, 0, sizeof(prx_client_socket_t));
    do
    {
        sock->log = log_get("client_sk");
        sock->timeout = 60000;
#ifdef _DEBUG
        sock->control_timeout = 60000;
#else
        sock->control_timeout = 20000;
#endif
        sock->props.family = family;
        sock->props.sock_type = sock_type;
        sock->props.proto_type = proto_type;

        sock->recv_credit = k_default_recv_credit;
        sock->send_credit = k_default_send_credit;

        // Set local address
        result = prx_host_clone(host, &sock->host);
        if (result != er_ok)
            break;
        io_ref_copy(prx_host_get_id(host), &sock->l_addr);

        sock->sock_fd = handle_map_get_handle(sock);
        if (sock->sock_fd == handle_map_invalid_handle)
        {
            result = er_out_of_memory;
            prx_client_socket_log_error(sock, "failed to get handle (%s)",
                prx_err_string(result));
            break;
        }

        // Take a ref count for the handle map
        INC_REF(prx_client_socket_t, sock);
        *created = sock;
        return er_ok;

    } while (0);

    prx_client_socket_release(sock);
    return result;
}

//
// Get a ref counted instance of socket by id
//
static prx_client_socket_t* prx_client_socket_get_by_id(
    int32_t id,
    bool check_open
)
{
    prx_client_socket_t* sock = NULL;

    sock = (prx_client_socket_t*)handle_map_get_pointer(id);
    if (!sock)
    {
        log_error(NULL, 
            "Virtual socket handle %d is not valid, it was likely closed.", id);
        return NULL;
    }

    if (check_open && !sock->connected)
    {
        prx_client_socket_log_error(sock, 
            "Attempt to operate on closed sock (id:%d).", id);
        return NULL;
    }

    //
    // This is ok, since the handle map has a ref count, too.
    // Once prx_client_close is called and the handle is removed
    // The sock will die.
    //
    INC_REF(prx_client_socket_t, sock);
    return sock;
}

//
// Invalidates the socket by detaching from interface and stream
//
static void prx_client_socket_invalidate(
    prx_client_socket_t* sock
)
{
    io_ref_clear(&sock->c_addr);
    io_ref_clear(&sock->r_addr);

    sock->connected = false;

   // if (sock->stream)
   // {
   //     io_channel_close(sock->stream);
   //     sock->stream = NULL;
   // }
   //
   // if (sock->itf)
   // {
   //     io_cm_interface_release(sock->itf);
   //     sock->itf = NULL;
   // }

    memset(&sock->r_peer, 0, sizeof(prx_socket_address_t));
    memset(&sock->r_name, 0, sizeof(prx_socket_address_t));
}

//
// Open a new port
//
static int32_t prx_client_socket_create_channel(
    prx_client_socket_t* sock,
    uint32_t message_type,
    io_channel_t** channel
)
{
    int32_t result;
    
    // Assign a new channel/session id per created channel
    result = io_channel_create(NULL, &sock->r_addr, &sock->l_addr,
        message_type, channel);

    if (result != er_ok)
    {
        prx_client_socket_log_error(sock, "Failed to allocate port (%s).",
            prx_err_string(result));
    }

    else if (sock->itf)
    {
        result = io_channel_connect_to_interface(*channel, sock->itf);
    }

    return result;
}

//
// Helper macro for request/response operation
//
#define __prx_client_socket_invoke(request, response, timeout, log) \
    result = io_channel_begin_write(control, 0, &message); \
    if (result != er_ok) \
        break; \
    __io_proto_pack(message, request, log); \
    if (result != er_ok) { \
        (void)io_channel_abort_write(control, message); \
        break; \
    } \
    result = io_channel_complete_write(control, message, true); \
    if (result != er_ok) \
        break; \
    result = io_channel_begin_read(control, timeout, &message); \
    if (result != er_ok) \
        break; \
    __io_proto_unpack(message, response, log); \
    if (result != er_ok) { \
        (void)io_channel_complete_read(control); \
        break; \
    } \
    result = io_channel_complete_read(control); \
    if (result != er_ok) \
        break;

//
// Undo link if possible
//
static void prx_client_socket_unlink(
    prx_client_socket_t* sock
)
{
    int32_t result;
    io_message_t* message;
    io_channel_t* control;
    void* void_request = NULL;

    dbg_assert_ptr(sock);
    dbg_assert_ptr(sock->host);
    dbg_assert(sock->itf && !io_ref_equals(&sock->c_addr, &io_ref_null),
        "No interface selected");
    dbg_assert(sock->connected && !io_ref_equals(&sock->r_addr, &io_ref_null),
        "Not connected");
    do
    {
        result = prx_client_socket_create_channel(sock, io_message_type_close, &control);
        if (result != er_ok)
            break;

        result = io_channel_begin_write(control, 0, &message);
        if (result != er_ok)
            break;
        __io_proto_pack(message, void_request, sock->log);
        if (result != er_ok)
        {
            (void)io_channel_abort_write(control, message); 
            break;
        }
        // Just wait for it to be sent, then we are done...
        result = io_channel_complete_write(control, message, true);
        if (result != er_ok)
            break;

    } while (0);

    if (control)
        io_channel_close(control);

    prx_client_socket_invalidate(sock);
}

//
// Send link request to bind or connect the remote socket
//
static int32_t prx_client_socket_link(
    prx_client_socket_t* sock,
    prx_ns_entry_t* host,
    uint16_t port
)
{
    int32_t result;
    io_message_t* message = NULL;
    io_channel_t* control = NULL;
    io_link_request_t link_request;
    io_link_response_t link_response;

    dbg_assert_ptr(host);
    dbg_assert_ptr(sock);
    dbg_assert(sock->itf && !io_ref_equals(&sock->c_addr, &io_ref_null), 
        "No interface selected");
    dbg_assert(!sock->connected && io_ref_equals(&sock->r_addr, &io_ref_null),
        "Connected or remote address already set");
    do
    {   
        result = prx_client_socket_create_channel(sock, io_message_type_link, &control);
        if (result != er_ok)
            break;

        memset(&link_request, 0, sizeof(link_request));

        //
        // The handshake includes sending the sock's properties which 
        // include requested address and flags, and receiving back the real 
        // addresses as response.
        //
        memcpy(&link_request.props, &sock->props, sizeof(link_request.props));
        link_request.version = LINK_VERSION;
        result = prx_ns_entry_to_prx_socket_address(
            host, prx_address_family_proxy, &link_request.props.address);
        if (result != er_ok)
            break;
        link_request.props.address.un.ip.port = port;

        __prx_client_socket_invoke(
            link_request, link_response, sock->control_timeout, sock->log);

        // Now set remote address of the socket and mark connected
        io_ref_copy(&link_response.address, &sock->r_addr);
        sock->connected = true;

        memcpy(&sock->r_name, &link_response.local, sizeof(prx_socket_address_t));
        memcpy(&sock->r_peer, &link_response.peer, sizeof(prx_socket_address_t));

        result = er_ok;

    } while (0);

    if (result != er_ok)
    {
        prx_client_socket_log_error(sock, "Failed to send/receive link request/response (%s).",
            prx_err_string(result));
    }

    if (control)
        io_channel_close(control);
    return result;
}

//
// Open linked socket
//
static int32_t prx_client_socket_open(
    prx_client_socket_t* sock
)
{
    int32_t result;
    io_message_t* message = NULL;
    io_channel_t* control = NULL;
    io_open_request_t open_request;
    void* void_response;

    dbg_assert_ptr(sock);
    dbg_assert_ptr(sock->host);
    dbg_assert_ptr(!sock->stream);

    dbg_assert(sock->itf && !io_ref_equals(&sock->c_addr, &io_ref_null),
        "No interface selected");
    dbg_assert(sock->connected && !io_ref_equals(&sock->r_addr, &io_ref_null),
        "Not connected");
    do
    {
        // Open control channel to send open request
        result = prx_client_socket_create_channel(sock, io_message_type_open, &control);
        if (result != er_ok)
            break;

        //
        // Create stream channel using socket remote address as channel id
        // This is done to simplify message matching on the device host since
        // the socket server has to handle multiple control channels and only
        // one stream channel.
        //
        result = io_ref_new(&open_request.stream_id);
        if (result != er_ok)
            break;
        result = io_channel_create(&sock->r_addr, &open_request.stream_id, 
            &sock->l_addr, io_message_type_data, &sock->stream);
        if (result != er_ok)
        {
            prx_client_socket_log_error(sock, "Failed to allocate stream (%s).",
                prx_err_string(result));
            break;
        }

        result = io_channel_connect_to_interface(sock->stream, sock->itf);
        if (result != er_ok)
            break;

        open_request.recv_credit = sock->recv_credit;
        open_request.send_credit = sock->send_credit;

        __prx_client_socket_invoke(
            open_request, void_response, sock->control_timeout, sock->log);

        // Done, now we can send and receive...
    }
    while (0);

    if (result != er_ok)
    {
        prx_client_socket_log_error(sock, 
            "Failed to send/receive open request/response (%s).",
            prx_err_string(result));

        if (sock->stream)
        {
            io_channel_close(sock->stream);
            sock->stream = NULL;
        }
    }

    if (control)
        io_channel_close(control);

    return result;
}

//
// Connect socket
//
static int32_t prx_client_socket_connect_bound(
    prx_client_socket_t* sock,
    prx_ns_entry_t* host,
    uint16_t port
)
{
    int32_t result;
    prx_ns_entry_t* host = NULL;

    dbg_assert_ptr(sock);
    dbg_assert_ptr(sock->host);
    dbg_assert_ptr(!sock->stream);
    dbg_assert(!io_ref_equals(&sock->c_addr, &io_ref_null), 
        "No interface selected");
    dbg_assert_ptr(!sock->itf);
    do
    {
        result = prx_ns_get_entry_by_addr(
            prx_host_get_ns(sock->host), &sock->c_addr, &host);
        if (result != er_ok)
            break;

        result = io_cm_get_interface(
            prx_host_get_cm(sock->host), host, &sock->itf);
        if (result != er_ok)
            break;

        prx_ns_entry_release(host);
        host = NULL;

        result = prx_client_socket_link(sock, host, port);
        if (result != er_ok)
            break;

        result = prx_client_socket_open(sock);
        if (result != er_ok)
            break;

        return er_ok;
    } 
    while (0);

    if (host)
        prx_ns_entry_release(host);
    if (sock->connected)
        prx_client_socket_unlink(sock);
    if (sock->itf)
    {
        io_cm_interface_release(sock->itf);
        sock->itf = NULL;
    }
    return result;
}

//
// Open the stream
//
static int32_t prx_client_socket_connect_local(
    prx_client_socket_t* sock,
    io_ref_t* r_addr
)
{
    int32_t result;

    chk_arg_fault_return(r_addr);
    chk_arg_fault_return(sock);
    if (!sock->host)
        return er_bad_state;
    if (sock->stream)
        return er_already_exists;
    do
    {
        dbg_assert(io_ref_equals(&sock->c_addr, &io_ref_null),
            "An interface was selected");

        io_ref_copy(r_addr, &sock->r_addr);

        // Bind the socket to a stream channel 
        result = prx_client_socket_create_channel(
            sock, io_message_type_data, &sock->stream);

        if (result != er_ok)
            break;

        sock->connected = true;
        return er_ok;
    } while (0);

    io_ref_clear(&sock->l_addr);
    io_ref_clear(&sock->r_addr);
    return result;
}

//
// Dicover and connect to first host on a route
//
static int32_t prx_client_socket_connect_unbound(
    prx_client_socket_t* sock,
    prx_ns_entry_t* host,
    uint16_t port
)
{
    int32_t result;
    io_message_t* message = NULL;
    prx_ns_result_t* routes = NULL;
    io_ping_request_t ping_request;
    io_ping_response_t ping_response;
    io_channel_t* control = NULL;
    prx_ns_entry_t* host = NULL;
    io_cm_interface_t* itf = NULL;
    int32_t num_response = 0;

    dbg_assert_ptr(sock);
    dbg_assert_ptr(sock->host);
    dbg_assert_ptr(!sock->stream);
    dbg_assert_ptr(host);

    dbg_assert(io_ref_equals(&sock->c_addr, &io_ref_null), "interface must be unbound");
    do
    {
        // If no routes configured, use all interfaces
        result = prx_ns_entry_get_routes(host, &routes);
        if (result == er_not_found)
            result = prx_ns_get_entry_by_type(prx_host_get_ns(sock->host), 
                prx_ns_entry_type_proxy, &routes);
        if (result != er_ok)
            break;

        // Open control and subscribed stream channel
        result = prx_client_socket_create_channel(sock, io_message_type_ping, &control);
        if (result != er_ok)
            break;

        // For each route, attach the corresponding interface to the channel
        while (true)
        {
            host = prx_ns_result_pop(routes);
            if (!host)
                break;
            result = io_cm_get_interface(prx_host_get_cm(sock->host), host, &itf);
            if (result != er_ok)
                break;
            result = io_channel_connect_to_interface(control, itf);
            io_cm_interface_release(itf);
            itf = NULL;
            prx_ns_entry_release(host);
            host = NULL;
            if (result != er_ok)
                break;
        }

        // Broadcast ping request to all interfaces
        result = io_channel_begin_write(control, 0, &message);
        if (result != er_ok)
            break;
        result = prx_ns_entry_to_prx_socket_address(
            host, prx_address_family_proxy, &ping_request.address);
        if (result != er_ok)
            break;
        ping_request.address.un.host.port = port;
        __io_proto_pack(message, ping_request, sock->log);
        if (result != er_ok)
        {
            io_channel_abort_write(control, message);
            break;
        }
        result = io_channel_complete_write(control, message, true);
        if (result != er_ok)
            break;

        // Now we try to link on the fastest returned ping interface
        while (true)
        {
            // Ping responses are only sent in case of successfully pinging a remote peer
            if (er_ok != io_channel_begin_read(control, 
                num_response == 0 ? sock->control_timeout : 0, &message))
            {
                if (num_response == 0)
                    result = er_timeout;
                break;
            }

            num_response++;
            __io_proto_unpack(message, ping_response, sock->log);
            if (result == er_ok)
                io_ref_copy(&message->interface_id, &sock->c_addr);
            io_channel_complete_read(control);
            if (result != er_ok)
                continue;

            // TODO: Check and compare returned MAC address to ensure uniqueness

            // Link through interface
            result = prx_client_socket_connect_bound(sock, host, port);
            if (result == er_ok)
                break;
            io_ref_clear(&sock->c_addr);
            continue;
        }

        if (result != er_ok)
            break;
    }
    while (0);

    if (result != er_ok)
    {
        prx_client_socket_log_error(sock, "Failed to connect on unbound sock (%s).", 
            prx_err_string(result));
    }

    if (host)
        prx_ns_entry_release(host);
    if (itf)
        io_cm_interface_release(itf);
    if (routes)
        prx_ns_result_release(routes);
    if (control)
        io_channel_close(control);
    return result;
}

//
// Connect synchronously
//
static int32_t prx_client_socket_connect(
    prx_client_socket_t* sock,
    prx_socket_address_t* sa
)
{
    int32_t result;
    prx_ns_entry_t* host = NULL;
    io_ref_t r_addr;


    dbg_assert(sock->props.sock_type == prx_socket_type_stream, "");

    do
    {
        if (sa->un.family != prx_address_family_proxy)
        {
            result = io_ref_from_prx_socket_address(sa, &r_addr);
            if (result != er_ok)
                break;
            result = prx_ns_get_entry_by_addr(
                prx_host_get_ns(sock->host), &r_addr, &host);
            if (result != er_ok)
                break;
        }
        else
        {
            // Create fake entry for dynamic lookup on connect
            result = prx_ns_entry_create(
                prx_ns_entry_type_host, sa->un.host.host, sa->un.host.host, &host);
            if (result != er_ok)
                break;
        }

        // Check if already bound to an interface address
        if (io_ref_equals(&sock->c_addr, &io_ref_null))
        {
            result = prx_client_socket_connect_unbound(sock, host, sa->un.ip.port);
        }
        else
        {
            // Otherwise connect through bound interface
            result = prx_client_socket_connect_bound(sock, host, sa->un.ip.port);
        }
        if (result != er_ok)
            break;
    } while (0);

    if (host)
        prx_ns_entry_release(host);

    return result;
}

//
// Binds the socket to an interface to send and receive data
//
static int32_t prx_client_socket_bind_by_addr(
    prx_client_socket_t* sock,
    prx_socket_address_t* sa
)
{
    int32_t result;
    prx_ns_entry_t* host = NULL;
    prx_ns_result_t* resultset = NULL;
    prx_ns_t* ns;

    dbg_assert(sa->un.family != prx_address_family_proxy, "");

    do
    {
        result = io_ref_from_prx_socket_address(sa, &sock->c_addr);
        if (result != er_ok)
            break;

        ns = prx_host_get_ns(sock->host);
        if (!ns)
        {
            result = er_closed;
            break;
        }

        // Check if "any address" == null was passed
        if (io_ref_equals(&sock->c_addr, &io_ref_null))
        {
            if (sock->props.sock_type == prx_socket_type_stream)
            {
                // Will bind later on connect
                result = er_ok;
                break;
            }

            // Connect to first successful interface
            result = prx_ns_get_entry_by_type(
                ns, prx_ns_entry_type_proxy, &resultset);
            while (true)
            {
                // Update target and interface on message so it is routed correctly.
                host = prx_ns_result_pop(resultset);
                if (!host)
                    break;

                // try bind
                result = prx_ns_entry_get_addr(host, &sock->c_addr);
                if (result != er_ok)
                    break;
                result = prx_client_socket_connect_bound(sock, host, sa->un.ip.port);
                if (result == er_ok)
                    break;

                prx_ns_entry_release(host);
                io_ref_clear(&sock->c_addr);
                continue;
            }
        }
        else
        {
            // Concrete interface chosen
            result = prx_ns_get_entry_by_addr(ns, &sock->c_addr, &host);
            if (result != er_ok)
                break;
            if (sock->props.sock_type == prx_socket_type_stream)
            {
                // Will bind later
                result = er_ok;
                break;
            }

            result = prx_client_socket_connect_bound(sock, host, sa->un.ip.port);
            if (result == er_ok)
                break;
            io_ref_clear(&sock->c_addr);
        }
        break;
    } while (0);

    if (host)
        prx_ns_entry_release(host);
    if (resultset)
        prx_ns_result_release(resultset);
    prx_client_socket_release(sock);
    return result;
}

//
// Bind the socket to an named interface
//
static int32_t prx_client_socket_bind_by_name(
    prx_client_socket_t* sock,
    char* if_name,
    uint16_t port
)
{
    int32_t result;
    size_t prx_ifa_count;
    prx_ifaddrinfo_t* prx_ifa;

    // Enumerate interfaces to bind to, then bind to first successful one.
    result = prx_client_getifaddrinfo(if_name, 0, &prx_ifa, &prx_ifa_count);
    if (result != er_ok)
        return result;

    for (size_t i = 0; i < prx_ifa_count; i++)
    {
        // Bind interface (= gw or host) to send or receive data
        prx_ifa[i].address.un.ip.port = port;
        result = prx_client_socket_bind_by_addr(sock, &prx_ifa[i].address);
        if (result == er_ok)
            break;
    }

    prx_client_freeifaddrinfo(prx_ifa);
    return result;
}

//
// Send data to specified socket
//
static int32_t prx_client_socket_send(
    prx_client_socket_t* sock,
    uint8_t * buf,
    size_t off,
    size_t len,
    prx_socket_address_t* address,
    size_t* sent
)
{
    int32_t result;
    io_message_t* message = NULL;
    io_data_t data;

    dbg_assert_ptr(sock);
    dbg_assert_ptr(sock->stream);
    dbg_assert_ptr(buf);
    dbg_assert_ptr(sent);
    do
    {
        if (!sock->send_credit)
        {
            result = er_shutdown;
            break;
        }

        if (sock->send_count > sock->send_credit)
        {
            prx_client_setsockopt(sock->sock_fd, prx_so_send_credit, sock->send_credit);
            sock->send_count = 0;
        }

        result = io_channel_begin_write(sock->stream, sock->timeout, &message);
        if (result != er_ok)
            break;

        data.buffer = &buf[off];
        data.buffer_length = len;

        // Set address based on whether provided or not
        if (address)
            memcpy(&data.source_address, address, sizeof(prx_socket_address_t));
        else
            data.source_address.un.family = prx_address_family_unspec;

        __io_proto_pack(message, data, sock->log);
        if (result != er_ok)
        {
            prx_client_socket_log_error(sock, "encode send data failed (%s).",
                prx_err_string(result));
            break;
        }

        result = io_channel_complete_write(sock->stream, message, true);
        sock->send_count++;

        message = NULL;
    } while (0);

    if (message)
        io_channel_abort_write(sock->stream, message);

    *sent = result == er_ok ? len : 0;
    return result;
}

//
// Receive data from socket
//
static int32_t prx_client_socket_recv(
    prx_client_socket_t* sock,
    uint8_t *buf,
    size_t off,
    size_t len,
    prx_socket_address_t* address,
    size_t* received
)
{
    int32_t result;
    io_data_t data;
    io_message_t* message;

    dbg_assert_ptr(sock);
    dbg_assert_ptr(sock->stream);
    dbg_assert_ptr(buf);
    dbg_assert_ptr(received);

    do
    {
        if (sock->props.sock_type == prx_socket_type_stream)
        {
            if (sock->recv_buffer_size > sock->recv_buffer_offset)
            {
                *received = min(len, 
                    sock->recv_buffer_size - sock->recv_buffer_offset);
                memcpy(&buf[off], &sock->recv_buffer[sock->recv_buffer_offset], *received);
                sock->recv_buffer_offset += *received;
                result = er_ok;
                break;
            }
        }

        if (!sock->recv_credit)
        {
            result = er_shutdown;
            break;
        }

        if (sock->recv_count >= sock->recv_credit)
        {
            dbg_assert(sock->recv_count == sock->recv_credit, "");
            prx_client_setsockopt(sock->sock_fd, prx_so_recv_credit, sock->recv_credit);
            sock->recv_count = 0;
        }

        result = io_channel_begin_read(sock->stream, sock->timeout, &message);
        if (result == er_timeout && sock->recv_count > 0)
        {
            //
            // If we time out having received some messages, we might have lost one, 
            // assign more recv credit and try again...
            //
            sock->recv_count = sock->recv_credit;
            continue;
        }

        if (result != er_ok)
            break;

        sock->recv_count++;

        memset(&data, 0, sizeof(data));
        if (sock->props.sock_type == prx_socket_type_dgram)
        {
            data.buffer = &buf[off];
            data.buffer_length = len;
        }
        else if (sock->props.sock_type == prx_socket_type_stream)
        {
            sock->recv_buffer = (uint8_t*)mem_realloc(
                sock->recv_buffer, io_message_get_size(message));
            if (!sock->recv_buffer)
            {
                result = er_out_of_memory;
                break;
            }

            // Stream buffer
            data.buffer = sock->recv_buffer;
            data.buffer_length = io_message_get_size(message);
        }
        else
        {
            dbg_assert_ptr(0);
        }

        __io_proto_unpack(message, data, sock->log);
        if (result != er_ok)
        {
            prx_client_socket_log_error(sock, "decode recv data failed (%s).",
                prx_err_string(result));
            break;
        }

        // Copy the address from header if address pointer provided
        if (sock->props.sock_type == prx_socket_type_dgram)
        {
            if (address)
                memcpy(address, &data.source_address, sizeof(prx_socket_address_t));
            *received = data.buffer_length;
            break;
        }
        
        sock->recv_buffer_size = data.buffer_length;
        sock->recv_buffer_offset = 0;

    } while (true);

    io_channel_complete_read(sock->stream);
    return result;
}

//
// Get a ref counted instance of global host
//
static int32_t prx_client_socket_get_prx_host_ref(
    prx_host_t** host
)
{
    chk_arg_fault_return(host);

    if (!global_state.host)
        return er_bad_state;

    return prx_host_clone(global_state.host, host);
}


//
// Init platform independent API layer
//
int32_t prx_client_init(
    const char* config
)
{
    int32_t result;

    if (global_state.init)
        return er_ok;

    global_state.init = true;

    result = handle_map_init();
    if (result == er_ok)
    {
        result = prx_host_init();
        if (result == er_ok)
        {
            result = prx_host_create(
                config, proxy_type_client, &global_state.host);

            if (result == er_ok)
            {
                // TODO: Start lazily...
                result = prx_host_start(global_state.host);
                if (result == er_ok)
                {
                    return result;
                }

                prx_host_release(global_state.host);
            }
            prx_host_deinit();
        }
        handle_map_deinit();
    }

    return result;
}

//
// Deinit platform independent API
//
int32_t prx_client_deinit(
    void
)
{
    if (!global_state.init)
        return er_ok;

    global_state.init = false;

    if (global_state.host)
    {
        prx_host_release(global_state.host); // Note: Calls prx_host_stop()
        global_state.host = NULL;
    }

    handle_map_deinit();

    prx_host_deinit();

    return er_ok;
}

//
// Free the address info struct
//
int32_t prx_client_freeaddrinfo(
    prx_addrinfo_t* prx_ai
)
{
    chk_arg_fault_return(prx_ai);

    for (int32_t i = 0; prx_ai[i].name != NULL; i++)
        mem_free(prx_ai[i].name);

    mem_free(prx_ai);
    return er_ok;
}

//
// Returns a list of address entries for a name
//
int32_t prx_client_getaddrinfo(
    const char* name,
    const char* svc,
    prx_address_family_t family,
    uint32_t flags,
    prx_addrinfo_t** prx_ai,
    size_t* prx_ai_count
)
{
    int32_t result;
    prx_host_t* host;
    prx_ns_t* ns;
    prx_ns_entry_t* host = NULL;
    prx_ns_result_t* resultset = NULL;
    prx_addrinfo_t* prx_ai_cur;

    chk_arg_fault_return(prx_ai);
    chk_arg_fault_return(prx_ai_count);

    if (family != prx_address_family_unspec &&
        family != prx_address_family_inet &&
        family != prx_address_family_inet6 &&
        family != prx_address_family_proxy)
        return er_arg;

    result = prx_client_socket_get_prx_host_ref(&host);
    if (result != er_ok)
        return result;

    *prx_ai = NULL;
    *prx_ai_count = 0;

    do
    {
        if (!name && (flags & prx_ai_passive))
        {
            // Just return an empty address (equivalent to INADDR_ANY)
            *prx_ai_count = 1;
            *prx_ai = (prx_addrinfo_t*)mem_zalloc(sizeof(prx_addrinfo_t));
            if (!*prx_ai)
            {
                result = er_out_of_memory;
                break;
            }

            prx_ai_cur = *prx_ai;
            prx_ai_cur->address.un.ip.family = family;
            if (svc)
                prx_ai_cur->address.un.ip.port = atoi(svc);

            result = er_ok;
            break;
        }

        ns = prx_host_get_ns(host);
        dbg_assert_ptr(ns);

        // Use localhost if null passed
        if (!name)
            name = "localhost";

        result = prx_ns_get_entry_by_name(ns, name, &resultset);
        if (result != er_ok)
            break;

        *prx_ai_count = prx_ns_result_size(resultset);
        if (!*prx_ai_count)
        {
            result = er_prx_host_unknown;
            break;
        }

        result = er_ok;
        *prx_ai = (prx_addrinfo_t*)mem_zalloc((*prx_ai_count + 1) * sizeof(prx_addrinfo_t));
        for (size_t i = 0; i < *prx_ai_count; i++)
        {
            prx_ai_cur = &(*prx_ai)[i];
            host = prx_ns_result_pop(resultset);
            if (!host)
            {
                result = er_fatal;
                break;
            }

            result = prx_ns_entry_to_prx_socket_address(host, family, &prx_ai_cur->address);
            if (result != er_ok)
                break;
            if (svc)
                prx_ai_cur->address.un.ip.port = atoi(svc);
            result = string_clone(prx_ns_entry_get_name(host), &prx_ai_cur->name);
            if (result != er_ok)
                break;

            prx_ai_cur->reserved = 1;
            prx_ns_entry_release(host);
            host = NULL;
        }
    } 
    while (0);

    if (result != er_ok && *prx_ai)
    {
        prx_client_freeaddrinfo(*prx_ai);
        *prx_ai = NULL;
        *prx_ai_count = 0;
    }

    if (host)
        prx_ns_entry_release(host);
    if (resultset)
        prx_ns_result_release(resultset);
    prx_host_release(host);
    return result;
}

//
// Returns host and service name strings from socket address
//
int32_t prx_client_getnameinfo(
    prx_socket_address_t* address,
    char* prx_host_name,
    size_t prx_host_length,
    char* service,
    size_t service_length,
    int32_t flags
)
{
    char buf[MAX_PORT_LENGTH];
    int32_t result;
    prx_host_t* host;
    prx_ns_t* ns;
    prx_ns_entry_t* host = NULL;
    io_ref_t address;

    (void)flags;

    chk_arg_fault_return(address);

    result = prx_client_socket_get_prx_host_ref(&host);
    if (result != er_ok)
        return result;
    do
    {
        ns = prx_host_get_ns(host);
        dbg_assert_ptr(ns);

        result = io_ref_from_prx_socket_address(address, &address);
        if (result != er_ok)
            break;
        result = prx_ns_get_entry_by_addr(ns, &address, &host);
        if (result != er_ok)
            break;

        if (!strncpy(prx_host_name, prx_ns_entry_get_name(host), prx_host_length))
        {
            result = er_arg;
            break;
        }
        
        result = string_from_int(
            address->un.ip.port, 10, buf, sizeof(buf));
        if (result != er_ok)
        {
            result = er_arg;
            break;
        }
        
        if (service && !strncpy(service, buf, service_length))
        {
            result = er_arg;
            break;
        }
    } while (0);

    if (host)
        prx_ns_entry_release(host);
    prx_host_release(host);
    return result;
}

//
// Free the interface address info struct
//
int32_t prx_client_freeifaddrinfo(
    prx_ifaddrinfo_t* prx_ifa
)
{
    chk_arg_fault_return(prx_ifa);
    mem_free(prx_ifa);
    return er_ok;
}

//
// Returns a list of interface address entries for a name
//
int32_t prx_client_getifaddrinfo(
    const char* if_name,
    uint32_t flags,
    prx_ifaddrinfo_t** prx_ifa,
    size_t* prx_ifa_count
)
{
    int32_t result;
    prx_host_t* host;
    prx_ns_t* ns;
    prx_ns_entry_t* itf = NULL;
    prx_ns_result_t* resultset = NULL;
    prx_ifaddrinfo_t* prx_ifa_cur;

    (void)flags;


    chk_arg_fault_return(prx_ifa);
    chk_arg_fault_return(prx_ifa_count);

    result = prx_client_socket_get_prx_host_ref(&host);
    if (result != er_ok)
        return result;
    do
    {
        ns = prx_host_get_ns(host);
        dbg_assert_ptr(ns);

        result = prx_ns_get_entry_by_name(ns, if_name, &resultset);
        if (result != er_ok)
            break;

        *prx_ifa_count = prx_ns_result_size(resultset);
        if (!*prx_ifa_count)
        {
            result = er_prx_host_unknown;
            break;
        }

        result = er_ok;
        *prx_ifa = (prx_ifaddrinfo_t*)
            mem_zalloc((*prx_ifa_count + 1) * sizeof(prx_ifaddrinfo_t));
        for (size_t i = 0; i < *prx_ifa_count; i++)
        {
            prx_ifa_cur = &(*prx_ifa)[i];
            itf = prx_ns_result_pop(resultset);
            if (!itf)
            {
                result = er_fatal;
                break;
            }

            prx_ifa_cur->index = prx_ns_entry_get_index(itf);
            result = prx_ns_entry_to_prx_socket_address(
                itf, prx_address_family_unspec, &prx_ifa_cur->address);
            if (result != er_ok)
                break;
            (void)strncpy(prx_ifa_cur->name,
                prx_ns_entry_get_name(itf), sizeof(prx_ifa_cur->name)-1);

            prx_ns_entry_release(itf);
            itf = NULL;
        }
    } 
    while (0);

    if (result != er_ok && *prx_ifa)
    {
        prx_client_freeifaddrinfo(*prx_ifa);
        *prx_ifa = NULL;
        *prx_ifa_count = 0;
    }

    if (itf)
        prx_ns_entry_release(itf);
    if (resultset)
        prx_ns_result_release(resultset);
    prx_host_release(host);
    return result;
}

//
// Returns interface name for interface address
//
int32_t prx_client_getifnameinfo(
    prx_socket_address_t* if_address,
    char* if_name,
    size_t if_name_length,
    uint64_t *if_index
)
{
    int32_t result;
    prx_host_t* host;
    prx_ns_t* ns;
    prx_ns_entry_t* itf = NULL;
    io_ref_t address;

    chk_arg_fault_return(if_address);
    chk_arg_fault_return(if_name);
    chk_arg_fault_return(if_name_length);

    result = prx_client_socket_get_prx_host_ref(&host);
    if (result != er_ok)
        return result;
    do
    {
        ns = prx_host_get_ns(host);
        dbg_assert_ptr(ns);

        result = io_ref_from_prx_socket_address(if_address, &address);
        if (result != er_ok)
            break;
        result = prx_ns_get_entry_by_addr(ns, &address, &itf);
        if (result != er_ok)
            break;

        if (!strncpy(if_name, prx_ns_entry_get_name(itf), if_name_length))
        {
            result = er_arg;
            break;
        }

        if (if_index)
            *if_index = prx_ns_entry_get_index(itf);
    } 
    while (0);

    if (itf)
        prx_ns_entry_release(itf);
    prx_host_release(host);
    return result;
}

// Use pal layer directly
decl_internal_2(int32_t, pal_pton,
    const char*, addr_string, 
    prx_socket_address_t*, sa
);

//
// Convert an address string into an address
//
int32_t prx_client_pton(
    const char* addr_string,
    prx_socket_address_t* address
)
{
    return pal_pton(addr_string, address);
}

// Use pal layer directly
decl_internal_3(int32_t, pal_ntop,
    prx_socket_address_t*, sa, 
    char*, string, 
    size_t, size
);

//
// Convert an address to a string
//
int32_t prx_client_ntop(
    prx_socket_address_t* address,
    char* addr_string,
    size_t addr_string_size
)
{
    return pal_ntop(address, addr_string, addr_string_size);
}

//
// Returns a fake host name to the client.
//
int32_t prx_gethostname(
    char* name,
    size_t namelen
)
{
    int32_t result;
    prx_host_t* host;
    STRING_HANDLE id;

    chk_arg_fault_return(name);
    chk_arg_fault_return(namelen);

    result = prx_client_socket_get_prx_host_ref(&host);
    if (result != er_ok)
        return (result);
    
    id = io_ref_to_STRING(prx_host_get_id(host));
    prx_host_release(host);

    if (!id)
        return er_out_of_memory;
    strncpy(name, STRING_c_str(id), namelen - 1);
    STRING_delete(id);
    return er_ok;
}

//
// Returns the peer name of the socket
//
int32_t prx_client_getpeername(
    prx_fd_t s,
    prx_socket_address_t* address
)
{
    prx_client_socket_t* sock;
    chk_arg_fault_return(s);
    chk_arg_fault_return(address);

    sock = prx_client_socket_get_by_id((int32_t)s, true);
    if (!sock)
        return er_closed;
    memcpy(address, &sock->r_peer, sizeof(prx_socket_address_t));
    prx_client_socket_release(sock);
    return er_ok;
}

//
// Creates a new socket object
//
int32_t prx_client_socket(
    prx_address_family_t address_family,
    prx_socket_type_t  socket_type,
    prx_protocol_type_t protocol_type,
    prx_fd_t* created
)
{
    int32_t result;
    prx_client_socket_t* sock;
    prx_host_t* host;

    chk_arg_fault_return(created);

    result = prx_client_socket_get_prx_host_ref(&host);
    if (result != er_ok)
        return (result);

    result = prx_client_socket_create(
        host, address_family, socket_type, protocol_type, &sock);
    prx_host_release(host);
    if (result != er_ok)
        return result;

    // The id is our handle now
    *created = (prx_fd_t)sock->sock_fd;
    prx_client_socket_release(sock);
    return er_ok;
}

//
// Wait for an activity on any of the passed in sockets
//
int32_t prx_client_poll(
    size_t num,
    prx_fd_t* sockets,
    uint64_t* timeout_ms
)
{
    int32_t result = 0;
    ticks_t timer;
    prx_host_t* host;

    chk_arg_fault_return(timeout_ms);

    // TODO: not supported yet, we will just wait for activity and let
    // caller figure it out...
    (void)num;
    (void)sockets;

    result = prx_client_socket_get_prx_host_ref(&host);
    if (result != er_ok)
        return result;

    timer = ticks_get();
    
    (void)io_cm_wait(
        prx_host_get_cm(host), *timeout_ms < 0 ? -1 : (int32_t)*timeout_ms);

    timer = ticks_get() - timer;
    *timeout_ms = *timeout_ms > timer ? *timeout_ms - timer : 0;

    prx_host_release(host);
    return er_ok;
}

//
// Return sockets address
//
int32_t prx_client_getsockname(
    prx_fd_t s,
    prx_socket_address_t* address
)
{
    prx_client_socket_t* sock;
    if (!address)
        return er_arg;

    sock = prx_client_socket_get_by_id((int32_t)s, false);
    if (!sock)
        return er_closed;
    // When connected use the real one, else the one originally assigned...
    if (sock->connected)
        memcpy(address, &sock->l_addr, sizeof(prx_socket_address_t));
    else
    {
        io_ref_to_prx_socket_address(&sock->l_addr, address);
        address->un.ip.port = sock->props.address.un.ip.port;
    }
    prx_client_socket_release(sock);
    return er_ok;
}

//
// Connect synchronously
//
int32_t prx_client_connect(
    prx_fd_t s,
    prx_socket_address_t* sa
)
{
    int32_t result;
    prx_client_socket_t* sock;

    sock = prx_client_socket_get_by_id((int32_t)s, false);
    if (!sock)
        return er_closed;
    do
    {
        if (sock->props.sock_type != prx_socket_type_stream)
        {
            result = er_bad_state;
            prx_client_socket_log_error(sock, 
                "Connect can only be called on stream sockets (%s)",
                prx_err_string(result));
            break;
        }
        result = prx_client_socket_connect(sock, sa);
        break;
    }
    while (0);

    prx_client_socket_release(sock);
    return result;
}

//
// Binds the socket to an interface to send and receive data
//
int32_t prx_client_bind(
    prx_fd_t s,
    prx_socket_address_t* sa
)
{
    int32_t result;
    prx_client_socket_t* sock;

    sock = prx_client_socket_get_by_id((int32_t)s, false);
    if (!sock)
        return er_closed;

    if (sa->un.family == prx_address_family_proxy)
    {
        result = prx_client_socket_bind_by_name(
            sock, sa->un.proxy.host, sa->un.proxy.port);
    }
    else
    {
        result = prx_client_socket_bind_by_addr(sock, sa);
    }

    prx_client_socket_release(sock);
    return result;
}

//
// Get the next socket accepted on the passive socket
//
int32_t prx_client_accept(
    prx_fd_t s,
    uintptr_t key,
    prx_socket_address_t* sa,
    prx_fd_t* accepted
)
{
    int32_t result;
    io_message_t* message = NULL;
    io_link_request_t link_request;
    prx_client_socket_t* sock, *remote = NULL;
    (void)key;

    chk_arg_fault_return(accepted);

    sock = prx_client_socket_get_by_id((int32_t)s, true);
    if (!sock)
        return er_closed;

    if (0 == (sock->props.flags & socket_flag_acceptconn))
    {
        prx_client_socket_release(sock);
        return er_bad_state;
    }
    do
    {
        // Read link request from stream
        result = io_channel_begin_read(
            sock->stream, sock->timeout ? -1 : 0, &message);
        if (result != er_ok)
            break;

        memset(&link_request, 0, sizeof(link_request));
        __io_proto_unpack(message, link_request, sock->log);
        if (result != er_ok)
        {
            io_channel_complete_read(sock->stream);
            break;
        }
        result = io_channel_complete_read(sock->stream);
        if (result != er_ok)
            break;

        dbg_assert(link_request.props.sock_type == prx_socket_type_stream, 
            "Accept only streams");

        // Open new sock with the received properties
        result = prx_client_socket_create(sock->host, 
            link_request.props.family, 
            link_request.props.sock_type, 
            link_request.props.proto_type, &remote);
        if (result != er_ok)
            break;
        io_ref_copy(&message->source_id, &remote->r_addr);
        io_ref_copy(&message->interface_id, &remote->c_addr);
        result = prx_client_socket_open(remote);
        if (result != er_ok)
            break;

        if (sa)
            memcpy(sa, &remote->props.address, sizeof(prx_socket_address_t));
        *accepted = (prx_fd_t)remote->sock_fd;
        return er_ok;

    } while (0);

    if (remote)
        prx_client_socket_release(remote);
    prx_client_socket_release(sock);
    return (result);
}

//
// True if data is available to receive
//
bool prx_client_can_recv(
    prx_fd_t s
)
{
    int32_t result;
    size_t available;
    prx_client_socket_t* sock;

    sock = prx_client_socket_get_by_id((int32_t)s, false);
    if (!sock)
        return false;

    result = io_channel_get_available(sock->stream, &available);
    prx_client_socket_release(sock);
    return result == er_ok && available > 0;
}

//
// True if ready to send data
//
bool prx_client_can_send(
    prx_fd_t s
)
{
    int32_t result;
    size_t capacity;
    prx_client_socket_t* sock;

    sock = prx_client_socket_get_by_id((int32_t)s, false);
    if (!sock)
        return false;

    result = io_channel_get_capacity(sock->stream, &capacity);
    prx_client_socket_release(sock);
    return result == er_ok && capacity > 0;
}

//
// returns true if socket is in error or closed condition
//
bool prx_client_has_error(
    prx_fd_t s
)
{
    prx_client_socket_t* sock;
    bool has_error;

    sock = prx_client_socket_get_by_id((int32_t)s, false);
    if (!sock)
        return true;

    has_error = sock->stream && io_channel_get_last_error(sock->stream) != er_ok;
    prx_client_socket_release(sock);
    return has_error;
}

//
// returns true if socket is disconnected
//
bool prx_client_is_disconnected(
    prx_fd_t s
)
{
    prx_client_socket_t* sock;
    bool is_disconnected;

    sock = prx_client_socket_get_by_id((int32_t)s, false);
    if (!sock)
        return true;

    is_disconnected = (sock->stream == NULL);
    prx_client_socket_release(sock);
    return is_disconnected;
}

//
// Send data to specified socket
//
int32_t prx_client_sendto(
    prx_fd_t s,
    uintptr_t key,
    int32_t flags,
    uint8_t * buf,
    size_t off,
    size_t len,
    prx_socket_address_t* address,
    size_t* sent
)
{
    int32_t result;
    prx_client_socket_t* sock;
    (void)key;
    (void)flags;

    chk_arg_fault_return(buf);
    chk_arg_fault_return(sent);

    sock = prx_client_socket_get_by_id((int32_t)s, true);
    if (!sock)
        return er_closed;
    result = prx_client_socket_send(sock, buf, off, len, address, sent);
    prx_client_socket_release(sock);
    return result;
}

//
// Send data on socket
//
int32_t prx_client_send(
    prx_fd_t s,
    uintptr_t key,
    int32_t flags,
    uint8_t* buf,
    size_t off,
    size_t len,
    size_t* sent
)
{
    int32_t result;
    prx_client_socket_t* sock;
    (void)key;
    (void)flags;

    chk_arg_fault_return(buf);
    chk_arg_fault_return(sent);

    sock = prx_client_socket_get_by_id((int32_t)s, true);
    if (!sock)
        return er_closed;
    result = prx_client_socket_send(sock, buf, off, len, NULL, sent);
    prx_client_socket_release(sock);
    return result;
}

//
// Receive data on a bound socket
//
int32_t prx_client_recvfrom(
    prx_fd_t s,
    uintptr_t key,
    int32_t flags,
    uint8_t* buf,
    size_t off,
    size_t len,
    prx_socket_address_t* address,
    size_t* received
)
{
    int32_t result;
    prx_client_socket_t* sock;
    (void)key;
    (void)flags;

    chk_arg_fault_return(buf);
    chk_arg_fault_return(received);

    sock = prx_client_socket_get_by_id((int32_t)s, true);
    if (!sock)
        return er_closed;
    result = prx_client_socket_recv(sock, buf, off, len, address, received);
    prx_client_socket_release(sock);
    return result;
}

//
// Receive data from socket
//
int32_t prx_client_recv(
    prx_fd_t s,
    uintptr_t key,
    int32_t flags,
    uint8_t *buf,
    size_t off,
    size_t len,
    size_t* received
)
{
    int32_t result;
    prx_client_socket_t* sock;
    (void)key;
    (void)flags;

    chk_arg_fault_return(buf);
    chk_arg_fault_return(received);

    sock = prx_client_socket_get_by_id((int32_t)s, true);
    if (!sock)
        return er_closed;
    result = prx_client_socket_recv(sock, buf, off, len, NULL, received);
    prx_client_socket_release(sock);
    return result;
}

//
// Close the socket
//
int32_t prx_client_close(
    prx_fd_t s
)
{
    int32_t result;
    io_message_t* message;
    io_close_response_t close_response;
    prx_client_socket_t* sock;
    io_channel_t* control = NULL;
    void* void_request;

    //
    // Close remote socket.  Need to do it here instead of release, since
    // release might be called in the message pump path.  This is ok, since
    // all user calls come through here.
    //
    sock = prx_client_socket_get_by_id((int32_t)s, false);
    if (!sock)
        return er_closed;
    do
    {
        if (!sock->stream)
        {
            result = er_ok;
            break;
        }

        dbg_assert(sock->connected, "");

        result = prx_client_socket_create_channel(sock, io_message_type_close, &control);
        if (result != er_ok)
            break;

        memset(&close_response, 0, sizeof(close_response));
        __prx_client_socket_invoke(
            void_request, close_response, sock->control_timeout, sock->log);
        if (result != er_ok)
            break;

        // Remote error code
        result = close_response.error_code;

        prx_client_socket_log_info(sock, 
            "closed (was open for %lld ms, %lld bytes sent, %lld bytes received",
            close_response.time_open, close_response.bytes_sent, 
            close_response.bytes_received);
    } 
    while (0);

    if (result != er_ok && result != er_closed)
    {
        prx_client_socket_log_error(sock, "Failed to close remote socket (%s)", 
            prx_err_string(result));
    }

    if (control)
        io_channel_close(control);

    // Remove the handle from the handle map and release its ref count
    handle_map_remove_handle(sock->sock_fd);
    prx_client_socket_release(sock);

    // Invalidate the sock and release our last reference
    prx_client_socket_invalidate(sock);
    prx_client_socket_release(sock);

    return er_ok;
}

//
// Do the linking for passive connection oriented sockets
//
int32_t prx_client_listen(
    prx_fd_t s,
    int32_t backlog
)
{
    int32_t result = er_ok;
    prx_client_socket_t* sock;

    (void)backlog;

    sock = prx_client_socket_get_by_id((int32_t)s, false);
    if (!sock)
        return er_closed;

    if (0 == (sock->props.flags & prx_socket_flag_passive))
        result = er_bad_state;
    else
        sock->props.flags |= socket_flag_acceptconn;

    prx_client_socket_release(sock);
    return result;
}

//
// Returns socket options 
//
int32_t prx_client_getsockopt(
    prx_fd_t s,
    prx_socket_option_t option,
    uint64_t* value
)
{
    int32_t result;
    prx_client_socket_t* sock;
    io_message_t* message = NULL;
    io_channel_t* control = NULL;
    io_getopt_request_t getopt_request;
    io_getopt_response_t getopt_response;

    chk_arg_fault_return(value);

    sock = prx_client_socket_get_by_id((int32_t)s, false);
    if (!sock)
        return er_closed;

    // Try to satisfy from local state
    *value = 0;
    result = er_unknown;
    switch (option)
    {
    case prx_so_available:
        if (!sock->connected)
        {
            size_t avail;
            result = io_channel_get_available(sock->stream, &avail);
            if (result != er_ok)
                break;
            *value = avail;
            result = er_ok;
        }
        break;
    case prx_so_nonblocking:
        // TODO
        *value = !!sock->timeout;
        result = er_ok;
        break;
    case prx_so_tcp_nodelay:
        *value = 1;
        result = er_ok;
        break;
    case prx_so_keepalive:
        *value = 1;
        result = er_ok;
        break;
    case prx_so_rcvbuf:
    case prx_so_sndbuf:
        if (!sock->connected)
        {
            *value = 0x1000;
            result = er_ok;
            break;
        }
        break;
    case prx_so_error:
        if (sock->connected)
            *value = io_channel_get_last_error(sock->stream);
        else
            *value = er_ok;
        result = er_ok;
        break;
    case prx_so_acceptconn:
        *value = 0 != (sock->props.flags & socket_flag_acceptconn);
        result = er_ok;
        break;
    default:
        break;
    }
    do
    {
        if (result != er_unknown)
            break;
        if (!sock->connected)
        {
            result = er_closed;
            break;
        }

        // Get a control port to message the remote side to do the same.
        result = prx_client_socket_create_channel(sock, io_message_type_getopt, &control);
        if (result != er_ok)
            break;

        getopt_request.so_opt = option;
        memset(&getopt_response, 0, sizeof(getopt_response));
        __prx_client_socket_invoke(
            getopt_request, getopt_response, sock->control_timeout, sock->log);
        dbg_assert(getopt_response.so_val.option == option, 
            "Option type different in response");
        *value = getopt_response.so_val.value;
        break;

    } while (0);

    if (control)
        io_channel_close(control);

    prx_client_socket_release(sock);
    return (result);
}

//
// Set socket options for the unconnected socket
//
int32_t prx_client_setsockopt(
    prx_fd_t s,
    prx_socket_option_t option,
    uint64_t value
)
{
    int32_t result;
    io_message_t* message = NULL;
    prx_client_socket_t* sock;
    io_channel_t* control = NULL;
    io_setopt_request_t setopt_request;
    void* void_response;

    sock = prx_client_socket_get_by_id((int32_t)s, false);
    if (!sock)
        return er_closed;

    // Filter first on what we can set in local state based on state of socket
    result = er_unknown;
    switch (option)
    {
    case prx_so_error:
    case prx_so_available:
        result = er_arg;
        break;
    case prx_so_acceptconn:
        if (0 == (sock->props.flags & prx_socket_flag_passive))
            result = er_arg;
        else
            sock->props.flags |= socket_flag_acceptconn;
        break;
    case prx_so_nonblocking:
        // TODO
        sock->timeout = value ? 0 : sock->control_timeout;
        break;
    case prx_so_shutdown:
        if (sock->stream)
        {
            // Shutdown the stream port queues
            if (value != prx_shutdown_op_read)
                io_channel_shutdown_write(sock->stream);
            if (value != prx_shutdown_op_write)
                io_channel_shutdown_read(sock->stream);
        }
        break;
    case prx_so_tcp_nodelay:
    case prx_so_keepalive:
        // Always using keep alive on device side
        if (!sock->connected)
        {
            result = er_ok;
            break;
        }
        break;
    case prx_so_recv_credit:
        sock->recv_credit = (uint32_t)value;
        if (!sock->connected)
        {
            result = er_ok;
            break;
        }
        break;
    case prx_so_send_credit:
        sock->send_credit = (uint32_t)value;
        if (!sock->connected)
        {
            result = er_ok;
            break;
        }
        break;
    case prx_so_rcvbuf:
    case prx_so_sndbuf:
        if (!sock->connected)
        {
            result = er_ok;
            break;
        }
        break;
    case prx_so_broadcast:
        if (sock->props.sock_type != prx_socket_type_dgram)
            return er_arg;
        if (!value)
            sock->props.flags &= ~socket_flag_broadcast;
        else
            sock->props.flags |= socket_flag_broadcast;
        if (!sock->connected)
        {
            result = er_ok;
            break;
        }
        break;
    }
    do
    {
        if (result != er_unknown)
            break;
        if (!sock->connected)
        {
            result = er_closed;
            break;
        }
        // Get a control port to message the remote side to do the same.
        result = prx_client_socket_create_channel(sock, io_message_type_setopt, &control);
        if (result != er_ok)
            break;

        setopt_request.so_val.option = option;
        setopt_request.so_val.value = value;

        __prx_client_socket_invoke(
            setopt_request, void_response, sock->control_timeout, sock->log);
    } 
    while (0);

    if (control)
        io_channel_close(control);
    prx_client_socket_release(sock);
    return result;
}
#endif