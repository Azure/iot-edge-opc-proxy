// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "util_misc.h"
#include "util_string.h"

#include "io_browse.h"
#include "io_queue.h"

#include "prx_browse.h"
#include "prx_config.h"

#include "pal.h"
#include "pal_mt.h"
#include "pal_sd.h"
#include "pal_scan.h"
#include "pal_net.h"
#include "pal_file.h"

#include "hashtable.h"
#include "hashtable_itr.h"

#include "azure_c_shared_utility/doublylinkedlist.h"

//
// The browse server object
//
struct prx_browse_server
{
    prx_scheduler_t* scheduler;            // Scheduler the server runs on
    DLIST_ENTRY sessions;                       // List of active sessions
    lock_t sessions_lock;
                   // Client handles to browsable objects in the pal, etc.
    bool allow_fs_browse;           // Whether to allow file system browse
    bool supports_scan;       // Whether port and ip scanning is supported
    pal_sdclient_t* sdclient;                     // Service browse client
    bool destroy;                             // Set to free browse server
    log_t log;
};

//
// Represents a remote client opening browse requests on the server
//
struct prx_browse_session
{
    pal_socket_t* sock;           // Accepted and connected browser socket
    bool closed;                                 // Socket has been closed
    bool destroy;
    prx_browse_server_t* server;
    io_codec_t* codec;               // Codec to use for decode and encode
    prx_scheduler_t* scheduler;                  // browser scheduler copy
                                // Thread safe inbound and outbound queues
    io_queue_t* inbound;    // Inbound buffer queue with received messages
    io_queue_t* outbound;           // Outbound queue for messages to send

    int32_t last_error;  // Last error received in connect/disconnect path
    struct hashtable* requests;                 // List of active requests

    pal_sdclient_t* owner; // The client used to create contained browsers
    DLIST_ENTRY link;           // Link to queue browser clients in server
    log_t log;
};

#define MAX_MESSAGE_SIZE 0x800

//
// A request initiated browsing stream
//
typedef struct prx_browse_stream
{
    prx_browse_session_t* session;               // Owning browser session
    io_ref_t handle;                                     // session handle
    pal_sdbrowser_t* sdbrowser;              // Proxied sd browser browser
    pal_scan_t* scan;                         // If stream contains a scan
}
prx_browse_stream_t;

//
// Wait this long until a stream is declared as all for now
//
#define STREAM_TIMEOUT 3000

DEFINE_HASHTABLE_INSERT(prx_browse_session_add, io_ref_t, prx_browse_stream_t);
DEFINE_HASHTABLE_SEARCH(prx_browse_session_get, io_ref_t, prx_browse_stream_t);
DEFINE_HASHTABLE_REMOVE(prx_browse_session_remove, io_ref_t, prx_browse_stream_t);

//
// Free stream on scheduler
//
static void prx_browse_stream_free(
    prx_browse_stream_t* stream
)
{
    if (!stream)
        return;

    if (stream->session && stream->session->scheduler)
        prx_scheduler_clear(stream->session->scheduler, NULL, stream);

    if (stream->sdbrowser)
        pal_sdbrowser_free(stream->sdbrowser);

    if (stream->scan)
        pal_scan_close(stream->scan);

    // ...

    mem_free_type(prx_browse_stream_t, stream);
}

//
// Free session on scheduler thread when fully disconnected.
//
static void prx_browse_session_free(
    prx_browse_session_t* session
)
{
    prx_browse_stream_t* next;
    struct hashtable_itr* itr;

    dbg_assert_ptr(session);
    dbg_assert_is_task(session->scheduler);

    // Free all streams still open in this session...
    if (hashtable_count(session->requests) > 0)
    {
        itr = hashtable_iterator(session->requests);
        if (itr)
        {
            do
            {
                next = (prx_browse_stream_t*)hashtable_iterator_value(itr);
                log_info(session->log, "Freeing open stream %p", next);
                prx_browse_stream_free(next);
            }
            while (0 != hashtable_iterator_remove(itr));
            crt_free(itr);
        }
    }

    if (!session->destroy) // Set only when pal closed socket
    {
        if (!session->closed)
        {
            pal_socket_close(session->sock);
            session->closed = true;
        }
        return; // Called again when close completes async..
    }

    if (session->requests)
        hashtable_destroy(session->requests, 0);
    if (session->sock)
        pal_socket_free(session->sock);
    if (session->outbound)
        io_queue_free(session->outbound);
    if (session->inbound)
        io_queue_free(session->inbound);
    if (session->scheduler)
        prx_scheduler_release(session->scheduler, session);

    if (session->server)
    {
        lock_enter(session->server->sessions_lock);
        DList_RemoveEntryList(&session->link);
        if (session->server->destroy)
        {
            __do_next(session->server, prx_browse_server_free);
        }
        lock_exit(session->server->sessions_lock);
    }

    mem_free_type(prx_browse_session_t, session);
}

//
// Send all outbound buffers
//
static void prx_browse_session_send_buffers(
    prx_browse_session_t* session
)
{
    dbg_assert_ptr(session);
    dbg_assert_is_task(session->scheduler);
    pal_socket_can_send(session->sock, true);
}

