//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System.Threading.Tasks.Dataflow;

    /// <summary>
    /// Message stream for async message send/receive between client
    /// and proxy and thus backbone of the endpoint implementation.
    /// </summary>
    public interface IMessageStream {

        /// <summary>
        /// Target block to send messages to proxy
        /// </summary>
        ITargetBlock<Message> SendBlock {
            get;
        }

        /// <summary>
        /// Source block to receive messages from proxy
        /// </summary>
        ISourceBlock<Message> ReceiveBlock {
            get;
        }
    }
}