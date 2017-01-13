// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "prx_host.h"
#include "prx_module.h"
#include "prx_sched.h"
#include "prx_server.h"
#include "io_host.h"
#include "io_transport.h"

#include "pal.h"
#include "pal_file.h"
#include "pal_net.h"  // for pal_gethostname

#include "util_signal.h"
#include "util_string.h"

#include "azure_c_shared_utility/doublylinkedlist.h"
#include "azure_c_shared_utility/refcount.h"

#include <stdio.h>
#include "getopt.h"

const char* k_ns_local_json_file = "ns.local.json";
const char* k_ns_hub_json_file = "ns.hub.json";

//
// Server running in or outside host process
//
typedef struct prx_host_module
{
    prx_ns_entry_t* entry;
    prx_module_t* module;
    DLIST_ENTRY link;
}
prx_host_module_t;

//
// Host process instance structure
//
struct prx_host
{
    prx_host_type_t type;
    bool hidden;                    // Whether the host should be run hidden
    io_ref_t id;                                           // Unique host id
    prx_ns_t* local;           // Local registry, either from file or memory
    prx_ns_t* remote;                    // Remote registry instance or null
    bool uninstall_on_exit;      // Whether to uninstall all modules on exit
    prx_scheduler_t* scheduler;           // Host specific scheduler or null
    DLIST_ENTRY modules;                  // List of modules managed by host
    bool running;
    signal_t* exit_signal;                // Allows waiting for host to exit
    prx_module_api_t* module;      // entry points to manage gateway modules
    log_t log;
};

DEFINE_REFCOUNT_TYPE(prx_host_t);

//
// Global host when initialized
//
static prx_host_t* process_host = NULL;

//
// Install
//
static int32_t prx_host_install_server(
    prx_host_t* host,
    const char* name
)
{
    int32_t result;
    prx_ns_entry_t* entry = NULL;
    prx_ns_result_t* resultset = NULL;

    dbg_assert_ptr(host);
    dbg_assert_ptr(host->remote);
    dbg_assert_ptr(host->local);

    while (true)
    {
        result = prx_ns_get_entry_by_name(host->remote, name, &resultset);
        if (result == er_not_found)
        {
            result = prx_ns_entry_create(
                prx_ns_entry_type_proxy, name, name, &entry);
            if (result != er_ok)
                break;
            result = prx_ns_create_entry(host->remote, entry);
            prx_ns_entry_release(entry);
            entry = NULL;
            if (result != er_ok)
                break;
            continue;
        }

        if (result != er_ok)
            break;
        entry = prx_ns_result_pop(resultset);
        while (entry)
        {
            if (prx_ns_entry_get_type(entry) & prx_ns_entry_type_proxy)
            {
                result = prx_ns_create_entry(host->local, entry);
                if (result != er_ok)
                    break;
            }
            prx_ns_entry_release(entry);
            entry = prx_ns_result_pop(resultset);
        }
        break;
    }

    if (entry)
        prx_ns_entry_release(entry);
    if (resultset)
        prx_ns_result_release(resultset);
    return result;
}

//
// Uninstall server
//
static int32_t prx_host_uninstall_server(
    prx_host_t* host,
    const char* name
)
{
    int32_t result;
    prx_ns_entry_t* entry = NULL;
    prx_ns_result_t* resultset = NULL;

    dbg_assert_ptr(host);
    dbg_assert_ptr(host->remote);
    dbg_assert_ptr(host->local);

    do
    {
        result = prx_ns_get_entry_by_name(host->local, name, &resultset);
        if (result != er_ok)
            break;

        entry = prx_ns_result_pop(resultset);
        while (entry)
        {
            if (prx_ns_entry_get_type(entry) & prx_ns_entry_type_proxy)
            {
                result = prx_ns_remove_entry(host->local, entry);
                if (result != er_ok)
                    break;
                result = prx_ns_remove_entry(host->remote, entry);
                if (result != er_ok)
                {
                    (void)prx_ns_create_entry(host->local, entry);
                    break;
                }
            }
            prx_ns_entry_release(entry);
            entry = prx_ns_result_pop(resultset);
        }
    } while (0);

    if (entry)
        prx_ns_entry_release(entry);
    if (resultset)
        prx_ns_result_release(resultset);
    return result;
}

