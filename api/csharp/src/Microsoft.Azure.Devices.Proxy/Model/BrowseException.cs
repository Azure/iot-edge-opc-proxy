// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Model {
    using System;

    public class BrowseException : ProxyException {

        /// <summary>
        /// Constructor taking a socket error.
        /// </summary>
        /// <param name="error"></param>
        public BrowseException(SocketError error) :
            base(error) {
        }

        /// <summary>
        /// Constructor taking a string
        /// </summary>
        /// <param name="message"></param>
        public BrowseException(string message) :
            base(message) {
        }

        /// <summary>
        /// Constructor taking an exception
        /// </summary>
        /// <param name="message"></param>
        public BrowseException(Exception inner) :
            this("An exception occurred browsing.", inner) {
        }

        /// <summary>
        /// Constructor taking an exception
        /// </summary>
        /// <param name="message"></param>
        public BrowseException(string message, Exception inner) :
            base(message, inner) {
        }
    }
}