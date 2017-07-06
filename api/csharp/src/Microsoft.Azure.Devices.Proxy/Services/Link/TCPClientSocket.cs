// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Linq;
    using System.Collections.Generic;
    using System.Threading;
    using System.Threading.Tasks.Dataflow;
    using System.Threading.Tasks;

    /// <summary>
    /// Concrete tcp proxy socket implementation - wraps a link to a proxy connected
    /// to the host address.
    /// </summary>
    internal class TCPClientSocket : ProxySocket {

        /// <summary>
        /// Returns the proxy address of the underlying link
        /// </summary>
        public override SocketAddress ProxyAddress {
            get => _link.ProxyAddress;
        }

        /// <summary>
        /// Local address on proxy for underlying link
        /// </summary>
        public override SocketAddress LocalAddress {
            get => _link.LocalAddress;
        }

        /// <summary>
        /// peer address of the Host the linked proxy is connected to.
        /// </summary>
        public override SocketAddress PeerAddress {
            get => _link.PeerAddress;
        }

        /// <summary>
        /// The underlying link's send block
        /// </summary>
        public override ITargetBlock<Message> SendBlock {
            get => _link.SendBlock;
        }

        /// <summary>
        /// The underlying link's receive block
        /// </summary>
        public override ISourceBlock<Message> ReceiveBlock {
            get => _link.ReceiveBlock;
        }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="info"></param>
        /// <param name="provider"></param>
        internal TCPClientSocket(SocketInfo info, IProvider provider) :
            base(info, provider) {
            if (info.Type != SocketType.Stream) {
                throw new ArgumentException("Tcp only supports streams");
            }
        }

        /// <summary>
        /// Host record the underlying link is connected to
        /// </summary>
        public INameRecord Host { get; private set; }

        /// <summary>
        /// Create unicast link
        /// </summary>
        /// <param name="proxy"></param>
        /// <param name="linkId"></param>
        /// <param name="localAddress"></param>
        /// <param name="peerAddress"></param>
        /// <returns></returns>
        protected override IProxyLink CreateLink(INameRecord proxy, Reference linkId,
            SocketAddress localAddress, SocketAddress peerAddress) {
            // now create local link and open link for streaming
            return new ProxyLink(this, proxy, linkId, localAddress, peerAddress);
        }

        /// <summary>
        /// Select the proxy to bind to
        /// </summary>
        /// <param name="endpoint"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public override Task BindAsync(SocketAddress endpoint, CancellationToken ct) {
            if (_boundEndpoint != null) {
                throw new SocketException(
                    "Cannot double bind already bound socket. Use collection address.");
            }
            _boundEndpoint = endpoint ?? throw new ArgumentNullException(nameof(endpoint));

            while (_boundEndpoint.Family == AddressFamily.Bound) {
                // Unwrap bound address
                _boundEndpoint = ((BoundSocketAddress)_boundEndpoint).LocalAddress;
            }

            return TaskEx.Completed;
        }

        /// <summary>
        /// Connect to a target on first of bound proxies, or use ping based dynamic lookup
        /// </summary>
        /// <param name="address"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public override async Task ConnectAsync(SocketAddress address, CancellationToken ct) {
            //
            // The address is a combination of proxy binding and remote address.  This is 
            // the case for all addresses returned by Dns resolution.  If no address was 
            // previously bound - use the one provided here.
            //
            Info.Address = address;
            if (Info.Address.Family == AddressFamily.Bound) {
                // Unwrap proxy and connect address.  If not bound, use local address to bind to.
                if (_boundEndpoint == null) {
                    _boundEndpoint = address;
                    // Unwrap bound address
                    while (_boundEndpoint.Family == AddressFamily.Bound) {
                        _boundEndpoint = ((BoundSocketAddress)_boundEndpoint).LocalAddress;
                    }
                }
                Info.Address = ((BoundSocketAddress)Info.Address).RemoteAddress;
            }

            //
            // Get the named host from the registry if it exists - there should only be one...
            // This is the host we shall connect to.  It can contain proxies to use as well.
            //
            var hostList = await Provider.NameService.LookupAsync(
                Info.Address.ToString(), NameRecordType.Host, ct).ConfigureAwait(false);
            Host = hostList.FirstOrDefault();
            if (Host == null) {
                // If there is no host in the registry, create a fake host record for this address
                Host = new NameRecord(NameRecordType.Host, Info.Address.ToString());
            }
            else {
                if (!Host.Name.Equals(Info.Address.ToString(), StringComparison.CurrentCultureIgnoreCase)) {
                    // Translate the address to host address
                    Info.Address = new ProxySocketAddress(Host.Name);
                }
            }

            // Commit all options that were set until now into info
            Info.Options.UnionWith(_optionCache.Select(p => Property<ulong>.Create(
                (uint)p.Key, p.Value)));

            //
            // Create tpl network for connect - prioritize input above errored attempts using
            // prioritized scheduling queue.
            //
            var input = DataflowMessage<INameRecord>.CreateAdapter(new ExecutionDataflowBlockOptions {
                NameFormat = "Adapt (Connect) Id={1}",
                CancellationToken = ct,
                MaxDegreeOfParallelism = DataflowBlockOptions.Unbounded,
                MaxMessagesPerTask = DataflowBlockOptions.Unbounded,
                SingleProducerConstrained = true,
                EnsureOrdered = false
            });

            var errors = new TransformBlock<DataflowMessage<INameRecord>, DataflowMessage<INameRecord>>(
            async (error) => {
                ProxyEventSource.Log.LinkFailure(this, error.Arg, Host);
                Host.RemoveReference(error.Arg.Address);
                await Provider.NameService.Update.SendAsync(Tuple.Create(Host, true), 
                    ct).ConfigureAwait(false);
                return error;
            },
            new ExecutionDataflowBlockOptions {
                NameFormat = "Error (Connect) Id={1}",
                CancellationToken = ct,
                MaxDegreeOfParallelism = DataflowBlockOptions.Unbounded,
                MaxMessagesPerTask = DataflowBlockOptions.Unbounded,
                EnsureOrdered = false
            });

            var query = Provider.NameService.Lookup(new ExecutionDataflowBlockOptions {
                NameFormat = "Lookup (Connect) Id={1}",
                CancellationToken = ct,
                MaxDegreeOfParallelism = 2, // 2 parallel lookups
                MaxMessagesPerTask = 1,
                EnsureOrdered = false
            });

            var pinger = CreatePingBlock(errors, Info.Address, new ExecutionDataflowBlockOptions {
                NameFormat = "Ping (Connect) Id={1}",
                CancellationToken = ct,
                MaxDegreeOfParallelism = DataflowBlockOptions.Unbounded,
                MaxMessagesPerTask = 1,
                EnsureOrdered = false
            });

            var linker = CreateLinkBlock(errors, new ExecutionDataflowBlockOptions {
                NameFormat = "Link (Connect) Id={1}",
                CancellationToken = ct,
                MaxDegreeOfParallelism = 1, // Ensure one link is created at a time.
                MaxMessagesPerTask = DataflowBlockOptions.Unbounded,
                SingleProducerConstrained = true,
                EnsureOrdered = false
            });

            var connection = new WriteOnceBlock<IProxyLink>(l => l, new DataflowBlockOptions {
                NameFormat = "Final (Connect) Id={1}",
                MaxMessagesPerTask = 1, // Auto complete when link is created
                EnsureOrdered = false
            });

            query.ConnectTo(input);
            input.ConnectTo(pinger);
            errors.ConnectTo(pinger);
            pinger.ConnectTo(linker);
            linker.ConnectTo(connection);

            // Now post proxies to the lookup block - start by sending all bound addresses
            var queries = new List<Task<bool>>();
            if (_boundEndpoint != null) {
                // Todo - consider removing ping and try link only here...

                if (_boundEndpoint.Family == AddressFamily.Collection) {
                    foreach (var item in ((SocketAddressCollection)_boundEndpoint).Addresses()) {
                        queries.Add(query.SendAsync(Provider.NameService.NewQuery(
                            item.ToString(), NameRecordType.Proxy), ct));
                    }
                }
                else {
                    queries.Add(query.SendAsync(Provider.NameService.NewQuery(
                        _boundEndpoint.ToString(), NameRecordType.Proxy), ct));
                }
            }
            else {
                if (Host.References.Any()) {
                    foreach (var item in Host.References) {
                        queries.Add(query.SendAsync(Provider.NameService.NewQuery(
                            item, NameRecordType.Proxy), ct));
                    }
                }
                else {
                    queries.Add(query.SendAsync(Provider.NameService.NewQuery(
                        Reference.All, NameRecordType.Proxy), ct));
                }
            }

            // TODO: Consider waiting later or cancelling wait when link received...
            await Task.WhenAll(queries.ToArray()).ConfigureAwait(false);

            // Wait until a connected link is received.  Then cancel the remainder of the pipeline.
            _link = await connection.ReceiveAsync(ct);
            connection.Complete();

            Host.AddReference(_link.Proxy.Address);
            var update = Provider.NameService.Update.SendAsync(Tuple.Create(Host, true), ct);
        }

        public override Task ListenAsync(int backlog, CancellationToken ct) {
            throw new NotSupportedException("Cannot call listen on client socket!");
        }

        /// <summary>
        /// Send socket option message to all streams
        /// </summary>
        /// <param name="option"></param>
        /// <param name="value"></param>
        /// <param name="ct"></param>
        public override async Task SetSocketOptionAsync(SocketOption option, ulong value,
            CancellationToken ct) {
            if (_link == null) {
                _optionCache[option] = value;
                return;
            }
            await _link.SetSocketOptionAsync(option, value, ct).ConfigureAwait(false);
        }

        /// <summary>
        /// Get socket option
        /// </summary>
        /// <param name="option"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public override async Task<ulong> GetSocketOptionAsync(SocketOption option,
            CancellationToken ct) {
            if (_link == null) {
                return _optionCache.ContainsKey(option) ? _optionCache[option] : 0;
            }
            return await _link.GetSocketOptionAsync(option, ct).ConfigureAwait(false);
        }

        /// <summary>
        /// Close underlying link
        /// </summary>
        /// <param name="ct"></param>
        /// <returns></returns>
        public override Task CloseAsync(CancellationToken ct) => _link.CloseAsync(ct);

        /// <summary>
        /// Send buffer
        /// </summary>
        /// <param name="buffer"></param>
        /// <param name="endpoint"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async override Task<int> SendAsync(ArraySegment<byte> buffer,
            SocketAddress endpoint, CancellationToken ct) {
            bool sent = await SendBlock.SendAsync(Message.Create(null, null, null, 
                DataMessage.Create(buffer, endpoint)), ct).ConfigureAwait(false);
            return buffer.Count;
        }

