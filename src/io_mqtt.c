// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "io_mqtt.h"
#include "io_queue.h"
#include "xio_sk.h"
#include "pal.h"
#include "pal_time.h"
#include "util_string.h"

#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/buffer_.h"
#include "azure_c_shared_utility/wsio.h"
#include "azure_c_shared_utility/tlsio.h"
#include "azure_c_shared_utility/refcount.h"

#include "azure_umqtt_c/mqtt_codec.h"
#include "azure_umqtt_c/mqtt_client.h"

// #define LOG_MQTT

//
// Connection status
//
typedef enum io_mqtt_status
{
    io_mqtt_status_reset,                   // Connection is held in reset 
    io_mqtt_status_connecting,                 // Connection is connecting
    io_mqtt_status_connected,                   // Connection is connected
    io_mqtt_status_disconnecting,             // or gracefully diconnected
    io_mqtt_status_closing                   // or being gracefully closed
}
io_mqtt_status_t;

//
// Connection
//
struct io_mqtt_connection
{
    io_url_t* address;
    bool is_websocket;
    ticks_t expiry;    // When the credential used to authenticate expires

    prx_scheduler_t* scheduler;
    XIO_HANDLE socket_io;                                  // The io layer
    STRING_HANDLE client_id;
    MQTT_CLIENT_HANDLE client;
    uint16_t pkt_counter;
#define KEEP_ALIVE_INTERVAL 10 * 1000 
    uint32_t keep_alive_interval;
    bool disabled;

    DLIST_ENTRY send_queue;                       // Send queue (messages)
    DLIST_ENTRY subscriptions;             // Topics subscribed to on conn
    io_mqtt_status_t status;                          // connection status
    ticks_t last_success;                     // Last successful connected
    int32_t last_error;
    ticks_t last_activity;
    io_mqtt_connection_reconnect_t reconnect_cb; // Call when disconnected
    void* reconnect_ctx;
    uint32_t back_off_in_seconds;      // Delay until next connect attempt
    log_t log;
};

//
// Represents a message to send in send queue
//
typedef struct io_mqtt_message
{
    MQTT_MESSAGE_HANDLE msg_handle;
    uint16_t pkt_id;
    bool published;
    ticks_t attempted;
    size_t buf_len;
    io_mqtt_connection_t* connection;
    io_mqtt_publish_complete_t cb;
    void* context;
    DLIST_ENTRY qlink;
}
io_mqtt_message_t;

//
// Represents a subscription to a broker endpoint on a connection
//
struct io_mqtt_subscription
{
    STRING_HANDLE uri;       // Uri of the topic, not including properties

    bool subscribed;                 // Whether the subscription is active
    uint16_t pktid;                                 // To sub/unsub packet
                                          // State based on these members:
#define __subscription_subscribing(s)     ((s)->pktid && !(s)->subscribed)
#define __subscription_subscribed(s)     (!(s)->pktid &&  (s)->subscribed)
#define __subscription_unsubscribing(s)   ((s)->pktid &&  (s)->subscribed)
#define __subscription_unsubscribed(s)   (!(s)->pktid && !(s)->subscribed)

    io_mqtt_subscription_receiver_t receiver_cb;      // receiver callback
    void* receiver_ctx;                                    // with context

    bool disabled;                                // Flow control handling
    io_mqtt_connection_t* connection;                 // Owning connection
    DLIST_ENTRY link;
    log_t log;
};


//
// Compare 2 topics, minus properties etc. and returns remainder or NULL
//
static const char* io_mqtt_topic_matches(
    const char* topic,
    const char* match
)
{
    size_t i = 0;

    while(true)
    {
        if (!topic[i] && !match[i])
            return &topic[i];  // Complete match
        if (topic[i] != match[i])
            break;

        if (topic[i] == '/' && match[i + 1])
        {
            // match wildcard
            switch (match[i + 1])
            {
            case '#':
                // Skip all further levels.
                return &topic[i];
            case '+':
                // Skip a single level
                while (topic[++i] && topic[i] != '/')
                    ;
                if (!topic[i]) // Reached end, i.e. single level matched
                    return &topic[i];
                break;
            }
        }
        ++i;
    }
    // No match
    return NULL;
}

//
// Free message
//
static void io_mqtt_message_free(
    io_mqtt_message_t* message
)
{
    dbg_assert_ptr(message);

    if (message->msg_handle)
        mqttmessage_destroy(message->msg_handle);

    mem_free_type(io_mqtt_message_t, message);
}

//
// Clear back_off timer 
//
static void io_mqtt_connection_clear_failures(
    io_mqtt_connection_t* connection
)
{
    connection->last_success = connection->last_activity = ticks_get();
    connection->last_error = er_ok;
    connection->back_off_in_seconds = 0;
}

//
// Hard reset connection
//
static void io_mqtt_connection_hard_reset(
    io_mqtt_connection_t* connection
);

//
// Soft reset connection, performing a graceful disconnect
//
static void io_mqtt_connection_soft_reset(
    io_mqtt_connection_t* connection
);

// 
// Begin graceful disconnect
//
static void io_mqtt_connection_begin_disconnect(
    io_mqtt_connection_t* connection
);

// 
// Complete disconnect
//
static void io_mqtt_connection_complete_disconnect(
    io_mqtt_connection_t* connection
);

//
// Return a new id, skip 0
//
static uint16_t io_mqtt_connection_next_pkt_id(
    io_mqtt_connection_t* connection
)
{
    uint16_t id = ++connection->pkt_counter;
    if (id) return id;
    return ++connection->pkt_counter;
}

