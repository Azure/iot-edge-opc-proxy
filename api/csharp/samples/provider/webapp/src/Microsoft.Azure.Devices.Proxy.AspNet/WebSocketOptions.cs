// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Provider {
    using System;

    public class WebSocketOptions {

        /// <summary>
        /// Iot Hub owner connection string
        /// </summary>
        public string IoTHubOwnerConnectionString { get; set; }

        /// <summary>
        /// Public endpoint that clients should connect up to.
        /// </summary>
        public Uri PublicEndpoint { get; set; }
    }
}
