// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os.h"
#include "util_mem.h"
#include "pal_ws.h"
#include "pal_mt.h"
#include "util_string.h"
#include "prx_config.h"

#include "azure_c_shared_utility/doublylinkedlist.h"
#include "azure_c_shared_utility/threadapi.h"

#if !defined(UNIT_TEST)
#include "libwebsockets.h"
#include "openssl/ssl.h"
#endif

// Must correspond to libwebsockets max_fds value
#define MAX_WEBSOCKET_PER_WORKER (getdtablesize() - 1)

// #define LOG_VERBOSE

// 
// State flags
// 
typedef enum pal_wsclient_flag
{
    pal_wsclient_connecting_bit,
    pal_wsclient_connected_bit,
    pal_wsclient_disconnecting_bit,
    pal_wsclient_closing_bit
}
pal_wsclient_flag_t;

//
// Keeps a list of workers as a pool to circumvent max fd per worker
//
typedef struct pal_wsworker_pool
{
    atomic_t counter;
    lock_t pool_lock;                 // Lock protecting worker pool
    DLIST_ENTRY workers;                          // List of workers
    log_t log;
}
pal_wsworker_pool_t;

//
// Represents a websocket worker handler.  
//
typedef struct pal_wsworker
{
    int id;                                             // Worker id
    char* proxy_address;                            // Proxy address 
    uint16_t proxy_port;    // + port are per context not connection
    struct lws_protocols* protocols;    // The worker object for lws
    struct lws_context* context;       // set of pollfd's to service
    THREAD_HANDLE service_thread;     // Services the context object
    bool kill;                 // Whether the pool killed the worker
    pal_wsworker_pool_t* pool;             // Websocket pool pointer
    DLIST_ENTRY link;                // Link into the websocket pool
    DLIST_ENTRY clients;         // Websocket client objects managed
    lock_t clients_lock;                  // Protect list of clients
    log_t log;
}
pal_wsworker_t;

//
// Web socket client instance
//
struct pal_wsclient
{
    char* host;                    // Info to connect to host server
    uint16_t port;
    char* relative_path;
    char* protocol_name;
    bool secure;
    STRING_HANDLE headers;

    atomic_t state;                           // State of the client
    uint8_t* send_buffer;  // to do async submits with worker insitu
    size_t send_buffer_size;                      // Bytes to submit
    size_t send_buffer_offset;                 // For partial writes 
    int send_flags;                         // Flags to use for send
    bool can_send;                     // Whether sending is enabled
    uint8_t* recv_buffer;                  // Any left over receives
    size_t recv_buffer_size;               // Left over receive size
    int recv_flags;                        // last flags for receive
    bool can_recv;                   // Whether receiving is enabled

    pal_wsworker_t* worker;               // Websocket worker thread
    struct lws* wsi;                       // lws websocket instance
    int disconnect_reason;       // Reason to send during disconnect
    int32_t last_error;
    DLIST_ENTRY link;              // Link into the websocket worker
    pal_wsclient_event_handler_t cb;               // Event callback
    void* context;                               // and user context
    log_t log;
};

static pal_wsworker_pool_t* global_wsworker_pool;

// 
// Logger hook into lws
//
static void pal_wsclient_lws_log(
    int level,
    const char* msg
)
{
    dbg_assert_ptr(global_wsworker_pool);

#if !defined(UNIT_TEST)
    /**/ if (level & LLL_ERR)
    {
        log_error(global_wsworker_pool->log, "%s", msg);
    }
    else if (level & LLL_WARN)
    {
        log_info(global_wsworker_pool->log, " -- WARNING -- %s", msg);
    }
    else if (level & LLL_NOTICE)
    {
        log_info(global_wsworker_pool->log, "%s", msg);
    }
    else if (level & LLL_INFO)
    {
        log_debug(global_wsworker_pool->log, "%s", msg);
    }
    else if (level & LLL_CLIENT)
    {
        log_debug(global_wsworker_pool->log, "CLIENT: %s", msg);
    }
#ifdef LOG_VERBOSE
    else if (level & LLL_DEBUG)
    {
        log_debug(global_wsworker_pool->log, "%s", msg);
    }
    else if (level & LLL_PARSER) 
    {
        log_debug(global_wsworker_pool->log, "PARSE: %s", msg);
    }
    else if (level & LLL_HEADER)
    {
        log_debug(global_wsworker_pool->log, "HEADER: %s", msg);
    }
    else if (level & LLL_EXT)
    {
        log_debug(global_wsworker_pool->log, "EXT: %s", msg);
    }
    else if (level & LLL_LATENCY)
    {
        log_debug(global_wsworker_pool->log, "LATENCY: %s", msg);
    }
#endif
#else
    (void)level;
    (void)msg;
#endif
}

//
// Returns host trusted certs to validate certs against
//
extern const char* trusted_certs(
    void
);

