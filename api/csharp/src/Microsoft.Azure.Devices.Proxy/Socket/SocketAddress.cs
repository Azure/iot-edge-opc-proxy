// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Linq;
    using System.Runtime.Serialization;
    using Model;

    /// <summary>
    /// Socket address base class
    /// </summary>
    [DataContract]
    public abstract class SocketAddress : IEquatable<SocketAddress> {

        /// <summary>
        /// Address family
        /// </summary>
        [DataMember(Name = "family", Order = 1)]
        public AddressFamily Family { get; set; }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(SocketAddress that) {
            if (that == null) {
                return false;
            }
            return this.Family.Equals(that.Family);
        }
    }


    /// <summary>
    /// A null socket address (unspecified)
    /// </summary>
    [DataContract]
    public class NullSocketAddress : SocketAddress, IEquatable<NullSocketAddress> {

        /// <summary>
        /// Default constructor
        /// </summary>
        public NullSocketAddress() {
            base.Family = AddressFamily.Unspecified;
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(NullSocketAddress that) {
            if (that == null) {
                return false;
            }
            return
                this.Family.Equals(AddressFamily.Unspecified) &&
                that.Family.Equals(AddressFamily.Unspecified);
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(Object that) {
            return Equals(that as NullSocketAddress);
        }

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return (int)AddressFamily.Unspecified;
        }
    }

    /// <summary>
    /// Unix socket address
    /// </summary>
    [DataContract]
    public class UnixSocketAddress : SocketAddress, IEquatable<UnixSocketAddress> {

        /// <summary>
        /// Default constructor
        /// </summary>
        public UnixSocketAddress() {
            base.Family = AddressFamily.Unix;
        }

        /// <summary>
        /// Default constructor
        /// </summary>
        public UnixSocketAddress(string path) : this() {
            Path = path;
        }

        /// <summary>
        /// Path to the unix pipe
        /// </summary>
        [DataMember(Name = "path", Order = 2)]
        public String Path { get; set; } = "";

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(UnixSocketAddress that) {
            if (that == null) {
                return false;
            }
            return
                this.Equals(that as SocketAddress) &&
                this.Path.Equals(that.Path);
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(Object that) {
            return Equals(that as UnixSocketAddress);
        }

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return Path.GetHashCode();
        }
    }

    /// <summary>
    /// IP socket address, base class
    /// </summary>
    [DataContract]
    public abstract class InetSocketAddress : SocketAddress, IEquatable<InetSocketAddress> {

        /// <summary>
        /// Flow
        /// </summary>
        [DataMember(Name = "flow", Order = 2)]
        public UInt32 Flow { get; set; }

        /// <summary>
        /// Port in host byte order
        /// </summary>
        [DataMember(Name = "port", Order = 3)]
        public UInt16 Port { get; set; }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(InetSocketAddress that) {
            if (that == null) {
                return false;
            }
            return
                this.Equals(that as SocketAddress) &&
                this.Flow.Equals(that.Flow) &&
                this.Port.Equals(that.Port);
        }
    }

    /// <summary>
    /// IPv4 socket address, base class
    /// </summary>
    [DataContract]
    public class Inet4SocketAddress : InetSocketAddress, IEquatable<Inet4SocketAddress> {

        /// <summary>
        /// Default constructor
        /// </summary>
        public Inet4SocketAddress() {
            base.Family = AddressFamily.InterNetwork;
        }

        /// <summary>
        /// Sets property values
        /// </summary>
        public Inet4SocketAddress(UInt32 address, UInt16 port, UInt32 flow) : 
            this(BitConverter.GetBytes(address), port, flow) {
        }

        /// <summary>
        /// Sets property values
        /// </summary>
        public Inet4SocketAddress(byte[] address, UInt16 port, UInt32 flow) : this() {
            Address = address;
            Port = port;
            Flow = flow;
        }

        /// <summary>
        /// Returns the 32 bit address as a buffer
        /// </summary>
        [DataMember(Name = "addr", Order = 4)]
        public byte[] Address { get; set; }

        /// <summary>
        /// Returns address as 32 bit int
        /// </summary>
        /// <returns></returns>
        public UInt32 ToUInt32() => BitConverter.ToUInt32(Address, 0);

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(Inet4SocketAddress that) {
            if (that == null) {
                return false;
            }
            return
                this.Equals(that as InetSocketAddress) &&
                this.ToUInt32().Equals(that.ToUInt32());
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(Object that) {
            return Equals(that as Inet4SocketAddress);
        }

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return
                (int)ToUInt32() ^
                Port.GetHashCode() ^
                Flow.GetHashCode();
        }
    }

    /// <summary>
    /// Ipv6 address
    /// </summary>
    [DataContract]
    public class Inet6SocketAddress : InetSocketAddress, IEquatable<Inet6SocketAddress> {

        /// <summary>
        /// Default constructor
        /// </summary>
        public Inet6SocketAddress() : this(new byte[16]) {
        }

        /// <summary>
        /// Sets property values
        /// </summary>
        public Inet6SocketAddress(byte[] address,
            UInt16 port, UInt32 flow, UInt32 scopeId) : this() {
            Address = address;
            Port = port;
            Flow = flow;
            ScopeId = scopeId;
        }

        /// <summary>
        /// Buffer constructor
        /// </summary>
        /// <param name="buffer"></param>
        public Inet6SocketAddress(byte[] address) {
            base.Family = AddressFamily.InterNetworkV6;
            Address = address;
        }

        /// <summary>
        /// Serialization only
        /// </summary>
        [DataMember(Name = "addr", Order = 4)]
        public byte[] Address { get; set; }


        /// <summary>
        /// Scope id
        /// </summary>
        [DataMember(Name = "scope_id", Order = 5)]
        public UInt32 ScopeId { get; set; }

        /// <summary>
        /// Converts this socket address to a address
        /// </summary>
        /// <returns></returns>
        public Reference ToReference() {
            return new Reference(Address);
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(Inet6SocketAddress that) {
            if (that == null) {
                return false;
            }
            return
                this.Equals(that as InetSocketAddress) &&
                this.Address.SequenceEqual(that.Address) &&
                this.ScopeId.Equals(that.ScopeId);
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(Object that) {
            return Equals(that as Inet6SocketAddress);
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
                Port.GetHashCode() ^
                Flow.GetHashCode() ^
                ScopeId.GetHashCode();
        }
    }

    /// <summary>
    /// Proxy socket address (prx_address_family_proxy)
    /// </summary>
    [DataContract]
    public class ProxySocketAddress : InetSocketAddress, IEquatable<ProxySocketAddress> {

        /// <summary>
        /// Default constructor
        /// </summary>
        public ProxySocketAddress() {
            base.Family = AddressFamily.Proxy;
        }

        /// <summary>
        /// Create proxy socket address
        /// </summary>
        /// <param name="host"></param>
        /// <param name="port"></param>
        public ProxySocketAddress(string host, int port) : this() {
            if (host == null) {
                throw new ArgumentNullException("host");
            }
            if (port <= 0 || port > ushort.MaxValue) {
                throw new ArgumentNullException("port");
            }
            Host = host;
            Port = (UInt16)port;
            Flow = 0;
        }

        /// <summary>
        /// Sets property values
        /// </summary>
        public ProxySocketAddress(string host, UInt16 port, UInt32 flow) : this() {
            Port = port;
            Flow = flow;
            Host = host;
        }

        /// <summary>
        /// Host name to use
        /// </summary>
        [DataMember(Name = "host", Order = 4)]
        public String Host { get; set; } = "";

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(ProxySocketAddress that) {
            if (that == null) {
                return false;
            }
            return
                this.Equals(that as InetSocketAddress) &&
                this.Host.Equals(that.Host);
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(Object that) {
            return Equals(that as ProxySocketAddress);
        }

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return
                Host.GetHashCode() ^
                Port.GetHashCode() ^
                Flow.GetHashCode();
        }
    }
}