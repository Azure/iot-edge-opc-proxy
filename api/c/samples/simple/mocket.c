// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "api_host.h"
#include "api_errors.h"
#include "pal_proc.h"

//
// Handlers for send and receive
//
typedef int32_t(*send_func_t)(
    fd_t sock,
    const void* context
    );

typedef bool (*recv_func_t)(
    char* buf,
    int buf_len,
    const void* context
    );

#define DEFAULT_BUFLEN 0x400

//
// Run as client, connect to server
//
int socket_connect_send_receive_close(
    unsigned short port,
    send_func_t send_func,
    recv_func_t recv_func,
    const void* context
    )
{
    fd_t ConnectSocket = _invalid_fd;
    struct addrinfo *address = NULL, *ptr = NULL, hints;
    char buf[DEFAULT_BUFLEN];
    int buflen = DEFAULT_BUFLEN;
    int result;
    char host_name[128] = "mocket_exe";
    int max_recv = 6000; 
    do
    {
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        // Resolve the server address and port
#ifdef _WIN32
        _itoa(port, buf, 10);
#else
        sprintf(buf, "%d", port);
#endif
        gethostname(host_name, 128);
        
        result = getaddrinfo(host_name, buf, &hints, &address);
        if (result != 0) 
        {
            printf("getaddrinfo failed with error: %d\n", result);
            break;
        }

        // Attempt to connect to an address until one succeeds
        for (ptr = address; ptr != NULL; ptr = ptr->ai_next)
        {
            // Create a SOCKET for connecting to server
            ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
                ptr->ai_protocol);
            if (ConnectSocket == -1) {
                printf("socket failed with error: %d\n", errno);
                break;
            }

            // Connect to server.
            result = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
            if (result == -1) 
            {
#ifdef _WIN32
                closesocket(ConnectSocket);
#else
                close(ConnectSocket);
#endif
                ConnectSocket = _invalid_fd;
                continue;
            }
            break;
        }

        freeaddrinfo(address);

        if (ConnectSocket == _invalid_fd) 
        {
            printf("Unable to connect to server!\n");
            break;
        }

        if (send_func)
        {
            if (send_func(ConnectSocket, context) < 0)
            {
                printf("Unable to send data to server!\n");
                break;
            }

            // shutdown the SEND since no more data will be sent
            result = shutdown(ConnectSocket, SHUT_WR);
            if (result == -1)
            {
                printf("shutdown failed with error: %d\n", errno);
                break;
            }
        }

        // Receive until the peer closes the connection
        do 
        {
            result = recv(ConnectSocket, buf, buflen, 0);
            if (result > 0)
            {
                if (recv_func && !recv_func(buf, result, context))
                    break;
            }
            else if (result == 0)
                printf("Connection closed, %d left\n", max_recv);
            else
                printf("recv failed with error: %d, %d left\n", errno, max_recv);
        } 
        while (result > 0 && --max_recv > 0);

        // cleanup
        result = 0;
    }
    while(0);

    if (ConnectSocket != _invalid_fd)
#ifndef _WIN32
        close(ConnectSocket);
#else
        closesocket(ConnectSocket);
#endif
    return result;
}

//
// Test printer, prints string to console
//
bool test_printer(
    char* buffer,
    int buf_len,
    const void* context
)
{
    (void)buf_len, context;

    (void)buffer;
    printf("RECEIVED: %s\n", buffer);
    return true;
}

//
// Port 19: Chargen http://tools.ietf.org/html/rfc864
//
int test_chargen(
    void
)
{
    // Good for backpressure testing as 19 will flood the pipe with chars
    return socket_connect_send_receive_close(19, NULL, test_printer, NULL);
}

//
// Port 17: quotd http://tools.ietf.org/html/rfc865
//
int test_quotd(
    void
)
{
    // Prints quote of the day
    return socket_connect_send_receive_close(17, NULL, test_printer, NULL);
}

//
// Port 13: daytime http://tools.ietf.org/html/rfc867
//
int test_daytime(
    void
)
{
    // Prints day and time
    return socket_connect_send_receive_close(13, NULL, test_printer, NULL);
}

