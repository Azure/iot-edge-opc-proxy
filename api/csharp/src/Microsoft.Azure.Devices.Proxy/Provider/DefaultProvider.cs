// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Provider {
    using Model;
    using System.Threading.Tasks;
    using System;

    /// <summary>
    /// Default provider combines IoTHub name service with service bus stream service
    /// </summary>
    public class DefaultProvider : IProvider {

        public static readonly DefaultProvider Instance = new DefaultProvider();

        private IoTHub _iothub;
        private IStreamService _relay;

        public IRemotingService ControlChannel {
            get {
                return _iothub;
            }
        }

        public INameService NameService {
            get {
                return _iothub;
            }
        }

        public IStreamService StreamService {
            get {
                return _relay;
            }
        }

        /// <summary>
        /// Initialize default provider
        /// </summary>
        /// <param name="iothub"></param>
        /// <param name="relay"></param>
        /// <returns></returns>
        public async Task Init(ConnectionString iothub, ConnectionString relay) {
            if (iothub == null)
                throw new ArgumentException("Must provide iot hub connection string.");
            if (relay == null)
                throw new ArgumentException("Must provide relay connection string.");
            _iothub = 
                new IoTHub(iothub);
            _relay = await ServiceBusRelay.CreateAsync(relay.Entity ?? 
                "__Microsoft_Azure_Devices_Proxy_1__", relay).ConfigureAwait(false);
        }

        /// <summary>
        /// Initialize default provider
        /// </summary>
        /// <param name="iothub"></param>
        /// <param name="relay"></param>
        /// <returns></returns>
        public async Task Init(string iothub, string relay) {
            if (iothub == null)
                iothub = Environment.GetEnvironmentVariable("_HUB_CS");
            if (relay == null)
                relay = Environment.GetEnvironmentVariable("_SB_CS");
            if (iothub == null)
                throw ProxyEventSource.Log.ArgumentNull("iothub", this);
            if (relay == null)
                throw ProxyEventSource.Log.ArgumentNull("relay", this);
            try {
                await Init(ConnectionString.Parse(iothub),
                    ConnectionString.Parse(relay)).ConfigureAwait(false);
            }
            catch(Exception e) {
                throw ProxyEventSource.Log.Rethrow(e, this);
            }
        }

        /// <summary>
        /// Init from environment
        /// </summary>
        /// <returns></returns>
        public Task Init() =>
            Init((string)null, (string)null);
    }
}
