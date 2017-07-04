// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;

    public class SocketAsyncResult : IEquatable<SocketAsyncResult> {

        //
        // Endpoint
        //
        public SocketAddress Endpoint {
            get; internal set;
        }

        //
        // Owning socket
        //
        public Socket Socket {
            get; internal set;
        }

        //
        // Buffer received
        //
        public byte[] Buffer {
            get; internal set;
        }

        //
        // Constructor
        //
        protected SocketAsyncResult(Socket socket, SocketAddress endpoint) {
            Endpoint = endpoint;
            Socket = socket;
        }

        //
        // Constructor with buffer
        //
        protected SocketAsyncResult(Socket socket, byte[] buffer, int size, SocketAddress endpoint) :
            this(socket, endpoint) {
            Buffer = new byte[size];
            System.Buffer.BlockCopy(buffer, 0, Buffer, 0, size);
        }

        //
        // Hash 
        //
        public override int GetHashCode() {
            return ((Buffer != null ? Buffer.GetHashCode() : 0) * 31) ^ 
                (Endpoint != null ? Endpoint.GetHashCode() : 0);
        }

        //
        // Equality
        //
        public override bool Equals(object obj) {
            if (!(obj is SocketAsyncResult)) {
                return false;
            }
            return Equals((SocketAsyncResult)obj);
        }

        //
        // Equality
        //
        public bool Equals(SocketAsyncResult other) {
            return
                object.Equals(Buffer, other.Buffer) &&
                object.Equals(Endpoint, other.Endpoint);
        }

        //
        // Equality
        //
        public static bool operator ==(SocketAsyncResult left, SocketAsyncResult right) {
            return left.Equals(right);
        }

        //
        // Inequality
        //
        public static bool operator !=(SocketAsyncResult left, SocketAsyncResult right) {
            return !left.Equals(right);
        }
    }

    //
    // Result for udp async receive calls
    //
    public class UdpReceiveResult : SocketAsyncResult {

        //
        // Constructor for simple receive
        //
        internal UdpReceiveResult(Socket socket, byte[] buffer, int size) :
            base(socket, buffer, size, null) {
        }

        //
        // Constructor for ReceiveFrom
        //
        internal UdpReceiveResult(Socket socket, byte[] buffer, int size, SocketAddress endpoint) :
            base(socket, buffer, size, endpoint) {
        }
    }
}