//
// Free the connection
//
void io_mqtt_connection_free(
    io_mqtt_connection_t* connection
)
{
    io_mqtt_subscription_t* next;
    dbg_assert_ptr(connection);

    io_mqtt_connection_complete_disconnect(connection);

    if (connection->client_id)
        STRING_delete(connection->client_id);

    if (connection->address)
        io_url_free(connection->address);

    dbg_assert(DList_IsListEmpty(&connection->send_queue), 
        "Leaking messages");

    dbg_assert(DList_IsListEmpty(&connection->subscriptions), 
        "Leaking subscriptions");
    while (!DList_IsListEmpty(&connection->subscriptions))
    {
        next = containingRecord(
            DList_RemoveHeadList(&connection->subscriptions),
            io_mqtt_subscription_t, link);
        next->connection = NULL;
        dbg_assert_ptr(connection->scheduler);
        prx_scheduler_clear(connection->scheduler, NULL, next);
    }

    if (connection->scheduler)
        prx_scheduler_release(connection->scheduler, connection);

    log_trace(connection->log, "Connection freed.");
    mem_free_type(io_mqtt_connection_t, connection);
}

//
// Subscribe all
//
static void io_mqtt_connection_subscribe_all(
    io_mqtt_connection_t* connection
)
{
    int32_t result;
    io_mqtt_subscription_t* next;
    SUBSCRIBE_PAYLOAD* payload;
    uint16_t pkt_id;
    size_t payload_count = 0;

    // Allocate subscription payload
    for (PDLIST_ENTRY p = connection->subscriptions.Flink;
        p != &connection->subscriptions; p = p->Flink)
    {
        next = containingRecord(p, io_mqtt_subscription_t, link);
        if (__subscription_unsubscribed(next) && !next->disabled)
            payload_count++;
    }
    if (!payload_count)
        return;
    payload = (SUBSCRIBE_PAYLOAD*)mem_zalloc(payload_count * sizeof(SUBSCRIBE_PAYLOAD));
    if (!payload)
    {
        connection->last_error = er_out_of_memory;
        __do_next(connection, io_mqtt_connection_hard_reset);
        return;
    }
    do
    {
        pkt_id = io_mqtt_connection_next_pkt_id(connection);
        payload_count = 0;
        for (PDLIST_ENTRY p = connection->subscriptions.Flink;
            p != &connection->subscriptions; p = p->Flink)
        {
            next = containingRecord(p, io_mqtt_subscription_t, link);
            if (__subscription_unsubscribed(next) && !next->disabled)
            {
                payload[payload_count].qosReturn = DELIVER_AT_MOST_ONCE;
                payload[payload_count].subscribeTopic = STRING_c_str(next->uri);
                payload_count++;
            }
        }

        result = mqtt_client_subscribe(connection->client, pkt_id, payload, 
            payload_count);
        if (result != 0)
        {
            connection->last_error = er_comm;
            __do_next(connection, io_mqtt_connection_hard_reset);
            break;
        }

        for (PDLIST_ENTRY p = connection->subscriptions.Flink;
            p != &connection->subscriptions; p = p->Flink)
        {
            next = containingRecord(p, io_mqtt_subscription_t, link);
            if (__subscription_unsubscribed(next))
            {
                next->pktid = pkt_id;
            }
        }
    } 
    while (0);
    mem_free(payload);
}

//
// Unsubscribe all
//
static void io_mqtt_connection_unsubscribe_all(
    io_mqtt_connection_t* connection
)
{
    int32_t result;
    io_mqtt_subscription_t* next;
    char** topics;
    uint16_t pkt_id;
    size_t topics_count = 0;

    // Allocate subscription payload
    for (PDLIST_ENTRY p = connection->subscriptions.Flink;
        p != &connection->subscriptions; p = p->Flink)
    {
        next = containingRecord(p, io_mqtt_subscription_t, link);
        if (__subscription_subscribed(next))
            topics_count++;
    }

    if (!topics_count)
    {
        if (connection->status == io_mqtt_status_disconnecting ||
            connection->status == io_mqtt_status_closing)
            __do_next(connection, io_mqtt_connection_begin_disconnect);
        return;
    }

    topics = (char**)mem_zalloc(topics_count * sizeof(const char*));
    if (!topics)
    {
        connection->last_error = er_out_of_memory;
        __do_next(connection, io_mqtt_connection_hard_reset);
        return;
    }
    do
    {
        pkt_id = io_mqtt_connection_next_pkt_id(connection);
        topics_count = 0;
        for (PDLIST_ENTRY p = connection->subscriptions.Flink;
            p != &connection->subscriptions; p = p->Flink)
        {
            next = containingRecord(p, io_mqtt_subscription_t, link);
            if (__subscription_subscribed(next))
            {
                topics[topics_count] = (char*)STRING_c_str(next->uri);
                topics_count++;
            }
        }
        result = mqtt_client_unsubscribe(connection->client, pkt_id, 
            (const char**)topics, topics_count);
        if (result != 0)
        {
            connection->last_error = er_out_of_memory;
            __do_next(connection, io_mqtt_connection_hard_reset);
            break;
        }

        for (PDLIST_ENTRY p = connection->subscriptions.Flink;
            p != &connection->subscriptions; p = p->Flink)
        {
            next = containingRecord(p, io_mqtt_subscription_t, link);
            if (__subscription_subscribed(next))
            {
                next->pktid = pkt_id;
            }
        }
    } while (0);

    mem_free(topics);
}

//
// Send a message to a topic
//
static void io_mqtt_connection_publish_message(
    io_mqtt_connection_t* connection
)
{
    io_mqtt_message_t* message, *next;

    if (connection->status != io_mqtt_status_connected)
        return;

    message = NULL;
    for (PDLIST_ENTRY p = connection->send_queue.Flink;
        p != &connection->send_queue; p = p->Flink)
    {
        next = containingRecord(p, io_mqtt_message_t, qlink);
        if (!next->published)
        {
            message = next;
            break;
        }
    }
    if (!message)
        return;

    message->published = true;
    message->attempted = ticks_get();
    if (0 != mqtt_client_publish(connection->client, message->msg_handle))
    {
        message->published = false;
        log_error(connection->log, "Failure publishing messages payload...");
        connection->last_error = er_writing;
        __do_next(connection, io_mqtt_connection_soft_reset);
    }
    // Wait for pub_ack
}

