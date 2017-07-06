// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System.Runtime.Serialization;

    /// <summary>
    /// prx_addrinfo provides a platform independent host resolution
    /// </summary>
    [DataContract]
    public class AddressInfo : Poco<AddressInfo> {

        /// <summary>
        /// Address found for resolved name
        /// </summary>
        [DataMember(Name = "address", Order = 1)]
        public SocketAddress SocketAddress {
            get; set;
        }

        /// <summary>
        /// Canonical name
        /// </summary>
        [DataMember(Name = "name", Order = 2)]
        public string CanonicalName {
            get; set;
        }

        /// <summary>
        /// Create info
        /// </summary>
        public static AddressInfo Create(SocketAddress socketAddress, 
            string canonicalName = "") {
            var info = Get();
            info.SocketAddress = socketAddress;
            info.CanonicalName = canonicalName;
            return info;
        }

        public AddressInfo Clone() => Create(SocketAddress, CanonicalName);

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool IsEqual(AddressInfo that) {
            return
                IsEqual(SocketAddress, that.SocketAddress) &&
                IsEqual(CanonicalName, that.CanonicalName);
        }

        protected override void SetHashCode() {
            MixToHash(SocketAddress);
            MixToHash(CanonicalName);
        }
    }
}