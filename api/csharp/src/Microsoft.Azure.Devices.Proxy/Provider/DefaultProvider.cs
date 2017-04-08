// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Provider {
    using Proxy;
    using Model;
    using System;

    /// <summary>
    /// Default service provider uses IoTHub service.
    /// </summary>
    public class DefaultProvider : IProvider {

        public virtual IRemotingService ControlChannel  => _iothub;
        public virtual INameService NameService => _iothub;
        public virtual IStreamService StreamService => _iothub;

        /// <summary>
        /// Initialize default provider
        /// </summary>
        /// <param name="iothub"></param>
        /// <param name="registryRefresh"></param>
        public DefaultProvider(ConnectionString iothub, TimeSpan registryRefresh) {
            if (iothub == null)
                throw new ArgumentException("Must provide iot hub connection string.");
            try {
                _iothub = new IoTHubService(iothub, registryRefresh);
            }
            catch(Exception e) {
                throw ProxyEventSource.Log.Rethrow(e, this);
            }
        }

        /// <summary>
        /// Initialize default provider
        /// </summary>
        /// <param name="iothub"></param>
        /// <param name="registryRefresh"></param>
        public DefaultProvider(string iothub, TimeSpan registryRefresh) {
            if (iothub == null)
                iothub = Environment.GetEnvironmentVariable("_HUB_CS");
            if (iothub == null) {
                throw ProxyEventSource.Log.ArgumentNull(nameof(iothub), this);
            }
            try {
                _iothub = new IoTHubService(ConnectionString.Parse(iothub), registryRefresh);
            }
            catch(Exception e) {
                throw ProxyEventSource.Log.Rethrow(e, this);
            }
        }

        /// <summary>
        /// Initialize default provider
        /// </summary>
        /// <param name="iothub"></param>
        public DefaultProvider(ConnectionString iothub) : this(iothub, _defaultRefresh) { }

        /// <summary>
        /// Initialize default provider
        /// </summary>
        /// <param name="iothub"></param>
        public DefaultProvider(string iothub) : this(iothub, _defaultRefresh) { }

        /// <summary>
        /// Default constructor, initializes from environment
        /// </summary>
        public DefaultProvider() : this((string)null) {}
        
        /// <summary>
        /// Same as constructor
        /// </summary>
        /// <param name="iothub"></param>
        /// <returns></returns>
        public static IProvider Create(string iothub) => new DefaultProvider(iothub);

        /// <summary>
        /// Same as constructor
        /// </summary>
        /// <param name="iothub"></param>
        /// <returns></returns>
        public static IProvider Create(ConnectionString iothub) => new DefaultProvider(iothub);

        private readonly IoTHubService _iothub;
        private static readonly TimeSpan _defaultRefresh = TimeSpan.FromMinutes(1);
    }
}
