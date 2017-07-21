// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _os_linux_h_
#define _os_linux_h_

//
// Linux OS api used in PAL implementation
//

// Include default apis ...
#if !defined(_DEFAULT_SOURCE)
#define _DEFAULT_SOURCE 1
#define _BSD_SOURCE 1
#define _SVID_SOURCE 1
#endif

// ... plus xopen and posix apis
#if !defined(_XOPEN_SOURCE) || (_XOPEN_SOURCE < 600)
#undef _XOPEN_SOURCE
#define _XOPEN_SOURCE 600
#endif

// ... and any linux specific ones
#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE 1
#endif

// Ensure safe string manipulation ...
#if !defined(_FORTIFY_SOURCE)
#define _FORTIFY_SOURCE 1
#endif

// ... and C'99 compliant source
#if !defined(_ISOC99_SOURCE)
#define _ISOC99_SOURCE 1
#endif

#include <features.h>
#include "os_posix.h"

#if !defined(TCP_USER_TIMEOUT)
#define TCP_USER_TIMEOUT 18
#endif

#endif // _os_linux_h_