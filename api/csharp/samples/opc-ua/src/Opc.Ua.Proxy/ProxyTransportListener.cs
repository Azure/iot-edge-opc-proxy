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
using System.Collections.Generic;
using System.Threading;
using System.Security.Cryptography.X509Certificates;
using Opc.Ua.Bindings;
using Microsoft.Azure.Devices.Proxy;

namespace Opc.Ua.Proxy
{
    /// <summary>
    /// Manages the connections for a UA TCP server.
    /// </summary>
    public partial class ProxyTransportListener : IDisposable, ITransportListener
    {
        #region Constructors
        /// <summary>
        /// Initializes a new instance of the <see cref="ProxyTransportListener"/> class.
        /// </summary>
        public ProxyTransportListener()
        {
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
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Usage", "CA2213:DisposableFieldsShouldBeDisposed", MessageId = "m_simulator")]
        protected virtual void Dispose(bool disposing)
        {
            if (disposing) 
            {
                lock (m_lock)
                {
                    if (m_listeningSocket != null)
                    {
                        try
                        {
                            m_listeningSocket.Dispose();
                        }
                        catch
                        {
                            // ignore errors.
                        }
                        
                        m_listeningSocket = null;
                    }

                    if (m_simulator != null)
                    {
                        Utils.SilentDispose(m_simulator);
                        m_simulator = null; 
                    }

                    foreach (ProxyServerChannel channel in m_channels.Values)
                    {
                        Utils.SilentDispose(channel);
                    }
                }
            }
        }
        #endregion
        
        #region ITransportListener Members
        /// <summary>
        /// Opens the listener and starts accepting connection.
        /// </summary>
        /// <param name="baseAddress">The base address.</param>
        /// <param name="settings">The settings to use when creating the listener.</param>
        /// <param name="callback">The callback to use when requests arrive via the channel.</param>
        /// <exception cref="ArgumentNullException">Thrown if any parameter is null.</exception>
        /// <exception cref="ServiceResultException">Thrown if any communication error occurs.</exception>
        public void Open(Uri baseAddress, TransportListenerSettings settings, ITransportListenerCallback callback)
        {
            // assign a unique guid to the listener.
            m_listenerId = Guid.NewGuid().ToString();

            m_uri = baseAddress;
            m_descriptions = settings.Descriptions;
            m_configuration = settings.Configuration;

            // initialize the quotas.
            m_quotas = new TcpChannelQuotas();

            m_quotas.MaxBufferSize = m_configuration.MaxBufferSize;
            m_quotas.MaxMessageSize = m_configuration.MaxMessageSize;
            m_quotas.ChannelLifetime = m_configuration.ChannelLifetime;
            m_quotas.SecurityTokenLifetime = m_configuration.SecurityTokenLifetime;

            m_quotas.MessageContext = new ServiceMessageContext();

            m_quotas.MessageContext.MaxArrayLength = m_configuration.MaxArrayLength;
            m_quotas.MessageContext.MaxByteStringLength = m_configuration.MaxByteStringLength;
            m_quotas.MessageContext.MaxMessageSize = m_configuration.MaxMessageSize;
            m_quotas.MessageContext.MaxStringLength = m_configuration.MaxStringLength;
            m_quotas.MessageContext.NamespaceUris = settings.NamespaceUris;
            m_quotas.MessageContext.ServerUris = new StringTable();
            m_quotas.MessageContext.Factory = settings.Factory;

            m_quotas.CertificateValidator = settings.CertificateValidator;

            // save the server certificate.
            m_serverCertificate = settings.ServerCertificate;
            m_serverCertificateChain = settings.ServerCertificateChain;

            m_bufferManager = new BufferManager("Server", (int)Int32.MaxValue, m_quotas.MaxBufferSize);
            m_channels = new Dictionary<uint, ProxyServerChannel>();

            // save the callback to the server.
            m_callback = callback;

            // start the listener.
            Start();
        }

        /// <summary>
        /// Closes the listener and stops accepting connection.
        /// </summary>
        /// <exception cref="ServiceResultException">Thrown if any communication error occurs.</exception>
        public void Close()
        {
            Stop();
        }
        #endregion

        #region Public Methods
        /// <summary>
        /// Gets the URL for the listener's endpoint.
        /// </summary>
        /// <value>The URL for the listener's endpoint.</value>
        public Uri EndpointUrl
        {
            get { return m_uri; }
        }

        /// <summary>
        /// Starts listening at the specified port.
        /// </summary>
        public void Start()
        {
            lock (m_lock)
            {
                // ensure a valid port.
                int port = m_uri.Port;

                if (port <= 0 || port > UInt16.MaxValue)
                {
                    port = Utils.UaTcpDefaultPort;
                }

                try                    
                {
                    m_listeningSocket = new Socket(SocketType.Stream, ProtocolType.Tcp);
                    m_listeningSocket.Bind(m_uri.DnsSafeHost, port);
                    m_listeningSocket.Listen();

                    SocketAsyncEventArgs args = new SocketAsyncEventArgs();
                    args.Completed += OnAccept;
                    args.UserToken = m_listeningSocket;
                    m_listeningSocket.AcceptAsync(args);
                }
                catch (Exception ex)
                {
                    m_listeningSocket = null;
                    Utils.Trace("failed to create listening socket: " + ex.Message);
                }
            }
        }

        /// <summary>
        /// Stops listening.
        /// </summary>
        public void Stop()
        {
            lock (m_lock)
            {
                if (m_listeningSocket != null)
                {
                    m_listeningSocket.Dispose();
                    m_listeningSocket = null;
                }
            }
        }

        /// <summary>
        /// Binds a new socket to an existing channel.
        /// </summary>
        internal bool ReconnectToExistingChannel(
            ProxyMessageSocket         socket, 
            uint                     requestId,
            uint                     sequenceNumber,
            uint                     channelId,
            X509Certificate2         clientCertificate, 
            TcpChannelToken          token,
            OpenSecureChannelRequest request)
        {            
            ProxyServerChannel channel = null;

            lock (m_lock)
            {
                if (!m_channels.TryGetValue(channelId, out channel))
                {
                    throw ServiceResultException.Create(StatusCodes.BadTcpSecureChannelUnknown, "Could not find secure channel referenced in the OpenSecureChannel request.");
                }
            }
                       
            channel.Reconnect(socket, requestId, sequenceNumber, clientCertificate, token, request);
            Utils.Trace("Channel {0} reconnected", channelId);
            return true;
        }

        /// <summary>
        /// Called when a channel closes.
        /// </summary>
        internal void ChannelClosed(uint channelId)
        {
            lock (m_lock)
            {
                m_channels.Remove(channelId);
            }

            Utils.Trace("Channel {0} closed", channelId);
        }
        #endregion
        
        #region Socket Event Handler
        /// <summary>
        /// Handles a new connection.
        /// </summary>
        private void OnAccept(object sender, SocketAsyncEventArgs e)
        {
            ProxyServerChannel channel = null;

            lock (m_lock)
            {
                Socket listeningSocket = e.UserToken as Socket;

                if (listeningSocket == null)
                {
                    Utils.Trace("OnAccept: Listensocket was null." );
                    e.Dispose();
                    return;
                }

                // check if the accept socket has been created.
                if (e.AcceptSocket != null && e.SocketError == SocketError.Ok)
                {
                    try
                    {
                        // create the channel to manage incoming messages.
                        channel = new ProxyServerChannel(
                            m_listenerId,
                            this,
                            m_bufferManager,
                            m_quotas,
                            m_serverCertificate,
                            m_descriptions);

                        // start accepting messages on the channel.
                        channel.Attach(++m_lastChannelId, e.AcceptSocket);

                        // save the channel for shutdown and reconnects.
                        m_channels.Add(m_lastChannelId, channel);

                        if (m_callback != null)
                        {
                            channel.SetRequestReceivedCallback(new ProxyChannelRequestEventHandler(OnRequestReceived));
                        }
                    }
                    catch (Exception ex)
                    {
                        Utils.Trace(ex, "Unexpected error accepting a new connection.");
                    }
                }

                e.Dispose();

                if (e.SocketError != SocketError.Aborted)
                { 
                    // go back and wait for the next connection.
                    try
                    {
                        e = new SocketAsyncEventArgs();
                        e.Completed += OnAccept;
                        e.UserToken = listeningSocket;
                        listeningSocket.AcceptAsync(e);
                    }
                    catch (Exception ex)
                    {
                        Utils.Trace(ex, "Unexpected error listening for a new connection.");
                    }
                }
            }
        }
#endregion
                
#region Private Methods
        /// <summary>
        /// Handles requests arriving from a channel.
        /// </summary>
        private void OnRequestReceived(ProxyServerChannel channel, uint requestId, IServiceRequest request)
        {
            try
            {
                if (m_callback != null)
                {
                    IAsyncResult result = m_callback.BeginProcessRequest(
                        channel.GlobalChannelId,
                        channel.EndpointDescription,
                        request,
                        OnProcessRequestComplete,
                        new object[] { channel, requestId, request });
                }
            }
            catch (Exception e)
            {
                Utils.Trace(e, "TCPLISTENER - Unexpected error processing request.");
            }
        }

        private void OnProcessRequestComplete(IAsyncResult result)
        {
            try
            {
                object[] args = (object[])result.AsyncState;

                if (m_callback != null)
                {
                    ProxyServerChannel channel = (ProxyServerChannel)args[0];
                    IServiceResponse response = m_callback.EndProcessRequest(result);
                    channel.SendResponse((uint)args[1], response);
                }
            }
            catch (Exception e)
            {
                Utils.Trace(e, "TCPLISTENER - Unexpected error sending result.");
            }
        }
#endregion

#region Private Fields
        private object m_lock = new object();

        private string m_listenerId;
        private Uri m_uri;
        private EndpointDescriptionCollection m_descriptions;
        private EndpointConfiguration m_configuration;

        private BufferManager m_bufferManager;
        private TcpChannelQuotas m_quotas;
        private X509Certificate2 m_serverCertificate;
        private X509Certificate2Collection m_serverCertificateChain;

        private uint m_lastChannelId;

        private Socket m_listeningSocket;
        private Dictionary<uint,ProxyServerChannel> m_channels;

        private Timer m_simulator;
        private ITransportListenerCallback m_callback;
#endregion
    }    
}
