// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Model {
    using System;
    using System.Collections.Generic;
    using System.Runtime.Serialization;

    [Flags]
    public enum RecordType {

        Hub = 0x0,
        Host = 0x1,
        Proxy = 0x2,
        Startup = 0x4,
        Link = 0x8,
        All = 0xf
    }

    /// <summary>
    /// Name service entry 
    /// </summary>
    [DataContract]
    public class Record : INameRecord {
        /// <summary>
        /// Name of entry
        /// </summary>
        [DataMember(Name = "name", Order = 1)]
        public string Name { get; set; }

        /// <summary>
        /// Address of entry
        /// </summary>
        [DataMember(Name = "addr", Order = 2)]
        public Reference Address { get; set; }

        /// <summary>
        /// Connection string for the entry
        /// </summary>
        [DataMember(Name = "cs", Order = 3)]
        public string ConnectionString { get; set; }

        /// <summary>
        /// Type of entry
        /// </summary>
        [DataMember(Name = "type", Order = 4)]
        public RecordType Type { get; set; }

        /// <summary>
        /// Id of entry
        /// </summary>
        [DataMember(Name = "id", Order = 5)]
        public string Id { get; set; }

        /// <summary>
        /// References for this entry to support serializing
        /// </summary>
        [DataMember(Name = "references", Order = 6)]
        public HashSet<Reference> ReferenceSet { get; set; } = new HashSet<Reference>();

        /// <summary>
        /// References as enumerable
        /// </summary>
        public IEnumerable<Reference> References => ReferenceSet;

        /// <summary>
        /// Default constructor
        /// </summary>
        public Record() {
            // no op
        }

        /// <summary>
        /// Copy constructor for record
        /// </summary>
        /// <param name="record"></param>
        public Record(INameRecord record) {
            Address = record.Address;
            Type = record.Type;
            Id = record.Id;
            Name = record.Name;
            ReferenceSet.AddRange(record.References);
        }

        /// <summary>
        /// Construct a new record
        /// </summary>
        /// <param name="type"></param>
        /// <param name="name"></param>
        /// <param name="address"></param>
        public Record(RecordType type, string name, Reference address = null) {
            Name = name;
            Id = name;
            Address = address ?? new Reference();
            Type = type;
        }

        /// <summary>
        /// Add a address
        /// </summary>
        /// <param name="address"></param>
        public void AddReference(Reference address) {
            ReferenceSet.Add(address);
        }

        /// <summary>
        /// Add a address
        /// </summary>
        /// <param name="address"></param>
        public void RemoveReference(Reference address) {
            ReferenceSet.Remove(address);
        }

        /// <summary>
        /// Copies members from passed in record
        /// </summary>
        /// <param name="record"></param>
        public INameRecord Assign(INameRecord record) {
            if (!Id.Equals(record.Id)) {
                return record;
            }

            Type = record.Type;
            Name = record.Name;

            ReferenceSet.Clear();
            ReferenceSet.AddRange(record.References);
            return this;
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(INameRecord that) {
            if (that == null) {
                return false;
            }
            return
                Address.Equals(that.Address) &&
                Type.Equals(that.Type) &&
                Id.Equals(that.Id) &&
                Name.Equals(that.Name, StringComparison.CurrentCultureIgnoreCase) &&
                ReferenceSet.SetEquals(that.References)
                ;
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(Object that) {
            return Equals(that as INameRecord);
        }

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return Address.GetHashCode();
        }
    }
}
