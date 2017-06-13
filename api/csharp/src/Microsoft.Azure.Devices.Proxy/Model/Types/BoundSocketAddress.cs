// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Runtime.Serialization;

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
                IsEqual(LocalAddress, that.LocalAddress) &&
                IsEqual(RemoteAddress, that.RemoteAddress);
        }

        public override bool IsEqual(object that) => Equals(that as BoundSocketAddress);

        protected override void SetHashCode() {
            MixToHash(LocalAddress);
            MixToHash(RemoteAddress);
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
    }
}