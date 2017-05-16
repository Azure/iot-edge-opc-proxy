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
    /// Proxy link represents a 1:1 link with a remote socket. Proxy
    /// Link is created via LinkRequest, and OpenRequest Handshake.
    /// </summary>
    internal sealed class ProxyLink : IProxyLink {

        /// <summary>
        /// Remote link id
        /// </summary>
        public Reference RemoteId { get; private set; }

        /// <summary>
        /// Local address on remote side
        /// </summary>
        public SocketAddress LocalAddress { get; private set; }

        /// <summary>
        /// Peer address on remote side
        /// </summary>
        public SocketAddress PeerAddress { get; private set; }

        /// <summary>
        /// Bound proxy for this stream
        /// </summary>
        public INameRecord Proxy { get; private set; }

        /// <summary>
        /// Proxy the socket is bound on.  
        /// </summary>
        public SocketAddress ProxyAddress => Proxy.Address.ToSocketAddress();

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
            _streamId = new Reference();
            _socket = socket;
            Proxy = proxy;
            RemoteId = remoteId;
            LocalAddress = localAddress;
            PeerAddress = peerAddress;
        }

        /// <summary>
        /// Begin open of stream, this provides the connection string for the
        /// remote side, that is passed as part of the open request.
        /// </summary>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async Task<OpenRequest> BeginOpenAsync(CancellationToken ct) {
            try {
                _connection = await _socket.Provider.StreamService.CreateConnectionAsync(
                    _streamId, RemoteId, Proxy).ConfigureAwait(false);
                return new OpenRequest {
                    ConnectionString = _connection.ConnectionString != null ?
                        _connection.ConnectionString.ToString() : "",
                    IsPolled = _connection.IsPolled,
                    StreamId = _streamId
                };
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
                _stream = await _connection.OpenAsync(ct).ConfigureAwait(false);
                return true;
            }
            catch (OperationCanceledException) {
                return false;
            }
        }

        /// <summary>
        /// Receive queue from stream to read from
        /// </summary>
        public ConcurrentQueue<Message> ReceiveQueue => 
            _stream?.ReceiveQueue;

        /// <summary>
        /// Receive more on link stream
        /// </summary>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async Task ReceiveAsync(CancellationToken ct) {
            if (_stream == null) {
                throw new SocketException(SocketError.Closed);
            }
            await _stream.ReceiveAsync(ct);
        }

        /// <summary>
        /// Forward cloned message through this link
        /// </summary>
        /// <param name="message"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async Task SendAsync(Message message, CancellationToken ct) {
            if (_stream == null) {
                throw new SocketException(SocketError.Closed);
            }
            await _stream.SendAsync(new Message(
                _streamId, RemoteId, Proxy.Address, message.Content), ct);
        }


        /// <summary>
        /// Send socket option message
        /// </summary>
        /// <param name="option"></param>
        /// <param name="value"></param>
        /// <param name="ct"></param>
        public async Task SetSocketOptionAsync(
            SocketOption option, ulong value, CancellationToken ct) {

            var response = await _socket.Provider.ControlChannel.CallAsync(Proxy,
                new Message(_socket.Id, RemoteId, new SetOptRequest(
                    new SocketOptionValue(option, value))), ct).ConfigureAwait(false);
            ProxySocket.ThrowIfFailed(response);
        }

        /// <summary>
        /// Get socket option
        /// </summary>
        /// <param name="option"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async Task<ulong> GetSocketOptionAsync(
            SocketOption option, CancellationToken ct) {
            var response = await _socket.Provider.ControlChannel.CallAsync(Proxy,
                new Message(_socket.Id, RemoteId, new GetOptRequest(
                    option)), ct).ConfigureAwait(false);
            ProxySocket.ThrowIfFailed(response);
            return ((GetOptResponse)response.Content).OptionValue.Value;
        }

        /// <summary>
        /// Close link
        /// </summary>
        /// <param name="ct"></param>
        public async Task CloseAsync(CancellationToken ct) {
            var tasks = new Task[] { UnlinkAsync(ct), CloseStreamAsync(ct) };
            try {
                // Close both ends
                await Task.WhenAll(tasks).ConfigureAwait(false);
            }
            catch (AggregateException ae) {
                if (ae.InnerExceptions.Count == tasks.Length) {
                    // Only throw if all tasks failed.
                    throw new SocketException("Exception during close", ae.GetSocketError());
                }
                ProxyEventSource.Log.HandledExceptionAsInformation(this, ae);
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
        private async Task CloseStreamAsync(CancellationToken ct) {
            IConnection connection = _connection;
            IMessageStream stream = _stream;
            _connection = null;
            _stream = null;
            try {
                ProxyEventSource.Log.StreamClosing(this, _stream);
                await stream.SendAsync(new Message(_socket.Id, RemoteId, 
                    new CloseRequest()), ct).ConfigureAwait(false);
            }
            finally {
                await connection.CloseAsync().ConfigureAwait(false);
                ProxyEventSource.Log.StreamClosed(this, null);
            }
        }

        /// <summary>
        /// Remove link on remote proxy through rpc
        /// </summary>
        /// <param name="ct"></param>
        /// <returns></returns>
        private async Task UnlinkAsync(CancellationToken ct) {
            var response = await _socket.Provider.ControlChannel.CallAsync(Proxy,
                new Message(_socket.Id, RemoteId, 
                    new CloseRequest()), ct).ConfigureAwait(false);
            ProxyEventSource.Log.ObjectDestroyed(this);
            if (response != null) {
                SocketError errorCode = (SocketError)response.Error;
                if (errorCode != SocketError.Success &&
                    errorCode != SocketError.Timeout &&
                    errorCode != SocketError.Closed) {
                    throw new SocketException(errorCode);
                }
            }
            else {
                throw new SocketException(SocketError.Closed);
            }
        }

        /// <summary>
        /// Returns a string that represents the current object.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString() {
            return 
                $"Link {PeerAddress} through {LocalAddress} on {Proxy} "
              + $"with stream {_streamId} (Socket {_socket})";
        }

        private IMessageStream _stream;
        private readonly ProxySocket _socket;
        private IConnection _connection;
        private readonly Reference _streamId;
    }
}