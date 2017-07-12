// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Linq;
    using System.Collections.Generic;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Threading.Tasks.Dataflow;

    /// <summary>
    /// Broadcast socket implementation. Maintains a list of n proxy links that it manages,
    /// including  keep alive and re-connects. In addition, it provides input and 
    /// output transform from binary buffer to actual messages that are serialized/
    /// deserialized at the provider level (next level).
    /// </summary>
    public abstract class BroadcastSocket : ProxySocket {

        /// <summary>
        /// List of proxy links - i.e. open sockets or bound sockets on the remote
        /// proxy server.  This is a list of links allowing this socket to create 
        /// aggregate and broadcast type networks across multiple proxies.
        /// </summary>
        protected List<IProxyLink> Links {
            get;
        } = new List<IProxyLink>();

        /// <summary>
        /// Returns an address representing the proxy address(s)
        /// </summary>
        public override SocketAddress ProxyAddress {
            get => SocketAddressCollection.Create(
                Links.Where(l => l.ProxyAddress != null).Select(l => l.ProxyAddress));
        }

        /// <summary>
        /// Returns an address representing the address(es) bound on proxy
        /// </summary>
        public override SocketAddress LocalAddress {
            get => SocketAddressCollection.Create(
                Links.Where(l => l.LocalAddress != null).Select(l => l.LocalAddress));
        }

        /// <summary>
        /// Returns an address representing the peer(s) of all links.
        /// </summary>
        public override SocketAddress PeerAddress {
            get => SocketAddressCollection.Create(
                Links.Where(l => l.PeerAddress != null).Select(l => l.PeerAddress));
        }

        /// <summary>
        /// Send block
        /// </summary>
        public override ITargetBlock<Message> SendBlock {
            get => _send;
        }

        /// <summary>
        /// Receive block
        /// </summary>
        public override ISourceBlock<Message> ReceiveBlock {
            get => _receive;
        }

        /// <summary>
        /// Constructor - hidden, use Create to create a proxy socket object.
        /// </summary>
        /// <param name="info">Properties that the socket should have</param>
        /// <param name="provider">The provider to use for communication, etc.</param>
        protected BroadcastSocket(SocketInfo info, IProvider provider) : 
            base (info, provider) {

            // Broadcast to all connected links
            _send = new BroadcastBlock<Message>(message => message.Clone(),
            new DataflowBlockOptions {
                NameFormat = "Send (in Socket) Id={1}",
                EnsureOrdered = true,
                BoundedCapacity = 1
            });

            // Receive from all connected links
            _receive = new BufferBlock<Message>(
            new DataflowBlockOptions {
                NameFormat = "Receive (in Socket) Id={1}",
                EnsureOrdered = true,
                BoundedCapacity = 3
            });

            // Reconnect block for connected links to push themselves for reconnect
            _reconnect = new TransformBlock<BroadcastLink, INameRecord>(link => link.Proxy, 
            new ExecutionDataflowBlockOptions {
                NameFormat = "Reconnect (in Socket) Id={1}",
                EnsureOrdered = true,
                BoundedCapacity = 3
            });
        }

        /// <summary>
        /// Close all socket streams and thus this socket
        /// </summary>
        /// <param name="ct"></param>
        public override async Task CloseAsync(CancellationToken ct) {
            try {
                var links = Links.ToArray();
                await Task.WhenAll(links.Select(l => l.CloseAsync(ct))).ConfigureAwait(false);
            }
            catch (Exception e) {
                throw SocketException.Create("Close failed", e);
            }
            finally {
                _open.Cancel(true);
            }
        }

        /// <summary>
        /// Send socket option message to all streams
        /// </summary>
        /// <param name="option"></param>
        /// <param name="value"></param>
        /// <param name="ct"></param>
        public override async Task SetSocketOptionAsync(SocketOption option, ulong value,
            CancellationToken ct) {
            if (!Links.Any()) {
                _optionCache[option] = value;
                return;
            }
            try {
                await Task.WhenAll(Links.Select(
                    i => i.SetSocketOptionAsync(option, value, ct))).ConfigureAwait(false);
            }
            catch (Exception e) {
                throw SocketException.Create("SetSocketOption failed", e);
            }
        }

        /// <summary>
        /// Get socket option
        /// </summary>
        /// <param name="option"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public override async Task<ulong> GetSocketOptionAsync(SocketOption option,
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
            throw SocketException.Create("GetSocketOption failed", e);
        }

        /// <summary>
        /// Bind to provided endpoint(s) - sets up tpl flow network to mimic binding to 
        /// a set of endpoints or all endpoints, and handling reconnects until the socket
        /// is closed.  The socket is bound even if there are no endpoints immediately
        /// connected (e.g. if proxies are off).  However, the idea is that the links are
        /// automatically connected or reconnected as they become available.
        /// </summary>
        /// <param name="endpoint"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        protected async Task LinkAsync(SocketAddress endpoint, CancellationToken ct) {

            // Complete socket info
            Info.Options.UnionWith(_optionCache.Select(p => Property<ulong>.Create(
                (uint)p.Key, p.Value)));

            var input = DataflowMessage<INameRecord>.CreateAdapter(new ExecutionDataflowBlockOptions {
                NameFormat = "Adapt (Bind) Id={1}",
                CancellationToken = _open.Token,
                EnsureOrdered = false,
                SingleProducerConstrained = true,
                MaxDegreeOfParallelism = DataflowBlockOptions.Unbounded,
                MaxMessagesPerTask = DataflowBlockOptions.Unbounded
            });

            // handle errors by throttling and then retry...
            var errors = new TransformBlock<DataflowMessage<INameRecord>, DataflowMessage<INameRecord>>(
            async (error) => {
                await Task.Delay(error.Exceptions.Count * _throttleDelayMs).ConfigureAwait(false);
                return error;
            },
            new ExecutionDataflowBlockOptions {
                NameFormat = "Error (Bind) Id={1}",
                CancellationToken = _open.Token
            });

            var query = Provider.NameService.Lookup(new ExecutionDataflowBlockOptions {
                NameFormat = "Lookup (Bind) Id={1}",
                CancellationToken = _open.Token,
                EnsureOrdered = false,
                MaxDegreeOfParallelism = 2, // 2 parallel lookups
                MaxMessagesPerTask = 1
            });

            var linker = CreateLinkBlock(errors, new ExecutionDataflowBlockOptions {
                NameFormat = "Link (Bind) Id={1}",
                CancellationToken = _open.Token,
                MaxDegreeOfParallelism = DataflowBlockOptions.Unbounded,
                EnsureOrdered = false,
                MaxMessagesPerTask = 1
            });

            // When first connected mark tcs as complete
            var connected = new ActionBlock<IProxyLink>(l => {
                lock (Links) {
                    var link = l as BroadcastLink;
                    link.Attach(_send, _receive);
                    Links.Add(link);
                }
            }, new ExecutionDataflowBlockOptions {
                NameFormat = "Connected (Bind) Id={1}",
                CancellationToken = _open.Token,
                MaxDegreeOfParallelism = DataflowBlockOptions.Unbounded,
                SingleProducerConstrained = true,
                EnsureOrdered = false
            });

            query.ConnectTo(input);
            _reconnect.ConnectTo(input);
            input.ConnectTo(linker);
            errors.ConnectTo(linker);
            linker.ConnectTo(connected);

            //
            // Query generates name records from device registry based on the passed 
            // proxy information. These are then posted to the linker for linking. 
            //
            var queries = new List<Task<bool>>();
            if (endpoint == null || endpoint is AnySocketAddress) {
                queries.Add(query.SendAsync(Provider.NameService.NewQuery(
                    Reference.All, NameRecordType.Proxy), ct));
            }
            else {
                while (endpoint.Family == AddressFamily.Bound) {
                    // Unwrap bound address
                    endpoint = ((BoundSocketAddress)endpoint).LocalAddress;
                }

                if (endpoint.Family == AddressFamily.Collection) {
                    foreach (var item in ((SocketAddressCollection)endpoint).Addresses()) {
                        queries.Add(query.SendAsync(Provider.NameService.NewQuery(
                            item.ToString(), NameRecordType.Proxy), ct));
                    }
                }
                else {
                    queries.Add(query.SendAsync(Provider.NameService.NewQuery(
                        endpoint.ToString(), NameRecordType.Proxy), ct));
                }
            }
            await Task.WhenAll(queries.ToArray()).ConfigureAwait(false);

            // Now the network is primed with proxy endpoints
        }

        /// <summary>
        /// Reconnect the broadcast socket by requeing to reconnect block.
        /// </summary>
        /// <param name="broadcastLink"></param>
        internal void Reconnect(BroadcastLink link) {
            lock (Links) {
                Links.Remove(link);
            }
            _reconnect.Post(link);
        }

        /// <summary>
        /// Create broadcast link
        /// </summary>
        /// <param name="proxy"></param>
        /// <param name="linkId"></param>
        /// <param name="localAddress"></param>
        /// <param name="peerAddress"></param>
        /// <returns></returns>
        protected override IProxyLink CreateLink(INameRecord proxy, Reference linkId,
            SocketAddress localAddress, SocketAddress peerAddress) {
            // now create broadcast link and open link for streaming
            return new BroadcastLink(this, proxy, linkId, localAddress, peerAddress);
        }

        public override void Dispose() {
            _open.Cancel(false);
        }

        protected CancellationTokenSource _open = new CancellationTokenSource();
        private readonly IPropagatorBlock<Message, Message> _send;
        private readonly IPropagatorBlock<Message, Message> _receive;
        private readonly IPropagatorBlock<BroadcastLink, INameRecord> _reconnect;
        private const int _throttleDelayMs = 1000;
    }
}
