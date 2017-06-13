// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    /// <summary>
    /// Type of socket
    /// </summary>
    public enum SocketType {
        Stream = 1,
        Dgram = 2,
        Raw = 3,
        RDM = 4,
        SeqPacket = 5
    }
}