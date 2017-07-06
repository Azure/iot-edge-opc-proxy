// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Linq;
    using System.Collections.Generic;
    using System.Collections.Concurrent;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Threading.Tasks.Dataflow;

    /// <summary>
    /// Proxy link represents a 1:1 link with a remote socket. Proxy
    /// Link is created via LinkRequest, and OpenRequest Handshake.
    /// </summary>
    internal class ProxyLink : IProxyLink {

        /// <summary>
        /// Remote link id
        /// </summary>
        public Reference RemoteId {
            get; private set;
        }

        /// <summary>
        /// Local address on remote side
        /// </summary>
        public SocketAddress LocalAddress {
            get; private set;
        }

        /// <summary>
        /// Peer address on remote side
        /// </summary>
        public SocketAddress PeerAddress {
            get; private set;
        }

        /// <summary>
        /// Bound proxy for this stream
        /// </summary>
        public INameRecord Proxy {
            get; private set;
        }

        /// <summary>
        /// Proxy the socket is bound on.  
        /// </summary>
        public SocketAddress ProxyAddress {
            get => Proxy.Address.ToSocketAddress();
        }

        /// <summary>
        /// Target buffer block
        /// </summary>
        public ITargetBlock<Message> SendBlock {
            get => _send;
        }

        /// <summary>
        /// Source buffer block
        /// </summary>
        public ISourceBlock<Message> ReceiveBlock {
            get => _receive;
        }

        /// <summary>
        /// Constructor for proxy link object
        /// </summary>
        /// <param name="socket"></param>
        /// <param name="proxy"></param>
        /// <param name="remoteId"></param>
        /// <param name="localAddress"></param>
        /// <param name="peerAddress"></param>
        internal ProxyLink(ProxySocket socket, INameRecord proxy, Reference remoteId, 
            SocketAddress localAddress, SocketAddress peerAddress) {

            _socket = socket ?? throw new ArgumentNullException(nameof(socket));

            Proxy = proxy ?? throw new ArgumentNullException(nameof(proxy));
            RemoteId = remoteId ?? throw new ArgumentNullException(nameof(remoteId));
            LocalAddress = localAddress ?? throw new ArgumentNullException(nameof(localAddress));
            PeerAddress = peerAddress ?? throw new ArgumentNullException(nameof(peerAddress));

            _send = new BufferBlock<Message>(new DataflowBlockOptions {
                NameFormat = "Send (in Link) Id={1}",
                EnsureOrdered = true,
                BoundedCapacity = 3
            });

            _receive = new TransformManyBlock<Message, Message>((message) => {
                if (message.Error == (int)SocketError.Closed ||
                    message.TypeId == MessageContent.Close) {
                    // Remote side closed
                    OnRemoteClose(message);
                }
                else if (message.Error != (int)SocketError.Success) {
                    if (!OnReceiveError(message)) {
                        ProxySocket.ThrowIfFailed(message);
                    }
                }
                else if (message.TypeId == MessageContent.Data) {
                    return message.AsEnumerable();
                }
                else {
                    // Todo: log error?
                }
                return Enumerable.Empty<Message>();
            },
            new ExecutionDataflowBlockOptions {
                NameFormat = "Receive (in Link) Id={1}",
                EnsureOrdered = true,
                MaxMessagesPerTask = DataflowBlockOptions.Unbounded,
                SingleProducerConstrained = true,
                BoundedCapacity = 3
            });
        }


        /// <summary>
        /// Begin open of stream, this provides the connection string for the
        /// remote side, that is passed as part of the open request.
        /// </summary>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async Task<OpenRequest> BeginOpenAsync(CancellationToken ct) {
            try {
#if DEBUG_MESSAGE_CONTENT
                var encoding = CodecId.Json;
#else
                var encoding = CodecId.Mpack;
#endif
                _connection = await _socket.Provider.StreamService.CreateConnectionAsync(
                    _streamId, RemoteId, Proxy, encoding).ConfigureAwait(false);

                return OpenRequest.Create(_streamId, (int)encoding, 
                    _connection.ConnectionString != null ?
                        _connection.ConnectionString.ToString() : "", 0, _connection.IsPolled,
#if NO_MSG_OVER_8K 
                    // TODO: Remove this when > 88 k messages are supported
                    800
#else
                    0
#endif
                    );
            }
            catch (OperationCanceledException) {
                return null;
            }
        }

        /// <summary>
        /// Complete connection by waiting for remote side to connect.
        /// </summary>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async Task<bool> TryCompleteOpenAsync(CancellationToken ct) {
            if (_connection == null)
                return false;
            try {
                var stream = await _connection.OpenAsync(ct).ConfigureAwait(false);

                _streamReceive = stream.ReceiveBlock.ConnectTo(_receive);
                _streamSend = _send.ConnectTo(stream.SendBlock);

                return true;
            }
            catch (OperationCanceledException) {
                return false;
            }
        }

        /// <summary>
        /// Send socket option message
        /// </summary>
        /// <param name="option"></param>
        /// <param name="value"></param>
        /// <param name="ct"></param>
        public async Task SetSocketOptionAsync(
            SocketOption option, ulong value, CancellationToken ct) {
            using (var request = Message.Create(_socket.Id, RemoteId, SetOptRequest.Create(
                Property<ulong>.Create((uint)option, value)))) {

                var response = await _socket.Provider.ControlChannel.CallAsync(
                    Proxy, request, TimeSpan.MaxValue, ct).ConfigureAwait(false);
                ProxySocket.ThrowIfFailed(response);
            }
        }

        /// <summary>
        /// Get socket option
        /// </summary>
        /// <param name="option"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async Task<ulong> GetSocketOptionAsync(
            SocketOption option, CancellationToken ct) {
            using (var request = Message.Create(_socket.Id, RemoteId, GetOptRequest.Create(
                option))) {

                var response = await _socket.Provider.ControlChannel.CallAsync(
                    Proxy, request, TimeSpan.MaxValue, ct).ConfigureAwait(false);
                ProxySocket.ThrowIfFailed(response);

                var optionValue = ((GetOptResponse)response.Content).OptionValue as Property<ulong>;
                if (optionValue == null) {
                    throw new ProxyException("Bad option value returned");
                }
                return optionValue.Value;
            }
        }

        /// <summary>
        /// Close link
        /// </summary>
        /// <param name="ct"></param>
        public async Task CloseAsync(CancellationToken ct) {
            var tasks = new Task[] { UnlinkAsync(ct), TerminateConnectionAsync(ct) };
            try {
                // Close both ends
                await Task.WhenAll(tasks).ConfigureAwait(false);
            }
            catch (AggregateException ae) {
                if (ae.InnerExceptions.Count == tasks.Length) {
                    // Only throw if all tasks failed.
                    throw SocketException.Create("Exception during close", ae);
                }
                ProxyEventSource.Log.HandledExceptionAsInformation(this, ae.Flatten());
            }
            catch (Exception e) {
                ProxyEventSource.Log.HandledExceptionAsInformation(this, e);
            }
        }

        /// <summary>
        /// Close the stream part
        /// </summary>
        /// <param name="ct"></param>
        /// <returns></returns>
        private async Task TerminateConnectionAsync(CancellationToken ct) {
            IConnection connection = _connection;
            _connection = null;

            ProxyEventSource.Log.StreamClosing(this, null);
            try {
                try {
                    await SendBlock.SendAsync(Message.Create(_socket.Id, RemoteId,
                    CloseRequest.Create()), ct).ConfigureAwait(false);
                }
                catch { }
                try {
                    await connection.CloseAsync().ConfigureAwait(false);
                }
                catch { }

               // try {
               //     SendBlock.Complete();
               //     await SendBlock.Completion.ConfigureAwait(false);
               // }
               // catch { }
               // try {
               //     ReceiveBlock.Complete();
               //     await ReceiveBlock.Completion.ConfigureAwait(false);
               // }
               // catch { }
            }
            finally {
                ProxyEventSource.Log.StreamClosed(this, null);
            }
        }

        /// <summary>
        /// Remove link on remote proxy through rpc
        /// </summary>
        /// <param name="ct"></param>
        /// <returns></returns>
        private async Task UnlinkAsync(CancellationToken ct) {
            var request = Message.Create(_socket.Id, RemoteId, CloseRequest.Create());
            try {
                var response = await _socket.Provider.ControlChannel.CallAsync(Proxy,
                    request, TimeSpan.FromSeconds(10), ct).ConfigureAwait(false);
                ProxyEventSource.Log.ObjectDestroyed(this);
                if (response != null) {
                    SocketError errorCode = (SocketError)response.Error;
                    if (errorCode != SocketError.Success &&
                        errorCode != SocketError.Timeout &&
                        errorCode != SocketError.Closed) {
                        throw new SocketException(errorCode);
                    }
                }
            }
            catch (Exception e) when (!(e is SocketException)) {
                throw SocketException.Create("Failed to close", e);
            }
            finally {
                request.Dispose();
            }
        }


        /// <summary>
        /// Called when error message is received over stream
        /// </summary>
        /// <param name="message"></param>
        /// <returns>Whether to continue (true) or fail (false)</returns>
        protected virtual bool OnReceiveError(Message message) => 
            false;

        /// <summary>
        /// Called when we determine that remote side closed
        /// </summary>
        protected virtual void OnRemoteClose(Message message) =>
            throw new SocketException("Remote side closed", null, SocketError.Closed);


        /// <summary>
        /// Returns a string that represents the current object.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString() {
            return 
                $"Link {PeerAddress} through {LocalAddress} on {Proxy} "
              + $"with stream {_streamId} (Socket {_socket})";
        }

        protected readonly ProxySocket _socket;
        private IConnection _connection;
        private IDisposable _streamSend;
        private IDisposable _streamReceive;
        private readonly IPropagatorBlock<Message, Message> _send;
        private readonly IPropagatorBlock<Message, Message> _receive;
        private readonly Reference _streamId = new Reference();
    }
}