//
// Load extra certificates to validate against
//
static int32_t pal_wsworker_load_extra_client_verify_certs(
    void* user,
    const char* trusted_ca
)
{
    int32_t result;
    X509_STORE* cert_store;
    BIO* cert_memory_bio = NULL;
    BIO_METHOD* bio_method;
    X509* certificate;
    do
    {
        cert_store = SSL_CTX_get_cert_store(user);
        if (!cert_store)
        {
            result = er_fatal;
            break;
        }

        result = er_out_of_memory;
        bio_method = BIO_s_mem();
        if (!bio_method)
            break;
        cert_memory_bio = BIO_new(bio_method);
        if (!cert_memory_bio)
            break;
        result = BIO_puts(cert_memory_bio, trusted_ca);
        if (result < 0 || (size_t)result != strlen(trusted_ca))
        {
            result = er_fatal;
            break;
        }

        result = er_ok;
        while(true)
        {
            certificate = PEM_read_bio_X509(
                cert_memory_bio, NULL, NULL, NULL);
            if (!certificate)
                break;
            if (!X509_STORE_add_cert(cert_store, certificate))
            {
                result = er_fatal;
                X509_free(certificate);
                break;
            }
        } 
        if (result != er_ok)
            break;

    } while (0);

    if (cert_memory_bio)
        BIO_free(cert_memory_bio);
    return result;
}

//
// Free client 
//
static void pal_wsclient_free(
    pal_wsclient_t* wsclient
)
{
    dbg_assert_ptr(wsclient);
    dbg_assert(!wsclient->wsi, "Still open");

    wsclient->cb(wsclient->context, pal_wsclient_event_closed,
        NULL, NULL, NULL, er_ok);

    wsclient->cb = NULL;
    wsclient->context = NULL;

    if (wsclient->host)
        mem_free(wsclient->host);
    if (wsclient->relative_path)
        mem_free(wsclient->relative_path);
    if (wsclient->protocol_name)
        mem_free(wsclient->protocol_name);
    if (wsclient->headers)
        STRING_delete(wsclient->headers);

    if (wsclient->send_buffer)
        mem_free(wsclient->send_buffer);
    if (wsclient->recv_buffer)
        mem_free(wsclient->recv_buffer);

    mem_free_type(pal_wsclient_t, wsclient);
}

//
// Convert a buffer type from lws protocol flags
//
static pal_wsclient_buffer_type_t pal_wsclient_buffer_type_from_lws_flags(
    int flags,
    bool force_partial
)
{
    // Do not send out a fin if we cannot write all data to receive buffer...
    if (force_partial)
        flags |= LWS_WRITE_NO_FIN;

    // Note: No continuation here, flags was artificially created...
    switch (flags)
    {
    case LWS_WRITE_BINARY:
        return pal_wsclient_buffer_type_binary_msg;
    case LWS_WRITE_BINARY | LWS_WRITE_NO_FIN:
        return pal_wsclient_buffer_type_binary_fragment;
    case LWS_WRITE_TEXT:
        return pal_wsclient_buffer_type_utf8_msg;
    case LWS_WRITE_TEXT | LWS_WRITE_NO_FIN:
        return pal_wsclient_buffer_type_utf8_fragment;
    default:
        return pal_wsclient_buffer_type_unknown;
    }
}

//
// Convert a buffer type to lws protocol flags
//
static enum lws_write_protocol pal_wsclient_buffer_type_to_lws_flags(
    int last_mode,
    pal_wsclient_buffer_type_t type
)
{
    int flags = 0;
    if (last_mode & LWS_WRITE_NO_FIN)
    {
        // First frame was already sent, now sent follow on fragments
        switch (type)
        {
        case pal_wsclient_buffer_type_unknown:
        case pal_wsclient_buffer_type_binary_msg:
        case pal_wsclient_buffer_type_utf8_msg:
            // Final fragment
            flags = LWS_WRITE_CONTINUATION; 
            break;
        case pal_wsclient_buffer_type_binary_fragment:
        case pal_wsclient_buffer_type_utf8_fragment:
            // Continuation fragment
            flags = LWS_WRITE_CONTINUATION | LWS_WRITE_NO_FIN;  
            break;
        }
    }
    else
    {
        // New sequence of fragments
        switch (type)
        {
        case pal_wsclient_buffer_type_unknown:
        case pal_wsclient_buffer_type_binary_msg:
            flags = LWS_WRITE_BINARY; // single frame, no fragmentation
            break;
        case pal_wsclient_buffer_type_utf8_msg:
            flags = LWS_WRITE_TEXT;  // single frame, no fragmentation
            break;
        case pal_wsclient_buffer_type_binary_fragment:
            flags = LWS_WRITE_BINARY | LWS_WRITE_NO_FIN; // first fragment
            break;
        case pal_wsclient_buffer_type_utf8_fragment:
            flags = LWS_WRITE_TEXT | LWS_WRITE_NO_FIN;  // first fragment
            break;
        }
    }
    return (enum lws_write_protocol)flags;
}

//
// Validate certificates
//
static int pal_wsworker_lws_on_verify_certs(
    pal_wsworker_t* worker,
    void* user
)
{
    int32_t result;
    const char* trusted_ca;

    dbg_assert_ptr(worker);
    dbg_assert_ptr(user);

    trusted_ca = trusted_certs();
    if (trusted_ca)
    {
        result = pal_wsworker_load_extra_client_verify_certs(user, trusted_ca);
        if (result != er_ok)
        {
            log_error(worker->log, "Failed loading certs (%s)", 
                prx_err_string(result));
            return -1;
        }
    }
    return 0;
}

//
// Handle any error based on state
//
static int pal_wsworker_lws_on_error(
    pal_wsclient_t* wsclient,
    struct lws *wsi,
    const char* error
)
{
    dbg_assert_ptr(wsclient);
    dbg_assert(!wsclient->wsi || wsclient->wsi == wsi, "Unexpected");

    if (!error)
        error = "<Unknown lws error>";

    (void)wsi;
    // TODO: Do this based on current state (e.g. connecting, sending, etc.)
    // wsclient->last_error = er_unknown;

    dbg_assert_ptr(error);
    log_error(wsclient->log, "'%s'", error);
    return -1;
}

