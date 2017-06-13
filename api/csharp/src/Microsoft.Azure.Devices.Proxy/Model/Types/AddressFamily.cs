// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!
namespace Microsoft.Azure.Devices.Proxy {
    /// <summary>
    /// Address family 
    /// </summary>
    public enum AddressFamily {
        // Standard http://www.iana.org assigned numbers
        Unspecified = 0,
        Unix = 1,
        InterNetwork = 2,
        InterNetworkV6 = 23,

        // Non standard first come first serve af for proxy
        Proxy = 28165,

        // .net internal family identifiers, not known to proxy
        Collection,
        Bound
    }
}