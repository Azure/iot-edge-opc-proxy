// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    
    /// <summary>
    /// Service browser interface - enumerate service records
    /// </summary>
    public interface IDnsServiceRecordBrowser : 
        IAsyncEnumerator<DnsServiceRecord>, IDisposable { }

    /// <summary>
    /// Represents an abstract service record. In the case of dns-sd, this is the 
    /// record pointed to by PTR. 
    /// </summary>
    public class DnsServiceRecord : IEquatable<DnsServiceRecord> {

        /// <summary>
        /// Name of service 
        /// </summary>
        public string Name { get; internal set; }

        /// <summary>
        /// Type of service
        /// </summary>
        public string Type { get; internal set; }

        /// <summary>
        /// Reply Domain
        /// </summary>
        public string Domain { get; internal set; }

        /// <summary>
        /// Proxy on which this record is valid.
        /// </summary>
        public SocketAddress Interface { get; internal set; }

        /// <summary>
        /// Whether the record was removed
        /// </summary>
        public bool Removed { get; internal set; }

        /// <summary>
        /// Default constructor
        /// </summary>
        internal DnsServiceRecord() {}

        /// <summary>
        /// Clone record
        /// </summary>
        /// <param name="service"></param>
        internal DnsServiceRecord(DnsServiceRecord service) {
            Name = service.Name;
            Type = service.Type;
            Domain = service.Domain;
            Interface = service.Interface;
        }

        /// <summary>
        /// Parses a service string to create a service record
        /// </summary>
        /// <param name="address"></param>
        /// <returns></returns>
        public static DnsServiceRecord Parse(string fullString) {
            string name = "";
            string type = "";
            string domain = "";

            var labels = fullString.Split('.');
            int index = 0;

            // Name is everything until service type
            for (; index < labels.Length; index++) {
                if (labels[index].StartsWith("_")) {
                    break;
                }
                if (index != 0)
                    name += ".";
                name += labels[index];
            }
            // All components of type start with _
            for (; index < labels.Length; index++) {
                if (!labels[index].StartsWith("_")) {
                    break;
                }
                type += labels[index];
                type += ".";
            }
            type = type.TrimEnd('.');
            // If not type, then we have a label.
            if (index == labels.Length) {
                domain = name;
                name = "";
            }
            else {
                // Or remainder is label
                for (; index < labels.Length; index++) {
                    domain += labels[index];
                    domain += ".";
                }
                domain.TrimEnd('.');
            }
            return new DnsServiceRecord {
                Name = name, 
                Type = type, 
                Domain = domain
            };
        }

        /// <summary>
        /// Stringify address
        /// </summary>
        /// <returns></returns>
        public string FullString {
            get {
                string fullName;
                if (!string.IsNullOrEmpty(Name)) {
                    fullName = Name;
                    fullName += ".";
                }
                else {
                    fullName = "";
                }

                if (!string.IsNullOrEmpty(Type)) {
                    fullName += Type;
                    fullName += ".";
                }

                if (!string.IsNullOrEmpty(Domain)) {
                    fullName += Domain;
                }
                else if (!string.IsNullOrEmpty(fullName)) {
                    fullName += "local";
                }
                return fullName;
            }
        }

        /// <summary>
        /// Converts a record to a socket address item
        /// </summary>
        /// <returns></returns>
        internal ProxySocketAddress ToSocketAddress() => 
            new ProxySocketAddress(FullString) {
                InterfaceIndex = _interfaceIndex,
                Flags = _flags
            };

        /// <summary>
        /// Converts a socket address to record
        /// </summary>
        /// <param name="address"></param>
        /// <returns></returns>
        internal static DnsServiceRecord FromSocketAddress(
            ProxySocketAddress address) {
            var record = Parse(address.Host);
            record._flags = address.Flags;
            record._interfaceIndex = address.InterfaceIndex;
            return record;
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(DnsServiceRecord that) {
            if (that == null) {
                return false;
            }
            if (string.IsNullOrEmpty(this.Name)) {
                if (!string.IsNullOrEmpty(that.Name))
                    return false;
            }
            else if (!this.Name.Equals(that.Name))
                return false;
            if (string.IsNullOrEmpty(this.Type)) {
                if (!string.IsNullOrEmpty(that.Type))
                    return false;
            }
            else if (!this.Type.Equals(that.Type))
                return false;
            if (string.IsNullOrEmpty(this.Domain)) {
                if (!string.IsNullOrEmpty(that.Domain))
                    return false;
            }
            else if (!this.Domain.Equals(that.Domain))
                return false;
            return true;
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(object that) {
            return Equals(that as DnsServiceRecord);
        }

        /// <summary>
        /// Stringify address
        /// </summary>
        /// <returns></returns>
        public override string ToString() => 
            $"{FullString} on {Interface}{(Removed ? " REMOVED" : "")}";

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() => ToString().GetHashCode();


        private ushort _flags = 0;
        private int _interfaceIndex = -1;
    }
}