//
// Send response object - called from pal thread!
//
static void prx_browse_session_send_response(
    prx_browse_session_t* session,
    io_browse_response_t* browse_response
)
{
    int32_t result;
    io_queue_buffer_t* buffer;
    io_codec_ctx_t ctx, obj;

    dbg_assert_ptr(session);
    dbg_assert_ptr(browse_response);

    // Assumption is that session is valid here, i.e. sched thread to ensure
    // all pal resources (sk, sd) are properly destroyed before session itself
    // is taken down!

    result = io_queue_create_buffer(session->outbound, NULL,
        MAX_MESSAGE_SIZE, &buffer);
    if (result != er_ok)
    {
        // failed to create buffer - bail
        log_error(session->log, "Failed to create outbound buffer (%s)",
            prx_err_string(result));
        return;
    }
    do
    {
        result = io_codec_ctx_init(session->codec, &ctx,
            io_queue_buffer_as_stream(buffer), false, session->log);
        if (result != er_ok)
        {
            log_error(session->log,
                "Failed to initialize codec from stream (%s)",
                prx_err_string(result));
            break;
        }

        // Encode response
        result = io_encode_object(&ctx, NULL, false, &obj);
        if (result == er_ok)
            result = io_encode_browse_response(&obj, browse_response);
        if (result != er_ok)
        {
            log_error(session->log, "Failed to encode response object (%s)",
                prx_err_string(result));
            break;
        }

        // Note: dynamic stream has no close function, so no cleanup needed
        result = io_codec_ctx_fini(&ctx, io_queue_buffer_as_stream(buffer), true);
        if (result != er_ok)
        {
            log_error(session->log, "Failed to finalize codec to stream (%s)",
                prx_err_string(result));
            break;
        }

        // Mark read and ask for socket to pick up
        io_queue_buffer_set_ready(buffer);

        if (browse_response->error_code != er_ok)
        {
            log_trace(session->log, "Browse response sent (with error code %s)",
                prx_err_string(browse_response->error_code));
        }
        else
        {
            log_debug(session->log, "Browse response sent...");
        }

        __do_next(session, prx_browse_session_send_buffers);
        return;
    }
    while (0);
    // Error, release buffer here
    io_queue_buffer_release(buffer);
}

//
// Send an error response for the request handle and clean up any session
//
static void prx_browse_session_send_error_response(
    prx_browse_session_t* session,
    io_ref_t* handle,
    int32_t error
)
{
    io_browse_response_t browse_response;

    dbg_assert_ptr(session);
    dbg_assert_is_task(session->scheduler);
    dbg_assert_ptr(handle);

    memset(&browse_response, 0, sizeof(io_browse_response_t));
    io_ref_copy(handle, &browse_response.handle);
    browse_response.error_code = error;
    browse_response.flags = io_browse_response_empty;
    session->last_error = error;
    prx_browse_session_send_response(session, &browse_response);
}

//
// Send all for now if not received response for a while
//
static void prx_browse_stream_timeout(
    prx_browse_stream_t* stream
)
{
    io_browse_response_t browse_response;
    dbg_assert_ptr(stream);
    dbg_assert_ptr(stream->session);
    dbg_assert_is_task(stream->session->scheduler);

    log_info(stream->session->log, "Timing out stream %p...", stream);
    memset(&browse_response, 0, sizeof(io_browse_response_t));
    io_ref_copy(&stream->handle, &browse_response.handle);
    browse_response.flags = io_browse_response_empty | io_browse_response_allfornow;
    prx_browse_session_send_response(stream->session, &browse_response);
}

//
// Returns an active stream in the session
//
static int32_t prx_browse_session_get_or_create_request(
    prx_browse_session_t* session,
    io_ref_t* handle,
    prx_browse_stream_t** created
)
{
    prx_browse_stream_t* stream = NULL;

    dbg_assert_ptr(session);
    dbg_assert_ptr(session->server);
    dbg_assert_ptr(handle);
    dbg_assert_is_task(session->scheduler);

    stream = prx_browse_session_get(session->requests, handle);
    if (!stream)
    {
        stream = (prx_browse_stream_t*)mem_zalloc_type(prx_browse_stream_t);
        if (!stream)
            return er_out_of_memory;
        stream->session = session;
        io_ref_copy(handle, &stream->handle);
        prx_browse_session_add(session->requests, handle, stream);
    }
    *created = stream;
    return er_ok;
}

//
// Cancels a browse stream if it is open
//
static int32_t prx_browse_session_close_request(
    prx_browse_session_t* browser,
    io_ref_t* handle
)
{
    prx_browse_stream_t* stream = NULL;

    dbg_assert_ptr(browser);
    dbg_assert_ptr(browser->server);
    dbg_assert_ptr(handle);
    dbg_assert_is_task(browser->scheduler);

    stream = prx_browse_session_get(browser->requests, handle);
    if (stream)
    {
        prx_browse_session_remove(browser->requests, handle);
        prx_browse_stream_free(stream);
    }
    return er_ok;
}

//
// Handle unknown request
//
static void prx_browse_session_handle_unknown_request(
    prx_browse_session_t* session,
    io_browse_request_t* browse_request
)
{
    dbg_assert_ptr(session);
    dbg_assert_ptr(browse_request);
    dbg_assert_is_task(session->scheduler);

    log_trace(session->log, "Unsupported request received (%d)", browse_request->type);
    prx_browse_session_send_error_response(session, &browse_request->handle,
        er_not_supported);
}

