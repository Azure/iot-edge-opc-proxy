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
    /// pi_addrinfo provides a platform independent host resolution
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
        public String CanonicalName { get; set; } = "";

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
            return SocketAddress.GetHashCode() ^ CanonicalName.GetHashCode();
        }
    }

    /// <summary>
    /// Address info flags
    /// </summary>
    public enum IfAddrInfoFlag {
        InterfaceUp = 0x1,
        Loopback = 0x2,
        Multicast = 0x4
    }

    /// <summary>
    /// Platform independent network interface address info
    /// </summary>
    [DataContract]
    public class InterfaceAddressInfo : IEquatable<InterfaceAddressInfo> {

        /// <summary>
        /// Address of interface
        /// </summary>
        [DataMember(Name = "address", Order = 1)]
        public SocketAddress Address { get; set; }

        /// <summary>
        /// Subnet mask in form of prefix
        /// </summary>
        [DataMember(Name = "prefix", Order = 2)]
        public byte SubnetPrefix { get; set; }

        /// <summary>
        /// Flags (see above)
        /// </summary>
        [DataMember(Name = "flags", Order = 3)]
        public byte Flags { get; set; }

        /// <summary>
        /// Name of interface entry, e.g. eth0:
        /// </summary>
        [DataMember(Name = "name", Order = 4)]
        public String Name { get; set; } = "";

        /// <summary>
        /// Index of interface 
        /// </summary>
        [DataMember(Name = "index", Order = 5)]
        public Int32 Index { get; set; }

        /// <summary>
        /// Broadcast address for this interface
        /// </summary>
        [DataMember(Name = "broadcast_addr", Order = 6)]
        public SocketAddress Broadcast { get; set; }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(InterfaceAddressInfo that) {
            if (that == null) {
                return false;
            }
            return
                this.Address.Equals(that.Address) &&
                this.SubnetPrefix.Equals(that.SubnetPrefix) &&
                this.Flags.Equals(that.Flags) &&
                this.Broadcast.Equals(that.Broadcast) &&
                this.Index.Equals(that.Index) &&
                this.Name.Equals(that.Name);
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(Object that) {
            return Equals(that as InterfaceAddressInfo);
        }

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return
                Address.GetHashCode() ^
                SubnetPrefix.GetHashCode() ^
                Flags.GetHashCode() ^
                Name.GetHashCode() ^
                Index.GetHashCode() ^
                Broadcast.GetHashCode();
        }
    }
}