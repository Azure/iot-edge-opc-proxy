// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "io_token.h"
#include "util_string.h"
#include "prx_config.h"
#include "pal_time.h"
#include "pal_cred.h"

#include "azure_c_shared_utility/agenttime.h"
#include "azure_c_shared_utility/refcount.h"
#include "azure_c_shared_utility/urlencode.h"
#include "azure_c_shared_utility/buffer_.h"

#if !defined(DEBUG)
#define DEFAULT_RENEWAL_TIMEOUT_SEC 8 * 60 * 60 
#else
#define DEFAULT_RENEWAL_TIMEOUT_SEC     30 * 60 
#endif // !defined(DEBUG)

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
// Build sas token
//
static int32_t io_sas_token_create(
    STRING_HANDLE key_or_handle,
    const char* scope,
    const char* keyname,
    size_t expiry,
    STRING_HANDLE* created
)
{
    int32_t result;
    uint8_t hmac[32];

    STRING_HANDLE hash_string;
    STRING_HANDLE token = NULL;
    BUFFER_HANDLE hash = NULL;
    STRING_HANDLE signature = NULL;
    STRING_HANDLE url_encode = NULL;

    hash_string = STRING_new();
    if (!hash_string)
        return er_out_of_memory;
    do
    {
        // Create signature
        result = er_out_of_memory;
        if (0 != STRING_concat(hash_string, scope) ||
            0 != STRING_concat(hash_string, "\n") ||
            0 != STRING_concat_int(hash_string, (int)expiry, 10))
            break;

        result = pal_cred_hmac_sha256(key_or_handle, 
            STRING_c_str(hash_string), STRING_length(hash_string), 
            hmac, sizeof(hmac));
        if (result != er_ok)
            break;

        // url encode base64 hmac
        result = er_out_of_memory;
        signature = STRING_construct_base64(hmac, sizeof(hmac));
        if (!signature)
            break;
        url_encode = URL_Encode(signature);
        if (!url_encode)
            break;
        token = STRING_construct("SharedAccessSignature sr=");
        if (!token)
            break;
        if (0 != STRING_concat(token, scope) ||
            0 != STRING_concat(token, "&sig=") ||
            0 != STRING_concat_with_STRING(token, url_encode) ||
            0 != STRING_concat(token, "&se=") ||
            0 != STRING_concat_int(token, (int)expiry, 10) ||
            0 != STRING_concat(token, "&skn=") ||
            0 != STRING_concat(token, keyname))
            break;

        *created = token;
        token = NULL;
        result = er_ok;
        break;
    } while (0);

    STRING_delete(hash_string);

    if (token)
        STRING_delete(token);
    if (signature)
        STRING_delete(signature);
    if (url_encode)
        STRING_delete(url_encode);
    if (hash)
        BUFFER_delete(hash);
    return result;
}

//
// Destroy the iothub provider context, i.e. the shared access key
//
static void io_sas_token_provider_on_release(
    io_sas_token_provider_t* provider
)
{
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
    io_sas_token_provider_t* provider,
    io_token_provider_t** clone
)
{
    dbg_assert_ptr(provider);
    INC_REF(io_sas_token_provider_t, provider);
    *clone = &provider->funcs;
    return er_ok;
}

//
// Get properties
//
static const char* io_sas_token_provider_on_get_property(
    io_sas_token_provider_t* provider,
    io_token_property_id_t id
)
{
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
    io_sas_token_provider_t* provider,
    STRING_HANDLE* token,
    int64_t* ttl
)
{
    int32_t result;
    chk_arg_fault_return(token);
    chk_arg_fault_return(ttl);

    dbg_assert_ptr(provider);
    // Create token
    result = io_sas_token_create(provider->shared_access_key, 
        STRING_c_str(provider->scope), STRING_c_str(provider->policy), 
        (int32_t)get_time(NULL) + provider->ttl_in_seconds, token);
    if (result == er_ok)
    {
        // Leave 20 % to renew
        *ttl = (provider->ttl_in_seconds * 800);
    }
    return result;
}

//
// Destroy the iothub provider context, i.e. the shared access key
//
static void io_passthru_token_provider_on_release(
    io_passthru_token_provider_t* provider
)
{
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
    io_passthru_token_provider_t* provider,
    io_token_provider_t** clone
)
{
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
    io_passthru_token_provider_t* provider,
    STRING_HANDLE* token,
    int64_t* ttl
)
{
    dbg_assert_ptr(provider);
    *token = STRING_clone(provider->shared_access_token);
    if (!*token)
        return er_out_of_memory;
    *ttl = 0x0ffffff;
    return er_ok;
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
        provider->funcs.on_new_token = (io_token_provider_on_new_token_t)
            io_passthru_token_provider_on_new_token; 
        provider->funcs.on_clone = (io_token_provider_on_clone_t)
            io_passthru_token_provider_on_clone; 
        provider->funcs.on_get_property = (io_token_provider_on_get_property_t)
            io_passthru_token_provider_on_get_property;
        provider->funcs.on_release = (io_token_provider_on_release_t)
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
// Create iothub sas token provider
//
int32_t io_iothub_token_provider_create(
    const char* policy,
    STRING_HANDLE key_or_handle,
    const char* scope,
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
        provider->funcs.on_new_token = (io_token_provider_on_new_token_t)
            io_token_provider_on_new_iothub_token;
        provider->funcs.on_clone = (io_token_provider_on_clone_t)
            io_sas_token_provider_on_clone;
        provider->funcs.on_get_property = (io_token_provider_on_get_property_t)
            io_sas_token_provider_on_get_property;
        provider->funcs.on_release = (io_token_provider_on_release_t)
            io_sas_token_provider_on_release;

        provider->scope =
            STRING_construct(scope);
        provider->policy =
            STRING_construct(policy ? policy : "");
        provider->shared_access_key =
            STRING_clone(key_or_handle);

        if (!provider->scope ||
            !provider->policy ||
            !provider->shared_access_key)
        {
            result = er_out_of_memory;
            break;
        }

        provider->ttl_in_seconds = __prx_config_get_int(prx_config_key_token_ttl,
            DEFAULT_RENEWAL_TIMEOUT_SEC);
        *created = &provider->funcs;
        return er_ok;
    } while (0);

    io_sas_token_provider_on_release(provider);
    return result;
}
