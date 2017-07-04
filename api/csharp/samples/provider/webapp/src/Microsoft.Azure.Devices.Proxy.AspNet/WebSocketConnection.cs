// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Provider {
    using Proxy;
    using System;
    using System.Threading;
    using System.Threading.Tasks;

    /// <summary>
    /// Specialized implementation of a websocket based message stream
    /// </summary>
    internal class WebSocketConnection : ConnectionBase<WebSocketStream> {
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="provider"></param>
        /// <param name="streamId"></param>
        /// <param name="encoding"></param>
        /// <param name="connectionString"></param>
        public WebSocketConnection(WebSocketMiddleware provider, Reference streamId,
            Reference remoteId, CodecId encoding, ConnectionString connectionString) : 
            base(streamId, remoteId, encoding, connectionString) {
            _provider = provider;
        }

        /// <summary>
        /// Close connection
        /// </summary>
        /// <returns></returns>
        public override void Close() {
            // Remove ourselves from the listener...
            _provider._connectionMap.TryRemove(StreamId, out WebSocketConnection stream);
        }

        protected override async Task CloseStreamAsync(ICodecStream<WebSocketStream> codec) {
            try {
                await codec.Stream.CloseAsync(new CancellationTokenSource(_closeTimeout).Token);
            }
            catch {}
        }

        private static readonly TimeSpan _closeTimeout = TimeSpan.FromSeconds(3);
        private WebSocketMiddleware _provider;
    }
}

