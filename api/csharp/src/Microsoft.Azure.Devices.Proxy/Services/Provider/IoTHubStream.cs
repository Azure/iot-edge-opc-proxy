// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Provider {
    using System;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Threading.Tasks.Dataflow;

    /// <summary>
    /// IoT Hub device method based message stream
    /// </summary>
    internal class IoTHubStream : IConnection, IMessageStream {

        /// <summary>
        /// Always polled
        /// </summary>
        public bool IsPolled {
            get => true;
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
        /// Constructor creating a method based polled stream.
        /// </summary>
        /// <param name="iothub"></param>
        /// <param name="streamId"></param>
        /// <param name="remoteId"></param>
        /// <param name="link"></param>
        /// <param name="connectionString"></param>
        public IoTHubStream(IoTHubService iothub, Reference streamId,
            Reference remoteId, INameRecord link, ConnectionString connectionString) {
            _iotHub = iothub;
            _streamId = streamId;
            _remoteId = remoteId;
            _link = link;

            ConnectionString = connectionString;

            ReceiveBlock = _receive = new BufferBlock<Message>(new DataflowBlockOptions {
                NameFormat = "Receive (in Stream) Id={1}",
                EnsureOrdered = true,
                BoundedCapacity = 1,
                MaxMessagesPerTask = DataflowBlockOptions.Unbounded,
                CancellationToken = _open.Token
            });

            SendBlock = new ActionBlock<Message>(async (message) => {
                if (message.TypeId != MessageContent.Data) {
                    // No sending of anything but data
                    return;
                }
                message.Source = _streamId;
                message.Target = _remoteId;
                try {
                    var d = message.Content as DataMessage;
                    if (d != null && d.Payload == null) {
                        throw new Exception();
                    }
                    var response = await _iotHub.TryInvokeDeviceMethodWithRetryAsync(
                        _link, message, TimeSpan.FromMilliseconds(_pollTimeout * 2),
                        _open.Token).ConfigureAwait(false);

                    if (!_open.IsCancellationRequested) {
                        if (response.Error != (int)SocketError.Success) {
                            throw new SocketException("Failure during send.", 
                                (SocketError)response.Error);
                        }
                        else if (response.TypeId != MessageContent.Poll) {
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
            }, new ExecutionDataflowBlockOptions {
                NameFormat = "Send (in Stream) Id={1}",
                CancellationToken = _open.Token,
                MaxDegreeOfParallelism = 1,
                MaxMessagesPerTask = DataflowBlockOptions.Unbounded,
                SingleProducerConstrained = true,
                EnsureOrdered = true
            });
        }

        /// <summary>
        /// Open stream
        /// </summary>
        /// <param name="ct"></param>
        /// <returns></returns>
        public Task<IMessageStream> OpenAsync(CancellationToken ct) {

            // Link to action block sending in order
            var sendTimeout = TimeSpan.FromMilliseconds(_pollTimeout * 2);

            // Start producer receiving from poll
            _producerTask = Task.Factory.StartNew(async () => {
                try {
                    ulong nextSequenceNumber = 0;
                    while(true) {
                        using (var request = Message.Create(_streamId, _remoteId,
                            PollRequest.Create(_pollTimeout, nextSequenceNumber))) {

                            var response = await _iotHub.TryInvokeDeviceMethodAsync(
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
                catch(OperationCanceledException) {
                }
                catch (Exception e) {
                    ProxyEventSource.Log.HandledExceptionAsError(this, e);
                    throw;
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
            return TaskEx.Completed;
        }

        private Task _producerTask;
        private readonly CancellationTokenSource _open = new CancellationTokenSource();
        private readonly BufferBlock<Message> _receive;
        private readonly IoTHubService _iotHub;
        private readonly Reference _streamId;
        private readonly Reference _remoteId;
        private readonly INameRecord _link;
        private static readonly ulong _pollTimeout = 120000; // 120 seconds default poll timeout
    }
}