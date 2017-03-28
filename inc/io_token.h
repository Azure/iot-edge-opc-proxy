// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _io_token_h_
#define _io_token_h_

#include "common.h"
#include "util_dbg.h"
#include "azure_c_shared_utility/strings.h"

//
// Token provider provides token for a link or connection
//
typedef struct io_token_provider io_token_provider_t;

//
// Called to generate token for a link or connection
//
typedef int32_t (*io_token_provider_on_new_token_t)(
    void* context,
    STRING_HANDLE* token,
    int64_t* expiry
    );

//
// Called to compare provider context
//
typedef bool (*io_token_provider_on_is_equal_t)(
    void* that,
    void* other
    );

//
// List of properties a provider has that define how tokens are issued
//
typedef enum io_token_property_id
{
    io_token_property_type,
    io_token_property_scope,
    io_token_property_policy,
    //...
    io_token_property_unknown
}
io_token_property_id_t;

//
// Get a property of the provider
//
typedef const char* (*io_token_provider_on_get_property_t)(
    void* context,
    io_token_property_id_t prop_id
    );

//
// Called to release a token provider
//
typedef void (*io_token_provider_on_release_t)(
    void* context
    );

//
// Called to clone a token provider
//
typedef int32_t (*io_token_provider_on_clone_t)(
    void* context,
    io_token_provider_t** clone
    );

//
// Token provider interface 
//
struct io_token_provider
{
    io_token_provider_on_get_property_t on_get_property;
    io_token_provider_on_new_token_t on_new_token;
    io_token_provider_on_clone_t on_clone;
    io_token_provider_on_release_t on_release;

    void* context;      // Concrete provider implementation, e.g. tpm
};

//
// Safely clone provider
//
decl_inline_2(int32_t, io_token_provider_clone,
    io_token_provider_t*, provider,
    io_token_provider_t**, clone
)
{
    dbg_assert_ptr(provider);
    dbg_assert_ptr(provider->on_clone);
    return provider->on_clone(provider->context, clone);
}

//
// Check whether 2 providers are equivalent
//
decl_internal_2(bool, io_token_provider_is_equivalent,
    io_token_provider_t*, that,
    io_token_provider_t*, other
);

//
// Safely get property
//
decl_inline_2(const char*, io_token_provider_get_property,
    io_token_provider_t*, provider,
    io_token_property_id_t, id
)
{
    dbg_assert_ptr(provider);
    dbg_assert_ptr(provider->on_get_property);
    return provider->on_get_property(provider->context, id);
}

//
// Safely make new token
//
decl_inline_3(int32_t, io_token_provider_new_token,
    io_token_provider_t*, provider,
    STRING_HANDLE*, token,
    int64_t*, expiry
)
{
    dbg_assert_ptr(provider);
    dbg_assert_ptr(provider->on_new_token);
    return provider->on_new_token(provider->context, token, expiry);
}

//
// Safely release provider
//
decl_inline_1(void, io_token_provider_release,
    io_token_provider_t*, provider
)
{
    dbg_assert_ptr(provider);
    dbg_assert_ptr(provider->on_release);
    provider->on_release(provider->context);
}

//
// Create pass thru token provider 
//
decl_internal_2(int32_t, io_passthru_token_provider_create,
    const char*, shared_access_token,
    io_token_provider_t**, provider
);

//
// Create iothub sas token provider
//
decl_internal_4(int32_t, io_iothub_token_provider_create,
    const char*, policy,
    STRING_HANDLE, key_or_handle,
    const char*, scope,
    io_token_provider_t**, provider
);

#endif // _io_token_h_
