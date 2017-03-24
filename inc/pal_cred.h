// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _pal_cred_h_
#define _pal_cred_h_

#include "common.h"

#include "azure_c_shared_utility/strings.h"

// 
// An interface to a platform specific secret storage that
// enables the connection string class to protect any long 
// term shared access key secret, and a hardware or OS 
// specific way to create short term tokens. Keys are imported
// into a container (e.g. TPM) and a persisted handle is 
// returned. The handle can be used to sign buffers. While
// secrets are long lived, it is possible that old secrets
// accumulate, so a robust implementation will limit the 
// number of keys and store them as a persisted FIFO.
//

//
// Initialize credential store 
//
decl_public_0(int32_t, pal_cred_init,
    void
);

//
// Import credential blob and return an identifying handle
//
decl_public_5(int32_t, pal_cred_import,
    const char*, key_name,
    void*, key_val,
    size_t, key_len,
    bool, persist,
    STRING_HANDLE*, handle
);

//
// hmac sha256 using credential blob identified by passed handle
//
decl_public_5(int32_t, pal_cred_hmac_sha256,
    STRING_HANDLE, handle,
    const void*, buf,
    size_t, buf_len,
    void*, sig,
    size_t, sig_len
);

//
// Delete credential from underlying storage
//
decl_public_1(void, pal_cred_remove,
    STRING_HANDLE, handle
);

//
// Release all global credential store resources
//
decl_public_0(void, pal_cred_deinit,
    void
);

#endif // _pal_cred_h_