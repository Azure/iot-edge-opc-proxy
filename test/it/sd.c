// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "prx_log.h"
#include "pal.h"
#include "pal_sd.h"
#include "pal_mt.h"
#include "util_string.h"
#include "util_signal.h"
#include <stdio.h>

static log_t cat;

typedef struct sd_ctx
{
    bool recursive;
    pal_sdclient_t* client;
    pal_sdbrowser_t* browser;
}
sd_ctx_t;

//
// Prints results
//
static int32_t pal_sd_result_printer(
    void *context,
    int32_t itf_index,
    int32_t error,
    pal_sd_result_type_t type,
    void *result,
    int32_t flags
)
{
    pal_sd_browse_result_t* rec = (pal_sd_browse_result_t*)result;
    pal_sd_service_entry_t* res = (pal_sd_service_entry_t*)result;
    prx_addrinfo_t* ainfo = (prx_addrinfo_t*)result;
    sd_ctx_t* ctx = (sd_ctx_t*)context;

    if (error != er_ok)
    {
        printf("!!! ERROR Result %d !!!", error);
        return er_ok;
    }

    switch (type)
    {
    case pal_sd_result_domain:
        printf("[DOM] %s I:%x  D: %s \n", flags & pal_sd_result_removed ? "REM" : "   ",
            itf_index, rec->domain);
        break;
    case pal_sd_result_type:
        printf("[TYP] %s I:%x  D: %s  T: %s \n", flags & pal_sd_result_removed ? "REM" : "   ",
            itf_index, rec->domain, rec->service_type);
        break;
    case pal_sd_result_service:
        printf("[SVC] %s I:%x  D: %s  T: %s  N: %s \n", flags & pal_sd_result_removed ? "REM" : "   ",
            itf_index, rec->domain, rec->service_type, rec->service_name);
        break;
    case pal_sd_result_entry:
        printf("[RES]   ->: %s:%d  I:%x \n", prx_socket_address_proxy_get_host(res->addr),
            res->addr->port, itf_index);
        for (size_t i = 0; i < res->records_len; i++)
        {
            printf("[RES]        TXT[%zu]: %.*s \n", i,
               (int)res->records[i].property.bin.size, 
                    res->records[i].property.bin.value);
        }
        break;
    case pal_sd_result_addrinfo:
        if (ainfo->address.un.family == prx_address_family_inet)
        {
            printf("[ADR]   ->: %s: %d.%d.%d.%d:%d \n", ainfo->name,
                ainfo->address.un.ip.un.in4.un.u8[0], ainfo->address.un.ip.un.in4.un.u8[1],
                ainfo->address.un.ip.un.in4.un.u8[2], ainfo->address.un.ip.un.in4.un.u8[3],
                ainfo->address.un.ip.port);
        }
        else
        {
            printf("[ADR]   ->: %s: [%x:%x:%x:%x:%x:%x:%x:%x]:%d \n", ainfo->name,
                ainfo->address.un.ip.un.in6.un.u16[0], ainfo->address.un.ip.un.in6.un.u16[1],
                ainfo->address.un.ip.un.in6.un.u16[2], ainfo->address.un.ip.un.in6.un.u16[3],
                ainfo->address.un.ip.un.in6.un.u16[4], ainfo->address.un.ip.un.in6.un.u16[5],
                ainfo->address.un.ip.un.in6.un.u16[6], ainfo->address.un.ip.un.in6.un.u16[7],
                ainfo->address.un.ip.port);
        }
        return er_ok;
    default:
        dbg_assert(0, "Unexpected");
        return er_fatal;
    }

    if (!(flags & pal_sd_result_removed) && ctx->recursive)
    {
        if (type == pal_sd_result_entry)
            return pal_sdbrowser_resolve(ctx->browser, res->addr, prx_itf_index_all);

        return pal_sdbrowser_browse(ctx->browser, rec->service_name,
            rec->service_type, rec->domain, prx_itf_index_all);
    }
    return er_ok;
}

