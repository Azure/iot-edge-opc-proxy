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
    public class Inet4MulticastOption : Poco<Inet4MulticastOption>, IMulticastOption {

        [DataMember(Name = "family", Order = 1)]
        public AddressFamily Family {
            get => AddressFamily.InterNetwork;
        }

        /// <summary>
        /// Interface to use
        /// </summary>
        [DataMember(Name = "itf_index", Order = 2)]
        public int InterfaceIndex {
            get; set;
        }

        /// <summary>
        /// Returns the 32 bit address as a buffer
        /// </summary>
        [DataMember(Name = "addr", Order = 3)]
        public byte[] Address {
            get; set;
        }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="interfaceIndex"></param>
        /// <param name="address"></param>
        /// <returns></returns>
        public static Inet4MulticastOption Create(int interfaceIndex, byte[] address) {
            var option = Get();
            option.InterfaceIndex = interfaceIndex;
            option.Address = address;
            return option;
        }

        /// <summary>
        /// Returns address as 32 bit int
        /// </summary>
        /// <returns></returns>
        public uint ToUInt32() => BitConverter.ToUInt32(Address, 0);

        public override bool IsEqual(Inet4MulticastOption that) {
            return
                IsEqual(Family, that.Family) &&
                IsEqual(InterfaceIndex, that.InterfaceIndex) &&
                IsEqual(ToUInt32(), that.ToUInt32());
        }

        protected override void SetHashCode() {
            MixToHash(Family);
            MixToHash(InterfaceIndex);
            MixToHash(ToUInt32());
        }
    }
}