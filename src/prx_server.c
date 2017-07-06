// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "prx_server.h"
#include "prx_ns.h"
#include "prx_log.h"
#include "prx_buffer.h"
#include "prx_browse.h"
#include "prx_config.h"

#include "pal.h"
#include "pal_sk.h"
#include "pal_net.h"
#include "pal_mt.h"
#include "pal_time.h"

#include "io_proto.h"
#include "io_transport.h"
#include "util_string.h"

#include "hashtable.h"
#include "hashtable_itr.h"

//
// Handles socket layer targeted requests
//
struct prx_server
{
    io_ref_t id;              // Server id == Proxy id == Module entry id
    io_connection_t* listener;     // connection used by server to listen
    io_transport_t* transport;                 // Transport instance used
    prx_browse_server_t* browser;              // Internal browser server
#define DEFAULT_RESTRICTED_PORTS ""
    int32_t* restricted_ports;    // Tuple range list of restricted ports
    size_t restricted_port_count;       // Number of tuples in restricted
    struct hashtable* sockets;                  // List of active sockets
    lock_t sockets_lock;
    prx_scheduler_t* scheduler; // All server tasks are on this scheduler
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
    prx_scheduler_t* scheduler; // All socket tasks are on this scheduler
    io_message_t* link_message;       // Message used to link this socket

    prx_server_socket_state_t state;
    int32_t last_error;             // Last error that occurred on socket
#define MIN_GC_TIMEOUT 10000
#define DEFAULT_GC_TIMEOUT 30000
#define LINGER_TIMEOUT 1 * DEFAULT_GC_TIMEOUT
#define CLOSING_TIMEOUT 1 * DEFAULT_GC_TIMEOUT
    ticks_t last_activity;        // Last activitiy on this socket for gc
    ticks_t time_opened;

    io_ref_t id;                                         // Local link id
    io_ref_t owner;  // Remote socket id for link used for socket control

    io_ref_t stream_id;      // Remote stream id for link, used to stream
    bool server_stream;      // Whether the stream is the server listener
    io_connection_t* stream;       // Local stream connection once opened
    bool polled;                     // Whether the stream will be polled
    DLIST_ENTRY read_queue;            // Poll message queue, when polled
    DLIST_ENTRY write_queue;       // Send response queue, or error queue

    size_t bytes_sent;            // Number of bytes sent from send queue
    DLIST_ENTRY send_queue;                  // Sender queue, from stream
    lock_t send_lock;                // Lock to guard multi thread access
            // - and -
    size_t bytes_recvd;
    DLIST_ENTRY recv_queue;                  // Receiver queue, to stream
    lock_t recv_lock;                // Lock to guard multi thread access

    io_message_factory_t* message_pool;                  // Receiver pool
    size_t buffer_size;                        // Cached recv buffer size
    size_t pool_size;           // Number of preallocated buffers in pool
#define RECV_POOL_MIN 4              // Minimum number of buffers in pool
#define RECV_POOL_MAX 0x20000              // Max size of pool per socket
#define RECV_POOL_LWM 1  // Flow off when we hit one message left and ...
#define RECV_POOL_HWM 1   // ... on when we have all but one back in pool
    log_t log;
}
prx_server_socket_t;

DEFINE_HASHTABLE_INSERT(prx_server_add, io_ref_t, prx_server_socket_t);
DEFINE_HASHTABLE_SEARCH(prx_server_get, io_ref_t, prx_server_socket_t);
DEFINE_HASHTABLE_REMOVE(prx_server_remove, io_ref_t, prx_server_socket_t);

//
// Websocket server transport (stream)
//
extern io_transport_t* io_iot_hub_ws_server_transport(
    void
);

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

    if (server->browser)
        prx_browse_server_free(server->browser);

    if (server->scheduler)
        prx_scheduler_release(server->scheduler, server);

    dbg_assert(hashtable_count(server->sockets) == 0,
        "socketlist should now be empty");
    if (server->sockets)
        hashtable_destroy(server->sockets, 0);

    if (server->sockets_lock)
        lock_free(server->sockets_lock);

    if (server->restricted_ports)
        mem_free(server->restricted_ports);

    log_trace(server->log, "Freeing server.");
    mem_free_type(prx_server_t, server);
}

//
// Check whether port is restricted
//
static int32_t prx_server_check_restricted_port(
    prx_server_t* server,
    uint16_t port
)
{
    size_t index = 0;
    dbg_assert_ptr(server);
    // Go through tuples and check whether port is in range...
    while (index < server->restricted_port_count * 2)
    {
        if ((int32_t)port >= server->restricted_ports[index++] &&
            (int32_t)port <= server->restricted_ports[index++])
        {
            log_trace(server->log, "Blocking access to port %d", (int32_t)port);
            return er_refused;
        }
    }
    return er_ok;
}

//
// Clear transport send and receive queues
//
static void prx_server_socket_empty_transport_queues(
    prx_server_socket_t* server_sock
)
{
    dbg_assert_ptr(server_sock);
    dbg_assert_ptr(server_sock->send_lock);
    dbg_assert_ptr(server_sock->recv_lock);

    // Write queue - Send/poll responses when polling and errors in any case
    lock_enter(server_sock->send_lock);
    while (!DList_IsListEmpty(&server_sock->write_queue))
    {
        io_message_release(containingRecord(DList_RemoveHeadList(
            &server_sock->write_queue), io_message_t, link));
    }
    lock_exit(server_sock->send_lock);

    // Read queue - only used when polling - all are allocated in connection.
    lock_enter(server_sock->recv_lock);
    dbg_assert(server_sock->polled || DList_IsListEmpty(&server_sock->read_queue), 
        "should be empty when not polled");
    while (!DList_IsListEmpty(&server_sock->read_queue))
    {
        io_message_release(containingRecord(DList_RemoveHeadList(
            &server_sock->read_queue), io_message_t, link));
    }
    lock_exit(server_sock->recv_lock);
}

