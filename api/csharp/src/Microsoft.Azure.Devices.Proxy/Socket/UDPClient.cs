// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Threading.Tasks;

    public class UdpClient : IDisposable {
        private bool _cleanedUp;
        private readonly byte[] _buffer = new byte[0x4000];

        //
        // Initializes a new instance of the System.Net.Sockets.UdpClientclass.
        //
        public UdpClient(string host, int port) {
            Socket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
            Socket.Bind(host, port);
        }

        // Used by the class to provide the underlying network socket.
        public Socket Socket {
            get; private set;
        }

        //
        // Socket connected.
        //
        public bool Active {
            get; private set;
        }

        //
        // Returns how many bytes are available to read
        //
        public int Available => Socket.Available;

        //
        // Make the underlying socket a broadcast socket
        //
        public bool EnableBroadcast {
            get {
                return Socket.EnableBroadcast;
            }
            set {
                Socket.EnableBroadcast = value;
            }
        }

        //
        // Semd async on unconnected socket
        //
        public Task<int> SendAsync(byte[] datagram, int bytes) =>
            Socket.SendAsync(datagram, bytes);

        //
        // Send async on unconnected socket to host
        //
        public Task<int> SendAsync(byte[] datagram, int bytes, SocketAddress endpoint) =>
            Socket.SendToAsync(datagram, bytes, endpoint);

        //
        // Begin send on connected socket
        //
        public IAsyncResult BeginSend(byte[] datagram, int bytes, AsyncCallback requestCallback, object state) =>
            TaskToApm.Begin(SendAsync(datagram, bytes), requestCallback, state);

        //
        // Begin sending on unconnected socket to named host
        //
        public IAsyncResult BeginSend(byte[] datagram, int bytes, SocketAddress endpoint, AsyncCallback requestCallback, object state) =>
            TaskToApm.Begin(SendAsync(datagram, bytes, endpoint), requestCallback, state);

        //
        // Complete sending
        //
        public int EndSend(IAsyncResult asyncResult) {
            return TaskToApm.End<int>(asyncResult);
        }

        //
        // Receive on connected or unconnected socket async
        //
        public Task<UdpReceiveResult> ReceiveAsync() {
            return Socket.ReceiveFromAsync(_buffer).ContinueWith(
                t => new UdpReceiveResult(Socket, _buffer, t.Result.Item2, t.Result.Item1));
        }

        //
        // Begin receive on socket
        //
        public IAsyncResult BeginReceive(AsyncCallback requestCallback, object state) {
            return TaskToApm.Begin(ReceiveAsync(), requestCallback, state);
        }

        //
        // Complete receive
        //
        public byte[] EndReceive(IAsyncResult asyncResult) {
            UdpReceiveResult result = TaskToApm.End<UdpReceiveResult>(asyncResult);
            return result.Buffer;
        }

        #region internal
        //
        // Dispose
        //
        protected virtual void Dispose(bool disposing) {
            if (disposing) {
                if (_cleanedUp) {
                    return;
                }
                Socket chkClientSocket = Socket;
                Socket = null;
                if (chkClientSocket != null) {
                    chkClientSocket.Dispose();
                    chkClientSocket = null;
                }
                _cleanedUp = true;
                GC.SuppressFinalize(this);
            }
        }

        public void Dispose() {
            Dispose(true);
        }

        ~UdpClient() {
            Dispose(false);
        }
        #endregion
    }
}
