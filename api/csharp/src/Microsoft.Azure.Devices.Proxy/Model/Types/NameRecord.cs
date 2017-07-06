// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Collections.Generic;
    using System.Runtime.Serialization;

    /// <summary>
    /// Name service entry 
    /// </summary>
    [DataContract]
    public class NameRecord : Poco<NameRecord>, INameRecord {

        /// <summary>
        /// Name of entry
        /// </summary>
        [DataMember(Name = "name", Order = 1)]
        public string Name {
            get; set;
        }

        /// <summary>
        /// Address of entry
        /// </summary>
        [DataMember(Name = "addr", Order = 2)]
        public Reference Address {
            get; set;
        }

        /// <summary>
        /// Connection string for the entry
        /// </summary>
        [DataMember(Name = "cs", Order = 3)]
        public string ConnectionString {
            get; set;
        }

        /// <summary>
        /// Type of entry
        /// </summary>
        [DataMember(Name = "type", Order = 4)]
        public NameRecordType Type {
            get; set;
        }

        /// <summary>
        /// Id of entry
        /// </summary>
        [DataMember(Name = "id", Order = 5)]
        public string Id {
            get; set;
        }

        /// <summary>
        /// References for this entry to support serializing
        /// </summary>
        [DataMember(Name = "references", Order = 6)]
        public HashSet<Reference> ReferenceSet {
            get; set;
        } = new HashSet<Reference>();

        /// <summary>
        /// References as enumerable
        /// </summary>
        public IEnumerable<Reference> References {
            get => ReferenceSet;
        }

        /// <summary>
        /// Default constructor
        /// </summary>
        public NameRecord() {
            // no op
        }

        /// <summary>
        /// Copy constructor for record
        /// </summary>
        /// <param name="record"></param>
        public NameRecord(INameRecord record) {
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
        public NameRecord(NameRecordType type, string name, Reference address = null) {
            Name = name;
            Id = name;
            Address = address ?? new Reference();
            Type = type;
        }

        /// <summary>
        /// Add a address
        /// </summary>
        /// <param name="address"></param>
        public void AddReference(Reference address) => 
            ReferenceSet.Add(address);

        /// <summary>
        /// Add a address
        /// </summary>
        /// <param name="address"></param>
        public void RemoveReference(Reference address) => 
            ReferenceSet.Remove(address);

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
            return
                IsEqual(Address, that.Address) &&
                IsEqual(Type, that.Type) &&
                IsEqual(Id, that.Id) &&
                IsEqual(Name, that.Name) &&
                ReferenceSet.SetEquals(that.References)
                ;
        }

        public override string ToString() => 
            $"Record {Id} for {Name} with address {Address}";

        public override bool IsEqual(NameRecord other) =>
            Equals(other as INameRecord);

        protected override void SetHashCode() => 
            MixToHash(Address);
    }
}