//
// Copy data to send into send buffer
//
static void pal_wsworker_lws_copy_in(
    pal_wsclient_t* wsclient
)
{
    int32_t result;
    pal_wsclient_buffer_type_t type;
    uint8_t* out;
    void* buf;

    dbg_assert_ptr(wsclient);

    dbg_assert(wsclient->send_buffer_offset == wsclient->send_buffer_size, 
        "Expected entire buffer to be sent by wsclient! (offset %d != size %d)", 
            wsclient->send_buffer_offset, wsclient->send_buffer_size);

    wsclient->cb(wsclient->context, pal_wsclient_event_begin_send,
        &out, &wsclient->send_buffer_size, &type, er_ok);
    wsclient->send_buffer_offset = wsclient->send_buffer_size;
    if (!out)
    {
        // Nothing to send, turn send off
        log_debug(wsclient->log, "Nothing to send...");
        wsclient->can_send = false;
        return;
    }
    do
    {
        result = er_ok;
#define LWS_BUFFER_PADDING (LWS_SEND_BUFFER_PRE_PADDING + LWS_SEND_BUFFER_POST_PADDING)
        buf = mem_realloc(wsclient->send_buffer, 
            wsclient->send_buffer_size + LWS_BUFFER_PADDING);
        if (!buf)
        {
            result = er_out_of_memory;
            wsclient->can_send = false;
            break;
        }

        wsclient->send_buffer = (uint8_t*)buf;
        wsclient->send_buffer_offset = 0;
        wsclient->send_flags = pal_wsclient_buffer_type_to_lws_flags(
            wsclient->send_flags, type);

        memcpy(&wsclient->send_buffer[LWS_SEND_BUFFER_PRE_PADDING], 
            out, wsclient->send_buffer_size);

        log_debug(wsclient->log, "Sending %zu bytes...", wsclient->send_buffer_size);
        break;
    } while (0);

    wsclient->cb(wsclient->context, pal_wsclient_event_end_send,
        &out, &wsclient->send_buffer_size, NULL, result);
}

//
// Send next tranche on writable wsi
//
static int pal_wsworker_lws_on_send(
    pal_wsclient_t* wsclient,
    struct lws *wsi
)
{
    int written;

    dbg_assert_ptr(wsclient);
    dbg_assert(wsclient->wsi == wsi, "Unexpected");

    do
    {
        if (wsclient->state & (1 << pal_wsclient_disconnecting_bit))
        {
            // Write close and indicate stack should close
#if (LWS_LIBRARY_VERSION_MAJOR > 1)
            lws_close_reason(wsi, 
                (enum lws_close_status)wsclient->disconnect_reason, NULL, 0);
#endif
            wsclient->disconnect_reason = LWS_CLOSE_STATUS_NOSTATUS;
            return -1;
        }

        if (wsclient->send_buffer_offset >= wsclient->send_buffer_size)
        {
            pal_wsworker_lws_copy_in(wsclient);
            if (!wsclient->can_send)
                break;
        }

        // Submit from offset with padding
        written = lws_write(wsclient->wsi,
            (unsigned char*)&wsclient->send_buffer[
                LWS_SEND_BUFFER_PRE_PADDING + wsclient->send_buffer_offset],
            wsclient->send_buffer_size - wsclient->send_buffer_offset,
                    (enum lws_write_protocol)wsclient->send_flags);
        if (written < 0)
        {
            // Handle error
            return pal_wsworker_lws_on_error(wsclient, wsclient->wsi, "Write error");
        }

        log_debug(wsclient->log, "... %d bytes (of %zu) sent (%x)!", written,
            wsclient->send_buffer_size, wsclient->send_flags);

        wsclient->send_buffer_offset += (size_t)written;
        (void)lws_callback_on_writable(wsclient->wsi);
#if (LWS_LIBRARY_VERSION_MAJOR > 1)
        lws_cancel_service_pt(wsi);
#endif
    }
    while (0);

    return 0;
}

//
// Copy out received data
//
static void pal_wsworker_lws_copy_out(
    pal_wsclient_t* wsclient,
    void *buf,
    size_t in_len,
    int flags
)
{
    uint8_t* out, *in = (uint8_t*)buf;
    size_t out_len, in_off = 0;
    pal_wsclient_buffer_type_t type;

    // Write received bytes to callback buffers
    while (in_len > 0)
    {
        wsclient->cb(wsclient->context, pal_wsclient_event_begin_recv,
            &out, &out_len, NULL, er_ok);
        if (!out)
        {
            // Stop can_recv...
            wsclient->can_recv = false;
            lws_rx_flow_control(wsclient->wsi, false);

            // Save off in_len data of buffer
            wsclient->recv_buffer = (uint8_t*)mem_alloc(in_len);
            if (wsclient->recv_buffer)
            {
                memcpy(wsclient->recv_buffer, &in[in_off], in_len);
                wsclient->recv_buffer_size = in_len;
                wsclient->recv_flags = flags;
            }
            else
            {
                wsclient->recv_buffer_size = 0;
                wsclient->recv_flags = 0;
            }
            return;
        }

        type = pal_wsclient_buffer_type_from_lws_flags(flags, out_len < in_len);
        if (out_len >= in_len)
            out_len = in_len;
        memcpy(out, &in[in_off], out_len);

        wsclient->cb(wsclient->context, pal_wsclient_event_end_recv,
            &out, &out_len, &type, er_ok);

        in_len -= out_len;
        in_off += out_len;
    }
}

