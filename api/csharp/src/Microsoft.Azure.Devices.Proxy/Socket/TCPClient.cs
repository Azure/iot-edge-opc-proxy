// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Threading;
    using System.Threading.Tasks;

    public class TcpClient : IDisposable {

        private NetworkStream _dataStream;
        private bool _cleanedUp = false;

        //
        // Provide already accepted socket, used by listener
        //
        internal TcpClient(Socket acceptedSocket)
        {
            Socket = acceptedSocket;
            Active = true;
        }

        //
        // Initializes a new instance of the System.Net.Sockets.TcpClient class.
        //
        public TcpClient()
            : this(AddressFamily.InterNetwork) {
        }

        //
        // Initializes a new instance of the System.Net.Sockets.TcpClient class.
        //
        public TcpClient(AddressFamily family) {
            // Validate parameter
            if (family != AddressFamily.InterNetwork && family != AddressFamily.InterNetworkV6) {
                throw new ArgumentException("Invalid address family");
            }

            Socket = new Socket(family, SocketType.Stream, ProtocolType.Tcp);
            Active = false;
        }

        //
        // Underlying socket
        //
        public Socket Socket {
            get; set;
        }

        //
        // Used by the class to indicate that a connection has been made.
        //
        protected bool Active {
            get; set;
        }

        //
        // How much data is available
        //
        public int Available => Socket.Available;

        //
        // Whether the socket is connected
        //
        public bool Connected => Socket.Connected;

        //
        // Connect async
        //
        public Task ConnectAsync(SocketAddress endpoint, CancellationToken ct) =>
            Socket.ConnectAsync(endpoint, ct);

        //
        // Connect async
        //
        public Task ConnectAsync(SocketAddress endpoint) =>
            Socket.ConnectAsync(endpoint);

        //
        // Connect async
        //
        public Task ConnectAsync(string host, int port, CancellationToken ct) =>
            Socket.ConnectAsync(host, port, ct);

        //
        // Connect async
        //
        public Task ConnectAsync(string host, int port) =>
            Socket.ConnectAsync(host, port);

        //
        // Connect async
        //
        public void Connect(SocketAddress endpoint) =>
            Socket.Connect(endpoint);

        //
        // Connect async
        //
        public void Connect(string host, int port) =>
            Socket.Connect(host, port);

        //
        // Begin connecting to device with specified name and port
        //
        public IAsyncResult BeginConnect(string host, int port, AsyncCallback requestCallback, object state) =>
            TaskToApm.Begin(ConnectAsync(host, port), requestCallback, state);

        //
        // Complete connection
        //
        public static void EndConnect(IAsyncResult asyncResult) =>
            TaskToApm.End(asyncResult);

        //
        // Returns a stream used to read and write data to the remote host.
        //
        public NetworkStream GetStream() {
            if (_cleanedUp) {
                throw new ObjectDisposedException(this.GetType().FullName);
            }
            if (_dataStream == null) {
                _dataStream = new NetworkStream(Socket, false);
            }
            return _dataStream;
        }

        //
        // Disposes the Tcp client.
        //
        protected virtual void Dispose(bool disposing) {
            if (_cleanedUp) {
                return;
            }
            if (disposing) {
                IDisposable dataStream = _dataStream;
                _dataStream = null;
                if (dataStream != null) {
                    dataStream.Dispose();
                }

                Socket chkClientSocket = Socket;
                Socket = null;
                if (chkClientSocket != null) {
                    chkClientSocket.Dispose();
                    chkClientSocket = null;
                }
                GC.SuppressFinalize(this);
                _cleanedUp = true;
            }
        }

        public void Dispose() {
            Dispose(true);
        }

        ~TcpClient() {
            Dispose(false);
        }
    }
}
