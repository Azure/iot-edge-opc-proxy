// Copyright (c) 1996-2016, OPC Foundation. All rights reserved.
// The source code in this file is covered under a dual-license scenario:
//   - RCL: for OPC Foundation members in good-standing
//   - GPL V2: everybody else
// RCL license terms accompanied with this source code. See http://opcfoundation.org/License/RCL/1.00/
// GNU General Public License as published by the Free Software Foundation;
// version 2 of the License are accompanied with this source code. See http://opcfoundation.org/License/GPLv2
// This source code is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//

using System;
using System.Threading.Tasks;
using Microsoft.Azure.Devices.Proxy;

namespace Opc.Ua.Bindings.Proxy
{
    /// <summary>
    /// Creates a new ProxyTransportChannel with ITransportChannel interface.
    /// </summary>
    public class ProxyTransportChannelFactory : ITransportChannelFactory
    {
        /// <summary>
        /// The method creates a new instance of a Proxy transport channel
        /// </summary>
        /// <returns> the transport channel</returns>
        public ITransportChannel Create()
        {
            return new ProxyTransportChannel();
        }
    }

    /// <summary>
    /// Creates a transport channel with proxy transport, UA-SC security and UA Binary encoding
    /// </summary>
    public class ProxyTransportChannel : UaSCUaBinaryTransportChannel
    {
        public ProxyTransportChannel() :
            base(new ProxyMessageSocketFactory())
        {
        }
    }

    /// <summary>
    /// Handles async event callbacks from a socket
    /// </summary>
    public class ProxyMessageSocketAsyncEventArgs : IMessageSocketAsyncEventArgs
    {
        public ProxyMessageSocketAsyncEventArgs()
        {
            m_args = new SocketAsyncEventArgs();
            m_args.UserToken = this;
        }

        #region IDisposable Members
        /// <summary>
        /// Frees any unmanaged resources.
        /// </summary>
        public void Dispose()
        {
            m_args.Dispose();
        }
        #endregion

        public object UserToken
        {
            get { return m_UserToken; }
            set { m_UserToken = value; }
        }

        public void SetBuffer(byte[] buffer, int offset, int count)
        {
            m_args.SetBuffer(buffer, offset, count);
        }

        public bool IsSocketError
        {
            get { return m_args.SocketError != SocketError.Success; }
        }

        public string SocketErrorString
        {
            get { return m_args.SocketError.ToString(); }
        }


        public event EventHandler<IMessageSocketAsyncEventArgs> Completed
        {
            add
            {
                m_internalComplete += value;
                m_args.Completed += OnComplete;
            }
            remove
            {
                m_internalComplete -= value;
                m_args.Completed -= OnComplete;
            }
        }

        protected void OnComplete(object sender, SocketAsyncEventArgs e)
        {
            if (e.UserToken == null) return;
            m_internalComplete(this, e.UserToken as IMessageSocketAsyncEventArgs);
        }

        public int BytesTransferred
        {
            get { return m_args.BytesTransferred; }
        }

        public byte[] Buffer
        {
            get { return m_args.Buffer; }
        }

        public BufferCollection BufferList
        {
            get { return m_args.BufferList as BufferCollection; }
            set { m_args.BufferList = value; }
        }

        public SocketAsyncEventArgs m_args;
        private object m_UserToken;
        private event EventHandler<IMessageSocketAsyncEventArgs> m_internalComplete;
    }

    /// <summary>
    /// Creates a new ProxyMessageSocket with IMessageSocket interface.
    /// </summary>
    public class ProxyMessageSocketFactory : IMessageSocketFactory
    {
        /// <summary>
        /// The method creates a new instance of a proxy message socket
        /// </summary>
        /// <returns> the message socket</returns>
        public IMessageSocket Create(
                IMessageSink sink,
                BufferManager bufferManager,
                int receiveBufferSize
            )
        {
            return new ProxyMessageSocket(sink, bufferManager, receiveBufferSize);
        }

