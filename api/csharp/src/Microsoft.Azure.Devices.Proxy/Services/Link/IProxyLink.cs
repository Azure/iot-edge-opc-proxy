// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {

    using System;
    using System.Threading;
    using System.Threading.Tasks;

    /// <summary>
    /// Proxy link interface represents a virtual link to an activated socket 
    /// instance on a remote proxy.  It can be made up of many endpoints, which
    /// are themselves links.
    /// </summary>
    public interface IProxyLink : IProxyOptions, IMessageStream {

        /// <summary>
        /// Proxy target record
        /// </summary>
        INameRecord Proxy {
            get;
        }

        /// <summary>
        /// Remote id of this link
        /// </summary>
        Reference RemoteId {
            get;
        }

        /// <summary>
        /// Begin connect sequence, returns connection string to
        /// be brokered to remote side.
        /// </summary>
        /// <param name="ct"></param>
        /// <returns></returns>
        Task<OpenRequest> BeginOpenAsync(CancellationToken ct);

        /// <summary>
        /// Waits for connect from remote side to complete.
        /// </summary>
        /// <param name="ct">Cancels the current wait</param>
        /// <returns>True if completed</returns>
        Task<bool> TryCompleteOpenAsync(CancellationToken ct);

        /// <summary>
        /// Close the stream asynchronously
        /// </summary>
        /// <returns></returns>
        Task CloseAsync(CancellationToken ct);
    }
}