// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "prx_server.h"
#include "prx_ns.h"
#include "pal.h"
#include "pal_sk.h"
#include "pal_net.h"
#include "pal_mt.h"
#include "pal_time.h"
#include "io_proto.h"
#include "io_transport.h"
#include "prx_buffer.h"
#include "util_string.h"

#include "hashtable.h"
#include "hashtable_itr.h"

//
// Handles socket layer targeted requests
//
struct prx_server
{
    io_ref_t id;                                             // Server id
    io_connection_t* listener;     // connection used by server to listen
    int32_t timeout;
    struct hashtable* sockets;                  // List of active sockets
    lock_t sockets_lock;       
    prx_scheduler_t* scheduler;  // All server tasks are on this scheduler
    bool exit;
    log_t log;
};

//
// State of the socket - used by management task
//
typedef enum prx_server_socket_state
{
    prx_server_socket_created,     // Socket created using link or accept
    prx_server_socket_opened,                     // Opened and connected
    prx_server_socket_collect,                  // Garbage collect socket
    prx_server_socket_closing,
    prx_server_socket_closed
}
prx_server_socket_state_t;

//
// Represents a link = socket that handles socket specific requests
//
typedef struct prx_server_socket
{
    pal_socket_t* sock;                              // Pal socket object
    pal_socket_client_itf_t client_itf;           // Pal client interface
    prx_server_t* server;                                // Owning server
    prx_scheduler_t* scheduler;  // All socket tasks are on this scheduler
    io_message_t* link_message;       // Message used to link this socket

    prx_server_socket_state_t state;
    int32_t last_error;             // Last error that occurred on socket
    ticks_t last_activity;        // Last activitiy on this socket for gc
    ticks_t time_opened;
    uint32_t timeout;                        // Timeout for current state

    io_ref_t id;                                         // Local link id
    io_ref_t owner;  // Remote socket id for link used for socket control

    io_ref_t stream_id;      // Remote stream id for link, used to stream
    io_connection_t* stream;       // Local stream connection once opened

    uint64_t bytes_sent;          // Number of bytes sent from send queue
    DLIST_ENTRY send_queue;                  // Sender queue, from stream
    lock_t send_lock;                // Lock to guard multi thread access
            // - and -
    uint64_t bytes_recvd;
    DLIST_ENTRY recv_queue;                  // Receiver queue, to stream
    lock_t recv_lock;                // Lock to guard multi thread access

    io_message_factory_t* message_pool;                  // Receiver pool
#define RECV_POOL_SIZE 6        // Default receive pool size = 6 messages
#define RECV_POOL_LWM 1  // Flow off until we have 1 message back in pool
#define RECV_POOL_HWM 5             // Number of bytes received on socket
    log_t log;
}
prx_server_socket_t;


//
// Websocket server transport (stream)
//
extern io_transport_t* io_iot_hub_ws_server_transport(
    void
);

//
// Free the socket - on scheduler thread
//
static void prx_server_socket_free(
    prx_server_socket_t *server_sock
)
{
    dbg_assert_ptr(server_sock);
    dbg_assert_is_task(server_sock->scheduler);

    if (server_sock->sock)
        pal_socket_free(server_sock->sock);
    if (server_sock->stream)
        io_connection_free(server_sock->stream);

    // Free send queue
    if (server_sock->send_lock)
        lock_enter(server_sock->send_lock);
    while (!DList_IsListEmpty(&server_sock->send_queue))
        io_message_release(containingRecord(DList_RemoveHeadList(
            &server_sock->send_queue), io_message_t, link));
    if (server_sock->send_lock)
        lock_exit(server_sock->send_lock);

    // Free recv queue
    if (server_sock->recv_lock)
        lock_enter(server_sock->recv_lock);
    while (!DList_IsListEmpty(&server_sock->recv_queue))
        io_message_release(containingRecord(DList_RemoveHeadList(
            &server_sock->recv_queue), io_message_t, link));
    if (server_sock->recv_lock)
        lock_exit(server_sock->recv_lock);

    if (server_sock->link_message)
        io_message_release(server_sock->link_message);

    if (server_sock->message_pool)
        io_message_factory_free(server_sock->message_pool);

    if (server_sock->send_lock)
        lock_free(server_sock->send_lock);
    if (server_sock->recv_lock)
        lock_free(server_sock->recv_lock);

    if (server_sock->scheduler)
        prx_scheduler_release(server_sock->scheduler, server_sock);

    log_info(server_sock->log, "Server socket %p freed!", server_sock);
    mem_free_type(prx_server_socket_t, server_sock);
}

//
// Free proxy server
//
static void prx_server_free(
    prx_server_t* server
)
{
    dbg_assert_ptr(server);
    dbg_assert(server->exit, "Should be exiting");
    dbg_assert(!server->listener, "Should not have open listener");

    if (server->scheduler)
        prx_scheduler_release(server->scheduler, server);

    dbg_assert(hashtable_count(server->sockets) == 0,
        "socketlist should now be empty");

    if (server->sockets)
        hashtable_destroy(server->sockets, 0);

    if (server->sockets_lock)
        lock_free(server->sockets_lock);

    log_info(server->log, "Freeing server.");
    mem_free_type(prx_server_t, server);
}

