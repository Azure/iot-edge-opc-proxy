// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System;
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
        Tcp_nodelay,
        // ...
        __pi_so_max
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
    /// Socket option value
    /// </summary>
    [DataContract]
    public class SocketOptionValue : IEquatable<SocketOptionValue> {

        /// <summary>
        /// Default constructor
        /// </summary>
        public SocketOptionValue() {
        }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="option"></param>
        /// <param name="value"></param>
        public SocketOptionValue(SocketOption option, UInt64 value) {
            Option = option;
            Value = value;
        }

        /// <summary>
        /// Option name
        /// </summary>
        [DataMember(Name = "option", Order = 1)]
        public SocketOption Option { get; set; } = SocketOption.Unknown;

        /// <summary>
        /// Option value
        /// </summary>
        [DataMember(Name = "value", Order = 2)]
        public UInt64 Value { get; set; }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(SocketOptionValue that) {
            if (that == null) {
                return false;
            }
            return
                this.Option.Equals(that.Option) &&
                this.Value.Equals(that.Value);
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(Object that) {
            return Equals(that as SocketOptionValue);
        }

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return Value.GetHashCode() ^ Option.GetHashCode();
        }
    }
}