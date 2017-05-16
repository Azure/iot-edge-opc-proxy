// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os.h"
#include "util_mem.h"
#include "util_misc.h"
#include "util_string.h"
#include "pal_cred.h"

#include "azure_c_shared_utility/hmacsha256.h"
#include "azure_c_shared_utility/buffer_.h"

//
// Init stub
//
int32_t pal_cred_init(
    void
)
{
    log_info(NULL, "Pal does not provide credential storage support. "
        "Ensure you properly safeguard all secrets on this device!");

    //
    // While cred store is not supported, hmac calc is still performed 
    // using below implementation.
    //
    return er_not_supported;
}

//
// Import stub - no op
//
int32_t pal_cred_import(
    const char* key_name,
    void* key_val,
    size_t key_len,
    bool persist,
    STRING_HANDLE* handle
)
{
    (void)key_name;
    (void)key_val;
    (void)key_len;
    (void)handle;
    (void)persist;
    dbg_assert(0, "Unexpected");
    return er_not_supported;
}

//
// Remove stub - no op
//
void pal_cred_remove(
    STRING_HANDLE handle
)
{
    (void)handle;
    dbg_assert(0, "Unexpected");
}

//
// Create SHA2 hash using a handle which the base 64 encoded key.
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
    unsigned char* raw = NULL;
    size_t len;
    BUFFER_HANDLE hash;

    if (!handle || !buf || !sig)
        return er_fault;
    if (!buf_len || sig_len < 32)
        return er_arg;

    hash = BUFFER_new();
    if (!hash)
        return er_out_of_memory;
    do
    {
        result = string_base64_to_byte_array(STRING_c_str(handle),
            &raw, &len);
        if (result != er_ok)
            break;
        if (HMACSHA256_OK != HMACSHA256_ComputeHash(
            raw, len, (unsigned char*)buf, buf_len, hash))
        {
            result = er_invalid_format;
            break;
        }
        len = BUFFER_length(hash);
        memcpy(sig, BUFFER_u_char(hash), min(sig_len, len));
        result = er_ok;
        break;
    } while (0);
    BUFFER_delete(hash);
    if (raw)
        mem_free(raw);
    return result;
}

//
// Destroy stub
//
void pal_cred_deinit(
    void
)
{
    // no-op 
}
