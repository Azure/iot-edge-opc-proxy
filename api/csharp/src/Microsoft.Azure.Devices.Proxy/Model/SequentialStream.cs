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
        public override Task<ProxyAsyncResult> ReceiveAsync(
            ArraySegment<byte> buffer, CancellationToken ct) {

            if (buffer.Count == 0) {
                return Task.FromResult(new ProxyAsyncResult());
            }

            if (_lastData != null) {
                int copied = CopyBuffer(ref buffer);
                if (copied > 0) {
                    if (_lastRead == null || _lastRead.Result.Count != copied) {
                        var result = new ProxyAsyncResult();
                        result.Count = copied;
                        _lastRead = Task.FromResult(result);
                    }
                    return _lastRead;
                }
            }
            _lastRead = ReceiveInternalAsync(buffer, ct);
            return _lastRead;
        }

        /// <summary>
        /// Receive using async state machine
        /// </summary>
        /// <param name="buffer"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async Task<ProxyAsyncResult> ReceiveInternalAsync(
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
#if PERF
                    _transferred += _lastData.Payload.Length;
                    Console.CursorLeft = 0; Console.CursorTop = 0;
                    Console.WriteLine(
                        $"{ _transferred / _transferredw.ElapsedMilliseconds} kB/sec");
#endif
                }
                result.Count = CopyBuffer(ref buffer);
                if (result.Count > 0)
                    break;
            }
            return result;
        }

        /// <summary>
        /// Copies from the last buffer
        /// </summary>
        /// <param name="buffer"></param>
        /// <returns></returns>
        private int CopyBuffer(ref ArraySegment<Byte> buffer) {
            // How much to copy from the last data buffer.
            int toCopy = Math.Min(buffer.Count, _lastData.Payload.Length - _offset);
            Buffer.BlockCopy(_lastData.Payload, _offset,
                buffer.Array, buffer.Offset, toCopy);
            _offset += toCopy;

            if (_offset >= _lastData.Payload.Length) {
                // Last data exhausted, release
                _lastData = null;
                _offset = 0;
            }
            return toCopy;
        }

        private Task<ProxyAsyncResult> _lastRead;
        private DataMessage _lastData;
        private int _offset;
#if PERF
        private long _transferred;
        private Stopwatch _transferredw = Stopwatch.StartNew();
#endif
    }
}