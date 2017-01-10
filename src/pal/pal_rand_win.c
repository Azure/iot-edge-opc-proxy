// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.


#include "util_mem.h"
#include "pal_err.h"
#include "pal_rand.h"
#include "os_win.h"

static HCRYPTPROV h_prov = 0;

//
// Acquire crypto provider
//
int32_t pal_rand_init(
    void
)
{
    if (h_prov)
        return er_bad_state;
    if (!CryptAcquireContextA(&h_prov, NULL, NULL, PROV_RSA_FULL,
            CRYPT_VERIFYCONTEXT | CRYPT_SILENT))
        return pal_os_last_error_as_prx_error();
    return er_ok;
}

//
// Fill buffer with random data
//
int32_t pal_rand_fill(
    void* buf,
    size_t len
)
{
    if (!CryptGenRandom(h_prov, (DWORD)len, (BYTE*)buf))
        return pal_os_last_error_as_prx_error();
    return er_ok;
}

//
// Release crypto provider
//
void pal_rand_deinit(
    void
)
{
    if (!h_prov)
        return;
    (void)CryptReleaseContext(h_prov, 0);
    h_prov = 0;
}
