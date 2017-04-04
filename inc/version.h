// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _version_h_
#define _version_h_

#if !defined(MODULE_MAJ_VER)
#define MODULE_MAJ_VER 0
#endif

#if !defined(MODULE_MIN_VER)
#define MODULE_MIN_VER 1
#endif

#if !defined(MODULE_REL_VER)
#define MODULE_REL_VER 2
#endif

#if !defined(SCM_VERSION)
#define SCM_VERSION "develop"
#endif

// Helper macros
#define _TOSTRING_(x) #x
#define _TOSTRING(x) _TOSTRING_(x)

// Stringified version
#define MODULE_VERSION \
     "" _TOSTRING(MODULE_MAJ_VER) \
    "." _TOSTRING(MODULE_MIN_VER) \
    "." _TOSTRING(MODULE_REL_VER) \
    "." SCM_VERSION

#define MODULE_NAME "iot-gateway-proxy"

#endif // _version_h_
