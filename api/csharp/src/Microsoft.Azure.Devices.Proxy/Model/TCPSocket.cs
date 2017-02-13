// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Model {
    using System;
    using System.Linq;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Collections.Generic;

    /// <summary>
    /// Concrete tcp proxy socket implementation
    /// </summary>
    internal class TCPSocket : ProxySocket {

        private IEnumerable<INameRecord> _bindList;

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
        /// Select the proxy to use for listen or connect, caches the proxy list for connect or listen
        /// </summary>
        /// <param name="endpoint"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public override async Task BindAsync(SocketAddress endpoint, CancellationToken ct) {
            // Proxy selected, look up records
            var bindList = await LookupRecordsForAddressAsync(endpoint, ct).ConfigureAwait(false);
            if (!bindList.Any()) {
                throw new SocketException(SocketError.No_address);
            }
            _bindList = bindList;
        }

        /// <summary>
        /// Connect to a target on first of bound proxies, or use ping based dynamic lookup
        /// </summary>
        /// <param name="address"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public override async Task ConnectAsync(SocketAddress address, CancellationToken ct) {
            bool connected = false;

            // If bind list present, use it
            if (_bindList != null) {
                foreach (var proxy in _bindList) {
                    bool success = await LinkAsync(proxy, address, ct).ConfigureAwait(false);
                    if (!connected && success)
                        connected = true;
                }
                if (!connected) {
                    throw new SocketException(SocketError.No_host);
                }
            }
            else {
                // Otherwise find a proxy that knows about the address
                address = await TranslateAddressAsync(address, ct).ConfigureAwait(false);

                await PingAsync(address, async (response, proxy, ct2) => {
                    if (connected) {
                        return Disposition.Done;
                    }
                    if (response != null && response.Error == (int)SocketError.Success) {
                        try {
                            // Foreach returned item, try to link now
                            connected = await LinkAsync(proxy, address, ct2).ConfigureAwait(false);
                            // Once connected, create stream, and cancel the rest
                            if (connected)
                                _stream = new SequentialStream(this);
                        }
                        catch (Exception) {
                            return Disposition.Retry;
                        }
                    }
                    return connected ? Disposition.Done : Disposition.Continue; 
                }, (ex) => {
                    if (!connected) {
                        throw new SocketException(
                            "Could not link socket on proxy", ex, SocketError.No_host);
                    }
                }, ct).ConfigureAwait(false);
                ct.ThrowIfCancellationRequested();
            }
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
            if (!listening)
                throw new SocketException(SocketError.No_host);

            _stream = new ListenStream(this);
        }
    }
}