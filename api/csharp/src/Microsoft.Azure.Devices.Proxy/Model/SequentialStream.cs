// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Model {
    using System;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Diagnostics;

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

#if PERF
        private long _transferred;
        private Stopwatch _transferredw = Stopwatch.StartNew();
#endif

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
                    // Get new data
                    _lastData = await ReceiveAsync(ct).ConfigureAwait(false);
                    _offset = 0;

                    // Disconnect, break
                    if (_lastData == null)
                        break;

                    // Break on 0 sized packets
                    if (_lastData.Payload.Length == 0) {
                        _lastData = null;
                        break;
                    }
                }

                // How much to copy from the last data buffer.
                int toCopy = Math.Min(buffer.Count - result.Count,
                    _lastData.Payload.Length - _offset);

                Buffer.BlockCopy(_lastData.Payload, _offset,
                    buffer.Array, buffer.Offset + result.Count, toCopy);

                result.Count += toCopy;
                _offset += toCopy;

                if (_offset >= _lastData.Payload.Length) {
                    // Last data exhausted, release
                    _lastData = null;
                    _offset = 0;
                }
                if (result.Count > 0)
                    break;
            }
#if PERF
            _transferred += result.Count;
            Console.CursorLeft = 0; Console.CursorTop = 0;
            Console.WriteLine($"{ _transferred / _transferredw.ElapsedMilliseconds} kB/sec");
#endif
            return result;
        }

        private DataMessage _lastData;
        private int _offset;
    }
}