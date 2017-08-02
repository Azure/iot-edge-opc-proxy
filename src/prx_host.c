// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "prx_host.h"
#include "prx_log.h"
#include "prx_module.h"
#include "prx_sched.h"
#include "prx_server.h"
#include "prx_config.h"
#include "io_host.h"
#include "io_transport.h"

#include "pal.h"
#include "pal_file.h"
#include "pal_net.h"  // for pal_gethostname

#include "util_misc.h"
#include "util_signal.h"
#include "util_string.h"

#include "azure_c_shared_utility/doublylinkedlist.h"
#include "azure_c_shared_utility/refcount.h"
#include "version.h"

#include <stdio.h>
#include "getopt.h"

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
    prx_module_api_t* module;         // entry points to manage edge modules
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
    const char* name,
    const char* domain
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
            result = prx_ns_entry_create(prx_ns_entry_type_proxy,
                name, name, domain, MODULE_VER_NUM, &entry);
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

    for (int local = 0; local < 2; local++)
    {
        //
        // First round remove from remote and local, next remove remaining
        // matching entries from local database.  If remote removal fails,
        // re-add to local, and exit.
        //
        if (name)
            result = prx_ns_get_entry_by_name(local ? host->local : host->remote,
                name, &resultset);
        else
            result = prx_ns_get_entry_by_type(local ? host->local : host->remote,
                prx_ns_entry_type_proxy, &resultset);
        if (result == er_not_found)
            continue;
        if (result != er_ok)
            break;

        entry = prx_ns_result_pop(resultset);
        while (entry)
        {
            if (prx_ns_entry_get_type(entry) & prx_ns_entry_type_proxy)
            {
                result = prx_ns_remove_entry(host->local, entry);
                if (local && result != er_ok)
                    break;
                if (!local)
                {
                    result = prx_ns_remove_entry(host->remote, entry);
                    if (result != er_ok)
                    {
                        (void)prx_ns_create_entry(host->local, entry);
                        break;
                    }
                }
            }
            prx_ns_entry_release(entry);
            entry = prx_ns_result_pop(resultset);
        }
        if (result != er_ok)
            break;
    }

    if (result == er_not_found)
        result = er_ok;
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

    int c, tmp;
    int option_index = 0;

    io_cs_t *cs = NULL;
    prx_ns_entry_t* entry = NULL;
    const char *server_name = NULL;
    const char *domain = NULL;
    const char *log_config = NULL;
    const char *log_file = NULL;
    const char *ns_registry = NULL;
    const char *c_string = NULL;

    char buffer[128];
    bool is_install = false;
    bool is_uninstall = false;
    bool should_exit = false;

    static struct option long_options[] =
    {
        { "install",                        no_argument,                 NULL, 'i' },
        { "uninstall",                      no_argument,                 NULL, 'u' },
        { "only-websocket",                 no_argument,                 NULL, 'w' },
        { "daemonize",                      no_argument,                 NULL, 'd' },
        { "help",                           no_argument,                 NULL, 'h' },
        { "version",                        no_argument,                 NULL, 'v' },
        { "log-to-iothub",                  no_argument,                 NULL, 'T' },
        { "allow-fs-browsing",              no_argument,                 NULL, 'F' },
        { "allow-ws-unsecure",              no_argument,                 NULL, 'W' },
        { "bind-to-device",                 required_argument,           NULL, 'b' },
        { "blacklisted-ports",              required_argument,           NULL, 'r' },
        { "import",                         required_argument,           NULL, 's' },
        { "name",                           required_argument,           NULL, 'n' },
        { "domain",                         required_argument,           NULL, 'N' },
        { "connection-string",              required_argument,           NULL, 'c' },
        { "log-file",                       required_argument,           NULL, 'l' },
        { "log-config-file",                required_argument,           NULL, 'L' },
        { "connection-string-file",         required_argument,           NULL, 'C' },
        { "hub-config-file",                required_argument,           NULL, 'H' },
        { "db-file",                        required_argument,           NULL, 'D' },
        { "proxy",                          required_argument,           NULL, 'p' },
        { "proxy-user",                     required_argument,           NULL, 'x' },
        { "proxy-pwd",                      required_argument,           NULL, 'y' },
        { "token-ttl",                      required_argument,           NULL, 't' },
        { 0,                                0,                           NULL,  0  }
    };

    do
    {
        result = er_ok;
        // Parse options
        while (result == er_ok)
        {
            c = getopt_long(argc, argv, "iuwWdhvTFr:s:n:N:c:l:L:C:H:D:p:x:y:t:b:",
                long_options, &option_index);
            if (c == -1)
                break;
            switch (c)
            {
            case 'W':
            case 'w':
                if (0 == (pal_caps() & pal_cap_wsclient))
                {
                    printf("ERROR: Websocket not supported!\n");
                    result = er_arg;
                    break;
                }
                tmp = __prx_config_get_int(prx_config_key_connect_flag, 0);
                __prx_config_set_int(prx_config_key_connect_flag,
                    tmp | (c == 'w' ? 0x1 : 0x2));
                break;
            case 'p':
                __prx_config_set(prx_config_key_proxy_host, optarg);
                break;
            case 'T':
                __prx_config_set_int(prx_config_key_log_telemetry, 1);
                break;
            case 'F':
                __prx_config_set_int(prx_config_key_browse_fs, 1);
                break;
            case 'r':
                result = string_parse_range_list(optarg, NULL, NULL);
                if (result != er_ok)
                {
                    printf("ERROR: Bad arg for --blacklisted-ports option (%.128s). \n\n",
                        optarg ? optarg : "");
                    break;
                }
                __prx_config_set(prx_config_key_restricted_ports, optarg);
                break;
            case 'b':
                __prx_config_set(prx_config_key_bind_device, optarg);
                break;
            case 'x':
                __prx_config_set(prx_config_key_proxy_user, optarg);
                break;
            case 'y':
                __prx_config_set(prx_config_key_proxy_pwd, optarg);
                break;
            case 't':
                __prx_config_set(prx_config_key_token_ttl, optarg);
                break;
            case 'd':
                host->hidden = true;
                break;
            case 'i':
                should_exit = true;
                is_install = true;
                break;
            case 'u':
                should_exit = true;
                is_uninstall = true;
                break;
            case 'v':
#if defined(MODULE_VERSION)
                printf("Version: " MODULE_VERSION "\n");
#else
                printf("Version: <UNKNOWN>\n");
#endif
                break;
            case 'n':
                server_name = optarg;
                break;
            case 'N':
                domain = optarg;
                break;
            case 's':
                if (0 == (pal_caps() & pal_cap_cred))
                {
                    printf("ERROR: Secret store not supported on this platform!\n");
                    result = er_arg;
                    break;
                }
                if (cs)
                {
                    printf("ERROR: Multiple connection string arguments"
                        " encountered...\n\n");
                    result = er_arg;
                    break;
                }
                printf("Importing connection string...\n");
                __prx_config_set(prx_config_key_policy_import, optarg);
                should_exit = true;
                c_string = optarg;
                if (c_string && strlen(c_string) > 0)
                    result = io_cs_create_from_string(c_string, &cs);
                else
                    result = er_arg;
                if (result != er_ok)
                    printf("ERROR: Malformed --import argument. \n\n");
                break;
            case 'c':
                if (cs)
                {
                    printf("ERROR: Multiple connection string arguments"
                        " encountered...\n\n");
                    result = er_arg;
                    break;
                }
                c_string = optarg;
                if (c_string && strlen(c_string) > 0)
                    result = io_cs_create_from_string(c_string, &cs);
                else
                    result = er_arg;
                if (result != er_ok)
                    printf("ERROR: Malformed --connection-string argument. \n\n");
                break;
            case 'D':
                ns_registry = optarg;
                break;
            case 'C':
                if (cs)
                {
                    printf("ERROR: Multiple connection string arguments"
                        " encountered...\n\n");
                    result = er_arg;
                    break;
                }
                result = io_cs_create_from_raw_file(optarg, &cs);
                if (result != er_ok)
                    printf("ERROR: Failed to load iothubowner connection string from "
                        "--connection-string-file '%.128s' arg. \n\n", optarg ? optarg : "");
                break;
            case 'H':
                result = prx_ns_iot_hub_create(optarg, &host->remote);
                if (result != er_ok)
                    printf("ERROR: Failed to load iot hub registry from "
                        "--hub-config-file '%.128s' arg. \n\n", optarg ? optarg : "");
                break;
            case 'L':
                log_config = optarg;
                break;
            case 'l':
                log_file = optarg;
                break;
            default:
                printf("ERROR: Unrecognized option %c\n\n", (char)c);
                // Fall through
            case 'h':
            case '?':
                result = er_arg;
                break;
            }
        }

        if (result != er_ok)
            break;

        if (is_install && is_uninstall)
        {
            printf("ERROR: Cannot use --install and --uninstall together...");
            result = er_arg;
            break;
        }

        if (log_config && log_file)
            printf("WARNING: --log-file overrides --log-config-file option...");
        if (log_config)
        {
            // Configure logging
            result = pal_get_real_path(log_config ? log_config : "log.config", &log_config);
            if (result != er_ok)
                break;
        }
        else if (!log_file)
        {
            // try loading default log configuration file
            result = pal_get_real_path("log.config", &log_config);
            if (result == er_ok && !pal_file_exists(log_config))
            {
                pal_free_path(log_config);
                log_config = NULL;
            }
        }

        if (log_config)
        {
            result = log_read_config(log_config);
            if (result != er_ok)
                printf("WARNING: Logging config file %s was not used. \n\n", log_config);
            pal_free_path(log_config);
            log_config = NULL;
        }

        if (log_file)
        {
            result = log_set_log_file(log_file);
            if (result != er_ok)
            {
                printf("ERROR: Failed to configure logging to use log file %s. \n\n",
                    log_file);
                break;
            }
        }

        if (!cs)
        {
            c_string = getenv("_HUB_CS");
            if (c_string && strlen(c_string) > 0)
            {
                result = io_cs_create_from_string(c_string, &cs);
                if (result != er_ok)
                    break;
            }
        }

        // Load registry from file or create in memory one if no file specified
        result = prx_ns_generic_create(ns_registry, &host->local);
        if (result != er_ok)
        {
            printf("ERROR: Failed to create local registry. \n\n");
            break;
        }

        if (cs && io_cs_get_device_id(cs))
        {
            if (is_install || is_uninstall || server_name)
            {
                printf("ERROR: A device connection string cannot be used for -i or -u or -n");
                result = er_arg;
                break;
            }

            if (should_exit)
                break; // Exit now, a user might have just wanted to import the string

            // If connection string is proxy connection string add to local registry
            result = prx_ns_entry_create_from_cs(
                prx_ns_entry_type_startup | prx_ns_entry_type_proxy, NULL, cs, &entry);
            if (result != er_ok)
                break;

            result = prx_ns_create_entry(host->local, entry);
            if (result != er_ok)
            {
                printf("ERROR: Failed to add device connection string to local registry");
                break;
            }
            // Ok, done...
            break;
        }

        // Handle install / uninstall
        if (!host->remote)
        {
            if (!cs)
            {
                if (is_install || is_uninstall)
                {
                    // If we want to install or uninstall we need a policy connection string
                    printf("ERROR: Missing --connection-string option with policy. \n\n");
                    result = er_arg;
                    break;
                }
            }
            else
            {
                result = prx_ns_iot_hub_create_from_cs(cs, &host->remote);
                if (result != er_ok)
                {
                    printf("ERROR: Failed to create iothub registry from "
                        "--connection-string supplied connection string. \n\n");
                    break;
                }
            }
        }

        if (!ns_registry && !is_install && !is_uninstall)
        {
            if (should_exit) // Exit now
                break;

            // If we should not exit, we must try to install a proxy to run
            is_install = true;
            if (!host->remote)
            {
                // Empty local registry and no remote, nothing to do...
                printf("ERROR: Cannot connect without connection strings...");
                result = er_arg;
                break;
            }
        }

        if (is_install || is_uninstall)
        {
            // Ensure we have a name
            if (!server_name)
            {
                result = pal_gethostname(buffer, _countof(buffer));
                if (result != er_ok)
                    break;
                server_name = buffer;
            }
            else if (is_uninstall && 0 == string_compare(server_name, "*"))
                server_name = NULL;  // Remove all!

            // Run install/uninstall
            /**/ if (is_install)
                result = prx_host_install_server(host, server_name, domain);
            else if (is_uninstall)
                result = prx_host_uninstall_server(host, server_name);

            if (result != er_ok)
            {
                printf("ERROR: %s %s failed! Check parameters...\n",
                    prx_err_string(result), !is_uninstall ? "Install" : "Uninstall");
                break;
            }
            printf("%s %s\n", server_name ? server_name : "All", !is_uninstall ?
                "installed" : "uninstalled");
            server_name = NULL;
        }
        break;
    }
    while (0);

    if (cs)
        io_cs_free(cs);
    if (entry)
        prx_ns_entry_release(entry);
    if (should_exit && !host->hidden && result == er_ok)
    {
        printf("Success.\n");
        return er_aborted;
    }
    if (result != er_arg)
    {
        if (result != er_ok)
            printf("Operation failed.\n");
        return result;
    }

    printf(" Command line options:                                                      \n");
    printf(" -c, --connection-string string     A connection string to use. This can be \n");
    printf("                                    either a policy connection string for -i\n");
    printf("                                    or -u, or a device connection string,   \n");
    printf("                                    used to connect to Iot Hub.             \n");
    if (pal_caps() & pal_cap_cred)
    {
    printf(" -s, --import string                While device connection strings are     \n");
    printf("                                    automatically persisted into the user's \n");
    printf("                                    secret store on your device, policy keys\n");
    printf("                                    are not. Use this option to import and  \n");
    printf("                                    persist any shared access keys.         \n");
    }
    if (pal_caps() & pal_cap_file)
    {
    printf(" -C, --connection-string-file file  same as -c but read from file. If -c or \n");
    printf("                                    -C options are not provided, connection \n");
    printf("                                    string is read from $_HUB_CS environment\n");
    printf("                                    variable.                               \n");
    printf(" -F, --allow-fs-browsing            Allow clients to browse the file system.\n");
    }
    printf(" -r, --blacklisted-ports range      Use this setting to blacklist ports the \n");
    printf("                                    proxy must never connect to. The value  \n");
    printf("                                    is a semicolon delimited list of values \n");
    printf("                                    that can contain ranges, for example -r \n");
    printf("                                    0-4839;4842-65536 restricts connections \n");
    printf("                                    to all ports except for 4840 and 4841.  \n");
    printf(" -b, --bind-to-device string        Use this setting to direct the proxy to \n");
    printf("                                    bind sockets to the named device.       \n");
    printf("                                    Note that if this fails, e.g. due to    \n");
    printf("                                    rights, the default OS behavior applies.\n");
    if (pal_caps() & pal_cap_wsclient)
    {
    printf(" -w, --only-websocket               Always use websockets for outbound      \n");
    printf("                                    connections. Without -w Azure connection\n");
    printf("                                    will failover if opening a raw tcp/ip   \n");
    printf("                                    connection to Azure fails.              \n");
    printf(" -p, --proxy host:port              Local web proxy to use for all outbound \n");
    printf("     --proxy-user string            traffic, with user name and password if \n");
    printf("     --proxy-pwd string             needed.                                 \n");
    }
    printf(" -T, --log-to-iothub                Send raw log output to IoT Hub on the   \n");
    printf("                                    proxy telemetry endpoint.               \n");
