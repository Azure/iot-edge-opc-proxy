// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Model {
    using System;
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
        /// Default constructor
        /// </summary>
        public Record() {
            // no op
        }

        /// <summary>
        /// Create entry info
        /// </summary>
        /// <param name="address"></param>
        /// <param name="name"></param>
        /// <param name="id"></param>
        public Record(RecordType type, Reference address, string name, string id) {

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
        }
    }
}
