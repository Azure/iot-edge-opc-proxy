// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Linq;
    using System.Collections.Generic;
    using System.Collections.Concurrent;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Threading.Tasks.Dataflow;

    /// <summary>
    /// Proxy link represents a 1:1 link with a remote socket. Proxy
    /// Link is created via LinkRequest, and OpenRequest Handshake.
    /// </summary>
    internal class BroadcastLink : ProxyLink {

        /// <summary>
        /// Constructor for proxy link object
        /// </summary>
        /// <param name="socket"></param>
        /// <param name="proxy"></param>
        /// <param name="remoteId"></param>
        /// <param name="localAddress"></param>
        /// <param name="peerAddress"></param>
        internal BroadcastLink(ProxySocket socket, INameRecord proxy, Reference remoteId, 
            SocketAddress localAddress, SocketAddress peerAddress) : 
            base(socket, proxy, remoteId, localAddress, peerAddress) {
        }

        /// <summary>
        /// Called when error message is received over stream
        /// </summary>
        /// <param name="message"></param>
        /// <returns></returns>
        protected override bool OnReceiveError(Message message) => 
            true; // Continue -- TODO:

        /// <summary>
        /// Called when we determine that remote side closed
        /// </summary>
        protected override void OnRemoteClose(Message message) {
            // TODO: - link should reconnect...
        }
    }
}