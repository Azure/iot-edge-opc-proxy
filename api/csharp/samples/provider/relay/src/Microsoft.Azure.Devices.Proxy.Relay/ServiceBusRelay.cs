// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Provider {
    using System;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Collections.Concurrent;
    using Relay;
    using System.Linq;

    /// <summary>
    /// A stream service built on top of service bus hybrid connections Relay service. 
    /// Service bus relay provides a scalable, passthru websocket stream, whereby the 
    /// client library acts as listener, and the proxy as client. This is a different 
    /// pattern than more commonly used by relay, but saves users from standing
    /// up a stateful rendezvous point or scaled out websocket services.
    /// </summary>
    public class ServiceBusRelay : IStreamService, IDisposable {

        internal ConcurrentDictionary<Reference, ServiceBusRelayConnection> _connectionMap =
            new ConcurrentDictionary<Reference, ServiceBusRelayConnection>();

        /// <summary>
        /// Constructor to create service bus relay
        /// </summary>
        /// <param name="hcUri">Uri of connection in namespace</param>
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
        /// <returns>Stream service instance wrapping name space</returns>
        public static async Task<IStreamService> CreateAsync(string name, 
            ConnectionString connectionString) {
            var ns  = new ServiceBusNamespace(connectionString);
            var key = await ns.GetConnectionKeyAsync(name).ConfigureAwait(false);
            var relay = new ServiceBusRelay(ns.GetConnectionUri(name), 
                TokenProvider.CreateSharedAccessSignatureTokenProvider("proxy", key));

            await relay.OpenAsync().ConfigureAwait(false);
            return relay;
        }

        /// <summary>
        /// Creates connection
        /// </summary>
        /// <param name="streamId">Local reference id of the stream</param>
        /// <param name="remoteId">Remote reference of link</param>
        /// <param name="proxy">The proxy server</param>
        /// <param name="encoding">The encoding to use</param>
        /// <returns>Created connection</returns>
        public async Task<IConnection> CreateConnectionAsync(Reference streamId,
            Reference remoteId, INameRecord proxy, CodecId encoding) {
            var uri = new UriBuilder(_uri);
            uri.Scheme = "http";
            var token = await _tokenProvider.GetTokenAsync(uri.ToString(), 
                TimeSpan.FromHours(24)).ConfigureAwait(false);
            var cs = ConnectionString.Create(_uri.DnsSafeHost,
                _uri.AbsolutePath.TrimStart('/'), "proxy", token.TokenString, false);
            var connection = new ServiceBusRelayConnection(this, streamId, remoteId, encoding, cs);
            _connectionMap.AddOrUpdate(streamId, connection, (r, s) => connection);
            return connection;
        }

        /// <summary>
        /// Opens the listener and starts listening thread
        /// </summary>
        private async Task OpenAsync() {
            _listener = new HybridConnectionListener(_uri, _tokenProvider);

            // Subscribe to the status events
            _listener.Connecting += (o, e) => OnConnecting(o, e);
            _listener.Offline += (o, e) => OnDisconnected(o, e);
            _listener.Online += (o, e) => OnConnected(o, e);
            _listener.AcceptHandler = (c) => OnAcceptAsync(c);

            await _listener.OpenAsync(_cts.Token).ConfigureAwait(false);
            _listenerTask = Task.Factory.StartNew(async () => await ListenAsync(), _cts.Token,
                TaskCreationOptions.LongRunning, TaskScheduler.Default).Unwrap();

            ProxyEventSource.Log.LocalListenerStarted(this);
        }

        /// <summary>
        /// Pre-check before accept.
        /// </summary>
        /// <param name="context"></param>
        /// <returns>Successful acceptance or not</returns>
        private Task<bool> OnAcceptAsync(RelayedHttpListenerContext context) {
            var id = context.Request.Headers["x-Id"];
            if (!string.IsNullOrEmpty(id) && Reference.TryParse(id, out Reference streamId)) {
                if (_connectionMap.TryGetValue(streamId, out ServiceBusRelayConnection connection)) {
                    // We correlate context and relay stream by finding 
                    // the stream id in the stream's tracking id
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
        private async Task ListenAsync() {
            while (!_cts.IsCancellationRequested) {
                try {
                    var relayConnection = await _listener.AcceptConnectionAsync().ConfigureAwait(false); 
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
                        var ctx = relayConnection.ToString().Split(' ', ':', '_');
                        foreach (var id in ctx) {
                            if (Reference.TryParse(id, out Reference streamId)) {
                                if (_connectionMap.TryGetValue(streamId, out ServiceBusRelayConnection connection)) {
                                    connected = connection.OpenAsync(relayConnection) != null;
                                    ProxyEventSource.Log.ConnectionAccepted(relayConnection);
                                    break;
                                }
                            }
                        }
                        if (!connected) {
                            ProxyEventSource.Log.ConnectionRejected(relayConnection, null);
                        }
                    }
                    catch (Exception e) {
                        // Some error occurred
                        ProxyEventSource.Log.ConnectionRejected(relayConnection, e);
                    }
                    finally {
                        if (!connected) {
                            // ... close the connection
                            var closing = relayConnection.CloseAsync(new CancellationTokenSource(
                                _closeTimeout).Token);
                        }
                    }
                }
                catch (Exception ex) {
                    ProxyEventSource.Log.HandledExceptionAsError(this, ex);
                }
            }
            await _listener.CloseAsync().ConfigureAwait(false);
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
                await Task.WhenAll(
                    _connectionMap.Values.Select((s) => s.CloseAsync())).ConfigureAwait(false);
            }
            catch (Exception ex) {
                ProxyEventSource.Log.HandledExceptionAsWarning(this, ex);
            }
            finally {
                _connectionMap.Clear();
                await _listenerTask.ConfigureAwait(false);
            }
        }

        public void Dispose() {
            try {
                CloseAsync().GetAwaiter().GetResult();
            }
            catch {
            }
        }

        private TokenProvider _tokenProvider;
        private Uri _uri;
        private HybridConnectionListener _listener;
        private Task _listenerTask;
        private CancellationTokenSource _cts = new CancellationTokenSource();
        internal static readonly TimeSpan _closeTimeout = TimeSpan.FromSeconds(3);
    }
}