//
// Monitor connection is alive and reset with token
//
static void io_mqtt_connection_monitor(
    io_mqtt_connection_t* connection
)
{
    ticks_t now;
    BUFFER_HANDLE pingPacket;
    uint32_t time_to_wait = 0, time_diff;
    io_mqtt_message_t* next;

    now = ticks_get();

    prx_scheduler_clear(connection->scheduler, (prx_task_t)io_mqtt_connection_monitor,
        connection);

    // If connection has expired, reset connection
    if (connection->expiry && connection->expiry < now)
    {
        log_info(connection->log, "Need to refresh token, soft reset...");
        // Ensure we stay on the current transport
        if (!connection->address->scheme)
            connection->is_websocket = !connection->is_websocket;
        io_mqtt_connection_clear_failures(connection);
        __do_next(connection, io_mqtt_connection_soft_reset);
        return;
    }

    if (connection->disabled)
    {
        __do_later(connection, io_mqtt_connection_monitor, KEEP_ALIVE_INTERVAL);
        return;
    }

    // Check whether we timed out - if we did, reset
    time_diff = (uint32_t)(now - connection->last_activity);
    if (time_diff >= connection->keep_alive_interval)
    {
        // Check for missing pub ack
        for (PDLIST_ENTRY p = connection->send_queue.Flink;
            p != &connection->send_queue; p = p->Flink)
        {
            next = containingRecord(p, io_mqtt_message_t, qlink);
            if ((uint32_t)(now - next->attempted) > (2 * connection->keep_alive_interval))
            {
                log_error(connection->log, "missing pub ack, hard reset...",
                    time_diff);
                connection->last_error = er_writing;
                __do_next(connection, io_mqtt_connection_hard_reset);
                return;
            }
        }

        time_to_wait = connection->keep_alive_interval;

        if (time_diff >= (6 * connection->keep_alive_interval) ||
            connection->status == io_mqtt_status_connecting)
        {
            log_error(connection->log, "no activity for %d ms, hard reset...",
                time_diff);
            connection->last_error = er_timeout;
            __do_next(connection, io_mqtt_connection_hard_reset);
            return;
        }

        // Send ping packet
        pingPacket = mqtt_codec_ping();
        if (pingPacket != NULL)
        {
            xio_send(connection->socket_io, BUFFER_u_char(pingPacket),
                BUFFER_length(pingPacket), NULL, NULL);
            BUFFER_delete(pingPacket);
            time_diff = 0;
        }
    }
    else
    {
        // Otherwise wait at most keep alive time
        time_to_wait = connection->keep_alive_interval - time_diff;
    }

    if (connection->expiry)
    {
        time_diff = (uint32_t)(connection->expiry - now);
        if (time_to_wait > time_diff)
            time_to_wait = time_diff;
    }

    __do_later(connection, io_mqtt_connection_monitor, time_to_wait);
}

// 
// Message receive callback
//
static void io_mqtt_connection_receive_message(
    MQTT_MESSAGE_HANDLE msg_handle,
    void* context
)
{
    int32_t result;
    io_mqtt_connection_t* connection = (io_mqtt_connection_t*)context;
    io_mqtt_subscription_t* subscription = NULL;
    const APP_PAYLOAD* payload;
    STRING_HANDLE properties = NULL;
    const char* props = NULL, *uri;

    /**/ if (connection->status == io_mqtt_status_connecting)
        connection->status = io_mqtt_status_connected;
    else if (connection->status != io_mqtt_status_connected)
    {
        log_error(connection->log, "Receiving message while not connected. "
            "State %d.", connection->status);

        if (connection->status == io_mqtt_status_closing)
            return;
    }

    uri = mqttmessage_getTopicName(msg_handle);
    dbg_assert_ptr(uri);

    // Find subscription by subscription name on message
    result = er_not_found;
    for (PDLIST_ENTRY p = connection->subscriptions.Flink;
        p != &connection->subscriptions; p = p->Flink)
    {
        subscription = containingRecord(p, io_mqtt_subscription_t, link);
        if (!subscription->subscribed)
            continue;
        props = io_mqtt_topic_matches(uri, STRING_c_str(subscription->uri));
        if (props)
        {
            result = er_ok;
            break;
        }
    }

    if (result == er_not_found)
    {
        log_error(connection->log,
            "Received message for %s, but not subscribed to it.", uri);
        return;
    }

    dbg_assert_ptr(props);
    dbg_assert(subscription->subscribed, "Should be subscribed");

    // Check if receiver has released the subscription
    if (!subscription->receiver_cb)
        return;

    // Get payload and create properties
    properties = STRING_construct(props);
    payload = mqttmessage_getApplicationMsg(msg_handle);
    
    log_debug(subscription->log, "RECV [size: %08d]", payload->length);
    // Do callback
    subscription->receiver_cb(subscription->receiver_ctx,
        (io_mqtt_properties_t*)properties, payload->message, payload->length);
    STRING_delete(properties);
    io_mqtt_connection_clear_failures(subscription->connection);
}

//
// Handle publish ack
//
static int32_t io_mqtt_connection_handle_PUBLISH_ACK(
    io_mqtt_connection_t* connection,
    const PUBLISH_ACK* puback
)
{
    io_mqtt_message_t* message, *next;

    if (!puback)
        return er_arg;

    message = NULL;
    for (PDLIST_ENTRY p = connection->send_queue.Flink;
        p != &connection->send_queue; p = p->Flink)
    {
        next = containingRecord(p, io_mqtt_message_t, qlink);
        if (next->pkt_id == puback->packetId)
        {
            DList_RemoveEntryList(&next->qlink);
            message = next;
            break;
        }
    }
    if (!message)
    {
        log_error(connection->log,
            "Received publish ack for nonexistant message");
        return er_not_found; 
    }

    log_debug(connection->log, "SENT [size: %08d, took %d]",
        message->buf_len, ticks_get() - message->attempted);
    io_mqtt_connection_clear_failures(connection);
    
    if (message->cb)
        message->cb(message->context, er_ok);
    io_mqtt_message_free(message);

    // Publish more if possible
    if (connection->status == io_mqtt_status_connected)
        __do_next(connection, io_mqtt_connection_publish_message);
    return er_ok;
}

