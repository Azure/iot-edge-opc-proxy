// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Provider {

    using System;
    using System.IO;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Collections.Concurrent;
    using Relay;
    using Model;
    using System.Collections.Generic;
    using System.Linq;

    /// <summary>
    /// A stream service built on top of service bus hybrid connections Relay service. 
    /// Service bus relay provides a scalable, passthru websocket stream, whereby the 
    /// client library acts as listener, and the proxy as client. This is a different 
    /// pattern than more commonly used by relay, but saves users from standing
    /// up a stateful rendezvous point or scaled out websocket services.
    /// </summary>
    public class ServiceBusRelay : IStreamService, IDisposable {
        private TokenProvider _tokenProvider;
        private Uri _uri;
        private HybridConnectionListener _listener;
        private Task _listenerTask;
        private CancellationTokenSource _cts = new CancellationTokenSource();

        /// <summary>
        /// Specialized implementation of relay based message stream
        /// </summary>
        class RelayStream : IConnection, IMessageStream {
            internal ServiceBusRelay _relay;
            private HybridConnectionStream _stream;
            private CancellationTokenSource _open = new CancellationTokenSource();

#if NET45 || NET46
            // An outside buffered or memory stream is needed to avoid
            // message pack to wrap with its own buffered stream which 
            // we cannot flush. 
            private BufferedStream _buffered;
#endif
            private Task _producerTask;
            private CancellationTokenSource _streaming;
            private BlockingCollection<TaskCompletionSource<bool>> _consumerQueue =
                new BlockingCollection<TaskCompletionSource<bool>>();

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
            /// Link reference
            /// </summary>
            internal Reference LinkId { get; private set; }

            /// <summary>
            /// Whether we were closed
            /// </summary>
            public bool Connected { get; set; } = false;

            /// <summary>
            /// Connection string for connection
            /// </summary>
            public ConnectionString ConnectionString { get; private set; }

            /// <summary>
            /// Constructor
            /// </summary>
            /// <param name="relay"></param>
            /// <param name="linkId"></param>
            /// <param name="connectionString"></param>
            public RelayStream(ServiceBusRelay relay, Reference linkId,
                ConnectionString connectionString) {
                _relay = relay;
                LinkId = linkId;
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
                RelayStream stream;
                _relay._streamMap.TryRemove(LinkId, out stream);
                if (_open.IsCancellationRequested)
                    return;
                try {
                    // Set close state
                    _open.Cancel();
                    _consumerQueue.CompleteAdding();

                    // Fail any in progress open 
                    Tcs.TrySetException(new SocketException(SocketError.Closed));

                    if (_producerTask != null) {
                        // Wait until stream etc. is closed and gone
                        _streaming.Cancel();
                        await _producerTask;
                    }
                }
                catch (Exception) {
                    // No-op
                }
                _producerTask = null;
                TaskCompletionSource<bool> tcs;
                while (_consumerQueue.TryTake(out tcs, 0)) {
                    // Cancel all waiting readers due to closing
                    tcs.TrySetCanceled();
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
#if NET45 || NET46
                    await message.EncodeAsync(_buffered, CodecId.Mpack, ct);
                    await _buffered.FlushAsync();
#else
                    // Poor man's buffered stream.  
                    var stream = new MemoryStream();
                    message.Encode(stream, CodecId.Mpack);
                    var buffered = stream.ToArray();

                    await _stream.WriteAsync(buffered, 0, buffered.Length, ct);
                    await _stream.FlushAsync(ct);
#endif
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
            /// Receive producer, reading messages one by one from relay and 
            /// notifying consumers by completing queued completion sources.
            /// </summary>
            /// <returns></returns>
            private async Task ReceiveProducerAsync() {
                Connected = true;
                ProxyEventSource.Log.StreamOpened(this, _stream);
                try {
                    // Start to pump and satisfy all waiting consumers
                    await Task.Factory.StartNew(() => {
                        Message message;
                        TaskCompletionSource<bool> tcs = null;

                        while(true) {
                            try {
                                if (_consumerQueue.TryTake(out tcs, -1, _streaming.Token)) {
                                    if (!tcs.Task.IsCanceled) {
                                        try {
                                            // Read and decode a message
                                            message = Message.DecodeAsync(
                                                _stream, CodecId.Mpack, _streaming.Token).Result;
                                            ReceiveQueue.Enqueue(message);
                                            tcs.TrySetResult(true);
                                        }
                                        catch (Exception e) {
                                            ProxyEventSource.Log.StreamException(this, _stream, e);
                                            // Give up on this stream for now, 
                                            // and try again later
                                            _consumerQueue.TryAdd(tcs);
                                            break;
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
                    }, _streaming.Token, TaskCreationOptions.LongRunning, TaskScheduler.Default);

                    if (_open.IsCancellationRequested) {
                        // User is asking for a graceful close, shutdown properly
                        await _stream.ShutdownAsync(new CancellationTokenSource(
                            _closeTimeout).Token);
                    }
                }
                catch (Exception) { }
                ProxyEventSource.Log.StreamClosing(this, _stream);
                if (_open.IsCancellationRequested) {
                    try {
                        // ... then gracefully close
                        await _stream.CloseAsync(new CancellationTokenSource(
                            _closeTimeout).Token);
                    }
                    catch (Exception) { }
                }
#if NET45 || NET46
                try {
                    _buffered.Dispose();
                }
                catch (Exception) { }
#endif
                try {
                    _stream.Dispose();
                }
                catch (Exception) { }
                ProxyEventSource.Log.StreamClosed(this, _stream);
                _stream = null;
#if NET45 || NET46
                _buffered = null;
#endif
                Connected = false;
            }

            /// <summary>
            /// Connect the stream to a accepted stream instance and start the producer
            /// </summary>
            /// <param name="stream"></param>
            internal bool TryConnect(HybridConnectionStream stream) {
                if (_open.IsCancellationRequested) {
                    // User closed, but proxy tries to connect, reject
                    return false;
                }
                if (_producerTask != null && !_producerTask.IsCompleted) {
                    // Ensure previous producer task has finished
                    try {
                        if (_streaming != null) {
                            _streaming.Cancel();
                        }
                        _producerTask.GetAwaiter().GetResult();
                    }
                    catch (Exception) {
                        // No-op
                    }
                }

                _stream = stream;
#if NET45 || NET46
                _buffered = new BufferedStream(_stream);
#endif
                _streaming = new CancellationTokenSource();
                _producerTask = ReceiveProducerAsync();

                Tcs.TrySetResult(this);
                return true;
            }
        }

        ConcurrentDictionary<Reference, RelayStream> _streamMap =
            new ConcurrentDictionary<Reference, RelayStream>();

        /// <summary>
        /// Constructor to create service bus relay
        /// </summary>
        /// <param name="hcName">Uri of connection in namespace</param>
        /// <param name="provider">Shared access token provider</param>
        private ServiceBusRelay(Uri hcUri, TokenProvider provider) {
            _tokenProvider = provider;
            _uri = hcUri;
        }

        /// <summary>
        /// Create stream service by accessing the namespace.  If the 
        /// connection with the given name does not exist, it is created.
        /// Using the key it creates a token provider to instantiate the
        /// relay service.
        /// </summary>
        /// <param name="name">Name of the listener connection</param>
        /// <param name="connectionString">Relay root connection string</param>
        /// <returns></returns>
        public static async Task<IStreamService> CreateAsync(string name, 
            ConnectionString connectionString) {
            var ns  = new ServiceBusNamespace(connectionString);
            var key = await ns.GetConnectionKeyAsync(name);
            var relay = new ServiceBusRelay(ns.GetConnectionUri(name), 
                TokenProvider.CreateSharedAccessSignatureTokenProvider("proxy", key));

            await relay.OpenAsync();
            return relay;
        }

        /// <summary>
        /// Creates connection
        /// </summary>
        /// <param name="linkId"></param>
        /// <returns></returns>
        public async Task<IConnection> CreateConnectionAsync(Reference linkId) {
            var uri = new UriBuilder(_uri);
            uri.Scheme = "http";
            var token = await _tokenProvider.GetTokenAsync(uri.ToString(), TimeSpan.FromHours(1));
            var connection = new RelayStream(
                this, linkId, new ConnectionString(_uri, "proxy", token));
            _streamMap.AddOrUpdate(linkId, connection, (r, s) => connection);
            return connection;
        }

        /// <summary>
        /// Opens the listener and starts listening thread
        /// </summary>
        /// <returns></returns>
        private async Task OpenAsync() {
            _listener = new HybridConnectionListener(_uri, _tokenProvider);

            // Subscribe to the status events
            _listener.Connecting += (o, e) => OnConnecting(o, e);
            _listener.Offline += (o, e) => OnDisconnected(o, e);
            _listener.Online += (o, e) => OnConnected(o, e);
            _listener.AcceptHandler = (c) => OnAcceptAsync(c);

            await _listener.OpenAsync(_cts.Token);
            _listenerTask = Task.Factory.StartNew(() => ListenAsync().Wait(), _cts.Token,
                TaskCreationOptions.LongRunning, TaskScheduler.Default);

            ProxyEventSource.Log.LocalListenerStarted(this);
        }

        /// <summary>
        /// Pre-check before accept.
        /// </summary>
        /// <param name="context"></param>
        /// <returns></returns>
        private Task<bool> OnAcceptAsync(RelayedHttpListenerContext context) {
            Reference streamId;
            var id = context.Request.Headers["x-Id"];
            if (!string.IsNullOrEmpty(id) && Reference.TryParse(id, out streamId)) {
                RelayStream stream;
                if (_streamMap.TryGetValue(streamId, out stream)) {
                    // We correlate context and relay stream by finding the stream id in the stream's tracking id
                    if (context.ToString().ToLower().Contains(id.ToLower())) {
                        return Task.FromResult(true);
                    }
                }
            }
            ProxyEventSource.Log.ConnectionRejected(context, null);
            return Task.FromResult(false);
        }

        /// <summary>
        /// Run listener
        /// </summary>
        /// <param name="ct"></param>
        /// <returns></returns>
        private async Task ListenAsync() {
            while (!_cts.IsCancellationRequested) {
                try {
                    var relayConnection = await _listener.AcceptConnectionAsync();
                    if (relayConnection == null) {
                        ProxyEventSource.Log.LocalListenerClosed(this);
                        break;
                    }

                    // Correlate the accepted connection to an open stream in our map
                    bool connected = false;
                    try {
                        //
                        // Find the stream id in the stream's tracking id and connect it
                        // Ideally we get the tracking id from the connection, like 
                        //   var id = relayConnection.Id;
                        // Instead we have to parse it out of the connection string...
                        //
                        Reference streamId;
                        var ctx = relayConnection.ToString().Split(' ', ':', '_');
                        foreach (var id in ctx) {
                            if (Reference.TryParse(id, out streamId)) {
                                RelayStream stream;
                                if (_streamMap.TryGetValue(streamId, out stream)) {
                                    connected = await Task.Run(() => stream.TryConnect(relayConnection));
                                    ProxyEventSource.Log.ConnectionAccepted(relayConnection);
                                    break;
                                }
                            }
                        }
                        if (!connected) {
                            // Socket was already disconnected, or accept was cancelled, shutdown...
                            await relayConnection.ShutdownAsync(new CancellationTokenSource(
                                _closeTimeout).Token);
                        }
                    }
                    catch (Exception e) {
                        // Some error occurred
                        ProxyEventSource.Log.ConnectionRejected(relayConnection, e);
                    }
                    finally {
                        if (!connected) {
                            // ... and close the connection
                            await relayConnection.CloseAsync(new CancellationTokenSource(
                                _closeTimeout).Token);
                        }
                    }
                }
                catch (Exception ex) {
                    ProxyEventSource.Log.HandledExceptionAsError(this, ex);
                }
            }
            await _listener.CloseAsync();
        }

        private void OnConnecting(object o, EventArgs e) {
        }

        private void OnDisconnected(object o, EventArgs e) {
        }

        private void OnConnected(object o, EventArgs e) {
        }

        /// <summary>
        /// Stop listener and close all associated streams
        /// </summary>
        /// <returns></returns>
        private async Task CloseAsync() {
            try {
                _cts.Cancel();
                await Task.WhenAll(_streamMap.Values.Select((s) => s.CloseAsync()));
            }
            catch (Exception ex) {
                ProxyEventSource.Log.HandledExceptionAsWarning(this, ex);
            }
            finally {
                _streamMap.Clear();
                await _listenerTask;
            }
        }

        public void Dispose() {
            try {
                CloseAsync().GetAwaiter().GetResult();
            }
            catch (Exception) {
            }
        }

        static readonly TimeSpan _closeTimeout = TimeSpan.FromSeconds(3);
    }
}
