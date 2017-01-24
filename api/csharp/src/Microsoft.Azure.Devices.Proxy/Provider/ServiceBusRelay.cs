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
            HybridConnectionStream _stream;
            CancellationTokenSource _open = new CancellationTokenSource();
            Task _producerTask;

            // An outside buffered or memory stream is needed to avoid
            // message pack to wrap with its own buffered stream which 
            // we cannot flush. (packer seems to be broken in 0.9.0)
#if NET45 || NET46
            BufferedStream _buffered;
#endif
            private ServiceBusRelay _relay;

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
                if (_stream == null)
                    return;
                try {
                    // Fail any in progress open 
                    Tcs.TrySetException(new SocketException(SocketError.Closed));
                    // Set close state
                    _consumerQueue.CompleteAdding();
                    _open.Cancel();
                    // Finish producing anything, which closes any open stream
                    await _producerTask;
                }
                catch (Exception) {
                    // No-op
                }
                _producerTask = null;

                // Remove ourselves from the relay listener...
                RelayStream stream;
                _relay._streamMap.TryRemove(LinkId, out stream);
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
                if (_open.IsCancellationRequested) {
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
                // Start to pump and satisfy all waiting consumers
                await Task.Factory.StartNew(() => {
                    Message message;
                    TaskCompletionSource<bool> tcs = null;

                    Connected = true;
                    for (int wait = 1; !_consumerQueue.IsAddingCompleted; wait *= 2) {
                        if (_consumerQueue.TryTake(out tcs, wait - 1, _open.Token)) {
                            wait = 1;
                            if (!tcs.Task.IsCanceled) {
                                if (_open.IsCancellationRequested) {
                                    // Cancelled due to closing
                                    tcs.TrySetCanceled();
                                }
                                else {
                                    try {
                                        // Read and decode a message
                                        message = Message.DecodeAsync(
                                            _stream, CodecId.Mpack, _open.Token).Result;
                                        ReceiveQueue.Enqueue(message);
                                        tcs.TrySetResult(true);
                                    }
                                    catch (Exception e) {
                                        // In case of exception, we do not propagate 
                                        // to let underlying stream recover, or user
                                        // tasks to time out eventually.
                                        ProxyEventSource.Log.StreamClosing(this, e);

                                        // Give up on this stream for now, 
                                        // and try again later
                                        _consumerQueue.TryAdd(tcs);
                                        break;
                                    }
                                }
                            }
                        }
                        else if (_open.IsCancellationRequested) {
                            ProxyEventSource.Log.StreamClosing(this, null);
                            break;
                        }
                    }
                }, _open.Token, TaskCreationOptions.LongRunning, TaskScheduler.Default);

                // Close down stream...
                try {
                    await _stream.ShutdownAsync(CancellationToken.None);
                    await _stream.CloseAsync(CancellationToken.None);
                }
                catch (Exception) {
                    // eat
                }
                finally {
                    try {
                        _stream.Dispose();
                    }
                    catch (Exception) { }
                    finally {
                        _stream = null;
                    }
#if NET45 || NET46
                    try {
                        _buffered.Dispose();
                    }
                    catch (Exception) { }
                    finally {
                        _buffered = null;
                    }
#endif
                    Connected = false;
                }
                ProxyEventSource.Log.StreamClosed(this);
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
            _listener.Connecting += (o, e) => { Console.WriteLine("Connecting"); };
            _listener.Offline += (o, e) => { Console.WriteLine("Offline"); };
            _listener.Online += (o, e) => { Console.WriteLine("Online"); };

            await _listener.OpenAsync(_cts.Token);
            _listenerTask = Task.Factory.StartNew(() => ListenAsync().Wait(), _cts.Token,
                TaskCreationOptions.LongRunning, TaskScheduler.Default);

            ProxyEventSource.Log.LocalListenerStarted(this);
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
                        await _listener.CloseAsync(_cts.Token);
                        ProxyEventSource.Log.LocalListenerClosed(this);
                        break;
                    }
                    else {
                        ProxyEventSource.Log.ConnectionAccepted(this, relayConnection);
                        var t = Task.Factory.StartNew(() => DoHandshakeAsync(relayConnection, _cts.Token).Wait());
                    }
                }
                catch(Exception ex) {
                    ProxyEventSource.Log.HandledExceptionAsError(this, ex);
                }
            }
            await _listener.CloseAsync(CancellationToken.None);
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

        /// <summary>
        /// Perform handshake
        /// </summary>
        /// <param name="relayConnection"></param>
        /// <param name="ct"></param>
        private async Task DoHandshakeAsync(HybridConnectionStream relayConnection, 
            CancellationToken ct) {
            try {
                bool handshakeCompleted = false;
                //
                // Accept connection.  The first message will be an open response, which
                // is sent as handshake on both the remoting side, as well as to open the 
                // stream.  This will allow us to connect the accepted connection to an
                // internal stream without knowing more about the client (since we cannot 
                // see the header and have no clue about auth.  
                //
                try {
                    var message = await Message.DecodeAsync(relayConnection, CodecId.Mpack, ct);
                    RelayStream stream;
                    if (_streamMap.TryGetValue(message.Target, out stream)) {
                        handshakeCompleted = await Task.Run(() => stream.TryConnect(relayConnection));
                        if (handshakeCompleted) {
                            ProxyEventSource.Log.StreamAcceptedAndConnected(message.Target);
                            return;
                        }
                        else {
                            // Race condition with user creating stream, but stream not yet removed.
                            ProxyEventSource.Log.StreamAcceptedNotConnected(message.Target);
                        }
                    }
                    else {
                        ProxyEventSource.Log.StreamRejected(message.Target);
                    }

                    // Socket was already disconnected, or accept was cancelled, shutdown...
                    await relayConnection.ShutdownAsync(ct);
                }
                catch (IOException ioex) {
                    // Remote side closed, log ...
                    ProxyEventSource.Log.RemoteProxyClosed(this, ioex);
                }
                catch (Exception e) {
                    // Another error occurred
                    ProxyEventSource.Log.RemoteProxyClosed(this, e);
                }
                finally {
                    if (!handshakeCompleted) {
                        // ... and close the connection
                        await relayConnection.CloseAsync(ct);
                    }
                }
            }
            catch (Exception e) {
                // Another exception occurred, catch to not crash...
                ProxyEventSource.Log.HandledExceptionAsError(this, e);
            }
        }
    }
}
