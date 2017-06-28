// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;

    public class ProxyNotFoundException : ProxyException {

        public ProxyNotFoundException(string message) : 
            base(message, null, SocketError.NoHost) {
        }

        public ProxyNotFoundException(Exception innerException) :
            base("Proxy not found", innerException, SocketError.NoHost) {
        }

        public ProxyNotFoundException(AggregateException innerException) :
            base("Proxy not found", innerException, SocketError.NoHost) {
        }
    }

}