//
// Server worker
//
static void prx_server_worker(
    prx_server_t* server
)
{
    prx_server_socket_t* next;
    struct hashtable_itr* itr;
    int result;
    bool sending, receiving, timedout;
    ticks_t now;

    dbg_assert_ptr(server);

    now = ticks_get();

    // Schedule again in 5 seconds
    __do_later(server, prx_server_worker, 5000);

    lock_enter(server->sockets_lock);
    if (hashtable_count(server->sockets) > 0)
    {
        itr = hashtable_iterator(server->sockets);
        if (itr) // See hashtable_itr - todo: Change to use stack
        {
            do
            {
                next = (prx_server_socket_t*)hashtable_iterator_value(itr);
                dbg_assert_ptr(next);
                if (next->last_activity == 0 ||
                    (uint32_t)(now - next->last_activity) < next->timeout)
                    timedout = false;
                else
                    timedout = true;  // State timed out

                switch (next->state)
                {
                case prx_server_socket_closing:
                    if (!timedout)
                        break;
                    log_error(next->log,
                        "Waited for close to complete on %p, but timed out...", next);
                    next->state = prx_server_socket_closed;
                    // fall through

                case prx_server_socket_closed:
                    if (next->stream)
                    {
                        log_info(next->log, "Socket %p closed, clean up stream", 
                            next);
                        io_connection_close(next->stream);
                    }
                    else
                    {
                        // Delay free to debounce hang up race condition
                        __do_later(next, prx_server_socket_free, 2000);
                    }
                    result = hashtable_iterator_remove(itr);
                    break;

                case prx_server_socket_created:
                case prx_server_socket_opened:
                    if (!timedout && !server->exit)
                        break;

                    next->state = prx_server_socket_collect;
                    if (timedout)
                        log_info(next->log, "No activity on socket %p, closing...",
                            next);

                    // Free send queue
                    lock_enter(next->send_lock);
                    while (!DList_IsListEmpty(&next->send_queue))
                        io_message_release(containingRecord(DList_RemoveHeadList(
                            &next->send_queue), io_message_t, link));
                    lock_exit(next->send_lock);

                    // Free recv queue
                    lock_enter(next->recv_lock);
                    while (!DList_IsListEmpty(&next->recv_queue))
                        io_message_release(containingRecord(DList_RemoveHeadList(
                            &next->recv_queue), io_message_t, link));
                    lock_exit(next->recv_lock);
                    // fall through

                case prx_server_socket_collect:
                    if (!next->sock)
                    {
                        next->state = prx_server_socket_closed;
                        break;
                    }

                    // Check whether send and recv queue are still full...
                    lock_enter(next->send_lock);
                    sending = !DList_IsListEmpty(&next->send_queue);
                    lock_exit(next->send_lock);

                    lock_enter(next->recv_lock);
                    receiving = !DList_IsListEmpty(&next->recv_queue);
                    lock_exit(next->recv_lock);

                    if ((sending || receiving) && !timedout && !server->exit)
                        break; // Linger...

                    // If not, initiate close...
                    next->last_activity = now;
                    next->state = prx_server_socket_closing;
                    log_debug(next->log, "Worker closing socket... (%p)", next);
                    pal_socket_close(next->sock, NULL); // Wait for close complete
                    break;
                default:
                    dbg_assert(0, "Unexpected state %d", next->state);
                    break;
                }
                result = hashtable_iterator_advance(itr);
            } while (result != 0);

            crt_free(itr);
        }
    }

    if (hashtable_count(server->sockets) == 0 && server->exit)
    {
        lock_exit(server->sockets_lock);
        // Done, this clears the scheduler
        prx_server_free(server);
        return;
    }
    lock_exit(server->sockets_lock);
}

// 
// Handle link message response on open - on scheduler thread
//
static void prx_server_socket_open_complete(
    prx_server_socket_t* server_sock
)
{
    int32_t result;
    io_message_t* message;

    dbg_assert_ptr(server_sock);
    dbg_assert_is_task(server_sock->scheduler);

    message = server_sock->link_message;
    result = server_sock->last_error;

    server_sock->link_message = NULL;
    server_sock->last_error = er_ok;

    dbg_assert_ptr(message);
    dbg_assert_ptr(server_sock->server);
    do
    {
        if (result != er_ok)
            break;

        // Update response
        memset(&message->content.link_response, 0, 
            sizeof(message->content.link_response));

        result = pal_socket_getsockname(server_sock->sock,
            &message->content.link_response.local_address);
        if (result != er_ok)
            break;

        result = pal_socket_getpeername(server_sock->sock,
            &message->content.link_response.peer_address);
        if (result != er_ok)
            break;

        io_ref_copy(&server_sock->id, &message->content.link_response.link_id);
        break;
    } 
    while (0);

    if (result != er_ok)
    {
        message->error_code = result;
        log_error(server_sock->log, "Failed to link socket (%s)",
            prx_err_string(result));
        server_sock->state = prx_server_socket_collect;
        __do_next(server_sock->server, prx_server_worker);
    }

    io_message_as_response(message);
    result = io_connection_send(server_sock->server->listener, message);
    if (result != er_ok)
    {
        log_error(server_sock->log, "Failed sending link response (%s).",
            prx_err_string(result));
    }
}

//
// Close completed - on scheduler thread
//
static void prx_server_socket_close_complete(
    prx_server_socket_t* server_sock
)
{
    dbg_assert_ptr(server_sock);
    dbg_assert_is_task(server_sock->scheduler);
    server_sock->state = prx_server_socket_closed;
    log_debug(server_sock->log, "Server socket closed! (%p)", server_sock);
    __do_next(server_sock->server, prx_server_worker);
}

