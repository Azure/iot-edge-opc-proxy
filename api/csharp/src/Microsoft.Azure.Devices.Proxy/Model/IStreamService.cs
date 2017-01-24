// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Model {
    using System;
    using System.Threading;
    using System.Threading.Tasks;

    /// <summary>
    /// Stream service provides stream endpoints to remote side.
    /// </summary>
    public interface IStreamService {

        /// <summary>
        /// Start opening a new stream.  Returns a connection string to be brokered to 
        /// remote side to open the remote end of the stream.  
        /// </summary>
        /// <param name="streamId">Reference of the caller</param>
        /// <returns>
        /// A connection string to be brokered to remote side to open the remote end 
        /// of the stream.  
        /// </returns>
        Task<IConnection> CreateConnectionAsync(Reference streamId);
    }
}