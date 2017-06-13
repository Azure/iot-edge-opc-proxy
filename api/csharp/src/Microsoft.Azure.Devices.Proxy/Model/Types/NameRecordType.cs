// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;

    [Flags]
    public enum NameRecordType {
        Hub = 0x0,
        Host = 0x1,
        Proxy = 0x2,
        Startup = 0x4,
        Link = 0x8,
        All = 0xf
    }
}