//
// Send all messages in the recv queue - on scheduler thread
//
static void prx_server_socket_deliver_results(
    prx_server_socket_t* server_sock
)
{
    int32_t result;
    io_message_t* message;
    dbg_assert_ptr(server_sock);
    dbg_assert_is_task(server_sock->scheduler);
    while (true)
    {
        lock_enter(server_sock->recv_lock);
        if (DList_IsListEmpty(&server_sock->recv_queue))
            message = NULL;
        else
            message = containingRecord(DList_RemoveHeadList(
                &server_sock->recv_queue), io_message_t, link);
        lock_exit(server_sock->recv_lock);
        if (!message)
            break;

        result = io_connection_send(server_sock->stream, message);
        if (result != er_ok)
        {
            log_error(server_sock->log, "Failed to send message (%s)",
                prx_err_string(result));

            // Push back to try again later...
            lock_enter(server_sock->recv_lock);
            DList_InsertHeadList(&server_sock->recv_queue, &message->link);
            lock_exit(server_sock->recv_lock);
            break;
        }

        // Done sending, release message
        io_message_release(message);
    }
}

//
// Add to receive queue to let scheduler publish on next turn. - async call
//
static void prx_server_socket_send_result(
    prx_server_socket_t* server_sock,
    io_message_t* message
)
{
    dbg_assert_ptr(server_sock);
    dbg_assert_ptr(message);

    lock_enter(server_sock->recv_lock);
    DList_InsertTailList(&server_sock->recv_queue, &message->link);
    lock_exit(server_sock->recv_lock);

    __do_next(server_sock, prx_server_socket_deliver_results);
}

//
// Begin receive operation - async call
//
static void prx_server_socket_on_begin_receive(
    prx_server_socket_t* server_sock,
    uint8_t** buffer,
    size_t* size,
    void** op_context
)
{
    int32_t result;
    io_message_t* message;

    dbg_assert_ptr(server_sock);
    dbg_assert_ptr(buffer);
    dbg_assert_ptr(op_context);
    dbg_assert(size != 0, "size");
    dbg_assert(server_sock->state != prx_server_socket_closed, "State");
    
    // Create new message from pool with a 64k buffer
    result = io_message_create(server_sock->message_pool, io_message_type_data,
        &server_sock->id, &server_sock->stream_id, &message);
    if (result == er_ok)
    {
#define BUF_SIZE 0x10000
        result = io_message_allocate_buffer(
            message, BUF_SIZE, (void**)&message->content.data_message.buffer);
        if (result == er_ok)
            message->content.data_message.buffer_length = BUF_SIZE;
    }

    if (result != er_ok)
    {
        *buffer = NULL;
        *size = 0;
        *op_context = NULL;
    }
    else 
    {
        *buffer = message->content.data_message.buffer;
        *size = message->content.data_message.buffer_length;
        *op_context = message;
    }
}

//
// End receive operation - async call
//
static void prx_server_socket_on_end_receive(
    prx_server_socket_t* server_sock,
    uint8_t** buffer,
    size_t* size,
    prx_socket_address_t* addr,
    int32_t* flags,
    int32_t result,
    void** op_context
)
{
    io_message_t* message;

    dbg_assert_ptr(server_sock);
    dbg_assert_ptr(buffer);
    dbg_assert_ptr(op_context);
    dbg_assert(size != 0, "size");

    message = *(io_message_t**)op_context;
    dbg_assert_ptr(message);
    dbg_assert(message->content.data_message.buffer == *buffer, "Bad buffer");
    do
    {
        if (result != er_ok)
        {
            if (result == er_aborted || // Abort is returned during close
                result == er_retry)  
                break;
            if (result == er_closed)
            {
                log_info(server_sock->log, "Remote close received (s: %d, %p)",
                    server_sock->state, server_sock);
                break;
            }
            
            log_error(server_sock->log, "Failed receive operation (s: %d, %s, %p)",
                server_sock->state, prx_err_string(result), server_sock);
            message->content.data_message.buffer_length = 0;

            // TODO: Decide whether to send an error to clients...
            break;
        }

        if (addr)
            memcpy(&message->content.data_message.source_address, addr, 
                sizeof(prx_socket_address_t));
        else
            message->content.data_message.source_address.un.family =
                prx_address_family_unspec;
        if (flags)
        {
            // TODO:
        }

        message->content.data_message.buffer_length = (prx_size_t)*size;
        message->error_code = result;

        server_sock->last_activity = ticks_get();
        server_sock->bytes_recvd += *size;
        prx_server_socket_send_result(server_sock, message);
        return;
    }
    while (0);
    io_message_release(message);
}

//
// Begin send operation - async call
//
static void prx_server_socket_on_begin_send(
    prx_server_socket_t* server_sock,
    uint8_t** buffer,
    size_t* size,
    prx_socket_address_t* addr,
    int32_t* flags,
    void** op_context
)
{
    io_message_t* message;

    dbg_assert_ptr(server_sock);
    dbg_assert_ptr(buffer);
    dbg_assert_ptr(flags);
    dbg_assert_ptr(op_context);
    dbg_assert(size != 0, "size");
    dbg_assert(server_sock->state != prx_server_socket_closed, "State");

    // Get message to send
    lock_enter(server_sock->send_lock);
    if (DList_IsListEmpty(&server_sock->send_queue))
        message = NULL;
    else
        message = containingRecord(DList_RemoveHeadList(&server_sock->send_queue),
            io_message_t, link);
    lock_exit(server_sock->send_lock);

    if (!message)
    {
        *buffer = NULL;
        *size = 0;
        *flags = 0;
        *op_context = NULL;
    }
    else
    {
        *buffer = message->content.data_message.buffer;
        *size = message->content.data_message.buffer_length;
        *flags = 0; // TODO
        *op_context = message;

        // Copy target address if datagram
        if (addr)
        {
            memcpy(addr, &message->content.data_message.source_address,
                sizeof(prx_socket_address_t));
        }
    }
}

