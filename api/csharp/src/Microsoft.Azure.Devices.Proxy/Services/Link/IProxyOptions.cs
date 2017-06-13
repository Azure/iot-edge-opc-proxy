// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System.Threading;
    using System.Threading.Tasks;

    /// <summary>
    /// Proxy link interface represents a virtual link to an activated socket 
    /// instance on a remote proxy.  It can be made up of many endpoints, which
    /// are themselves links.
    /// </summary>
    public interface IProxyOptions {

        /// <summary>
        /// Proxy the socket is bound on.  
        /// </summary>
        SocketAddress ProxyAddress { get; }

        /// <summary>
        /// Local bound address on the proxy.
        /// </summary>
        SocketAddress LocalAddress { get; }

        /// <summary>
        /// Peer address the socket is connected to on the proxy side. 
        /// </summary>
        SocketAddress PeerAddress { get; }

        /// <summary>
        /// Sets remote socket option (setopt)
        /// </summary>
        /// <param name="option"></param>
        /// <param name="value"></param>
        /// <param name="connectTimeout"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        Task SetSocketOptionAsync(SocketOption option, ulong value,
            CancellationToken ct);

        /// <summary>
        /// Gets remote socket option (getopt)
        /// </summary>
        /// <param name="option"></param>
        /// <param name="connectTimeout"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        Task<ulong> GetSocketOptionAsync(SocketOption option,
            CancellationToken ct);
    }
}