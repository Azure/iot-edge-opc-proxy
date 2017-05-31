// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os.h"
#include "util_mem.h"
#include "pal_err.h"
#include "pal_rand.h"

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
    int32_t result;
    int returned;
#if !defined(SYS_getrandom)
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0)
    {
        result = errno;
        return pal_os_to_prx_error(result);
    }
#endif
    result = er_ok;
    while (len > 0)
    {
#if defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
        memset(buf, 0xab, len);
        returned = len;
#elif !defined(SYS_getrandom)
        returned = read(fd, buf, len);
#else
        returned = (int)syscall(SYS_getrandom, buf, len, 0);
#endif
        if (returned < 1)
        {
            returned = errno;
            if (returned != EAGAIN &&
                returned != EINTR)
            {
                result = pal_os_to_prx_error(returned);
                break;
            }
            continue;
        }
        len -= returned;
        buf = ((char*)buf) + returned;
    }
#if !defined(SYS_getrandom)
    close(fd);
#endif
    return result;
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
