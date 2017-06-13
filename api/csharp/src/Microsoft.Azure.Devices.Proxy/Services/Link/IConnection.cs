//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System.Threading;
    using System.Threading.Tasks;

    /// <summary>
    /// Connection interface
    /// </summary>
    public interface IConnection {

        /// <summary>
        /// Connection string 
        /// </summary>
        ConnectionString ConnectionString { get; }

        /// <summary>
        /// Whether this stream is polled.
        /// </summary>
        bool IsPolled { get; }

        /// <summary>
        /// Accept a message stream from remote side
        /// </summary>
        /// <param name="ct">To cancel accept</param>
        /// <returns></returns>
        Task<IMessageStream> OpenAsync(CancellationToken ct);

        /// <summary>
        /// Close connection
        /// </summary>
        /// <returns></returns>
        Task CloseAsync();
    }
}