//
// Clear pal socket send and receive queues
//
static void prx_server_socket_empty_socket_queues(
    prx_server_socket_t* server_sock
)
{
    dbg_assert_ptr(server_sock);
    dbg_assert_ptr(server_sock->send_lock);
    dbg_assert_ptr(server_sock->recv_lock);

    // Messages to send through socket and allocated in connection
    lock_enter(server_sock->send_lock);
    while (!DList_IsListEmpty(&server_sock->send_queue))
    {
        io_message_release(containingRecord(DList_RemoveHeadList(
            &server_sock->send_queue), io_message_t, link));
    }
    lock_exit(server_sock->send_lock);

    // Messages received on the socket and allocated in the socket
    lock_enter(server_sock->recv_lock);
    while (!DList_IsListEmpty(&server_sock->recv_queue))
    {
        io_message_release(containingRecord(DList_RemoveHeadList(
            &server_sock->recv_queue), io_message_t, link));
    }
    lock_exit(server_sock->recv_lock);
}

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
    if (server_sock->stream && !server_sock->server_stream)
        io_connection_free(server_sock->stream);

    if (server_sock->link_message)
        io_message_release(server_sock->link_message);

    dbg_assert(DList_IsListEmpty(&server_sock->read_queue), 
        "read_queue not empty");
    dbg_assert(DList_IsListEmpty(&server_sock->recv_queue), 
        "recv_queue not empty");
    dbg_assert(DList_IsListEmpty(&server_sock->send_queue), 
        "send_queue not empty");
    dbg_assert(DList_IsListEmpty(&server_sock->write_queue), 
        "write_queue not empty");

    if (server_sock->send_lock && server_sock->recv_lock)
    {
        prx_server_socket_empty_socket_queues(server_sock);
        prx_server_socket_empty_transport_queues(server_sock);
    }

    if (server_sock->message_pool)
        io_message_factory_free(server_sock->message_pool);

    if (server_sock->send_lock)
        lock_free(server_sock->send_lock);
    if (server_sock->recv_lock)
        lock_free(server_sock->recv_lock);

    if (server_sock->scheduler)
        prx_scheduler_release(server_sock->scheduler, server_sock);

    log_info(server_sock->log, "Server socket %p destroyed!", server_sock);
    mem_free_type(prx_server_socket_t, server_sock);
}

//
// Check pending poll requests and handle timeouts
//
static void prx_server_socket_manage_read_queue(
    prx_server_socket_t* server_sock,
    bool close
)
{
    int32_t result;
    ticks_t now;
    io_message_t* next;

    dbg_assert_ptr(server_sock);

    now = ticks_get();
    for (PDLIST_ENTRY p = server_sock->read_queue.Flink;
        p != &server_sock->read_queue; )
    {
        next = containingRecord(p, io_message_t, link);
        if (close || (long)next->content.poll_message.timeout >= now)
        {
            p = p->Flink;
            DList_RemoveEntryList(&next->link);

            io_message_as_response(next);
            next->error_code = close ? er_closed : er_timeout;

            result = io_connection_send(server_sock->stream, next);
            if (result != er_ok)
            {
                log_error(server_sock->log, 
                    "Failed to send poll response, dropping message... (%s)",
                    prx_err_string(result));
            }
            io_message_release(next);
            continue;
        }
        p = p->Flink;
    }
}

//
// Get next received message
//
static io_message_t* prx_server_socket_pop_received_message(
    prx_server_socket_t* server_sock
)
{
    io_message_t* message;
    dbg_assert_ptr(server_sock);

    lock_enter(server_sock->recv_lock);
    if (DList_IsListEmpty(&server_sock->recv_queue))
        message = NULL;
    else
        message = containingRecord(DList_RemoveHeadList(
            &server_sock->recv_queue), io_message_t, link);
    lock_exit(server_sock->recv_lock);
    return message;
}

//
// Get next sent message
//
static io_message_t* prx_server_socket_pop_sent_message(
    prx_server_socket_t* server_sock
)
{
    io_message_t* message;
    dbg_assert_ptr(server_sock);

    lock_enter(server_sock->send_lock);
    if (DList_IsListEmpty(&server_sock->write_queue))
        message = NULL;
    else
        message = containingRecord(DList_RemoveHeadList(
            &server_sock->write_queue), io_message_t, link);
    lock_exit(server_sock->send_lock);
    return message;
}