//
// Handle subscribe ack
//
static int32_t io_mqtt_connection_handle_SUBSCRIBE_ACK(
    io_mqtt_connection_t* connection,
    const SUBSCRIBE_ACK* suback
)
{
    io_mqtt_subscription_t* subscription;
    if (!suback)
        return er_arg;

    for (size_t index = 0; index < suback->qosCount; index++)
    {
        if (suback->qosReturn[index] == DELIVER_FAILURE)
        {
            log_error(connection->log, 
                "Subscribe delivery failure of subscribe %zu", index);
            return er_connecting;
        }
    }

    for (PDLIST_ENTRY p = connection->subscriptions.Flink;
        p != &connection->subscriptions; p = p->Flink)
    {
        subscription = containingRecord(p, io_mqtt_subscription_t, link);
        if (subscription->pktid == suback->packetId)
        {
            dbg_assert(!subscription->subscribed, "Should not be subscribed");
            subscription->subscribed = true;
            subscription->pktid = 0;
            log_trace(subscription->log, "Subscribed to %s!",
                STRING_c_str(subscription->uri));
        }
    }

    // Subscribe more...
    __do_next(connection, io_mqtt_connection_subscribe_all);
    return er_ok;
}

//
// Handle unsubscribe ack
//
static int32_t io_mqtt_connection_handle_UNSUBSCRIBE_ACK(
    io_mqtt_connection_t* connection,
    const UNSUBSCRIBE_ACK* unsuback
)
{
    io_mqtt_subscription_t* subscription;
    if (!unsuback)
        return er_arg;

    for (PDLIST_ENTRY p = connection->subscriptions.Flink;
        p != &connection->subscriptions; p = p->Flink)
    {
        subscription = containingRecord(p, io_mqtt_subscription_t, link);
        if (subscription->pktid == unsuback->packetId)
        {
            dbg_assert(subscription->subscribed, "Should be subscribed");
            subscription->subscribed = false;
            subscription->pktid = 0;
            log_trace(subscription->log, "Unsubscribed from %s!",
                STRING_c_str(subscription->uri));
        }
    }

    if (connection->status == io_mqtt_status_disconnecting)
        __do_next(connection, io_mqtt_connection_unsubscribe_all);
    return er_ok;
}

//
// Handle connect ack
//
static int32_t io_mqtt_connection_handle_CONNECT_ACK(
    io_mqtt_connection_t* connection,
    const CONNECT_ACK* connack
)
{
    if (connack == NULL)
        return er_arg;

    switch (connack->returnCode)
    {
    case CONNECTION_ACCEPTED:
        // Connection established, complete connect
        log_info(connection->log, "Connection established (%s:%d)!", 
            STRING_c_str(connection->address->host_name), 
            connection->address->port ? connection->address->port : 
                (connection->is_websocket ? 443 : 8883));
        connection->status = io_mqtt_status_connected;
        __do_next(connection, io_mqtt_connection_subscribe_all);
        __do_next(connection, io_mqtt_connection_publish_message);
        __do_next(connection, io_mqtt_connection_monitor);
        return er_ok;
    case CONN_REFUSED_UNACCEPTABLE_VERSION:
        log_error(connection->log, "Connection refused, unacceptable version.");
        break;
    case CONN_REFUSED_ID_REJECTED:
        log_error(connection->log, "Connection refused, id was rejected.");
        break;
    case CONN_REFUSED_SERVER_UNAVAIL:
        log_error(connection->log, "Connection refused, server unavailable.");
        break;
    case CONN_REFUSED_BAD_USERNAME_PASSWORD:
        log_error(connection->log, "Connection refused, bad username or password.");
        break;
    case CONN_REFUSED_NOT_AUTHORIZED:
        log_error(connection->log, "Connection refused, unauthorized.");
        break;
    case CONN_REFUSED_UNKNOWN:
        log_error(connection->log, "Connection refused, reason unknown.");
        break;
    }
    return er_refused;
}

//
// Operation callback, complete the payload and pump more messages
//
static void io_mqtt_connection_operation_callback(
    MQTT_CLIENT_HANDLE handle,
    MQTT_CLIENT_EVENT_RESULT operation_result,
    const void* message_info,
    void* context
)
{
    int32_t result;
    io_mqtt_connection_t* connection = (io_mqtt_connection_t*)context;
    (void)handle;

    switch (operation_result)
    {
    case MQTT_CLIENT_ON_CONNACK:
        result = io_mqtt_connection_handle_CONNECT_ACK(
            connection, (const CONNECT_ACK*)message_info);
        break;

    case MQTT_CLIENT_ON_PUBLISH_COMP:
    case MQTT_CLIENT_ON_PUBLISH_ACK:
        result = io_mqtt_connection_handle_PUBLISH_ACK(
            connection, (const PUBLISH_ACK*)message_info);
        break;

    case MQTT_CLIENT_ON_PUBLISH_RECV:
    case MQTT_CLIENT_ON_PUBLISH_REL:
        result = er_ok;
        return;

    case MQTT_CLIENT_ON_SUBSCRIBE_ACK:
        result = io_mqtt_connection_handle_SUBSCRIBE_ACK(
            connection, (const SUBSCRIBE_ACK*)message_info);
        break;
    case MQTT_CLIENT_ON_UNSUBSCRIBE_ACK:
        result = io_mqtt_connection_handle_UNSUBSCRIBE_ACK(
            connection, (const UNSUBSCRIBE_ACK*)message_info);
        break;

    case MQTT_CLIENT_ON_DISCONNECT:
        // Connection disconnected, complete 
        result = er_ok;
        if (connection->status == io_mqtt_status_closing)
        {
            // Closing, so free connection next...
            __do_next(connection, io_mqtt_connection_free);
            break;
        }

        // otherwise, hard reset
        connection->last_error = er_closed;
        __do_next(connection, io_mqtt_connection_hard_reset);
        if (connection->status == io_mqtt_status_connecting ||
            connection->status == io_mqtt_status_connected)
        {
            log_info(connection->log, "Remote side disconnected.");
        }
        return;
    case MQTT_CLIENT_ON_PING_RESPONSE:
        log_info(connection->log, "Connection alive...");
        result = er_ok;
        break;
    default:
        dbg_assert(0, "Unknown operation action %x", operation_result);
        result = er_fatal;
        break;
    }

    if (result == er_ok)
    {
        connection->last_activity = ticks_get();
        return;
    }

    log_error(connection->log,
        "Connection %p in error status, reset connection...", connection);
    connection->last_error = result;
    __do_next(connection, io_mqtt_connection_hard_reset);
}

