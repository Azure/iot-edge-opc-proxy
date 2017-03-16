// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Provider {
    using Proxy;
    using Model;
    using System.Threading.Tasks;
    using System;

    /// <summary>
    /// Default service provider uses IoTHub service.
    /// </summary>
    public class DefaultProvider : IProvider {

        public virtual IRemotingService ControlChannel {
            get {
                return _iothub;
            }
        }

        public virtual INameService NameService {
            get {
                return _iothub;
            }
        }

        public virtual IStreamService StreamService {
            get {
                return _iothub;
            }
        }

        /// <summary>
        /// DefaultProvider constructor
        /// </summary>
        /// <param name="iothub"></param>
        /// <returns></returns>
        public DefaultProvider(ConnectionString iothub) {
            if (iothub == null)
                throw new ArgumentException("Must provide iot hub connection string.");
            try {
                _iothub = new IoTHub(iothub);
            }
            catch(Exception e) {
                throw ProxyEventSource.Log.Rethrow(e, this);
            }
        }

        /// <summary>
        /// Initialize default provider
        /// </summary>
        /// <param name="iothub"></param>
        /// <returns></returns>
        public DefaultProvider(string iothub) {
            var tcs = new TaskCompletionSource<bool>();
            if (iothub == null)
                iothub = Environment.GetEnvironmentVariable("_HUB_CS");
            if (iothub == null) {
                throw ProxyEventSource.Log.ArgumentNull("iothub", this);
            }
            try {
                _iothub = new IoTHub(ConnectionString.Parse(iothub));
            }
            catch(Exception e) {
                throw ProxyEventSource.Log.Rethrow(e, this);
            }
        }

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

        private IoTHub _iothub;
    }
}
