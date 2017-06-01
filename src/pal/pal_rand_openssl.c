// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os.h"
#include "util_mem.h"
#include "util_log.h"
#include "pal_err.h"
#include "pal_rand.h"

#if !defined(UNIT_TEST)
#include <openssl/rand.h>
#include <openssl/err.h>
#endif

//
// Helper to log ssl error
//
static void log_ssl_error(
    void
)
{
    char buf[256];
    u_long err;
    while(true)
    {
        err = ERR_get_error();
        if (err == 0)
            return;
        buf[0] = 0;
        buf[sizeof(buf) - 1] = 0;
        ERR_error_string_n(err, buf, sizeof(buf) - 1);
        log_error(NULL, "%s\n", buf);
    }
}

//
// Init openssl rand
//
int32_t pal_rand_init(
    void
)
{
    // Assume already inited
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
    if (!buf && len > 0)
        return er_fault;
    if (!len)
        return er_ok;

#if defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
    memset(buf, 0x05, len);
#else
    if (!RAND_bytes((unsigned char*)buf, (int)len))
    {
        log_ssl_error();
        return er_fatal;
    }
#endif
    return er_ok;
}

//
// Deinit openssl rand
//
void pal_rand_deinit(
    void
)
{
    // Assume otherwise deinited
}
