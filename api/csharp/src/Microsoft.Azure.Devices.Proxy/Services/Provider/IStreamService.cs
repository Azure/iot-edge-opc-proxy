// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System.Threading.Tasks;

    /// <summary>
    /// Stream service provides stream endpoints to remote side.
    /// </summary>
    public interface IStreamService {

        /// <summary>
        /// Creates a new endpoint and returns a connection string to be brokered to 
        /// remote side to open the remote endpoint. The returned connection object
        /// is then opened attaching a message stream interface.
        /// </summary>
        /// <param name="streamId">Local reference id of the stream</param>
        /// <param name="remoteId">Remote reference of link</param>
        /// <param name="proxy">The proxy server</param>
        /// <param name="encoding">The encoding to use for messages on the stream</param>
        /// <returns>
        /// A connection string to be brokered to remote side to open the remote end 
        /// of the stream.  
        /// </returns>
        Task<IConnection> CreateConnectionAsync(Reference streamId, 
            Reference remoteId, INameRecord proxy, CodecId encoding);
    }
}