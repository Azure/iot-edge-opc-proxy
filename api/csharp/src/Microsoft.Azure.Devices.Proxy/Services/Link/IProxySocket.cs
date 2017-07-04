// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Threading;
    using System.Threading.Tasks;

    /// <summary>
    /// Proxy socket represents a virtual socket that once activated can 
    /// be used to communicate to one or more proxy endpoint instances 
    /// (e.g. to support udp multicast send receive across proxies).
    /// </summary>
    public interface IProxySocket : IProxyOptions, IProxyStream, IDisposable {

        /// <summary>
        /// Local reference id of activated remote socket link
        /// </summary>
        Reference Id { get; }

        /// <summary>
        /// Information for this socket, exchanged with proxy server
        /// </summary>
        SocketInfo Info { get; }

        /// <summary>
        /// Connects a socket to an endpoint
        /// </summary>
        /// <param name="endpoint"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        Task ConnectAsync(SocketAddress address, CancellationToken ct);

        /// <summary>
        /// Creates a socket bound to proxy
        /// </summary>
        /// <param name="endpoint"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        Task BindAsync(SocketAddress endpoint, CancellationToken ct);

        /// <summary>
        /// Creates a listening link
        /// </summary>
        /// <param name="backlog"></param>
        /// <param name="none"></param>
        /// <returns></returns>
        Task ListenAsync(int backlog, CancellationToken ct);

        /// <summary>
        /// Close (close)
        /// </summary>
        /// <param name="connectTimeout"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        Task CloseAsync(CancellationToken ct);
    }
}