// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Collections.Generic;
    using System.Threading;
    using System.Threading.Tasks;
    using Provider;

    /// <summary>
    /// Socket facade
    /// </summary>
    public partial class Socket : IDisposable {

        private bool _cleanedUp;
        private IProxySocket _internal;

        //
        // Returns a socket file descriptor or handle
        //
        public IntPtr Handle {
            get { return (IntPtr)_internal.Id.GetHashCode(); }
        }

        //
        // Gets the socket's address family.
        //
        public AddressFamily AddressFamily {
            get { return _internal.Info.Family; }
        }

        //
        // Gets the socket's socketType.
        //
        public SocketType SocketType {
            get { return _internal.Info.Type; }
        }

        //
        // Gets the socket's protocol socketType.
        //
        public ProtocolType ProtocolType {
            get { return _internal.Info.Protocol; }
        }

        //
        // Returns whether this socket is bound to a port
        //
        public bool IsBound {
            get; private set;
        }

        //
        // Returns whether this socket is in listening mode
        //
        public bool IsListening {
            get; private set;
        }

        //
        // Returns whether the socket is connected or not
        //
        public bool Connected {
            get; private set;
        }

        //
        // Whether the socket is in blocking mode - no op
        //
        public bool Blocking {
            get; set;
        }

        //
        // Remote socket timeout
        //
        public static uint RemoteTimeout {
            get; set;
        }

        //
        // The service provider to use
        //
        public static IProvider Provider {
            get; set;
        } = NullProvider.Create();

        //
        // Create a socket
        //
        public Socket(SocketType socketType, ProtocolType protocolType)
            : this(AddressFamily.InterNetwork, socketType, protocolType) {
        }

        //
        // Create socket for address family 
        //
        public Socket(AddressFamily addressFamily, SocketType socketType, ProtocolType protocolType, uint keepAlive) :
            this(addressFamily, socketType, protocolType, null) {
            this._internal = ProxySocket.Create(new SocketInfo {
                Family = addressFamily,
                Protocol = protocolType,
                Type = socketType,
                Timeout = keepAlive
            }, Provider);
        }

        //
        // Create socket for address family 
        //
        public Socket(AddressFamily addressFamily, SocketType socketType, ProtocolType protocolType) :
            this(addressFamily, socketType, protocolType, RemoteTimeout) {
        }

        //
        // Create socket for address family 
        //
        internal Socket(AddressFamily addressFamily, SocketType socketType, ProtocolType protocolType, IProxySocket internalSocket) {
            this._internal = internalSocket;
            this.Connected = false;
            this.IsBound = false;
            this.IsListening = false;
        }

        //
        // Called by the class to create a socket to accept an incoming request.
        //
        internal Socket(Socket original, IProxySocket internalSocket) :
            this(original.AddressFamily, original.SocketType, original.ProtocolType, internalSocket) {
        }


        /// <summary>
        /// Returns the address of the proxy the socket is connected through
        /// </summary>
        public SocketAddress LocalEndPoint {
            get { return _internal?.ProxyAddress; }
        }

        /// <summary>
        /// Returns the address the proxy itself is connected to (peer)
        /// </summary>
        public SocketAddress RemoteEndPoint {
            get { return _internal?.PeerAddress; }
        }

        /// <summary>
        /// Returns the interface the proxy bound the remote connection to.
        /// </summary>
        public SocketAddress InterfaceEndPoint {
            get { return _internal?.LocalAddress; }
        }

        #region Connect

        //
        // Set/get the send timeout
        //
        public TimeSpan ConnectTimeout { get; set; } = TimeSpan.FromSeconds(30);

        //
        // Opens the socket and links as active
        //
        public async Task ConnectAsync(SocketAddress endpoint, CancellationToken ct) {
            if (_cleanedUp) {
                throw new ObjectDisposedException(this.GetType().FullName);
            }
            if (endpoint == null) {
                throw new ArgumentNullException("endpoint");
            }
            if (IsListening) {
                throw new InvalidOperationException("Socket is listening");
            }
            try {
                await _internal.ConnectAsync(endpoint, ct).ConfigureAwait(false);
                Connected = true;
                IsBound = true;
            }
            catch (OperationCanceledException) {
                throw;
            }
            catch (Exception e) {
                throw SocketException.Create($"Exception connecting to {endpoint.ToString()}", e);
            }
        }

        //
        // Without cancellation token
        //
        public async Task ConnectAsync(SocketAddress endpoint) {
            var cts = new CancellationTokenSource(ConnectTimeout);
            try {
                await ConnectAsync(endpoint, cts.Token).ConfigureAwait(false);
            }
            catch (OperationCanceledException) when (cts.IsCancellationRequested) {
                throw new TimeoutException(
                    $"Timeout after {ConnectTimeout.ToString()} connecting to {endpoint.ToString()}.");
            }
        }

        //
        // Sync connect
        //
        public void Connect(SocketAddress endpoint) =>
            TaskToSync.Run(() => ConnectAsync(endpoint));

        //
        // Using host and port
        //
        public void Connect(string host, int port) =>
            Connect(new ProxySocketAddress(host, (ushort)port, 0));

        //
        // Same, async
        //
        public Task ConnectAsync(string host, int port, CancellationToken ct) =>
            ConnectAsync(new ProxySocketAddress(host, port), ct);

        //
        // Same, async
        //
        public Task ConnectAsync(string host, int port) =>
            ConnectAsync(new ProxySocketAddress(host, port));

        //
        // Async, this time with event arg
        //
        public bool ConnectAsync(SocketAsyncEventArgs e) {
            var task = ConnectAsync(e.Endpoint).ContinueWith(t => {
                if (t.IsFaulted) {
                    e.SocketError = t.Exception.GetSocketError();
                }
                e.Complete(this);
            });
            return true;
        }

        //
        // Begin connecting
        //
        public IAsyncResult BeginConnect(SocketAddress endpoint, AsyncCallback callback, object state) =>
            TaskToApm.Begin(ConnectAsync(endpoint), callback, state);

        //
        // Begin connecting with host and port
        //
        public IAsyncResult BeginConnect(string host, int port, AsyncCallback callback, object state) =>
            BeginConnect(new ProxySocketAddress(host, port), callback, state);

        //
        // Complete connecting
        //
        public void EndConnect(IAsyncResult asyncResult) =>
            TaskToApm.End(asyncResult);

        #endregion

        #region Bind
        //
        // Binds a socket to a proxy identified by provided address
        //
        public async Task BindAsync(SocketAddress endpoint, CancellationToken ct) {
            if (_cleanedUp) {
                throw new ObjectDisposedException(this.GetType().FullName);
            }
            if (endpoint == null) {
                throw new ArgumentNullException("endpoint");
            }
            if (IsListening) {
                throw new InvalidOperationException("Socket is listening");
            }
            try {
                await _internal.BindAsync(endpoint, ct).ConfigureAwait(false);
                IsBound = true;
            }
            catch (OperationCanceledException) {
                throw;
            }
            catch (Exception e) {
                throw SocketException.Create($"Exception binding to {endpoint.ToString()}", e);
            }
        }

        //
        // Without cancellation token
        //
        public async Task BindAsync(SocketAddress endpoint) {
            var cts = new CancellationTokenSource(ConnectTimeout);
            try {
                await BindAsync(endpoint, cts.Token).ConfigureAwait(false);
            }
            catch (OperationCanceledException) when (cts.IsCancellationRequested) {
                throw new TimeoutException(
                    $"Timeout after {ConnectTimeout.ToString()} binding to {endpoint.ToString()}.");
            }
        }

        //
        // Bind, sync
        //
        public void Bind(SocketAddress endpoint) =>
            TaskToSync.Run(() => BindAsync(endpoint));

        //
        // With proxy name and port
        //
        public void Bind(string host, int port) =>
            Bind(new ProxySocketAddress(host, (ushort)port, 0));

        //
        // Same, but async
        //
        public Task BindAsync(string host, int port) =>
            BindAsync(new ProxySocketAddress(host, port));

        //
        // Bind, this time with event arg
        //
        public bool BindAsync(SocketAsyncEventArgs e) {
            var task = BindAsync(e.Endpoint).ContinueWith(t => {
                if (t.IsFaulted) {
                    e.SocketError = t.Exception.GetSocketError();
                }
                e.Complete(this);
            });
            return true;
        }

        //
        // Begin bind
        //
        public IAsyncResult BeginBind(SocketAddress endpoint, AsyncCallback callback, object state) =>
            TaskToApm.Begin(BindAsync(endpoint), callback, state);

        //
        // Begin bind with host and port
        //
        public IAsyncResult BeginBind(string host, int port, AsyncCallback callback, object state) =>
            BeginBind(new ProxySocketAddress(host, port), callback, state);

        //
        // Complete binding
        //
        public void EndBind(IAsyncResult asyncResult) =>
            TaskToApm.End(asyncResult);

        #endregion

        #region Listen

        //
        // Places a socket in a listening state.
        //
        public async Task ListenAsync(int backlog, CancellationToken ct) {
            if (_cleanedUp) {
                throw new ObjectDisposedException(this.GetType().FullName);
            }
            if (!IsBound) {
                throw new InvalidOperationException("Socket is not bound");
            }
            await _internal.ListenAsync(backlog, ct).ConfigureAwait(false);
            IsListening = true;
        }

        //
        // Without cancellation token
        //
        public async Task ListenAsync(int backlog = int.MaxValue) {
            var cts = new CancellationTokenSource(ConnectTimeout);
            try {
                await ListenAsync(backlog, cts.Token).ConfigureAwait(false);
            }
            catch (OperationCanceledException) when (cts.IsCancellationRequested) {
                throw new TimeoutException(
                    $"Timeout on listen after {ConnectTimeout.ToString()}.");
            }
        }

        //
        // Same, but sync
        //
        public void Listen(int backlog = int.MaxValue) =>
            TaskToSync.Run(() => ListenAsync(backlog));

        //
        // Same, this time with event arg
        //
        public bool ListenAsync(SocketAsyncEventArgs e) {
            var task = ListenAsync(e.Count).ContinueWith(t => {
                if (t.IsFaulted) {
                    e.SocketError = t.Exception.GetSocketError();
                }
                e.Complete(this);
            });
            return true;
        }

        //
        // Begin listening
        //
        public IAsyncResult BeginListen(int backlog, AsyncCallback callback, object state) {
            return TaskToApm.Begin(ListenAsync(backlog), callback, state);
        }

        //
        // Complete listening
        //
        public void EndListen(IAsyncResult asyncResult) {
            TaskToApm.End(asyncResult);
        }

        #endregion

        #region Accept

        //
        // Creates a new Socket instance to handle an incoming connection.
        //
        public async Task<Socket> AcceptAsync(CancellationToken ct) {
            if (_cleanedUp) {
                throw new ObjectDisposedException(this.GetType().FullName);
            }
            if (!IsListening) {
                throw new InvalidOperationException("Socket is not listening");
            }
            try {
                var result = await _internal.ReceiveAsync(
                    new ArraySegment<byte>(), ct).ConfigureAwait(false);
                ct.ThrowIfCancellationRequested();
                return new Socket(this, result.Link);
            }
            catch (OperationCanceledException) {
                throw;
            }
            catch (Exception e) {
                throw SocketException.Create("Exception during Accept", e);
            }
        }

        //
        // Without cancellation token
        //
        public async Task<Socket> AcceptAsync() {
            var cts = new CancellationTokenSource(ConnectTimeout);
            try {
                return await AcceptAsync(cts.Token).ConfigureAwait(false);
            }
            catch (OperationCanceledException) when (cts.IsCancellationRequested) {
                throw new TimeoutException(
                    $"Timeout on accept after {ConnectTimeout.ToString()}.");
            }
        }

        //
        // Same, but sync
        //
        public Socket Accept() => TaskToSync.Run(AcceptAsync);

        //
        // Same, but with async event args
        //
        public bool AcceptAsync(SocketAsyncEventArgs e) {
            AcceptAsync().ContinueWith(t => {
                try { 
                    e.AcceptSocket = t.Result;
                    e.SocketError = SocketError.Success;
                }
                catch (Exception ex) {
                    e.SocketError = ex.GetSocketError();
                }
                e.Complete(this);
                return t;
            });
            return true;
        }

        //
        // Begin accept call
        //
        public IAsyncResult BeginAccept(AsyncCallback callback, object state) {
            return TaskToApm.Begin(AcceptAsync(), callback, state);
        }

        //
        // Complete accept
        //
        public Socket EndAccept(IAsyncResult asyncResult) {
            return TaskToApm.End<Socket>(asyncResult);
        }

        #endregion

        #region Send

        //
        // Set/get the send timeout
        //
        public int SendTimeout { get; set; } = 20 * 1000;

        //
        // Send array segment 
        //
        public async Task<int> SendAsync(ArraySegment<byte> buffer, CancellationToken ct) {
            if (_cleanedUp) {
                throw new ObjectDisposedException(this.GetType().FullName);
            }
            if (buffer == null) {
                throw new ArgumentNullException("buffer");
            }
            try {
                var result = await _internal.SendAsync(buffer, null, ct).ConfigureAwait(false);
                ct.ThrowIfCancellationRequested();
                return result;
            }
            catch (OperationCanceledException) {
                throw;
            }
            catch (Exception e) {
                throw SocketException.Create("Exception during Send", e);
            }
        }

        //
        // Send array segment, without cancellation token
        //
        public async Task<int> SendAsync(ArraySegment<byte> buffer) {
            var cts = new CancellationTokenSource(SendTimeout);
            try {
                return await SendAsync(buffer, cts.Token).ConfigureAwait(false);
            }
            catch (OperationCanceledException) when (cts.IsCancellationRequested) {
                throw new TimeoutException(
                    $"Timeout on send after {SendTimeout.ToString()}.");
            }
        }

        //
        // Send buffer, with cancellation
        //
        public Task<int> SendAsync(byte[] buffer, int offset, int size, CancellationToken ct) =>
            SendAsync(CreateArraySegment(buffer, offset, size), ct);

        //
        // Send buffer without cancellation token
        //
        public Task<int> SendAsync(byte[] buffer, int offset, int size) =>
            SendAsync(CreateArraySegment(buffer, offset, size));

        //
        // Send buffer of size
        //
        public Task<int> SendAsync(byte[] buffer, int size) =>
            SendAsync(buffer, 0, size);

        //
        // Same, without offset
        //
        public Task<int> SendAsync(byte[] buffer) => 
            SendAsync(buffer, buffer != null ? buffer.Length : 0);

        //
        // Sends a data buffer to a connected socket with up to size bytes
        //
        public async Task<int> SendAsync(IEnumerable<ArraySegment<byte>> buffers, CancellationToken ct) {
            if (_cleanedUp) {
                throw new ObjectDisposedException(this.GetType().FullName);
            }
            if (buffers == null) {
                throw new ArgumentNullException("buffers");
            }
            if (!Connected) {
                throw new SocketException("Not connected");
            }
            int sent = 0;
            foreach (var buffer in buffers) {
                sent += await SendAsync(buffer, ct).ConfigureAwait(false);
            }
            return sent;
        }

        //
        // Without cancellation token
        //
        public async Task<int> SendAsync(IEnumerable<ArraySegment<byte>> buffers) {
            var cts = new CancellationTokenSource(SendTimeout);
            try {
                return await SendAsync(buffers, cts.Token).ConfigureAwait(false);
            }
            catch (OperationCanceledException) when (cts.IsCancellationRequested) {
                throw new TimeoutException(
                    $"Timeout on send after {SendTimeout.ToString()}.");
            }
        }

        //
        // Same, this time with event arg
        //
        public bool SendAsync(SocketAsyncEventArgs e) {
            Task<int> t1;
            if (e.Buffer != null) {
                t1 = SendAsync(CreateArraySegment(e.Buffer, e.Offset, e.Count));
            }
            else if (e.BufferList != null) {
                t1 = SendAsync(e.BufferList);
            }
            else {
                throw new ArgumentException("Buffer");
            }
            Task t2 = t1.ContinueWith(t => {
                try { 
                    e.BytesTransferred = t.Result;
                    e.SocketError = SocketError.Success;
                }
                catch (Exception ex) {
                    e.SocketError = ex.GetSocketError();
                }
                e.Complete(this);
            });
            return true;
        }

        //
        // Sync send a data buffer to a connected socket with up to size bytes
        //
        public int Send(ArraySegment<byte> buffer) =>
            TaskToSync.Run(() => SendAsync(buffer));

        //
        // Sends a data buffer to a connected socket with up to size bytes
        //
        public int Send(IEnumerable<ArraySegment<byte>> buffer) =>
            TaskToSync.Run(() => SendAsync(buffer));

        //
        // Sends a data buffer from offset to a connected socket with up to size bytes
        //
        public int Send(byte[] buffer, int offset, int size) =>
            Send(CreateArraySegment(buffer, offset, size));

        //
        // Sends a data buffer to a connected socket with up to size bytes
        //
        public int Send(byte[] buffer, int size) =>
            Send(buffer, 0, size);

        //
        // Sends a data buffer to a connected socket.
        //
        public int Send(byte[] buffer) =>
            Send(buffer, buffer != null ? buffer.Length : 0);

        //
        // Begin send call with buffer vector
        //
        public IAsyncResult BeginSend(IEnumerable<ArraySegment<byte>> buffers, AsyncCallback callback, object state) =>
            TaskToApm.Begin(SendAsync(buffers), callback, state);

        //
        // Begin send call
        //
        public IAsyncResult BeginSend(byte[] buffer, int offset, int size, AsyncCallback callback, object state) =>
            TaskToApm.Begin(SendAsync(buffer, offset, size), callback, state);

        //
        // Complete send
        //
        public int EndSend(IAsyncResult asyncResult) =>
            TaskToApm.End<int>(asyncResult);
       

        #endregion

        #region SendTo

        //
        // Send array segment to an endpoint on unconnected socket
        //
        public async Task<int> SendToAsync(ArraySegment<byte> buffer, SocketAddress endpoint, CancellationToken ct) {
            if (_cleanedUp) {
                throw new ObjectDisposedException(this.GetType().FullName);
            }
            if (buffer == null) {
                throw new ArgumentNullException("buffer");
            }
            if (endpoint == null) {
                throw new ArgumentNullException("endpoint");
            }
            if (!IsBound) {
                throw new SocketException("Not bound");
            }
            try {
                var result = await _internal.SendAsync(buffer, endpoint, ct).ConfigureAwait(false); 
                ct.ThrowIfCancellationRequested();
                return result;
            }
            catch (OperationCanceledException) {
                throw;
            }
            catch (Exception e) {
                throw SocketException.Create("Exception during SendTo", e);
            }
        }

        //
        // Send array segment, without cancellation token
        //
        public async Task<int> SendToAsync(ArraySegment<byte> buffer, SocketAddress endpoint) {
            var cts = new CancellationTokenSource(SendTimeout);
            try {
                return await SendToAsync(buffer, endpoint, cts.Token).ConfigureAwait(false);
            }
            catch (OperationCanceledException) when (cts.IsCancellationRequested) {
                throw new TimeoutException(
                    $"Timeout on sendto after {SendTimeout.ToString()}.");
            }
        }

        //
        // Send buffer, with cancellation
        //
        public Task<int> SendToAsync(byte[] buffer, int offset, int size, SocketAddress endpoint, CancellationToken ct) =>
            SendToAsync(CreateArraySegment(buffer, offset, size), endpoint, ct);

        //
        // Send buffer without cancellation token
        //
        public Task<int> SendToAsync(byte[] buffer, int offset, int size, SocketAddress endpoint) =>
            SendToAsync(CreateArraySegment(buffer, offset, size), endpoint);

        //
        // Send buffer of size
        //
        public Task<int> SendToAsync(byte[] buffer, int size, SocketAddress endpoint) =>
            SendToAsync(buffer, 0, size, endpoint);

        //
        // Same, without offset
        //
        public Task<int> SendToAsync(byte[] buffer, SocketAddress endpoint) =>
            SendToAsync(buffer, buffer != null ? buffer.Length : 0, endpoint);

        //
        // Sends a data buffer un unconnected socket with up to size bytes to endpoint
        //
        public async Task<int> SendToAsync(IEnumerable<ArraySegment<byte>> buffers, SocketAddress endpoint, CancellationToken ct) {
            if (_cleanedUp) {
                throw new ObjectDisposedException(this.GetType().FullName);
            }
            if (buffers == null) {
                throw new ArgumentNullException("buffers");
            }

            int sent = 0;
            foreach (var buffer in buffers) {
                sent += await SendToAsync(buffer, endpoint, ct).ConfigureAwait(false);
            }
            return sent;
        }

        //
        // Without cancellation token
        //
        public async Task<int> SendToAsync(IEnumerable<ArraySegment<byte>> buffers, SocketAddress endpoint) {
            var cts = new CancellationTokenSource(SendTimeout);
            try {
                return await SendToAsync(buffers, endpoint, cts.Token).ConfigureAwait(false);
            }
            catch (OperationCanceledException) when (cts.IsCancellationRequested) {
                throw new TimeoutException(
                    $"Timeout on sendto after {SendTimeout.ToString()}.");
            }
        }

        //
        // Same, this time with event arg
        //
        public bool SendToAsync(SocketAsyncEventArgs e) {
            Task<int> t1;
            if (e.Buffer != null) {
                t1 = SendToAsync(CreateArraySegment(e.Buffer, e.Offset, e.Count), e.Endpoint);
            }
            else if (e.BufferList != null) {
                t1 = SendToAsync(e.BufferList, e.Endpoint);
            }
            else {
                throw new ArgumentException("Buffer");
            }
            Task t2 = t1.ContinueWith(t => {
                try { 
                    e.BytesTransferred = t.Result;
                    e.SocketError = SocketError.Success;
                }
                catch (Exception ex) {
                    e.SocketError = ex.GetSocketError();
                }
                e.Complete(this);
            });
            return true;
        }

        //
        // Send to named host and port on unconnected socket
        //
        public Task<int> SendToAsync(ArraySegment<byte> buffer, string host, int port, CancellationToken ct) =>
            SendToAsync(buffer, new ProxySocketAddress(host, port), ct);

        //
        // Send to named host and port on unconnected socket
        //
        public Task<int> SendToAsync(ArraySegment<byte> buffer, string host, int port) =>
            SendToAsync(buffer, new ProxySocketAddress(host, port));

        //
        // With real buffer
        //
        public Task<int> SendToAsync(byte[] buffer, int offset, int size, string host, int port) =>
            SendToAsync(buffer, offset, size, new ProxySocketAddress(host, port));

        //
        // Sync send a data buffer to a connected socket with up to size bytes
        //
        public int SendTo(ArraySegment<byte> buffer, SocketAddress endpoint) =>
            TaskToSync.Run(() => SendToAsync(buffer, endpoint));

        //
        // Sends a data buffer to a connected socket with up to size bytes
        //
        public int SendTo(IEnumerable<ArraySegment<byte>> buffer, SocketAddress endpoint) =>
            TaskToSync.Run(() => SendToAsync(buffer, endpoint));

        //
        // Sends a data buffer from offset to a connected socket with up to size bytes
        //
        public int SendTo(byte[] buffer, int offset, int size, SocketAddress endpoint) =>
            SendTo(CreateArraySegment(buffer, offset, size), endpoint);

        //
        // Sends a data buffer to a connected socket with up to size bytes
        //
        public int SendTo(byte[] buffer, int size, SocketAddress endpoint) =>
            SendTo(buffer, 0, size, endpoint);

        //
        // Sends a data buffer to a connected socket.
        //
        public int SendTo(byte[] buffer, SocketAddress endpoint) =>
            SendTo(buffer, buffer != null ? buffer.Length : 0, endpoint);

        //
        // Sends data to a specific end point, with offset and size
        //
        public int SendTo(ArraySegment<byte> buffer, string host, int port) =>
            SendTo(buffer, new ProxySocketAddress(host, port));

        //
        // Sends data to a specific end point with size and offset
        //
        public int SendTo(byte[] buffer, int offset, int size, string host, int port) =>
            SendTo(buffer, offset, size, new ProxySocketAddress(host, port));

        //
        // Sends data to a specific end point with size, starting offset 0
        //
        public int SendTo(byte[] buffer, int size, string host, int port) =>
            SendTo(buffer, size, new ProxySocketAddress(host, port));

        //
        // Send buffer to endpoint
        //
        public int SendTo(byte[] buffer, string host, int port) =>
            SendTo(buffer, new ProxySocketAddress(host, port));

        //
        // Begin send call with buffer vector
        //
        public IAsyncResult BeginSendTo(IEnumerable<ArraySegment<byte>> buffers,
            SocketAddress endpoint, AsyncCallback callback, object state) =>
            TaskToApm.Begin(SendToAsync(buffers, endpoint), callback, state);

        //
        // Begin send call
        //
        public IAsyncResult BeginSendTo(byte[] buffer, int offset, int size, 
            SocketAddress endpoint, AsyncCallback callback, object state) =>
            TaskToApm.Begin(SendToAsync(buffer, offset, size, endpoint), callback, state);

        //
        // Begin send to operation
        //
        public IAsyncResult BeginSendTo(byte[] buffer, int offset, int size, 
            string host, int port, AsyncCallback callback, object state) {
            return TaskToApm.Begin(SendToAsync(buffer, offset, size, host, port), callback, state);
        }

        //
        // Complete send
        //
        public int EndSendTo(IAsyncResult asyncResult) =>
            TaskToApm.End<int>(asyncResult);

        #endregion

        #region Receive

        //
        // Gets the amount of data that can be read from input
        //
        public int Available {
            get {
                return (int)GetSocketOption(SocketOption.Available);
            }
        }

        //
        //  Set/get the timeout for receive calls
        //
        public int ReceiveTimeout { get; set; } = 20 * 1000;

        //
        // Receives data from a connected socket 
        //
        public async Task<int> ReceiveAsync(ArraySegment<byte> buffer, CancellationToken ct) {
            if (_cleanedUp) {
                throw new ObjectDisposedException(this.GetType().FullName);
            }
            if (buffer == null) {
                throw new ArgumentNullException("buffer");
            }
            try {
                var result = await _internal.ReceiveAsync(buffer, ct).ConfigureAwait(false);
                ct.ThrowIfCancellationRequested();
                return result.Count;
            }
            catch (OperationCanceledException) {
                throw;
            }
            catch (Exception e) {
                throw SocketException.Create("Exception during Receive", e);
            }
        }

        //
        // Without cancelation token
        //
        public async Task<int> ReceiveAsync(ArraySegment<byte> buffer) {
            var cts = new CancellationTokenSource(ReceiveTimeout);
            try {
                return await ReceiveAsync(buffer, cts.Token).ConfigureAwait(false);
            }
            catch (OperationCanceledException) when (cts.IsCancellationRequested) {
                throw new TimeoutException(
                    $"Timeout on receive after {ReceiveTimeout.ToString()}.");
            }
        }

        //
        // Receive into buffer vector
        //
        public async Task<int> ReceiveAsync(IEnumerable<ArraySegment<byte>> buffers, CancellationToken ct) {
            if (_cleanedUp) {
                throw new ObjectDisposedException(this.GetType().FullName);
            }
            if (buffers == null) {
                throw new ArgumentNullException("buffers");
            }
            if (!Connected) {
                throw new SocketException("Not connected");
            }
            int received = 0;
            foreach (var buffer in buffers) {
                received += await ReceiveAsync(buffer, ct).ConfigureAwait(false);
            }
            return received;
        }

        //
        // Receive into buffer vector
        //
        public async Task<int> ReceiveAsync(IEnumerable<ArraySegment<byte>> buffers) {
            var cts = new CancellationTokenSource(ReceiveTimeout);
            try {
                return await ReceiveAsync(buffers, cts.Token).ConfigureAwait(false);
            }
            catch (OperationCanceledException) when (cts.IsCancellationRequested) {
                throw new TimeoutException(
                    $"Timeout on receive after {ReceiveTimeout.ToString()}.");
            }
        }

        //
        // Into buffer
        //
        public Task<int> ReceiveAsync(byte[] buffer, int offset, int size, CancellationToken ct) =>
            ReceiveAsync(CreateArraySegment(buffer, offset, size), ct);

        //
        // Into buffer without cancellation token
        //
        public Task<int> ReceiveAsync(byte[] buffer, int offset, int size) =>
            ReceiveAsync(CreateArraySegment(buffer, offset, size));

        //
        // No offset
        //
        public Task<int> ReceiveAsync(byte[] buffer, int size) =>
            ReceiveAsync(buffer, 0, size);

        //
        // Just buffer
        //
        public Task<int> ReceiveAsync(byte[] buffer) =>
            ReceiveAsync(buffer, buffer != null ? buffer.Length : 0);

        //
        // Same, this time with event arg
        //
        public bool ReceiveAsync(SocketAsyncEventArgs e) {
            Task<int> t1;
            if (e.Buffer != null) {
                t1 = ReceiveAsync(CreateArraySegment(e.Buffer, e.Offset, e.Count));
            }
            else if (e.BufferList != null) {
                t1 = ReceiveAsync(e.BufferList);
            }
            else {
                throw new ArgumentException("Buffer");
            }
            Task t2 = t1.ContinueWith(t => {
                try { 
                    e.BytesTransferred = t.Result;
                    e.SocketError = SocketError.Success;
                }
                catch (Exception ex) {
                    e.SocketError = ex.GetSocketError();
                }
                e.Complete(this);
            });
            return true;
        }

        //
        // Receives data from a connected socket 
        //
        public int Receive(ArraySegment<byte> buffer) =>
            TaskToSync.Run(() => ReceiveAsync(buffer));

        //
        // Receives data from a connected socket 
        //
        public int Receive(byte[] buffer, int offset, int size) =>
            Receive(CreateArraySegment(buffer, offset, size));

        //
        // Receives data from a connected socket.
        //
        public int Receive(byte[] buffer, int size) =>
            Receive(buffer, 0, size);

        //
        // Receives data from a connected socket.
        //
        public int Receive(byte[] buffer) =>
            Receive(buffer, buffer != null ? buffer.Length : 0);

        //
        // Receive into list of buffers
        //
        public int Receive(IEnumerable<ArraySegment<byte>> buffers) =>
            TaskToSync.Run(() => ReceiveAsync(buffers));

        //
        // Begin receive of vector of buffers
        //
        public IAsyncResult BeginReceive(IList<ArraySegment<byte>> buffers, AsyncCallback callback, object state) {
            return TaskToApm.Begin(ReceiveAsync(buffers), callback, state);
        }

        //
        // Begin receive of buffer
        //
        public IAsyncResult BeginReceive(byte[] buffer, int offset, int size, AsyncCallback callback, object state) {
            return TaskToApm.Begin(ReceiveAsync(buffer, offset, size), callback, state);
        }

        //
        // Complete receive
        //
        public int EndReceive(IAsyncResult asyncResult) {
            return TaskToApm.End<int>(asyncResult);
        }

        #endregion

        #region ReceiveFrom

        //
        // Receives data from an unconnected socket 
        //
        public async Task<Tuple<SocketAddress, int>> ReceiveFromAsync(ArraySegment<byte> buffer, CancellationToken ct) {
            if (_cleanedUp) {
                throw new ObjectDisposedException(this.GetType().FullName);
            }
            if (buffer == null) {
                throw new ArgumentNullException("buffer");
            }
            if (!IsBound) {
                throw new SocketException("Not bound");
            }
            try {
                var result = await _internal.ReceiveAsync(buffer, ct).ConfigureAwait(false);
                ct.ThrowIfCancellationRequested();
                return Tuple.Create(result.Address, result.Count);
            }
            catch (OperationCanceledException) {
                throw;
            }
            catch (Exception e) {
                throw SocketException.Create("Exception during ReceiveFrom", e);
            }
        }

        //
        // Without cancelation token, but timeout
        //
        public async Task<Tuple<SocketAddress, int>> ReceiveFromAsync(ArraySegment<byte> buffer) {
            var cts = new CancellationTokenSource(ReceiveTimeout);
            try {
                return await ReceiveFromAsync(buffer, cts.Token).ConfigureAwait(false);
            }
            catch (OperationCanceledException) when (cts.IsCancellationRequested) {
                throw new TimeoutException(
                    $"Timeout on receivefrom after {ReceiveTimeout.ToString()}.");
            }
        }

        //
        // Into buffer
        //
        public Task<Tuple<SocketAddress, int>> ReceiveFromAsync(byte[] buffer, int offset, int size, CancellationToken ct) =>
            ReceiveFromAsync(CreateArraySegment(buffer, offset, size), ct);

        //
        // Into buffer without cancellation token
        //
        public Task<Tuple<SocketAddress, int>> ReceiveFromAsync(byte[] buffer, int offset, int size) =>
            ReceiveFromAsync(CreateArraySegment(buffer, offset, size));

        //
        // No offset
        //
        public Task<Tuple<SocketAddress, int>> ReceiveFromAsync(byte[] buffer, int size) =>
            ReceiveFromAsync(buffer, 0, size);

        //
        // Just buffer
        //
        public Task<Tuple<SocketAddress, int>> ReceiveFromAsync(byte[] buffer) =>
            ReceiveFromAsync(buffer, buffer != null ? buffer.Length : 0);

        //
        // Same, this time with event arg
        //
        public bool ReceiveFromAsync(SocketAsyncEventArgs e) {
            Task<Tuple<SocketAddress, int>> t1;
            if (e.Buffer != null) {
                t1 = ReceiveFromAsync(CreateArraySegment(e.Buffer, e.Offset, e.Count));
            }
            else if (e.BufferList != null) {
                throw new NotSupportedException("buffer list not supported with receivefrom");
            }
            else {
                throw new ArgumentException("Buffer");
            }
            Task t2 = t1.ContinueWith(t => {
                try {
                    e.Endpoint = t.Result.Item1;
                    e.BytesTransferred = t.Result.Item2;
                    e.SocketError = SocketError.Success;
                }
                catch (Exception ex) {
                    e.SocketError = ex.GetSocketError();
                }
                e.Complete(this);
            });
            return true;
        }

        //
        // Receives data from an unconnected socket 
        //
        public int ReceiveFrom(ArraySegment<byte> buffer, ref SocketAddress address) {
            var result = TaskToSync.Run(() => ReceiveFromAsync(buffer));
            address = result.Item1;
            return result.Item2;
        }

        //
        // Direct buffer
        //
        public int ReceiveFrom(byte[] buffer, int offset, int size, ref SocketAddress address) =>
            ReceiveFrom(CreateArraySegment(buffer, offset, size), ref address);

        //
        // no offset
        //
        public int ReceiveFrom(byte[] buffer, int size, ref SocketAddress address) =>
            ReceiveFrom(buffer, 0, size, ref address);

        //
        // Just buffer
        //
        public int ReceiveFrom(byte[] buffer, ref SocketAddress address) =>
            ReceiveFrom(buffer, buffer != null ? buffer.Length : 0, ref address);

        //
        // Begin receive of buffer into an unconnected socket
        //
        public IAsyncResult BeginReceiveFrom(byte[] buffer, int offset, int size, AsyncCallback callback, object state) {
            return TaskToApm.Begin(ReceiveFromAsync(buffer, offset, size), callback, state);
        }

        //
        // Complete receive
        //
        public int EndReceiveFrom(IAsyncResult asyncResult) {
            return TaskToApm.End<int>(asyncResult);
        }

        #endregion

        #region Options

        //
        // Enable broadcast moode
        //
        public bool EnableBroadcast {
            get {
                return GetSocketOption(SocketOption.Broadcast) != 0 ? true : false;
            }
            set {
                SetSocketOption(SocketOption.Broadcast, value ? 1UL : 0);
            }
        }

        //
        // Sets the specified option to the specified value.
        //
        public async Task SetSocketOptionAsync(SocketOption option, ulong value, CancellationToken ct) {
            if (_cleanedUp) {
                throw new ObjectDisposedException(this.GetType().FullName);
            }
            await _internal.SetSocketOptionAsync(option, value, ct).ConfigureAwait(false);
        }

        //
        // Without cancellation token
        //
        public async Task SetSocketOptionAsync(SocketOption option, ulong value) {
            var cts = new CancellationTokenSource(SendTimeout);
            try {
                await SetSocketOptionAsync(option, value, cts.Token).ConfigureAwait(false);
            }
            catch (OperationCanceledException) when (cts.IsCancellationRequested) {
                throw new TimeoutException(
                    $"Timeout on send socket option after {SendTimeout.ToString()}.");
            }
        }

        //
        // Sets the specified option to the specified value.
        //
        public void SetSocketOption(SocketOption option, ulong value) =>
            TaskToSync.Run(() => SetSocketOptionAsync(option, value));

        //
        // Begin setting socket options
        //
        public IAsyncResult BeginSetSocketOption(SocketOption option, ulong value, AsyncCallback callback, object state) {
            return TaskToApm.Begin(SetSocketOptionAsync(option, value), callback, state);
        }

        //
        // End set socket option call
        //
        public void EndSetSocketOption(IAsyncResult asyncResult) {
            TaskToApm.End(asyncResult);
        }

        //
        // Helper to get the value of a socket option.
        //
        public async Task<ulong> GetSocketOptionAsync(SocketOption option, CancellationToken ct) {
            if (_cleanedUp) {
                throw new ObjectDisposedException(this.GetType().FullName);
            }
            return await _internal.GetSocketOptionAsync(option, ct).ConfigureAwait(false);
        }

        //
        // Same without cancellation token
        //
        public async Task<ulong> GetSocketOptionAsync(SocketOption option) {
            var cts = new CancellationTokenSource(ReceiveTimeout);
            try {
                return await GetSocketOptionAsync(option, cts.Token).ConfigureAwait(false);
            }
            catch (OperationCanceledException) when (cts.IsCancellationRequested) {
                throw new TimeoutException(
                    $"Timeout on receive socket option after {ReceiveTimeout.ToString()}.");
            }
        }

        //
        // Helper to get the value of a socket option.
        //
        public ulong GetSocketOption(SocketOption option) =>
            TaskToSync.Run(() => GetSocketOptionAsync(option));

        //
        // Begin getting socket options
        //
        public IAsyncResult BeginGetSocketOption(SocketOption option, AsyncCallback callback, object state) =>
            TaskToApm.Begin(GetSocketOptionAsync(option), callback, state);

        //
        // End get socket option call
        //
        public int EndGetSocketOption(IAsyncResult asyncResult) =>
            TaskToApm.End<int>(asyncResult);


        #endregion

        #region Shutdown

        //
        // Disables sends and receives on a socket.
        //
        public Task ShutdownAsync(SocketShutdown how, CancellationToken ct) =>
            SetSocketOptionAsync(SocketOption.Shutdown, (ulong)how, ct);

        //
        // Disables sends and receives on a socket.
        //
        public Task ShutdownAsync(SocketShutdown how) =>
            SetSocketOptionAsync(SocketOption.Shutdown, (ulong)how);

        //
        // Same, but sync
        //
        public void Shutdown(SocketShutdown how) =>
            SetSocketOption(SocketOption.Shutdown, (ulong)how);

        //
        // Begin Shutdown
        //
        public IAsyncResult BeginShutdown(SocketShutdown how, AsyncCallback callback, object state) =>
            TaskToApm.Begin(ShutdownAsync(how), callback, state);

        //
        // Complete shutdown
        //
        public void EndShutdown(IAsyncResult asyncResult) =>
            TaskToApm.End(asyncResult);

        #endregion

        #region Close

        //
        // Closes the socket
        //
        public async Task CloseAsync(CancellationToken ct) {
            if (_cleanedUp) {
                throw new ObjectDisposedException(this.GetType().FullName);
            }
            if (!Connected) {
                return;
            }
            Connected = false;
            await _internal.CloseAsync(ct).ConfigureAwait(false);
        }

        //
        // Without cancellation token
        //
        public async Task CloseAsync() {
            if (_cleanedUp) {
                throw new ObjectDisposedException(this.GetType().FullName);
            }
            if (!Connected) {
                return;
            }

            var cts = new CancellationTokenSource(SendTimeout);
            try {
                await CloseAsync(cts.Token).ConfigureAwait(false);
            }
            catch (OperationCanceledException) when (cts.IsCancellationRequested) {
                throw new TimeoutException(
                    $"Timeout on close socket after {SendTimeout.ToString()}.");
            }
        }

        //
        // Sync close
        //
        public void Close() =>
            TaskToSync.Run(() => CloseAsync());

        //
        // Begin Close
        //
        public IAsyncResult BeginClose(AsyncCallback callback, object state) {
            return TaskToApm.Begin(CloseAsync(), callback, state);
        }

        //
        // Complete closing
        //
        public void EndClose(IAsyncResult asyncResult) {
            TaskToApm.End(asyncResult);
        }

        #endregion

        /// <summary>
        /// Convert to string
        /// </summary>
        /// <returns></returns>
        public override string ToString() {
            return _internal.Info.ToString();
        }

        #region internal

        //
        // Close and dispose of the socket
        //
        protected virtual void Dispose(bool disposing) {
            if (!disposing) {
                return;
            }
            if (_cleanedUp) {
                return;
            }
            lock (this) {
                if (!_cleanedUp) {
                    _internal?.Dispose();
                    _internal = null;

                    _cleanedUp = true;
                }
            }
        }

        //
        // Dispose
        //
        public void Dispose() {
            Dispose(true);
        }

        //
        // Helper to create array segment
        //
        private static ArraySegment<byte> CreateArraySegment(byte[] buffer, int offset, int size) {
            if (buffer == null) {
                throw new ArgumentNullException("buffer");
            }
            if (offset < 0 || offset > buffer.Length) {
                throw new ArgumentOutOfRangeException("offset");
            }
            if (size < 0 || size > buffer.Length - offset) {
                throw new ArgumentOutOfRangeException("size");
            }
            return new ArraySegment<byte>(buffer, offset, size);
        }

        #endregion
    }
}