//
// End send operation - async call
//
static void prx_server_socket_on_end_send(
    prx_server_socket_t* server_sock,
    uint8_t** buffer,
    size_t* size,
    int32_t result,
    void** op_context
)
{
    io_message_t* message;

    dbg_assert_ptr(server_sock);
    dbg_assert_ptr(buffer);
    dbg_assert_ptr(op_context);
    dbg_assert(size != 0, "size");

    message = *(io_message_t**)op_context;
    dbg_assert_ptr(message);

    /**/ if (result == er_retry)
    {
        // need to retry send, return to front of send_queue
        lock_enter(server_sock->send_lock);
        DList_InsertHeadList(&server_sock->send_queue, &message->link);
        lock_exit(server_sock->send_lock);
    }
    else if (result != er_ok && 
             result != er_aborted) // aborted is returned during close
    {
        log_error(server_sock->log, "Failed send operation, send %s error...",
            prx_err_string(result));

        io_message_as_response(message);
        message->error_code = result;
        message->content.data_message.buffer_length = 0;

        prx_server_socket_send_result(server_sock, message);
        return;
    }
    else
    {
        server_sock->last_activity = ticks_get();
    }
    io_message_release(message);
}

//
// Create socket on server
//
static int32_t prx_server_socket_create(
    prx_server_t* server,
    io_ref_t* owner_addr,
    prx_server_socket_t** created
);

//
// Begin accept operation - async call
//
static void prx_server_socket_on_begin_accept(
    prx_server_socket_t* server_sock,
    uint8_t** buffer,
    size_t* size,
    void** op_context
)
{
    int32_t result;
    prx_server_socket_t* accepted_sock = NULL;

    dbg_assert_ptr(server_sock);
    dbg_assert_ptr(buffer);
    dbg_assert_ptr(op_context);
    dbg_assert_ptr(size);

    // Create new socket object
    result = prx_server_socket_create(server_sock->server, &server_sock->owner, 
        &accepted_sock);
    if (result == er_ok)
    {
        // Create link message now, if we run out of memory, we fail accept
        result = io_message_create(server_sock->message_pool, io_message_type_link,
            &server_sock->id, &server_sock->stream_id, &accepted_sock->link_message);
        if (result != er_ok)
        {
            log_error(server_sock->log, "Failed to create link message (%s)",
                prx_err_string(result));
        }
    }

    if (result != er_ok)
    {
        *buffer = NULL;
        *size = 0;
        *op_context = NULL;
    }
    else
    {
        memcpy(&accepted_sock->client_itf.props, &server_sock->client_itf.props,
            sizeof(server_sock->client_itf.props));

        *buffer = (uint8_t*)&accepted_sock->client_itf;
        *size = sizeof(pal_socket_client_itf_t);
        *op_context = accepted_sock;
    }
}

//
// End accept operation - async call
//
static void prx_server_socket_on_end_accept(
    prx_server_socket_t* server_sock,
    uint8_t** buffer,
    size_t* size,
    int32_t result,
    void** op_context
)
{
    io_message_t* message = NULL;
    prx_server_socket_t* accepted_sock;

    dbg_assert_ptr(server_sock);
    dbg_assert_ptr(buffer);
    dbg_assert_ptr(size);

    accepted_sock = (prx_server_socket_t*)op_context;
    dbg_assert_ptr(accepted_sock);
    do
    {
        if (result != er_ok)
        {
            if (result == er_aborted)  // Abort is returned during close
                break;
            
            log_error(server_sock->log, "Failed to accept new socket (%s)",
                prx_err_string(result));
            // TODO: Decide whether to send an error to clients...
            // TODO: Decide whether to close server socket
            break;
        }

        dbg_assert(*size == sizeof(pal_socket_t*), 
            "Expected size of socket pointer");
        dbg_assert_ptr(*buffer);
        accepted_sock->sock = *(pal_socket_t**)buffer;
        message = accepted_sock->link_message;
        accepted_sock->link_message = NULL;
        dbg_assert_ptr(message);
        accepted_sock->last_activity = server_sock->last_activity = ticks_get();

        // Send link message
        message->content.link_request.version = LINK_VERSION;
        result = pal_socket_get_properties(
            accepted_sock->sock, &message->content.link_request.props);
        if (result != er_ok)
            break;

        message->error_code = result;
        prx_server_socket_send_result(server_sock, message);
        return;
    } 
    while (0);

    if (message)
        io_message_release(message);
    prx_server_socket_free(accepted_sock);
}

//
// Handle flow control for protocol message pool
//
static void prx_server_socket_flow_control(
    void* context,
    bool low_mem
)
{
    prx_server_socket_t* server_sock = (prx_server_socket_t*)context;
    dbg_assert_ptr(server_sock);
    if (server_sock->state != prx_server_socket_closed)
        pal_socket_can_recv(server_sock->sock, !low_mem);
}

//
// Handle new message received for a socket
//
static int32_t prx_server_socket_handler(
    void* context,
    io_connection_event_t ev,
    io_message_t* message
);

