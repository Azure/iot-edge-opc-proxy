// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Collections.Generic;

    public partial class SocketAsyncEventArgs : EventArgs, IDisposable {

        private event EventHandler<SocketAsyncEventArgs> _completed;

        //
        // Default constructor
        //
        public SocketAsyncEventArgs() {
        }

        //
        // Accepted socket
        //
        public Socket AcceptSocket {
            get; internal set;
        }

        //
        // Connected socket
        //
        public Socket ConnectSocket {
            get; internal set;
        }

        public object UserToken {
            get; set;
        }

        public byte[] Buffer {
            get; internal set;
        }

        public SocketAddress Endpoint {
            get; set;
        }

        public int Port {
            get; set;
        }

        public int Offset {
            get; internal set;
        }

        public int Count {
            get; internal set;
        }

        //
        // Set the buffer members
        //
        public void SetBuffer(byte[] buffer, int offset, int count) {
            if (buffer == null) {
                // Clear out existing buffer.
                Buffer = null;
                Offset = 0;
                Count = 0;
            }
            else {
                // Can't have both Buffer and BufferList.
                if (BufferList != null) {
                    throw new ArgumentException("Buffer List is set");
                }

                // Offset and count can't be negative and the 
                // combination must be in bounds of the array.
                if (offset < 0 || offset > buffer.Length) {
                    throw new ArgumentOutOfRangeException(nameof(offset));
                }
                if (count < 0 || count > (buffer.Length - offset)) {
                    throw new ArgumentOutOfRangeException(nameof(count));
                }

                Buffer = buffer;
                Offset = offset;
                Count = count;
            }
        }

        //
        // Buffer list to use
        //
        public IEnumerable<ArraySegment<byte>> BufferList {
            get; set;
        }

        //
        // Bytes transferred
        //
        public int BytesTransferred {
            get; internal set;
        }

        //
        // Error 
        //
        public SocketError SocketError {
            get; set;
        }

        // 
        // Completed callback
        //
        public event EventHandler<SocketAsyncEventArgs> Completed {
            add {
                _completed += value;
            }
            remove {
                _completed -= value;
            }
        }

        //
        // Complete and signal
        //
        internal void Complete(Socket socket) {
            CurrentSocket = socket;
            OnCompleted(this);
        }

        //
        // So we can do the callback
        //
        internal Socket CurrentSocket {
            get; set;
        }

        internal virtual void OnCompleted(SocketAsyncEventArgs e) {
            _completed?.Invoke(e.CurrentSocket, e);
        }

        public void Dispose() {
            // no op
        }
    }
}
