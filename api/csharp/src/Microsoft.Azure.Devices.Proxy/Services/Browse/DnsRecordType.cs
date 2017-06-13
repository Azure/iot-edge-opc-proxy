// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    //
    // Dns Record types - only txt is used returned as property right now.
    //
    public enum DnsRecordType {
        Simple = 100,
        Txt,
        Ptr,

        // ...
        __prx_record_max
    };
} 