//
// handle open message - called on scheduler thread...
//
static void prx_server_socket_handle_openrequest(
    prx_server_socket_t* server_sock,
    io_message_t* message
)
{
    int32_t result;
    io_cs_t* cs = NULL;
    prx_ns_entry_t* entry = NULL;

    dbg_assert_ptr(message);
    dbg_assert_ptr(server_sock);
    dbg_assert_is_task(server_sock->scheduler);
    do
    {
        // Set as response here already, since the message is sent twice...
        io_message_as_response(message);

        /**/ if (server_sock->state == prx_server_socket_opened)
        {
            result = er_bad_state;
            break;
        }
        else if (server_sock->state != prx_server_socket_created)
        {
            result = er_closed;
            break;
        }

        io_ref_copy(&message->content.open_request.stream_id, &server_sock->stream_id);

        // Make a new protocol message factory for received messages...
        result = io_message_factory_create(
            RECV_POOL_SIZE, RECV_POOL_LWM, RECV_POOL_HWM,
            prx_server_socket_flow_control, server_sock,
            &server_sock->message_pool);
        if (result != er_ok)
            break;

        // Create new websocket connection using websocket transport
        result = io_cs_create_from_string(
            message->content.open_request.connection_string, &cs);
        if (result != er_ok)
            break;
        result = prx_ns_entry_create_from_cs(prx_ns_entry_type_link, cs, &entry);
        if (result != er_ok)
            break;
        result = io_transport_create(io_iot_hub_ws_server_transport(),
            entry, prx_server_socket_handler, server_sock,
            server_sock->server->scheduler, &server_sock->stream);
        if (result != er_ok)
            break;

        //
        // Send open response over stream as first message to allow clients to 
        // correlate the stream with the open request.
        //
        message->error_code = er_ok;
        io_ref_copy(&message->content.open_request.stream_id, &message->target_id);
        result = io_connection_send(server_sock->stream, message);
        if (result != er_ok)
            break;

        server_sock->time_opened = server_sock->last_activity = ticks_get();
        server_sock->state = prx_server_socket_opened;
        log_info(server_sock->log, "Socket open!");

        // Start receiving on our socket, sending further messages over the stream
        result = pal_socket_can_recv(server_sock->sock, true);
        if (result != er_ok)
            break;
        break;
    } 
    while (0);

    if (result != er_ok)
    {
        log_error(server_sock->log, "Failed to handle open message (%s)!",
            prx_err_string(result));
        server_sock->state = prx_server_socket_collect;
        __do_next(server_sock->server, prx_server_worker);
    }

    message->error_code = result;
    io_ref_copy(&server_sock->owner, &message->target_id);
    result = io_connection_send(server_sock->server->listener, message);
    if (result != er_ok)
    {
        log_error(server_sock->log, "Failed sending open response (%s).",
            prx_err_string(result));
    }

    if (cs)
        io_cs_free(cs);
    if (entry)
        prx_ns_entry_release(entry);
}

//
// handle close message
//
static void prx_server_socket_handle_closerequest(
    prx_server_socket_t* server_sock,
    io_message_t* message
)
{
    int32_t result;
    dbg_assert_ptr(message);
    dbg_assert_ptr(server_sock);
    dbg_assert_is_task(server_sock->scheduler);

    message->content.close_response.time_open = 
        ticks_get() - server_sock->time_opened;
    message->content.close_response.bytes_received = 
        server_sock->bytes_recvd;
    message->content.close_response.bytes_sent = 
        server_sock->bytes_sent;

    if (server_sock->state == prx_server_socket_created ||
        server_sock->state == prx_server_socket_opened)
    {
        log_debug(server_sock->log, 
            "Server socket asked to close! (%p)", server_sock);

        // Shutdown
        (void)pal_socket_setsockopt(
            server_sock->sock, prx_so_shutdown, prx_shutdown_op_both);

        // Start closing socket
        server_sock->state = prx_server_socket_collect;
        __do_next(server_sock->server, prx_server_worker);
        message->content.close_response.error_code = er_ok;
    }
    else
    {
        message->content.close_response.error_code = er_closed;
    }

    io_message_as_response(message);
    message->error_code = er_ok;
    result = io_connection_send(server_sock->server->listener, message);
    if (result != er_ok)
    {
        log_error(server_sock->log, "Failed sending close response (%s).",
            prx_err_string(result));
    }
}

//
// handle set option message
//
static void prx_server_socket_handle_setoptrequest(
    prx_server_socket_t* server_sock,
    io_message_t* message
)
{
    int32_t result;
    dbg_assert_ptr(message);
    dbg_assert_ptr(server_sock);
    dbg_assert_is_task(server_sock->scheduler);
    do
    {
        if (server_sock->state != prx_server_socket_created &&
            server_sock->state != prx_server_socket_opened)
        {
            result = er_closed;
            break;
        }

        /**/ if (message->content.setopt_request.so_val.option == prx_so_shutdown)
        {
            result = pal_socket_setsockopt(server_sock->sock, 
                prx_so_shutdown, message->content.setopt_request.so_val.value);
        }
        else
        {
            result = pal_socket_setsockopt(server_sock->sock, 
                message->content.setopt_request.so_val.option, 
                message->content.setopt_request.so_val.value);
        }

        if (result != er_ok)
            break;
 
        server_sock->last_activity = ticks_get();
    }
    while (0);

    if (result != er_ok)
        log_error(server_sock->log, "Failed to handle set option message (%s).",
            prx_err_string(result));

    io_message_as_response(message);
    message->error_code = result;
    result = io_connection_send(server_sock->server->listener, message);
    if (result != er_ok)
    {
        log_error(server_sock->log, "Failed sending set option response (%s).",
            prx_err_string(result));
    }
}