//
// Received data
//
static int pal_wsworker_lws_on_receive(
    pal_wsclient_t* wsclient,
    struct lws *wsi,
    void *in,
    size_t in_len
)
{
    void *buf;
    size_t to_write;
    int flags;

    dbg_assert_ptr(wsclient);
    dbg_assert(wsclient->wsi == wsi, "Unexpected");

    if (wsclient->state & (1 << pal_wsclient_disconnecting_bit))
    {
        // Write close and indicate stack should close
#if (LWS_LIBRARY_VERSION_MAJOR > 1)
        lws_close_reason(wsi,
            (enum lws_close_status)wsclient->disconnect_reason, NULL, 0);
#endif
        wsclient->disconnect_reason = LWS_CLOSE_STATUS_NOSTATUS;
        return -1;
    }

    log_debug(wsclient->log, "Received %zu bytes...", in_len);

    if (wsclient->recv_buffer_size > 0)
    {
        buf = wsclient->recv_buffer;
        wsclient->recv_buffer = NULL;
        to_write = wsclient->recv_buffer_size;
        wsclient->recv_buffer_size = 0;
        flags = wsclient->recv_flags;
        wsclient->recv_flags = 0;

        // Write left over first
        pal_wsworker_lws_copy_out(wsclient, buf, to_write, flags);
        mem_free(buf);

        if (wsclient->recv_buffer_size > 0)
        {
            // TODO: Append in to recv_buffer...
            return 0;
        }
    }

    if (lws_frame_is_binary(wsi))
        flags = LWS_WRITE_BINARY;
    else
        flags = LWS_WRITE_TEXT;

    if (lws_remaining_packet_payload(wsi) > 0 || !lws_is_final_fragment(wsi))
        flags |= LWS_WRITE_NO_FIN;

    pal_wsworker_lws_copy_out(wsclient, in, in_len, flags);
    return 0;
}

//
// Write headers to passed in buffer
//
static int pal_wsworker_lws_on_header_write(
    pal_wsclient_t* wsclient,
    struct lws *wsi,
    void *in,
    size_t in_len
)
{
    char** pos = (char **)in;
    const char* headers;
    size_t out_len;
    dbg_assert_ptr(wsclient);
    (void)wsi;

    if (!wsclient->headers)
        return 0;

    headers = STRING_c_str(wsclient->headers);
    dbg_assert_ptr(headers);

    out_len = strlen(headers);
    if (out_len <= 2)
    {
        dbg_assert(0, "Length of headers is unexpected");
        return 1;
    }
    else if (out_len > in_len)
    {
        log_error(wsclient->log, "headers %s too large for lws - "
            "only %zu available, but %zu to write.",
            headers, in_len, out_len);
        return 1;
    }

    memcpy(*pos, headers, out_len);
    (*pos) += out_len;
    (*pos)[0] = '\0';
    return 0;
}

//
// Connected
//
static int pal_wsworker_lws_on_connect(
    pal_wsclient_t* wsclient,
    struct lws *wsi
)
{
    dbg_assert_ptr(wsclient);
    dbg_assert(wsclient->wsi == wsi, "Unexpected");

    if (wsclient->state & (1 << pal_wsclient_disconnecting_bit))
    {
        // Write close and indicate stack should close
#if (LWS_LIBRARY_VERSION_MAJOR > 1)
        lws_close_reason(wsi,
            (enum lws_close_status)wsclient->disconnect_reason, NULL, 0);
#endif
        wsclient->disconnect_reason = LWS_CLOSE_STATUS_NOSTATUS;
        return -1;
    }

    atomic_bit_set(wsclient->state, pal_wsclient_connected_bit);
    wsclient->cb(wsclient->context, pal_wsclient_event_connected,
        NULL, NULL, NULL, er_ok);
#if (LWS_LIBRARY_VERSION_MAJOR > 1)
    lws_cancel_service_pt(wsi);
#endif
    return 0;
}

//
// Disconnected
//
static int pal_wsworker_lws_on_disconnect(
    pal_wsclient_t* wsclient,
    struct lws *wsi
)
{
    dbg_assert_ptr(wsclient);
    dbg_assert(wsclient->wsi == wsi, "Unexpected");

    log_info(wsclient->log, "lws reported remote side closed");

    wsclient->last_error = er_closed;
    wsclient->wsi = NULL;

#if (LWS_LIBRARY_VERSION_MAJOR > 1)
    lws_cancel_service_pt(wsi);
#endif
    return 0;
}

//
// Destroyed
//
static int pal_wsworker_lws_on_destroy(
    pal_wsclient_t* wsclient,
    struct lws *wsi
)
{
    dbg_assert_ptr(wsclient);
    dbg_assert(!wsclient->wsi || wsclient->wsi == wsi, "Unexpected");

    wsclient->wsi = NULL; // Do not use anymore

#if (LWS_LIBRARY_VERSION_MAJOR > 1)
    lws_cancel_service_pt(wsi);
#endif
    return 0;
}

