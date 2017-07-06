// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
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
        /// Called when error message is received over stream
        /// </summary>
        /// <param name="message"></param>
        /// <returns></returns>
        protected override bool OnReceiveError(Message message) {
            OnRemoteClose(message);
            return true; 
        }

        /// <summary>
        /// Called when we determine that remote side closed
        /// </summary>
        protected override void OnRemoteClose(Message message) {
            // Reconnect
            Detach();
            ((BroadcastSocket)_socket).Reconnect(this);
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