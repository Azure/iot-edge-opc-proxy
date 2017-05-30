// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Model {
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Linq;
    using System.Threading;
    using System.Threading.Tasks;

    /// <summary>
    /// Virtual socket connected to a set of proxies that provide browse service
    /// support.
    /// </summary>
    internal class BrowseSocket : ProxySocket {
        private static readonly ushort _browsePort = 1; // Keep in sync with native

        /// <summary>
        /// Private constructor
        /// </summary>
        /// <param name="provider"></param>
        /// <param name="codec">Codec to use on wire</param>
        internal BrowseSocket(IProvider provider, int type, CodecId codec = CodecId.Json) :
            base(
                //
                // Set up socket as internal datagram socket and let proxy  
                // decide the protocol and address family part of the internal
                // connection.
                //
                new SocketInfo {
                    Flags = (uint)SocketFlags.Internal,
                    Type = SocketType.Dgram,
                    Timeout = 90000,
                    Protocol = ProtocolType.Unspecified,
                    Family = AddressFamily.Unspecified
                }, 
                provider) {

            _type = type;
            _codec = codec;
        }

        /// <summary>
        /// Connects to browse server on proxy
        /// </summary>
        /// <param name="endpoint">proxy endpoint or null for all proxies</param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public override async Task ConnectAsync(SocketAddress endpoint, CancellationToken ct) {
            IEnumerable<SocketAddress> addresses;
            if (endpoint != null && endpoint.Family == AddressFamily.Bound) {
                // Unwrap bound address
                endpoint = ((BoundSocketAddress)endpoint).LocalAddress;
            }

            if (endpoint == null || endpoint is NullSocketAddress) {
                _bindList = await Provider.NameService.LookupAsync(
                    Reference.All, RecordType.Proxy, ct).ConfigureAwait(false);
            }
            else {
                var bindList = new HashSet<INameRecord>();
                if (endpoint.Family == AddressFamily.Collection) {
                    // Unwrap collection
                    addresses = ((SocketAddressCollection)endpoint).Addresses();
                }
                else {
                    addresses = endpoint.AsEnumerable();
                }
                foreach (var address in addresses) {
                    var result = await Provider.NameService.LookupAsync(
                        address.ToString(), RecordType.Proxy, ct).ConfigureAwait(false);
                    bindList.AddRange(result);
                }
                _bindList = bindList;
            }
            if (!_bindList.Any()) {
                throw new SocketException(SocketError.NoAddress);
            }

            // Connect to internal socket
            _connected = await LinkAllAsync(_bindList,
                new ProxySocketAddress("", _browsePort, (ushort)_codec), ct);
            if (!_connected) {
                throw new SocketException(
                    "Could not link browse socket on proxy", SocketError.NoHost);
            }
        }

        /// <summary>
        /// Start browsing specified item
        /// </summary>
        /// <param name="item"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async Task BrowseBeginAsync(ProxySocketAddress item, CancellationToken ct) {
            if (!_connected) {
                throw new SocketException(SocketError.Closed);
            }
            var request = new BrowseRequest {
                Item = item,
                Handle = _id,
                Type = _type
            };
            var buffer = new MemoryStream();
            await request.EncodeAsync(buffer, _codec, ct);
            await SendAsync(new Message(null, null, null,
                new DataMessage { Payload = buffer.ToArray() }), ct).ConfigureAwait(false);
        }

        /// <summary>
        /// Read responses from socket
        /// </summary>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async Task<BrowseResponse> BrowseNextAsync(CancellationToken ct) {
            if (!_connected) {
                throw new SocketException(SocketError.Closed);
            }
            Message message;
            while (!ReceiveQueue.TryDequeue(out message)) {
                await ReceiveAsync(ct).ConfigureAwait(false);
            }
            ProxySocket.ThrowIfFailed(message);
            if (message.Error != (int)SocketError.Success) {
                throw new SocketException((SocketError)message.Error);
            }
            if (message.TypeId != MessageContent.Data) {
                throw new SocketException("No data message");
            }
            var data = message.Content as DataMessage;
            if (data == null) {
                throw new SocketException("Bad data");
            }
            var stream = new MemoryStream(data.Payload);

            var response = await BrowseResponse.DecodeAsync(stream, _codec, ct);
            response.Interface = message.Proxy.ToSocketAddress();
            return response;
        }

        private bool _connected;
        private readonly Reference _id = new Reference();
        private readonly int _type;
        private readonly CodecId _codec;
    }
}
