// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System;

    public class ProxyException : SocketException {
        /// <summary>
        /// Constructor taking a socket error.
        /// </summary>
        /// <param name="error"></param>
        public ProxyException(SocketError error) :
            base(error) {
        }

        /// <summary>
        /// Constructor taking a string
        /// </summary>
        /// <param name="message"></param>
        public ProxyException(string message) :
            base(message) {
        }

        /// <summary>
        /// Constructor taking an exception
        /// </summary>
        /// <param name="message"></param>
        public ProxyException(string message, Exception inner) :
            base(message, inner) {
        }

        /// <summary>
        /// Constructor taking an aggregate
        /// </summary>
        /// <param name="message"></param>
        public ProxyException(string message, AggregateException inner) :
            base(message, inner) {
        }

        /// <summary>
        /// Create proxy exception
        /// </summary>
        /// <param name="message"></param>
        /// <param name="inner"></param>
        /// <param name="errorcode"></param>
        public ProxyException(string message, Exception inner, SocketError errorcode) :
            base(message, inner, errorcode) {
        }

        /// <summary>
        /// Create proxy exception with aggregate
        /// </summary>
        /// <param name="message"></param>
        /// <param name="inner"></param>
        /// <param name="errorcode"></param>
        public ProxyException(string message, AggregateException inner, SocketError errorcode) :
            base(message, inner, errorcode) {
        }
    }
}