// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.IO;
    using System.Runtime.InteropServices;
    using System.Threading;
    using System.Threading.Tasks;

    //
    // Provides the underlying stream for TCP Client network access
    //
    public class NetworkStream : Stream {

        private volatile bool _cleanedUp = false;
        private bool _ownsSocket = false;

        //
        // Create stream with socket, and declare socket ownership
        //
        public NetworkStream(Socket socket, bool ownsSocket) {
            if (socket == null) {
                throw new ArgumentNullException(nameof(socket));
            }
            if (!socket.Connected) {
                throw new IOException("Socket not connected");
            }
            if (socket.SocketType != SocketType.Stream) {
                throw new IOException("Not a stream socket");
            }

            Socket = socket;
            _ownsSocket = ownsSocket;

            Readable = true;
            Writeable = true;
        }

        //
        // Creates a new NetworkStream class
        //
        public NetworkStream(Socket socket) : 
            this(socket, false) {
        }

        //
        // Returns the underlying socket 
        //
        public Socket Socket {
            get; private set;
        }

        //
        // Used by the class to indicate that the stream is readable.
        //
        protected bool Readable {
            get; set;
        }

        //
        // Used by the class to indicate that the stream is writable.
        //
        protected bool Writeable {
            get; set;
        }

        //
        // Indicates that data can be read from the stream.
        //
        public override bool CanRead {
            get { return Readable; }
        }

        //
        // Cannot seek
        //
        public override bool CanSeek {
            get { return false; }
        }

        //
        // Indicates that data can be written to the stream.
        //
        public override bool CanWrite {
            get { return Writeable; }
        }

        //
        // Can always timeout
        //
        public override bool CanTimeout {
            get { return true; }
        }

        // 
        // Returns the read timeout
        //
        public override int ReadTimeout {
            get {
                return Socket.ReceiveTimeout;
            }
            set {
                Socket.ReceiveTimeout = value;
            }
        }

        // 
        // Returns the write timeout 
        //
        public override int WriteTimeout {
            get {
                return Socket.SendTimeout;
            }
            set {
                Socket.SendTimeout = value;
            }
        }

        //
        // Indicates data is available on the stream to be read.
        // 
        public virtual bool DataAvailable {
            get {
                if (_cleanedUp) {
                    throw new ObjectDisposedException(this.GetType().FullName);
                }

                Socket chkStreamSocket = Socket;
                if (chkStreamSocket == null) {
                    throw new IOException("Connection closed");
                }
                return chkStreamSocket.Available != 0;
            }
        }

        //
        // Indicates whether the stream is still connected
        //
        internal bool Connected {
            get {
                Socket socket = Socket;
                if (!_cleanedUp && socket != null && socket.Connected) {
                    return true;
                }
                else {
                    return false;
                }
            }
        }

        //
        // Read - provide core Read functionality.
        //
        public override int Read([In, Out] byte[] buffer, int offset, int size) {
            bool canRead = CanRead;  // Prevent race with Dispose.
            if (_cleanedUp) {
                throw new ObjectDisposedException(this.GetType().FullName);
            }
            if (!canRead) {
                throw new InvalidOperationException("Cannot read on writeonly stream");
            }
            if (buffer == null) {
                throw new ArgumentNullException("buffer");
            }
            if (offset < 0 || offset > buffer.Length) {
                throw new ArgumentOutOfRangeException("offset");
            }
            if (size < 0 || size > buffer.Length - offset) {
                throw new ArgumentOutOfRangeException("size");
            }

            Socket chkStreamSocket = Socket;
            if (chkStreamSocket == null) {
                throw new IOException("connection closed");
            }
            try {
                int bytesTransferred = chkStreamSocket.Receive(buffer, offset, size);
                return bytesTransferred;
            }
            catch (Exception exception) {
                if (exception is OutOfMemoryException) {
                    throw;
                }
                throw new IOException("Failed to receive", exception);
            }
        }

        //
        // Write - provide core Write functionality.
        // 
        public override void Write(byte[] buffer, int offset, int size) {
            bool canWrite = CanWrite; // Prevent race with Dispose.
            if (_cleanedUp) {
                throw new ObjectDisposedException(this.GetType().FullName);
            }
            if (!canWrite) {
                throw new InvalidOperationException("Cannot write on readonly stream");
            }
            if (buffer == null) {
                throw new ArgumentNullException("buffer");
            }
            if (offset < 0 || offset > buffer.Length) {
                throw new ArgumentOutOfRangeException("offset");
            }
            if (size < 0 || size > buffer.Length - offset) {
                throw new ArgumentOutOfRangeException("size");
            }

            Socket chkStreamSocket = Socket;
            if (chkStreamSocket == null) {
                throw new IOException("connection closed");
            }
            try {
                chkStreamSocket.Send(buffer, offset, size);
            }
            catch (Exception exception) {
                if (exception is OutOfMemoryException) {
                    throw;
                }
                throw new IOException("Failed to send", exception);
            }
        }

#if NET45 || NET46
        //
        // BeginRead - provide async read functionality.
        // 
        public override IAsyncResult BeginRead(byte[] buffer, int offset, int size, AsyncCallback callback, Object state) {
            bool canRead = CanRead; // Prevent race with Dispose.
            if (_cleanedUp) {
                throw new ObjectDisposedException(this.GetType().FullName);
            }
            if (!canRead) {
                throw new InvalidOperationException("Cannot read on writeonly stream");
            }
            if (buffer == null) {
                throw new ArgumentNullException("buffer");
            }
            if (offset < 0 || offset > buffer.Length) {
                throw new ArgumentOutOfRangeException("offset");
            }
            if (size < 0 || size > buffer.Length - offset) {
                throw new ArgumentOutOfRangeException("size");
            }

            Socket chkStreamSocket = Socket;
            if (chkStreamSocket == null) {
                throw new IOException("connection closed");
            }
            try {
                IAsyncResult asyncResult =
                    chkStreamSocket.BeginReceive(buffer, offset, size, callback, state);
                return asyncResult;
            }
            catch (Exception exception) {
                if (exception is OutOfMemoryException) {
                    throw;
                }
                throw new IOException("Failed to receive", exception);
            }
        }

        //
        // EndRead - handle the end of an async read.
        // 
        public override int EndRead(IAsyncResult asyncResult) {
            if (_cleanedUp) {
                throw new ObjectDisposedException(this.GetType().FullName);
            }
            if (asyncResult == null) {
                throw new ArgumentNullException("asyncResult");
            }

            Socket chkStreamSocket = Socket;
            if (chkStreamSocket == null) {
                throw new IOException("connection closed");
            }
            try {
                int bytesTransferred = chkStreamSocket.EndReceive(asyncResult);
                return bytesTransferred;
            }
            catch (Exception exception) {
                if (exception is OutOfMemoryException) {
                    throw;
                }
                throw new IOException("Failed to receive", exception);
            }
        }
#endif
        //
        // ReadAsync - provide async read functionality.
        // 
        public override Task<int> ReadAsync(byte[] buffer, int offset, int size, CancellationToken ct) {
            bool canRead = CanRead;  // Prevent race with Dispose.
            if (_cleanedUp) {
                throw new ObjectDisposedException(this.GetType().FullName);
            }
            if (!canRead) {
                throw new InvalidOperationException("Cannot read on writeonly stream");
            }
            if (buffer == null) {
                throw new ArgumentNullException("buffer");
            }
            if (offset < 0 || offset > buffer.Length) {
                throw new ArgumentOutOfRangeException("offset");
            }
            if (size < 0 || size > buffer.Length - offset) {
                throw new ArgumentOutOfRangeException("size");
            }

            Socket chkStreamSocket = Socket;
            if (chkStreamSocket == null) {
                throw new IOException("connection closed");
            }
            return chkStreamSocket.ReceiveAsync(buffer, offset, size, ct).ContinueWith(t => {
                if (t.IsFaulted) {
                    throw new IOException("Failed to receive", t.Exception);
                }
                return t.Result;
            });
        }

        //
        // ReadAsync - provide async read functionality.
        // 
        public Task<int> ReadAsync(byte[] buffer) =>
            ReadAsync(buffer, 0, buffer.Length, CancellationToken.None);

#if NET45 || NET46
        //
        // BeginWrite - provide async write functionality.
        // 
        public override IAsyncResult BeginWrite(byte[] buffer, int offset, int size, AsyncCallback callback, Object state) {
            bool canWrite = CanWrite; // Prevent race with Dispose.
            if (_cleanedUp) {
                throw new ObjectDisposedException(this.GetType().FullName);
            }
            if (!canWrite) {
                throw new InvalidOperationException("Cannot write on readonly stream");
            }
            if (buffer == null) {
                throw new ArgumentNullException("buffer");
            }
            if (offset < 0 || offset > buffer.Length) {
                throw new ArgumentOutOfRangeException("offset");
            }
            if (size < 0 || size > buffer.Length - offset) {
                throw new ArgumentOutOfRangeException("size");
            }

            Socket chkStreamSocket = Socket;
            if (chkStreamSocket == null) {
                throw new IOException("connection closed");
            }
            try {
                // Call BeginSend on the Socket.
                IAsyncResult asyncResult =
                    chkStreamSocket.BeginSend(buffer, offset, size, callback, state);

                return asyncResult;
            }
            catch (Exception exception) {
                if (exception is OutOfMemoryException) {
                    throw;
                }
                throw new IOException("Failed to send", exception);
            }
        }

        // 
        // Complete write
        //
        public override void EndWrite(IAsyncResult asyncResult) {
            if (_cleanedUp) {
                throw new ObjectDisposedException(this.GetType().FullName);
            }
            if (asyncResult == null) {
                throw new ArgumentNullException("asyncResult");
            }

            Socket chkStreamSocket = Socket;
            if (chkStreamSocket == null) {
                throw new IOException("connection closed");
            }
            try {
                chkStreamSocket.EndSend(asyncResult);
            }
            catch (Exception exception) {
                if (exception is OutOfMemoryException) {
                    throw;
                }
                throw new IOException("Failed to send", exception);
            }
        }
#endif

        //
        // WriteAsync - provide async write functionality.
        //
        public override Task WriteAsync(byte[] buffer, int offset, int size, CancellationToken ct) {
            bool canWrite = CanWrite; // Prevent race with Dispose.
            if (_cleanedUp) {
                throw new ObjectDisposedException(this.GetType().FullName);
            }
            if (!canWrite) {
                throw new InvalidOperationException("Cannot write on readonly stream");
            }
            if (buffer == null) {
                throw new ArgumentNullException("buffer");
            }
            if (offset < 0 || offset > buffer.Length) {
                throw new ArgumentOutOfRangeException("offset");
            }
            if (size < 0 || size > buffer.Length - offset) {
                throw new ArgumentOutOfRangeException("size");
            }

            Socket chkStreamSocket = Socket;
            if (chkStreamSocket == null) {
                throw new IOException("connection closed");
            }
            return chkStreamSocket.SendAsync(buffer, offset, size, ct).ContinueWith(t => {
                if (t.IsFaulted) {
                    throw new IOException("Failed to send", t.Exception);
                }
                return t;
            });
        }

        //
        // WriteAsync with just buffer
        // 
        public Task WriteAsync(byte[] buffer) =>
            WriteAsync(buffer, 0, buffer.Length, CancellationToken.None);

        //
        // Flushes data from the stream.
        //
        public override void Flush() {
        }

        //
        // Same no op but async
        //
        public override Task FlushAsync(CancellationToken cancellationToken) {
            return (Task)Task.FromResult(false);
        }

        //
        // Always throws NotSupportedException.
        //
        public override long Length {
            get {
                throw new NotSupportedException("seek not supported on network streams");
            }
        }

        //
        // Always throws NotSupportedException
        //
        public override void SetLength(long value) {
            throw new NotSupportedException("seek not supported on network streams");
        }

        //
        // Always throws NotSupportedException.
        //
        public override long Position {
            get {
                return Length;
            }
            set {
                SetLength(value);
            }
        }

        //
        // Always throws NotSupportedException.
        //
        public override long Seek(long offset, SeekOrigin origin) {
            return Length;
        }

        #region internal
        //
        // Disposes the Network stream.
        //
        protected override void Dispose(bool disposing) {
            // Mark this as disposed before changing anything else.
            bool cleanedUp = _cleanedUp;
            _cleanedUp = true;
            if (!cleanedUp && disposing) {
                Socket chkStreamSocket = Socket;
                Socket = null;
                Readable = false;
                Writeable = false;

                if (chkStreamSocket != null) {
                    if (_ownsSocket) {
                        chkStreamSocket.Dispose();
                        chkStreamSocket = null;
                    }
                }
            }
            base.Dispose(disposing);
        }

        ~NetworkStream() {
            Dispose(false);
        }
        #endregion
    }
}
