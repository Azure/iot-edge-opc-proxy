// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    /// <summary>
    /// Protocol type
    /// </summary>
    public enum ProtocolType {
        Unspecified = 0,
        Icmp = 1,
        Tcp = 6,
        Udp = 17,
        Icmpv6 = 58
    }
}