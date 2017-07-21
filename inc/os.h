// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _os_h_
#define _os_h_

//
// OS layer
//
#if defined (UNIT_TEST)
#include "os_mock.h"
#elif defined(_WIN32)
#include "os_win.h"
#elif defined(__linux__)
#include "os_linux.h"
#elif defined(POSIX)
#include "os_posix.h"
#else
#define OS_H os_##PAL
#define OS_H_str #OS_H
#include OS_H_str
#endif

#endif // _os_h_