//
// Writes a string to echo
//
int test_echo_send_func(
    fd_t sock,
    const void* context
)
{
    const char* string_to_echo = (char*)context;
    return send(sock, string_to_echo, (int)strlen(string_to_echo), 0);
}

//
// Reads a string to echo
//
bool test_echo_recv_func(
    char* buffer,
    int buf_len,
    const void* context
)
{
    const char* string_to_echo = (const char*)context;
    buffer[buf_len] = 0;

    printf("ECHO received: %s", buffer);

    if (0 == strcmp(buffer, string_to_echo))
        printf("    SUCCESS!");
    else
        printf("    FAIL!");

    return false;
}

//
// Port 7:  Echo http://tools.ietf.org/html/rfc862
//
int test_echo(
    const char* string_to_echo
)
{
    return socket_connect_send_receive_close(
        7, test_echo_send_func, test_echo_recv_func, string_to_echo);
}


#define PORT 8914

//
// Send a series of pings to server to echo back
//
int test_client_send_func(
    fd_t sock,
    const void* context
)
{
    int result;
    int sent = 0;
    char buf[16];
    (void)context;
    for (int i = 0; i < 100; i++)
    {
        sprintf(buf, "Ping %d", i);

        // Send an initial buffer
        result = send(sock, buf, (int)strlen(buf), 0);
        if (result == -1) {
            printf("send failed with error: %d\n", errno);
            return result;
        }

        printf("Bytes Sent: %d\n", result);
        sent += result;
    }
    return sent;
}

//
// Test client for test server (-s)
//
int test_client(
    void
)
{
    return socket_connect_send_receive_close(
        PORT, test_client_send_func, test_printer, NULL);
}

//
// Test server for test client (-c)
//
int test_server(
    void
)
{
    int result = 1;
    fd_t ListenSocket = _invalid_fd;
    fd_t AcceptSocket = _invalid_fd;
    struct sockaddr_in service;
    char buf[DEFAULT_BUFLEN];
    int buflen = DEFAULT_BUFLEN;

    do
    {
        ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (ListenSocket == _invalid_fd) 
        {
            printf("socket failed with error: %d\n", errno);
            break;
        }

        service.sin_family = AF_INET;
        service.sin_addr.s_addr = INADDR_LOOPBACK;
        service.sin_port = htons(PORT);

        result = bind(ListenSocket, (const struct sockaddr *)&service, sizeof(service));
        if (result < 0)
        {
            printf("bind failed with error: %d\n", errno);
            break;
        }

        result = listen(ListenSocket, 1);
        if (result < 0)
        {
            printf("listen failed with error: %d\n", errno);
            break;
        }

        while (true)
        {
            printf("Waiting for client to connect...\n");

            AcceptSocket = accept(ListenSocket, NULL, NULL);
            if (AcceptSocket == _invalid_fd)
            {
                printf("accept failed with error: %d\n", errno);
                break;
            }

            printf("Client connected.\n");

            // Echo back until the peer closes the connection
            do
            {
                result = recv(AcceptSocket, buf, buflen, 0);
                if (result > 0)
                {
                    printf("Bytes received: %d\n", result);
                    result = send(AcceptSocket, buf, buflen, 0);
                    if (result >= 0)
                        printf("Bytes sent: %d\n", result);
                    else
                        printf("send failed with error: %d\n", errno);
                }
                else if (result == 0)
                    printf("Connection closed\n");
                else
                    printf("recv failed with error: %d\n", errno);
            } while (result > 0);

#ifndef _WIN32
            close(AcceptSocket);
#else
            closesocket(AcceptSocket);
#endif
        }
        result = 0;

    } while (0);

    if (ListenSocket != _invalid_fd)
#ifndef _WIN32
        close(ListenSocket);
#else
        closesocket(ListenSocket);
#endif

    return 0;
}