#if !defined(NO_ZLOG)
    printf(" -l, --log-file file                File to log to using simple formatting. \n");
    printf(" -L, --log-config-file file         For more advanced settings, the zlog    \n");
    printf("                                    configuration file to use. Defaults to  \n");
    printf("                                    ./log.config.                           \n");
#endif
    printf(" -i, --install                      Installs a proxy server in the IoT Hub  \n");
    printf("                                    device registry, then exits.            \n");
    printf(" -u, --uninstall                    Uninstalls proxy server on Iot Hub then \n");
    printf("                                    exits.                                  \n");
    printf("                                    -i and -u requires -c or -C, or $_HUB_CS\n");
    printf("                                    with 'manage' policy connection string  \n");
    printf("                                    and access to the shared access key.    \n");
    printf(" -n, --name <string>                Name of proxy to install or uninstall.  \n");
    printf("                                    If -n is not provided, hostname is used.\n");
    printf("     --domain <string>              Virtual domain the proxy is assigned to.\n");
    printf("                                    Use for -i. If not provided, no domain  \n");
    printf("                                    is assigned.                            \n");
    if (pal_caps() & pal_cap_file)
    {
    printf(" -D, --db-file <file-name>          Local storage for proxy connection info.\n");
    printf("                                    This is where newly registered instances\n");
    printf("                                    are persisted. If not specified,        \n");
    printf("                                    connection string is kept in memory.    \n");
    }
    printf(" -t, --token-ttl int                Time to live in seconds for all shared  \n");
    printf("                                    access tokens provided to IoT Hub if you\n");
    printf("                                    prefer a value different from default.  \n");