//
// Called for found ports and addresses
//
static void prx_browse_session_handle_scan_response(
    void *context,
    uint64_t itf_index,
    int32_t error,
    prx_socket_address_t *addr,
    const char* host_name
)
{
    io_browse_response_t browse_response;
    prx_property_t prop;
    prx_browse_stream_t* stream = (prx_browse_stream_t*)context;

    (void)itf_index;
    dbg_assert_ptr(context);

    memset(&browse_response, 0, sizeof(io_browse_response_t));
    io_ref_copy(&stream->handle, &browse_response.handle);

    if (error != er_ok)
    {
        browse_response.flags |= io_browse_response_eos;
        if (error != er_nomore)
            browse_response.error_code = error;
    }
    if (addr)
        memcpy(&browse_response.item, addr, sizeof(prx_socket_address_t));
    else
    {
        browse_response.flags |= io_browse_response_empty;
        browse_response.flags |= io_browse_response_allfornow;
    }
    if (host_name)
    {
        browse_response.props_size = 1;
        browse_response.props = &prop;
        prop.type = prx_record_type_default;
        prop.property.bin.value = (uint8_t*)host_name;
        prop.property.bin.size = strlen(host_name) + 1;
    }
    prx_browse_session_send_response(stream->session, &browse_response);
}

//
// Create or update a port scanning session
//
static void prx_browse_session_handle_portscan_request(
    prx_browse_session_t* session,
    io_browse_request_t* browse_request
)
{
    int32_t result;
    prx_browse_stream_t* stream = NULL;
    uint16_t port_start;
    uint16_t port_end;
    prx_addrinfo_t* info = NULL;
    size_t info_count;

    dbg_assert_ptr(session);
    dbg_assert_ptr(browse_request);
    dbg_assert_is_task(session->scheduler);
    do
    {
        if (!session->server->supports_scan)
        {
            // ip scanning is not supported
            prx_browse_session_handle_unknown_request(session, browse_request);
            return;
        }

        // Now get or create the stream for this handle
        result = prx_browse_session_get_or_create_request(session,
            &browse_request->handle, &stream);
        if (result != er_ok)
            break;

        // Parse and validate host string
        if (browse_request->item.un.family != prx_address_family_proxy &&
            browse_request->item.un.family != prx_address_family_inet &&
            browse_request->item.un.family != prx_address_family_inet6)
            port_start = 0;
        else
            port_start = browse_request->item.un.ip.port;

        port_end = (uint16_t)-1;  // todo: Through upper 16 bit of flags?

        if (browse_request->item.un.family == prx_address_family_proxy)
        {
            // Convert to ip address
            result = pal_getaddrinfo(
                prx_socket_address_proxy_get_host(&browse_request->item.un.proxy),
                NULL, prx_address_family_unspec, 0, &info, &info_count);
            if (result != er_ok)
                break;
            if (info_count == 0)
            {
                prx_browse_session_handle_scan_response(stream, 0, er_nomore, NULL, NULL);
                pal_freeaddrinfo(info);
                return;
            }
            result = pal_scan_ports(&info[0].address, port_start, port_end, 0,
                prx_browse_session_handle_scan_response, stream, &stream->scan);
            pal_freeaddrinfo(info);
        }
        else
        {
            result = pal_scan_ports(&browse_request->item, port_start, port_end, 0,
                prx_browse_session_handle_scan_response, stream, &stream->scan);
        }
        if (result != er_ok)
            break;
        return;
    }
    while (0);

    log_error(session->log, "Failed scanning host (%s)", prx_err_string(result));
    prx_browse_session_send_error_response(session, &browse_request->handle, result);
    if (stream)
        prx_browse_session_close_request(session, &stream->handle);
}

//
// Create or update a ip scanning session
//
static void prx_browse_session_handle_ipscan_request(
    prx_browse_session_t* session,
    io_browse_request_t* browse_request
)
{
    int32_t result;
    prx_browse_stream_t* stream = NULL;
    uint16_t port;

    dbg_assert_ptr(session);
    dbg_assert_ptr(browse_request);
    dbg_assert_is_task(session->scheduler);
    do
    {
        if (!session->server->supports_scan)
        {
            // ip scanning is not supported
            prx_browse_session_handle_unknown_request(session, browse_request);
            return;
        }

        // Now get or create the stream for this handle
        result = prx_browse_session_get_or_create_request(session,
            &browse_request->handle, &stream);
        if (result != er_ok)
            break;

        // Parse and validate host string
        if (browse_request->item.un.family != prx_address_family_proxy &&
            browse_request->item.un.family != prx_address_family_inet &&
            browse_request->item.un.family != prx_address_family_inet6)
            port = 0;
        else
            port = browse_request->item.un.ip.port;

        result = pal_scan_net(port, browse_request->flags & io_browse_response_cache_only ?
            pal_scan_cache_only : 0, prx_browse_session_handle_scan_response, stream,
            &stream->scan);
        if (result != er_ok)
            break;
        return;
    }
    while (0);

    log_error(session->log, "Failed scanning network (%s)", prx_err_string(result));
    prx_browse_session_send_error_response(session, &browse_request->handle, result);
    if (stream)
        prx_browse_session_close_request(session, &stream->handle);
}

