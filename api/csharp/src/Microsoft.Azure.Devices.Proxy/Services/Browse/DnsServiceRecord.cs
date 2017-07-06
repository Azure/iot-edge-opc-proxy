// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;

    /// <summary>
    /// Represents an abstract service record. In the case of dns-sd, this is the 
    /// record pointed to by PTR. 
    /// </summary>
    public class DnsServiceRecord : Poco<DnsServiceRecord> {

        /// <summary>
        /// Name of service 
        /// </summary>
        public string Name {
            get; private set;
        }

        /// <summary>
        /// Type of service
        /// </summary>
        public string Type {
            get; private set;
        }

        /// <summary>
        /// Reply Domain
        /// </summary>
        public string Domain {
            get; private set;
        }

        /// <summary>
        /// Proxy on which this record is valid.
        /// </summary>
        public SocketAddress Interface {
            get; private set;
        }

        /// <summary>
        /// Whether the record was removed
        /// </summary>
        public bool Removed {
            get; private set;
        }

        /// <summary>
        /// Clone record
        /// </summary>
        /// <param name="service"></param>
        internal static DnsServiceRecord Clone(DnsServiceRecord service) => Create(
            service.Name, service.Type, service.Domain, service.Interface, service.Removed);

        /// <summary>
        /// Clone and update record
        /// </summary>
        /// <param name="service"></param>
        /// <param name="interface"></param>
        /// <param name="removed"></param>
        /// <returns></returns>
        internal static DnsServiceRecord Create(DnsServiceRecord service, 
            SocketAddress @interface, bool removed) {
            var record = Clone(service);
            record.Interface = @interface;
            record.Removed = removed;
            return record;
        }

        /// <summary>
        /// Create new record
        /// </summary>
        /// <param name="name"></param>
        /// <param name="type"></param>
        /// <param name="domain"></param>
        /// <param name="interface"></param>
        /// <param name="removed"></param>
        /// <returns></returns>
        internal static DnsServiceRecord Create(string name, string type, 
            string domain, SocketAddress @interface, bool removed = false) {
            var record = Get();
            record.Name = name;
            record.Type = type;
            record.Domain = domain;
            record.Interface = @interface;
            record.Removed = removed;
            return record;
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
            return Create(name, type, domain, null, false);
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
        internal static DnsServiceRecord FromSocketAddress(ProxySocketAddress address, 
            SocketAddress @interface = null, bool removed = false) {
            var record = Parse(address.Host);
            record._flags = address.Flags;
            record._interfaceIndex = address.InterfaceIndex;
            record.Interface = @interface;
            record.Removed = removed;
            return record;
        }

        public override bool IsEqual(DnsServiceRecord that) {
            return
                IsEqual(Name, that.Name) &&
                IsEqual(Type, that.Type) &&
                IsEqual(Domain, that.Domain);
        }

        protected override void SetHashCode() {
            MixToHash(Name);
            MixToHash(Type);
            MixToHash(Domain);
        }

        /// <summary>
        /// Stringify address
        /// </summary>
        /// <returns></returns>
        public override string ToString() => 
            $"{FullString} on {Interface}{(Removed ? " REMOVED" : "")}";


        private ushort _flags = 0;
        private int _interfaceIndex = -1;
    }
}
