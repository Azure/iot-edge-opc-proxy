// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy.Model {
    using System;
    using System.Runtime.Serialization;

    internal class ProxyException : Exception {

        /// <summary>
        /// Returns error code
        /// </summary>
        public SocketError ErrorCode { get; private set; }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="error"></param>
        public ProxyException(SocketError error) : base (error.ToString()) {
            ErrorCode = error;
        }
    }
}