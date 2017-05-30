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
        public abstract AddressFamily Family { get; }

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

        public abstract ProxySocketAddress AsProxySocketAddress();
    }


    /// <summary>
    /// A null socket address (unspecified)
    /// </summary>
    [DataContract]
    public class NullSocketAddress : SocketAddress, IEquatable<NullSocketAddress> {

        [DataMember(Name = "family", Order = 1)]
        public override AddressFamily Family => AddressFamily.Unspecified;

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(NullSocketAddress that) =>
            base.Equals(that);
        
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

        public override ProxySocketAddress AsProxySocketAddress() =>
            new ProxySocketAddress();
    }

    /// <summary>
    /// Unix socket address
    /// </summary>
    [DataContract]
    public class UnixSocketAddress : SocketAddress, IEquatable<UnixSocketAddress> {

        [DataMember(Name = "family", Order = 1)]
        public override AddressFamily Family => AddressFamily.Unix;

        /// <summary>
        /// Path to the unix pipe
        /// </summary>
        [DataMember(Name = "path", Order = 2)]
        public string Path { get; set; } = "";

        /// <summary>
        /// Default constructor
        /// </summary>
        public UnixSocketAddress() {
        }

        /// <summary>
        /// Default constructor
        /// </summary>
        public UnixSocketAddress(string path) : this() {
            Path = path;
        }

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

        public override ProxySocketAddress AsProxySocketAddress() =>
            new ProxySocketAddress(Path);

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

        [DataMember(Name = "family", Order = 1)]
        public abstract override AddressFamily Family { get; }

        /// <summary>
        /// Port in host byte order
        /// </summary>
        [DataMember(Name = "port", Order = 2)]
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
                this.Port.Equals(that.Port);
        }
    }

    /// <summary>
    /// IPv4 socket address, base class
    /// </summary>
    [DataContract]
    public class Inet4SocketAddress : InetSocketAddress, IEquatable<Inet4SocketAddress> {

        [DataMember(Name = "family", Order = 1)]
        public override AddressFamily Family => AddressFamily.InterNetwork;

        /// <summary>
        /// Returns the 32 bit address as a buffer
        /// </summary>
        [DataMember(Name = "addr", Order = 3)]
        public byte[] Address { get; set; }

        /// <summary>
        /// Default constructor
        /// </summary>
        public Inet4SocketAddress() {
        }

        /// <summary>
        /// Sets property values
        /// </summary>
        public Inet4SocketAddress(uint address, ushort port) :
            this(BitConverter.GetBytes(address), port) {
        }

        /// <summary>
        /// Sets property values
        /// </summary>
        public Inet4SocketAddress(byte[] address, ushort port) : this() {
            Address = address;
            Port = port;
        }

        /// <summary>
        /// Returns address as 32 bit int
        /// </summary>
        /// <returns></returns>
        public uint AsUInt32() => BitConverter.ToUInt32(Address, 0);

        /// <summary>
        /// Stringify address
        /// </summary>
        /// <returns></returns>
        public string AsString() {
            return $"{Address[0]}.{Address[1]}.{Address[2]}.{Address[3]}";
        }

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
                this.AsUInt32().Equals(that.AsUInt32());
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
            return AsString() + $":{Port}";
        }

        public override ProxySocketAddress AsProxySocketAddress() =>
            new ProxySocketAddress(AsString(), Port);

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return
                (int)AsUInt32() ^
                Port.GetHashCode();
        }
    }

    /// <summary>
    /// Ipv6 address
    /// </summary>
    [DataContract]
    public class Inet6SocketAddress : InetSocketAddress, IEquatable<Inet6SocketAddress> {

        [DataMember(Name = "family", Order = 1)]
        public override AddressFamily Family => AddressFamily.InterNetworkV6;

        /// <summary>
        /// Flow
        /// </summary>
        [DataMember(Name = "flow", Order = 3)]
        public uint Flow { get; set; }

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
            Address = address;
        }

        /// <summary>
        /// Converts this socket address to a address
        /// </summary>
        /// <returns></returns>
        public Reference AsReference() {
            return new Reference(Address);
        }

        /// <summary>
        /// Returns the address as string
        /// </summary>
        /// <returns></returns>
        public string AsString() {
            var str = new StringBuilder();
            for (int i = 0; i < Address.Length; i += 2) {
                if (i != 0) {
                    str.Append(":");
                }
                // Omit any fancy ipv6 formatting, it is not needed...
                str.Append(BitConverter.ToUInt16(Address, i).ToString("X"));
            }
            return str.ToString();
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
                return AsReference().ToString();
            }
            else {
                var str = new StringBuilder();
                if (Port != 0) {
                    str.Append("[");
                }
                str.Append(AsString());
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

        public override ProxySocketAddress AsProxySocketAddress() =>
            new ProxySocketAddress(AsString(), Port);

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            int result = 0;
            foreach (byte b in Address)
                result = (result * 31) ^ b;
            return (((
                result * 31) ^
                Port.GetHashCode() * 31) ^
                Flow.GetHashCode() * 31) ^
                ScopeId.GetHashCode();
        }
    }

    /// <summary>
    /// Proxy socket address (prx_address_family_proxy)
    /// </summary>
    [DataContract]
    public class ProxySocketAddress : InetSocketAddress, IEquatable<ProxySocketAddress> {

        [DataMember(Name = "family", Order = 1)]
        public override AddressFamily Family => AddressFamily.Proxy;

        /// <summary>
        /// Interface Index field
        /// </summary>
        [DataMember(Name = "itf_index", Order = 3)]
        public int InterfaceIndex { get; set; } = -1;

        /// <summary>
        /// Interface Index field
        /// </summary>
        [DataMember(Name = "flags", Order = 4)]
        public ushort Flags { get; set; }

        /// <summary>
        /// Host name to use
        /// </summary>
        [DataMember(Name = "host", Order = 5)]
        public string Host { get; set; } = "";

        /// <summary>
        /// Default constructor
        /// </summary>
        public ProxySocketAddress() {
        }

        /// <summary>
        /// Sets property values
        /// </summary>
        public ProxySocketAddress(string host, ushort port = 0,
            ushort flags = 0, int interfaceIndex = -1) : this() {
            if (host == null) {
                throw new ArgumentNullException(nameof(host));
            }
            Port = port;
            Flags = flags;
            Host = host;
            InterfaceIndex = interfaceIndex;
        }

        /// <summary>
        /// Sets property values
        /// </summary>
        public ProxySocketAddress(string host, int port) : this(host, (ushort)port) {
            if (port < 0 || port > ushort.MaxValue) {
                throw new ArgumentNullException(nameof(port));
            }
        }

        /// <summary>
        /// Creates an address from a proxy socket address string representation
        /// </summary>
        public static ProxySocketAddress Parse(string address) {
            string host;
            ushort port;
            int index = address.IndexOf(':');
            if (index <= 0) {
                host = address;
                port = 0;
            }
            else {
                host = address.Substring(0, index);
                port = ushort.Parse(address.Substring(index + 1));
            }
            return new ProxySocketAddress(host, port, 0);
        }

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

        public override ProxySocketAddress AsProxySocketAddress() => this;

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
            return (((
                Host.GetHashCode() * 31) ^
                Port.GetHashCode() * 31) ^
                InterfaceIndex.GetHashCode() * 31) ^
                Flags.GetHashCode();
        }
    }


    /// <summary>
    /// A list of socket addresses
    /// </summary>
    [DataContract]
    public class SocketAddressCollection : SocketAddress, IEquatable<SocketAddressCollection> {

        [DataMember(Name = "family", Order = 1)]
        public override AddressFamily Family => AddressFamily.Collection;

        /// <summary>
        /// Storage of socket addresses
        /// </summary>
        [DataMember(Name = "includes", Order = 2)]
        HashSet<SocketAddress> Inner { get; set; }

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
        /// Returns the first address if any as proxy address
        /// </summary>
        /// <returns></returns>
        public override ProxySocketAddress AsProxySocketAddress() =>
            Addresses().First().AsProxySocketAddress();

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return
                Inner.GetHashCode();
        }
    }

    /// <summary>
    /// A socket address bound to a proxy
    /// </summary>
    [DataContract]
    public class BoundSocketAddress : SocketAddress, IEquatable<BoundSocketAddress> {

        [DataMember(Name = "family", Order = 1)]
        public override AddressFamily Family => AddressFamily.Bound;

        /// <summary>
        /// Bound addresses
        /// </summary>
        [DataMember(Name = "local", Order = 2)]
        public SocketAddress LocalAddress { get; set; }

        /// <summary>
        /// Host name to use
        /// </summary>
        [DataMember(Name = "remote", Order = 3)]
        public SocketAddress RemoteAddress { get; set; }

        /// <summary>
        /// Default constructor
        /// </summary>
        public BoundSocketAddress() {
        }

        /// <summary>
        /// Bind constructor
        /// </summary>
        public BoundSocketAddress(SocketAddress localAddress, SocketAddress remoteAddress) {
            LocalAddress = localAddress;
            RemoteAddress = remoteAddress;
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(BoundSocketAddress that) {
            if (that == null) {
                return false;
            }
            return
                this.LocalAddress.Equals(that.LocalAddress) &&
                this.RemoteAddress.Equals(that.RemoteAddress);
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(object that) {
            return Equals(that as BoundSocketAddress);
        }

        /// <summary>
        /// Returns the remote address as a proxy address
        /// </summary>
        /// <returns></returns>
        public override ProxySocketAddress AsProxySocketAddress() =>
           RemoteAddress.AsProxySocketAddress();

        /// <summary>
        /// Stringify address
        /// </summary>
        /// <returns></returns>
        public override string ToString() {
            return RemoteAddress.ToString();
        }

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return (RemoteAddress.GetHashCode() * 31) ^ 
                LocalAddress.GetHashCode();
        }
    }
}