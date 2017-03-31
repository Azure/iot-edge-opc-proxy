// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os.h"
#include "util_mem.h"
#include "util_misc.h"
#include "util_string.h"
#include "pal_cred.h"

#if !defined(__HAVE_LIBSECRET) && defined(UNIT_TEST)
#define __HAVE_LIBSECRET 1
#endif

#if !defined(__HAVE_LIBSECRET)
//
// Gnome keyring is deprecated, but 
// available on older distributions. 
// Use libsecret if available.
//
#include <gnome-keyring.h>
#include <gnome-keyring-result.h>
#include <gnome-keyring-memory.h>

#define g_error_t \
    GnomeKeyringResult
#define g_is_error(e) \
    (e != GNOME_KEYRING_RESULT_OK)
#define g_error_msg(e) \
    gnome_keyring_result_to_message(e)
#define g_error_free(e) (void)0

#define SecretSchema \
    GnomeKeyringPasswordSchema

#define secret_password_free \
    gnome_keyring_free_password
#define secure_memory_alloc \
    gnome_keyring_memory_alloc
#define secure_memory_free \
    gnome_keyring_memory_free

#else // __HAVE_LIBSECRET || UNIT_TEST
#if !defined(UNIT_TEST)
#include <libsecret/secret.h>
#if 0
#define secret_password_lookup_sync \
    secret_password_lookup_nonpageable_sync
#endif
#endif

#define g_error_t \
    GError*
#define g_is_error(e) \
    (e != NULL)
#define g_error_msg(e) \
    (e->message)

#define secure_memory_alloc \
    mem_alloc
#define secure_memory_free \
    mem_free

#endif // __HAVE_LIBSECRET 

#include "azure_c_shared_utility/base64.h"
#include "azure_c_shared_utility/hmacsha256.h"
#include "azure_c_shared_utility/buffer_.h"


// 
// Proxy credential schema
//
static const SecretSchema* pal_cred_schema(
    void
)
{
#define PASSWORD_TARGET "target"
    static const SecretSchema schema =
    {
#if !defined(__HAVE_LIBSECRET)
        GNOME_KEYRING_ITEM_GENERIC_SECRET,
        {
            { PASSWORD_TARGET, 
                GNOME_KEYRING_ATTRIBUTE_TYPE_STRING },
            { NULL, 0 }
        }
#else
        MODULE_NAME, SECRET_SCHEMA_NONE,
        {
            { PASSWORD_TARGET, 
                SECRET_SCHEMA_ATTRIBUTE_STRING },
            { "NULL", 0 },
        }
#endif
    };
    return &schema;
}

//
// Initialize credential store
//
int32_t pal_cred_init(
    void
)
{
    return er_ok;
}

//
// Import password into credential store
//
int32_t pal_cred_import(
    const char* key_name,
    void* key_val,
    size_t key_len,
    bool persist,
    STRING_HANDLE* created
)
{
    int32_t result;
    g_error_t error;
    STRING_HANDLE handle, key = NULL;

    if (!created || !key_val)
        return er_fault;
    if (!key_len)
        return er_arg;
    do
    {
        result = er_out_of_memory;
        if (!key_name) // Generate random key name 
            handle = STRING_construct_random(16);
        else
            handle = STRING_construct(key_name);
        if (!handle)
            break;
        key = STRING_construct_base16((uint8_t*)key_val, key_len);
        if (!key)
            break;

#if defined(__HAVE_LIBSECRET)
        error = NULL;
        (void)secret_password_store_sync(pal_cred_schema(),
            persist ? SECRET_COLLECTION_DEFAULT : SECRET_COLLECTION_SESSION,
            key_name, STRING_c_str(key), NULL, &error,
            PASSWORD_TARGET, key_name, NULL);
#else
        error = gnome_keyring_store_password_sync(pal_cred_schema(),
            persist ? GNOME_KEYRING_DEFAULT : GNOME_KEYRING_SESSION,
            key_name, STRING_c_str(key), PASSWORD_TARGET, key_name, NULL);
#endif
        if (g_is_error(error))
        {
            log_error(NULL, "Error storing password: %s", 
                g_error_msg(error));
            g_error_free(error);
            result = er_fatal;
            break;
        }
        *created = handle;
        handle = NULL;
        result = er_ok;
        break;
    } 
    while (0);

    if (handle)
        STRING_delete(handle);
    //
    // Always remove passed in secret from memory even though
    // the import path is not considered a safe path for keys
    //
    memset(key_val, 0, key_len);
    if (key)
    {
        key_val = (void*)STRING_c_str(key);
        key_len = STRING_length(key);
         memset(key_val, 0, key_len);
        STRING_delete(key);
    }
    return result;
}