//
// Send all messages in the recv queue - on scheduler thread
//
static void prx_server_socket_deliver_results(
    prx_server_socket_t* server_sock
)
{
    int32_t result;
    io_message_t* message, *poll_message = NULL;

    dbg_assert_ptr(server_sock);
    dbg_assert_is_task(server_sock->scheduler);


    if (server_sock->state != prx_server_socket_opened && 
        server_sock->state != prx_server_socket_collect)
    {
        log_debug(server_sock->log, "No deliveries for socket %p when in state %d.", 
            server_sock, server_sock->state);
        return;
    }

    while (true)
    {
        message = NULL;
        poll_message = prx_server_socket_pop_sent_message(server_sock);
        if (!poll_message)
            break;

        DList_InitializeListHead(&poll_message->link);

        //
        // Check if message is a poll response - if it is - see if we can substitute
        // it for a received data message to utilize the return trip
        //
        if (server_sock->polled && 
            poll_message->type == io_message_type_poll && 
            poll_message->error_code == er_ok)
        {
            message = prx_server_socket_pop_received_message(server_sock);
            if (message)
            {
                message->correlation_id = poll_message->correlation_id;
                io_ref_copy(&server_sock->server->id, &message->proxy_id);
                io_ref_copy(&server_sock->stream_id, &message->target_id);
                io_ref_copy(&server_sock->id, &message->source_id);

                DList_InitializeListHead(&message->link);
            }
        }

        result = io_connection_send(server_sock->stream, message ? message : poll_message);
        if (result != er_ok)
        {
            log_error(server_sock->log, "Failed to write sent message to stream (%s)",
                prx_err_string(result));

            // Undo --- Push to front of send queue to try again later...
            lock_enter(server_sock->send_lock);
            DList_InsertHeadList(&server_sock->write_queue, &poll_message->link);
            lock_exit(server_sock->send_lock);

            if (message)
            {
                // Also push to front of receive queue to try again later...
                lock_enter(server_sock->recv_lock);
                DList_InsertHeadList(&server_sock->recv_queue, &message->link);
                lock_exit(server_sock->recv_lock);
            }
            break;  // Break for now... TODO: Consider reschedule
        }

        // Done sending, release message
        io_message_release(poll_message);
        if (message)
            io_message_release(message);
    }

    while (true)
    {
        // Get remaining messages from receive queue to send
        if (server_sock->polled && DList_IsListEmpty(&server_sock->read_queue))
            break; // Done

        message = prx_server_socket_pop_received_message(server_sock);
        if (!message)
            break;
        DList_InitializeListHead(&message->link);

        // Pick a poll message's correlation id, if socket is polled
        if (server_sock->polled)
        {
            poll_message = containingRecord(DList_RemoveHeadList(
                &server_sock->read_queue), io_message_t, link);
            dbg_assert_ptr(poll_message);

            DList_InitializeListHead(&poll_message->link);
            io_message_as_response(poll_message);

            // Copy correlation id information and addresses before sending
            message->correlation_id = poll_message->correlation_id;
        }

        io_ref_copy(&server_sock->server->id, &message->proxy_id);
        io_ref_copy(&server_sock->stream_id, &message->target_id);
        io_ref_copy(&server_sock->id, &message->source_id);

        result = io_connection_send(server_sock->stream, message);
        if (result != er_ok)
        {
            log_error(server_sock->log, "Failed to write recv message to stream (%s)",
                prx_err_string(result));

            // Undo --- Push to front of receive queue to try again later...
            lock_enter(server_sock->recv_lock);
            DList_InsertHeadList(&server_sock->recv_queue, &message->link);
            lock_exit(server_sock->recv_lock);
            
            if (poll_message)
            {
                // Also push to front of poll queue if poll message...
                DList_InsertHeadList(&server_sock->read_queue, &poll_message->link);
            }
            break;  // Break for now... TODO: Consider reschedule
        }

        // Done sending, release message
        io_message_release(message);
        if (poll_message)
            io_message_release(poll_message);
    }
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
    io_message_t* closerequest;
    int result;
    bool sending, receiving, timedout;
    ticks_t now;

    dbg_assert_ptr(server);
    dbg_assert_is_task(server->scheduler);
    now = ticks_get();

    // Schedule again in 5 seconds
    prx_scheduler_clear(server->scheduler, (prx_task_t)prx_server_worker, server);
    __do_later(server, prx_server_worker, 10000);

    lock_enter(server->sockets_lock);
    if (hashtable_count(server->sockets) > 0)
    {
        itr = hashtable_iterator(server->sockets);
        if (itr) // See hashtable_itr - todo: Change to use stack allocated
        {
            do
            {
                next = (prx_server_socket_t*)hashtable_iterator_value(itr);
                dbg_assert_ptr(next);
                if (next->last_activity == 0 || next->client_itf.props.timeout == 0 ||
                    (uint64_t)(now - next->last_activity) < next->client_itf.props.timeout)
                {
                    timedout = false;
                }
                else
                {
                    // State timed out - reset timeout to default timeout now
                    timedout = true;  
                    next->client_itf.props.timeout = DEFAULT_GC_TIMEOUT;
                }

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

                    prx_server_socket_empty_socket_queues(next);
                    prx_server_socket_empty_transport_queues(next);

                    if (next->stream && !next->server_stream)
                    {
                        log_trace(next->log, "Socket %p closed, clean up stream", next);
                        io_connection_close(next->stream);
                    }
                    else
                    {
                        next->stream = NULL;
                        // Delay free to debounce hang up race condition
                        __do_later(next, prx_server_socket_free, 2000);
                    }
                    result = hashtable_iterator_remove(itr);
                    break;

                case prx_server_socket_opened:
                    if (!timedout && !server->exit)
                        break;

                    if (timedout && !server->exit)
                    {
                        log_info(next->log, "No activity on socket %p, closing...",
                            next);

                        //
                        // Free inbound and outbound socket queues now to make room 
                        // for close message, if we still run out of memory, continue on 
                        //
                        prx_server_socket_empty_socket_queues(next);

                        result = io_message_create(next->message_pool,
                            io_message_type_close, &next->id, &next->stream_id,
                            &closerequest);
                        if (result == er_ok)
                        {
                            lock_enter(next->recv_lock);
                            DList_InsertTailList(&next->recv_queue, &closerequest->link);
                            lock_exit(next->recv_lock);

                            // Deliver close message to stream
                            next->last_activity = now;
                            prx_server_socket_deliver_results(next);
                            next->state = prx_server_socket_collect;
                            break;
                        }
                    }

                    next->state = prx_server_socket_created;

                    // Fall through
                case prx_server_socket_created:
                    if (!timedout && !server->exit)
                        break;
                    if (timedout && !server->exit)
                    {
                        log_info(next->log, "No activity on socket %p, destroying...",
                            next);
                    }

                    prx_server_socket_empty_socket_queues(next);
                    next->state = prx_server_socket_collect;

                    // Fall through
                case prx_server_socket_collect:
                    if (!next->sock)
                    {
                        next->state = prx_server_socket_closed;
                        break;
                    }

                    prx_server_socket_deliver_results(next);

                    // Check whether send and recv queue are still full...
                    lock_enter(next->send_lock);
                    sending = !DList_IsListEmpty(&next->send_queue);
                    lock_exit(next->send_lock);

                    lock_enter(next->recv_lock);
                    receiving = !DList_IsListEmpty(&next->recv_queue);
                    lock_exit(next->recv_lock);

                    if ((sending || receiving) && !timedout && !server->exit)
                    {
                        // ... if so linger...
                        next->client_itf.props.timeout = LINGER_TIMEOUT;
                        break; 
                    }

                    // If not, initiate close - answer all poll requests first.
                    prx_server_socket_manage_read_queue(next, true);

                    // Now enter closing state - from here on no more sending...
                    log_trace(next->log, "Worker closing socket %p...", next);
                    next->client_itf.props.timeout = CLOSING_TIMEOUT;
                    next->last_activity = now;
                    next->state = prx_server_socket_closing;
                    pal_socket_close(next->sock); 
                    // Wait for close complete or timeout
                    break;
                default:
                    dbg_assert(0, "Unexpected state %d", next->state);
                    break;
                }
                result = hashtable_iterator_advance(itr);
            }
            while (result != 0);

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
    io_connection_t* responder;

    dbg_assert_ptr(server_sock);
    dbg_assert_is_task(server_sock->scheduler);

    message = server_sock->link_message;
    result = server_sock->last_error;
    responder = (io_connection_t*)message->context;

    server_sock->link_message = NULL;
    server_sock->last_error = er_ok;
    message->context = NULL;

    dbg_assert_ptr(message);
    dbg_assert_ptr(server_sock->server);

    io_message_as_response(message);
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
        server_sock->state = prx_server_socket_created;
        __do_next(server_sock->server, prx_server_worker);
    }

    if (responder)
    {
        result = io_connection_send(responder, message);
        if (result != er_ok)
        {
            log_error(server_sock->log, "Failed sending link response (%s).",
                prx_err_string(result));
        }
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
    server_sock->last_activity = ticks_get();
    server_sock->state = prx_server_socket_closed;
    log_debug(server_sock->log, "Server socket closed! (%p)", server_sock);
    __do_next(server_sock->server, prx_server_worker);
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
        result = io_message_allocate_buffer(message, server_sock->buffer_size,
            (void**)&message->content.data_message.buffer);
        if (result == er_ok)
            message->content.data_message.buffer_length = server_sock->buffer_size;
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

    dbg_assert_ptr(op_context);
    dbg_assert_ptr(size);
    dbg_assert_ptr(server_sock);
    dbg_assert_ptr(buffer);

    message = *(io_message_t**)op_context;
    dbg_assert_ptr(message);
    dbg_assert(message->content.data_message.buffer == *buffer, "Bad buffer");
    do
    {
        (void)buffer;
        // log_trace(server_sock->log, "received %d bytes", *size);
        // log_trace_b(server_sock->log, (const char*)*buffer, *size);

        if (result == er_aborted || // Abort is returned during close
            result == er_retry)
        {
            io_message_release(message);
            return;  // No need to send to receiver
        }

        message->content.data_message.buffer_length = *size;
            
        if (result == er_closed || result == er_reset)
        {
            log_info(server_sock->log, "Remote close received (s: %d, %p)",
                server_sock->state, server_sock);
            server_sock->state = prx_server_socket_collect;
            __do_next(server_sock->server, prx_server_worker);
            break;
        }

        if (result != er_ok)
        {
            log_error(server_sock->log, "Failed receive operation (s: %d, %s, %p)",
                server_sock->state, prx_err_string(result), server_sock);
            break;
        }

        // TODO:
        (void)flags;
        // if (flags) + control buffer...

        if (addr)
        {
            memcpy(&message->content.data_message.source_address, addr,
                sizeof(prx_socket_address_t));
            break;
        }

        message->content.data_message.source_address.un.family =
            prx_address_family_unspec;
        break;
    }
    while (0);

    message->error_code = result;
    server_sock->last_activity = ticks_get();
    server_sock->bytes_recvd += *size;

    lock_enter(server_sock->recv_lock);
    DList_InsertTailList(&server_sock->recv_queue, &message->link);
    lock_exit(server_sock->recv_lock);
    __do_next(server_sock, prx_server_socket_deliver_results);
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
    uint64_t seq;

    dbg_assert_ptr(server_sock);
    dbg_assert_ptr(buffer);
    dbg_assert_ptr(op_context);
    dbg_assert_ptr(size);

    message = *(io_message_t**)op_context;
    dbg_assert_ptr(message);

    (void)buffer;
    (void)size;

    // log_trace(server_sock->log, "sent %d bytes", *size);
    // log_trace_b(server_sock->log, (const char*)*buffer, *size);

    /**/ if (result == er_retry)
    {
        // need to retry send, return to front of send_queue
        log_trace(server_sock->log, "Retrying send...");

        lock_enter(server_sock->send_lock);
        DList_InsertHeadList(&server_sock->send_queue, &message->link);
        lock_exit(server_sock->send_lock);

        return;
    }

    else if (result == er_ok)
    {
        if (*size == message->content.data_message.buffer_length)
        {
            server_sock->last_activity = ticks_get();

            if (!server_sock->polled)
            {
                //
                // no need to send a response to a successfully sent message
                // when streaming.  Only ack on poll for now.
                //
                io_message_release(message);
                return;
            }
        }
        else
        {
            dbg_assert(0, "Unexpected: sent length does not match up");
            result = er_writing;
        }
    }
    else if (result == er_aborted) // aborted is returned during close
    {
        log_debug(server_sock->log, "Operation aborted...");
        io_message_release(message);
        return;
    }
    else if (result != er_closed && result != er_reset)
    {
        log_error(server_sock->log, "Failed send operation, return %s error...",
            prx_err_string(result));
    }

    //
    // Send back a poll response with the send result.  If not polled, this is
    // done only in case of an error sending.
    //
    io_message_as_response(message);
    seq = message->content.data_message.sequence_number;
    message->type = io_message_type_poll;
    
    message->content.poll_message.sequence_number = seq;
    message->content.poll_message.timeout = 0;
    message->error_code = result;

    lock_enter(server_sock->send_lock);
    DList_InsertTailList(&server_sock->write_queue, &message->link);
    lock_exit(server_sock->send_lock);

    __do_next(server_sock, prx_server_socket_deliver_results);

    if (result == er_closed || result == er_reset)
    {
        log_info(server_sock->log, "Remote side closed - collecting socket...");
        server_sock->state = prx_server_socket_collect;
        __do_next(server_sock->server, prx_server_worker);
    }
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
        (void)size;

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
        lock_enter(server_sock->recv_lock);
        DList_InsertTailList(&server_sock->recv_queue, &message->link);
        lock_exit(server_sock->recv_lock);
        __do_next(server_sock, prx_server_socket_deliver_results);
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
    if (server_sock->state == prx_server_socket_opened)
        pal_socket_can_recv(server_sock->sock, !low_mem);
}

//
// Handle new message received for a socket
//
static int32_t prx_server_socket_stream_handler(
    void* context,
    io_connection_event_t ev,
    io_message_t* message,
    int32_t last_error
);

//
// Handle new message received for a socket
//
static int32_t prx_server_socket_control_handler(
    void* context,
    io_connection_event_t ev,
    io_message_t* message,
    int32_t last_error
);

//
// handle open message - called on scheduler thread...
//
static int32_t prx_server_socket_handle_openrequest(
    prx_server_socket_t* server_sock,
    io_connection_t* responder,
    io_message_t* message
)
{
    int32_t result;
    uint64_t value;
    io_cs_t* cs = NULL;
    prx_ns_entry_t* entry = NULL;

    dbg_assert_ptr(message);
    dbg_assert_ptr(responder);
    dbg_assert_ptr(server_sock);
    dbg_assert_is_task(server_sock->scheduler);
    do
    {
        // Set as response here already, since the message is sent twice...
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
        server_sock->polled = message->content.open_request.polled;
        server_sock->buffer_size = (size_t)message->content.open_request.max_recv;
        // If no buffer size is configured, read recv buffer from socket...
        if (!server_sock->buffer_size)
        {
            result = pal_socket_getsockopt(server_sock->sock, prx_so_rcvbuf, &value);
            if (result != er_ok)
            {
                if (result != er_not_supported)
                {
                    log_error(server_sock->log, "Failed to read receive buffer size (%s)"
                        " using default value.", prx_err_string(result));
                }
#define MAX_BUF_SIZE 0x10000
                server_sock->buffer_size = MAX_BUF_SIZE;
            }
            else
            {
                server_sock->buffer_size = (size_t)value;
            }
        }

        server_sock->pool_size = (RECV_POOL_MAX / server_sock->buffer_size);
        if (server_sock->pool_size < RECV_POOL_MIN)
            server_sock->pool_size = RECV_POOL_MIN;

        dbg_assert(server_sock->pool_size >= RECV_POOL_MIN, "Must have set pool size");
        // Make a new protocol message factory for received messages...
        result = io_message_factory_create(server_sock->pool_size, RECV_POOL_LWM, 
            server_sock->pool_size - RECV_POOL_HWM, prx_server_socket_flow_control, 
            server_sock, &server_sock->message_pool);
        if (result != er_ok)
            break;

        if (message->content.open_request.connection_string &&
            strlen(message->content.open_request.connection_string) > 0)
        {
            if (message->content.open_request.type == 0)
            {
                switch (message->content.open_request.encoding)
                {
                case io_codec_auto:
                case io_codec_mpack:
                case io_codec_json:
                    result = er_ok;
                    break;
                default:
                    log_error(server_sock->log, "Stream encoding %d not supported.",
                        message->content.open_request.encoding);
                    result = er_not_supported;
                    break;
                }
                if (result != er_ok)
                    break;
                // Create new websocket connection using websocket transport
                result = io_cs_create_from_string(
                    message->content.open_request.connection_string, &cs);
                if (result != er_ok)
                    break;
                result = prx_ns_entry_create_from_cs(
                    prx_ns_entry_type_link, &server_sock->stream_id, cs, &entry);
                if (result != er_ok)
                    break;
            }
            else
            {
                log_error(server_sock->log, "Connection string type %d not supported.",
                    message->content.open_request.type);
                result = er_not_supported;
                break;
            }
        }
        else if (!server_sock->polled)
        {
            result = er_invalid_format;
            log_error(server_sock->log, "Open request contained no connection string"
                " but polling flag was false. Since server connection is always polled,"
                " this is an invalid combination.");
            break;
        }

        if (!entry)
        {
            // If no entry, and previously not errored out, we will be polled on 
            // server listener connection which forwards to socket control handler.
            server_sock->stream = server_sock->server->listener;
            server_sock->server_stream = true;
            result = er_ok; 
        }
        else if (server_sock->polled)
        {
            // Polled streams assume the server transport posting to control handler
            result = io_transport_create(server_sock->server->transport, 
                entry, (io_codec_id_t)message->content.open_request.encoding, 
                prx_server_socket_control_handler, server_sock,
                server_sock->server->scheduler, &server_sock->stream);
        }
        else if (pal_caps() & pal_cap_wsclient)
        {
            // ... otherwise transport is websocket based, and stream handler.
            result = io_transport_create(io_iot_hub_ws_server_transport(),
                entry, (io_codec_id_t)message->content.open_request.encoding,
                prx_server_socket_stream_handler, server_sock,
                server_sock->server->scheduler, &server_sock->stream);
        }
        else
        {
            result = er_not_supported;
        }
        if (result != er_ok)
            break;

        // Start receiving on our socket immediately, sending first messages
        result = pal_socket_can_recv(server_sock->sock, true);
        if (result != er_ok)
            break;

        server_sock->time_opened = server_sock->last_activity = ticks_get();
        server_sock->state = prx_server_socket_opened;
        log_info(server_sock->log, "Socket open!");
    } 
    while (0);

    io_message_as_response(message);
    if (result == er_ok)
    {
        message->error_code = er_ok;
        io_ref_copy(&server_sock->stream_id, &message->target_id);
    } 
    else
    {
        log_error(server_sock->log, "Failed to handle open message (%s)!",
            prx_err_string(result));
        server_sock->state = prx_server_socket_created;
        __do_next(server_sock->server, prx_server_worker);
    }

    message->error_code = result;
    io_ref_copy(&server_sock->owner, &message->target_id);

    result = io_connection_send(responder, message);
    if (result != er_ok)
    {
        log_error(server_sock->log, "Failed sending open response (%s).",
            prx_err_string(result));
    }

    if (cs)
        io_cs_free(cs);
    if (entry)
        prx_ns_entry_release(entry);
    return result;
}

//
// handle sending data
//
static int32_t prx_server_socket_handle_datamessage(
    prx_server_socket_t* server_sock,
    io_connection_t* responder,
    io_message_t* message
)
{
    int32_t result;
    dbg_assert_ptr(message);
    dbg_assert_ptr(server_sock);
    dbg_assert_is_task(server_sock->scheduler);

   // log_trace_b(server_sock->log, (const char*)message->content.data_message.buffer, 
   //     message->content.data_message.buffer_length);

    if (server_sock->state != prx_server_socket_opened)
    {
        log_info(server_sock->log, "Received data after close!");

        // Socket is closed, send error response, or otherwise swallow
        if (server_sock->polled)
        {
            io_message_as_response(message);
            message->error_code = er_closed;

            result = io_connection_send(responder, message);
            if (result != er_ok)
            {
                log_error(server_sock->log, "Failed send error response (%s).",
                    prx_err_string(result));
                return result;
            }
        }
        return er_ok; 
    }

    dbg_assert(!responder || server_sock->stream == responder,
        "Expected no responder, or stream to be responder");

    result = io_message_clone(message, &message);
    if (result == er_ok)
    {
        lock_enter(server_sock->send_lock);
        DList_InsertTailList(&server_sock->send_queue, &message->link);
        lock_exit(server_sock->send_lock);

        // Flow on
        pal_socket_can_send(server_sock->sock, true);
    }
    return result;
}

//
// Check pending poll requests and handle timeouts
//
static void prx_server_socket_timeout_pollrequest(
    prx_server_socket_t* server_sock
)
{
    dbg_assert_ptr(server_sock);
    dbg_assert_is_task(server_sock->scheduler);
    prx_server_socket_manage_read_queue(server_sock, false);
}

//
// handle polling data
//
static int32_t prx_server_socket_handle_pollrequest(
    prx_server_socket_t* server_sock,
    io_connection_t* responder,
    io_message_t* message
)
{
    int32_t result;
    uint64_t timeout;
    ticks_t now;
    dbg_assert_ptr(message);
    dbg_assert_ptr(server_sock);
    dbg_assert_is_task(server_sock->scheduler);

    if (server_sock->state != prx_server_socket_opened)
    {
        // Socket is closed, send error response, or otherwise swallow
        if (server_sock->polled)
        {
            io_message_as_response(message);
            message->error_code = er_closed;

            result = io_connection_send(responder, message);
            if (result != er_ok)
            {
                log_error(server_sock->log, "Failed poll error response (%s).",
                    prx_err_string(result));
                return result;
            }
        }
        return er_ok;
    }

    //
    // Reset activity timer and prolong socket gc to make a multiple 
    // of the timeout.  If not polled, then poll message acts as keep
    // alive for the stream.
    //
    now = ticks_get();

    timeout = message->content.poll_message.timeout;
    server_sock->client_itf.props.timeout = (((uint32_t)timeout) * 3);
    server_sock->last_activity = now;

    if (!server_sock->polled)
        return er_ok;

    dbg_assert(responder && server_sock->stream == responder,
        "Expected stream to be responder");

    result = io_message_clone(message, &message);
    if (result == er_ok)
    {
        // Make absolute timeout so we can gc this poll request
        message->content.poll_message.timeout = now + timeout;
        DList_InsertTailList(&server_sock->read_queue, &message->link);

        // Do one round of deliveries - then check if there is more left...
        prx_server_socket_deliver_results(server_sock);

        if (!DList_IsListEmpty(&server_sock->read_queue))
        {
            // ... if still poll requests left, flow on and wait
            pal_socket_can_recv(server_sock->sock, true);

            // Timeout after the specified timeout period
            __do_later(server_sock, prx_server_socket_timeout_pollrequest,
                (uint32_t)timeout);
        }
    }
    return result;
}

//
// handle close message
//
static int32_t prx_server_socket_handle_closerequest(
    prx_server_socket_t* server_sock,
    io_connection_t* responder,
    io_message_t* message
)
{
    int32_t result;
    dbg_assert_ptr(message);
    dbg_assert_ptr(server_sock);
    dbg_assert_is_task(server_sock->scheduler);

    io_message_as_response(message);

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

        // Collect socket - linger to drain queues
        server_sock->state = prx_server_socket_collect;
        __do_next(server_sock->server, prx_server_worker);
        message->content.close_response.error_code = er_ok;
    }
    else
    {
        message->content.close_response.error_code = er_closed;
    }

    if (responder)
    {
        message->error_code = er_ok;
        result = io_connection_send(responder, message);
        if (result != er_ok)
        {
            log_error(server_sock->log, "Failed sending close response (%s).",
                prx_err_string(result));
            return result;
        }
    }
    return er_ok;
}

