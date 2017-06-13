// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Provider {
    using Proxy;
    using System.Threading.Tasks;
    using System;
    using Microsoft.AspNetCore.Builder;
    using Microsoft.AspNetCore.Http;
    using System.Net.WebSockets;
    using System.Collections.Concurrent;
    using System.Threading;

    /// <summary>
    /// asp.net core middleware that provides streamprovider that uses websockets as stream service
    /// implementation.
    /// </summary>
    public partial class WebsocketProvider : DefaultProvider, IStreamService {

        /// <summary>
        /// Exposes the new asp.net stream service on the default provider
        /// </summary>
        public override IStreamService StreamService {
            get {
                return this;
            }
        }

        ConcurrentDictionary<Reference, WebSocketMessageStream> _streamMap =
            new ConcurrentDictionary<Reference, WebSocketMessageStream>();

        /// <summary>
        /// Specialized implementation of a websocket based message stream
        /// </summary>
        class WebSocketMessageStream : IConnection, IMessageStream {
            WebsocketProvider _provider;

            /// <summary>
            /// Receive queue 
            /// </summary>
            public ConcurrentQueue<Message> ReceiveQueue { get; } =
                new ConcurrentQueue<Message>();

            /// <summary>
            /// Stream open completion source
            /// </summary>
            internal TaskCompletionSource<IMessageStream> Tcs { get; private set; } =
                new TaskCompletionSource<IMessageStream>();

            /// <summary>
            /// Stream reference
            /// </summary>
            internal Reference StreamId { get; private set; }

            /// <summary>
            /// Whether we were closed
            /// </summary>
            public bool Connected { get; set; } = false;

            /// <summary>
            /// Connection string for connection
            /// </summary>
            public ConnectionString ConnectionString { get; private set; }

            /// <summary>
            /// Never polled
            /// </summary>
            public bool IsPolled { get; } = false;

            /// <summary>
            /// Constructor
            /// </summary>
            /// <param name="provider"></param>
            /// <param name="streamId"></param>
            /// <param name="connectionString"></param>
            public WebSocketMessageStream(WebsocketProvider provider, Reference streamId,
                ConnectionString connectionString) {
                _provider = provider;
                StreamId = streamId;
                ConnectionString = connectionString;
            }

            /// <summary>
            /// Accept this stream
            /// </summary>
            /// <param name="ct"></param>
            /// <returns></returns>
            public Task<IMessageStream> OpenAsync(CancellationToken ct) {
                ct.Register(() => {
                    Tcs.TrySetCanceled();
                });
                return Tcs.Task;
            }

            /// <summary>
            /// Close stream
            /// </summary>
            /// <returns></returns>
            public async Task CloseAsync() {
                // Remove ourselves from the listener...
                WebSocketMessageStream stream;
                _provider._streamMap.TryRemove(StreamId, out stream);

                // If not remotely closed, close streaming now
                if (!_open.IsCancellationRequested) {
                    try {
                        // Set close state
                        _open.Cancel();
                        _consumerQueue.CompleteAdding();

                        // Fail any in progress open 
                        Tcs.TrySetException(new SocketException(SocketError.Closed));

                        ProxyEventSource.Log.StreamClosing(this, _codec.Stream);
                        if (_producerTask != null) {
                            // Wait until stream etc. is closed and gone
                            _streaming.Cancel();
                            await _producerTask.ConfigureAwait(false);
                        }
                    }
                    catch (Exception ex) {
                        ProxyEventSource.Log.HandledExceptionAsWarning(this, ex);
                    }
                    _producerTask = null;
                }

                // Mark all waiting readers as cancelled
                TaskCompletionSource<bool> tcs;
                while (_consumerQueue.TryTake(out tcs, 0)) {
                    // Cancel all waiting readers due to closing
                    tcs.TrySetException(new SocketException(SocketError.Closed));
                }
            }

            /// <summary>
            /// Send message
            /// </summary>
            /// <param name="message"></param>
            /// <param name="ct"></param>
            /// <returns></returns>
            public async Task SendAsync(Message message, CancellationToken ct) {
                try {
                    await _codec.WriteAsync(message, ct).ConfigureAwait(false);
                    await _codec.Stream.FlushAsync(ct).ConfigureAwait(false);
                }
                catch (Exception e) {
                    throw ProxyEventSource.Log.Rethrow(e, this);
                }
            }

            /// <summary>
            /// Receive consumer using task completion source as notification.
            /// </summary>
            /// <param name="ct"></param>
            /// <returns></returns>
            public Task ReceiveAsync(CancellationToken ct) {
                if (_open.IsCancellationRequested ||
                    _consumerQueue.IsAddingCompleted) {
                    throw new ProxyException(SocketError.Closed);
                }

                Message message;
                if (ReceiveQueue.TryPeek(out message)) {
                    return Task.FromResult(true);
                }
                else {
                    // Add cancellable item to signal on completion
                    var tcs = new TaskCompletionSource<bool>();
                    ct.Register(() => {
                        tcs.TrySetCanceled();
                    });
                    try {
                        _consumerQueue.Add(tcs, _open.Token);
                    }
                    catch (OperationCanceledException) {
                    }
                    catch (Exception ex) {
                        // Continue waiting for receives as long as we are not cancelled
                        ProxyEventSource.Log.HandledExceptionAsInformation(this, ex);
                    }
                    ct.ThrowIfCancellationRequested();
                    return tcs.Task;
                }
            }

            /// <summary>
            /// Receive producer, reads messages one by one from websocket and 
            /// notifies consumers by completing queued completion sources.
            /// </summary>
            /// <returns></returns>
            private async Task ReceiveProducerAsync(CancellationToken ct) {
                Connected = true;
                ProxyEventSource.Log.StreamOpened(this, _codec.Stream);
                try {
                    // Start to pump and satisfy all waiting consumers
                    await Task.Factory.StartNew(() => ReceiveProducer(ct),
                        _open.Token, TaskCreationOptions.LongRunning,
                            TaskScheduler.Default).ConfigureAwait(false);
                }
                catch { }
                try {
                    // ... then gracefully close
                    await _codec.Stream.CloseAsync(
                        CancellationToken.None).ConfigureAwait(false);
                }
                catch { }
                try {
                    _codec.Stream.Dispose();
                }
                catch { }
                ProxyEventSource.Log.StreamClosed(this, _codec.Stream);
                _codec.Stream = null;
                Connected = false;
            }

            /// <summary>
            /// Sync Receive producer loop
            /// </summary>
            /// <param name="stream"></param>
            /// <param name="ct"></param>
            private void ReceiveProducer(CancellationToken ct) {
                Message message;
                while (true) {
                    try {
                        TaskCompletionSource<bool> tcs = null;
                        if (_consumerQueue.TryTake(out tcs, -1, ct)) {
                            if (!tcs.Task.IsCanceled) {
                                try {
                                    message = _codec.ReadAsync<Message>(ct).Result;
                                    ReceiveQueue.Enqueue(message);
                                    if (message.TypeId == MessageContent.Close) {
                                        // Remote side closed, close the stream
                                        _open.Cancel();
                                        break;
                                    }
                                }
                                catch (Exception e) {
                                    ProxyEventSource.Log.StreamException(
                                        this, _codec.Stream, e);
                                    // Exit
                                    break;
                                }
                                finally {
                                    // Caller will retry if the queue is empty
                                    tcs.TrySetResult(true);
                                }
                            }
                        }
                    }
                    catch (OperationCanceledException) {
                        // Stream closing
                        break;
                    }
                    if (_streaming.IsCancellationRequested ||
                        _open.IsCancellationRequested ||
                        _consumerQueue.IsAddingCompleted) {
                        break;
                    }
                }
            }

            /// <summary>
            /// Connect the stream to a accepted stream instance and start the producer
            /// </summary>
            /// <param name="stream"></param>
            internal bool TryConnect(WebSocket webSocket) {
                if (_open.IsCancellationRequested) {
                    // Stream closed, but proxy tries to connect, reject
                    return false;
                }
                // Ensure previous producer task finishes
                if (_producerTask != null && !_producerTask.IsCompleted) {
                    if (_streaming != null) {
                        _streaming.Cancel();
                        try {
                            _producerTask.Wait();
                        }
                        catch (Exception) {
                            return false;
                        }
                    }
                }

                _codec.Stream = new WebSocketStream(webSocket);
                _streaming = new CancellationTokenSource();
                _producerTask = ReceiveProducerAsync(_streaming.Token);
                Tcs.TrySetResult(this);
                return true;
            }

            private MsgPackStream<WebSocketStream> _codec = 
                new MsgPackStream<WebSocketStream>();
            private CancellationTokenSource _open = new CancellationTokenSource();
            private Task _producerTask;
            private CancellationTokenSource _streaming;
            private BlockingCollection<TaskCompletionSource<bool>> _consumerQueue =
                new BlockingCollection<TaskCompletionSource<bool>>();
        }
        
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="next"></param>
        /// <param name="uri"></param>
        /// <param name="iothub"></param>
        public WebsocketProvider(RequestDelegate next, Uri uri, string iothub) :
            base(iothub) {
            _uri = uri;
            _next = next;
            Socket.Provider = this;
        }

        /// <summary>
        /// Handle all websocket requests
        /// </summary>
        /// <param name="context"></param>
        /// <returns></returns>
        public async Task Invoke(HttpContext context) {
            try {
                if (context.WebSockets.IsWebSocketRequest && context.Request.IsHttps) {
                    // Correlate the accepted socket to an open stream in our map
                    Reference streamId;
                    if (Reference.TryParse(context.Request.PathBase, out streamId)) {
                        WebSocketMessageStream stream;
                        if (_streamMap.TryGetValue(streamId, out stream)) {
                            var webSocket = await context.WebSockets.AcceptWebSocketAsync();
                            bool connected = await Task.Run(
                                () => stream.TryConnect(webSocket)).ConfigureAwait(false);
                            if (connected) {
                                ProxyEventSource.Log.ConnectionAccepted(context);
                                return; // Connected
                            }
                        }
                        ProxyEventSource.Log.ConnectionRejected(context, null);
                    }
                }
            }
            catch (Exception e) {
                // Some error occurred
                ProxyEventSource.Log.ConnectionRejected(context, e);
            }
            await _next(context);
        }

        /// <summary>
        /// Returns a new connection
        /// </summary>
        /// <param name="streamId">Local reference id of the stream</param>
        /// <param name="remoteId">Remote reference of link</param>
        /// <param name="proxy">The proxy server</param>
        /// <returns></returns>
        public Task<IConnection> CreateConnectionAsync(Reference streamId,
            Reference remoteId, INameRecord proxy) {
            var uri = new UriBuilder(_uri);
            uri.Scheme = "wss";
            uri.Path = streamId.ToString();
            var connection = new WebSocketMessageStream(
                this, streamId, new ConnectionString(uri.Uri, "proxy", "secret"));
            _streamMap.AddOrUpdate(streamId, connection, (r, s) => connection);
            return Task.FromResult((IConnection)connection);
        }

        private Uri _uri;
        private RequestDelegate _next;
    }


    /// <summary>
    /// Helper extension class
    /// </summary>
    public static class Extensions {
        /// <summary>
        /// Attach proxy handler to application
        /// </summary>
        /// <param name="app"></param>
        /// <returns></returns>
        public static void ConfigureProxy(this IApplicationBuilder app, Uri endpoint) {
            app.UseWebSockets();
            app.UseMiddleware<WebsocketProvider>(endpoint, null);
        }

        /// <summary>
        /// Attach proxy handler to application
        /// </summary>
        /// <param name="app"></param>
        /// <param name="iothub"></param>
        /// <returns></returns>
        public static void ConfigureProxy(this IApplicationBuilder app, Uri endpoint,
            string iotHub) {
            app.UseWebSockets();
            app.UseMiddleware<WebsocketProvider>(endpoint, iotHub);
        }
    }
}
