// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Linq;
    using System.Collections.Generic;
    using System.Collections.Concurrent;
    using System.Threading;
    using System.Threading.Tasks;

    /// <summary>
    /// Stream to read socket accepts from
    /// </summary>
    internal class ListenStream : ProxyStream {

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="socket"></param>
        internal ListenStream(ProxySocket socket) : 
            base(socket) {
        }

        public override Task<ProxyAsyncResult> ReceiveAsync(
            ArraySegment<byte> buffer, CancellationToken ct) {
            throw new NotImplementedException();
        }

        public override Task<int> SendAsync(ArraySegment<byte> buffer,
            SocketAddress endpoint, CancellationToken ct) {
            throw new NotImplementedException();
        }
    }
}