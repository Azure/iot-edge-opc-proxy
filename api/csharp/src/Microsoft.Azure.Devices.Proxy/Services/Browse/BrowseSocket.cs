// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Linq;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Threading.Tasks.Dataflow;

    /// <summary>
    /// Virtual socket connected to a set of proxies that provide browse services.
    /// support.
    /// </summary>
    internal class BrowseSocket : BroadcastSocket {
        private static readonly ushort _browsePort = 1; // Keep in sync with native

        /// <summary>
        /// Internal constructor to create browse socket.
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
        /// Connects to browse server on a proxy
        /// </summary>
        /// <param name="endpoint">proxy endpoint or null for all proxies</param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public override Task ConnectAsync(SocketAddress endpoint, CancellationToken ct) {
            // Complete socket info by setting browse server port
            Info.Address = new ProxySocketAddress("", _browsePort, (ushort)_codec);
            return LinkAsync(endpoint, ct);
        }

     //   public IPropagatorBlock<BrowseRequest, BrowseResponse> BrowseBlock(ExecutionDataflowBlockOptions option) {
     //
     //       var responses = new TransformManyBlock<Message, BrowseResponse>(async (message) => {
     //           if (_open.IsCancellationRequested) {
     //               throw new SocketException(SocketError.Closed);
     //           }
     //           ThrowIfFailed(message);
     //           if (message.TypeId != MessageContent.Data) {
     //               throw new SocketException("No data message");
     //           }
     //           var data = message.Content as DataMessage;
     //           if (data == null) {
     //               throw new SocketException("Bad data");
     //           }
     //           var stream = new MemoryStream(data.Payload);
     //
     //           var response = await BrowseResponse.DecodeAsync(stream, _codec, ct);
     //           response.Interface = message.Proxy.ToSocketAddress();
     //           return response;
     //       }, option);
     //
     //       var requests = new ActionBlock<BrowseRequest>(async (request) => {
     //
     //           if (_open.IsCancellationRequested) {
     //               throw new SocketException(SocketError.Closed);
     //           }
     //           var buffer = new MemoryStream();
     //           await request.EncodeAsync(buffer, _codec, ct);
     //           await SendBlock.SendAsync(new Message(null, null, null,
     //               new DataMessage(buffer.ToArray())), ct).ConfigureAwait(false);
     //
     //       }, option);
     //
     //       return DataflowBlockEx.Encapsulate(requests, responses);
     //   }

        /// <summary>
        /// Start browsing specified item
        /// </summary>
        /// <param name="item"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async Task BrowseBeginAsync(ProxySocketAddress item, CancellationToken ct) {
            if (_open.IsCancellationRequested) {
                throw new SocketException(SocketError.Closed);
            }
            var request = new BrowseRequest {
                Item = item,
                Handle = _id,
                Type = _type
            };
            var buffer = new MemoryStream();
            await request.EncodeAsync(buffer, _codec, ct);
            await SendBlock.SendAsync(Message.Create(null, null, null, 
                DataMessage.Create(buffer.ToArray())), ct).ConfigureAwait(false);
        }

        /// <summary>
        /// Read responses from socket
        /// </summary>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async Task<BrowseResponse> BrowseNextAsync(CancellationToken ct) {
            if (_open.IsCancellationRequested) {
                throw new SocketException(SocketError.Closed);
            }
            using (var message = await ReceiveBlock.ReceiveAsync(ct).ConfigureAwait(false)) {
                ThrowIfFailed(message);
                if (message.TypeId != MessageContent.Data) {
                    throw new SocketException("No data message");
                }
                var data = message.Content as DataMessage;
                if (data == null) {
                    throw new SocketException("Bad data");
                }

                var stream = new MemoryStream(data.Payload);
                var response = await Serializable.DecodeAsync<BrowseResponse>(stream, _codec, ct);
                response.Interface = message.Proxy.ToSocketAddress();
                return response;
            }
        }

        public override Task BindAsync(SocketAddress address, CancellationToken ct) {
            throw new NotSupportedException("Cannot call bind on browse socket");
        }

        public override Task ListenAsync(int backlog, 
            CancellationToken ct) {
            throw new NotSupportedException("Cannot call listen on browse socket");
        }

        public override Task<int> SendAsync(ArraySegment<Byte> buffer, SocketAddress endpoint,
            CancellationToken ct) {
            throw new NotSupportedException("Cannot call send on browse socket");
        }

        public override Task<ProxyAsyncResult> ReceiveAsync(ArraySegment<byte> buffer, 
            CancellationToken ct) {
            throw new NotSupportedException("Cannot call receive on browse socket");
        }

        private readonly Reference _id = new Reference();
        private readonly int _type;
        private readonly CodecId _codec;
    }
}