//
// Error callback
//
static void io_mqtt_connection_error_callback(
    MQTT_CLIENT_HANDLE handle,
    MQTT_CLIENT_EVENT_ERROR error,
    void* context
)
{
    io_mqtt_connection_t* connection = (io_mqtt_connection_t*)context;
    (void)handle;
    switch (error)
    {
    case MQTT_CLIENT_CONNECTION_ERROR:
        connection->last_error = er_connecting;
        log_error(connection->log, "Connection error, reset connection...");
        break;
    case MQTT_CLIENT_PARSE_ERROR:
        connection->last_error = er_invalid_format;
        log_error(connection->log, "Parser error, reset connection...");
        break;
    case MQTT_CLIENT_MEMORY_ERROR:
        connection->last_error = er_out_of_memory;
        log_error(connection->log, "Memory error, reset connection...");
        break;
    case MQTT_CLIENT_COMMUNICATION_ERROR:
        connection->last_error = er_comm;
        log_error(connection->log, "Communication error, reset connection...");
        break;
    case MQTT_CLIENT_NO_PING_RESPONSE:
        connection->last_error = er_timeout;
        log_error(connection->log, "No ping response, reset connection...");
        break;
    case MQTT_CLIENT_UNKNOWN_ERROR:
    default:
        connection->last_error = er_unknown;
        log_error(connection->log, "Unknown error, reset connection...");
        break;
    }
    __do_next(connection, io_mqtt_connection_hard_reset);
}

// 
// Begin graceful disconnect
//
static void io_mqtt_connection_begin_disconnect(
    io_mqtt_connection_t* connection
)
{
    io_mqtt_subscription_t* next;

    for (PDLIST_ENTRY p = connection->subscriptions.Flink;
        p != &connection->subscriptions; p = p->Flink)
    {
        next = containingRecord(p, io_mqtt_subscription_t, link);
        if (__subscription_subscribed(next))
        {
            // Unsubscribe all subscriptions first...
            __do_next(connection, io_mqtt_connection_unsubscribe_all);
            return;
        }
    }

    dbg_assert_ptr(connection->client);
    // Disconnect connection, sends a disconnect packet...
    mqtt_client_disconnect(connection->client);
    // Wait for disconnect to complete
}

// 
// Complete disconnect, amounts to a hard disconnect/reset 
//
static void io_mqtt_connection_complete_disconnect(
    io_mqtt_connection_t* connection
)
{
    io_mqtt_message_t* message;
    io_mqtt_subscription_t* subscription;
    dbg_assert_ptr(connection);

    if (connection->socket_io)
    {
        xio_close(connection->socket_io, NULL, NULL);
        xio_destroy(connection->socket_io);
        connection->socket_io = NULL;

        log_trace(connection->log, "Socket disconnected.");
    }

    if (connection->client)
    {
        mqtt_client_deinit(connection->client);
        connection->client = NULL;

        log_trace(connection->log, "Client disconnected.");
    }

    // Reset send queue
    for (PDLIST_ENTRY p = connection->send_queue.Flink;
        p != &connection->send_queue; p = p->Flink)
    {
        message = containingRecord(p, io_mqtt_message_t, qlink);
        message->attempted = 0;
        message->published = false;
    }

    // Clear all subscriptions
    for (PDLIST_ENTRY p = connection->subscriptions.Flink;
        p != &connection->subscriptions; p = p->Flink)
    {
        subscription = containingRecord(p, io_mqtt_subscription_t, link);
        subscription->pktid = 0;
        subscription->subscribed = false;
        subscription->disabled = false;
    }

    // Clear all connection tasks...
    prx_scheduler_clear(connection->scheduler, NULL, connection);
}

//
// Returns host trusted certs to validate certs against
//
extern const char* trusted_certs(
    void
);