//
// libwebsocket callback for worker
//
static int pal_wsworker_lws_callback(
    struct lws *wsi,
    enum lws_callback_reasons reason,
    void *user,
    void *in,
    size_t len
)
{
    int result;
    switch (reason)
    {
    case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER:
        result = pal_wsworker_lws_on_header_write(
            (pal_wsclient_t*)lws_wsi_user(wsi), wsi, in, len);
        break;
    case LWS_CALLBACK_CLIENT_ESTABLISHED:
        result = pal_wsworker_lws_on_connect(
            (pal_wsclient_t*)user, wsi);
        break;
    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        result = pal_wsworker_lws_on_error(
            (pal_wsclient_t*)user, wsi, (const char*)in);
        break;
    case LWS_CALLBACK_CLOSED:
        result = pal_wsworker_lws_on_disconnect(
            (pal_wsclient_t*)user, wsi);
        break;
    case LWS_CALLBACK_WSI_DESTROY:
        result = pal_wsworker_lws_on_destroy(
            (pal_wsclient_t*)user, wsi);
        break;
    case LWS_CALLBACK_CLIENT_WRITEABLE:
        result = pal_wsworker_lws_on_send(
            (pal_wsclient_t*)user, wsi);
        break;
    case LWS_CALLBACK_CLIENT_RECEIVE:
        result = pal_wsworker_lws_on_receive(
            (pal_wsclient_t*)user, wsi, in, len);
        break;
    case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS:
        // This is a fake wsi, only context is valid, user is a ssl_context
        result = pal_wsworker_lws_on_verify_certs(
            (pal_wsworker_t*)lws_context_user(lws_get_context(wsi)), user);
        break;
    case LWS_CALLBACK_GET_THREAD_ID:
        result = ((pal_wsworker_t*)lws_context_user(lws_get_context(wsi)))->id;
        break;
    default:
        log_debug(NULL, ">>>>>> %d", reason);
        // fall through
    case LWS_CALLBACK_CHANGE_MODE_POLL_FD:
    case LWS_CALLBACK_LOCK_POLL:
    case LWS_CALLBACK_UNLOCK_POLL:
        result = 0;
        break;
    }
    return result;
}

//
// Free worker resources
//
static void pal_wsworker_free(
    pal_wsworker_t* worker
)
{
    dbg_assert_ptr(worker);

    // Detach all remaining clients...
    dbg_assert(DList_IsListEmpty(&worker->clients), "Still clients attached");

    if (worker->context)
        lws_context_destroy(worker->context);

    if (worker->protocols)
    {
        if (worker->protocols->name)
            mem_free((void*)worker->protocols->name);
        mem_free(worker->protocols);
    }

    if (worker->clients_lock)
        lock_free(worker->clients_lock);

    mem_free_type(pal_wsworker_t, worker);
}

//
// Kill and free worker
//
static void pal_wsworker_kill(
    pal_wsworker_t* worker
)
{
    int result;
    THREAD_HANDLE worker_thread;
    dbg_assert_ptr(worker);

    worker_thread = worker->service_thread;
    worker->service_thread = NULL;

    if (!worker_thread)
    {
        pal_wsworker_free(worker);
        return;
    }

    worker->kill = false;
    lws_cancel_service(worker->context);

    if (THREADAPI_OK != ThreadAPI_Join(worker_thread, &result))
    {
        log_error(worker->log, "Failed stopping lws service thread");
    }
    // Worker is now freed...
}

