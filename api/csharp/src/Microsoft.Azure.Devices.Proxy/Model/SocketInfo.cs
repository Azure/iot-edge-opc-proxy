// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy.Model {
    using System;
    using System.Collections.Generic;
    using System.Runtime.Serialization;

    /// <summary>
    /// Flags for socket properties, determine how socket is opened
    /// </summary>
    public enum SocketFlags {
        Passive = 0x1
    }

    /// <summary>
    /// Describes the properties of the socket, e.g. what and how it is opened
    /// </summary>
    [DataContract]
    public class SocketInfo : IEquatable<SocketInfo> {

        /// <summary>
        /// Address family
        /// </summary>
        [DataMember(Name = "family", Order = 1)]
        public AddressFamily Family { get; set; } = AddressFamily.InterNetworkV6;

        /// <summary>
        /// Type of socket
        /// </summary>
        [DataMember(Name = "sock_type", Order = 2)]
        public SocketType Type { get; set; } = SocketType.Stream;

        /// <summary>
        /// Protocol
        /// </summary>
        [DataMember(Name = "proto_type", Order = 3)]
        public ProtocolType Protocol { get; set; } = ProtocolType.Tcp;

        /// <summary>
        /// Socket flags
        /// </summary>
        [DataMember(Name = "flags", Order = 4)]
        public UInt32 Flags { get; set; }

        /// <summary>
        /// Address to use to open, if proxy address, will be resolved.
        /// </summary>
        [DataMember(Name = "address", Order = 5)]
        public SocketAddress Address { get; set; }

        /// <summary>
        /// Socket options that apply to the socket
        /// </summary>
        [DataMember(Name = "options", Order = 6)]
        public HashSet<SocketOptionValue> Options { get; set; } = new HashSet<SocketOptionValue>();

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(SocketInfo that) {
            if (that == null) {
                return false;
            }
            return
                this.Family.Equals(that.Family) &&
                this.Type.Equals(that.Type) &&
                this.Protocol.Equals(that.Protocol) &&
                this.Flags.Equals(that.Flags) &&
                this.Address.Equals(that.Address) &&
                this.Options.SetEquals(that.Options);
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(Object that) {
            return Equals(that as SocketInfo);
        }

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return
                Address.GetHashCode() ^
            //    Options.GetHashCode() ^
                Flags.GetHashCode() ^
                Family.GetHashCode() ^
                Protocol.GetHashCode() ^
                Type.GetHashCode();
        }
    }
}