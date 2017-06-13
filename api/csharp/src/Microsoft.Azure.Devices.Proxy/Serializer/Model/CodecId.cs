// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    /// <summary>
    /// Known codecs
    /// </summary>
    public enum CodecId {
        Mpack = 1,
        Json,
        //...
        Unknown = 0
    }
}
