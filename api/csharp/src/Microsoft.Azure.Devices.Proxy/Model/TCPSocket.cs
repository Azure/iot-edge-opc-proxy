// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Model {
    using System;
    using System.Linq;
    using System.Collections.Generic;
    using System.Threading;
    using System.Threading.Tasks;

    /// <summary>
    /// Concrete tcp proxy socket implementation
    /// </summary>
    internal class TCPSocket : ProxySocket {

        /// <summary>
        /// Host record socket is connected to
        /// </summary>
        public INameRecord Host { get; private set; }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="info"></param>
        /// <param name="provider"></param>
        internal TCPSocket(SocketInfo info, IProvider provider) :
            base(info, provider) {
            if (info.Type != SocketType.Stream)
                throw new ArgumentException("Tcp only supports streams");
        }

        /// <summary>
        /// Start listening
        /// </summary>
        /// <param name="backlog"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public override async Task ListenAsync(int backlog, CancellationToken ct) {
            bool listening = false;

            if (_bindList != null) {
                listening = await LinkAllAsync(_bindList, null, ct).ConfigureAwait(false);
            }
            else {
                // Not bound, must be bound
                throw new SocketException(SocketError.NotSupported);
            }
            // Check to see if listen completed
            if (!listening) {
                throw new SocketException(SocketError.NoHost);
            }
            _stream = new ListenStream(this);
        }

        /// <summary>
        /// Connect to a target on first of bound proxies, or use ping based dynamic lookup
        /// </summary>
        /// <param name="address"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public override async Task ConnectAsync(SocketAddress address, CancellationToken ct) {

            if (address.Family == AddressFamily.Bound) {
                // Unwrap proxy and connect address.  If not bound, use local address to bind to.
                if (_bindList == null) {
                    await BindAsync(((BoundSocketAddress)address).LocalAddress, ct);
                }
                address = ((BoundSocketAddress) address).RemoteAddress;
            }

            // Get the named host from the registry if it exists - there should only be one...
            Host = null;
            var hostList = await Provider.NameService.LookupAsync(
                address.ToString(), RecordType.Host, ct).ConfigureAwait(false);
            foreach (var host in hostList) {
                Host = host;
                break;
            }

            // If there is no host in the registry, create a fake host record for this address
            if (Host == null) {
                Host = new Record(RecordType.Host, address.ToString());
            }
            else if (!Host.Name.Equals(address.ToString(), StringComparison.CurrentCultureIgnoreCase)) {
                // Translate the address to host address
                address = new ProxySocketAddress(Host.Name);
            }

            // Set bind list before connecting if it is not already set during Bind
            bool autoBind = _bindList == null;
            if (autoBind) {
                var bindList = new HashSet<INameRecord>();
                foreach (var proxyRef in Host.References) {
                    var results = await Provider.NameService.LookupAsync(
                        proxyRef, RecordType.Proxy, ct).ConfigureAwait(false);
                    bindList.AddRange(results);
                }
                _bindList = bindList.Any() ? bindList : null;
            }

            bool connected = false;
            if (_bindList != null) {
                // Try to connect through each proxy in the bind list
                foreach (var proxy in _bindList) {
                    connected = await ConnectAsync(address, proxy, ct).ConfigureAwait(false);
                    if (connected) {
                        break;
                    }
                }
                // If there was a bind list and we could not connect through it, throw
                if (!connected && !autoBind) {
                    throw new SocketException(SocketError.NoHost);
                }
                _bindList = null;
            }

            if (!connected) {
                await PingAsync(address, async (response, proxy, ct2) => {
                    if (connected) {
                        return Disposition.Done;
                    }
                    if (response.Error == (int)SocketError.Success) {
                        try {
                            connected = await ConnectAsync(address, proxy, ct).ConfigureAwait(false); 
                        }
                        catch (Exception) {
                            return Disposition.Retry;
                        }
                    }
                    return connected ? Disposition.Done : Disposition.Continue; 
                }, (ex) => {
                    if (!connected) {
                        throw new SocketException(
                            "Could not link socket on proxy", ex, SocketError.NoHost);
                    }
                }, ct).ConfigureAwait(false);

                ct.ThrowIfCancellationRequested();
            }

            await Provider.NameService.AddOrUpdateAsync(Host, ct).ConfigureAwait(false);
        }

        /// <summary>
        /// Connect to address through a proxy
        /// </summary>
        /// <param name="address"></param>
        /// <param name="proxy"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        private async Task<bool> ConnectAsync(SocketAddress address, INameRecord proxy, 
            CancellationToken ct) {

            bool connected = await LinkAsync(proxy, address, ct).ConfigureAwait(false);
            if (connected) {
                _stream = new SequentialStream(this);
                Host.AddReference(proxy.Address);
            }
            else {
                Host.RemoveReference(proxy.Address);
            }
            return connected;
        }

        /// <summary>
        /// Select a bind list for a host address if one exists
        /// </summary>
        /// <param name="endpoint"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        private async Task BindAsync(CancellationToken ct) {
            var bindList = new HashSet<INameRecord>();
            foreach (var proxyRef in Host.References) {
                var results = await Provider.NameService.LookupAsync(
                    proxyRef, RecordType.Proxy, ct).ConfigureAwait(false);
                bindList.AddRange(bindList);
            }
            _bindList = bindList.Any() ? bindList : null;
        }
    }
}