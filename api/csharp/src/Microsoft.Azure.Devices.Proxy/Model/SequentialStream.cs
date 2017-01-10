// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Model {
    using System;
    using System.Linq;
    using System.Collections.Generic;
    using System.Collections.Concurrent;
    using System.Threading;
    using System.Threading.Tasks;

    /// <summary>
    /// Endpoint to read arbitrary sized buffers from
    /// </summary>
    internal class SequentialStream : PacketStream {

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="socket"></param>
        internal SequentialStream(ProxySocket socket) : 
            base(socket) {
        }

        /// <summary>
        /// Buffered receive
        /// </summary>
        /// <param name="buffer"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async override Task<ProxyAsyncResult> ReceiveAsync(
            ArraySegment<byte> buffer, CancellationToken ct) {
            var result = new ProxyAsyncResult();
            while (true) {
                if (_lastData == null) {
                    _lastData = await ReceiveAsync(ct);
                    _offset = 0;

                    if (_lastData == null)  
                        break;
                }

                int toCopy = Math.Min(
                    buffer.Count - result.Count, _lastData.Payload.Length);
                Buffer.BlockCopy(
                    _lastData.Payload, _offset, buffer.Array, result.Count, toCopy);

                result.Count += toCopy;
                _offset += toCopy;

                if (_offset == _lastData.Payload.Length) {
                    _lastData = null;
                }
                if (result.Count > 0)
                    break;
            }
            return result;
        }

        private DataMessage _lastData;
        private int _offset;
    }
}