//
// handle get option message
//
static void prx_server_socket_handle_getoptrequest(
    prx_server_socket_t* server_sock,
    io_message_t* message
)
{
    int32_t result;
    dbg_assert_ptr(message);
    dbg_assert_ptr(server_sock);
    dbg_assert_is_task(server_sock->scheduler);
    do
    {
        if (server_sock->state != prx_server_socket_created &&
            server_sock->state != prx_server_socket_opened)
        {
            result = er_closed;
            break;
        }

        result = pal_socket_getsockopt(server_sock->sock, 
            message->content.getopt_request.so_opt, 
            &message->content.getopt_response.so_val.value);

        if (result != er_ok)
            break;

        message->content.getopt_response.so_val.option = 
            message->content.getopt_request.so_opt;

        server_sock->last_activity = ticks_get();
    }
    while (0);

    if (result != er_ok)
        log_error(server_sock->log, "Failed to handle get option message! (%s).",
            prx_err_string(result));

    io_message_as_response(message);
    message->error_code = result;
    result = io_connection_send(server_sock->server->listener, message);
    if (result != er_ok)
    {
        log_error(server_sock->log, "Failed sending get option response (%s).",
            prx_err_string(result));
    }
}

//
// Handle new message
//
static int32_t prx_server_socket_handler(
    void* context,
    io_connection_event_t ev,
    io_message_t* message
)
{
    int32_t result;
    prx_server_socket_t* server_sock = (prx_server_socket_t*)context;

    /**/ if (ev == io_connection_received)
    {
        dbg_assert_ptr(server_sock);
        dbg_assert_ptr(message);
        dbg_assert_is_task(server_sock->scheduler);

        /**/ if (message->type == io_message_type_data)
        {
            if (server_sock->state != prx_server_socket_opened)
                return er_closed;

            result = io_message_clone(message, &message);
            if (result != er_ok)
                return result;

            lock_enter(server_sock->send_lock);
            DList_InsertTailList(&server_sock->send_queue, &message->link);
            lock_exit(server_sock->send_lock);

            // Flow on
            pal_socket_can_send(server_sock->sock, true);
        }

        else if (message->type == io_message_type_open)
            prx_server_socket_handle_openrequest(server_sock, message);
        else if (message->type == io_message_type_getopt)
            prx_server_socket_handle_getoptrequest(server_sock, message);
        else if (message->type == io_message_type_setopt)
            prx_server_socket_handle_setoptrequest(server_sock, message);
        else if (message->type == io_message_type_close)
            prx_server_socket_handle_closerequest(server_sock, message);

        else
        {
            log_error(server_sock->log, "Received bad message type %d.",
                message->type);
            return er_not_supported;
        }
    }
    else if (ev == io_connection_closed)
    {
        // Stream closed, now free socket and exit
        log_debug(server_sock->log, "Stream closed, schedule socket %p free", 
            server_sock);
        // Delay free to debounce hang up race condition
        __do_later(server_sock, prx_server_socket_free, 2000);
    }
    return er_ok;
}

//
// Pal socket event callback 
//
static void prx_server_socket_event_handler(
    void* context,
    pal_socket_event_t ev,
    uint8_t** buffer,
    size_t* size,
    prx_socket_address_t* addr,
    int32_t* flags,
    int32_t error,
    void** op_context
)
{
    prx_server_socket_t* sock = (prx_server_socket_t*)context;
    switch (ev)
    {
    case pal_socket_event_opened:
        dbg_assert(!buffer && !size && !flags && !addr, "arg not expected.");
        sock->last_error = error;
        __do_next(sock, prx_server_socket_open_complete);
        break;
    case pal_socket_event_begin_recv:
        dbg_assert(error == er_ok, "no error expected.");
        dbg_assert(!flags && !addr, "arg not expected.");
        prx_server_socket_on_begin_receive(sock, buffer, size, op_context);
        break;
    case pal_socket_event_end_recv:
        prx_server_socket_on_end_receive(sock, buffer, size, addr, flags, error, op_context);
        break;
    case pal_socket_event_begin_accept:
        dbg_assert(error == er_ok, "no error expected.");
        dbg_assert(!flags && !addr, "arg not expected.");
        prx_server_socket_on_begin_accept(sock, buffer, size, op_context);
        break;
    case pal_socket_event_end_accept:
        dbg_assert(!flags && !addr, "arg not expected.");
        prx_server_socket_on_end_accept(sock, buffer, size, error, op_context);
        break;
    case pal_socket_event_begin_send:
        dbg_assert(error == er_ok, "no error expected.");
        prx_server_socket_on_begin_send(sock, buffer, size, addr, flags, op_context);
        break;
    case pal_socket_event_end_send:
        dbg_assert(!flags && !addr, "arg not expected.");
        prx_server_socket_on_end_send(sock, buffer, size, error, op_context);
        break;
    case pal_socket_event_closed:
        dbg_assert(!buffer && !size && !flags && !addr, "arg not expected.");
        sock->last_error = error;
        __do_next(sock, prx_server_socket_close_complete);
        break;
    default:
        dbg_assert(0, "Should not be here!");
    }
}

DEFINE_HASHTABLE_INSERT(prx_server_add, io_ref_t, prx_server_socket_t);
DEFINE_HASHTABLE_SEARCH(prx_server_get, io_ref_t, prx_server_socket_t);
DEFINE_HASHTABLE_REMOVE(prx_server_remove, io_ref_t, prx_server_socket_t);

