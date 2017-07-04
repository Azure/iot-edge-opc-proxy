//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System.Threading;
    using System.Threading.Tasks;

    /// <summary>
    /// Connection represents an endpoint on our side that is accessible
    /// by the proxy (e.g. a websocket endpoint, or iot hub). The connection
    /// string property provides direct help as to how the proxy can connect
    /// to the endpoint.  Once the proxy was informed about the endpoint and
    /// connects up to it, the owner of the connection can open it and attach
    /// a message stream.
    /// </summary>
    public interface IConnection {

        /// <summary>
        /// Connection string for this connection that allows the proxy to
        /// open up a connection to us.  This can be null, in which case the
        /// proxy is assumed to use the existing device methods interface
        /// to connect up. 
        /// </summary>
        ConnectionString ConnectionString {
            get;
        }

        /// <summary>
        /// Whether this stream must be polled for receiving. This option 
        /// must be communicated to the proxy to allow it to set up the right
        /// send code path.
        /// </summary>
        bool IsPolled {
            get;
        }

        /// <summary>
        /// Opens the connection and connects the message stream to the 
        /// underlying streaming implementation (e.g. a websocket stream or
        /// http send/recv).  Open might need to wait until the proxy connects
        /// before it returns hence it should be possible to cancel.
        /// </summary>
        /// <param name="ct">To cancel the open operation</param>
        /// <returns></returns>
        Task<IMessageStream> OpenAsync(CancellationToken ct);

        /// <summary>
        /// Close connection and unlink any connected message stream blocks.
        /// </summary>
        /// <returns></returns>
        Task CloseAsync();
    }
}