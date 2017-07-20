// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Linq;
    using System.Threading.Tasks.Dataflow;

    /// <summary>
    /// Proxy link represents a 1:1 link with a remote socket. Proxy
    /// Link is created via LinkRequest, and OpenRequest Handshake.
    /// This is a specialization for bound broadcast sockets, in that
    /// the broadcast link reconnects automatically until closed.
    /// </summary>
    internal class BroadcastLink : ProxyLink {

        /// <summary>
        /// Constructor for broadcast link object
        /// </summary>
        /// <param name="socket"></param>
        /// <param name="proxy"></param>
        /// <param name="remoteId"></param>
        /// <param name="localAddress"></param>
        /// <param name="peerAddress"></param>
        internal BroadcastLink(BroadcastSocket socket, INameRecord proxy,
            Reference remoteId, SocketAddress localAddress, SocketAddress peerAddress) :
            base(socket, proxy, remoteId, localAddress, peerAddress) {
        }

        /// <summary>
        /// Create send block
        /// </summary>
        /// <param name="maxSize"></param>
        protected override void CreateSendBlock(int maxSize) {
            if (maxSize == 0) {
                base.CreateSendBlock(maxSize);
                return;
            }
            _send = new TransformBlock<Message, Message>((message) => {
                if (maxSize == 0 || message.TypeId != MessageContent.Data ||
                   ((DataMessage)message.Content).Payload.Length <= maxSize) {
                    return message;
                }
                throw new ProxyException(
                    $"Cannot send more than {maxSize} bytes as single message");
            },
            new ExecutionDataflowBlockOptions {
                NameFormat = "Send (in Link) Id={1}",
                EnsureOrdered = true,
                MaxMessagesPerTask = DataflowBlockOptions.Unbounded,
                SingleProducerConstrained = false,
                BoundedCapacity = 3
            });
        }

        /// <summary>
        /// Create receive block
        /// </summary>
        protected override void CreateReceiveBlock() {
            _receive = new TransformManyBlock<Message, Message>((message) => {
                if (message.TypeId == MessageContent.Data) {
                    if (message.Error == (int)SocketError.Duplicate) {
                        // Drop message
                        return Enumerable.Empty<Message>();
                    }

                    if (message.Error == (int)SocketError.Success) {
                        return message.AsEnumerable();
                    }
                }

                if (message.TypeId == MessageContent.Close ||
                    message.Error != (int)SocketError.Success) {

                    if (message.Error != (int)SocketError.Success) {
                        // Todo: log error
                    }
                    
                    // Remote side closed or error occurred - reconnect
                    Detach();
                    ((BroadcastSocket)_socket).Reconnect(this);
                }
                return Enumerable.Empty<Message>();
            },
            new ExecutionDataflowBlockOptions {
                NameFormat = "Receive (in Link) Id={1}",
                EnsureOrdered = true,
                MaxMessagesPerTask = DataflowBlockOptions.Unbounded,
                SingleProducerConstrained = true,
                BoundedCapacity = 3
            });
        }

        /// <summary>
        /// Attach send and receive blocks
        /// </summary>
        /// <param name="send"></param>
        /// <param name="receive"></param>
        public virtual void Attach(
            IPropagatorBlock<Message, Message> send,
            IPropagatorBlock<Message, Message> receive) {
            _socketSend = send.ConnectTo(SendBlock);
            _socketReceive = ReceiveBlock.ConnectTo(receive);
        }

        /// <summary>
        /// Detach the connected socket
        /// </summary>
        public virtual void Detach() {
            _socketReceive.Dispose();
            _socketSend.Dispose();

            ReceiveBlock.Complete();
            SendBlock.Complete();
        }

        protected IDisposable _socketSend;
        protected IDisposable _socketReceive;
    }
}