//
// Print all domains
//
int32_t print(
    const char* domain,
    const char* type,
    const char* name,
    sd_ctx_t* ctx
)
{
    int32_t result;
    signal_t* signal;

    result = signal_create(true, false, &signal);
    if (result != er_ok)
        return result;
    ctx->client = NULL;
    ctx->browser = NULL;
    do
    {
        result = pal_sdclient_create(NULL, NULL, &ctx->client);
        if (result != er_ok)
            break;

        result = pal_sdbrowser_create(ctx->client, pal_sd_result_printer, ctx, 
            &ctx->browser);
        if (result != er_ok)
        {
            log_error(cat, "Failed creating browser %s", prx_err_string(result));
            break;
        }

        result = pal_sdbrowser_browse(ctx->browser, name, type, domain, 
            prx_itf_index_all);
        if (result != er_ok)
        {
            log_error(cat, "Failed using browser %s", prx_err_string(result));
            break;
        }

        signal_wait(signal, 120000);
        result = er_ok;
        break;
    } while (0);

    if (ctx->browser)
        pal_sdbrowser_free(ctx->browser);
    if (ctx->client)
        pal_sdclient_free(ctx->client);
    
    if (signal)
        signal_free(signal);
    return result;
}

//
// Resolve address
//
int32_t resolve(
    const char* host,
    const char* port,
    sd_ctx_t* ctx
)
{
    int32_t result;
    signal_t* signal;
    prx_socket_address_proxy_t addr;

    if (!host)
        return er_arg;

    result = signal_create(true, false, &signal);
    if (result != er_ok)
        return result;
    ctx->client = NULL;
    ctx->browser = NULL;
    ctx->recursive = false;
    do
    {
        result = pal_sdclient_create(NULL, NULL, &ctx->client);
        if (result != er_ok)
            break;

        result = pal_sdbrowser_create(ctx->client, pal_sd_result_printer, ctx,
            &ctx->browser);
        if (result != er_ok)
        {
            log_error(cat, "Failed creating browser %s", prx_err_string(result));
            break;
        }

        memset(&addr, 0, sizeof(addr));
        addr.family = prx_address_family_proxy;
        if (port) addr.port = (uint16_t)atoi(port);
        addr.host_dyn = host;

        result = pal_sdbrowser_resolve(ctx->browser, &addr, prx_itf_index_all);
        if (result != er_ok)
        {
            log_error(cat, "Failed using browser %s", prx_err_string(result));
            break;
        }

        signal_wait(signal, 500);
        result = er_ok;
        break;
    } while (0);

    if (ctx->browser)
        pal_sdbrowser_free(ctx->browser);
    if (ctx->client)
        pal_sdclient_free(ctx->client);

    if (signal)
        signal_free(signal);
    return result;
}

//
// Name service registry test utility
//
int main_sd(int argc, char *argv[])
{
    int32_t result;
    const char* name = NULL, *type = NULL, *domain = NULL;
    sd_ctx_t ctx;

    if (argc <= 1)
        return er_arg;

    cat = log_get("test.sd");

    memset(&ctx, 0, sizeof(sd_ctx_t));
    while (argc > 1)
    {
        argv++;
        argc--;

        /**/ if (!strcmp(argv[0], "--opc"))
            type = "_opcua-tcp._tcp.";
        else if (!strcmp(argv[0], "--recursive"))
            ctx.recursive = true;
        
        
        else if (!domain)
            domain = argv[0];
        else if (!type)
            type = argv[0];
        else if (!name)
            name = argv[0];
    } 

    result = pal_init();
    if (result != er_ok)
        return result;

    result = print(domain, type, name, &ctx);

    pal_deinit();
    return result;
}

//
// Name resolver
//
int main_sr(int argc, char *argv[])
{
    int32_t result;
    const char* host = NULL;
    const char* port = NULL;
    sd_ctx_t ctx;

    if (argc <= 1)
        return er_arg;

    cat = log_get("test.sr");

    memset(&ctx, 0, sizeof(sd_ctx_t));
    while (argc > 1)
    {
        argv++;
        argc--;

        if (!host)
            host = argv[0];
        if (!port)
            port = argv[0];
    }

    result = pal_init();
    if (result != er_ok)
        return result;

    result = resolve(host, port, &ctx);

    pal_deinit();
    return result;
}
