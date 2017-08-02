// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "prx_log.h"
#include "pal.h"
#include "pal_sk.h"
#include "pal_net.h"
#include "util_string.h"
#include "util_signal.h"
#include <stdio.h>


typedef struct scan_ctx
{
    pal_socket_client_itf_t client;
    uint32_t upper_bound;
    pal_socket_t* sock;
    signal_t* signal;
    log_t log;
}
scan_ctx_t;

//
// Callback
//
void test_scan_cb(
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
    int32_t result;
    scan_ctx_t* scanner = (scan_ctx_t*)context;

    (void)context;
    (void)buffer;
    (void)size;
    (void)addr;
    (void)flags;
    (void)op_context;

    switch (ev)
    {
    case pal_socket_event_opened:
        dbg_assert(buffer && size && *size == sizeof(pal_socket_t*) &&
            (pal_socket_t*)*buffer == scanner->sock, "Socket expected.");
        pal_socket_close(scanner->sock);
        if (error == er_ok)
        {
            log_info(scanner->log, "Found: %d.%d.%d.%d:%d",
                scanner->client.props.address.un.ip.un.in4.un.u8[0],
                scanner->client.props.address.un.ip.un.in4.un.u8[1],
                scanner->client.props.address.un.ip.un.in4.un.u8[2],
                scanner->client.props.address.un.ip.un.in4.un.u8[3],
                scanner->client.props.address.un.ip.port);
        }
        else
        {
            log_error(scanner->log, "Error opening (%s)", prx_err_string(error));
            signal_set(scanner->signal);
        }
        break;
    case pal_socket_event_closed:
        pal_socket_free(scanner->sock);
        scanner->client.props.address.un.ip.un.in4.un.addr++;
        if (scanner->client.props.address.un.ip.un.in4.un.addr ==
            scanner->upper_bound)
        {
            log_info(scanner->log, "Done.");
            signal_set(scanner->signal);
            break;
        }
        log_info(scanner->log, "Trying: %d.%d.%d.%d:%d",
            scanner->client.props.address.un.ip.un.in4.un.u8[0],
            scanner->client.props.address.un.ip.un.in4.un.u8[1],
            scanner->client.props.address.un.ip.un.in4.un.u8[2],
            scanner->client.props.address.un.ip.un.in4.un.u8[3],
            scanner->client.props.address.un.ip.port);
        result = pal_socket_create(&scanner->client, &scanner->sock);
        if (result != er_ok)
        {
            log_error(scanner->log, "Error creating (%s)",
                prx_err_string(result));
            signal_set(scanner->signal);
            break;
        }
        result = pal_socket_open(scanner->sock, NULL);
        if (result != er_ok)
        {
            log_error(scanner->log, "Error opening socket (%s)",
                prx_err_string(result));
            signal_set(scanner->signal);
            break;
        }
        break;
    case pal_socket_event_begin_recv:
    case pal_socket_event_end_recv:
    case pal_socket_event_begin_accept:
    case pal_socket_event_end_accept:
    case pal_socket_event_begin_send:
    case pal_socket_event_end_send:
    default:
        dbg_assert(0, "Should not be here!");
    }
}

//
// Subnet scan test utility
//
int main_scan(int argc, char *argv[])
{
    int32_t result;
    const char* name = NULL, *port = NULL;
    prx_ifaddrinfo_t* info = NULL;
    size_t info_count;
    scan_ctx_t ctx, *scanner = &ctx;
    uint32_t mask;

    if (argc <= 1)
        return er_arg;


    while (argc > 1)
    {
        argv++;
        argc--;

        /**/ if (!port)
            port = argv[0];
        else if (!name)
            name = argv[0];
    }

    if (!port)
        port = "4840";

    result = pal_init();
    if (result != er_ok)
        return result;
    do
    {
        memset(scanner, 0, sizeof(scan_ctx_t));
        scanner->log = log_get("test.scan");
        scanner->client.cb = test_scan_cb;
        scanner->client.context = scanner;
        scanner->client.props.sock_type = prx_socket_type_stream;
        scanner->client.props.proto_type = prx_protocol_type_tcp;
        scanner->client.props.timeout = 5000;

        result = signal_create(true, false, &scanner->signal);
        if (result != er_ok)
            break;

        scanner->client.props.family = prx_address_family_inet;
        scanner->client.props.address.un.ip.family = prx_address_family_inet;
        scanner->client.props.address.un.ip.port = (uint16_t)atoi(port);

        result = pal_getifaddrinfo(name, 0, &info, &info_count);
        if (result != er_ok)
        {
            log_error(scanner->log, "Failure to get interface info.");
            break;
        }
        for (size_t i = 0; i < info_count; i++)
        {
            if (!scanner->client.props.address.un.ip.un.in4.un.addr &&
                info[i].address.un.family == prx_address_family_inet)
            {
                mask = (~0 << info[i].prefix);
                scanner->client.props.address.un.ip.un.in4.un.addr =
                    info[i].address.un.ip.un.in4.un.addr & ~mask;
                scanner->upper_bound =
                    info[i].address.un.ip.un.in4.un.addr | mask;

                log_info(scanner->log, "Scanning %s: %d.%d.%d.%d:%d",
                    info[i].name,
                    scanner->client.props.address.un.ip.un.in4.un.u8[0],
                    scanner->client.props.address.un.ip.un.in4.un.u8[1],
                    scanner->client.props.address.un.ip.un.in4.un.u8[2],
                    scanner->client.props.address.un.ip.un.in4.un.u8[3],
                    scanner->client.props.address.un.ip.port);
            }
            else
            {
                log_info(scanner->log, "Skipping %s...");
            }
        }

        if (!scanner->client.props.address.un.ip.un.in4.un.addr)
        {
            scanner->client.props.address.un.ip.un.in4.un.addr = 1;
            scanner->upper_bound = (uint32_t)~0;
        }
        result = pal_socket_create(&scanner->client, &scanner->sock);
        if (result != er_ok)
        {
            log_error(scanner->log, "Error creating first socket (%s)",
                prx_err_string(result));
            break;
        }
        result = pal_socket_open(scanner->sock, NULL);
        if (result != er_ok)
        {
            log_error(scanner->log, "Error opening first socket (%s)",
                prx_err_string(result));
            break;
        }

        signal_wait(scanner->signal, -1);
    }
    while (0);

    if (info)
        pal_freeifaddrinfo(info);
    signal_free(scanner->signal);
    pal_deinit();
    return result;
}