//
// Called for each file
//
int32_t prx_browse_session_handle_dirpath_response(
    void* context,
    int32_t error,
    const char* name,
    prx_file_info_t* stat
)
{
    io_browse_response_t browse_response;
    prx_property_t prop;
    prx_browse_stream_t* stream = (prx_browse_stream_t*)context;

    dbg_assert_ptr(context);

    memset(&browse_response, 0, sizeof(io_browse_response_t));
    browse_response.item.un.family = prx_address_family_proxy;
    io_ref_copy(&stream->handle, &browse_response.handle);

    if (error != er_ok)
    {
        browse_response.flags |= io_browse_response_eos;
        if (error != er_nomore)
            browse_response.error_code = error;
    }

    if (!name)
    {
        browse_response.flags |= io_browse_response_empty;
        browse_response.flags |= io_browse_response_allfornow;
    }
    else
    {
        browse_response.item.un.proxy.host_dyn = name;
        if (stat)
        {
            // Append file info
            browse_response.props_size = 1;
            browse_response.props = &prop;
            prop.type = prx_property_type_file_info;
            memcpy(&prop.property.file_info, stat, sizeof(prx_file_info_t));
        }
    }

    prx_browse_session_send_response(stream->session, &browse_response);
    return er_ok;
}

//
// Create or update directory path browsing session
//
static void prx_browse_session_handle_dirpath_request(
    prx_browse_session_t* session,
    io_browse_request_t* browse_request
)
{
    int32_t result;
    prx_browse_stream_t* stream = NULL;
    const char* query;

    dbg_assert_ptr(session);
    dbg_assert_ptr(browse_request);
    dbg_assert_is_task(session->scheduler);
    do
    {
        if (!session->server->allow_fs_browse)
        {
            // file system browsing is not supported, close reqeust
            prx_browse_session_handle_unknown_request(session, browse_request);
            return;
        }

        // Parse and validate host string
        if (browse_request->item.un.family != prx_address_family_proxy)
        {
            // Must have proxy
            log_error(session->log, "Service browse item must be a proxy address");
            result = er_invalid_format;
            break;
        }

        // Now get or create the stream for this handle
        result = prx_browse_session_get_or_create_request(session,
            &browse_request->handle, &stream);
        if (result != er_ok)
            break;

        query = prx_socket_address_proxy_get_host(&browse_request->item.un.proxy);
        log_trace(session->log, "Dir query: %s...", query);
        result = pal_read_dir(query, prx_browse_session_handle_dirpath_response, stream);
        if (result != er_ok)
        {
            // Send end of stream
            prx_browse_session_handle_dirpath_response(stream, er_nomore, NULL, NULL);
        }
        return;
    }
    while (0);

    log_error(session->log, "Failed reading folder (%s)", prx_err_string(result));
    prx_browse_session_send_error_response(session, &browse_request->handle, result);
    if (stream)
        prx_browse_session_close_request(session, &stream->handle);
}

//
// Callback for browse results - called from sd browser thread!
//
static int32_t prx_browse_session_handle_service_response(
    void *context,
    int32_t itf_index,
    int32_t error,
    pal_sd_result_type_t type,
    void *browse_result,
    int32_t flags
)
{
    int32_t result;
    io_browse_response_t browse_response;
    prx_browse_stream_t* stream = (prx_browse_stream_t*)context;
    pal_sd_browse_result_t* rec;
    pal_sd_service_entry_t* res;
    prx_addrinfo_t* ainfo;
    prx_property_t prop;
    char* full_name = NULL;
    size_t full_name_len;

    dbg_assert_ptr(stream);
    dbg_assert_ptr(stream->session);
    (void)itf_index;

    // Clear timeout countdown we previously scheduled since we got results
    prx_scheduler_clear(stream->session->scheduler,
        (prx_task_t)prx_browse_stream_timeout, stream);

    memset(&browse_response, 0, sizeof(io_browse_response_t));
    browse_response.item.un.family = prx_address_family_proxy;
    io_ref_copy(&stream->handle, &browse_response.handle);

    if (!browse_result)
    {
        browse_response.flags |= io_browse_response_empty;
    }
    else if (type == pal_sd_result_entry)
    {
        res = (pal_sd_service_entry_t*)browse_result;

        dbg_assert_ptr(res->addr);
        dbg_assert(res->addr->family == prx_address_family_proxy,
            "Expected proxy address from pal.");
        dbg_assert(res->records_len == 0 || res->records,
            "Expected %d records but got NULL ptr", res->records_len);

        memcpy(&browse_response.item.un.proxy, res->addr,
            sizeof(prx_socket_address_proxy_t));
        browse_response.props_size = res->records_len;
        browse_response.props = res->records;
    }
    else if (type == pal_sd_result_addrinfo)
    {
        ainfo = (prx_addrinfo_t*)browse_result;

        browse_response.item = ainfo->address;
        prop.type = prx_property_type_addrinfo;

        memcpy(&prop.property.addr_info, ainfo, sizeof(prx_addrinfo_t));
        browse_response.props_size = 1;
        browse_response.props = &prop;
    }
    else
    {
        rec = (pal_sd_browse_result_t*)browse_result;
        full_name_len =
            (rec->service_name ? strlen(rec->service_name) : 0) + 1 +
            (rec->service_type ? strlen(rec->service_type) : 0) + 1 +
            (rec->domain ? strlen(rec->domain) : 8) + 1;
        full_name = (char*)mem_zalloc(full_name_len);
        if (!full_name)
            result = er_out_of_memory;
        else
        {
            result = string_copy_service_full_name(rec->service_name,
                rec->service_type, rec->domain, full_name, full_name_len);
            browse_response.item.un.proxy.host_dyn =
                result == er_ok ? full_name : NULL;
        }

        if (result != er_ok && error == er_ok)
        {
            log_error(stream->session->log,
                "Failed to make full name from supposedly good results (%s).",
                    prx_err_string(result));
            error = result;
        }
    }

    // Send response
    if (0 != (flags & pal_sd_result_removed))
        browse_response.flags |= io_browse_response_removed;
    if (0 != (flags & pal_sd_result_all_for_now))
        browse_response.flags |= io_browse_response_allfornow;

    browse_response.error_code = error;
    prx_browse_session_send_response(stream->session, &browse_response);

    if (full_name)
        mem_free(full_name);

    // Schedule timeout to send all for now if not already done so...
    if (0 == (flags & pal_sd_result_all_for_now))
    {
        __do_later_s(stream->session->scheduler, prx_browse_stream_timeout,
            stream, STREAM_TIMEOUT);
    }
    return er_ok;
}

