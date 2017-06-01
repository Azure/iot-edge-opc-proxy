// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Model {
    using System;
    using System.Threading;
    using System.Threading.Tasks;

    internal class UDPSocket : ProxySocket {
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="info"></param>
        /// <param name="provider"></param>
        internal UDPSocket(SocketInfo info, IProvider provider) :
            base(info, provider) {
            if (info.Type != SocketType.Dgram) {
                throw new ArgumentException("Udp only supports datagrams");
            }
        }

        /// <summary>
        /// Bind udp socket
        /// </summary>
        /// <param name="endpoint"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public override async Task BindAsync(SocketAddress endpoint, CancellationToken ct) {
            await base.BindAsync(endpoint, ct);
            bool connected = await LinkAllAsync(_bindList, endpoint, ct);
            if (!connected) {
                throw new SocketException(
                    "Could not link browse socket on proxy", SocketError.NoHost);
            }
        }
    }
}