//
// Connect all unconnected layers
//
static void io_mqtt_connection_reconnect(
    io_mqtt_connection_t* connection
)
{
    int32_t result;
    TLSIO_CONFIG tls_io_config;
    WSIO_CONFIG ws_io_config;
    MQTT_CLIENT_OPTIONS options;
    STRING_HANDLE token = NULL;
    int64_t ttl = 0;

    dbg_assert_ptr(connection);
    do
    {
        result = er_connecting;

        if (!connection->client)
        {
            connection->client = mqtt_client_init(io_mqtt_connection_receive_message,
                io_mqtt_connection_operation_callback, connection, 
                io_mqtt_connection_error_callback, connection);
            if (!connection->client)
            {
                log_error(connection->log, "Failed to create mqtt client handle!");
                break;
            }
        }

#ifdef LOG_MQTT
        mqtt_client_set_trace(connection->client, true, false);
#endif
        if (!connection->socket_io)
        {
            // Either select explicit io or cycle the use of websockets and raw io
            if (!connection->address->scheme)
                connection->is_websocket = !connection->is_websocket;

            if (connection->is_websocket && (pal_caps() & pal_cap_wsclient))
            {
                memset(&ws_io_config, 0, sizeof(WSIO_CONFIG));
                ws_io_config.hostname = 
                    STRING_c_str(connection->address->host_name);
                ws_io_config.port =
                    connection->address->port ? connection->address->port : 443;
                ws_io_config.protocol =
                    "MQTT";
                ws_io_config.resource_name = 
                    STRING_c_str(connection->address->path);

                connection->socket_io = xio_create(
                    wsio_get_interface_description(), &ws_io_config);
                if (!connection->socket_io)
                    break;
            }
            else
            {
                memset(&tls_io_config, 0, sizeof(tls_io_config));
                tls_io_config.port = 
                    connection->address->port ? connection->address->port : 8883;
                tls_io_config.hostname = STRING_c_str(connection->address->host_name);

                connection->socket_io = xio_create(
                    platform_get_default_tlsio(), &tls_io_config);
                if (!connection->socket_io)
                    break;

                // OpenSSL tls needs the trusted certs
                (void)xio_setoption(connection->socket_io, "TrustedCerts", trusted_certs());
            }

            // Set scheduler after the fact - this will push receives back to us
            if (0 != xio_setoption(connection->socket_io, xio_opt_scheduler, connection->scheduler))
                break;

            //
            // Establish connection, set user name and password
            //
            memset(&options, 0, sizeof(options));
            options.qualityOfServiceValue = DELIVER_AT_LEAST_ONCE;
            options.clientId = (char*)STRING_c_str(connection->client_id);
            options.keepAliveInterval = 60 * 4; 

            if (connection->address->token_provider)
            {
                result = io_token_provider_new_token(connection->address->token_provider,
                    &token, &ttl);
                if (result != er_ok)
                {
                    log_error(connection->log, "Failed to make token for connection (%s)!",
                        prx_err_string(result));
                    break;
                }
                connection->expiry = ticks_get() + ttl;

                options.username = (char*)io_token_provider_get_property(
                    connection->address->token_provider, io_token_property_policy);
                options.password = (char*)STRING_c_str(token);
            }

            if (connection->address->user_name)
                options.username = (char*)STRING_c_str(connection->address->user_name);
            if (connection->address->password)
                options.password = (char*)STRING_c_str(connection->address->password);

            result = mqtt_client_connect(connection->client, connection->socket_io, &options);
            if (token)
                STRING_delete(token);
            if (result != 0)
            {
                log_error(connection->log,
                    "Failed to connect connection during reconnecting!");
                break;
            }
        }

        connection->last_activity = ticks_get(); 
        connection->status = io_mqtt_status_connecting;
        connection->disabled = false;
        // Kick off activity monitor to montior connect
        __do_later(connection, io_mqtt_connection_monitor, 30000); 
        return;

    } while (0);

    log_error(connection->log,
        "Failed to connect connection (%s)", prx_err_string(result));
    connection->last_error = result;
    __do_next(connection, io_mqtt_connection_hard_reset);
}

//
// Reset connection, by scheduling a disconnect and reconnect
//
static void io_mqtt_connection_hard_reset(
    io_mqtt_connection_t* connection
)
{
    dbg_assert_ptr(connection);

    if (connection->status == io_mqtt_status_closing)
    {
        // Completed a full disconnect, now free connection
        __do_next(connection, io_mqtt_connection_free);
        return;
    }

    // First hard disconnect, release all resources
    connection->status = io_mqtt_status_reset;
    io_mqtt_connection_complete_disconnect(connection);

    if (!connection->reconnect_cb)
        return;
    
    if (!connection->reconnect_cb(connection->reconnect_ctx, 
        connection->last_error, &connection->back_off_in_seconds))
        return;

    if (connection->back_off_in_seconds > 0)
    {
        log_info(connection->log, "Reconnecting in %d seconds...",
            connection->back_off_in_seconds);
        __do_later(connection, io_mqtt_connection_reconnect,
            connection->back_off_in_seconds * 1000);
    }
    else
    {
        log_info(connection->log, "Reconnecting...");
        __do_next(connection, io_mqtt_connection_reconnect);
    }

    if (!connection->back_off_in_seconds)
        connection->back_off_in_seconds = 1;
    connection->back_off_in_seconds *= 2;
    if (connection->back_off_in_seconds > 24 * 60 * 60)
        connection->back_off_in_seconds = 24 * 60 * 60;
}

//
// Soft reset connection, performing a graceful disconnect first
//
static void io_mqtt_connection_soft_reset(
    io_mqtt_connection_t* connection
)
{
    connection->status = io_mqtt_status_disconnecting;
    __do_next(connection, io_mqtt_connection_begin_disconnect);
    // This will eventually lead to a hard reset, after graceful disconnect
}

//
// Free the subscription
//
static void io_mqtt_subscription_free(
    io_mqtt_subscription_t* subscription
)
{
    SUBSCRIBE_PAYLOAD payload;

    log_trace(subscription->log, "Free subscription for topic %s...",
        STRING_c_str(subscription->uri));

    if (subscription->connection)
    {
        if (__subscription_subscribed(subscription))
        {
            //
            // Unsubscribe, but no matter whether we failed, delete the 
            // subscription, and also do not wait for any ack.
            //
            payload.subscribeTopic = STRING_c_str(subscription->uri);
            subscription->pktid = io_mqtt_connection_next_pkt_id(
                subscription->connection);
            (void)mqtt_client_unsubscribe(subscription->connection->client,
                subscription->pktid, &payload.subscribeTopic, 1);
        }

        prx_scheduler_clear(subscription->connection->scheduler, 
            NULL, subscription);
        DList_RemoveEntryList(&subscription->link);
    }

    if (subscription->uri)
        STRING_delete(subscription->uri);

    mem_free_type(io_mqtt_subscription_t, subscription);
}

//
// (Re-)enable the subscription
//
static void io_mqtt_subscription_enable(
    io_mqtt_subscription_t* subscription
)
{
    dbg_assert_ptr(subscription->connection);
    dbg_assert_is_task(subscription->connection->scheduler);

    if (!subscription->disabled)
        return;
    if (subscription->connection->status != io_mqtt_status_connected)
        return;

    subscription->disabled = false;

    if (!__subscription_unsubscribing(subscription) &&
        !__subscription_unsubscribed(subscription))
        return;

    __do_next_s(subscription->connection->scheduler,
        io_mqtt_connection_subscribe_all, subscription->connection);
}

