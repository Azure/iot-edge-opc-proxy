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
                IsEqual(that as MulticastOption) &&
                IsEqual(Address, that.Address) &&
                IsEqual(ScopeId, that.ScopeId);
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool IsEqual(object that) =>
            Equals(that as Inet6MulticastOption);

        protected override void SetHashCode() {
            base.SetHashCode();
            MixToHash(Address);
            MixToHash(ScopeId);
        }
    }
}