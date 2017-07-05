// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Runtime.Serialization;

    /// <summary>
    /// Platform independent network interface info
    /// </summary>
    [DataContract]
    public class InterfaceInfo : Poco<InterfaceInfo> {

        /// <summary>
        /// Address of interface
        /// </summary>
        [DataMember(Name = "address", Order = 1)]
        public SocketAddress Address {
            get; set;
        }

        /// <summary>
        /// Subnet mask in form of prefix
        /// </summary>
        [DataMember(Name = "prefix", Order = 2)]
        public byte SubnetPrefix {
            get; set;
        }

        public static readonly byte InterfaceUp = 0x1;
        public static readonly byte Loopback = 0x2;
        public static readonly byte Multicast = 0x4;

        /// <summary>
        /// Address info flags - see above
        /// </summary>
        [DataMember(Name = "flags", Order = 3)]
        public byte Flags {
            get; set;
        }

        /// <summary>
        /// Name of interface entry, e.g. eth0:
        /// </summary>
        [DataMember(Name = "name", Order = 4)]
        public string Name {
            get; set;
        } = "";

        /// <summary>
        /// Index of interface 
        /// </summary>
        [DataMember(Name = "index", Order = 5)]
        public int Index {
            get; set;
        }

        /// <summary>
        /// Broadcast address for this interface
        /// </summary>
        [DataMember(Name = "broadcast_addr", Order = 6)]
        public SocketAddress Broadcast {
            get; set;
        }

        /// <summary>
        /// Create interface info
        /// </summary>
        /// <param name="address"></param>
        /// <param name="subnetPrefix"></param>
        /// <param name="flags"></param>
        /// <param name="name"></param>
        /// <param name="index"></param>
        /// <param name="broadcast"></param>
        /// <returns></returns>
        public static InterfaceInfo Create(SocketAddress address, byte subnetPrefix,
            byte flags, string name, int index, SocketAddress broadcast) {
            var info = Get();
            info.Address = address;
            info.SubnetPrefix = subnetPrefix;
            info.Flags = flags;
            info.Name = name;
            info.Index = index;
            info.Broadcast = broadcast;
            return info;
        }

        public InterfaceInfo Clone() => Create(Address, SubnetPrefix, Flags, Name,
            Index, Broadcast);

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool IsEqual(InterfaceInfo that) {
            return
                IsEqual(Address, that.Address) &&
                IsEqual(SubnetPrefix, that.SubnetPrefix) &&
                IsEqual(Flags, that.Flags) &&
                IsEqual(Broadcast, that.Broadcast) &&
                IsEqual(Index, that.Index) &&
                IsEqual(Name, that.Name);
        }

        protected override void SetHashCode() {
            MixToHash(Address);
            MixToHash(SubnetPrefix);
            MixToHash(Flags);
            MixToHash(Broadcast);
            MixToHash(Index);
            MixToHash(Name);
        }
    }
}