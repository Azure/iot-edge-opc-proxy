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
        /// <param name="timeout"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public override async Task BindAsync(SocketAddress endpoint, TimeSpan timeout, CancellationToken ct) {
            // Proxy selected, look up records
            var bindList = await LookupRecordsForAddressAsync(endpoint, ct);
            if (!bindList.Any()) {
                throw new SocketException(SocketError.No_address);
            }
            _bindList = bindList;
        }

        /// <summary>
        /// Connect to a target on first of bound proxies, or use ping based dynamic lookup
        /// </summary>
        /// <param name="address"></param>
        /// <param name="timeout"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public override async Task ConnectAsync(SocketAddress address, TimeSpan timeout, CancellationToken ct) {
            bool connected = false;

            // If bind list present, use it
            if (_bindList != null) {
                connected = await LinkAsync(_bindList, address, timeout, ct);
            }
            else {
                // Otherwise find a proxy that knows about the address
                address = await TranslateAddressAsync(address, ct);

                await PingAsync(address, async c => {

                    connected = await LinkAsync(c.GetConsumingEnumerable(ct), address, timeout, ct);

                    // Check to see if connect completed
                    if (!connected)
                        throw new SocketException(SocketError.No_host);

                    _stream = new SequentialStream(this);
                }, 
                timeout, ct);
            }
        }

        /// <summary>
        /// Start listening
        /// </summary>
        /// <param name="backlog"></param>
        /// <param name="timeout"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public override async Task ListenAsync(int backlog, TimeSpan timeout, CancellationToken ct) {
            bool listening = false;

            if (_bindList != null) {
                listening = await LinkAllAsync(_bindList, null, timeout, ct);
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