// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Provider {
    using System;
    using System.Threading.Tasks;
    using Proxy;

    /// <summary>
    /// Provider that uses relay as stream channel, instead of the default 
    /// implementation
    /// </summary>
    public class ServiceBusRelayProvider : DefaultProvider {
        private IStreamService _relay;
        public override IStreamService StreamService {
            get {
                return _relay;
            }
        }

        public static Task<IProvider> CreateAsync() => 
            RelayProvider.CreateAsync((string)null, (string)null);

        /// <summary>
        /// Private constructor
        /// </summary>
        /// <param name="iothub"></param>
        /// <param name="relay"></param>
        internal ServiceBusRelayProvider(ConnectionString iothub, IStreamService relay) :
            base(iothub) {
            _relay = relay;
        }

        /// <summary>
        /// Private constructor
        /// </summary>
        /// <param name="iothub"></param>
        /// <param name="relay"></param>
        internal ServiceBusRelayProvider(string iothub, IStreamService relay) :
            base(iothub) {
            _relay = relay;
        }
    }

    /// <summary>
    /// Provider that uses relay as stream channel, instead of the default 
    /// implementation
    /// </summary>
    public class RelayProvider {

        /// <summary>
        /// Create relay provider
        /// </summary>
        /// <param name="iothub"></param>
        /// <param name="relay"></param>
        /// <returns></returns>
        public static async Task<IProvider> CreateAsync(
            ConnectionString iothub, ConnectionString relay) {
            if (relay == null)
                throw new ArgumentException("Must provide relay connection string.");
            var service = await ServiceBusRelay.CreateAsync(relay.Entity ??
                "__Microsoft_Azure_Devices_Proxy_1__", relay).ConfigureAwait(false);
            return new ServiceBusRelayProvider(iothub, service);
        }

        /// <summary>
        /// Create relay provider
        /// </summary>
        /// <param name="iothub"></param>
        /// <param name="relay"></param>
        /// <returns></returns>
        public static async Task<IProvider> CreateAsync(
            string iothub, string relay) {
            if (relay == null)
                relay = Environment.GetEnvironmentVariable("_SB_CS");
            if (relay == null)
                throw ProxyEventSource.Log.ArgumentNull("relay");

            var cs = ConnectionString.Parse(relay);
            var service = await ServiceBusRelay.CreateAsync(cs.Entity ??
                "__Microsoft_Azure_Devices_Proxy_1__", cs).ConfigureAwait(false);
            return new ServiceBusRelayProvider(iothub, service);
        }

        /// <summary>
        /// Create relay provider from environment
        /// </summary>
        /// <returns></returns>
        public static Task<IProvider> CreateAsync() =>
            CreateAsync((string)null, (string)null);
    }
}