//
// Service context in worker
//
static int32_t pal_wsworker_thread(
    void* ctx
)
{
    bool running = true;
    pal_wsclient_t* next;
#if (LWS_LIBRARY_VERSION_MAJOR > 1)
    struct lws_client_connect_info connect_info;
#endif
    pal_wsworker_t* worker = (pal_wsworker_t*)ctx;
    dbg_assert_ptr(worker);

    while (true)
    {
        // Update pollfd's or complete close
        lock_enter(worker->clients_lock);
        for (PDLIST_ENTRY p = worker->clients.Flink; p != &worker->clients;)
        {
            next = containingRecord(p, pal_wsclient_t, link);

            if (worker->kill)
            {
                // Pool is closing, kill attached clients
                atomic_bit_clear(next->state, pal_wsclient_connecting_bit);
                atomic_bit_set(next->state, pal_wsclient_disconnecting_bit);
                atomic_bit_set(next->state, pal_wsclient_closing_bit);
                next->disconnect_reason = LWS_CLOSE_STATUS_GOINGAWAY;
            }

            if (!next->wsi && next->last_error == er_ok)
            {
                if (atomic_bit_clear(next->state, pal_wsclient_connecting_bit))
                {
                    // If we were asked to connect, connect and wait
                    log_debug(next->log, "Connecting %p ...", next);
#if (LWS_LIBRARY_VERSION_MAJOR > 1)
                    memset(&connect_info, 0, sizeof(connect_info));
                    connect_info.context = worker->context;
                    connect_info.address = next->host;
                    connect_info.port = next->port;
                    connect_info.ssl_connection = next->secure ? LCCSCF_USE_SSL : 0;
                    connect_info.path = next->relative_path;
                    connect_info.host = next->host;
                    connect_info.origin = next->host;
                    connect_info.protocol = worker->protocols->name;
                    connect_info.ietf_version_or_minus_one = -1;
                    connect_info.pwsi = &next->wsi;
                    connect_info.userdata = next;
                    next->wsi = lws_client_connect_via_info(&connect_info);
#else
                    next->wsi = lws_client_connect_extended(worker->context, 
                        next->host, next->port, next->secure ? 1 : 0,
                        next->relative_path, next->host, next->host, 
                        worker->protocols->name, -1 /* ietf_latest */, next);
#endif
                    if (!next->wsi)
                    {
                        log_error(next->log, "Failed to connect %p", next);
                        atomic_bit_set(next->state, pal_wsclient_disconnecting_bit);
                        next->last_error = er_connecting;
                    }
                    else
                    {
                        p = p->Flink;
                        continue;
                    }
                }
            }

            /**/ if (next->state & (1 << pal_wsclient_closing_bit))
            {
                if (next->wsi)
                {
                    dbg_assert(next->last_error == er_ok, "Should we be here then?");
                    // write close reason
                    if (-1 == lws_callback_on_writable(next->wsi))
                    {
                        log_error(next->log, "Failed to set up final close for %p", next);
                        next->last_error = er_closed;
                        next->wsi = NULL; // Give up on wsi
                    }
                }

                // If we were asked to close the client, and we are disconnected, close
                if (!next->wsi)
                {
                    atomic_bit_clear(next->state, pal_wsclient_closing_bit);
                    // Remove from list and continue
                    p = p->Flink;
                    DList_RemoveEntryList(&next->link);
                    pal_wsclient_free(next);
                    continue;
                }
            }
            else if (next->state & (1 << pal_wsclient_disconnecting_bit) ||
                     next->last_error != er_ok)
            {
                if (next->wsi)
                {
                    // write close reason
                    if (-1 == lws_callback_on_writable(next->wsi))
                    {
                        log_error(next->log, "Failed to set up disconnect close for %p", next);
                        next->last_error = er_closed;
                        next->wsi = NULL; // Give up on wsi
                    }
                }
                if (!next->wsi)
                {
                    atomic_bit_clear(next->state, pal_wsclient_disconnecting_bit);
                    log_debug(next->log, "Notifiying disconnect for %p", next);
                    // Notify disconnect so caller can free socket
                    next->cb(next->context, pal_wsclient_event_disconnected,
                        NULL, NULL, NULL, next->last_error);
                    next->last_error = er_ok;
                }
            }
            else if (next->wsi && next->state & (1 << pal_wsclient_connected_bit))
            {
                if (next->can_send)
                {
                    if (-1 == lws_callback_on_writable(next->wsi))
                    {
                        log_error(next->log, "Failed to enable sending for %p", next);
                    }
                }
                
                // Modify pollfd for receive side
                if (-1 == lws_rx_flow_control(next->wsi, next->can_recv))
                {
                    log_error(next->log, "Failed to enable receiving for %p", next);
                }
            }
            p = p->Flink;
        }

        running = !DList_IsListEmpty(&worker->clients);
        lock_exit(worker->clients_lock);

        // If last client was disconnected, release worker. Need to coordinate with user.
        if (!running)
        {
            // Remove from worker list
            lock_enter(worker->pool->pool_lock);
            DList_RemoveEntryList(&worker->link);
            lock_exit(worker->pool->pool_lock);

            // Check again to see if a client was attached while we waited for worker lock
            lock_enter(worker->clients_lock);
            running = !DList_IsListEmpty(&worker->clients);
            lock_exit(worker->clients_lock);
            if (!running)
                break; // No, nothing added, we can kill...

            // Re-attach to the pool list, and continue running...
            lock_enter(worker->pool->pool_lock);
            DList_InsertTailList(&worker->pool->workers, &worker->link);
            lock_exit(worker->pool->pool_lock);
            continue;
        }

        // Service the context until expires or cancels
        (void)lws_service(worker->context, 3600000);
    }

    pal_wsworker_free(worker);
    return 0;
}

//
// Provide proxy information if configured
//
static int32_t pal_wsworker_get_proxy_info(
    char** proxy_address
)
{
    size_t buf_len;
    const char* proxy;
    const char* user;
    const char* pwd;

    dbg_assert_ptr(proxy_address);

    proxy = __prx_config_get(prx_config_key_proxy_host, NULL);
    if (!proxy)
    {
        *proxy_address = NULL;
        return er_ok;  // No proxy configured, return success
    }
    user = __prx_config_get(prx_config_key_proxy_user, NULL);
    pwd = __prx_config_get(prx_config_key_proxy_pwd, NULL);

    buf_len = 
          strlen(proxy) 
        + (user ? strlen(user) + 1 : 0) // user sep @
        + (pwd ? strlen(pwd) + 1 : 0) // add pwd sep :
        + 1;  // plus null term
    
    // Use malloc for lws to free
    *proxy_address = (char*)mem_zalloc(buf_len);
    if (!*proxy_address)
        return er_out_of_memory;
    // Concat proxy address string
    if (user)
    {
        strcat(*proxy_address, user);
        if (pwd)
        {
            strcat(*proxy_address, ":");
            strcat(*proxy_address, pwd);
        }
        strcat(*proxy_address, "@");
    }
    strcat(*proxy_address, proxy);
    return er_ok;
}

