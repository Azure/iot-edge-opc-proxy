// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy.Model {
    using System;
    using System.Runtime.Serialization;

    
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
    public class InterfaceInfo : IEquatable<InterfaceInfo> {

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
        public string Name { get; set; } = "";

        /// <summary>
        /// Index of interface 
        /// </summary>
        [DataMember(Name = "index", Order = 5)]
        public int Index { get; set; }

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
        public bool Equals(InterfaceInfo that) {
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
            return Equals(that as InterfaceInfo);
        }

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return (((((
                Address.GetHashCode() * 31) ^
                SubnetPrefix.GetHashCode() * 31) ^
                Flags.GetHashCode() * 31) ^
                Name.GetHashCode() * 31) ^
                Index.GetHashCode() * 31) ^
                Broadcast.GetHashCode();
        }
    }
}