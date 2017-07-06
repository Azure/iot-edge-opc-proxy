// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Threading;
    using System.Threading.Tasks;

    /// <summary>
    /// Services providing remote procedure call functionality for proxy
    /// command control implement this interface.
    /// </summary>
    public interface IRemotingService {

        /// <summary>
        /// Send message and receive response in one call using timeout
        /// </summary>
        /// <param name="proxy"></param>
        /// <param name="message"></param>
        /// <param name="timeout"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        Task<Message> CallAsync(INameRecord proxy, Message message, TimeSpan timeout,
            CancellationToken ct);
    }
}
