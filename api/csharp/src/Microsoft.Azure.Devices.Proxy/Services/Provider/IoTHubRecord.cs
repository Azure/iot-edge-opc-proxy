// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Provider {
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using Newtonsoft.Json.Linq;
    using System.Text;

    /// <summary>
    /// Wrapper around device twin result
    /// </summary>
    internal class IoTHubRecord : INameRecord {

        private static readonly string _typeTag = "type";
        private static readonly string _addressTag = "address";
        private static readonly string _referencesTag = "references";
        private static readonly string _nameTag = "name";

        /// <summary>
        /// Device id as unique identifier of the record on the hub
        /// </summary>
        public string Id {
            get; set;
        }

        /// <summary>
        /// Returns the address of the record
        /// </summary>
        public Reference Address {
            get {
                var addr = this[_addressTag];
                return addr == null ? Reference.Null : Reference.Parse(addr);
            }
            set {
                // Do not update an existing address...
                if (_tags[_addressTag] != null)
                    return; 
                this[_addressTag] = value?.ToString();
            }
        }

        /// <summary>
        /// Returns the name of the record
        /// </summary>
        public string Name {
            get => this[_nameTag] ?? "";
            set => this[_nameTag] = value;
        }

        /// <summary>
        /// The type of the record
        /// </summary>
        public NameRecordType Type {
            get {
                var value = this[_typeTag];
                return (NameRecordType)(value == null ? 0 : int.Parse(value));
            }
            set {
                this[_typeTag] = ((int)value).ToString();
                // No support for bit queries ...
                if (0 == (value & NameRecordType.Proxy))
                    this["proxy"] = null;
                else
                    this["proxy"] = "1";
                if (0 == (value & NameRecordType.Host))
                    this["host"] = null;
                else
                    this["host"] = "1";
                if (0 == (value & NameRecordType.Link))
                    this["link"] = null;
                else
                    this["link"] = "1";
            }
        }

        /// <summary>
        /// Returns references of this record
        /// </summary>
        public IEnumerable<Reference> References {
            get {
                var refs = _tags[_referencesTag] as JObject;
                if (refs == null) {
                    return Enumerable.Empty<Reference>();
                }
                return refs.Properties().Select(p => Reference.Parse((string)p.Value));
            }
        }

        /// <summary>
        /// Device twin tags based record
        /// </summary>
        public IoTHubRecord(string deviceId, string etag = null, JObject tags = null) {
            Id = deviceId;
            Etag = etag ?? "*";
            _tags = tags ?? new JObject();
        }

        /// <summary>
        /// Clone constructor
        /// </summary>
        public IoTHubRecord(INameRecord record) : this(record.Id) => Assign(record);

        /// <summary>
        /// Add address
        /// </summary>
        /// <param name="address"></param>
        public void AddReference(Reference address) {
            if (References.Contains(address))
                return;
            var refs = _tags[_referencesTag] as JObject;
            if (refs == null) {
                refs = new JObject();
                _tags.Add(_referencesTag, refs);
            }
            JToken val;
            if (!refs.TryGetValue(address.ToString(), 
                StringComparison.CurrentCultureIgnoreCase, out val) ||
                !val.ToString().Equals(address.ToString())) {

                refs[address.ToString()] = address.ToString();

                var patch = _changes[_referencesTag] as JObject;
                if (patch == null) {
                    _changes[_referencesTag] = patch = new JObject();
                }
                patch[address.ToString()] = address.ToString();
            }
        }

        /// <summary>
        /// Add address
        /// </summary>
        /// <param name="address"></param>
        public void RemoveReference(Reference address) {
            if (!References.Contains(address))
                return;
            var refs = _tags[_referencesTag] as JObject;
            if (refs != null) {

                refs.Remove(address.ToString());
                if (refs.HasValues) {
                    var patch = _changes[_referencesTag] as JObject;
                    if (patch == null) {
                        _changes[_referencesTag] = patch = new JObject();
                    }
                    patch[address.ToString()] = null;
                }
                else {
                    _changes[_referencesTag] = null;
                }
            }
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
            Address = record.Address;

            // TODO: Need to create intersection...!!!

            foreach (var reference in record.References) {
                AddReference(reference);
            }

            return this;
        }

        /// <summary>
        /// The etag for this record instance
        /// </summary>
        internal string Etag { get; set; }

        /// <summary>
        /// Returns the patches needed based on the changes in this record
        /// </summary>
        internal string Log {
            get {
                if (!_changes.HasValues) {
                    return null;
                }
                return new JObject(new JProperty("tags", _changes)).ToString();
            }
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(IoTHubRecord that) {
            if (that == null) {
                return false;
            }
            if (!Etag.Equals(that.Etag)) {
                return false;
            }
            return Equals((INameRecord) that);
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
                References.SameAs(that.References)
                ;
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(object that) => Equals(that as INameRecord);
        

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return Address.GetHashCode();
        }

        public override string ToString() => $"Record {Id} for {Name} with address {Address}";

        /// <summary>
        /// Create record type expression from type
        /// </summary>
        /// <param name="type"></param>
        /// <returns></returns>
        internal static string CreateTypeQueryString(NameRecordType type) {
            var sql = new StringBuilder("(");
            bool concat = false;

            // No support for bit queries ...
            if (0 != (type & NameRecordType.Proxy)) {
                sql.Append("tags.proxy=1");
                concat = true;
            }
            if (0 != (type & NameRecordType.Host)) {
                if (concat) sql.Append(" OR ");
                sql.Append("tags.host=1");
                concat = true;
            }
            if (0 != (type & NameRecordType.Link)) {
                if (concat) sql.Append(" OR ");
                sql.Append("tags.link=1");
                concat = true;
            }
            if (!concat) {
                return "";
            }
            sql.Append(")");
            return sql.ToString();
        }

        /// <summary>
        /// Helper to set or get a property in the twin tag and writing the 
        /// patch log at the same time.
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        private string this[string name] {
            get {
                var prop = _tags[name];
                return prop?.ToString();
            }
            set {
                var prop = _tags[name];
                if (prop == null) {
                    if (value != null) {
                        _tags.Add(name, value);
                        _changes.Add(name, value);
                    }
                }
                else if (!prop.ToString().Equals(value)) {
                    _tags[name] = value;
                    _changes[name] = value;
                }
            }
        }

        private readonly JObject _tags;
        private readonly JObject _changes = new JObject();
    }
}
