// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Provider {
    using System;
    using System.Threading.Tasks;
    using System.Collections.Concurrent;
    using System.Threading;
    using Model;

    /// <summary>
    /// IoT Hub device method based message stream
    /// </summary>
    internal class IoTHubStream : IConnection, IMessageStream {

        /// <summary>
        /// Connection string for connection
        /// </summary>
        public ConnectionString ConnectionString { get; private set; }

        /// <summary>
        /// Always polled
        /// </summary>
        public bool IsPolled { get; } = true;

        /// <summary>
        /// Queue to read from 
        /// </summary>
        public ConcurrentQueue<Message> ReceiveQueue { get; } =
            new ConcurrentQueue<Message>();

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
        }

        /// <summary>
        /// Open stream
        /// </summary>
        /// <param name="ct"></param>
        /// <returns></returns>
        public Task<IMessageStream> OpenAsync(CancellationToken ct) {
            _open = new CancellationTokenSource();
            return Task.FromResult((IMessageStream)this);
        }

        /// <summary>
        /// Close stream
        /// </summary>
        /// <returns></returns>
        public Task CloseAsync() {
            _open.Cancel();
            return Task.FromResult(true);
        }

        /// <summary>
        /// Sends a poll request and enqueues result to receive queue.
        /// </summary>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async Task ReceiveAsync(CancellationToken ct) {
            Message response = await _iotHub.TryInvokeDeviceMethodAsync(_link,
                new Message(_streamId, _remoteId, new PollRequest(120000)),
                    TimeSpan.FromMinutes(3), _open.Token).ConfigureAwait(false);
            if (response != null) {
                ReceiveQueue.Enqueue(response);
            }
        }

        /// <summary>
        /// Send data message
        /// </summary>
        /// <param name="message"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async Task SendAsync(Message message, CancellationToken ct) {
            message.Source = _streamId;
            message.Target = _remoteId;
            Message response = await _iotHub.CallAsync(_link, message, ct).ConfigureAwait(false);
        }

        private readonly IoTHubService _iotHub;
        private readonly Reference _streamId;
        private readonly Reference _remoteId;
        private readonly INameRecord _link;
        private CancellationTokenSource _open;
    }
}