//
// disable the subscription
//
static void io_mqtt_subscription_disable(
    io_mqtt_subscription_t* subscription
)
{
    int32_t result;
    SUBSCRIBE_PAYLOAD payload;

    dbg_assert_ptr(subscription->connection);
    dbg_assert_is_task(subscription->connection->scheduler);

    if (subscription->disabled)
        return;
    if (subscription->connection->status != io_mqtt_status_connected)
        return;

    subscription->disabled = true;

    if (!__subscription_subscribed(subscription) &&
        !__subscription_subscribing(subscription))
        return;

    payload.subscribeTopic = STRING_c_str(subscription->uri);

    //
    // If in progress of being subscribed, but waiting for ack, declare 
    // the subscription subscribed so we can unsubscribe it.
    //
    subscription->subscribed = true;
    subscription->pktid = io_mqtt_connection_next_pkt_id(
        subscription->connection);

    result = mqtt_client_unsubscribe(subscription->connection->client,
        subscription->pktid, &payload.subscribeTopic, 1);
}

//
// Enable / Disable receive flow
//
int32_t io_mqtt_subscription_receive(
    io_mqtt_subscription_t* subscription,
    bool flow_on_off
)
{
    chk_arg_fault_return(subscription);

    //
    // Cannot flow at tcp layer or else we cause issues with keep alive
    // handling. So we disable or enable subscription.  That will however
    // be seen by client as a disconnect, thus messages will come back
    // with error and need to be retried.
    //
    // TODO:
    // Better might be to ack published packet ids only when handled,
    // but that would require a change to the mqtt_publish function since
    // packets get processed asynchronously. 
    //
    if (!subscription->connection ||
        subscription->connection->status != io_mqtt_status_connected)
        return er_closed;

    if (flow_on_off)
        __do_next_s(subscription->connection->scheduler,
            io_mqtt_subscription_enable, subscription);
    else
        __do_next_s(subscription->connection->scheduler,
            io_mqtt_subscription_disable, subscription);

    return er_ok;
}

//
// Free the subscription
//
void io_mqtt_subscription_release(
    io_mqtt_subscription_t* subscription
)
{
    if (!subscription)
        return;

    dbg_assert_ptr(subscription->connection);
    dbg_assert_ptr(subscription->connection->scheduler);

    subscription->receiver_cb = NULL;
    subscription->receiver_ctx = NULL;

    __do_next_s(subscription->connection->scheduler,
        io_mqtt_subscription_free, subscription);
}

//
// Create properties bag, which is really a string handle
//
int32_t io_mqtt_properties_create(
    io_mqtt_properties_t** created
)
{
    STRING_HANDLE properties;

    properties = STRING_new();
    if (!properties)
        return er_out_of_memory;
    *created = (io_mqtt_properties_t*)properties;
    return er_ok;
}

//
// Free properties bag, i.e. the string handle
//
void io_mqtt_properties_free(
    io_mqtt_properties_t* properties
)
{
    dbg_assert_ptr(properties);
    STRING_delete((STRING_HANDLE)properties);
}

//
// Add property
//
int32_t io_mqtt_properties_add(
    io_mqtt_properties_t* properties,
    const char* key,
    const char* value
)
{
    STRING_HANDLE prop_string;

    chk_arg_fault_return(properties);
    chk_arg_fault_return(key);
    chk_arg_fault_return(value);
    if (0 == strlen(value))
        return er_arg;

    prop_string = (STRING_HANDLE)properties;
    if (0 != STRING_concat(prop_string, "&") ||
        0 != STRING_concat(prop_string, key) ||
        0 != STRING_concat(prop_string, "=") ||
        0 != STRING_concat(prop_string, value))
    {
        return er_out_of_memory;
    }
    return er_ok;
}

//
// Get property
//
int32_t io_mqtt_properties_get(
    io_mqtt_properties_t* properties,
    const char* key,
    char* value_buf,
    size_t value_len
)
{
    size_t found_len = 0;
    const char* found;

    chk_arg_fault_return(properties);
    chk_arg_fault_return(key);
    chk_arg_fault_return(value_buf);
    chk_arg_fault_return(value_len);

    found = string_find_nocase(
        STRING_c_str((STRING_HANDLE)properties), key);
    if (!found)
        return er_not_found;
    found += strlen(key);
    if (*found != '=')
        return er_not_found;
    found++;
    while (found[found_len] && found[found_len] != '&')
        ++found_len;

    if (value_len <= found_len)
        return er_arg;

    memcpy(value_buf, found, found_len);
    value_buf[found_len] = 0;
    return er_ok;
}

