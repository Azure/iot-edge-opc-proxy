// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Provider {
    using System;

    public class NullProvider : IProvider {
        public IRemotingService ControlChannel {
            get => throw new NotImplementedException();
        }

        public INameService NameService {
            get => throw new NotImplementedException();
        }

        public IStreamService StreamService {
            get => throw new NotImplementedException();
        }

        /// <summary>
        /// Initialize default provider
        /// </summary>
        /// <returns></returns>
        public static IProvider Create() {
            var iothub = Environment.GetEnvironmentVariable("_HUB_CS");
            if (iothub != null) {
                return new DefaultProvider(iothub);
            }
            return new NullProvider();
        }

        /// <summary>
        /// Default constructor, initializes from environment
        /// </summary>
        private NullProvider() {}
    }
}