//
// Initialize host object from passed command line arguments
//
static int32_t prx_host_init_from_command_line(
    prx_host_t* host,
    int32_t argc,
    char *argv[]
)
{
    int32_t result;

    int c;
    int option_index = 0;

    io_cs_t *cs = NULL;
    const char *server_name = NULL;
    const char *log_config = NULL;
    const char *ns_registry = NULL;
    char buffer[128];
    bool is_install = false;
    bool is_uninstall = false;
    bool should_exit = false;
    bool is_adhoc = false;
    io_ref_t adhoc;

    static struct option long_options[] =
    {
        { "ad-hoc",                     no_argument,            NULL, 'a' },
        { "exit",                       no_argument,            NULL, 'x' },
        { "hidden",                     no_argument,            NULL, 'h' },
        { "install",                    no_argument,            NULL, 'i' },
        { "uninstall",                  no_argument,            NULL, 'u' },
        { "name",                       required_argument,      NULL, 'n' },
        { "connection-string",          required_argument,      NULL, 'c' },
        { "connection-string-file",     required_argument,      NULL, 'C' },
        { "log-config-file",            required_argument,      NULL, 'L' },
        { "ns-db-file",                 required_argument,      NULL, 'd' },
        { 0,                            0,                      NULL,  0  }
    };

    do
    {
        result = er_ok;
        // Parse options
        while (result == er_ok)
        {
            c = getopt_long(
                argc, argv, "axhiuc:n:L:C:d:", long_options, &option_index);
            if (c == -1)
                break;

            switch (c)
            {
            case 'x':
                should_exit = true;
                break;
            case 'h':
                host->hidden = true;
                break;
            case 'i':
                is_install = true;
                break;
            case 'u':
                is_uninstall = true;
                break;
            case 'a':
                is_adhoc = true;
                result = io_ref_new(&adhoc);
                if (result != er_ok)
                    printf("Failed making ad-hoc id for proxy server. \n\n");
                break;
            case 'n':
                server_name = optarg;
                if (!server_name)
                {
                    printf("Missing arg for -name option. \n\n");
                    result = er_arg;
                }
                break;
            case 'c':
                result = io_cs_create_from_string(optarg, &cs);
                if (result != er_ok)
                    printf("Malformed --connection-string argument. \n\n");
                break;
            case 'd':
                ns_registry = optarg;
                if (!ns_registry)
                {
                    printf("Missing arg for --ns-db-file option. \n\n");
                    result = er_arg;
                }
                break;
            case 'C':
                result = prx_ns_iot_hub_create(optarg, &host->remote);
                if (result != er_ok)
                    printf("Failed to load iot hub registry from "
                        "--connection-string-file '%s' arg. \n\n", 
                            optarg ? optarg : "");
                break;
            case 'L':
                log_config = optarg;
                if (!log_config)
                {
                    printf("Missing arg for --log-config-file option. \n\n");
                    result = er_arg;
                }
                break;
            default:
                printf("Unrecognized option %c\n\n", (char)c);
                // Fall through
            case '?':
                result = er_arg;
                break;
            }
        }

        if (result != er_ok)
            break;

        if ((is_install && is_uninstall) ||
            (is_adhoc && is_install) ||
            (is_adhoc && is_uninstall))
        {
            printf("Cannot use --install and --uninstall and --adhoc together...");
            result = er_arg;
            break;
        }

        // Configure logging
        log_config = pal_create_full_path(log_config ? log_config : "log.config");
        if (!log_config)
        {
            result = er_out_of_memory;
        }
        else
        {
            // Configure logging
            result = log_configure(log_config);
            if (result != er_ok)
            {
                printf("Logging config file %s not found. \r\n", log_config);
            }
        
            pal_free_path(log_config);
            log_config = NULL;
        }

        // Load registry from file or create in memory one if no file specified
        result = prx_ns_generic_create(ns_registry, &host->local);
        if (result != er_ok)
        {
            printf("Failed to create local registry. \n\n");
            break;
        }

        if (!host->remote)
        {
            // Ensure we have the secrets to create the remote registry
            if (!cs)
            {
                (void)io_cs_create_from_string(getenv("_HUB_CS"), &cs);
            }

            // If we want to install or uninstall we will need secrets
            if (!cs && (is_install || is_uninstall))
            {
                printf("Missing --connection-string option. \n\n");
                result = er_arg;
                break;
            }

            if (cs)
            {
                result = prx_ns_iot_hub_create_from_cs(cs, &host->remote);
                if (result != er_ok)
                {
                    printf("Failed to create iothub registry from "
                        "--connection-string supplied connection string. \n\n");
                    break;
                }
            }
        }

        if (is_install || is_uninstall || is_adhoc)
        {
            // Ensure we have a name
            if (!server_name)
            {
                if (!is_adhoc)
                    result = pal_gethostname(buffer, _countof(buffer));
                else
                    result = io_ref_to_string(&adhoc, buffer, _countof(buffer));
                if (result != er_ok)
                    break;
                server_name = buffer;
            }

            // Run install/uninstall
            /**/ if (is_install || is_adhoc)
                result = prx_host_install_server(host, server_name);
            else if (is_uninstall)
                result = prx_host_uninstall_server(host, server_name);

            if (result != er_ok)
            {
                printf("FAILURE %s: %s failed! Check parameters...",
                    prx_err_string(result), !is_uninstall ? "Install" : "Uninstall");
                break;
            }

            printf("Proxy %s %s\n", server_name, !is_uninstall ? 
                "installed" : "uninstalled");
            if (is_adhoc)
                host->uninstall_on_exit = true;
        }

        break;
    } 
    while (0);

    if (cs)
        io_cs_free(cs);

    if (result != er_arg)
        return result;
    if (should_exit)
        return er_aborted;

printf(" Proxy command line options:                                       \n\n");
printf(" --log-config-file,-L                                                \n");
printf("                    Log configuration file to use. Defaults to       \n");
printf("                    ./log.config.                                    \n");
printf(" --ns-db-file,-d                                                     \n");
printf("                    Local name service database file to use.         \n");
printf("                    If not provided keeps ns in memory.              \n");
printf(" --connection-string,-c <connection-string>                          \n");
printf("                    iothubowner connection string for install or     \n");
printf("                    uninstall. Required for -i, -u, or -a            \n");
printf(" --connection-string-file,-C <file-name>                             \n");
printf("                    same as above but read from file.                \n");
printf(" --name,-n <server-name>                                             \n");
printf("                    Name of servers to install or uninstall.         \n");
printf(" --install,-i                                                        \n");
printf("                    Installs a proxy server in the IoT Hub device    \n");
printf("                    registry and creates a local database entry.     \n");
printf("                    Requires -c or -C.                               \n");
printf("                    If -n is not specified, uses hostname.           \n");
printf(" --uninstall,-u                                                      \n");
printf("                    Uninstalls proxy server on Iot Hub and removes   \n");
printf("                    the entry from the local database file.          \n");
printf("                    Requires -c or -C.                               \n");
printf("                    If -n is not specified, uses hostname.           \n");
printf(" --ad-hoc,-a                                                         \n");
printf("                    Installs a proxy server under a random name on   \n");
printf("                    startup and removes it on clean(!) exit.         \n");
printf("                    Requires -c or -C.                               \n");
printf(" --exit,-x                                                           \n");
printf("                    Performs console tasks and then exits, otherwise \n");
printf("                    runs proxy host process as console process.      \n");
printf(" --hidden,-h                                                         \n");
printf("                    Runs the proxy as a service/daemon process.      \n");
    return er_arg;
}

