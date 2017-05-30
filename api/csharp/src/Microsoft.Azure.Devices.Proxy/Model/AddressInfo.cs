// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy.Model {
    using System;
    using System.Runtime.Serialization;

    /// <summary>
    /// flags used in calls to getnameinfo
    /// </summary>
    public enum GetNameInfoFlag {
        HostNameRequired = 0x1
    }

    /// <summary>
    /// flags used in calls to getaddrinfo
    /// </summary>
    public enum GetAddrInfoFlag {
        Passive = 0x1
    }

    /// <summary>
    /// prx_addrinfo provides a platform independent host resolution
    /// </summary>
    [DataContract]
    public class AddressInfo : IEquatable<AddressInfo> {

        /// <summary>
        /// Address found for resolved name
        /// </summary>
        [DataMember(Name = "address", Order = 1)]
        public SocketAddress SocketAddress { get; set; }

        /// <summary>
        /// Canonical name
        /// </summary>
        [DataMember(Name = "name", Order = 2)]
        public string CanonicalName { get; set; } = "";

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(AddressInfo that) {
            if (that == null) {
                return false;
            }
            return
                this.SocketAddress.Equals(that.SocketAddress) &&
                this.CanonicalName.Equals(that.CanonicalName);
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(Object that) {
            return Equals(that as AddressInfo);
        }

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return (SocketAddress.GetHashCode() * 31) ^ 
                CanonicalName.GetHashCode();
        }
    }
}