//
// Get a worker from the websocket client pool
//
static int32_t pal_wsworker_pool_attach(
    pal_wsworker_pool_t* pool,
    pal_wsclient_t* wsclient
)
{
    int32_t result;
    THREADAPI_RESULT thread_result;
    struct lws_context_creation_info info;
    pal_wsworker_t* worker;
    size_t count;

    dbg_assert_ptr(pool);
    dbg_assert_ptr(wsclient);

    lock_enter(pool->pool_lock);
    // Find a worker that matches the protocol and has still room (max fd per worker)
    for (PDLIST_ENTRY p = pool->workers.Flink; p != &pool->workers; p = p->Flink)
    {
        worker = containingRecord(p, pal_wsworker_t, link);
        if (0 != string_compare_nocase(worker->protocols->name, wsclient->protocol_name))
            continue;

        count = 0;
        for (PDLIST_ENTRY c = worker->clients.Flink; c != &worker->clients; c = c->Flink)
            ++count;
        if (count < MAX_WEBSOCKET_PER_WORKER)
        {
            // Insert client here and signal worker to connect it
            wsclient->worker = worker;
            atomic_bit_set(wsclient->state, pal_wsclient_connecting_bit);

            lock_enter(worker->clients_lock);
            DList_InsertTailList(&worker->clients, &wsclient->link);
            lock_exit(worker->clients_lock);

            lws_cancel_service(worker->context);
            lock_exit(pool->pool_lock);
            return er_ok;
        }
    }

    // No worker found, create new worker for this client
    do
    {
        worker = mem_zalloc_type(pal_wsworker_t);
        if (!worker)
        {
            result = er_out_of_memory;
            break;
        }

        DList_InitializeListHead(&worker->link);
        DList_InitializeListHead(&worker->clients);
        worker->log = log_get("pal_ws_proto");
        worker->pool = pool;
        worker->protocols = (struct lws_protocols*)mem_zalloc(
            sizeof(struct lws_protocols) * 2);
        if (!worker->protocols)
        {
            result = er_out_of_memory;
            break;
        }

        result = string_clone(wsclient->protocol_name, (char**)&worker->protocols->name);
        if (result != er_ok)
            break;
        result = lock_create(&worker->clients_lock);
        if (result != er_ok)
            break;

        worker->protocols->callback = pal_wsworker_lws_callback;
        worker->protocols->user = worker;

        // Create context. 
        memset(&info, 0, sizeof(info));
        info.port = CONTEXT_PORT_NO_LISTEN;
        info.protocols = worker->protocols;
        info.gid = -1;
        info.uid = -1;
#if (LWS_LIBRARY_VERSION_MAJOR > 1)
        info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
#else
        info.extensions = lws_get_internal_extensions();
#endif
        info.user = worker;

        result = pal_wsworker_get_proxy_info((char**)&info.http_proxy_address);
        if (result != er_ok)
            break;
        worker->context = lws_create_context(&info);
        if (info.http_proxy_address)
            mem_free((char*)info.http_proxy_address);
        if (!worker->context)
        {
            log_error(worker->log, "Failed to create lws context!");
            result = er_out_of_memory;
            break;
        }

        // Insert client here as first citizen
        atomic_bit_set(wsclient->state, pal_wsclient_connecting_bit);
        wsclient->worker = worker;
        DList_InsertTailList(&worker->clients, &wsclient->link);
        worker->id = (int)atomic_inc(pool->counter);

        // Start worker thread servicing the context, which will connect this client
        thread_result = ThreadAPI_Create(
            &worker->service_thread, pal_wsworker_thread, worker);
        if (THREADAPI_OK != thread_result)
        {
            log_error(worker->log, "Failed to start worker thread!");
            DList_RemoveEntryList(&wsclient->link);

            // TODO: Check closing bit here...
            wsclient->worker = NULL;
            result = er_fault;
            break;
        }

        DList_InsertTailList(&pool->workers, &worker->link);
        result = er_ok; // success
    } 
    while (0);
    lock_exit(pool->pool_lock);

    if (result != er_ok && worker)
        pal_wsworker_free(worker);
    return result;
}

//
// Free websocket client pool
//
static void pal_wsworker_pool_free(
    pal_wsworker_pool_t* pool
)
{
    pal_wsworker_t* next;
    dbg_assert_ptr(pool);

    if (pool->pool_lock)
    {
        // Kill all workers one by one...
        lock_enter(pool->pool_lock);
        while (!DList_IsListEmpty(&pool->workers))
        {
            next = containingRecord(pool->workers.Flink, pal_wsworker_t, link);
            lock_exit(pool->pool_lock); // Cannot hold lock
            pal_wsworker_kill(next);
            lock_enter(pool->pool_lock);
        }
        lock_exit(pool->pool_lock);
        lock_free(pool->pool_lock);
    }

    mem_free_type(pal_wsworker_pool_t, pool);
}

//
// Create websocket client pool
//
static int32_t pal_wsworker_pool_create(
    pal_wsworker_pool_t** created
)
{
    int32_t result;
    pal_wsworker_pool_t* pool;

    dbg_assert_ptr(created);
    pool = mem_zalloc_type(pal_wsworker_pool_t);
    if (!pool)
        return er_out_of_memory;
    do
    {
        DList_InitializeListHead(&pool->workers);
        pool->log = log_get("pal_ws_lws");
        pool->counter = 1;
        result = lock_create(&pool->pool_lock);
        if (result != er_ok)
            break;

        *created = pool;
        return er_ok;
    } while (0);

    pal_wsworker_pool_free(pool);
    return result;
}

