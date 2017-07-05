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
    public class Inet6MulticastOption : Poco<Inet6MulticastOption>, IMulticastOption {

        [DataMember(Name = "family", Order = 1)]
        public AddressFamily Family {
            get => AddressFamily.InterNetworkV6;
        }

        /// <summary>
        /// Interface to use
        /// </summary>
        [DataMember(Name = "itf_index", Order = 2)]
        public int InterfaceIndex {
            get; set;
        }

        /// <summary>
        /// Bind address
        /// </summary>
        [DataMember(Name = "addr", Order = 3)]
        public byte[] Address {
            get; set;
        }

        /// <summary>
        /// Scope id
        /// </summary>
        public uint ScopeId => InterfaceIndex > 0 ? (uint)InterfaceIndex : 0;

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="interfaceIndex"></param>
        /// <param name="address"></param>
        /// <returns></returns>
        public static Inet6MulticastOption Create(int interfaceIndex, byte[] address) {
            var option = Get();
            option.InterfaceIndex = interfaceIndex;
            option.Address = address;
            return option;
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool IsEqual(Inet6MulticastOption that) {
            return
                IsEqual(Family, that.Family) &&
                IsEqual(InterfaceIndex, that.InterfaceIndex) &&
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
            MixToHash(Family);
            MixToHash(InterfaceIndex);
            MixToHash(Address);
            MixToHash(ScopeId);
        }
    }
}