//
// Create or update address resolver session
//
static void prx_browse_session_handle_resolve_request(
    prx_browse_session_t* session,
    io_browse_request_t* browse_request
)
{
    int32_t result;
    prx_browse_stream_t* stream = NULL;

    dbg_assert_ptr(session);
    dbg_assert_ptr(browse_request);
    dbg_assert_is_task(session->scheduler);
    do
    {
        if (!session->server->sdclient)
        {
            // sd client is not supported, close reqeust
            prx_browse_session_handle_unknown_request(session, browse_request);
            return;
        }

        // Parse and validate host string
        if (browse_request->item.un.family != prx_address_family_proxy)
        {
            // Must have proxy
            log_error(session->log, "Service browse item must be a proxy address");
            result = er_invalid_format;
            break;
        }

        // Now get or create the stream for this handle
        result = prx_browse_session_get_or_create_request(session,
            &browse_request->handle, &stream);
        if (result != er_ok)
            break;
        if (!stream->sdbrowser)
        {
            // First stream, lazily create sd browser in this stream
            result = pal_sdbrowser_create(session->server->sdclient,
                prx_browse_session_handle_service_response, stream,
                &stream->sdbrowser);
            if (result != er_ok)
                break;

            //
            // Save the sdclient used to create browser in this session. This allows
            // us later to only clean up the browse sessions related to the client
            // should the client fail.
            //
            session->owner = session->server->sdclient;
        }
        else
        {
            log_trace(session->log, "Using existing browse stream to resolve...");
        }

        // Resolve with specified query
        log_trace(session->log, "Resolving %s...",
            prx_socket_address_proxy_get_host(&browse_request->item.un.proxy));
        result = pal_sdbrowser_resolve(stream->sdbrowser, &browse_request->item.un.proxy,
            prx_itf_index_all /*browse_request->item.un.proxy.itf_index*/);
        if (result != er_ok)
            break;

        // Make sure to send an all for now for this stream after timeout has passed
        __do_later_s(session->scheduler, prx_browse_stream_timeout,
            stream, STREAM_TIMEOUT);
        return;
    }
    while (0);

    log_error(session->log, "Failed creating browse stream (%s)", prx_err_string(result));
    prx_browse_session_send_error_response(session, &browse_request->handle, result);
    if (stream)
        prx_browse_session_close_request(session, &stream->handle);
}

//
// Create or update service browsing session
//
static void prx_browse_session_handle_service_request(
    prx_browse_session_t* session,
    io_browse_request_t* browse_request
)
{
    int32_t result;
    char* service_name;
    char* service_type;
    char* domain;
    prx_browse_stream_t* stream = NULL;
    char* query = NULL;

    dbg_assert_ptr(session);
    dbg_assert_ptr(browse_request);
    dbg_assert_is_task(session->scheduler);
    do
    {
        if (!session->server->sdclient)
        {
            // sd client is not supported, close reqeust
            prx_browse_session_handle_unknown_request(session, browse_request);
            return;
        }

        // Parse and validate host string
        if (browse_request->item.un.family != prx_address_family_proxy)
        {
            // Must have proxy
            log_error(session->log, "Service browse item must be a proxy address");
            result = er_invalid_format;
            break;
        }

        result = string_clone(
            prx_socket_address_proxy_get_host(&browse_request->item.un.proxy), &query);
        if (result != er_ok)
        {
            log_error(session->log, "Failed to clone request host name string (%s)",
                prx_err_string(result));
            break;
        }

        result = string_parse_service_full_name(query, &service_name, &service_type,
            &domain);
        if (result != er_ok)
        {
            log_error(session->log, "Failed to parse service query string %s (%s)",
                query, prx_err_string(result));
            break;
        }

        // Now get or create the stream for this handle
        result = prx_browse_session_get_or_create_request(session,
            &browse_request->handle, &stream);
        if (result != er_ok)
            break;
        if (!stream->sdbrowser)
        {
            // First stream, lazily create sd browser in this stream
            result = pal_sdbrowser_create(session->server->sdclient,
                prx_browse_session_handle_service_response, stream,
                &stream->sdbrowser);
            if (result != er_ok)
                break;
        }
        else
        {
            log_trace(session->log, "Using existing browse stream to browse...");
        }

        // Browse with specified query
        log_trace(session->log, "Service query: %s...", query);
        result = pal_sdbrowser_browse(stream->sdbrowser, service_name, service_type,
            domain, prx_itf_index_all /*browse_request->item.un.proxy.itf_index*/);
        if (result != er_ok)
            break;

        // Make sure to send an all for now for this stream after timeout has passed
        __do_later_s(session->scheduler, prx_browse_stream_timeout,
            stream, STREAM_TIMEOUT);
        mem_free(query);
        return;
    }
    while (0);

    log_error(session->log, "Failed creating browse stream (%s)", prx_err_string(result));
    prx_browse_session_send_error_response(session, &browse_request->handle, result);
    if (stream)
        prx_browse_session_close_request(session, &stream->handle);
    if (query)
        mem_free(query);
}

