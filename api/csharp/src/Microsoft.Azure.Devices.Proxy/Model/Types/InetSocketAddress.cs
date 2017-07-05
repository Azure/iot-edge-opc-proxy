// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Runtime.Serialization;

    /// <summary>
    /// IP socket address, base class
    /// </summary>
    [DataContract]
    public abstract class InetSocketAddress : SocketAddress, IEquatable<InetSocketAddress> {

        [DataMember(Name = "family", Order = 1)]
        public abstract override AddressFamily Family {
            get;
        }

        /// <summary>
        /// Port in host byte order
        /// </summary>
        [DataMember(Name = "port", Order = 2)]
        public ushort Port {
            get; set;
        }

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
                IsEqual(that as SocketAddress) &&
                IsEqual(Port, that.Port);
        }

        public override bool IsEqual(object that) => Equals(that as InetSocketAddress);

        protected override void SetHashCode() {
            MixToHash(Family);
            MixToHash(Port);
        }

        /// <summary>
        /// Parse a string into a inet 4 address
        /// </summary>
        /// <param name="address"></param>
        /// <param name="parsed"></param>
        /// <returns></returns>
        public static bool TryParse(string address, out InetSocketAddress parsed) {
            if (Inet4SocketAddress.TryParse(address, out Inet4SocketAddress inet4)) {
                parsed = inet4;
            }
            else if (Inet6SocketAddress.TryParse(address, out Inet6SocketAddress inet6)) {
                parsed = inet6;
            }
            else {
                parsed = null;
            }
            return parsed != null;
        }
    }
}