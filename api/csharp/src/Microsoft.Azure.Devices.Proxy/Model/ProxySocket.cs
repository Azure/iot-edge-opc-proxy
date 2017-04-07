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
    /// Proxy socket implementation, core of socket
    /// </summary>
    public abstract class ProxySocket : IProxySocket, IMessageStream {

        /// <summary>
        /// Reference id for this socket
        /// </summary>
        public Reference Id { get; } = new Reference();

        /// <summary>
        /// Proxy provider implementation to use for communication and lookup
        /// </summary>
        public IProvider Provider { get; private set; }

        /// <summary>
        /// Information for this socket, exchanged with proxy server
        /// </summary>
        public SocketInfo Info { get; private set; }

        /// <summary>
        /// Receive queue for all links
        /// </summary>
        public ConcurrentQueue<Message> ReceiveQueue { get; } = new ConcurrentQueue<Message>();

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="info"></param>
        /// <param name="provider"></param>
        protected ProxySocket(SocketInfo info, IProvider provider) {
            Provider = provider;
            Info = info;
        }

        /// <summary>
        /// Create real proxy socket based on passed socket description
        /// </summary>
        /// <param name="info"></param>
        /// <param name="provider"></param>
        /// <returns></returns>
        public static ProxySocket Create(SocketInfo info, IProvider provider) {
            // Create specializations for tcp and udp
            if (info.Protocol== ProtocolType.Tcp) {
                return new TCPSocket(info, provider);
            }
            else if (info.Protocol == ProtocolType.Udp) {
                return new UDPSocket(info, provider);
            }
            else {
                throw new NotSupportedException("Only UDP and TCP supported");
            }
        }

        /// <summary>
        /// Receives ping responses and handles them one by one through a handler callback.
        /// </summary>
        /// <param name="address"></param>
        /// <param name="handler"></param>
        /// <param name="last"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public Task PingAsync(SocketAddress address,
            Func<Message, INameRecord, CancellationToken, Task<Disposition>> handler,
            Action<Exception> last, CancellationToken ct) =>
            Provider.ControlChannel.BroadcastAsync(new Message(
                Id, Reference.Null, new PingRequest(address)), handler, last, ct);


        /// <summary>
        /// Perform a link handshake with the passed proxies and populate streams
        /// </summary>
        /// <param name="proxies">The proxies (interfaces) to bind the link on</param>
        /// <param name="address">Address to connect to, or null if proxy bound</param>
        /// <param name="ct">Cancels operation</param>
        /// <returns></returns>
        public async Task<bool> LinkAllAsync(IEnumerable<INameRecord> proxies, SocketAddress address, 
            CancellationToken ct) {

            // Complete socket info
            Info.Address = address ?? new NullSocketAddress();
            Info.Flags = address != null ? 0 : (uint)SocketFlags.Passive;
            Info.Options.UnionWith(_optionCache.Select(p => new SocketOptionValue(p.Key, p.Value)));

            var tasks = new List<Task<IProxyLink>>();
            foreach (var proxy in proxies) {
                if (proxy == null)
                    break;
                tasks.Add(CreateLinkAsync(proxy, ct));
            }
            try {
                var results = await Task.WhenAll(tasks.ToArray()).ConfigureAwait(false);
                Links.AddRange(results.Where(v => v != null));
                return results.Any();
            }
            catch (Exception ex) {
                ProxyEventSource.Log.HandledExceptionAsInformation(this, ex);
                // continue...
            }
            return false;
        }

        /// <summary>
        /// Perform excatly one or zero link handshakes with one of the passed proxies 
        /// and populate streams
        /// </summary>
        /// <param name="proxy">The proxy to bind the link on</param>
        /// <param name="address">Address to connect to, or null if proxy bound</param>
        /// <param name="ct">Cancels operation</param>
        /// <returns></returns>
        public async Task<bool> LinkAsync(INameRecord proxy, SocketAddress address, 
            CancellationToken ct) {

            // Complete socket info
            Info.Address = address ?? new NullSocketAddress();
            Info.Flags = address != null ? 0 : (uint)SocketFlags.Passive;
            Info.Options.UnionWith(_optionCache.Select(p => new SocketOptionValue(p.Key, p.Value)));

            try {
                var link = await CreateLinkAsync(proxy, ct).ConfigureAwait(false);
                if (link != null) {
                    Links.Add(link);
                    return true;
                }
            }
            catch(Exception ex) {
                ProxyEventSource.Log.HandledExceptionAsInformation(this, ex);
                // continue...
            }
            return false;
        }

        /// <summary>
        /// List of streams for this socket
        /// </summary>
        protected List<IProxyLink> Links { get; } = new List<IProxyLink>();

        /// <summary>
        /// Returns an address representing the proxy address
        /// </summary>
        public SocketAddress ProxyAddress {
            get {
                return SocketAddressCollection.Create(
                    Links.Where(l => l.ProxyAddress != null).Select(l => l.ProxyAddress));
            }
        }

        /// <summary>
        /// Returns an address representing the address bound on proxy
        /// </summary>
        public SocketAddress LocalAddress {
            get {
                return SocketAddressCollection.Create(
                    Links.Where(l => l.LocalAddress != null).Select(l => l.LocalAddress));
            }
        }

        /// <summary>
        /// Returns an address representing the peer
        /// </summary>
        public SocketAddress PeerAddress {
            get {
                return SocketAddressCollection.Create(
                    Links.Where(l => l.PeerAddress != null).Select(l => l.PeerAddress));
            }
        }

        /// <summary>
        /// Link one remote endpoint
        /// </summary>
        /// <param name="proxy"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        private async Task<IProxyLink> CreateLinkAsync(INameRecord proxy, 
            CancellationToken ct) {

            ProxyEventSource.Log.LinkCreate(this, proxy.Name, Info.Address);
            // Create link, i.e. perform bind, connect, listen, etc. on proxy
            Message response = await Provider.ControlChannel.CallAsync(proxy,
                new Message(Id, Reference.Null, new LinkRequest {
                    Properties = Info
                }), ct);
            if (response == null || response.Error != (int)SocketError.Success) {
                ProxyEventSource.Log.LinkFailure(this, proxy.Name, Info, response, null);
                return null;
            }

            var linkResponse = response.Content as LinkResponse;
            if (linkResponse == null) {
                ProxyEventSource.Log.LinkFailure(this, proxy.Name, Info, response, null);
                return null;
            }

            // now create local link and open link for streaming
            var link = new ProxyLink(this, proxy, linkResponse.LinkId,
                linkResponse.LocalAddress, linkResponse.PeerAddress);
            try {
                // Broker connection string to proxy
                var openRequest = await link.BeginOpenAsync(ct).ConfigureAwait(false);
                ProxyEventSource.Log.LinkOpen(this, proxy.Name, Info.Address);

                await Provider.ControlChannel.CallAsync(proxy,
                    new Message(Id, linkResponse.LinkId, openRequest), ct).ConfigureAwait(false);

                // Wait until remote side opens stream connection
                bool success = await link.TryCompleteOpenAsync(ct).ConfigureAwait(false);
                if (success) {
                    ProxyEventSource.Log.LinkComplete(this, proxy.Name, Info.Address);
                    return link;
                }
            }
            catch (Exception e) {
                // Try to close remote side
                await link.CloseAsync(CancellationToken.None).ConfigureAwait(false);
                ProxyEventSource.Log.LinkFailure(this, proxy.Name, Info.Address, null, e);
            }
            return null;
        }

        /// <summary>
        /// Send socket option message to all streams
        /// </summary>
        /// <param name="option"></param>
        /// <param name="value"></param>
        /// <param name="ct"></param>
        public async Task SetSocketOptionAsync(SocketOption option, ulong value, 
            CancellationToken ct) {
            if (!Links.Any()) {
                _optionCache[option] = value;
            }
            try {
                await Task.WhenAll(Links.Select(
                    i => i.SetSocketOptionAsync(option, value, ct))).ConfigureAwait(false);
            }
            catch (Exception e) {
                throw new SocketException(e);
            }
        }

        /// <summary>
        /// Get socket option
        /// </summary>
        /// <param name="option"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async Task<ulong> GetSocketOptionAsync(SocketOption option, 
            CancellationToken ct) {
            if (!Links.Any()) {
                return _optionCache.ContainsKey(option) ? _optionCache[option] : 0;
            }
            var cts = new CancellationTokenSource();
            ct.Register(() => {
                cts.Cancel();
            });
            var tasks = Links.Select(
                i => i.GetSocketOptionAsync(option, cts.Token)).ToList();
            Exception e = null;
            while (tasks.Count > 0) {
                var result = await Task.WhenAny(tasks).ConfigureAwait(false);
                try {
                    ulong value = await result.ConfigureAwait(false);
                    cts.Cancel(); // Cancel the rest
                    return value;
                }
                catch (Exception thrown) {
                    tasks.Remove(result);
                    e = thrown;
                }
            }
            throw new SocketException(e);
        }

        /// <summary>
        /// Close all socket streams and thus this socket
        /// </summary>
        /// <param name="ct"></param>
        public async Task CloseAsync(CancellationToken ct) {
            try {
                await Task.WhenAll(Links.Select(i => i.CloseAsync(ct))).ConfigureAwait(false);
            }
            catch (Exception e) {
                throw new SocketException(e);
            }
        }

        protected IProxyStream _stream;

        /// <summary>
        /// Sends array of bytes on this socket
        /// </summary>
        /// <param name="buffer"></param>
        /// <param name="endpoint"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async Task<int> SendAsync(ArraySegment<byte> buffer, SocketAddress endpoint, 
            CancellationToken ct) {
            if (_stream == null)
                throw new SocketException("Socket not ready for sending");
            return await _stream.SendAsync(buffer, endpoint, ct).ConfigureAwait(false);
        }

        /// <summary>
        /// Receives array of bytes on this socket
        /// </summary>
        /// <param name="buffer"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async Task<ProxyAsyncResult> ReceiveAsync(ArraySegment<byte> buffer, 
            CancellationToken ct) {
            if (_stream == null)
                throw new SocketException("Socket not ready for receiving");
            return await _stream.ReceiveAsync(buffer, ct).ConfigureAwait(false);
        }

        /// <summary>
        /// Receive from one of the contained streams
        /// </summary>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async Task ReceiveAsync(CancellationToken ct) {
            if (!Links.Any()) {
                throw new SocketException(SocketError.Closed);
            }

            // Fill receive queue from any of the link's receive queue.  If queue is empty
            // replenish it from all streams...
            while (true) {
                foreach (var link in Links) {
                    Message message;
                    var queue = link.ReceiveQueue;
                    if (queue == null) {
                        throw new SocketException(SocketError.Closed);
                    }
                    while (queue.TryDequeue(out message)) {
                        if (message.TypeId == MessageContent.Close) {
                            // Remote side closed, close link
                            Links.Remove(link);
                            try {
                                await link.CloseAsync(CancellationToken.None).ConfigureAwait(false);
                            }
                            catch { }
                            if (!Links.Any()) {
                                throw new SocketException("Remote side closed",
                                    null, SocketError.Closed);
                            }
                        }
                        else {
                            ReceiveQueue.Enqueue(message);
                        }
                    }
                }
                if (ReceiveQueue.Any()) { 
                    return;
                }
                else {
                    try {
                        var tasks = Links.Select(i => i.ReceiveAsync(ct));
                        await Task.WhenAny(tasks).ConfigureAwait(false);
                    }
                    catch(AggregateException ae) {
                        throw new SocketException("Receive await failed", 
                            ae, ae.GetSocketError());
                    }
                }
            }
        }

        /// <summary>
        /// Send to all contained streams
        /// </summary>
        /// <param name="message"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public Task SendAsync(Message message, CancellationToken ct) {
            if (!Links.Any()) {
                throw new SocketException(SocketError.Closed);
            }
            try {
                return Task.WhenAll(Links.Select(i => i.SendAsync(message, ct)));
            }
            catch (Exception e) {
                throw new SocketException(e);
            }
        }

        /// <summary>
        /// Select the proxy to bind to
        /// </summary>
        /// <param name="endpoint"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public virtual async Task BindAsync(SocketAddress endpoint, CancellationToken ct) {
            // Proxy selected, look up name records for the proxy address
            var bindList = await Provider.NameService.LookupAsync(
                endpoint.ToString(), RecordType.Proxy, ct).ConfigureAwait(false);
            _bindList = bindList;
            if (!_bindList.Any()) {
                throw new SocketException(SocketError.NoAddress);
            }
        }

        public abstract Task ConnectAsync(
            SocketAddress address, CancellationToken ct);

        public abstract Task ListenAsync(
            int backlog, CancellationToken ct);

        /// <summary>
        /// Returns a string that represents the socket.
        /// </summary>
        /// <returns>A string that represents the socket.</returns>
        public override string ToString() {
            return $"Socket {Id} : {Info}";
        }

        //
        // Helper to throw if error code is not success
        //
        internal static void ThrowIfFailed(Message response) {
            if (response == null) {
                throw new SocketException(SocketError.Fatal);
            }
            SocketError errorCode = (SocketError)response.Error;
            if (errorCode != SocketError.Success &&
                errorCode != SocketError.Timeout) {
                throw new SocketException(errorCode);
            }
        }

        protected IEnumerable<INameRecord> _bindList;
        private readonly Dictionary<SocketOption, ulong> _optionCache = 
            new Dictionary<SocketOption, ulong>();
    }
}