//
// Process browse request
//
static void prx_browse_session_process_request(
    prx_browse_session_t* browser,
    io_browse_request_t* browse_request
)
{
    dbg_assert_ptr(browser);
    dbg_assert_ptr(browse_request);
    dbg_assert_is_task(browser->scheduler);

    switch (browse_request->type)
    {
    case io_browse_request_cancel:
        prx_browse_session_close_request(browser, &browse_request->handle);
        break;
    case io_browse_request_resolve:
        prx_browse_session_handle_resolve_request(browser, browse_request);
        break;
    case io_browse_request_service:
        prx_browse_session_handle_service_request(browser, browse_request);
        break;
    case io_browse_request_dirpath:
        prx_browse_session_handle_dirpath_request(browser, browse_request);
        break;
    case io_browse_request_ipscan:
         prx_browse_session_handle_ipscan_request(browser, browse_request);
        break;
    case io_browse_request_portscan:
       prx_browse_session_handle_portscan_request(browser, browse_request);
        break;
    default:
        prx_browse_session_handle_unknown_request(browser, browse_request);
        break;
    }
}

//
// Decode browse request from stream
//
static void prx_browse_session_decode_and_process_request(
    prx_browse_session_t* session,
    io_stream_t* stream
)
{
    int32_t result;
    io_codec_ctx_t ctx, obj;
    io_browse_request_t browse_request;
    bool is_null;

    dbg_assert_ptr(session);
    dbg_assert_ptr(stream);
    dbg_assert_is_task(session->scheduler);

    result = io_codec_ctx_init(session->codec, &ctx, stream, true,
        session->log);
    if (result != er_ok)
    {
        log_error(session->log,
            "Failed to initialize codec from stream (%s)",
            prx_err_string(result));
        return;
    }
    do
    {
        // Decode json buffer into protocol message
        result = io_decode_object(&ctx, NULL, &is_null, &obj);
        if (result != er_ok)
        {
            log_error(session->log, "Failed to decode object (%s)",
                prx_err_string(result));
            break;
        }

        if (is_null) // Must never be null...
            result = er_invalid_format;
        else
            result = io_decode_browse_request(&obj, &browse_request);
        if (result != er_ok)
        {
            log_error(session->log, "Failed to decode request (%s)",
                prx_err_string(result));
            break;
        }

        // Handle request
        prx_browse_session_process_request(session, &browse_request);
        break;
    }
    while (0);
    (void)io_codec_ctx_fini(&ctx, stream, false);
}

//
// Decode and handle all buffers in inbound queue
//
static void prx_browse_session_receive_buffers(
    prx_browse_session_t* session
)
{
    io_queue_buffer_t* buffer;

    dbg_assert_ptr(session);
    dbg_assert_is_task(session->scheduler);

    if (session->closed)
        return;

    while (true)
    {
        buffer = io_queue_pop_done(session->inbound);
        if (!buffer)
            break;

        // Decode and process request
        prx_browse_session_decode_and_process_request(session,
            io_queue_buffer_as_stream(buffer));

        // release buffer
        io_queue_buffer_release(buffer);
    }
}

//
// Socket closed event
//
static void prx_browse_session_socket_closed(
    prx_browse_session_t* session
)
{
    dbg_assert_ptr(session);
    dbg_assert_ptr(session->sock);
    dbg_assert_is_task(session->scheduler);

    // Finished closing - call again to destroy session
    session->destroy = session->closed = true;
    prx_browse_session_free(session);
}

//
// Socket opened event
//
static void prx_browse_session_socket_opened(
    prx_browse_session_t* session
)
{
    dbg_assert_ptr(session);
    dbg_assert_ptr(session->sock);
    dbg_assert_is_task(session->scheduler);

    if (session->last_error != er_ok)
    {
        log_error(session->log, "Failure during open of socket (%s)",
            prx_err_string(session->last_error));
        prx_browse_session_socket_closed(session);
        return;
    }

    // Session socket is opened - start receiving
    session->destroy = session->closed = false;
    session->last_error = pal_socket_can_recv(session->sock, true);
}

//
// Begin receive operation - called from pal thread
//
static void prx_browse_session_on_begin_receive(
    prx_browse_session_t* session,
    uint8_t** buf,
    size_t* length
)
{
    int32_t result;
    io_queue_buffer_t* buffer = NULL;

    dbg_assert_ptr(session);
    dbg_assert_ptr(buf);
    dbg_assert_ptr(length);

    // Make a buffer and receive into it.
    result = io_queue_create_buffer(session->inbound, NULL,
        MAX_MESSAGE_SIZE, &buffer);
    if (result != er_ok)
    {
        *buf = NULL;
        *length = 0;
    }
    else
    {
        *buf = io_queue_buffer_to_ptr(buffer);
        *length = MAX_MESSAGE_SIZE;
    }
}

