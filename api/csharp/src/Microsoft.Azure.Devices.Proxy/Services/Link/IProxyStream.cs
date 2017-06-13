// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Threading;
    using System.Threading.Tasks;

    /// <summary>
    /// Proxy stream providing streaming of raw buffers
    /// </summary>
    public interface IProxyStream {

        /// <summary>
        /// Send data (data)
        /// </summary>
        /// <param name="buffer"></param>
        /// <param name="sendTimeout"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        Task<int> SendAsync(ArraySegment<byte> buffer,
            SocketAddress endpoint, CancellationToken ct);

        /// <summary>
        /// Receive data (data)
        /// </summary>
        /// <param name="buffer"></param>
        /// <param name="receiveTimeout"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        Task<ProxyAsyncResult> ReceiveAsync(
            ArraySegment<byte> buffer, CancellationToken ct);
    }
}