        /// <summary>
        /// Gets the implementation description.
        /// </summary>
        /// <value>The implementation string.</value>
        public string Implementation { get { return "UA-Proxy"; } }
    }

    /// <summary>
    /// Handles reading and writing of message chunks over a socket.
    /// </summary>
    public class ProxyMessageSocket : IMessageSocket
    {
        /// <summary>
        /// The proxy socket
        /// </summary>
        public Socket ProxySocket { get; set; }

        #region Constructors
        /// <summary>
        /// Creates an unconnected socket.
        /// </summary>
        public ProxyMessageSocket(
            IMessageSink sink,
            BufferManager bufferManager,
            int receiveBufferSize)
        {
            if (bufferManager == null) throw new ArgumentNullException("bufferManager");

            m_sink = sink;
            ProxySocket = null;
            m_bufferManager = bufferManager;
            m_receiveBufferSize = receiveBufferSize;
            m_incomingMessageSize = -1;
            m_ReadComplete = new EventHandler<SocketAsyncEventArgs>(OnReadComplete);
        }

        /// <summary>
        /// Attaches the object to an existing socket.
        /// </summary>
        public ProxyMessageSocket(
            IMessageSink sink,
            Socket socket,
            BufferManager bufferManager,
            int receiveBufferSize)
        {
            if (socket == null) throw new ArgumentNullException("socket");
            if (bufferManager == null) throw new ArgumentNullException("bufferManager");

            m_sink = sink;
            ProxySocket = socket;
            m_bufferManager = bufferManager;
            m_receiveBufferSize = receiveBufferSize;
            m_incomingMessageSize = -1;
            m_ReadComplete = new EventHandler<SocketAsyncEventArgs>(OnReadComplete);
        }
        #endregion

        #region IDisposable Members
        /// <summary>
        /// Frees any unmanaged resources.
        /// </summary>
        public void Dispose()
        {
            Dispose(true);
        }

        /// <summary>
        /// An overrideable version of the Dispose.
        /// </summary>
        protected virtual void Dispose(bool disposing)
        {
            if (disposing)
            {
                ProxySocket.Dispose();
            }
        }
        #endregion

        #region Connect/Disconnect Handling
        /// <summary>
        /// Gets the socket handle.
        /// </summary>
        /// <value>The socket handle.</value>
        public int Handle
        {
            get
            {
                if (ProxySocket != null)
                {
                    return ProxySocket.GetHashCode();
                }

                return -1;
            }
        }

        /// <summary>
        /// Connects to an endpoint.
        /// </summary>
        public async Task<bool> BeginConnect(Uri endpointUrl, EventHandler<IMessageSocketAsyncEventArgs> callback, object state)
        {
            bool result = false;

            if (endpointUrl == null) throw new ArgumentNullException("endpointUrl");

            if (ProxySocket != null)
            {
                throw new InvalidOperationException("The socket is already connected.");
            }

            ProxyMessageSocketAsyncEventArgs args = new ProxyMessageSocketAsyncEventArgs();
            args.UserToken = state;
            args.m_args.SocketError = SocketError.HostUnknown;

            ProxySocket = new Socket(SocketType.Stream, ProtocolType.Tcp);

            try
            {
                await ProxySocket.ConnectAsync(endpointUrl.DnsSafeHost, endpointUrl.Port);
                args.m_args.SocketError = SocketError.Success;
                result = true;
            }
            catch (Exception e)
            {
                SocketException se = e as SocketException;
                if (se != null)
                {
                    args.m_args.SocketError = se.Error;
                }
            }
            finally
            {
                Task t = Task.Run(() => callback(this, args));
            }
            return result;
        }

        /// <summary>
        /// Forcefully closes the socket.
        /// </summary>
        public void Close()
        {
            // get the socket.
            Socket socket = null;

            lock (m_socketLock)
            {
                socket = ProxySocket;
                ProxySocket = null;
            }

            // shutdown the socket.
            if (socket != null)
            {
                try
                {
                    socket.Dispose();
                }
                catch (Exception e)
                {
                    Utils.Trace(e, "Unexpected error closing socket.");
                }
            }
        }
        #endregion

