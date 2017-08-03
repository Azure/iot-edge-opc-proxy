// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Provider {
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Threading.Tasks.Dataflow;

    /// <summary>
    /// IoT Hub device method based message stream
    /// </summary>
    internal class IoTHubStream : IConnection, IMessageStream {

        /// <summary>
        /// Returns the maximum size of a buffer that can be sent through this connection.
        /// Control buffer and payload buffer combined must respect this size restriction.
        /// Currently max buffer is whatever can be added as b64 encoded string to the
        /// payload entry in the json data message.  This is currently up to 8 kb.
        /// We assume that the framing portion for a data message is consistently 2kb.
        /// Assuming a 1/4 increase for base 64, this would mean something like 4-5k.
        /// </summary>
        public uint MaxBufferSize {
#if !SUPPORT_MSG_OVER_8K
            // TODO: Remove this when > 88 k messages are supported
            get => 5000;
#else
            get => 0;
#endif
        }

        /// <summary>
        /// Always polled
        /// </summary>
        public bool IsPolled {
            get => true;
        }

        /// <summary>
        /// Connection string for connection - for proxy to use
        /// </summary>
        public ConnectionString ConnectionString {
            get;
        }

        /// <summary>
        /// Block to send to
        /// </summary>
        public ITargetBlock<Message> SendBlock {
            get => _send;
        }

        /// <summary>
        /// Block to receive from
        /// </summary>
        public ISourceBlock<Message> ReceiveBlock {
            get => _receive;
        }

        /// <summary>
        /// Constructor creating a method based polled stream.
        /// </summary>
        /// <param name="streamId"></param>
        /// <param name="remoteId"></param>
        /// <param name="link"></param>
        /// <param name="connectionString"></param>
        public IoTHubStream(ConnectionString hubConnectionString, Reference streamId,
            Reference remoteId, INameRecord link, ConnectionString connectionString) {
            _hubConnectionString = hubConnectionString ??
                throw new ArgumentNullException(nameof(hubConnectionString));
            _streamId = streamId;
            _remoteId = remoteId;
            _link = link;
            ConnectionString = connectionString;
        }

        /// <summary>
        /// Open stream
        /// </summary>
        /// <param name="ct"></param>
        /// <returns></returns>
        public Task<IMessageStream> OpenAsync(CancellationToken ct) {

            CreateReceiveBlock();
            CreateSendBlock();

            // Start producer receiving from poll
            _receiveTask = Task.Factory.StartNew(async () => {
                var sendTimeout = TimeSpan.FromMilliseconds(_pollTimeout * 2);
                var receiver = new IoTHubInvoker(_hubConnectionString);
                try {
                    ulong nextSequenceNumber = 0;
                    while (true) {
                        using (var request = Message.Create(_streamId, _remoteId,
                            PollRequest.Create(_pollTimeout, nextSequenceNumber))) {

                            var response = await receiver.TryCallAsync(
                                _link, request, sendTimeout, _open.Token).ConfigureAwait(false);
                            //
                            // Poll receives back a timeout error in case no data was available
                            // within the requested timeout. This is decoupled from the consumers
                            // that time out on their cancellation tokens.
                            //
                            if (response != null && response.Error != (int)SocketError.Timeout) {
                                if (!await _receive.SendAsync(response).ConfigureAwait(false)) {
                                    break;
                                }
                                nextSequenceNumber++;
                            }
                        }
                        // Continue polling until closed in which case we complete receive
                        _open.Token.ThrowIfCancellationRequested();
                    }
                }
                catch (OperationCanceledException) {
                }
                catch (Exception e) {
                    ProxyEventSource.Log.HandledExceptionAsError(this, e);
                    throw;
                }
                finally {
                    receiver.Dispose();
                }
            }, _open.Token, TaskCreationOptions.LongRunning, TaskScheduler.Default).Unwrap();

            return Task.FromResult((IMessageStream)this);
        }

        /// <summary>
        /// Close stream
        /// </summary>
        /// <returns></returns>
        public Task CloseAsync() {
            _open.Cancel();
            _sender.Dispose();
            return TaskEx.Completed;
        }

        /// <summary>
        /// Create receive block - re-orders messages if out of order up to the size
        /// of the receive window.
        /// </summary>
        private void CreateReceiveBlock() {
            _receiveWindow = new SortedDictionary<ulong, Message>();
            _receive = new TransformManyBlock<Message, Message>(response => {
                var data = response.Content as DataMessage;
                if (data != null) {
                    if (data.SequenceNumber != _nextReceiveSequenceNumber) {
                        //
                        // Message received with sequence number is not what we expected.
                        // This can happen since responses come in from 2 different "threads".
                        //
                        if (_receiveWindow.ContainsKey(data.SequenceNumber)) {
                            // Real duplicate - discard.
                            ProxyEventSource.Log.DuplicateData(this, data, _nextReceiveSequenceNumber);
                        }
                        else if (_receiveWindow.Count > _receiveWindowMax) {
                            // Too many missing messages, clear window...
                            ProxyEventSource.Log.MissingData(this, data, _nextReceiveSequenceNumber);
                            response.Error = (int)SocketError.Missing;
                            foreach (var r in _receiveWindow.Values) {
                                r.Dispose();
                            }
                            _receiveWindow.Clear();
                        }
                        else {
                            // Add message to sorted set and wait for sequence
                            // number to catch up.
                            ProxyEventSource.Log.MissingData(this, data, _nextReceiveSequenceNumber);
                            _receiveWindow.Add(data.SequenceNumber, response);
                        }
                        return Enumerable.Empty<Message>();
                    }

                    // we received message with sequence number we expected.
                    _nextReceiveSequenceNumber++;

                    if (_receiveWindow.Count > 0) {
                        var result = new List<Message>();
                        result.Add(response);

                        // Add messages with sequence number beyond ours to returned list...
                        foreach (var r in _receiveWindow.Values) {
                            var nextData = (DataMessage)r.Content;
                            if (nextData.SequenceNumber != _nextReceiveSequenceNumber) {
                                break;
                            }
                            _nextReceiveSequenceNumber++;
                            result.Add(r);
                        }
                        foreach (var item in result) {
                            _receiveWindow.Remove(((DataMessage)item.Content).SequenceNumber);
                        }
                        return result;
                    }
                }
                return response.AsEnumerable();
            },
            new ExecutionDataflowBlockOptions {
                NameFormat = "Receive (in Stream) Id={1}",
                EnsureOrdered = true,
                BoundedCapacity = 1,
                MaxDegreeOfParallelism = 1,
                MaxMessagesPerTask = DataflowBlockOptions.Unbounded,
                CancellationToken = _open.Token,
                SingleProducerConstrained = false
            });
        }

        /// <summary>
        /// Create send action block.
        /// </summary>
        private void CreateSendBlock() {
            _sender = new IoTHubInvoker(_hubConnectionString);
            _send = new ActionBlock<Message>(async (message) => {
                if (message.TypeId != MessageContent.Data) {
                    // No sending of anything but data
                    return;
                }
                message.Source = _streamId;
                message.Target = _remoteId;
                try {
                    ((DataMessage)message.Content).SequenceNumber = _nextSendSequenceNumber++;

                    var response = await _sender.TryCallWithRetryAsync(
                        _link, message, TimeSpan.FromMilliseconds(_pollTimeout * 2),
                        int.MaxValue, _open.Token).ConfigureAwait(false);

                    if (!_open.IsCancellationRequested) {
                        if (response.Error != (int)SocketError.Success) {
                            throw new SocketException("Failure during send.",
                                (SocketError)response.Error);
                        }
                        if (response.TypeId != MessageContent.Poll) {
                            await _receive.SendAsync(response).ConfigureAwait(false);
                        }
                    }
                }
                catch (OperationCanceledException) {
                }
                catch (Exception e) {
                    ProxyEventSource.Log.HandledExceptionAsError(this, e);
                    throw;
                }
                finally {
                    message.Dispose();
                }
            },
            new ExecutionDataflowBlockOptions {
                NameFormat = "Send (in Stream) Id={1}",
                EnsureOrdered = true,
                BoundedCapacity = 1,
                MaxDegreeOfParallelism = 1,
                MaxMessagesPerTask = DataflowBlockOptions.Unbounded,
                CancellationToken = _open.Token,
                SingleProducerConstrained = true
            });
        }

        private Task _receiveTask;
        private IPropagatorBlock<Message, Message> _receive;
        private ulong _nextReceiveSequenceNumber;
        private SortedDictionary<ulong, Message> _receiveWindow;

        private IoTHubInvoker _sender;
        private ITargetBlock<Message> _send;
        private ulong _nextSendSequenceNumber;

        private ConnectionString _hubConnectionString;
        private readonly Reference _streamId;
        private readonly Reference _remoteId;
        private readonly INameRecord _link;

        private readonly CancellationTokenSource _open = new CancellationTokenSource();

        private const int _receiveWindowMax = 5; // Max 5 messages to reorder...
        private const ulong _pollTimeout = 120000; // 120 seconds default poll timeout
    }
}
