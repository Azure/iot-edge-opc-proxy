// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Provider {
    using System;
    using System.IO;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Threading.Tasks.Dataflow;
    using Relay;

    /// <summary>
    /// Specialized implementation of relay based message stream
    /// </summary>
    internal class ServiceBusRelayConnection : ConnectionBase<HybridConnectionStream> {

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="relay"></param>
        /// <param name="streamId"></param>
        /// <param name="encoding"></param>
        /// <param name="connectionString"></param>
        public ServiceBusRelayConnection(ServiceBusRelay relay, Reference streamId,
            Reference remoteId, CodecId encoding, ConnectionString connectionString) : 
            base(streamId, remoteId, encoding, connectionString) { 
            _relay = relay;
        }

        /// <summary>
        /// Close stream
        /// </summary>
        /// <returns></returns>
        public override void Close() {
            // Remove ourselves from the listener...
            _relay._connectionMap.TryRemove(StreamId, out ServiceBusRelayConnection stream);
        }

        /// <summary>
        /// Close stream when producers finish
        /// </summary>
        /// <param name="codec"></param>
        /// <returns></returns>
        protected override async Task CloseStreamAsync(ICodecStream<HybridConnectionStream> codec) {
            try {
                if (_open.IsCancellationRequested) {
                    // User is asking for a graceful close, shutdown properly
                    await codec.Stream.ShutdownAsync(new CancellationTokenSource(
                        ServiceBusRelay._closeTimeout).Token).ConfigureAwait(false);
                }
            }
            catch { }
            try {
                // ... then gracefully close
                await codec.Stream.CloseAsync(new CancellationTokenSource(
                    ServiceBusRelay._closeTimeout).Token).ConfigureAwait(false);
            }
            catch (OperationCanceledException) {
            }
            catch (Exception e) {
                ProxyEventSource.Log.StreamException(this, codec.Stream, e);
            }
        }

#if !STREAM_FRAGMENT_BUG_FIXED
        protected override async Task SendConsumerAsync(
            ICodecStream<HybridConnectionStream> codec) {
            while (!_open.IsCancellationRequested) {
                try {
                    if (_lastMessage == null) {
                        if (_send.Completion.IsCompleted) {
                            // Pipeline closed, close the connection
                            _receive.Complete();
                            _open.Cancel();
                            break;
                        }
                        try {
                            _lastMessage = await _send.ReceiveAsync(_timeout,
                                _open.Token).ConfigureAwait(false);
                            var data = _lastMessage.Content as DataMessage;
                            if (data != null) {
                                data.SequenceNumber = _nextSendSequenceNumber++;
                            }
                        }
                        catch (TimeoutException) {
                            _lastMessage = Message.Create(StreamId, _remoteId,
                                PollRequest.Create((ulong)_timeout.TotalMilliseconds));
                        }
                        catch (OperationCanceledException) when (!_open.IsCancellationRequested) {
                            _lastMessage = Message.Create(StreamId, _remoteId,
                                PollRequest.Create((ulong)_timeout.TotalMilliseconds));
                        }
                    }
                    //
                    // Every write on the hybrid connection stream right now results in a binary 
                    // message not an individual fragment.
                    // when using the codec directly on the stream then the first write confuses the 
                    // proxy decoder, which assumes a websocket message contains an entire message.
                    // Override here to buffer the entire message in a memory stream before writing.
                    //
                    using (var mem = new MemoryStream()) {
                        _lastMessage.Encode(mem, CodecId.Mpack);
                        var buffered = mem.ToArray();
                        await codec.Stream.WriteAsync(
                            buffered, 0, buffered.Length, _open.Token).ConfigureAwait(false);
                    }
                    await codec.Stream.FlushAsync(_open.Token).ConfigureAwait(false);
                    _lastMessage.Dispose();
                    _lastMessage = null;
                }
                catch (OperationCanceledException) {
                    break;
                }
                catch (Exception e) {
                    ProxyEventSource.Log.StreamException(this, codec.Stream, e);
                    break;
                }
            }
        }
#endif

        private readonly ServiceBusRelay _relay;
    }
}
