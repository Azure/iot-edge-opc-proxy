// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _xio_sk_h_
#define _xio_sk_h_

#include "common.h"

//
// Defined option strings for xio layer
//
#define xio_opt_scheduler "scheduler"
#define xio_opt_flow_ctrl "xon"

//
// Passed to xio for socket interface
//
typedef struct xio_sk_config
{
    const char* hostname;
    int32_t port;
    void* reserved; 
}
xio_sk_config_t;

#endif // _xio_sk_h_