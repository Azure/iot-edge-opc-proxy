// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _os_linux_h_
#define _os_linux_h_

//
// Linux OS api used in PAL implementation
//
#define _DEFAULT_SOURCE 1
#define _BSD_SOURCE 1
#define _SVID_SOURCE 1
#define _XOPEN_SOURCE 600
#define _GNU_SOURCE 1
#define _FORTIFY_SOURCE 1
#define _ISOC99_SOURCE 1
#include <features.h>

#include "os_posix.h"

#endif // _os_linux_h_