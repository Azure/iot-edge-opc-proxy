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
    public class Inet4MulticastOption : MulticastOption, IEquatable<Inet4MulticastOption> {

        [DataMember(Name = "family", Order = 1)]
        public override AddressFamily Family => AddressFamily.InterNetwork;

        /// <summary>
        /// Returns the 32 bit address as a buffer
        /// </summary>
        [DataMember(Name = "addr", Order = 3)]
        public byte[] Address { get; set; }

        /// <summary>
        /// Returns address as 32 bit int
        /// </summary>
        /// <returns></returns>
        public uint ToUInt32() => BitConverter.ToUInt32(Address, 0);

        public bool Equals(Inet4MulticastOption that) {
            if (that == null) {
                return false;
            }
            return
                IsEqual(that as MulticastOption) &&
                IsEqual(ToUInt32(), that.ToUInt32());
        }

        public override bool IsEqual(object that) => Equals(that as Inet4MulticastOption);

        protected override void SetHashCode() {
            base.SetHashCode();
            MixToHash(ToUInt32());
        }
    }
}