//
// Create and connect websocket - wait for event
//
int32_t pal_wsclient_create(
    const char* protocol_name,
    const char* host,
    uint16_t port,
    const char* path,
    bool secure,
    pal_wsclient_event_handler_t callback,
    void* context,
    pal_wsclient_t** created
)
{
    int32_t result;
    pal_wsclient_t* wsclient;

    if (!protocol_name)
        protocol_name = "";
    if (!host || !path || !created || !callback)
        return er_fault;
    if (!global_wsworker_pool)
        return er_bad_state;

    wsclient = mem_zalloc_type(pal_wsclient_t);
    if (!wsclient)
        return er_out_of_memory;
    do
    {
        wsclient->log = log_get("pal_ws");
        wsclient->cb = callback;
        wsclient->context = context;
        wsclient->secure = secure;
        DList_InitializeListHead(&wsclient->link);

        result = string_clone(host, &wsclient->host);
        if (result != er_ok)
            break;
        wsclient->port = port;
        result = string_clone(path, &wsclient->relative_path);
        if (result != er_ok)
            break;

        result = string_clone(protocol_name, &wsclient->protocol_name);
        if (result != er_ok)
            break;

        *created = wsclient;
        return er_ok;
    } 
    while (0);
    
    pal_wsclient_free(wsclient);
    return result;
}

//
// Add key value to header
//
int32_t pal_wsclient_add_header(
    pal_wsclient_t* wsclient,
    const char* key,
    const char* value
)
{
    if (!wsclient || !key || !value)
        return er_fault;
    if (!strlen(key))
        return er_arg;

    if (!wsclient->headers)
    {
        wsclient->headers = STRING_new();
        if (!wsclient->headers)
            return er_out_of_memory;
    }

    if (0 != STRING_concat(wsclient->headers, key) ||
        0 != STRING_concat(wsclient->headers, ": ") ||
        0 != STRING_concat(wsclient->headers, value) ||
        0 != STRING_concat(wsclient->headers, "\r\n"))
    {
        STRING_delete(wsclient->headers);
        wsclient->headers = NULL;
        return er_out_of_memory;
    }
    return er_ok;
}
//
// Create and connect websocket - wait for event
//
int32_t pal_wsclient_connect(
    pal_wsclient_t* wsclient
)
{
    if (!wsclient)
        return er_fault;
    if (wsclient->wsi)
        return er_bad_state;

    if (wsclient->worker)
    {
        wsclient->last_error = er_ok;
        atomic_bit_set(wsclient->state, pal_wsclient_connecting_bit);
        return er_ok;
    }
    
    // Insert into a worker which will start connect
    return pal_wsworker_pool_attach(global_wsworker_pool, wsclient);
}

//
// Start to receive on the provided websocket client
//
int32_t pal_wsclient_can_recv(
    pal_wsclient_t* wsclient,
    bool enable
)
{
    if (!wsclient)
        return er_fault;

    if (wsclient->can_recv != enable)
    {
        wsclient->can_recv = enable;
        if (wsclient->worker)
        {
            // Interrupt worker to correct events
            lws_cancel_service(wsclient->worker->context);
        }
    }
    return er_ok;
}

//
// Start to receive on the provided websocket client
//
int32_t pal_wsclient_can_send(
    pal_wsclient_t* wsclient,
    bool enable
)
{
    if (!wsclient)
        return er_fault;

    if (wsclient->can_send != enable)
    {
        wsclient->can_send = enable;
        if (wsclient->worker)
        {
            // Interrupt worker to correct events
            lws_cancel_service(wsclient->worker->context);
        }
    }
    return er_ok;
}

//
// Disconnect websocket client
//
int32_t pal_wsclient_disconnect(
    pal_wsclient_t* wsclient
)
{
    if (!wsclient)
        return er_fault;

    if (!wsclient->worker)
    {
       // wsclient->cb(wsclient->context, pal_wsclient_event_disconnected,
       //     NULL, NULL, NULL, wsclient->disconnect_reason);
        return er_ok;
    }

    wsclient->disconnect_reason = LWS_CLOSE_STATUS_NORMAL;
    atomic_bit_set(wsclient->state, pal_wsclient_disconnecting_bit);
    // Interrupt worker to disconnect
    lws_cancel_service(wsclient->worker->context);
    // Wait for close to complete
    return er_ok;
}

//
// Close websocket client 
//
void pal_wsclient_close(
    pal_wsclient_t* wsclient
)
{
    if (!wsclient)
        return;

    if (!wsclient->worker)
    {
        pal_wsclient_free(wsclient);
        return;
    }

    wsclient->disconnect_reason = LWS_CLOSE_STATUS_GOINGAWAY;
    atomic_bit_set(wsclient->state, pal_wsclient_disconnecting_bit);
    atomic_bit_set(wsclient->state, pal_wsclient_closing_bit);
    // Interrupt worker to close 
    lws_cancel_service(wsclient->worker->context);
    // Wait for close to complete
}

//
// Initialize global websocket pool
//
int32_t pal_wsclient_init(
    void
)
{
    int32_t result;
    if (global_wsworker_pool)
        return er_bad_state;
    result = pal_wsworker_pool_create(&global_wsworker_pool);
    if (result == er_ok)
        lws_set_log_level(-1, pal_wsclient_lws_log);
    return result;
}

//
// Free the global websocket pool
//
void pal_wsclient_deinit(
    void
)
{
    if (!global_wsworker_pool)
        return;
    lws_set_log_level(0, NULL);
    pal_wsworker_pool_free(global_wsworker_pool);
    global_wsworker_pool = NULL;
}
