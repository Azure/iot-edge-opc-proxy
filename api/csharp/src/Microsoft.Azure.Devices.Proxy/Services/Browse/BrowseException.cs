// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;

    public class BrowseException : ProxyException {

        /// <summary>
        /// Constructor taking a socket error.
        /// </summary>
        /// <param name="error"></param>
        internal BrowseException(SocketError error) :
            base(error) {
        }
    }
}