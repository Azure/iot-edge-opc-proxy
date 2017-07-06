// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;

    public class ProxyNotFound : ProxyException {

        public ProxyNotFound(string message) : 
            base(message, null, SocketError.NoHost) {
        }

        public ProxyNotFound(Exception innerException) :
            base("Proxy not found", innerException, SocketError.NoHost) {
        }

        public ProxyNotFound(AggregateException innerException) :
            base("Proxy not found", innerException, SocketError.NoHost) {
        }
    }

}