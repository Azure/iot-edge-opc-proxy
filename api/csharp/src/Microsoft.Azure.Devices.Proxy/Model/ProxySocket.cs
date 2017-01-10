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
        public BlockingCollection<Message> ReceiveQueue { get; } = new BlockingCollection<Message>();

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
        /// <param name="family"></param>
        /// <param name="sockType"></param>
        /// <param name="protocolType"></param>
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
        /// Receives ping responses and handles them one by one through a producer/consumer queue
        /// </summary>
        /// <param name="handler">Handler that reads the responding proxy records from blocking collection.</param>
        /// <param name="timeout">Timeout for broadcast method calls</param>
        /// <param name="ct">Cancels operation</param>
        /// <returns></returns>
        public async Task PingAsync(SocketAddress address,
            Func<BlockingCollection<INameRecord>, Task> handler,
            TimeSpan timeout, CancellationToken ct) {

            // Producer / consumer queue for ping responses, ensures linking on response one at a time
            var queue = new BlockingCollection<INameRecord>(); // Default is queue

            var cts = new CancellationTokenSource();
            ct.Register(() => {
                cts.Cancel();
                queue.CompleteAdding();
            });

            var task = Provider.ControlChannel.BroadcastAsync(
                    new Message(Id, Reference.Null, new PingRequest(address)),
                (m, r) => {
                    queue.Add(r);
                    return Task.FromResult(false);
                },
                timeout, cts.Token).ContinueWith(t => {
                    if (t.IsCompleted) {
                        queue.CompleteAdding();
                    }
                });

            await handler(queue);
            try {
                cts.Cancel();   // Cancel all other broadcast no matter what..
                await task;
            }
            catch(Exception) {
                return;
            }
        }


        /// <summary>
        /// Perform a link handshake with the passed proxies and populate streams
        /// </summary>
        /// <param name="proxies">The proxies (interfaces) to bind the link on</param>
        /// <param name="address">Address to connect to, or null if proxy bound</param>
        /// <param name="timeout">Method timeout</param>
        /// <param name="ct">Cancels operation</param>
        /// <returns></returns>
        public async Task<bool> LinkAllAsync(
            IEnumerable<INameRecord> proxies, SocketAddress address, 
            TimeSpan timeout, CancellationToken ct) {

            // Complete socket info
            Info.Address = address ?? new NullSocketAddress();
            Info.Flags = address != null ? 0 : (uint)SocketFlags.Passive;
            Info.Options.UnionWith(_optionCache.Select(p => new SocketOptionValue(p.Key, p.Value)));

            var cts = new CancellationTokenSource();
            ct.Register(() => {
                cts.Cancel();
                });
            var tasks = new List<Task<IProxyLink>>();
            foreach (var proxy in proxies) {
                if (proxy == null)
                    break;
                tasks.Add(CreateLinkAsync(proxy, timeout, cts.Token));
            }
            try {
                var results = await Task.WhenAll(tasks.ToArray());
                Links.AddRange(results.Where(v => v != null));
                return results.Any();
            }
            catch (Exception) {
            }
            return false;
        }

        /// <summary>
        /// Perform excatly one or zero link handshakes with one of the passed proxies 
        /// and populate streams
        /// </summary>
        /// <param name="proxies">The proxies (interfaces) to bind the link on</param>
        /// <param name="address">Address to connect to, or null if proxy bound</param>
        /// <param name="timeout">Method timeout</param>
        /// <param name="ct">Cancels operation</param>
        /// <returns></returns>
        public async Task<bool> LinkAsync(
            IEnumerable<INameRecord> proxies, SocketAddress address,
            TimeSpan timeout, CancellationToken ct) {

            // Complete socket info
            Info.Address = address ?? new NullSocketAddress();
            Info.Flags = address != null ? 0 : (uint)SocketFlags.Passive;
            Info.Options.UnionWith(_optionCache.Select(p => new SocketOptionValue(p.Key, p.Value)));

            var cts = new CancellationTokenSource();
            ct.Register(() => {
                cts.Cancel();
            });
            var tasks = new List<Task<IProxyLink>>();
            foreach (var proxy in proxies) {
                if (proxy == null)
                    break;
                try {
                    var link = await CreateLinkAsync(proxy, timeout, cts.Token);
                    if (link == null)
                        continue;
                    Links.Add(link);
                    return true;
                }
                catch(Exception) {
                    // continue...
                }
            }
            return false;
        }

        /// <summary>
        /// List of streams for this socket
        /// </summary>
        protected List<IProxyLink> Links { get; } = new List<IProxyLink>();

        /// <summary>
        /// Link one remote endpoint
        /// </summary>
        /// <param name="proxy"></param>
        /// <param name="timeout"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        private async Task<IProxyLink> CreateLinkAsync(INameRecord proxy, 
            TimeSpan timeout, CancellationToken ct) {
            // Create link, i.e. perform bind, connect, listen, etc. on proxy
            Message response = await Provider.ControlChannel.CallAsync(proxy,
                new Message(Id, Reference.Null, new LinkRequest {
                    Properties = Info
                }),
                timeout, ct);
            if (response == null || response.Error != (int)SocketError.Ok) {
                return null;
            }

            var linkResponse = response.Content as LinkResponse;

            // now create local link and open link for streaming
            var link = new ProxyLink(this, proxy, linkResponse.LinkId,
                linkResponse.LocalAddress, linkResponse.PeerAddress);
            try {
                // Broker connection string to proxy
                var openRequest = await link.BeginOpenAsync(timeout, ct);

                response = await Provider.ControlChannel.CallAsync(proxy,
                    new Message(Id, linkResponse.LinkId, openRequest), timeout, ct).ConfigureAwait(false);

                if (response == null || response.Error == (int)SocketError.Ok) {
                    // Wait until remote side opens stream connection
                    if (await link.TryCompleteOpenAsync(ct))
                        return link;
                }
            }
            catch (Exception e) {
                // Try to close remote side
                await link.CloseAsync(timeout, CancellationToken.None);
                ProxyEventSource.Log.HandledExceptionAsError(this, e);
            }
            return null;
        }

        /// <summary>
        /// Returns records for socket address
        /// </summary>
        /// <param name="address"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        protected async Task<IEnumerable<INameRecord>> LookupRecordsForAddressAsync(
            SocketAddress address, CancellationToken ct) {
            if (address.Family == AddressFamily.Proxy) {
                return await Provider.NameService.LookupAsync(
                    ((ProxySocketAddress)address).Host, RecordType.Proxy, ct);
            }
            else if (address.Family == AddressFamily.InterNetworkV6) {
                return await Provider.NameService.LookupAsync(
                    ((Inet6SocketAddress)address).ToReference(), RecordType.Proxy, ct);
            }
            else {
                throw new NotSupportedException("Can only bind to proxy reference");
            }
        }

        /// <summary>
        /// Returns a proxy socket address for a user provided socket address by
        /// e.g. translating id into name.  Returns proxy socket address as is.
        /// </summary>
        /// <param name="address"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        protected async Task<SocketAddress> TranslateAddressAsync(
            SocketAddress address, CancellationToken ct) {
            if (address.Family != AddressFamily.InterNetwork &&
                address.Family != AddressFamily.InterNetworkV6) {
                return address;
            }

            var records = await LookupRecordsForAddressAsync(address, ct);
            if (!records.Any()) {
                return address; // Try our luck with the original address
            }
            return new ProxySocketAddress(
                records.First().Name, ((InetSocketAddress)address).Port);
        }

        /// <summary>
        /// Send socket option message to all streams
        /// </summary>
        /// <param name="option"></param>
        /// <param name="value"></param>
        /// <param name="timeout"></param>
        /// <param name="ct"></param>
        public async Task SetSocketOptionAsync(
            SocketOption option, ulong value, TimeSpan timeout, CancellationToken ct) {
            if (!Links.Any()) {
                _optionCache[option] = value;
            }
            try {
                await Task.WhenAll(Links.Select(
                    i => i.SetSocketOptionAsync(option, value, timeout, ct)));
            }
            catch (Exception e) {
                throw new SocketException(e);
            }
        }

        /// <summary>
        /// Get socket option
        /// </summary>
        /// <param name="option"></param>
        /// <param name="timeout"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async Task<ulong> GetSocketOptionAsync(
            SocketOption option, TimeSpan timeout, CancellationToken ct) {
            if (!Links.Any()) {
                return _optionCache.ContainsKey(option) ? _optionCache[option] : 0;
            }
            var cts = new CancellationTokenSource(timeout);
            ct.Register(() => {
                cts.Cancel();
            });
            var tasks = Links.Select(
                i => i.GetSocketOptionAsync(option, timeout, cts.Token)).ToList();
            Exception e = null;
            while (tasks.Count > 0) {
                var result = await Task.WhenAny(tasks);
                try {
                    ulong value = await result;
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
        /// <param name="timeout"></param>
        /// <param name="ct"></param>
        public Task CloseAsync(TimeSpan timeout, CancellationToken ct) {
            try {
                return Task.WhenAll(Links.Select(
                    i => i.CloseAsync(timeout, ct)));
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
        public async Task<int> SendAsync(ArraySegment<byte> buffer,
            SocketAddress endpoint, CancellationToken ct) {
            if (_stream == null)
                throw new SocketException("Socket not ready for sending");
            return await _stream.SendAsync(buffer, endpoint, ct);
        }

        /// <summary>
        /// Receives array of bytes on this socket
        /// </summary>
        /// <param name="buffer"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async Task<ProxyAsyncResult> ReceiveAsync(
            ArraySegment<byte> buffer, CancellationToken ct) {
            if (_stream == null)
                throw new SocketException("Socket not ready for receiving");
            return await _stream.ReceiveAsync(buffer, ct);
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
            Message message;
            while (true) {
                if (-1 != BlockingCollection<Message>.TryTakeFromAny(
                    Links.Select(i => i.ReceiveQueue).ToArray(), out message, 0, ct)) {
                    ReceiveQueue.Add(message);
                    return;
                }
                else {
                    try {
                        await Task.WhenAll(Links.Select(i => i.ReceiveAsync(ct)));
                    }
                    catch(AggregateException e) {
                        throw new SocketException(e);
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
        
        public abstract Task BindAsync(
            SocketAddress endpoint, TimeSpan timeout, CancellationToken ct);

        public abstract Task ConnectAsync(
            SocketAddress address, TimeSpan timeout, CancellationToken ct);

        public abstract Task ListenAsync(
            int backlog, TimeSpan timeout, CancellationToken ct);


        //
        // Helper to throw if error code is not success
        //
        internal static void ThrowIfFailed(Message response) {
            if (response == null) {
                throw new SocketException(SocketError.Fatal);
            }
            SocketError errorCode = (SocketError)response.Error;
            if (errorCode != SocketError.Ok &&
                errorCode != SocketError.Timeout) {
                throw new SocketException(errorCode);
            }
        }

        private Dictionary<SocketOption, ulong> _optionCache = new Dictionary<SocketOption, ulong>();
    }
}
