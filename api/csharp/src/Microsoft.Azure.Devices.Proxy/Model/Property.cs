// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy.Model {
    using System;
    using System.Collections;
    using System.Runtime.Serialization;

    internal enum PropertyType {
        FileInfo = 200,
        AddressInfo,
        InterfaceInfo,

        // ...
        __prx_property_type_max
    }

    /// <summary>
    /// Property base class - wrapper for serveral property types, 
    /// including socket option values and dns records. Describes
    /// properties of a remote item (e.g. socket, file, etc.)
    /// </summary>
    [DataContract]
    public class PropertyBase : IEquatable<PropertyBase> {

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
        public bool Equals(PropertyBase that) {
            if (that == null) {
                return false;
            }
            return
                this.Type.Equals(that.Type);
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(Object that) {
            return Equals(that as PropertyBase);
        }

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return Type.GetHashCode();
        }
    }

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
        public Property() : this (0, default(T)) {
        }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="type"></param>
        /// <param name="value"></param>
        public Property(uint type, T value) : base(type) {
            Value = value;
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(Property<T> that) {
            if (that == null) {
                return false;
            }
            return
                base.Equals(that) &&
                StructuralComparisons.StructuralEqualityComparer.Equals(
                    this.Value, that.Value);
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(Object that) {
            return Equals(that as Property<T>);
        }

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return (Value.GetHashCode() * 31) ^ base.GetHashCode();
        }
    }
}