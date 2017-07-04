// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;

    public class ProxyTimeout : TimeoutException {

        public ProxyTimeout() : 
            base("Proxy operation timed out") {
        }

        public ProxyTimeout(string message) :
            base(message) {
        }

        public ProxyTimeout(string message, Exception innerException) :
            base(message, innerException) {
        }

        public ProxyTimeout(string message, AggregateException innerException) :
            base(message, innerException.Flatten()) {
        }
    }
}