        #region Read Handling
        /// <summary>
        /// Starts reading messages from the socket.
        /// </summary>
        public void ReadNextMessage()
        {
            lock (m_readLock)
            {
                // allocate a buffer large enough to a message chunk.
                if (m_receiveBuffer == null)
                {
                    m_receiveBuffer = m_bufferManager.TakeBuffer(m_receiveBufferSize, "ReadNextMessage");
                }

                // read the first 8 bytes of the message which contains the message size.          
                m_bytesReceived = 0;
                m_bytesToReceive = TcpMessageLimits.MessageTypeAndSize;
                m_incomingMessageSize = -1;

                ReadNextBlock();
            }
        }

        /// <summary>
        /// Changes the sink used to report reads.
        /// </summary>
        public void ChangeSink(IMessageSink sink)
        {
            lock (m_readLock)
            {
                m_sink = sink;
            }
        }

        /// <summary>
        /// Handles a read complete event.
        /// </summary>
        private void OnReadComplete(object sender, SocketAsyncEventArgs e)
        {
            lock (m_readLock)
            {
                ServiceResult error = null;

                try
                {
                    error = DoReadComplete(e);
                }
                catch (Exception ex)
                {
                    Utils.Trace(ex, "Unexpected error during OnReadComplete,");
                    error = ServiceResult.Create(ex, StatusCodes.BadTcpInternalError, ex.Message);
                }
                finally
                {
                    e.Dispose();
                }

                if (ServiceResult.IsBad(error))
                {
                    if (m_receiveBuffer != null)
                    {
                        m_bufferManager.ReturnBuffer(m_receiveBuffer, "OnReadComplete");
                        m_receiveBuffer = null;
                    }

                    if (m_sink != null)
                    {
                        m_sink.OnReceiveError(this, error);
                    }
                }
            }
        }

        /// <summary>
        /// Handles a read complete event.
        /// </summary>
        private ServiceResult DoReadComplete(SocketAsyncEventArgs e)
        {
            // complete operation.
            int bytesRead = e.BytesTransferred;

            lock (m_socketLock)
            {
                BufferManager.UnlockBuffer(m_receiveBuffer);
            }

            Utils.Trace("Bytes read: {0}", bytesRead);

            if (bytesRead == 0 || e.SocketError != (int)SocketError.Success)
            {
                // free the empty receive buffer.
                if (m_receiveBuffer != null)
                {
                    m_bufferManager.ReturnBuffer(m_receiveBuffer, "DoReadComplete");
                    m_receiveBuffer = null;
                }

                if (bytesRead == 0)
                { 
                    return ServiceResult.Create(StatusCodes.BadConnectionClosed, "Remote side closed connection");
                }

                return ServiceResult.Create(StatusCodes.BadTcpInternalError, "Error {0} on connection during receive", e.SocketError.ToString());
            }

            m_bytesReceived += bytesRead;

            // check if more data left to read.
            if (m_bytesReceived < m_bytesToReceive)
            {
                ReadNextBlock();

                return ServiceResult.Good;
            }

            // start reading the message body.
            if (m_incomingMessageSize < 0)
            {
                m_incomingMessageSize = BitConverter.ToInt32(m_receiveBuffer, 4);

                if (m_incomingMessageSize <= 0 || m_incomingMessageSize > m_receiveBufferSize)
                {
                    Utils.Trace(
                        "BadTcpMessageTooLarge: BufferSize={0}; MessageSize={1}",
                        m_receiveBufferSize,
                        m_incomingMessageSize);

                    return ServiceResult.Create(
                        StatusCodes.BadTcpMessageTooLarge,
                        "Messages size {1} bytes is too large for buffer of size {0}.",
                        m_receiveBufferSize,
                        m_incomingMessageSize);
                }

                // set up buffer for reading the message body.
                m_bytesToReceive = m_incomingMessageSize;

                ReadNextBlock();

                return ServiceResult.Good;
            }

            // notify the sink.
            if (m_sink != null)
            {
                try
                {
                    // send notification (implementor responsible for freeing buffer) on success.
                    ArraySegment<byte> messageChunk = new ArraySegment<byte>(m_receiveBuffer, 0, m_incomingMessageSize);

                    // must allocate a new buffer for the next message.
                    m_receiveBuffer = null;

                    m_sink.OnMessageReceived(this, messageChunk);
                }
                catch (Exception ex)
                {
                    Utils.Trace(ex, "Unexpected error invoking OnMessageReceived callback.");
                }
            }

            // free the receive buffer.
            if (m_receiveBuffer != null)
            {
                m_bufferManager.ReturnBuffer(m_receiveBuffer, "DoReadComplete");
                m_receiveBuffer = null;
            }

            // start receiving next message.
            ReadNextMessage();

            return ServiceResult.Good;
        }

