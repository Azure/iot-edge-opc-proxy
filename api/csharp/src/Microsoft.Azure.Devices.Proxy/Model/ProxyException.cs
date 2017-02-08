// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy.Model {
    using System;

    internal class ProxyException : SocketException {

        /// <summary>
        /// Constructor taking a socket error.
        /// </summary>
        /// <param name="error"></param>
        public ProxyException(SocketError error) : base (error) {
        }

        /// <summary>
        /// Constructor taking a string
        /// </summary>
        /// <param name="message"></param>
        public ProxyException(string message) : base(message) {
        }

        /// <summary>
        /// Constructor taking an exception
        /// </summary>
        /// <param name="message"></param>
        public ProxyException(Exception inner) : base(inner) {
        }
    }
}