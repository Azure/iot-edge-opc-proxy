// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Linq;
    using System.Collections.Generic;
    using System.Runtime.Serialization;
    using Model;
    using System.Text;

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
        public override bool Equals(object that) {
            return Equals(that as NullSocketAddress);
        }

        /// <summary>
        /// Stringify address
        /// </summary>
        /// <returns></returns>
        public override string ToString() {
            return "null";
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
        public string Path { get; set; } = "";

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
        public override bool Equals(object that) {
            return Equals(that as UnixSocketAddress);
        }

        /// <summary>
        /// Stringify address
        /// </summary>
        /// <returns></returns>
        public override string ToString() {
            return Path;
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
        public uint Flow { get; set; }

        /// <summary>
        /// Port in host byte order
        /// </summary>
        [DataMember(Name = "port", Order = 3)]
        public ushort Port { get; set; }

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
        public Inet4SocketAddress(uint address, ushort port, uint flow) :
            this(BitConverter.GetBytes(address), port, flow) {
        }

        /// <summary>
        /// Sets property values
        /// </summary>
        public Inet4SocketAddress(byte[] address, ushort port, uint flow) : this() {
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
        public uint ToUInt32() => BitConverter.ToUInt32(Address, 0);

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
        public override bool Equals(object that) {
            return Equals(that as Inet4SocketAddress);
        }

        /// <summary>
        /// Stringify address
        /// </summary>
        /// <returns></returns>
        public override string ToString() {
            return $"{Address[0]}.{Address[1]}.{Address[2]}.{Address[3]}:{Port}";
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
            ushort port, uint flow, uint scopeId) : this() {
            Address = address;
            Port = port;
            Flow = flow;
            ScopeId = scopeId;
        }

        /// <summary>
        /// Buffer constructor
        /// </summary>
        /// <param name="address"></param>
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
        public uint ScopeId { get; set; }

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
        public override bool Equals(object that) {
            return Equals(that as Inet6SocketAddress);
        }

        /// <summary>
        /// Stringify address
        /// </summary>
        /// <returns></returns>
        public override string ToString() {
            if (Port == 0 && ScopeId == 0 && Flow == 0) {
                // This is actually a proxy reference id
                return ToReference().ToString();
            }
            else {
                var str = new StringBuilder();
                if (Port != 0) {
                    str.Append("[");
                }
                for (int i = 0; i < Address.Length; i += 2) {
                    if (i != 0) {
                        str.Append(":");
                    }
                    // Omit any fancy ipv6 formatting, it is not needed...
                    str.Append(BitConverter.ToUInt16(Address, i).ToString("X"));
                }
                if (ScopeId != 0) {
                    str.Append("/");
                    str.Append(ScopeId);
                }
                if (Port != 0) {
                    str.Append("]:");
                    str.Append(Port);
                }
                return str.ToString();
            }
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
                throw new ArgumentNullException(nameof(host));
            }
            if (port <= 0 || port > ushort.MaxValue) {
                throw new ArgumentNullException(nameof(port));
            }
            Host = host;
            Port = (ushort)port;
            Flow = 0;
        }

        /// <summary>
        /// Sets property values
        /// </summary>
        public ProxySocketAddress(string host, ushort port, uint flow) : this() {
            Port = port;
            Flow = flow;
            Host = host;
        }

        /// <summary>
        /// Creates an address from a proxy socket address string representation
        /// </summary>
        public ProxySocketAddress(string address) : this() {
            int index = address.IndexOf(':');
            if (index <= 0) {
                throw new ArgumentException($"{address} is not a proxy address!");
            }
            Host = address.Substring(0, index);
            Port = ushort.Parse(address.Substring(index + 1));
            Flow = 0;
        }

        /// <summary>
        /// Host name to use
        /// </summary>
        [DataMember(Name = "host", Order = 4)]
        public string Host { get; set; } = "";

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
        public override bool Equals(object that) {
            return Equals(that as ProxySocketAddress);
        }

        /// <summary>
        /// Stringify address
        /// </summary>
        /// <returns></returns>
        public override string ToString() {
            return $"{Host}:{Port}";
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

    /// <summary>
    /// A list of proxy addresses
    /// </summary>
    [DataContract]
    public class SocketAddressCollection : SocketAddress, IEquatable<SocketAddressCollection> {

        /// <summary>
        /// Default constructor
        /// </summary>
        public SocketAddressCollection() {
            base.Family = AddressFamily.Collection;
        }

        /// <summary>
        /// Create an address from a list
        /// </summary>
        /// <param name="addresses"></param>
        /// <returns></returns>
        public static SocketAddress Create(IEnumerable<SocketAddress> addresses) {
            var set = new HashSet<SocketAddress>(addresses);
            if (!set.Any()) {
                return null;
            } else {
                var address = new SocketAddressCollection {
                    Inner = set
                };
                try {
                    return address.Addresses().Single();
                } catch {
                    return address;
                }
            }
        }

        /// <summary>
        /// Storage of socket addresses
        /// </summary>
        [DataMember(Name = "includes", Order = 2)]
        HashSet<SocketAddress> Inner { get; set; }

        /// <summary>
        /// Return flattened list of addresses
        /// </summary>
        /// <returns></returns>
        public IEnumerable<SocketAddress> Addresses() {
            foreach(var address in Inner) {
                var collection = address as SocketAddressCollection;
                if (collection != null) {
                    foreach (var r in collection.Addresses()) {
                        yield return r;
                    }
                }
                else if (!(address is NullSocketAddress)) {
                    yield return address;
                }
            }
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(SocketAddressCollection that) {
            if (that == null) {
                return false;
            }
            return
                this.Equals(that as SocketAddress) &&
                this.Inner.SetEquals(that.Inner);
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(object that) {
            return Equals(that as SocketAddressCollection);
        }

        /// <summary>
        /// Stringify address
        /// </summary>
        /// <returns></returns>
        public override string ToString() {
            var bld = new StringBuilder();
            foreach(var a in Addresses()) {
                bld.AppendLine(a.ToString());
            }
            return bld.ToString();
        }

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return
                Inner.GetHashCode();
        }
    }
}