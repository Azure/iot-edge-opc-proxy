// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Threading.Tasks;

    public class TcpListener {

        //
        // Initializes a new instance of the TcpListener class that listens on proxy
        // 
        public TcpListener(SocketAddress endpoint) {
            Endpoint = endpoint;
            Active = false;
            Server = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
        }

        //
        // Proxy Endpoint 
        //
        public SocketAddress Endpoint {
            get; private set;
        }

        // 
        // Server socket
        //
        public Socket Server {
            get; private set;
        }

        //
        // True when the listener's socket has been bound to a port and is listening
        // 
        protected bool Active {
            get; private set;
        }

        //
        // Starts listening to network requests.
        //
        public void Start() {
            if (Active) {
                return;
            }

            Server.Bind(Endpoint);
            try {
                Server.Listen();
            }
            catch (SocketException) {
                // When there is an exception, unwind previous actions (bind, etc).
                Stop();
                throw;
            }

            Active = true;
        }

        //
        // Closes the network connection.
        //
        public void Stop() {
            if (Server != null) {
                Server.Close();
                Server = null;
            }
            Active = false;
            Server = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
        }

        //
        // Async accept operation
        //
        public Task<TcpClient> AcceptTcpClientAsync() =>
            Server.AcceptAsync().ContinueWith(t => new TcpClient(t.Result));
        
        //
        // Begin accept
        //
        public IAsyncResult BeginAcceptTcpClient(AsyncCallback callback, object state) =>
            TaskToApm.Begin(AcceptTcpClientAsync(), callback, state);

        //
        // Complete accept
        //
        public static TcpClient EndAcceptTcpClient(IAsyncResult asyncResult) =>
            TaskToApm.End<TcpClient>(asyncResult);
    }
}
