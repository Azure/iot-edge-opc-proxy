// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Provider {
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using Newtonsoft.Json;
    using Newtonsoft.Json.Linq;

    /// <summary>
    /// Wrapper around device twin result
    /// </summary>
    internal class IoTHubRecord : INameRecord, IComparable {

        private const string _typeTag = "type";
        private const string _addressTag = "address";
        private const string _referencesTag = "references";
        private const string _nameTag = "name";
        private const string _domainTag = "domain";
        private const string _lastAliveProp = "$metadata.alive.$lastUpdated";

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
                var addr = (string)this[_addressTag];
                return addr == null ? Reference.Null : Reference.Parse(addr);
            }
            set {
                // Do not update an existing address...
                if (_tags[_addressTag] != null) {
                    return;
                }
                this[_addressTag] = value?.ToString();
            }
        }

        /// <summary>
        /// Returns the name of the record
        /// </summary>
        public string Name {
            get => (string)this[_nameTag] ?? "";
            set => this[_nameTag] = value;
        }

        /// <summary>
        /// Returns the domain of the record
        /// </summary>
        public string Domain {
            get => (string)this[_domainTag];
            set => this[_domainTag] = value;
        }

        /// <summary>
        /// The type of the record
        /// </summary>
        public NameRecordType Type {
            get {
                var value = this[_typeTag];
                return (NameRecordType)(value == null ? 0 : (int)value);
            }
            set {
                this[_typeTag] = (int)value;
                // No support for bit queries ...
                if (0 == (value & NameRecordType.Proxy))
                    this["proxy"] = null;
                else
                    this["proxy"] = 1;
                if (0 == (value & NameRecordType.Host))
                    this["host"] = null;
                else
                    this["host"] = 1;
                if (0 == (value & NameRecordType.Link))
                    this["link"] = null;
                else
                    this["link"] = 1;
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
        /// Returns when this record was last seen
        /// </summary>
        public DateTime LastActivity {
            get {
                if (_lastActivity == null) {
                    try {
                        _lastActivity = (DateTime)_reported.SelectToken(_lastAliveProp, true);
                    }
                    catch(JsonException) {
                        _lastActivity = DateTime.MinValue;
                    }
                }
                return (DateTime)_lastActivity;
            }
            set {
                if (_lastActivity == null || value > _lastActivity) {
                    _lastActivity = value;
                }
            }
        }

        /// <summary>
        /// Device twin tags based record
        /// </summary>
        public IoTHubRecord(JObject twin) {
            Id = (string)twin["deviceId"] ?? throw new ArgumentNullException(nameof(twin));
            Etag = (string)twin["etag"] ?? "*";
            _tags = (JObject)twin["tags"] ?? new JObject();
            var props = (JObject)twin["properties"] ?? new JObject();
            _desired = (JObject)props["desired"] ?? new JObject();
            _reported = (JObject)props["reported"] ?? new JObject();
        }

        /// <summary>
        /// Clone constructor
        /// </summary>
        public IoTHubRecord(INameRecord record) : this(record.Id) => Assign(record);

        /// <summary>
        /// Clone constructor
        /// </summary>
        public IoTHubRecord(string deviceId) {
            Id = deviceId;
            Etag = "*";
            _tags = new JObject();
            _desired = new JObject();
            _reported = new JObject();
        }

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
            if (!refs.TryGetValue(address.ToString(),
                StringComparison.CurrentCultureIgnoreCase, out JToken val) ||
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
            LastActivity = record.LastActivity;

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
        internal string Patch {
            get {
                if (!_changes.HasValues) {
                    return null;
                }
                var patch = new JObject(new JProperty("tags", _changes)).ToString();
                return patch;
            }
        }

        public bool Disconnected { get; internal set; } = false;

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
        /// <param name="obj"></param>
        /// <returns></returns>
        public override bool Equals(object obj) => Equals(obj as INameRecord);

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() => Address.GetHashCode();

        /// <summary>
        /// Compares based on last activity...
        /// </summary>
        /// <param name="obj"></param>
        /// <returns></returns>
        public int CompareTo(object obj) {
            var that = obj as INameRecord;
            return LastActivity.CompareTo(that == null ? DateTime.MinValue : that.LastActivity);
        }

        public override string ToString() =>
            $"Record {Id} for {Name} with address {Address} ({LastActivity})";


        /// <summary>
        /// Helper to set or get a property in the twin tag and writing the
        /// patch log at the same time.
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        private JToken this[string name] {
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
                else if (!prop.Equals(value)) {
                    _tags[name] = value;
                    _changes[name] = value;
                }
            }
        }

        private DateTime? _lastActivity;
        private readonly JObject _tags;
        private readonly JObject _desired;
        private readonly JObject _reported;
        private readonly JObject _changes = new JObject();
    }
}
