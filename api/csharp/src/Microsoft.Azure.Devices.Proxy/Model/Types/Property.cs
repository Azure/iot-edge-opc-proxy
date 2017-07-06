// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System.Collections;
    using System.Runtime.Serialization;

    /// <summary>
    /// Generic Property object.
    /// </summary>
    [DataContract]
    public class Property<T> : Poco<Property<T>>, IProperty<T> {

        /// <summary>
        /// Property Type
        /// </summary>
        [DataMember(Name = "type", Order = 1)]
        public uint Type {
            get; set;
        }

        /// <summary>
        /// Property value
        /// </summary>
        [DataMember(Name = "property", Order = 2)]
        public T Value {
            get; set;
        }

        /// <summary>
        /// Create property
        /// </summary>
        /// <param name="type"></param>
        public static Property<T> Create(uint type) =>
            Create(type, default(T));

        /// <summary>
        /// Create property
        /// </summary>
        /// <param name="type"></param>
        /// <param name="value"></param>
        public static Property<T> Create(uint type, T value) {
            var prop = Get();
            prop.Type = type;
            prop.Value = value;
            return prop;
        }

        public override bool IsEqual(Property<T> that) {
            return
                IsEqual(Type, that.Type) &&
                StructuralComparisons.StructuralEqualityComparer.Equals(Value, that.Value);
        }

        protected override void SetHashCode() {
            MixToHash(Type);
            MixToHash(Value);
        }
    }
}