// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os.h"
#include "util_mem.h"
#include "util_string.h"
#include "pal_cred.h"
#include "pal_err.h"

#include "azure_c_shared_utility/hmacsha256.h"
#include "azure_c_shared_utility/buffer_.h"

#if !defined(UNIT_TEST)
#include <WinCred.h>
#endif

//
// Init credential store
//
int32_t pal_cred_init(
    void
)
{
    return er_ok;
}

//
// Protect a buffer
//
int32_t pal_cred_protect(
    void* key_val,
    size_t key_len,
    STRING_HANDLE* handle
)
{
    int32_t result;
    STRING_HANDLE base64 = NULL;
    size_t protected_len;
    unsigned char* protected_buf = NULL;

    dbg_assert_ptr(key_val);
    dbg_assert_ptr(handle);
#define HANDLE_PREFIX "b64:"
    do
    {
        protected_len = key_len;
        protected_len += sizeof(size_t);
        if (protected_len % CRYPTPROTECTMEMORY_BLOCK_SIZE)
        {
            // Round up to multiple of CRYPTPROTECTMEMORY_BLOCK_SIZE
            protected_len = (protected_len /
                CRYPTPROTECTMEMORY_BLOCK_SIZE + 1)
                * CRYPTPROTECTMEMORY_BLOCK_SIZE;
        }

        result = er_out_of_memory;
        protected_buf = (unsigned char*)mem_alloc(protected_len);
        if (!protected_buf)
            break;
        
        // Store size of key
        *(size_t*)protected_buf = key_len;
        memcpy(protected_buf + sizeof(size_t), key_val, key_len);
        if (!CryptProtectMemory(protected_buf, (DWORD)protected_len,
            CRYPTPROTECTMEMORY_SAME_LOGON))
        {
            result = pal_os_last_error_as_prx_error();
            break;
        }

        result = er_out_of_memory;
        base64 = STRING_construct_base64(protected_buf, protected_len);
        if (!base64)
            break;
        *handle = STRING_construct(HANDLE_PREFIX);
        if (!*handle)
            break;
        if (0 != STRING_concat_with_STRING(*handle, base64))
        {
            STRING_delete(*handle);
            break;
        }
        result = er_ok;
        break;
    } while (0);

    if (protected_buf)
    {
        RtlSecureZeroMemory(protected_buf, protected_len);
        mem_free(protected_buf);
    }
    if (base64)
        STRING_delete(base64);

    // Always remove passed in secret from memory
    RtlSecureZeroMemory(key_val, key_len);
    return result;
}

//
// Import credential key blob
//
int32_t pal_cred_import(
    const char* key_name,
    void* key_val,
    size_t key_len,
    bool persist,
    STRING_HANDLE* handle
)
{
    CREDENTIALA cred;
    chk_arg_fault_return(handle);
    chk_arg_fault_return(key_val);
    if (!key_len)
        return er_arg;

    memset(&cred, 0, sizeof(cred));
    if (key_name && persist)
    {
        cred.Type = CRED_TYPE_GENERIC;
        cred.TargetName = (LPSTR)key_name;
        cred.CredentialBlobSize = (DWORD)key_len;
        cred.CredentialBlob = (LPBYTE)key_val;
        cred.Persist = CRED_PERSIST_LOCAL_MACHINE;
        cred.UserName = (LPSTR)MODULE_NAME;

        *handle = STRING_construct(key_name); // Lookup using the key name as target
        if (!*handle)
            return er_out_of_memory;
        if (CredWriteA(&cred, 0))
        {
            // Always remove passed in secret from memory
            RtlSecureZeroMemory(key_val, key_len);
            return er_ok;
        }
        log_info(NULL, "Failed persisting key as credential (%d) - encrypting "
            "it using session key instead...", pal_os_last_error_as_prx_error());
        STRING_delete(*handle);
    }
    return pal_cred_protect(key_val, key_len, handle);
}

//
// Delete credential key blob
//
void pal_cred_remove(
    STRING_HANDLE handle
)
{
    const char* target;
    if (!handle)
        return;
    
    target = STRING_c_str(handle);
    if (string_starts_with_nocase(target, HANDLE_PREFIX))
        return; // not a persisted credential key
    
    if (!CredDeleteA(target, CRED_TYPE_GENERIC, 0))
    {
        log_error(NULL, "Failed deleting credential (%d)",
            pal_os_last_error_as_prx_error());
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
    BUFFER_HANDLE hash;
    unsigned char* key_val, *protected_buf = NULL;
    const char* target;
    size_t key_len;
    CREDENTIALA* pcred = NULL;

    chk_arg_fault_return(handle);
    chk_arg_fault_return(buf);
    chk_arg_fault_return(sig);
    if (!buf_len || sig_len < 32)
        return er_arg;

    hash = BUFFER_new();
    if (!hash)
        return er_out_of_memory;
    do
    {
        key_val = NULL;
        key_len = 0;

        // Retrieve key
        target = STRING_c_str(handle);
        if (!string_starts_with_nocase(target, HANDLE_PREFIX))
        {
            if (!CredReadA(target, CRED_TYPE_GENERIC, 0, &pcred))
            {
                result = pal_os_last_error_as_prx_error();
                break;
            }
            key_len = pcred->CredentialBlobSize;
            key_val = pcred->CredentialBlob;
        }
        else
        {
            result = string_base64_to_byte_array(
                target + strlen(HANDLE_PREFIX), &protected_buf, &key_len);
            if (result != er_ok)
                break;

            // Decrypt the key
            if (!CryptUnprotectMemory(protected_buf, (DWORD)key_len,
                CRYPTPROTECTMEMORY_SAME_LOGON))
            {
                result = pal_os_last_error_as_prx_error();
                break;
            }
            key_len = *(size_t*)protected_buf;
            key_val = protected_buf + sizeof(size_t);
        }

        //
        // Compute and return hash - since we needed to unprotect the key we
        // can use our embedded api which works. A TPM implementation would
        // never unprotect the key after import, and thus a hw hmac routine 
        // would be called...
        //
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
    if (key_len > 0 && key_val)
        RtlSecureZeroMemory(key_val, key_len);
    if (pcred)
        CredFree(pcred);
    else if (protected_buf)
        mem_free(protected_buf);
    return result;
}

//
// Destroy credential store
//
void pal_cred_deinit(
    void
)
{
    // no op
}
