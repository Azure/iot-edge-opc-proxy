// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Linq;
    using System.Runtime.Serialization;
    using System.Text;

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
        public uint Flow {
            get; set;
        }

        /// <summary>
        /// Serialization only
        /// </summary>
        [DataMember(Name = "addr", Order = 4)]
        public byte[] Address {
            get; set;
        }

        /// <summary>
        /// Scope id
        /// </summary>
        [DataMember(Name = "scope_id", Order = 5)]
        public uint ScopeId {
            get; set;
        }

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
        public Reference AsReference() => new Reference(Address);

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
        /// Parse a string into a inet 4 address
        /// </summary>
        /// <param name="address"></param>
        /// <param name="parsed"></param>
        /// <returns></returns>
        public static bool TryParse(string address, out Inet6SocketAddress parsed) {
            if (Reference.TryParse(address, out Reference reference)) {
                parsed = reference.ToSocketAddress() as Inet6SocketAddress;
            }
            else {

                // TODO

                parsed = null;
            }
            return parsed != null; ;
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
                IsEqual(that as InetSocketAddress) &&
                IsEqual(Address, that.Address) &&
                IsEqual(ScopeId, that.ScopeId);
        }

        public override bool IsEqual(object that) => Equals(that as Inet6SocketAddress);

        protected override void SetHashCode() {
            base.SetHashCode();
            MixToHash(Address);
            MixToHash(Flow);
            MixToHash(ScopeId);
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
    }
}