//
// handle set option message
//
static int32_t prx_server_socket_handle_setoptrequest(
    prx_server_socket_t* server_sock,
    io_connection_t* responder,
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

        server_sock->last_activity = ticks_get();

        /**/ if (message->content.setopt_request.so_val.type == prx_so_ip_multicast_join)
        {
            result = pal_socket_join_multicast_group(server_sock->sock,
                &message->content.setopt_request.so_val.property.mcast);
            if (result != er_ok)
                break;
            log_trace(server_sock->log, "Joined multicast group...");
        }
        else if (message->content.setopt_request.so_val.type == prx_so_ip_multicast_leave)
        {
            result = pal_socket_leave_multicast_group(server_sock->sock,
                &message->content.setopt_request.so_val.property.mcast);
            if (result != er_ok)
                break;
            log_trace(server_sock->log, "Left multicast group...");
        }
        else if (message->content.setopt_request.so_val.type == prx_so_props_timeout)
        {
            server_sock->client_itf.props.timeout = 
                message->content.setopt_request.so_val.property.value;
            result = er_ok;
            log_trace(server_sock->log, "Wrote socket gc timeout as %ull...",
                message->content.getopt_response.so_val.property.value);
        }
        else if (message->content.setopt_request.so_val.type < __prx_so_max)
        {
            result = pal_socket_setsockopt(server_sock->sock, 
                (prx_socket_option_t)message->content.setopt_request.so_val.type,
                message->content.setopt_request.so_val.property.value);
            if (result != er_ok)
                break;
            log_trace(server_sock->log, "Wrote socket option %d as %ull...",
                message->content.setopt_request.so_val.type, 
                message->content.getopt_response.so_val.property.value);
        }
        else
        {
            result = er_not_supported;
            break;
        }
    }
    while (0);

    if (result != er_ok)
        log_error(server_sock->log, "Failed to handle set option message (%s).",
            prx_err_string(result));
    if (responder)
    {
        io_message_as_response(message);
        message->error_code = result;
        result = io_connection_send(responder, message);
        if (result != er_ok)
        {
            log_error(server_sock->log, "Failed sending set option response (%s).",
                prx_err_string(result));
            return result;
        }
    }
    return er_ok;
}