        /// <summary>
        /// Reads the next block of data from the socket.
        /// </summary>
        private void ReadNextBlock()
        {
            Socket socket = null;

            // check if already closed.
            lock (m_socketLock)
            {
                if (ProxySocket == null)
                {
                    if (m_receiveBuffer != null)
                    {
                        m_bufferManager.ReturnBuffer(m_receiveBuffer, "ReadNextBlock");
                        m_receiveBuffer = null;
                    }

                    return;
                }

                socket = ProxySocket;

                // avoid stale ServiceException when socket is disconnected
                if (!socket.Connected)
                {
                    return;
                }

            }

            BufferManager.LockBuffer(m_receiveBuffer);

            ServiceResult error = ServiceResult.Good;
            SocketAsyncEventArgs args = new SocketAsyncEventArgs();
            try
            {
                args.SetBuffer(m_receiveBuffer, m_bytesReceived, m_bytesToReceive - m_bytesReceived);
                args.Completed += m_ReadComplete;
                if (!socket.ReceiveAsync(args))
                {
                    // I/O completed synchronously
                    if ((args.SocketError != SocketError.Success) || (args.BytesTransferred < (m_bytesToReceive - m_bytesReceived)))
                    {
                        throw ServiceResultException.Create(StatusCodes.BadTcpInternalError, args.SocketError.ToString());
                    }

                    args.Dispose();
                }
            }
            catch (ServiceResultException sre)
            {
                args.Dispose();
                BufferManager.UnlockBuffer(m_receiveBuffer);
                throw sre;
            }
            catch (Exception ex)
            {
                args.Dispose();
                BufferManager.UnlockBuffer(m_receiveBuffer);
                throw ServiceResultException.Create(StatusCodes.BadTcpInternalError, ex, "BeginReceive failed.");
            }
        }
        #endregion
        #region Write Handling
        /// <summary>
        /// Sends a buffer.
        /// </summary>
        public bool SendAsync(IMessageSocketAsyncEventArgs args)
        {
            ProxyMessageSocketAsyncEventArgs eventArgs = args as ProxyMessageSocketAsyncEventArgs;
            if (eventArgs == null)
            {
                throw new ArgumentNullException("args");
            }
            if (ProxySocket == null)
            {
                throw new InvalidOperationException("The socket is not connected.");
            }
            eventArgs.m_args.SocketError = SocketError.Unknown;
            return ProxySocket.SendAsync(eventArgs.m_args);
        }
        #endregion
        #region Event factory
        public IMessageSocketAsyncEventArgs MessageSocketEventArgs()
        {
            return new ProxyMessageSocketAsyncEventArgs();
        }
        #endregion
        #region Private Fields
        private IMessageSink m_sink;
        private BufferManager m_bufferManager;
        private int m_receiveBufferSize;
        private EventHandler<SocketAsyncEventArgs> m_ReadComplete;

        private object m_socketLock = new object();
        private object m_readLock = new object();
        private byte[] m_receiveBuffer;
        private int m_bytesReceived;
        private int m_bytesToReceive;
        private int m_incomingMessageSize;
        #endregion
    }
}