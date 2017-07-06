// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Runtime.Serialization;

    /// <summary>
    /// Unix socket address
    /// </summary>
    [DataContract]
    public class UnixSocketAddress : SocketAddress, IEquatable<UnixSocketAddress> {

        [DataMember(Name = "family", Order = 1)]
        public override AddressFamily Family {
            get => AddressFamily.Unix;
        }

        /// <summary>
        /// Path to the unix pipe
        /// </summary>
        [DataMember(Name = "path", Order = 2)]
        public string Path {
            get; set;
        } = "";

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
                IsEqual(that as SocketAddress) &&
                IsEqual(Path, that.Path);
        }

        public override bool IsEqual(object that) => Equals(that as UnixSocketAddress);

        protected override void SetHashCode() {
            MixToHash(Family);
            MixToHash(Path);
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
    }
}