//
// handle get option message
//
static int32_t prx_server_socket_handle_getoptrequest(
    prx_server_socket_t* server_sock,
    io_connection_t* responder,
    io_message_t* message
)
{
    int32_t result;
    prx_socket_option_t so_opt;
    dbg_assert_ptr(message);
    dbg_assert_ptr(server_sock);
    dbg_assert_is_task(server_sock->scheduler);

    if (!responder)
        return er_ok;

    so_opt = message->content.getopt_request.so_opt;
    io_message_as_response(message);

    do
    {
        if (server_sock->state != prx_server_socket_created &&
            server_sock->state != prx_server_socket_opened)
        {
            result = er_closed;
            break;
        }

        server_sock->last_activity = ticks_get();

        /**/ if (so_opt == prx_so_ip_multicast_join ||
                 so_opt == prx_so_ip_multicast_leave)
        {
            result = er_not_supported;
            break;
        }
        else if (so_opt == prx_so_props_timeout)
        {
            message->content.getopt_response.so_val.property.value =
                server_sock->client_itf.props.timeout;
            result = er_ok;
        }
        else if (so_opt < __prx_so_max)
        {
            result = pal_socket_getsockopt(server_sock->sock, so_opt,
                &message->content.getopt_response.so_val.property.value);
            if (result != er_ok)
                break;

            log_trace(server_sock->log, "Read socket option %d as %ull...",
                so_opt, message->content.getopt_response.so_val.property.value);
        }
        else
        {
            log_error(server_sock->log, "Unsupported option type %d...",
                so_opt);
            result = er_not_supported;
            break;
        }

        message->content.getopt_response.so_val.type = so_opt;
    }
    while (0);

    if (result != er_ok)
        log_error(server_sock->log, "Failed to handle get option message! (%s).",
            prx_err_string(result));

    message->error_code = result;
    result = io_connection_send(responder, message);
    if (result != er_ok)
    {
        log_error(server_sock->log, "Failed sending get option response (%s).",
            prx_err_string(result));
        return result;
    }
    return er_ok;
}

