// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "io_token.h"
#include "util_string.h"
#include "prx_config.h"
#include "pal_time.h"

#include "azure_c_shared_utility/refcount.h"
#include "azure_c_shared_utility/sastoken.h"

#define DEFAULT_RENEWAL_TIMEOUT_SEC 10 * 60

//
// Concrete sas token provider
//
typedef struct io_sas_token_provider
{
    io_token_provider_t funcs;
    const char* type;
    STRING_HANDLE scope;
    STRING_HANDLE policy;
    STRING_HANDLE shared_access_key;
    int32_t ttl_in_seconds;
}
io_sas_token_provider_t;

DEFINE_REFCOUNT_TYPE(io_sas_token_provider_t);

//
// Concrete sas token provider
//
typedef struct io_passthru_token_provider
{
    io_token_provider_t funcs;
    STRING_HANDLE shared_access_token;
}
io_passthru_token_provider_t;

DEFINE_REFCOUNT_TYPE(io_passthru_token_provider_t);

//
// Destroy the iothub provider context, i.e. the shared access key
//
static void io_sas_token_provider_on_release(
    void* context
)
{
    io_sas_token_provider_t* provider = (io_sas_token_provider_t*)context;
    dbg_assert_ptr(provider);
    if (DEC_REF(io_sas_token_provider_t, provider) == DEC_RETURN_ZERO)
    {
        if (provider->policy)
            STRING_delete(provider->policy);
        if (provider->scope)
            STRING_delete(provider->scope);
        if (provider->shared_access_key)
            STRING_delete(provider->shared_access_key);

        REFCOUNT_TYPE_FREE(io_sas_token_provider_t, provider);
    }
}

//
// Clone provider
//
static int32_t io_sas_token_provider_on_clone(
    void* context,
    io_token_provider_t** clone
)
{
    io_sas_token_provider_t* provider = (io_sas_token_provider_t*)context;
    dbg_assert_ptr(provider);
    INC_REF(io_sas_token_provider_t, provider);
    *clone = &provider->funcs;
    return er_ok;
}

//
// Get properties
//
static const char* io_sas_token_provider_on_get_property(
    void* context,
    io_token_property_id_t id
)
{
    io_sas_token_provider_t* provider = (io_sas_token_provider_t*)context;
    dbg_assert_ptr(provider);
    switch (id)
    {
#define CBS_DEFAULT_TOKEN_TYPE "servicebus.windows.net:sastoken"
    case io_token_property_type:    
        return CBS_DEFAULT_TOKEN_TYPE;
    case io_token_property_scope:
        return STRING_c_str(provider->scope);
    case io_token_property_policy:
        return STRING_c_str(provider->policy);
    default:
        return NULL;
    }
}

//
// Generate new token for iot hub
//
static int32_t io_token_provider_on_new_iothub_token(
    void* context,
    STRING_HANDLE* token,
    int64_t* ttl
)
{
    io_sas_token_provider_t* provider = (io_sas_token_provider_t*)context;
    dbg_assert_ptr(provider);
    dbg_assert_ptr(token);
    dbg_assert_ptr(ttl);

    *token = SASToken_Create(provider->shared_access_key, provider->scope, 
        provider->policy, (int32_t)get_time(NULL) + provider->ttl_in_seconds);
    if (!*token)
        return er_out_of_memory;
    // Leave 20 % to renew
    *ttl = (provider->ttl_in_seconds * 800);
    return er_ok;
}

//
// Helper to create token provider object
//
static int32_t io_sas_token_provider_create(
    const char* shared_access_key,
    const char* scope,
    const char* policy,
    int32_t ttl_in_seconds,
    io_token_provider_on_new_token_t on_new_token,
    io_token_provider_t** created
)
{
    int32_t result;
    io_sas_token_provider_t* provider;

    provider = REFCOUNT_TYPE_CREATE(io_sas_token_provider_t);
    if (!provider)
        return er_out_of_memory;
    memset(provider, 0, sizeof(io_sas_token_provider_t));
    do
    {
        provider->funcs.context =
            provider;
        provider->funcs.on_new_token =
            on_new_token;
        provider->funcs.on_clone =
            io_sas_token_provider_on_clone;
        provider->funcs.on_get_property =
            io_sas_token_provider_on_get_property;
        provider->funcs.on_release =
            io_sas_token_provider_on_release;

        provider->scope = 
            STRING_construct(scope);
        provider->policy = 
            STRING_construct(policy ? policy : "");
        provider->shared_access_key = 
            STRING_construct(shared_access_key);

        if (!provider->scope || 
            !provider->policy || 
            !provider->shared_access_key)
        {
            result = er_out_of_memory;
            break;
        }

        provider->ttl_in_seconds = ttl_in_seconds;
        *created = &provider->funcs;
        return er_ok;
    } while (0);

    io_sas_token_provider_on_release(provider);
    return result;
}
//
// Destroy the iothub provider context, i.e. the shared access key
//
static void io_passthru_token_provider_on_release(
    void* context
)
{
    io_passthru_token_provider_t* provider = (io_passthru_token_provider_t*)context;
    dbg_assert_ptr(provider);

    if (DEC_REF(io_passthru_token_provider_t, provider) == DEC_RETURN_ZERO)
    {
        if (provider->shared_access_token)
            STRING_delete(provider->shared_access_token);

        REFCOUNT_TYPE_FREE(io_passthru_token_provider_t, provider);
    }
}

