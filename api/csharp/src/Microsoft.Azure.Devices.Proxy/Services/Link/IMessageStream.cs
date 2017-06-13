//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Collections.Concurrent;
    using System.Threading;
    using System.Threading.Tasks;

    /// <summary>
    /// Message stream for async message send/receive between client
    /// and proxy and thus backbone of the endpoint implementation.
    /// </summary>
    public interface IMessageStream {

        ConcurrentQueue<Message> ReceiveQueue { get; }

        /// <summary>
        /// Receive a message from stream into queue
        /// </summary>
        /// <param name="ct">To cancel receive</param>
        /// <returns></returns>
        Task ReceiveAsync(CancellationToken ct);

        /// <summary>
        /// Send message on stream
        /// </summary>
        /// <param name="message"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        Task SendAsync(Message message, CancellationToken ct);
    }
}