//
// Handle new stream message
//
static int32_t prx_server_socket_stream_handler(
    void* context,
    io_connection_event_t ev,
    io_message_t* message,
    int32_t last_error
)
{
    prx_server_socket_t* server_sock = (prx_server_socket_t*)context;
    dbg_assert_is_task(server_sock->scheduler);
    dbg_assert_ptr(server_sock);

    /**/ if (ev == io_connection_received)
    {
        dbg_assert_ptr(message);
        /**/ if (message->type == io_message_type_close)
            return prx_server_socket_handle_closerequest(server_sock, NULL, message);
        else if (message->type == io_message_type_poll)
            return prx_server_socket_handle_pollrequest(server_sock, NULL, message);
        else if (message->type == io_message_type_data)
            return prx_server_socket_handle_datamessage(server_sock, NULL, message);
        else
        {
            log_error(server_sock->log, "Received unexpected message type %d.",
                message->type);
            return er_not_supported;
        }
    }
    else if (ev == io_connection_reconnecting)
    {
        /**/ if (last_error == er_ok)
        {
            if (server_sock->state == prx_server_socket_opened)
                return er_ok;
            last_error = er_closed;
        }
        else if (last_error != er_closed)
        {
            log_error(server_sock->log, "Stream connection with error (%s), closing...",
                prx_err_string(last_error));
        }

        if (last_error == er_closed || last_error == er_reset)
        {
            // Remote side closed, immediately cancel entire stream
            server_sock->state = prx_server_socket_created;
        }
        else
        {
            server_sock->state = prx_server_socket_collect;
        }
        __do_next(server_sock->server, prx_server_worker);
        return last_error;
    }
    else if (ev == io_connection_closed)
    {
        // Stream closed, now free socket and exit
        log_trace(server_sock->log, "Stream closed, schedule socket %p free", 
            server_sock);
        // Delay free to debounce hang up race condition
        __do_later(server_sock, prx_server_socket_free, 2000);
    }
    else
    {
        dbg_assert(0, "Unknown event %d.", ev);
        return er_not_supported;
    }
    return er_ok;
}