//
// Stop all running modules/servers
//
static void prx_host_modules_stop(
    prx_host_t* host
)
{
    int32_t result;
    dbg_assert_ptr(host);
    prx_host_module_t* next;
    
    while (!DList_IsListEmpty(&host->modules))
    {
        next = containingRecord(
            DList_RemoveHeadList(&host->modules), prx_host_module_t, link);

        // This will release, but async cleanup tasks will still run on scheduler
        host->module->on_destroy(next->module);

        // Remove this entry if it was ad-hoc created
        if (host->uninstall_on_exit)
        {
            result = prx_host_uninstall_server(
                host, prx_ns_entry_get_name(next->entry));
            if (result != er_ok)
            {
                log_error(host->log, "Failed uninstalling server %s", "");
            }
        }

        if (next->entry)
            prx_ns_entry_release(next->entry);

        mem_free_type(prx_host_module_t, next);
    }
}

//
// Load all startup modules/servers 
//
static int32_t prx_host_modules_start(
    prx_host_t* host
)
{
    int32_t result;
    prx_ns_result_t* startup;
    prx_ns_entry_t* proxy_entry;
    prx_host_module_t* module;
    
    dbg_assert_ptr(host);
    dbg_assert_ptr(host->local);

    result = prx_ns_get_entry_by_type(host->local, 
        prx_ns_entry_type_startup, &startup);
    if (result == er_not_found)
        return er_ok;
    if (result != er_ok)
        return result;
    do 
    {
        proxy_entry = prx_ns_result_pop(startup);
        if (!proxy_entry)
            break; // Done

        module = mem_zalloc_type(prx_host_module_t);
        if (!module)
        {
            prx_ns_entry_release(proxy_entry);
            result = er_out_of_memory;
            break;
        }

        DList_InsertTailList(&host->modules, &module->link);
        module->entry = proxy_entry;
        module->module = host->module->on_create(
            NULL, module->entry);

        if (!module->module)
        {
            result = er_out_of_memory;
            break;
        }
    }
    while (result == er_ok);
    
    if (startup)
        prx_ns_result_release(startup);
    return result;
}

