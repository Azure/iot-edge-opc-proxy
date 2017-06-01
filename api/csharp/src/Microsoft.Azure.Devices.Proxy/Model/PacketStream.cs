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
    internal class PacketStream : ProxyStream {

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="socket"></param>
        internal PacketStream(ProxySocket socket) :
            base(socket) {
        }

        /// <summary>
        /// Receive a data packet
        /// </summary>
        /// <param name="buffer"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async override Task<ProxyAsyncResult> ReceiveAsync(
            ArraySegment<byte> buffer, CancellationToken ct) {
            var data = await ReceiveAsync(ct).ConfigureAwait(false);
            int copy = Math.Min(data.Payload.Length, buffer.Count);
            Buffer.BlockCopy(data.Payload, 0, buffer.Array, buffer.Offset, copy);
            return new ProxyAsyncResult {
                Address = data.Source,
                Count = copy
            };
        }

        /// <summary>
        /// Send a data packet
        /// </summary>
        /// <param name="buffer"></param>
        /// <param name="endpoint"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async override Task<int> SendAsync(ArraySegment<byte> buffer, 
            SocketAddress endpoint, CancellationToken ct) {
            await Socket.SendAsync(new Message(null, null, null, 
                new DataMessage(buffer, endpoint)), ct).ConfigureAwait(false);
            return buffer.Count;
        }
    }
}