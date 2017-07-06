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
    /// Virtual broadcast socket connected to a set of proxies that provide browse services.
    /// </summary>
    internal class BrowseSocket : BroadcastSocket {
        private static readonly ushort _browsePort = 1; // Keep in sync with native

        /// <summary>
        /// Internal constructor to create browse socket. Used in browse client.
        /// </summary>
        /// <param name="provider"></param>
        /// <param name="codec">Codec to use on wire</param>
        internal BrowseSocket(IProvider provider, CodecId codec = CodecId.Mpack) :
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
                    Address = new ProxySocketAddress("", _browsePort, (ushort)codec),
                    Protocol = ProtocolType.Unspecified,
                    Family = AddressFamily.Unspecified
                }, 
                provider) {

            _requests = new TransformBlock<BrowseRequest, Message>(async (request) => {
                if (_open.IsCancellationRequested) {
                    throw new SocketException(SocketError.Closed);
                }
                var buffer = new MemoryStream();
                await request.EncodeAsync(buffer, codec, _open.Token).ConfigureAwait(false);
                return Message.Create(null, null, null, DataMessage.Create(buffer.ToArray()));
            },
            new ExecutionDataflowBlockOptions {
                NameFormat = "Encode (in BrowseSocket) Id={1}",
                EnsureOrdered = true,
                BoundedCapacity = 3
            });

            _requests.ConnectTo(SendBlock);

            _responses = new TransformBlock<Message, BrowseResponse>(async (message) => {
                if (_open.IsCancellationRequested) {
                    throw new SocketException(SocketError.Closed);
                }
                ThrowIfFailed(message);
                if (message.TypeId != MessageContent.Data) {
                    throw new SocketException("No data message");
                }
                var data = message.Content as DataMessage;
                if (data == null) {
                    throw new SocketException("Bad data");
                }
                var stream = new MemoryStream(data.Payload);

                var response = await Serializable.DecodeAsync<BrowseResponse>(stream, codec,
                    _open.Token).ConfigureAwait(false);
                response.Interface = message.Proxy.ToSocketAddress();
                return response;
            }, 
            new ExecutionDataflowBlockOptions {
                NameFormat = "Decode (in BrowseSocket) Id={1}",
                EnsureOrdered = true,
                BoundedCapacity = 3
            });

            ReceiveBlock.ConnectTo(_responses);
        }

        /// <summary>
        /// Connects to browse server on a proxy
        /// </summary>
        /// <param name="endpoint">proxy endpoint or null for all proxies</param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public override Task ConnectAsync(SocketAddress endpoint, CancellationToken ct) =>
            LinkAsync(endpoint, ct);


        // Init: Attach null block to receives.
        // Create browser object.
        //
        // Browser attaches buffer block to responses, filtering on unique id
        // Browser sends unique request to requests block
        // Next, next next, using receiveAsync
        //
        // When done, dispose browser.
        // Browser detaches from receive block on dispose
        
        /// <summary>
        /// Start browsing specified item
        /// </summary>
        /// <param name="item"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async Task BrowseBeginAsync(ProxySocketAddress item, int type, CancellationToken ct) {
            if (_open.IsCancellationRequested) {
                throw new SocketException(SocketError.Closed);
            }
            var request = new BrowseRequest {
                Item = item,
                Handle = _id,
                Type = type
            };
            await _requests.SendAsync(request, ct);
        }

        /// <summary>
        /// Read responses from socket
        /// </summary>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async Task<BrowseResponse> BrowseNextAsync(CancellationToken ct) {
            try {
                return await _responses.ReceiveAsync(ct);
            }
            catch (InvalidOperationException) {
                // Pipeline completed
                return null;
            }
            catch (OperationCanceledException) {
                throw;
            }
            catch (Exception e) {
                throw SocketException.Create("Browse next error", e);
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
        private IPropagatorBlock<BrowseRequest, Message> _requests;
        private IPropagatorBlock<Message, BrowseResponse> _responses;
    }
}
