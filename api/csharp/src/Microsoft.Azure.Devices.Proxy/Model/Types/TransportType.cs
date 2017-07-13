// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!
namespace Microsoft.Azure.Devices.Proxy {
    /// <summary>
    /// Transport type
    /// </summary>
    public enum TransportType {
        Ws = 0,
        Mqtt,
        Amqp,
        Unknown = -1
    }
}