//
// Start allm server modules if any
//
int32_t prx_host_start(
    prx_host_t* host
)
{
    int32_t result;

    if (!host)
        return er_fault;
    do
    {
        host->running = true;

        // Start socket servers
        result = prx_host_modules_start(host);
        if (result != er_ok)
            break;

        log_info(host->log, "Host server started!\n\n");
        return er_ok;

    } while (0);

    prx_host_stop(host);
    return result;
}

//
// Stop host
//
int32_t prx_host_stop(
    prx_host_t* host
)
{
    if (!host)
        return er_fault;
    if (!host->running)
        return er_ok;

    prx_host_modules_stop(host);
    host->running = false;
    return er_ok;
}

//
// Block until stopping
//
int32_t prx_host_sig_wait(
    prx_host_t* host
)
{
    if (!host)
        return er_fault;
    return signal_wait(host->exit_signal, ~0);
}

//
// Break execution
//
int32_t prx_host_sig_break(
    prx_host_t* host
)
{
    if (!host)
        return er_fault;
    if (!host->exit_signal)
        return er_bad_state;
    return signal_set(host->exit_signal);
}

// 
// Returns a reference to the host
//
int32_t prx_host_get(
    prx_host_t** cloned
)
{
    int32_t result;
    if (!cloned)
        return er_fault;
    if (!process_host)
    {
        // Lazy create
        result = prx_host_init(proxy_type_server, 0, NULL);
        if (result != er_ok)
            return result;
    }

    INC_REF(prx_host_t, process_host);
    *cloned = process_host;
    return er_ok;
}

//
// Returns the host address
//
io_ref_t* prx_host_get_id(
    prx_host_t* host
)
{
    if (!host)
        return NULL;
    return &host->id;
}

//
// returns the name service 
//
prx_ns_t* prx_host_get_ns(
    prx_host_t* host
)
{
    if (!host)
        return NULL;
    if (host->remote)
        return host->remote;
    return host->local;
}

//
// Returns null if multiple schedulers should be used
//
prx_scheduler_t* prx_host_get_scheduler(
    prx_host_t* host
)
{
    if (!host)
        return NULL;
    return host->scheduler;
}

