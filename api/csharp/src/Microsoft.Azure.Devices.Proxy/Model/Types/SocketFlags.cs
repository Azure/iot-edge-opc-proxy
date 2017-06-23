// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System;

    /// <summary>
    /// Flags for socket properties, determine how socket is opened
    /// </summary>
    [Flags]
    public enum SocketFlags {
        Passive = 0x1,
        Internal = 0x2
    }
}