//
// Publish data
//
int32_t io_mqtt_connection_publish(
    io_mqtt_connection_t* connection,
    const char* uri,
    io_mqtt_properties_t* properties,
    io_mqtt_qos_t qos,
    const uint8_t* buffer,
    size_t buf_len,
    io_mqtt_publish_complete_t cb,
    void* context
)
{
    int32_t result;
    io_mqtt_message_t* message;
    char* prop_string;
    STRING_HANDLE topic_name = NULL;
    QOS_VALUE qos_val;

    chk_arg_fault_return(connection);
    chk_arg_fault_return(uri);
    chk_arg_fault_return(buffer);

    message = mem_zalloc_type(io_mqtt_message_t);
    if (!message)
        return er_out_of_memory;
    do
    {
        message->buf_len = buf_len;
        message->pkt_id = io_mqtt_connection_next_pkt_id(connection);

        topic_name = STRING_construct(uri);
        if (!topic_name)
        {
            log_error(connection->log, "Creating message failed due to out of memory");
            result = er_out_of_memory;
            break;
        }

        if (properties)
        {
            // Add properties
            prop_string = (char*)STRING_c_str((STRING_HANDLE)properties);
            if (*prop_string == '&')
                *prop_string = '?';
            STRING_concat(topic_name, prop_string);
        }

        switch (qos)
        {
        case io_mqtt_qos_at_least_once:
            qos_val = DELIVER_AT_LEAST_ONCE;
            break;
        case io_mqtt_qos_exactly_once:
            qos_val = DELIVER_EXACTLY_ONCE;
            break;
        case io_mqtt_qos_at_most_once:
        default:
            qos_val = DELIVER_AT_MOST_ONCE;
            break;
        }

        message->msg_handle = mqttmessage_create(message->pkt_id,
            STRING_c_str(topic_name), qos_val, (const uint8_t*)buffer, buf_len);
        if (!message->msg_handle)
        {
            log_error(connection->log, "Could not create mqtt message from buffer");
            result = er_out_of_memory;
            break;
        }

        message->connection = connection;

        // if (DList_IsListEmpty(&connection->send_queue))
        {
            // Kick off publishing
            __do_next(connection, io_mqtt_connection_publish_message);
        }
        DList_InsertTailList(&connection->send_queue, &message->qlink);
        STRING_delete(topic_name);
        
        message->cb = cb;
        message->context = context;
        return er_ok;
    } 
    while (0);

    if (topic_name)
        STRING_delete(topic_name);
    if (cb) // Always call back to allow caller to free context
        cb(context, result);
    io_mqtt_message_free(message);
    return result;
}

//
// Create new subscription
//
int32_t io_mqtt_connection_subscribe(
    io_mqtt_connection_t* connection,
    const char* uri,
    io_mqtt_subscription_receiver_t cb,
    void* ctx,
    io_mqtt_subscription_t** created
)
{
    int32_t result;
    io_mqtt_subscription_t* subscription;

    chk_arg_fault_return(connection);
    chk_arg_fault_return(uri);
    chk_arg_fault_return(created);

    subscription = mem_zalloc_type(io_mqtt_subscription_t);
    if (!subscription)
        return er_out_of_memory;
    do
    {
        DList_InitializeListHead(&subscription->link);
        subscription->log = log_get("io_mqtt_sub");

        subscription->receiver_cb = cb;
        subscription->receiver_ctx = ctx;
        subscription->disabled = false;
        subscription->uri = STRING_construct(uri);
        if (!subscription->uri)
        {
            result = er_out_of_memory;
            break;
        }

        DList_InsertTailList(&connection->subscriptions, &subscription->link);
        subscription->connection = connection;

        if (connection->status == io_mqtt_status_connected)
            __do_next(connection, io_mqtt_connection_subscribe_all);

        dbg_assert(__subscription_unsubscribed(subscription), "Unexpected");
        *created = subscription;
        return er_ok;
    } while (0);

    io_mqtt_subscription_free(subscription);
    return result;
}

//
// Allocate a connection resource
//
int32_t io_mqtt_connection_create(
    io_url_t* address,
    const char* client_id,
    prx_scheduler_t* scheduler,
    io_mqtt_connection_t** created
)
{
    int32_t result;
    io_mqtt_connection_t* connection;

    chk_arg_fault_return(address);
    chk_arg_fault_return(created);

    connection = mem_zalloc_type(io_mqtt_connection_t);
    if (!connection)
        return er_out_of_memory;
    do
    {
        DList_InitializeListHead(&connection->subscriptions);
        DList_InitializeListHead(&connection->send_queue);

        connection->log = log_get("io_mqtt");
        connection->status = io_mqtt_status_reset;
        connection->keep_alive_interval = KEEP_ALIVE_INTERVAL;
        connection->disabled = false;

        if (client_id)
            connection->client_id = STRING_construct(client_id);
        else
            connection->client_id = STRING_construct_random(8);
        if (!connection->client_id)
        {
            result = er_out_of_memory;
            break;
        }

        result = io_url_clone(address, &connection->address);
        if (result != er_ok)
            break;

        //
        // If no scheme or wss specified, start out as websocket being true, else 
        // always use raw socket io for mqtts
        //
        if (!connection->address->scheme ||
            0 == STRING_compare_c_str_nocase(connection->address->scheme, "wss"))
            connection->is_websocket = true;

        result = prx_scheduler_create(scheduler, &connection->scheduler);
        if (result != er_ok)
            break;

        *created = connection;
        return result;

    } while (0);

    io_mqtt_connection_close(connection);
    return result;
}

//
// Close the connection
//
void io_mqtt_connection_close(
    io_mqtt_connection_t* connection
)
{
    io_mqtt_status_t status;
    io_mqtt_message_t* message;
    if (!connection)
        return;

    status = connection->status;

    log_trace(connection->log, "Closing connection ...");
    connection->status = io_mqtt_status_closing;
    connection->reconnect_cb = NULL;

    while (!DList_IsListEmpty(&connection->send_queue))
    {
        message = containingRecord(DList_RemoveHeadList(
            &connection->send_queue), io_mqtt_message_t, qlink);
        if (message->cb)
            message->cb(message->context, er_aborted);
        io_mqtt_message_free(message);
    }

    if (status != io_mqtt_status_reset)
    {
        log_trace(connection->log, "Begin disconnect and schedule free ...");

        __do_next(connection, io_mqtt_connection_begin_disconnect);
        // If we fail after 30 seconds, free the connection 
        __do_later(connection, io_mqtt_connection_free, 30000);
    }
    else
    {
        // Already in reset state, free...
        __do_next(connection, io_mqtt_connection_free);
    }
}

//
// Connect connection
//
int32_t io_mqtt_connection_connect(
    io_mqtt_connection_t* connection,
    io_mqtt_connection_reconnect_t reconnect_cb,
    void* reconnect_ctx
)
{
    chk_arg_fault_return(connection);

    connection->reconnect_cb = reconnect_cb;
    connection->reconnect_ctx = reconnect_ctx;

    __do_next(connection, io_mqtt_connection_reconnect);
    return er_ok;
}
