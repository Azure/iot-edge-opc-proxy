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

    /// <summary>
    /// Specialized implementation of relay based message stream
    /// </summary>
    public abstract class ConnectionBase<S> : IConnection, IMessageStream 
        where S : Stream {

        /// <summary>
        /// Whether the stream is polled or not.  This base class is not.
        /// </summary>
        public bool IsPolled {
            get => false;
        }

        /// <summary>
        /// Connection string for connection
        /// </summary>
        public ConnectionString ConnectionString {
            get; private set;
        }

        /// <summary>
        /// Block to send to
        /// </summary>
        public ITargetBlock<Message> SendBlock {
            get; private set;
        }

        /// <summary>
        /// Block to receive from
        /// </summary>
        public ISourceBlock<Message> ReceiveBlock {
            get; private set;
        }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="relay"></param>
        /// <param name="streamId"></param>
        /// <param name="connectionString"></param>
        protected ConnectionBase(Reference streamId, Reference remoteId, CodecId encoding, 
            ConnectionString connectionString) {

            _remoteId = remoteId;
            _encoding = encoding;

            StreamId = streamId;
            ConnectionString = connectionString;

            ReceiveBlock = _receive = new BufferBlock<Message>(new DataflowBlockOptions {
                NameFormat = "Receive (in Stream) Id={1}",
                BoundedCapacity = 1,
                EnsureOrdered = true,
                MaxMessagesPerTask = DataflowBlockOptions.Unbounded,
                CancellationToken = _open.Token
            });

            SendBlock = _send = new BufferBlock<Message>(new DataflowBlockOptions {
                NameFormat = "Send (in Stream) Id={1}",
                BoundedCapacity = 1, 
                EnsureOrdered = true,
                MaxMessagesPerTask = DataflowBlockOptions.Unbounded,
                CancellationToken = _open.Token
            });
        }

        /// <summary>
        /// Accept this stream
        /// </summary>
        /// <param name="ct"></param>
        /// <returns></returns>
        public Task<IMessageStream> OpenAsync(CancellationToken ct) {
            ct.Register(() => {
                Tcs.TrySetCanceled();
            });
            return Tcs.Task;
        }

        /// <summary>
        /// Close stream
        /// </summary>
        /// <returns></returns>
        public Task CloseAsync() {
            Task pumps;
            lock (this) {
                // Set close state
                _open.Cancel();

                _receive.Complete();
                _send.Complete();

                // Remove ourselves from the listener...
                Close();

                // Fail any in progress open 
                if (!Tcs.Task.IsCompleted) {
                    Tcs.TrySetException(new SocketException(SocketError.Closed));
                }

                if (_pumps == null) {
                    return TaskEx.Completed;
                }

                pumps = _pumps;
                _pumps = null;
                return pumps;
            }
        }

        public abstract void Close();

        /// <summary>
        /// Attach the stream to a accepted stream instance and start to produce/consume
        /// </summary>
        /// <param name="stream"></param>
        public virtual Task OpenAsync(S stream) {
            if (_open.IsCancellationRequested) {
                // Stream closed, but proxy tries to connect, reject
                return null;
            }
            lock (this) {

                // Start pumping...
                var codecStream = stream.AsCodecStream(_encoding);
                var pumps = Task.WhenAll(new Task[] {
                    Task.Factory.StartNew(async () =>
                        await SendConsumerAsync(codecStream), _open.Token).Unwrap(),
                    Task.Factory.StartNew(async () =>
                        await ReceiveProducerAsync(codecStream), _open.Token).Unwrap()
                }).ContinueWith(t => CloseStreamAsync(codecStream, t.IsFaulted)).Unwrap();

                if (_pumps != null) {
                    // Reconnect
                    _pumps = _pumps.ContinueWith(async (t) => {
                        await pumps.ConfigureAwait(false);
                    });
                }
                else {
                    // First connect
                    _pumps = pumps;
                    Tcs.TrySetResult(this);
                }
                ProxyEventSource.Log.StreamOpened(this, stream);

                return pumps;
            }
        }

        /// <summary>
        /// Stream open completion source
        /// </summary>
        protected TaskCompletionSource<IMessageStream> Tcs {
            get; private set;
        } = new TaskCompletionSource<IMessageStream>();

        /// <summary>
        /// Id of stream
        /// </summary>
        protected Reference StreamId {
            get; private set;
        }

        /// <summary>
        /// Send consumer, reading messages one by one from stream and writes
        /// to websocket stream.
        /// </summary>
        /// <returns></returns>
        protected virtual async Task SendConsumerAsync(ICodecStream<S> codec) {
            if (_lastMessage == null) {
                _lastMessage = Message.Create(StreamId, _remoteId,
                    PollRequest.Create((ulong)_timeout.TotalMilliseconds));
            }
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
                    await codec.WriteAsync(_lastMessage, _open.Token).ConfigureAwait(false);
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

        /// <summary>
        /// Receive producer, reading messages one by one from relay and 
        /// writing to receive block.  Manages stream as well.
        /// </summary>
        /// <returns></returns>
        protected virtual async Task ReceiveProducerAsync(ICodecStream<S> codec) {
            while (!_open.IsCancellationRequested) {
                try {
                    // Read message and send to source block
                    var message = await codec.ReadAsync<Message>(_open.Token).ConfigureAwait(false);
                    if (message != null) {
                        var data = message.Content as DataMessage;
                        if (data != null && data.SequenceNumber != _nextReceiveSequenceNumber++) {
                            // TODO: Implement poll for previous message
                            System.Diagnostics.Trace.TraceError(
                                $"{data.SequenceNumber} received, {_nextReceiveSequenceNumber - 1} expected.");
                            message.Error = (int)SocketError.Comm;
                        }
                        if (!await _receive.SendAsync(message, _open.Token).ConfigureAwait(false) ||
                                message.TypeId == MessageContent.Close) {
                            // Pipeline closed, close the connection
                            _send.Complete();
                            _open.Cancel();
                            break;
                        }
                    }
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

        /// <summary>
        /// Close stream orderly when stream is done. Not called when parent faulted.
        /// </summary>
        /// <param name="codec"></param>
        /// <returns></returns>
        protected abstract Task CloseStreamAsync(ICodecStream<S> codec);


        protected Message _lastMessage;
        protected TimeSpan _timeout = TimeSpan.FromMinutes(1);
        protected Task _pumps;
        protected readonly CancellationTokenSource _open = new CancellationTokenSource();
        protected readonly BufferBlock<Message> _receive;
        protected readonly BufferBlock<Message> _send;
        protected readonly CodecId _encoding;
        protected readonly Reference _remoteId;
        protected ulong _nextSendSequenceNumber = 0;
        protected ulong _nextReceiveSequenceNumber = 0;

        /// <summary>
        /// Wrapper that calls dispose on the stream - at a minimum...
        /// </summary>
        /// <param name="codec"></param>
        /// <param name="parentHasFaulted"></param>
        /// <returns></returns>
        private async Task CloseStreamAsync(ICodecStream<S> codec, 
            bool parentHasFaulted) {
            ProxyEventSource.Log.StreamClosing(this, codec.Stream);
            if (!parentHasFaulted) {
                await CloseStreamAsync(codec).ConfigureAwait(false);
            }
            try {
                codec.Stream.Dispose();
            }
            catch (Exception e) {
                ProxyEventSource.Log.HandledExceptionAsInformation(codec.Stream, e);
            }
            ProxyEventSource.Log.StreamClosed(this, codec.Stream);
        }
    }
}
