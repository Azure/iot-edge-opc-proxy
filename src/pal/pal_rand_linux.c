// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.


#include "util_mem.h"
#include "pal_err.h"
#include "pal_rand.h"
#include "os.h"

#if !defined(UNIT_TEST)
#include <sys/syscall.h>
#include <linux/random.h>
#endif

//
// No-op
//
int32_t pal_rand_init(
    void
)
{
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
    int result;
    while (len > 0)
    {
        result = (int)syscall(SYS_getrandom, buf, len, 0);
        if (result < 1)
        {
            result = errno;
            if (result != EAGAIN &&
                result != EINTR)
            {
                return pal_os_to_prx_error(result);
            }
            continue;
        }
        len -= result;
        buf = ((char*)buf) + result;
    }
    return er_ok;
}

//
// No-op
//
void pal_rand_deinit(
    void
)
{
    // no op
}