//
// Handle new control message
//
static int32_t prx_server_socket_control_handler(
    void* context,
    io_connection_event_t ev,
    io_message_t* message,
    int32_t last_error
)
{
    prx_server_socket_t* server_sock = (prx_server_socket_t*)context;
    io_connection_t* listener = server_sock->server->listener;

    dbg_assert_is_task(server_sock->scheduler);
    dbg_assert_ptr(server_sock);
    dbg_assert_ptr(listener);

    /**/ if (ev == io_connection_received)
    {
        dbg_assert_ptr(message);

        /**/ if (message->type == io_message_type_close)
            return prx_server_socket_handle_closerequest(server_sock, listener, message);
        else if (message->type == io_message_type_data)
            return prx_server_socket_handle_datamessage(server_sock, listener, message);
        else if (message->type == io_message_type_poll)
            return prx_server_socket_handle_pollrequest(server_sock, listener, message);
        else if (message->type == io_message_type_open)
            return prx_server_socket_handle_openrequest(server_sock, listener, message);
        else if (message->type == io_message_type_getopt)
            return prx_server_socket_handle_getoptrequest(server_sock, listener, message);
        else if (message->type == io_message_type_setopt)
            return prx_server_socket_handle_setoptrequest(server_sock, listener, message);
        else
        {
            log_error(server_sock->log, "Received bad message type %d.",
                message->type);
            return er_not_supported;
        }
    }
    else 
    {
        dbg_assert(!server_sock->server_stream, "Unexpected - server stream should handle.");
        return prx_server_socket_stream_handler(context, ev, message, last_error);
    }
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
        dbg_assert(buffer && size && *size == sizeof(pal_socket_t*) && 
            (pal_socket_t*)*buffer == sock->sock, "Socket expected.");
        sock->last_error = error;
        __do_next(sock, prx_server_socket_open_complete);
        break;
    case pal_socket_event_begin_recv:
        dbg_assert(error == er_ok, "no error expected.");
        prx_server_socket_on_begin_receive(sock, buffer, size, op_context);
        break;
    case pal_socket_event_end_recv:
        prx_server_socket_on_end_receive(sock, buffer, size, addr, flags, error, op_context);
        break;
    case pal_socket_event_begin_accept:
        dbg_assert(error == er_ok, "no error expected.");
        prx_server_socket_on_begin_accept(sock, buffer, size, op_context);
        break;
    case pal_socket_event_end_accept:
        prx_server_socket_on_end_accept(sock, buffer, size, error, op_context);
        break;
    case pal_socket_event_begin_send:
        dbg_assert(error == er_ok, "no error expected.");
        prx_server_socket_on_begin_send(sock, buffer, size, addr, flags, op_context);
        break;
    case pal_socket_event_end_send:
        prx_server_socket_on_end_send(sock, buffer, size, error, op_context);
        break;
    case pal_socket_event_closed:
        sock->last_error = error;
        __do_next(sock, prx_server_socket_close_complete);
        break;
    default:
        dbg_assert(0, "Should not be here!");
    }
}

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
        DList_InitializeListHead(&server_sock->read_queue);
        DList_InitializeListHead(&server_sock->write_queue);

        server_sock->state = prx_server_socket_created;
        server_sock->log = log_get("server_sk");

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
            pal_socket_close(next->sock);
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
    pal_socket_client_itf_t internal_itf;
    pal_socket_t* internal_sock;

    dbg_assert_ptr(message);
    dbg_assert_ptr(server);
    dbg_assert_is_task(server->scheduler);
    do
    {
        // Check if we need to block link requests for connection to restricted ports
        if (!(message->content.link_request.props.flags & prx_socket_flag_internal) &&
            !(message->content.link_request.props.flags & prx_socket_flag_passive))
        {
            result = prx_server_check_restricted_port(
                server, message->content.link_request.props.address.un.ip.port);
            if (result != er_ok)
                break;
        }

        // Create empty socket object
        result = prx_server_socket_create(server, &message->source_id, &server_sock);
        if (result != er_ok)
        {
            log_error(server->log, "Failed to make server socket object for link (%s)", 
                prx_err_string(result));
            break;
        }

        // Create socket handle
        memcpy(&server_sock->client_itf.props, &message->content.link_request.props,
            sizeof(server_sock->client_itf.props));
        if (!server_sock->client_itf.props.timeout)
            server_sock->client_itf.props.timeout = DEFAULT_GC_TIMEOUT;
        else if (server_sock->client_itf.props.timeout < MIN_GC_TIMEOUT)
            server_sock->client_itf.props.timeout = MIN_GC_TIMEOUT;

        //
        // Create socket. If this is an internal socket open a socket pair with 
        // requested server's client interface.  Socket pairs are already open.
        //
        if (!(message->content.link_request.props.flags & prx_socket_flag_internal))
        {
            result = pal_socket_create(&server_sock->client_itf, &server_sock->sock);
            if (result != er_ok)
            {
                log_error(server->log, "Failed to create client socket object (%s)",
                    prx_err_string(result));
                break;
            }

            // Now connect to external socket using given address
            result = pal_socket_open(server_sock->sock);
            if (result != er_ok)
            {
                log_error(server->log, "Failed to open client socket (%s)",
                    prx_err_string(result));
                break;
            }
        }
        else if (!message->content.link_request.props.address.un.proxy.host[0])
        {
            // Host must be empty, then pick the right internal server based on the 
            // provided port. Codec id is sent as part of flow member.
            switch (message->content.link_request.props.address.un.proxy.port)
            {
            case prx_internal_service_port_browse:
                result = prx_browse_server_accept(server->browser, (io_codec_id_t)
                    message->content.link_request.props.address.un.proxy.flags,
                    &internal_itf);
                break;
            case prx_internal_service_port_invalid:
            default:
                result = er_not_supported;
                break;
            }
            if (result != er_ok)
            {
                log_error(server->log, "Failed to accept internal server socket (%s)", 
                    prx_err_string(result));
                break;
            }

            result = pal_socket_pair(&server_sock->client_itf, &server_sock->sock,
                &internal_itf, &internal_sock);
            if (result != er_ok)
            {
                log_error(server->log, "Failed to create internal socket pair (%s)",
                    prx_err_string(result));
                dbg_assert(0, "Leaking client interface - pal should notify cb");
                break;
            }
            dbg_assert_ptr(internal_sock);
            internal_sock = NULL; 
            // Already opened
        }
        else
        {
            log_error(server->log, "Bad address provided for internal server link!");
            result = er_invalid_format;
            break;
        }

        // Save context for async completion
        message->context = server->listener;
        server_sock->link_message = message;
        server_sock->last_activity = ticks_get();

        return; // Now wait for our open callback to complete the connection
    } 
    while (0);

    io_message_as_response(message);
    message->error_code = result;

    if (server_sock)
        prx_server_socket_free(server_sock);

    result = io_connection_send(server->listener, message);
    if (result != er_ok)
    {
        log_error(server->log, "Failed sending link error response (%s).",
            prx_err_string(result));
        // Let client time out
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
    size_t prx_ai_count = 0;

    dbg_assert_ptr(message);
    dbg_assert_ptr(server);
    dbg_assert_is_task(server->scheduler);

    // Todo: ntop and then resolve.
    // Todo: pal_ping, with SendArp or alike...

    do
    {
        if (message->content.ping_request.address.un.family != prx_address_family_inet6 &&
            message->content.ping_request.address.un.family != prx_address_family_inet &&
            message->content.ping_request.address.un.family != prx_address_family_proxy)
        {
            log_error(server->log, "Ping request for address with invalid address family %d",
                message->content.ping_request.address.un.family);
            result = er_invalid_format;
            break;
        }

        result = prx_server_check_restricted_port(
            server, message->content.ping_request.address.un.ip.port);
        if (result != er_ok)
            break;
        result = string_from_int(message->content.ping_request.address.un.ip.port, 10, 
            port, sizeof(port));
        if (result != er_ok)
            break;

        if (message->content.ping_request.address.un.family == prx_address_family_proxy)
        {
            if (strlen(message->content.ping_request.address.un.proxy.host) == 0)
            {
                result = er_invalid_format;
                break;
            }

            result = pal_getaddrinfo(message->content.ping_request.address.un.proxy.host,
                port, prx_address_family_unspec, 0, &prx_ai, &prx_ai_count);
            if (result != er_ok)
            {
                log_error(server->log, "pal_getaddrinfo for %.128s:%s failed (%s).",
                    message->content.ping_request.address.un.proxy.host, port,
                    prx_err_string(result));
                break;
            }
        }
        else
        {
            result = pal_ntop(&message->content.ping_request.address, host_ip,
                sizeof(host_ip));
            if (result != er_ok)
                break;

            result = pal_getaddrinfo(host_ip, port,
                message->content.ping_request.address.un.family, 0, &prx_ai, &prx_ai_count);
            if (result != er_ok)
            {
                log_error(server->log, "pal_getaddrinfo for %.128s:%s failed (%s).",
                    host_ip, port, prx_err_string(result));
                break;
            }
        }
        result = er_ok;
    } 
    while (0);

    io_message_as_response(message);
    message->error_code = result;
    if (result == er_ok)
    {
        dbg_assert_ptr(prx_ai);
        dbg_assert(prx_ai_count > 0, "Unexpected");
        memcpy(&message->content.ping_response, &prx_ai->address, sizeof(prx_socket_address_t));
    }

    result = io_connection_send(server->listener, message);
    if (result != er_ok)
    {
        log_error(server->log, "Failed sending ping response (%s).",
            prx_err_string(result));
    }

    if (prx_ai)
        pal_freeaddrinfo(prx_ai);
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
    io_message_t* message,
    int32_t last_error
)
{
    prx_server_t* server = (prx_server_t*)context;
    prx_server_socket_t* server_sock;
    dbg_assert_ptr(server);
    dbg_assert_is_task(server->scheduler);

    (void)last_error;

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
                return prx_server_socket_control_handler(
                    server_sock, ev, message, last_error);
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

        else
        {
            log_error(server->log, "Received unsupported message type %d.", 
                message->type);
            return er_not_supported;
        }
    }
    else if (ev == io_connection_reconnecting)
    {
        log_trace(server->log, "Server connection is reconnecting (%s)", 
            prx_err_string(last_error));
    }
    else if (ev == io_connection_closed)
    {
        // Listener closed, free and exit
        io_connection_free(server->listener);
        server->listener = NULL;

        server->exit = true;
        __do_next(server, prx_server_worker);
    }
    else
    {
        dbg_assert(0, "Unknown event %d.", ev);
        return er_not_supported;
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

    chk_arg_fault_return(transport);
    chk_arg_fault_return(created);

    server = mem_zalloc_type(prx_server_t);
    if (!server)
        return er_out_of_memory;
    do
    {
        server->log = log_get("server");
        server->transport = transport;

        result = prx_ns_entry_get_addr(entry, &server->id);
        if (result != er_ok)
            break;

        result = string_parse_range_list(__prx_config_get(
            prx_config_key_restricted_ports, DEFAULT_RESTRICTED_PORTS),
            &server->restricted_ports, &server->restricted_port_count);
        if (result != er_ok)
            break;

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
        result = io_transport_create(server->transport, entry, io_codec_json,
            prx_server_handler, server, server->scheduler, 
            &server->listener);
        if (result != er_ok)
            break;

        result = prx_browse_server_create(scheduler, &server->browser);
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

