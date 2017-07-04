// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    /// <summary>
    /// Shutdown value for Shutdown option
    /// </summary>
    public enum SocketShutdown {
        Read = 0,
        Write = 1,
        Both = 2
    }
}