#if PERF
        private long _transferred;
        private Stopwatch _transferredw = Stopwatch.StartNew();
#endif

        /// <summary>
        /// Buffered receive
        /// </summary>
        /// <param name="buffer"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public override Task<ProxyAsyncResult> ReceiveAsync(
            ArraySegment<byte> buffer, CancellationToken ct) {

            if (buffer.Count == 0) {
                return Task.FromResult(new ProxyAsyncResult());
            }

            if (_lastData != null) {
                int copied = CopyBuffer(ref buffer);
                if (copied > 0) {
                    return Task.FromResult(new ProxyAsyncResult { Count = copied });
                }
            }
            return ReceiveInternalAsync(buffer, ct);
        }

        /// <summary>
        /// Receive using async state machine
        /// </summary>
        /// <param name="buffer"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        private async Task<ProxyAsyncResult> ReceiveInternalAsync(
            ArraySegment<byte> buffer, CancellationToken ct) {
            var result = new ProxyAsyncResult();
            while (true) {
                if (_lastData == null) {
                    try {
                        var message = await ReceiveBlock.ReceiveAsync(ct).ConfigureAwait(false);
                        if (message.TypeId != MessageContent.Data) {
                            message.Dispose();
                            continue;
                        }

                        _lastData = message.Content as DataMessage;
                        message.Content = null;
                        message.Dispose();
                        _offset = 0;
                    }
                    catch (Exception e) {
                        if (ReceiveBlock.Completion.IsFaulted) {
                            e = ReceiveBlock.Completion.Exception;
                        }
                        throw SocketException.Create("Failed to receive", e);
                    }

                    // Break on 0 sized packets
                    if (_lastData == null) {
                        break;
                    }
                    else if (_lastData.Payload.Length == 0) {
                        _lastData.Dispose();
                        _lastData = null;
                        break;
                    }
#if PERF
                    _transferred += _lastData.Payload.Length;
                    Console.CursorLeft = 0; Console.CursorTop = 0;
                    Console.WriteLine(
                        $"{ _transferred / _transferredw.ElapsedMilliseconds} kB/sec");
#endif
                }
                result.Count = CopyBuffer(ref buffer);
                if (result.Count > 0) {
                    break;
                }
            }
            return result;
        }

        /// <summary>
        /// Copies from the last buffer
        /// </summary>
        /// <param name="buffer"></param>
        /// <returns></returns>
        private int CopyBuffer(ref ArraySegment<Byte> buffer) {
            // How much to copy from the last data buffer.
            int toCopy = Math.Min(buffer.Count, _lastData.Payload.Length - _offset);
            Buffer.BlockCopy(_lastData.Payload, _offset,
                buffer.Array, buffer.Offset, toCopy);
            _offset += toCopy;

            if (_offset >= _lastData.Payload.Length) {
                // Last data exhausted, release
                _lastData.Dispose();
                _lastData = null;
                _offset = 0;
            }
            return toCopy;
        }


        public override void Dispose() {
            if (_lastData != null) {
                _lastData.Dispose();
                _lastData = null;
            }

            if (_boundEndpoint != null) {
                _boundEndpoint.Dispose();
                _boundEndpoint = null;
            }
            // if (_link != null) {
            //     _link.Dispose();
            // }
        }

        private IProxyLink _link;
        private SocketAddress _boundEndpoint;
        private DataMessage _lastData;
        private int _offset;
#if PERF
        private long _transferred;
        private Stopwatch _transferredw = Stopwatch.StartNew();
#endif
    }
}