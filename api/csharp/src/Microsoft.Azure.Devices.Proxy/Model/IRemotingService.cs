// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Model {
    using System;
    using System.Threading;
    using System.Threading.Tasks;

    /// <summary>
    /// Services providing remote procedure call functionality for proxy
    /// command control implement this interface.
    /// </summary>
    public interface IRemotingService {
        /// <summary>
        /// Send message and receive response in one call
        /// </summary>
        /// <param name="message"></param>
        /// <param name="timeout"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        Task<Message> CallAsync(INameRecord proxy, Message message, 
            TimeSpan timeout, CancellationToken ct);

        /// <summary>
        /// Broad cast to all proxies, calls delegate for each response
        /// </summary>
        /// <param name="message"></param>
        /// <param name="handler"></param>
        /// <param name="timeout"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        Task BroadcastAsync(Message message, Func<Message, INameRecord, Task> handler, 
            TimeSpan timeout, CancellationToken ct);
    }
}
