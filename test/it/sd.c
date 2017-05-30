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
    pal_sd_resolve_result_t* res = (pal_sd_resolve_result_t*)result;
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
    case pal_sd_result_resolve:
        printf("[RES]   ->: %s:%d  I:%x \n", res->addr->host, 
            res->addr->port, itf_index);
        for (size_t i = 0; i < res->records_len; i++)
        {
            printf("[RES]        TXT[%zu]: %.*s \n", i,
               (int)res->records[i].property.bin.size, 
                    res->records[i].property.bin.value);
        }
        return er_ok;
    default:
        dbg_assert(0, "Unexpected");
        return er_fatal;
    }

    if (!(flags & pal_sd_result_removed) && ctx->recursive)
        return pal_sdbrowser_browse(ctx->browser, rec->service_name,
            rec->service_type, rec->domain, prx_itf_index_all);
    else
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
        result = pal_sdclient_create(&ctx->client);
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
        pal_sdclient_release(ctx->client);
    
    signal_wait(signal, 2000);

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