//
// Clone provider
//
static int32_t io_passthru_token_provider_on_clone(
    void* context,
    io_token_provider_t** clone
)
{
    io_passthru_token_provider_t* provider = (io_passthru_token_provider_t*)context;
    dbg_assert_ptr(provider);
    INC_REF(io_passthru_token_provider_t, provider);
    *clone = &provider->funcs;
    return er_ok;
}

//
// Get properties
//
static const char* io_passthru_token_provider_on_get_property(
    void* context,
    io_token_property_id_t id
)
{
    dbg_assert_ptr(context);
    (void)context;
    switch (id)
    {
    case io_token_property_type:
        return NULL; // TODO
    case io_token_property_scope:
        return NULL; // TODO
    case io_token_property_policy:
        return NULL; // TODO
    default:
        return NULL;
    }
}

//
// Generate new token for iot hub
//
static int32_t io_passthru_token_provider_on_new_token(
    void* context,
    STRING_HANDLE* token,
    int64_t* ttl
)
{
    io_passthru_token_provider_t* provider = (io_passthru_token_provider_t*)context;
    dbg_assert_ptr(provider);
    *token = STRING_clone(provider->shared_access_token);
    if (!*token)
        return er_out_of_memory;
    *ttl = 0x0ffffff;
    return er_ok;
}

//
// Create pass thru token provider 
//
int32_t io_passthru_token_provider_create(
    const char* shared_access_token,
    io_token_provider_t** created
)
{
    int32_t result;
    io_passthru_token_provider_t* provider;

    provider = REFCOUNT_TYPE_CREATE(io_passthru_token_provider_t);
    if (!provider)
        return er_out_of_memory;
    memset(provider, 0, sizeof(io_passthru_token_provider_t));
    do
    {
        provider->funcs.context =
            provider;
        provider->funcs.on_new_token =
            io_passthru_token_provider_on_new_token;
        provider->funcs.on_clone =
            io_passthru_token_provider_on_clone;
        provider->funcs.on_get_property =
            io_passthru_token_provider_on_get_property;
        provider->funcs.on_release =
            io_passthru_token_provider_on_release;

        provider->shared_access_token = STRING_construct(shared_access_token);
        if (!provider->shared_access_token)
        {
            result = er_out_of_memory;
            break;
        }

        *created = &provider->funcs;
        return er_ok;
    } while (0);

    io_passthru_token_provider_on_release(provider);
    return result;
}

//
// Check whether 2 providers are equivalent based on properties
//
bool io_token_provider_is_equivalent(
    io_token_provider_t* that,
    io_token_provider_t* other
)
{
    const char* that_prop;
    const char* other_prop;

    if (that == other)
        return true;
    if (!that || !other)
        return false;
    dbg_assert_ptr(that->on_get_property);
    dbg_assert_ptr(other->on_get_property);
    for (int i = 0; i < io_token_property_unknown; i++)
    {
        that_prop = that->on_get_property(
            that->context, (io_token_property_id_t)i);
        other_prop = other->on_get_property(
            other->context, (io_token_property_id_t)i);
        if (!that_prop && !other_prop)
            continue;
        if (0 == string_compare_nocase(that_prop, other_prop))
            continue;
        return false;
    }
    return true;
}

//
// Create iothub sas token provider
//
int32_t io_iothub_token_provider_create(
    const char* policy,
    const char* shared_access_key,
    const char* scope,
    io_token_provider_t** provider
)
{
    int32_t timeout = __prx_config_get_int(prx_config_key_token_ttl, 
        DEFAULT_RENEWAL_TIMEOUT_SEC);
    return io_sas_token_provider_create(shared_access_key, scope, policy, timeout,
        io_token_provider_on_new_iothub_token, provider);
}

