// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "prx_err.h"
#include "prx_host.h"
#include "util_mem.h"
#include "util_signal.h"
#if _WIN32
#include "windows.h"
#include "wincon.h"
#else
#include <signal.h>
#endif
#include <stdio.h>

typedef enum service_status
{
    service_status_init,
    service_status_stopped,
    service_status_starting,
    service_status_running,
    service_status_stopping,
    service_status_error,
    service_status_deinit
}
service_status_t;

typedef void(*on_status_change_t)(
    service_status_t,
    int32_t result
);

//
// Called on termination 
//
#if _WIN32
static int32_t signal_handler(
#else
static void signal_handler(
#endif
    int32_t signal
)
{
    int32_t result;
    prx_host_t* host;
#if _WIN32
    if (signal != CTRL_C_EVENT)
        return 0;
#endif
    result = prx_host_get(&host);
    if (result == er_ok)
    {
        prx_host_sig_break(host);
        prx_host_release(host);
    }
#if _WIN32
    return 1;
#endif
}

//
// Dummy callback
//
static void service_status_cb_dummy(
    service_status_t state,
    int32_t result
)
{
    (void)result;
    switch (state)
    {
    case service_status_init:
        printf("\n=== Azure " MODULE_NAME " " MODULE_VERSION " ===\n\n");
        break;
    case service_status_deinit:
        printf("Proxy exits... Goodbye!\n");
        break;
    case service_status_stopped:
    case service_status_starting:
    case service_status_running:
    case service_status_stopping:
    case service_status_error:
        break;
    default:
        break;
    }
}

//
// Main program thread
//
static int32_t service_main(
    int32_t argc,
    char *argv[],
    on_status_change_t on_status
)
{
    int32_t result;
    prx_host_t* host = NULL;
    if (!on_status)
        on_status = service_status_cb_dummy;

    on_status(service_status_init, 0);
    result = prx_host_init(proxy_type_server, argc, argv);
    if (result != er_ok)
    {
        on_status(service_status_error, result);
        return result;
    }
    do
    {
        result = prx_host_get(&host);
        if (result != er_ok)
            break;

        on_status(service_status_starting, 0);
        result = prx_host_start(host);
        if (result != er_ok)
            break;

        on_status(service_status_running, 0);
        result = prx_host_sig_wait(host);
        on_status(service_status_stopping, 0);

        result = prx_host_stop(host);
        if (result != er_ok)
            break;

        on_status(service_status_stopped, 0);
    } while (0);

    if (result != er_ok)
        on_status(service_status_error, result);

    if (host)
        prx_host_release(host);

    on_status(service_status_deinit, 0);
    return 0;
}

#ifdef _WIN32

static SERVICE_STATUS_HANDLE service_status_handle = NULL;

//
// Callback when service status changes
//
static void service_status_cb(
    service_status_t state,
    int32_t result
)
{
    static SERVICE_STATUS service_status;
    static DWORD counter = 1;

    if (result != er_ok)
    {
        service_status.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
        service_status.dwServiceSpecificExitCode = result;
    }

    switch (state)
    {
    case service_status_init:
    case service_status_deinit:
        memset(&service_status, 0, sizeof(SERVICE_STATUS));
        service_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
        service_status.dwWaitHint = 0;
        // Fall through
    case service_status_stopped:
        service_status.dwCheckPoint = 0;
        service_status.dwCurrentState = SERVICE_STOPPED;
        service_status.dwControlsAccepted =
            SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
        break;
    case service_status_starting:
        service_status.dwCheckPoint = counter++;
        service_status.dwCurrentState = SERVICE_START_PENDING;
        service_status.dwControlsAccepted = 0;
        service_status.dwWaitHint = 4000;
        break;
    case service_status_running:
        service_status.dwWaitHint = 0;
        service_status.dwCheckPoint = 0;
        service_status.dwCurrentState = SERVICE_RUNNING;
        service_status.dwControlsAccepted =
            SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
        break;
    case service_status_stopping:
        service_status.dwCheckPoint = counter++;
        service_status.dwCurrentState = SERVICE_STOP_PENDING; 
        break;
    case service_status_error:
        break;
    default:
        return;
    }

    if (!SetServiceStatus(service_status_handle, &service_status))
        printf("SetServiceStatus failed with errror code 0x%08X.\r\n", GetLastError());
}

//
// Entry point for scm to dispatch service control messages
//
DWORD WINAPI ServiceControl(
    DWORD control
)
{
    DWORD result = NO_ERROR;
    switch (control)
    {
    case SERVICE_CONTROL_INTERROGATE:
        break;
    case SERVICE_CONTROL_SHUTDOWN:
    case SERVICE_CONTROL_STOP:
        signal_handler(CTRL_C_EVENT);
        break;
    default:
        result = ERROR_CALL_NOT_IMPLEMENTED;
        break;
    }
    return result;
}

//
// Main service entry point
//
void _ext__ WINAPI ServiceMain(
    DWORD argc,
    LPSTR *argv
)
{
    service_status_handle = RegisterServiceCtrlHandlerA("proxyd", ServiceControl);
    if (service_status_handle == NULL)
    {
        printf("RegisterServiceCtrlHandler failed with error code 0x%08X.\r\n",
            GetLastError());
        return;
    }
    service_main((int32_t)argc, argv, service_status_cb);
}

#endif // _WIN32

//
// Main program thread
//
int32_t main(
    int32_t argc,
    char *argv[]
)
{
    int32_t result;
#if _WIN32
    // If called by SCM, then register entry point and start dispatcher
    SERVICE_TABLE_ENTRYA service_table[2] = { { "proxyd", ServiceMain }, 0 };
    if (StartServiceCtrlDispatcherA(service_table))
    {
        // Success, running as daemon
        return 0;
    }
#endif
    mem_init();
#if _WIN32
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)signal_handler, TRUE);
#else
    signal(SIGINT, signal_handler);
#endif
    result = service_main(argc, argv, NULL);
    mem_deinit();
    return result;
}
