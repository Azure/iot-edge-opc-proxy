// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Linq;
    using System.Runtime.Serialization;

    /// <summary>
    /// Socket option name for set and get
    /// </summary>
    public enum SocketOption {
        Unknown,
        Nonblocking,
        Available,
        Shutdown,
        Debug,
        Acceptconn,
        Reuseaddr,
        Keepalive,
        Dontroute,
        Broadcast,
        Linger,
        Oobinline,
        Sndbuf,
        Rcvbuf,
        Sndlowat,
        Rcvlowat,
        Sndtimeo,
        Rcvtimeo,
        Error,
        Type,
        IpOptions,
        IpHdrincl,
        IpTos,
        IpTtl,
        IpMulticasTtl,
        IpMulticastLoop,
        IpPktInfo,
        Ipv6Hoplimit,
        Ipv6ProtectionLevel,
        Ipv6Only,
        TcpNodelay,
        IpMulticastJoin, 
        IpMulticastLeave,

        // ...
        __prx_so_max
    }


    /// <summary>
    /// Shutdown value for Shutdown option
    /// </summary>
    public enum SocketShutdown {
        Read = 0,
        Write = 1,
        Both = 2
    }


    /// <summary>
    /// Multi cast socket option
    /// </summary>
    [DataContract]
    public abstract class MulticastOption : IEquatable<MulticastOption> {

        /// <summary>
        /// Group address
        /// </summary>
        [DataMember(Name = "family", Order = 1)]
        public abstract AddressFamily Family { get; }

        /// <summary>
        /// Interface to use
        /// </summary>
        [DataMember(Name = "itf_index", Order = 2)]
        public int InterfaceIndex { get; set; }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(MulticastOption that) {
            if (that == null) {
                return false;
            }
            return 
                this.Family.Equals(that.Family) && 
                this.InterfaceIndex.Equals(that.InterfaceIndex);
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(object that) {
            return Equals(that as MulticastOption);
        }

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return
                (this.InterfaceIndex * 31) ^ (int)this.Family;
        }
    }

    /// <summary>
    /// Multi cast socket option
    /// </summary>
    [DataContract]
    public class Inet4MulticastOption : MulticastOption, IEquatable<Inet4MulticastOption> {

        [DataMember(Name = "family", Order = 1)]
        public override AddressFamily Family => AddressFamily.InterNetwork;

        /// <summary>
        /// Returns the 32 bit address as a buffer
        /// </summary>
        [DataMember(Name = "addr", Order = 3)]
        public byte[] Address { get; set; }

        /// <summary>
        /// Returns address as 32 bit int
        /// </summary>
        /// <returns></returns>
        public uint ToUInt32() => BitConverter.ToUInt32(Address, 0);
        
        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(Inet4MulticastOption that) {
            if (that == null) {
                return false;
            }
            return
                this.Equals(that as MulticastOption) &&
                this.ToUInt32().Equals(that.ToUInt32());
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(object that) {
            return Equals(that as Inet4MulticastOption);
        }

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return
                ((int)ToUInt32() * 31) ^ base.GetHashCode();
        }
    }

    /// <summary>
    /// Multi cast socket option
    /// </summary>
    [DataContract]
    public class Inet6MulticastOption : MulticastOption, IEquatable<Inet6MulticastOption> {

        [DataMember(Name = "family", Order = 1)]
        public override AddressFamily Family => AddressFamily.InterNetworkV6;

        /// <summary>
        /// Bind address
        /// </summary>
        [DataMember(Name = "addr", Order = 3)]
        public byte[] Address { get; set; }

        /// <summary>
        /// Scope id
        /// </summary>
        public uint ScopeId => InterfaceIndex > 0 ? (uint)InterfaceIndex : 0;

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(Inet6MulticastOption that) {
            if (that == null) {
                return false;
            }
            return
                this.Equals(that as MulticastOption) &&
                this.Address.SequenceEqual(that.Address) &&
                this.ScopeId.Equals(that.ScopeId);
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(object that) {
            return Equals(that as Inet6MulticastOption);
        }

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            int result = 0;
            foreach (byte b in Address)
                result = (result * 31) ^ b;
            return
                result ^
                base.GetHashCode() ^
                    ScopeId.GetHashCode();
        }
    }
}