//
// Destroy the host
//
void prx_host_release(
    prx_host_t* host
)
{
    if (!host)
        return;

    if (DEC_REF(prx_host_t, host) == DEC_RETURN_ZERO)
    {
        prx_host_stop(host);

        if (host->remote)
            prx_ns_close(host->remote);
        if (host->local)
            prx_ns_close(host->local);

        if (host->scheduler)
        {
            prx_scheduler_release(host->scheduler, host);

            // Wait for all cleanup tasks still in scheduler to complete
            prx_scheduler_at_exit(host->scheduler);
        }

        if (host->exit_signal)
            signal_free(host->exit_signal);

        pal_deinit();

        log_deinit();

        dbg_assert(host == process_host, "Unexpected");
        process_host = NULL;
        REFCOUNT_TYPE_FREE(prx_host_t, host);
    }
}

//
// Init host
//
int32_t prx_host_init(
    prx_host_type_t type,
    int32_t argc,
    char *argv[]
)
{
    int32_t result;

    if (process_host)  // only one host per process
        return er_bad_state;

    process_host = REFCOUNT_TYPE_CREATE(prx_host_t);
    if (!process_host)
        return er_out_of_memory;
    do
    {
        memset(process_host, 0, sizeof(prx_host_t));
        DList_InitializeListHead(&process_host->modules);

        // Init logging and pal
        result = log_init();
        if (result != er_ok)
        {
            printf("Failed to init logging!");
            break;
        }

        process_host->log = log_get("host");
        process_host->type = type;
        process_host->module = Module_GetAPIS(0);
        dbg_assert_ptr(process_host->module);

        result = pal_init();
        if (result != er_ok)
        {
            log_error(process_host->log, "Failed to initialize pal (%s)",
                prx_err_string(result));
            break;
        }

        // Init from console
        result = prx_host_init_from_command_line(process_host, argc, argv);
        if (result == er_aborted)
        {
            // exit
            result = er_ok;
            break;
        }
        else if (result != er_ok)
            break;

        // Assign a random id to the host
        result = io_ref_new(&process_host->id);
        if (result != er_ok)
            break;

        result = signal_create(true, false, &process_host->exit_signal);
        if (result != er_ok)
            break;

        result = prx_scheduler_create(NULL, &process_host->scheduler);
        if (result != er_ok)
            break;

        // Host created
        log_info(process_host->log, "Proxy host created!");
        DEC_REF(prx_host_t, process_host); // weak reference
        return er_ok;
            
    } while (0);

    prx_host_release(process_host);
    return result;
}

