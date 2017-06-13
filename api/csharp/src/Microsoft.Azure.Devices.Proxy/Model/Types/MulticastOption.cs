// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Runtime.Serialization;

    /// <summary>
    /// Multi cast socket option
    /// </summary>
    [DataContract]
    public abstract class MulticastOption : Serializable<MulticastOption> {

        /// <summary>
        /// Group address
        /// </summary>
        [DataMember(Name = "family", Order = 1)]
        public abstract AddressFamily Family { get; }

        /// <summary>
        /// Interface to use
        /// </summary>
        [DataMember(Name = "itf_index", Order = 2)]
        public int InterfaceIndex { get; set; }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool IsEqual(MulticastOption that) {
            return IsEqual(Family, that.Family) && 
                IsEqual(InterfaceIndex, that.InterfaceIndex);
        }

        protected override void SetHashCode() {
            MixToHash(Family);
            MixToHash(InterfaceIndex);
        }
    }
}