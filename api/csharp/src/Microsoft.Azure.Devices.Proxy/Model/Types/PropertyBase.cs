// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Runtime.Serialization;

    /// <summary>
    /// Property base class - wrapper for serveral property types, 
    /// including socket option values and dns records. Describes
    /// properties of a remote item (e.g. socket, file, etc.)
    /// </summary>
    [DataContract]
    public class PropertyBase : Serializable<PropertyBase> {

        /// <summary>
        /// Property type
        /// </summary>
        [DataMember(Name = "type", Order = 1)]
        public uint Type { get; set; }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="type"></param>
        public PropertyBase(uint type) {
            Type = type;
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool IsEqual(PropertyBase that) =>
            IsEqual(Type, that.Type);

        protected override void SetHashCode() =>
            MixToHash(Type);
    }
}