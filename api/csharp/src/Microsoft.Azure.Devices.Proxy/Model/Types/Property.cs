// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Collections;
    using System.Runtime.Serialization;

    /// <summary>
    /// Generic Property object.
    /// </summary>
    [DataContract]
    public class Property<T> : PropertyBase, IEquatable<Property<T>> {

        /// <summary>
        /// Option value
        /// </summary>
        [DataMember(Name = "property", Order = 2)]
        public T Value { get; set; }

        /// <summary>
        /// Default constructor
        /// </summary>
        public Property() : this (0, default(T)) {}

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="type"></param>
        /// <param name="value"></param>
        public Property(uint type, T value) : base(type) => Value = value;

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(Property<T> that) {
            if (that == null) {
                return false;
            }
            return base.IsEqual(that) &&
                StructuralComparisons.StructuralEqualityComparer.Equals(Value, that.Value);
        }

        public override bool IsEqual(object that) => Equals(that as Property<T>);

        protected override void SetHashCode() {
            MixToHash(Type);
            MixToHash(Value);
        }
    }
}