#if defined(EXPERIMENTAL)
    printf(" -d, --daemonize                    Runs the proxy as a service/daemon,     \n");
    printf("                                    otherwise runs proxy host process as    \n");
    printf("                                    console process.                        \n");
#endif
    printf(" -v, --version                      Prints the version information for this \n");
    printf("                                    binary and exits.                       \n");
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
        module->module = host->module->on_create(NULL, module->entry);

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

    chk_arg_fault_return(host);
    do
    {
        host->running = true;

        // Start socket servers
        result = prx_host_modules_start(host);
        if (result != er_ok)
            break;

        log_info(host->log, "Host server started!");
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
    chk_arg_fault_return(host);
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
    chk_arg_fault_return(host);
    return signal_wait(host->exit_signal, ~0);
}

//
// Break execution
//
int32_t prx_host_sig_break(
    prx_host_t* host
)
{
    chk_arg_fault_return(host);
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
    chk_arg_fault_return(cloned);
    if (!process_host)
    {
        // Lazy create
        result = prx_host_init(proxy_type_server, 0, NULL);
        if (result != er_ok)
            return result;
        dbg_assert_ptr(process_host);
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

        prx_config_deinit();

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

        result = prx_config_init();
        if (result != er_ok)
        {
            log_error(process_host->log, "Failed to initialize config (%s)",
                prx_err_string(result));
            break;
        }

        result = pal_init();
        if (result != er_ok)
        {
            log_error(process_host->log, "Failed to initialize pal (%s)",
                prx_err_string(result));
            break;
        }

        // Init from console
        result = prx_host_init_from_command_line(process_host, argc, argv);
        if (result != er_ok)
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
        log_trace(process_host->log, "Proxy host created!");
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
    /* DigiCert Baltimore Root */
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
    /*DigiCert Global Root CA*/
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\r\n"
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\r\n"
    "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\r\n"
    "QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\r\n"
    "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\r\n"
    "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\r\n"
    "9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\r\n"
    "CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\r\n"
    "nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\r\n"
    "43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\r\n"
    "T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\r\n"
    "gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\r\n"
    "BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\r\n"
    "TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\r\n"
    "DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\r\n"
    "hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\r\n"
    "06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\r\n"
    "PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\r\n"
    "YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\r\n"
    "CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\r\n"
    "-----END CERTIFICATE-----\r\n"
    /*D-TRUST Root Class 3 CA 2 2009*/
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIEMzCCAxugAwIBAgIDCYPzMA0GCSqGSIb3DQEBCwUAME0xCzAJBgNVBAYTAkRF\r\n"
    "MRUwEwYDVQQKDAxELVRydXN0IEdtYkgxJzAlBgNVBAMMHkQtVFJVU1QgUm9vdCBD\r\n"
    "bGFzcyAzIENBIDIgMjAwOTAeFw0wOTExMDUwODM1NThaFw0yOTExMDUwODM1NTha\r\n"
    "ME0xCzAJBgNVBAYTAkRFMRUwEwYDVQQKDAxELVRydXN0IEdtYkgxJzAlBgNVBAMM\r\n"
    "HkQtVFJVU1QgUm9vdCBDbGFzcyAzIENBIDIgMjAwOTCCASIwDQYJKoZIhvcNAQEB\r\n"
    "BQADggEPADCCAQoCggEBANOySs96R+91myP6Oi/WUEWJNTrGa9v+2wBoqOADER03\r\n"
    "UAifTUpolDWzU9GUY6cgVq/eUXjsKj3zSEhQPgrfRlWLJ23DEE0NkVJD2IfgXU42\r\n"
    "tSHKXzlABF9bfsyjxiupQB7ZNoTWSPOSHjRGICTBpFGOShrvUD9pXRl/RcPHAY9R\r\n"
    "ySPocq60vFYJfxLLHLGvKZAKyVXMD9O0Gu1HNVpK7ZxzBCHQqr0ME7UAyiZsxGsM\r\n"
    "lFqVlNpQmvH/pStmMaTJOKDfHR+4CS7zp+hnUquVH+BGPtikw8paxTGA6Eian5Rp\r\n"
    "/hnd2HN8gcqW3o7tszIFZYQ05ub9VxC1X3a/L7AQDcUCAwEAAaOCARowggEWMA8G\r\n"
    "A1UdEwEB/wQFMAMBAf8wHQYDVR0OBBYEFP3aFMSfMN4hvR5COfyrYyNJ4PGEMA4G\r\n"
    "A1UdDwEB/wQEAwIBBjCB0wYDVR0fBIHLMIHIMIGAoH6gfIZ6bGRhcDovL2RpcmVj\r\n"
    "dG9yeS5kLXRydXN0Lm5ldC9DTj1ELVRSVVNUJTIwUm9vdCUyMENsYXNzJTIwMyUy\r\n"
    "MENBJTIwMiUyMDIwMDksTz1ELVRydXN0JTIwR21iSCxDPURFP2NlcnRpZmljYXRl\r\n"
    "cmV2b2NhdGlvbmxpc3QwQ6BBoD+GPWh0dHA6Ly93d3cuZC10cnVzdC5uZXQvY3Js\r\n"
    "L2QtdHJ1c3Rfcm9vdF9jbGFzc18zX2NhXzJfMjAwOS5jcmwwDQYJKoZIhvcNAQEL\r\n"
    "BQADggEBAH+X2zDI36ScfSF6gHDOFBJpiBSVYEQBrLLpME+bUMJm2H6NMLVwMeni\r\n"
    "acfzcNsgFYbQDfC+rAF1hM5+n02/t2A7nPPKHeJeaNijnZflQGDSNiH+0LS4F9p0\r\n"
    "o3/U37CYAqxva2ssJSRyoWXuJVrl5jLn8t+rSfrzkGkj2wTZ51xY/GXUl77M/C4K\r\n"
    "zCUqNQT4YJEVdT1B/yMfGchs64JTBKbkTCJNjYy6zltz7GRUUG3RnFX7acM2w4y8\r\n"
    "PIWmawomDeCTmGCufsYkl4phX5GOZpIJhzbNi5stPvZR1FDUWSi9g/LMKHtThm3Y\r\n"
    "Johw1+qRzT65ysCQblrGXnRl11z+o+I=\r\n"
    "-----END CERTIFICATE-----\r\n"
    /*WoSign*/
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIFdjCCA16gAwIBAgIQXmjWEXGUY1BWAGjzPsnFkTANBgkqhkiG9w0BAQUFADBV\r\n"
    "MQswCQYDVQQGEwJDTjEaMBgGA1UEChMRV29TaWduIENBIExpbWl0ZWQxKjAoBgNV\r\n"
    "BAMTIUNlcnRpZmljYXRpb24gQXV0aG9yaXR5IG9mIFdvU2lnbjAeFw0wOTA4MDgw\r\n"
    "MTAwMDFaFw0zOTA4MDgwMTAwMDFaMFUxCzAJBgNVBAYTAkNOMRowGAYDVQQKExFX\r\n"
    "b1NpZ24gQ0EgTGltaXRlZDEqMCgGA1UEAxMhQ2VydGlmaWNhdGlvbiBBdXRob3Jp\r\n"
    "dHkgb2YgV29TaWduMIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAvcqN\r\n"
    "rLiRFVaXe2tcesLea9mhsMMQI/qnobLMMfo+2aYpbxY94Gv4uEBf2zmoAHqLoE1U\r\n"
    "fcIiePyOCbiohdfMlZdLdNiefvAA5A6JrkkoRBoQmTIPJYhTpA2zDxIIFgsDcScc\r\n"
    "f+Hb0v1naMQFXQoOXXDX2JegvFNBmpGN9J42Znp+VsGQX+axaCA2pIwkLCxHC1l2\r\n"
    "ZjC1vt7tj/id07sBMOby8w7gLJKA84X5KIq0VC6a7fd2/BVoFutKbOsuEo/Uz/4M\r\n"
    "x1wdC34FMr5esAkqQtXJTpCzWQ27en7N1QhatH/YHGkR+ScPewavVIMYe+HdVHpR\r\n"
    "aG53/Ma/UkpmRqGyZxq7o093oL5d//xWC0Nyd5DKnvnyOfUNqfTq1+ezEC8wQjch\r\n"
    "zDBwyYaYD8xYTYO7feUapTeNtqwylwA6Y3EkHp43xP901DfA4v6IRmAR3Qg/UDar\r\n"
    "uHqklWJqbrDKaiFaafPz+x1wOZXzp26mgYmhiMU7ccqjUu6Du/2gd/Tkb+dC221K\r\n"
    "mYo0SLwX3OSACCK28jHAPwQ+658geda4BmRkAjHXqc1S+4RFaQkAKtxVi8QGRkvA\r\n"
    "Sh0JWzko/amrzgD5LkhLJuYwTKVYyrREgk/nkR4zw7CT/xH8gdLKH3Ep3XZPkiWv\r\n"
    "HYG3Dy+MwwbMLyejSuQOmbp8HkUff6oZRZb9/D0CAwEAAaNCMEAwDgYDVR0PAQH/\r\n"
    "BAQDAgEGMA8GA1UdEwEB/wQFMAMBAf8wHQYDVR0OBBYEFOFmzw7R8bNLtwYgFP6H\r\n"
    "EtX2/vs+MA0GCSqGSIb3DQEBBQUAA4ICAQCoy3JAsnbBfnv8rWTjMnvMPLZdRtP1\r\n"
    "LOJwXcgu2AZ9mNELIaCJWSQBnfmvCX0KI4I01fx8cpm5o9dU9OpScA7F9dY74ToJ\r\n"
    "MuYhOZO9sxXqT2r09Ys/L3yNWC7F4TmgPsc9SnOeQHrAK2GpZ8nzJLmzbVUsWh2e\r\n"
    "JXLOC62qx1ViC777Y7NhRCOjy+EaDveaBk3e1CNOIZZbOVtXHS9dCF4Jef98l7VN\r\n"
    "g64N1uajeeAz0JmWAjCnPv/So0M/BVoG6kQC2nz4SNAzqfkHx5Xh9T71XXG68pWp\r\n"
    "dIhhWeO/yloTunK0jF02h+mmxTwTv97QRCbut+wucPrXnbes5cVAWubXbHssw1ab\r\n"
    "R80LzvobtCHXt2a49CUwi1wNuepnsvRtrtWhnk/Yn+knArAdBtaP4/tIEp9/EaEQ\r\n"
    "PkxROpaw0RPxx9gmrjrKkcRpnd8BKWRRb2jaFOwIQZeQjdCygPLPwj2/kWjFgGce\r\n"
    "xGATVdVhmVd8upUPYUk6ynW8yQqTP2cOEvIo4jEbwFcW3wh8GcF+Dx+FHgo2fFt+\r\n"
    "J7x6v+Db9NpSvd4MVHAxkUOVyLzwPt0JfjBkUO1/AaQzZ01oT74V77D2AhGiGxMl\r\n"
    "OtzCWfHjXEa7ZywCRuoeSKbmW9m1vFGikpbbqsY3Iqb+zCB0oy2pLmvLwIIRIbWT\r\n"
    "ee5Ehr7XHuQe+w==\r\n"
    "-----END CERTIFICATE-----\r\n"
;
}