//
// Run proxy server
//
int proxy(
    void
)
{
    int32_t result;
    host_t* host;

    //
    // Ensure a ns config is created for interface mocket_exe
    //
    const char* proxy_config = "mocket-host.json";
    const char* cmd_line[3] = { "", "-i", NULL };

    char host_name[128] = "mocket_exe";
    gethostname(host_name, 128);
    cmd_line[2] = host_name;

    result = host_console(proxy_config, 5, (char**)cmd_line);
    if (result != er_ok)
    {
        // Failed to set up host
    }

    result = host_init();
    if (result != er_ok)
        return result;
    do
    {
        result = host_create(proxy_config, proxy_type_server, &host);
        if (result != er_ok)
            break;

        result = host_start(host);
        if (result != er_ok)
            break;

        result = host_sig_wait(host);

        result = host_stop(host);
        if (result != er_ok)
            break;

    } while (0);

    if (host)
        host_release(host);
    host_deinit();
    return 0;
}


//
// Setup name service service configuration 
//
int test_setup(
    void
)
{
#ifdef _WIN32
    WSADATA wsaData;
#endif
    int32_t result;
    const char* hub_cs;

    hub_cs = getenv("_HUB_CS");

#ifdef _WIN32
    if (!hub_cs)
    {
        // This would be what we call when loading hub
        // connection string from ns.hub.json ...
        result = WSAStartup(MAKEWORD(2, 2), &wsaData)
    }
    else
    {
        // ... but we use a hook for testing purposes
        wsaData.lpVendorInfo = hub_cs;
        result = WSAStartup(0xc5, &wsaData);
    }

    if (result != 0)
    {
        printf("WSAStartup failed with error: %d\n", result);
    }
#else
    extern void so_init2(const char* config);
    result = so_init2(hub_cs);
#endif
    return result;
}

//
// Cleanup test configuration
//
int test_teardown(
    void
)
{
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}

//
// Uses the main process thread to perform the work
//
int main(int argc, char *argv[])
{
    int32_t result;
    process_t* proc = NULL;
    const char* cmd_line_proxy = "proxy";

#if defined(_MSC_VER)
    // _CrtSetDbgFlag(_CRTDBG_FLAGS);
#endif

    if (argc > 1 && 0 == strcmp(argv[1], cmd_line_proxy))
    {
        result = proxy();
    }
    else
    {
        if (argc > 1 && 0 == strcmp(argv[1], "-x"))
        {
            argc--;
            argv++;
        }
        else
        {
            // Spawn proxy process
            result = pal_process_spawn(argv[0], 1, &cmd_line_proxy, &proc);
            if (result != er_ok)
                return result;
        }

        do
        {
            // Setup test
            result = test_setup();
            if (result != er_ok)
                break;

#ifdef LOOP
            for (int i = 0; i < LOOP; i++)
            {
#endif
                /**/ if (argc == 1 ||               // No arg defaults to client
                         0 == strcmp(argv[1], "client"))
                    result = test_client();
                else if (0 == strcmp(argv[1], "server"))
                    result = test_server();
                else if (0 == strcmp(argv[1], "quotd"))
                    result = test_quotd();
                else if (0 == strcmp(argv[1], "chargen"))
                    result = test_chargen();
                else if (0 == strcmp(argv[1], "daytime"))
                    result = test_daytime();
                else if (0 == strcmp(argv[1], "echo"))
                    result = test_echo(argv[argc > 2 ? 2 : 1]);
                else
                    goto help;
#ifdef LOOP
            }
#endif
        } while (0);

        test_teardown();

        if (proc)
        {
            pal_process_kill(proc);
            pal_process_yield(proc);
        }
    }
    return result;

help:
    printf(" Usage: mocket.exe <command> <args>                   \n\n");
    printf(" Commands:                                              \n");
    printf("  client  Run the mocket test client against the ...    \n");
    printf("  server  ... Test server                               \n");
    printf("  quotd   Print quote of the day from quotd service     \n");
    printf("  chargen Print characters generated through chargen    \n");
    printf("  daytime Print day and time from daytime service       \n");
    printf("  echo    Echo the proceeding string through echo server\n");
    printf("  proxy   Run a proxy instance (DO NOT USE)             \n");
    return -1;
}
