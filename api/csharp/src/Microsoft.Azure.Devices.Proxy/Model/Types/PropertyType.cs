// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {

    internal enum PropertyType {
        FileInfo = 200,
        AddressInfo,
        InterfaceInfo,

        // ...
        __prx_property_type_max
    }
}