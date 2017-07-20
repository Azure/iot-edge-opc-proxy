// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {

    /// <summary>
    /// Operations allowed on the name service.
    /// </summary>
    public enum NameServiceOperation {
        Remove,
        Add,
        Update,
        Disconnected,
        Connected
    }
}
