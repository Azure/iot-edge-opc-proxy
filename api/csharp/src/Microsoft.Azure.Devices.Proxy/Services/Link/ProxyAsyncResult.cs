// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {

    /// <summary>
    /// Result for async socket provider operations
    /// </summary>
    public class ProxyAsyncResult {
        /// <summary>
        /// Address, e.g. as result from receive or accept
        /// </summary>
        public SocketAddress Address {
            get; set;
        }

        /// <summary>
        /// Count bytes
        /// </summary>
        public int Count {
            get; set;
        }

        /// <summary>
        /// Proxy instance, e.g. created as part of accept
        /// </summary>
        public IProxySocket Link {
            get; set;
        }
    }
}