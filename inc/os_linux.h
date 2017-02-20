// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _os_linux_h_
#define _os_linux_h_

//
// OS layer api, used by pal implementation and clients
//
#define _GNU_SOURCE
#define __USE_GNU
#define __USE_MISC // needed for SOL_TCP in <netinet/tcp.h>
#include "os_posix.h"

#endif // _os_linux_h_