//
// Delete credential key 
//
void pal_cred_remove(
    STRING_HANDLE handle
)
{
    g_error_t error;
    if (!handle)
        return;
#if defined(__HAVE_LIBSECRET)
    error = NULL;
    (void)secret_password_clear_sync(
        pal_cred_schema(), NULL, &error,
#else
    error = gnome_keyring_delete_password_sync(
        pal_cred_schema(),
#endif
        PASSWORD_TARGET, STRING_c_str(handle), NULL);
    if (g_is_error(error))
    {
        log_info(NULL, "Failed deleting credential (%s)",
            g_error_msg(error));
        g_error_free(error);
    }
}

//
// Uses the persisted or protected key to create hmac
//
int32_t pal_cred_hmac_sha256(
    STRING_HANDLE handle,
    const void* buf,
    size_t buf_len,
    void* sig,
    size_t sig_len
)
{
    int32_t result;
    g_error_t error;
    BUFFER_HANDLE hash;
    gchar* password = NULL;
    uint8_t* key_val = NULL;
    size_t key_len;

    if (!handle || !buf || !sig)
        return er_fault;
    if (!buf_len || sig_len < 32)
        return er_arg;

    hash = BUFFER_new();
    if (!hash)
        return er_out_of_memory;
    do
    {
        // Retrieve base16 key 
#if defined(__HAVE_LIBSECRET)
        error = NULL;
        password = secret_password_lookup_sync(
            pal_cred_schema(), NULL, &error,
#else
        error = gnome_keyring_find_password_sync(
            pal_cred_schema(), &password, 
#endif
            PASSWORD_TARGET, STRING_c_str(handle), NULL);
        if (g_is_error(error))
        {
            log_error(NULL, "Error retrieving password: %s",
                g_error_msg(error));
            result = er_fatal;
            g_error_free(error);
            break;
        }

        if (!password)
        {
            log_error(NULL, "Password for %s was not found!",
                STRING_c_str(handle));
            result = er_not_found;
            break;
        }

        // Decode base 16 into secure key buffer
        result = er_out_of_memory;
        key_len = strlen(password) / 2;
        key_val = (uint8_t*)secure_memory_alloc(key_len);
        if (!key_val)
            break;
        result = string_base16_to_byte_array(password, 
            &key_val, &key_len);
        if (result != er_ok)
            break;

        // Compute and return hash showing we have the key
        if (HMACSHA256_OK != HMACSHA256_ComputeHash(key_val, key_len, 
            (unsigned char*)buf, buf_len, hash))
        {
            result = er_invalid_format;
            break;
        }
        buf_len = BUFFER_length(hash);
        memcpy(sig, BUFFER_u_char(hash), min(sig_len, buf_len));
        result = er_ok;
        break;
    } 
    while (0);
    
    BUFFER_delete(hash);
    // Ensure we remove the key from memory asap
    if (password)
        secret_password_free(password);
    if (key_val)
        secure_memory_free(key_val);
    return result;
}

//
// Destroy credential store
//
void pal_cred_deinit(
    void
)
{
    // Todo: Close proxy collection 
}