//
// Returns host trusted certs to validate certs against
//
const char* trusted_certs(
    void
)
{
    return
    /* Baltimore */
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIDdzCCAl+gAwIBAgIEAgAAuTANBgkqhkiG9w0BAQUFADBaMQswCQYDVQQGEwJJ\r\n"
    "RTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJlclRydXN0MSIwIAYD\r\n"
    "VQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTAwMDUxMjE4NDYwMFoX\r\n"
    "DTI1MDUxMjIzNTkwMFowWjELMAkGA1UEBhMCSUUxEjAQBgNVBAoTCUJhbHRpbW9y\r\n"
    "ZTETMBEGA1UECxMKQ3liZXJUcnVzdDEiMCAGA1UEAxMZQmFsdGltb3JlIEN5YmVy\r\n"
    "VHJ1c3QgUm9vdDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKMEuyKr\r\n"
    "mD1X6CZymrV51Cni4eiVgLGw41uOKymaZN+hXe2wCQVt2yguzmKiYv60iNoS6zjr\r\n"
    "IZ3AQSsBUnuId9Mcj8e6uYi1agnnc+gRQKfRzMpijS3ljwumUNKoUMMo6vWrJYeK\r\n"
    "mpYcqWe4PwzV9/lSEy/CG9VwcPCPwBLKBsua4dnKM3p31vjsufFoREJIE9LAwqSu\r\n"
    "XmD+tqYF/LTdB1kC1FkYmGP1pWPgkAx9XbIGevOF6uvUA65ehD5f/xXtabz5OTZy\r\n"
    "dc93Uk3zyZAsuT3lySNTPx8kmCFcB5kpvcY67Oduhjprl3RjM71oGDHweI12v/ye\r\n"
    "jl0qhqdNkNwnGjkCAwEAAaNFMEMwHQYDVR0OBBYEFOWdWTCCR1jMrPoIVDaGezq1\r\n"
    "BE3wMBIGA1UdEwEB/wQIMAYBAf8CAQMwDgYDVR0PAQH/BAQDAgEGMA0GCSqGSIb3\r\n"
    "DQEBBQUAA4IBAQCFDF2O5G9RaEIFoN27TyclhAO992T9Ldcw46QQF+vaKSm2eT92\r\n"
    "9hkTI7gQCvlYpNRhcL0EYWoSihfVCr3FvDB81ukMJY2GQE/szKN+OMY3EU/t3Wgx\r\n"
    "jkzSswF07r51XgdIGn9w/xZchMB5hbgF/X++ZRGjD8ACtPhSNzkE1akxehi/oCr0\r\n"
    "Epn3o0WC4zxe9Z2etciefC7IpJ5OCBRLbf1wbWsaY71k5h+3zvDyny67G7fyUIhz\r\n"
    "ksLi4xaNmjICq44Y3ekQEe5+NauQrz4wlHrQMz2nZQ/1/I6eYs9HRCwBXbsdtTLS\r\n"
    "R9I4LtD+gdwyah617jzV/OeBHRnDJELqYzmp\r\n"
    "-----END CERTIFICATE-----\r\n"
    /* MSIT */
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIFhjCCBG6gAwIBAgIEByeaqTANBgkqhkiG9w0BAQsFADBaMQswCQYDVQQGEwJJ\r\n"
    "RTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJlclRydXN0MSIwIAYD\r\n"
    "VQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTEzMTIxOTIwMDczMloX\r\n"
    "DTE3MTIxOTIwMDY1NVowgYsxCzAJBgNVBAYTAlVTMRMwEQYDVQQIEwpXYXNoaW5n\r\n"
    "dG9uMRAwDgYDVQQHEwdSZWRtb25kMR4wHAYDVQQKExVNaWNyb3NvZnQgQ29ycG9y\r\n"
    "YXRpb24xFTATBgNVBAsTDE1pY3Jvc29mdCBJVDEeMBwGA1UEAxMVTWljcm9zb2Z0\r\n"
    "IElUIFNTTCBTSEEyMIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEA0eg3\r\n"
    "p3aKcEsZ8CA3CSQ3f+r7eOYFumqtTicN/HJq2WwhxGQRlXMQClwle4hslAT9x9uu\r\n"
    "e9xKCLM+FvHQrdswbdcaHlK1PfBHGQPifaa9VxM/VOo6o7F3/ELwY0lqkYAuMEnA\r\n"
    "iusrr/466wddBvfp/YQOkb0JICnobl0JzhXT5+/bUOtE7xhXqwQdvDH593sqE8/R\r\n"
    "PVGvG8W1e+ew/FO7mudj3kEztkckaV24Rqf/ravfT3p4JSchJjTKAm43UfDtWBpg\r\n"
    "lPbEk9jdMCQl1xzrGZQ1XZOyrqopg3PEdFkFUmed2mdROQU6NuryHnYrFK7sPfkU\r\n"
    "mYsHbrznDFberL6u23UykJ5jvXS/4ArK+DSWZ4TN0UI4eMeZtgzOtg/pG8v0Wb4R\r\n"
    "DsssMsj6gylkeTyLS/AydGzzk7iWa11XWmjBzAx5ihne9UkCXgiAAYkMMs3S1pbV\r\n"
    "S6Dz7L+r9H2zobl82k7X5besufIlXwHLjJaoKK7BM1r2PwiQ3Ov/OdgmyBKdHJqq\r\n"
    "qcAWjobtZ1KWAH8Nkj092XA25epCbx+uleVbXfjQOsfU3neG0PyeTuLiuKloNwnE\r\n"
    "OeOFuInzH263bR9KLxgJb95KAY8Uybem7qdjnzOkVHxCg2i4pd+/7LkaXRM72a1o\r\n"
    "/SAKVZEhZPnXEwGgCF1ZiRtEr6SsxwUQ+kFKqPsCAwEAAaOCASAwggEcMBIGA1Ud\r\n"
    "EwEB/wQIMAYBAf8CAQAwUwYDVR0gBEwwSjBIBgkrBgEEAbE+AQAwOzA5BggrBgEF\r\n"
    "BQcCARYtaHR0cDovL2N5YmVydHJ1c3Qub21uaXJvb3QuY29tL3JlcG9zaXRvcnku\r\n"
    "Y2ZtMA4GA1UdDwEB/wQEAwIBhjAdBgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUH\r\n"
    "AwIwHwYDVR0jBBgwFoAU5Z1ZMIJHWMys+ghUNoZ7OrUETfAwQgYDVR0fBDswOTA3\r\n"
    "oDWgM4YxaHR0cDovL2NkcDEucHVibGljLXRydXN0LmNvbS9DUkwvT21uaXJvb3Qy\r\n"
    "MDI1LmNybDAdBgNVHQ4EFgQUUa8kJpz0aCJXgCYrO0ZiFXsezKUwDQYJKoZIhvcN\r\n"
    "AQELBQADggEBAHaFxSMxH7Rz6qC8pe3fRUNqf2kgG4Cy+xzdqn+I0zFBNvf7+2ut\r\n"
    "mIx4H50RZzrNS+yovJ0VGcQ7C6eTzuj8nVvoH8tWrnZDK8cTUXdBqGZMX6fR16p1\r\n"
    "xRspTMn0baFeoYWTFsLLO6sUfUT92iUphir+YyDK0gvCNBW7r1t/iuCq7UWm6nnb\r\n"
    "2DVmVEPeNzPR5ODNV8pxsH3pFndk6FmXudUu0bSR2ndx80oPSNI0mWCVN6wfAc0Q\r\n"
    "negqpSDHUJuzbEl4K1iSZIm4lTaoNKrwQdKVWiRUl01uBcSVrcR6ozn7eQaKm6ZP\r\n"
    "2SL6RE4288kPpjnngLJev7050UblVUfbvG4=\r\n"
    "-----END CERTIFICATE-----\r\n"
    /* *.azure-devices.net */
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIGcjCCBFqgAwIBAgITWgABtrNbz7vBeV0QWwABAAG2szANBgkqhkiG9w0BAQsF\r\n"
    "ADCBizELMAkGA1UEBhMCVVMxEzARBgNVBAgTCldhc2hpbmd0b24xEDAOBgNVBAcT\r\n"
    "B1JlZG1vbmQxHjAcBgNVBAoTFU1pY3Jvc29mdCBDb3Jwb3JhdGlvbjEVMBMGA1UE\r\n"
    "CxMMTWljcm9zb2Z0IElUMR4wHAYDVQQDExVNaWNyb3NvZnQgSVQgU1NMIFNIQTIw\r\n"
    "HhcNMTUwODI3MDMxODA0WhcNMTcwODI2MDMxODA0WjAeMRwwGgYDVQQDDBMqLmF6\r\n"
    "dXJlLWRldmljZXMubmV0MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA\r\n"
    "nXC/qBUdlnfIm5K3HYu0o/Mb5tNNcsr0xy4Do0Puwq2W1tz0ZHvIIS9VOANhkNCb\r\n"
    "VyOncnP6dvmM/rYYKth/NQ8RUiZOYlROZ0SYC8cvxq9WOln4GXtEU8vNVqJbYrJj\r\n"
    "rPMHfxqLzTE/0ZnQffnDT3iMUE9kFLHow0YgaSRU0KZsc9KAROmzBzu+QIB1WGKX\r\n"
    "D7CN361tG1UuN68Bz7MSnbgk98Z+DjDxfusoDhiiy/Y9MLOJMt4WIy5BqL3lfLnn\r\n"
    "r+JLqmpiFuyVUDacFQDprYJ1/AFgcsKYu/ydmASARPzqJhOGaC2sZP0U5oBOoBzI\r\n"
    "bz4tfn8Bi0kJKmS53mQt+wIDAQABo4ICOTCCAjUwCwYDVR0PBAQDAgSwMB0GA1Ud\r\n"
    "JQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjAdBgNVHQ4EFgQUKpYehBSNA53Oxivn\r\n"
    "aLCz3+eFUJ0wXQYDVR0RBFYwVIITKi5henVyZS1kZXZpY2VzLm5ldIIaKi5hbXFw\r\n"
    "d3MuYXp1cmUtZGV2aWNlcy5uZXSCISouc3UubWFuYWdlbWVudC1henVyZS1kZXZp\r\n"
    "Y2VzLm5ldDAfBgNVHSMEGDAWgBRRryQmnPRoIleAJis7RmIVex7MpTB9BgNVHR8E\r\n"
    "djB0MHKgcKBuhjZodHRwOi8vbXNjcmwubWljcm9zb2Z0LmNvbS9wa2kvbXNjb3Jw\r\n"
    "L2NybC9tc2l0d3d3Mi5jcmyGNGh0dHA6Ly9jcmwubWljcm9zb2Z0LmNvbS9wa2kv\r\n"
    "bXNjb3JwL2NybC9tc2l0d3d3Mi5jcmwwcAYIKwYBBQUHAQEEZDBiMDwGCCsGAQUF\r\n"
    "BzAChjBodHRwOi8vd3d3Lm1pY3Jvc29mdC5jb20vcGtpL21zY29ycC9tc2l0d3d3\r\n"
    "Mi5jcnQwIgYIKwYBBQUHMAGGFmh0dHA6Ly9vY3NwLm1zb2NzcC5jb20wTgYDVR0g\r\n"
    "BEcwRTBDBgkrBgEEAYI3KgEwNjA0BggrBgEFBQcCARYoaHR0cDovL3d3dy5taWNy\r\n"
    "b3NvZnQuY29tL3BraS9tc2NvcnAvY3BzADAnBgkrBgEEAYI3FQoEGjAYMAoGCCsG\r\n"
    "AQUFBwMBMAoGCCsGAQUFBwMCMA0GCSqGSIb3DQEBCwUAA4ICAQCrjzOSW+X6v+UC\r\n"
    "u+JkYyuypXN14pPLcGFbknJWj6DAyFWXKC8ihIYdtf/szWIO7VooplSTZ05u/JYu\r\n"
    "ZYh7fAw27qih9CLhhfncXi5yzjgLMlD0mlbORvMJR/nMl7Yh1ki9GyLnpOqMmO+E\r\n"
    "yTpOiE07Uyt2uWelLHjMY8kwy2bSRXIp7/+A8qHRaIIdXNtAKIK5jo068BJpo77h\r\n"
    "4PljCb9JFdEt6sAKKuaP86Y+8oRZ7YzU4TLDCiK8P8n/gQXH0vvhOE/O0n7gWPqB\r\n"
    "n8KxsnRicop6tB6GZy32Stn8w0qktmQNXOGU+hp8OL6irULWZw/781po6d78nmwk\r\n"
    "1IFl2TB4+jgyblvJdTM0rx8vPf3F2O2kgsRNs9M5qCI7m+he43Bhue0Fj/h3oIIo\r\n"
    "Qx7X/uqc8j3VTNE9hf2A4wksSRgRydjAYoo+bduNagC5s7Eucb4mBG0MMk7HAQU9\r\n"
    "m/gyaxqth6ygDLK58wojSV0i4RiU01qZkHzqIWv5FhhMjbFwyKEc6U35Ps7kP/1O\r\n"
    "fdGm13ONaYqDl44RyFsLFFiiDYxZFDSsKM0WDxbl9ULAlVc3WR85kEBK6I+pSQj+\r\n"
    "7/Z5z2zTz9qOFWgB15SegTbjSR7uk9mEVnj9KDlGtG8W1or0EGrrEDP2CMsp0oEj\r\n"
    "VTJbZAxEaZ3cVCKva5sQUxFMjwG32g==\r\n"
    "-----END CERTIFICATE-----\r\n";
}