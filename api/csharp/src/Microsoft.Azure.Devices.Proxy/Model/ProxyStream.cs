// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Model {
    using System;
    using System.Threading;
    using System.Threading.Tasks;

    /// <summary>
    /// Stream to read on packet/datagram boundary from message stream
    /// </summary>
    internal abstract class ProxyStream : IProxyStream {

        protected ProxySocket Socket { get; private set; }

        internal ProxyStream(ProxySocket socket) {
            Socket = socket;
        }

        public abstract Task<ProxyAsyncResult> ReceiveAsync(
            ArraySegment<byte> buffer, CancellationToken ct);

        public abstract Task<int> SendAsync(ArraySegment<byte> buffer, 
            SocketAddress endpoint, CancellationToken ct);


        /// <summary>
        /// Receive a message
        /// </summary>
        /// <param name="buffer"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        protected async Task<DataMessage> ReceiveAsync(
            CancellationToken ct) {
            Message message;
            while (!Socket.ReceiveQueue.TryTake(out message)) {
                await Socket.ReceiveAsync(ct);
            }
            ProxySocket.ThrowIfFailed(message);
            if (message.TypeId != MessageContent.Data)
                throw new SocketException("No data message");
            return message.Content as DataMessage;
        }
    }
}