//
// Complete receiving operation - called from pal thread
//
static void prx_browse_session_on_end_receive(
    prx_browse_session_t* session,
    uint8_t** buf,
    size_t* length,
    int32_t* flags,
    int32_t result
)
{
    io_queue_buffer_t* buffer;

    dbg_assert_ptr(session);
    dbg_assert_ptr(buf);
    dbg_assert_ptr(length);

    (void)flags;  // Flags are irrelevant to us

    buffer = io_queue_buffer_from_ptr(*buf);
    dbg_assert_ptr(buffer);

    if (result != er_ok &&
        result != er_aborted &&
        result != er_retry &&
        result != er_closed)
    {
        log_error(session->log, "Error during pal receive (%s).",
            prx_err_string(result));
    }

    if (result != er_ok)
    {
        // Short cut, just release
        io_queue_buffer_release(buffer);
        if (result == er_closed)
        {
            // Remote side closed, make sure we close the session
            __do_next(session, prx_browse_session_free);
        }
    }
    else
    {
        // Mark done and decode/handle on scheduler thread
        buffer->code = result;
        buffer->length = buffer->write_offset = *length;
        buffer->read_offset = 0;
        io_queue_buffer_set_done(buffer);
        __do_next(session, prx_browse_session_receive_buffers);
    }
}

//
// Request a buffer to be send - called from pal thread
//
static void prx_browse_session_on_begin_send(
    prx_browse_session_t* session,
    uint8_t** buf,
    size_t* length,
    int32_t* flags
)
{
    io_queue_buffer_t* buffer = NULL;

    dbg_assert_ptr(session);
    dbg_assert_ptr(buf);
    dbg_assert_ptr(length);
    dbg_assert_ptr(flags);

    // Grab a encoded buffer from queue encode and send
    buffer = io_queue_pop_inprogress(session->outbound);
    if (!buffer)
        buffer = io_queue_pop_ready(session->outbound);
    if (!buffer)
    {
        *buf = NULL;
        *length = 0;
        *flags = 0;
    }
    else
    {
        *buf = io_queue_buffer_to_ptr(buffer);
        *length = buffer->write_offset;
        *flags = 0;
    }
}

//
// Complete send operation - called from pal thread
//
static void prx_browse_session_on_end_send(
    prx_browse_session_t* session,
    uint8_t** buf,
    size_t* length,
    int32_t result
)
{
    io_queue_buffer_t* buffer;

    dbg_assert_ptr(session);
    dbg_assert_ptr(length);
    dbg_assert_ptr(*buf);

    if (result != er_ok &&
        result != er_aborted &&
        result != er_retry &&
        result != er_closed)
    {
        log_error(session->log, "Error during pal socket send (%s).",
            prx_err_string(result));
    }

    buffer = io_queue_buffer_from_ptr(*buf);
    dbg_assert_ptr(buffer);

    if (result == er_retry)
    {
        // Retry - put into in progress
        io_queue_buffer_set_inprogress(buffer);
        return;
    }

    dbg_assert(buffer->write_offset == *length || result != er_ok, "Not all sent");
    io_queue_buffer_release(buffer);

    if (result == er_closed)
    {
        // Remote side closed, make sure we close the session
        __do_next(session, prx_browse_session_free);
    }
}

//
// Event callback - called from pal thread
//
static void prx_browse_session_socket_event_handler(
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
    prx_browse_session_t* session = (prx_browse_session_t*)context;

    dbg_assert(!addr, "no address expected.");
    (void)op_context;
    (void)addr;

    switch (ev)
    {
    case pal_socket_event_opened:
        dbg_assert(buffer && size && *size == sizeof(pal_socket_t*),
            "Socket expected.");
        // Connected - set socket and add session
        session->sock = (pal_socket_t*)*buffer;
        session->last_error = error;
        __do_next(session, prx_browse_session_socket_opened);
        break;
    case pal_socket_event_begin_recv:
        dbg_assert(error == er_ok, "no error expected.");
        prx_browse_session_on_begin_receive(session, buffer, size);
        break;
    case pal_socket_event_end_recv:
        prx_browse_session_on_end_receive(session, buffer, size, flags, error);
        break;
    case pal_socket_event_begin_send:
        dbg_assert(error == er_ok, "no error expected.");
        prx_browse_session_on_begin_send(session, buffer, size, flags);
        break;
    case pal_socket_event_end_send:
        prx_browse_session_on_end_send(session, buffer, size, error);
        break;
    case pal_socket_event_closed:
        dbg_assert(buffer && size && *size == sizeof(pal_socket_t*),
            "Socket expected.");
        session->last_error = error;
        __do_next(session, prx_browse_session_socket_closed);
        break;
    case pal_socket_event_begin_accept:
    case pal_socket_event_end_accept:
    default:
        dbg_assert(0, "Should not be here!");
    }
}

//
// Called from different thread - indicates error in client.
//
static void prx_browse_server_sdclient_error(
    void* context,
    int32_t error
);

//
// Reconnect client - this could fail, or could result in another error...
//
static void prx_browse_server_sdclient_reconnect(
    prx_browse_server_t* server
)
{
    int32_t result;

    dbg_assert_ptr(server);
    dbg_assert(!server->sdclient, "Reconnect, but client exists");
    dbg_assert_is_task(server->scheduler);

    result = pal_sdclient_create(prx_browse_server_sdclient_error,
        server, &server->sdclient);
    if (result != er_ok)
    {
        log_error(server->log,
            "Failed to create sd client (%s), retrying later... ",
            prx_err_string(result));
        __do_later(server, prx_browse_server_sdclient_reconnect, 30000);
    }
}