//
// Create socket on server
//
static int32_t prx_server_socket_create(
    prx_server_t* server,
    io_ref_t* owner_addr,  
    prx_server_socket_t** created
)
{
    int32_t result;
    prx_server_socket_t* server_sock;

    dbg_assert_ptr(server);
    dbg_assert_ptr(owner_addr);
    dbg_assert_ptr(created);

    server_sock = mem_zalloc_type(prx_server_socket_t);
    if (!server_sock)
        return er_out_of_memory;
    do
    {
        DList_InitializeListHead(&server_sock->send_queue);
        DList_InitializeListHead(&server_sock->recv_queue);
        server_sock->state = prx_server_socket_created;

        // TODO: Configure this for socket in link request!
        server_sock->timeout = 30 * 1000;

        if (owner_addr)
            io_ref_copy(owner_addr, &server_sock->owner);

        result = io_ref_new(&server_sock->id);
        if (result != er_ok)
            break;

        result = lock_create(&server_sock->send_lock);
        if (result != er_ok)
            break;
        result = lock_create(&server_sock->recv_lock);
        if (result != er_ok)
            break;

        result = prx_scheduler_create(server->scheduler, &server_sock->scheduler);
        if (result != er_ok)
            break;

        server_sock->client_itf.context = server_sock;
        server_sock->client_itf.cb = prx_server_socket_event_handler;

        //
        // Insert ourselves in server and take a reference for it,
        // then wait for close req or gc.
        //
        lock_enter(server->sockets_lock);
        server_sock->server = server;
        prx_server_add(server->sockets, &server_sock->id, server_sock);
        lock_exit(server->sockets_lock);

        *created = server_sock;
        return er_ok;
    } while (0);

    prx_server_socket_free(server_sock);
    return result;
}

//
// Free all sockets in table
//
static void prx_server_free_sockets(
    prx_server_t* server
)
{
    prx_server_socket_t* next;
    struct hashtable_itr* itr;
    int result;

    dbg_assert_ptr(server);

    lock_enter(server->sockets_lock);
    if (hashtable_count(server->sockets) > 0)
    {
        itr = hashtable_iterator(server->sockets);
        do
        {
            next = (prx_server_socket_t*)hashtable_iterator_value(itr);
            log_error(server->log, "Freeing open socket %p", next);
            pal_socket_close(next->sock, NULL);
            prx_server_socket_free(next);
            result = hashtable_iterator_remove(itr);
        }
        while (result != 0);
    }
    lock_exit(server->sockets_lock);
}

// 
// Server callback to handle link message
//
static void prx_server_handle_linkrequest(
    prx_server_t* server,
    io_message_t* message
)
{
    int32_t result;
    prx_server_socket_t* server_sock = NULL;

    dbg_assert_ptr(message);
    dbg_assert_ptr(server);
    dbg_assert_is_task(server->scheduler);
    do
    {
        // Create empty socket object
        result = prx_server_socket_create(server, &message->source_id, &server_sock);
        if (result != er_ok)
            break;

        // Create socket handle
        memcpy(&server_sock->client_itf.props, &message->content.link_request.props,
            sizeof(server_sock->client_itf.props));
        result = pal_socket_create(&server_sock->client_itf, &server_sock->sock);
        if (result != er_ok)
            break;

        //
        // Start connecting the socket, which will post to socket event handler 
        // completion or error result
        //
        server_sock->link_message = message;
        server_sock->last_activity = ticks_get();
        result = pal_socket_open(server_sock->sock, NULL);
        if (result != er_ok)
            break;

        return; // Now wait for proxy socket open to complete...
    } 
    while (0);

    message->error_code = result;
    log_error(server->log, "Failed to link socket (%s)", prx_err_string(result));

    if (server_sock)
        prx_server_socket_free(server_sock);

    io_message_as_response(message);
    result = io_connection_send(server->listener, message);
    if (result != er_ok)
    {
        log_error(server->log, "Failed sending link error response (%s).",
            prx_err_string(result));
        // Let client time out
    }
}

//
// Server callback to handle resolve message
//
static void prx_server_handle_resolverequest(
    prx_server_t* server,
    io_message_t* message
)
{
    int32_t result;
    char port[MAX_PORT_LENGTH];

    dbg_assert_ptr(message);
    dbg_assert_ptr(server);
    dbg_assert_is_task(server->scheduler);
    do
    {
        // Resolve
        result = string_from_int(
            message->content.resolve_request.port, 10, port, sizeof(port));
        if (result != er_ok)
            break;

        result = pal_getaddrinfo(message->content.resolve_request.host,
            message->content.resolve_request.port != 0 ? port : NULL,
            message->content.resolve_request.family, 
            message->content.resolve_request.flags,
            &message->content.resolve_response.results, 
            &message->content.resolve_response.result_count);

        if (result != er_ok)
        {
            log_error(server->log, "Failed to resolve address for resolve message (%s)", 
                prx_err_string(result));
            break;
        }
    } 
    while (0);

    message->error_code = result;
    io_message_as_response(message);
    result = io_connection_send(server->listener, message);
    if (result != er_ok)
    {
        log_error(server->log, "Failed sending resolve response (%s).",
            prx_err_string(result));
    }
}