//
// Cancel all active browse sessions, close client and reconnect.
//
static void prx_browse_server_sdclient_reset(
    prx_browse_server_t* server
)
{
    prx_browse_session_t* next;
    pal_sdclient_t* sdclient;

    dbg_assert_ptr(server);
    dbg_assert_ptr(server->sdclient);
    dbg_assert_is_task(server->scheduler);

    // Detach client. Access is safe since we are on process thread.
    sdclient = server->sdclient;
    server->sdclient = NULL;

    // Close all active sd browsing sessions currently known to us.
    lock_enter(server->sessions_lock);
    if (!DList_IsListEmpty(&server->sessions))
    {
        next = containingRecord(server->sessions.Flink,
            prx_browse_session_t, link);
        if (next->owner == sdclient)
        {
            next->owner = NULL;
            __do_next(next, prx_browse_session_free);
        }
    }
    lock_exit(server->sessions_lock);

    // Free client and recreate/reconnect - clear out all queued resets...
    pal_sdclient_free(sdclient);
    prx_scheduler_clear(server->scheduler,
        (prx_task_t)prx_browse_server_sdclient_reset, server);

    __do_later(server, prx_browse_server_sdclient_reconnect, 3000);
}

//
// Called from different thread - indicates error in client.
//
static void prx_browse_server_sdclient_error(
    void* context,
    int32_t error
)
{
    prx_browse_server_t* server = (prx_browse_server_t*)context;
    dbg_assert_ptr(server);
    (void)error;
    log_error(server->log, "Error in sdclient (%s), reset all active sessions... ",
        prx_err_string(error));
    __do_next(server, prx_browse_server_sdclient_reset);
}

//
// Free browse server and release all associated resources
//
void prx_browse_server_free(
    prx_browse_server_t* server
)
{
    if (!server)
        return;
    // Free all sessions still open...
    if (server->sessions_lock)
        lock_enter(server->sessions_lock);
    server->destroy = true;
    if (!DList_IsListEmpty(&server->sessions))
    {
        // Free session first, it will schedule a free again...
        __do_next(containingRecord(server->sessions.Flink,
            prx_browse_session_t, link), prx_browse_session_free);
        lock_exit(server->sessions_lock);
        return; // Wait for session to close
    }
    if (server->sessions_lock)
        lock_exit(server->sessions_lock);

    // Release scheduler and all associated server tasks
    if (server->scheduler)
        prx_scheduler_release(server->scheduler, server);

    if (server->sdclient)
        pal_sdclient_free(server->sdclient);
    if (server->sessions_lock)
        lock_free(server->sessions_lock);
    mem_free_type(prx_browse_server_t, server);
}

//
// Create session instance and return client interface
//
int32_t prx_browse_server_accept(
    prx_browse_server_t* server,
    io_codec_id_t codec_id,
    pal_socket_client_itf_t* itf
)
{
    int32_t result;
    prx_browse_session_t* session;

    chk_arg_fault_return(server);
    chk_arg_fault_return(itf);

    session = (prx_browse_session_t*)mem_zalloc_type(prx_browse_session_t);
    if (!session)
        return er_out_of_memory;
    do
    {
        session->log = log_get("browse.session");
        DList_InitializeListHead(&session->link);
        session->scheduler = server->scheduler;
        session->last_error = er_ok;
        session->closed = session->destroy = true;
        session->codec = io_codec_by_id(codec_id);
        if (!session->codec)
        {
            result = er_not_supported;
            break;
        }

        session->requests = create_hashtable(10,
            (unsigned int(*) (void*)) io_ref_hash,
            (int(*) (void*, void*))  io_ref_equals);
        if (!session->requests)
        {
            result = er_out_of_memory;
            break;
        }

        result = prx_scheduler_create(server->scheduler,
            &session->scheduler);
        if (result != er_ok)
            break;

        result = io_queue_create("session-outbound", &session->outbound);
        if (result != er_ok)
            break;
        result = io_queue_create("session-inbound", &session->inbound);
        if (result != er_ok)
            break;

        memset(itf, 0, sizeof(pal_socket_client_itf_t));
        itf->props.family =
            prx_address_family_unspec;
        itf->props.proto_type =
            prx_protocol_type_unspecified;
        itf->props.sock_type =
            prx_socket_type_dgram;
        itf->cb =
            prx_browse_session_socket_event_handler;
        itf->context =
            session;

        lock_enter(server->sessions_lock);
        if (server->destroy)
        {
            lock_exit(server->sessions_lock);
            result = er_bad_state;
            break;
        }
        DList_InsertTailList(&server->sessions, &session->link);
        session->server = server;
        lock_exit(server->sessions_lock);
        return er_ok;
    }
    while (0);
    prx_browse_session_free(session);
    return result;
}

//
// Create browse server instance
//
int32_t prx_browse_server_create(
    prx_scheduler_t* scheduler,
    prx_browse_server_t** created
)
{
    int32_t result;
    prx_browse_server_t* server;

    chk_arg_fault_return(created);

    server = (prx_browse_server_t*)mem_zalloc_type(prx_browse_server_t);
    if (!server)
        return er_out_of_memory;
    do
    {
        server->log = log_get("browse.server");
        DList_InitializeListHead(&server->sessions);

        result = lock_create(&server->sessions_lock);
        if (result != er_ok)
            break;

        result = prx_scheduler_create(scheduler, &server->scheduler);
        if (result != er_ok)
            break;

        if (pal_caps() & pal_cap_file)
        {
            if (__prx_config_get_int(prx_config_key_browse_fs, 0))
                server->allow_fs_browse = true;
        }

        if (pal_caps() & pal_cap_dnssd)
        {
            __do_next(server, prx_browse_server_sdclient_reconnect);
        }

        if (pal_caps() & pal_cap_scan)
        {
            server->supports_scan = true;
        }

        *created = server;
        return er_ok;
    }
    while (0);
    prx_browse_server_free(server);
    return result;
}