//
// Server callback to handle ping message
//
static void prx_server_handle_pingrequest(
    prx_server_t* server,
    io_message_t* message
)
{
    int32_t result;
    char port[MAX_PORT_LENGTH];
    char host_ip[64];
    prx_addrinfo_t* prx_ai = NULL;
    prx_size_t prx_ai_count = 0;

    // Note: In case of error, we just do not respond...

    dbg_assert_ptr(message);
    dbg_assert_ptr(server);
    dbg_assert_is_task(server->scheduler);

    // Todo: ntop and then resolve.
    // Todo: pal_ping, with SendArp or alike...

    if (message->content.ping_request.address.un.family != prx_address_family_inet6 &&
        message->content.ping_request.address.un.family != prx_address_family_inet &&
        message->content.ping_request.address.un.family != prx_address_family_proxy)
    {
        log_error(server->log, "Ping request for address with invalid address family %d",
            message->content.ping_request.address.un.family);
        return;
    }

    result = string_from_int(
        message->content.ping_request.address.un.ip.port, 10, port, sizeof(port));
    if (result != er_ok)
        return;

    if (message->content.ping_request.address.un.family == prx_address_family_proxy)
    {
        if (strlen(message->content.ping_request.address.un.proxy.host) == 0)
            return;

        result = pal_getaddrinfo(message->content.ping_request.address.un.proxy.host, port,
            prx_address_family_unspec, 0, &prx_ai, &prx_ai_count);
        if (result != er_ok)
        {
            log_error(server->log, "pal_getaddrinfo for %s:%s failed (%s).",
                message->content.ping_request.address.un.proxy.host, port,
                prx_err_string(result));
        }
    }
    else 
    {
        result = pal_ntop(&message->content.ping_request.address, host_ip, sizeof(host_ip));
        if (result != er_ok)
            return;

        result = pal_getaddrinfo(host_ip, port,
            message->content.ping_request.address.un.family, 0, &prx_ai, &prx_ai_count);
        if (result != er_ok)
        {
            log_error(server->log, "pal_getaddrinfo for %s:%s failed (%s).",
                host_ip, port, prx_err_string(result));
        }
    }

    if (prx_ai)
    {
        dbg_assert(prx_ai_count > 0, "Unexpected");
        memcpy(&message->content.ping_response, &prx_ai->address, sizeof(prx_socket_address_t));
        pal_freeaddrinfo(prx_ai);

        io_message_as_response(message);
        message->error_code = result;
        result = io_connection_send(server->listener, message);
        if (result != er_ok)
        {
            log_error(server->log, "Failed sending ping response (%s).",
                prx_err_string(result));
        }
    }
}

//
// Server callback to handle close message for dead socket
//
static void prx_server_handle_invalid_socket(
    prx_server_t* server,
    io_message_t* message
)
{
    int32_t result;

    dbg_assert_ptr(message);
    dbg_assert_ptr(server);
    dbg_assert_is_task(server->scheduler);

    message->error_code = er_closed;
    io_message_as_response(message);

    result = io_connection_send(server->listener, message);
    if (result != er_ok)
    {
        log_error(server->log, "Failed sending error response (%s).",
            prx_err_string(result));
    }
}

//
// Handles a message 
//
static int32_t prx_server_handler(
    void* context,
    io_connection_event_t ev,
    io_message_t* message
)
{
    prx_server_t* server = (prx_server_t*)context;
    prx_server_socket_t* server_sock;
    dbg_assert_ptr(server);
    dbg_assert_is_task(server->scheduler);

    /**/ if (ev == io_connection_received)
    {
        //
        // Dispatch messages to target sockets or handle here
        //
        /**/ if (!io_ref_equals(&message->target_id, &io_ref_null))
        {
            server_sock = prx_server_get(server->sockets, &message->target_id);
            if (server_sock)
            {
                // target is a socket
                return prx_server_socket_handler(server_sock, ev, message);
            }
            else
            {
                prx_server_handle_invalid_socket(server, message);
            }
        }

        else if (message->type == io_message_type_ping)
            prx_server_handle_pingrequest(server, message);
        else if (message->type == io_message_type_link)
            prx_server_handle_linkrequest(server, message);
        else if (message->type == io_message_type_resolve)
            prx_server_handle_resolverequest(server, message);

        else
        {
            log_error(server->log,
                "Received unknown message type %d in message.", message->type);

            return er_not_supported;
        }
    }
    else if (ev == io_connection_closed)
    {
        // Listener closed, free and exit
        io_connection_free(server->listener);
        server->listener = NULL;

        server->exit = true;
        __do_next(server, prx_server_worker);
    }
    return er_ok;
}

//
// Create server connected to named endpoint
//
int32_t prx_server_create(
    io_transport_t* transport,
    prx_ns_entry_t* entry,
    prx_scheduler_t* scheduler,
    prx_server_t** created
)
{
    int32_t result;
    prx_server_t* server;

    if (!transport || !created)
        return er_fault;

    server = mem_zalloc_type(prx_server_t);
    if (!server)
        return er_out_of_memory;
    do
    {
        server->timeout = 60000; // TODO: make configurable...
        server->log = log_get("proxy.server");
        io_ref_new(&server->id);

        server->sockets = create_hashtable(10, 
            (unsigned int(*) (void*)) io_ref_hash, 
            (int (*) (void*, void*))  io_ref_equals);
        if (!server->sockets)
        {
            result = er_out_of_memory;
            break;
        }

        result = lock_create(&server->sockets_lock);
        if (result != er_ok)
            break;

        result = prx_scheduler_create(scheduler, &server->scheduler);
        if (result != er_ok)
            break;

        // Create listener and register handler
        result = io_transport_create(transport, entry,
            prx_server_handler, server, server->scheduler, 
            &server->listener);
        if (result != er_ok)
            break;

        // Start server worker task
        __do_next(server, prx_server_worker);
        *created = server;
        return er_ok;
    } while (0);

    prx_server_release(server);
    return result;
}

//
// Release proxy server
//
void prx_server_release(
    prx_server_t* server
)
{
    if (!server)
        return;

    if (server->listener)
    {
        // First close listener, and wait for close to complete
        io_connection_close(server->listener);
    }
    else
    {
        server->exit = true;
        __do_next(server, prx_server_worker);
    }
    // Wait for server to exit...
}

