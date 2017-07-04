// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Linq;
    using System.Collections.Generic;
    using System.Text;

    /// <summary>
    /// Container class for host name information
    /// </summary>
    public class DnsHostEntry : Poco<DnsHostEntry> {

        /// <summary>
        /// Contains host name
        /// </summary>
        public string HostName {
            get; private set;
        }

        /// <summary>
        /// Other dns names for this host
        /// </summary>
        public string[] Aliases {
            get; private set;
        }

        /// <summary>
        /// Addresses for this host
        /// </summary>
        public SocketAddress[] AddressList {
            get; private set;
        }

        /// <summary>
        /// Proxy on which this record is valid.
        /// </summary>
        public SocketAddress Interface {
            get; private set;
        }

        /// <summary>
        /// Create entry
        /// </summary>
        /// <param name="hostName"></param>
        /// <param name="aliases"></param>
        /// <param name="addressList"></param>
        /// <param name="interface"></param>
        /// <returns></returns>
        internal static DnsHostEntry Create(string hostName, string[] aliases,
            SocketAddress[] addressList, SocketAddress @interface) {
            var entry = Get();
            entry.HostName = hostName;
            entry.Aliases = aliases;
            entry.AddressList = addressList;
            entry.Interface = @interface;
            return entry;
        }

        public override bool IsEqual(DnsHostEntry that) {
            return
                IsEqual(HostName, that.HostName) &&
                IsEqual(Interface, that.Interface) &&
                Aliases.SequenceEqual(that.Aliases) &&
                AddressList.SequenceEqual(that.AddressList);
        }

        protected override void SetHashCode() {
            MixToHash(HostName);
            MixToHash(Interface);
            MixToHash(Aliases);
            MixToHash(AddressList);
        }

        /// <summary>
        /// Return object as string
        /// </summary>
        /// <returns></returns>
        public override string ToString() {
            var str = new StringBuilder();
            str.Append(HostName);
            foreach (var alias in Aliases) {
                str.Append("[");
                str.Append(alias);
                str.Append("]");
            }
            foreach (var address in AddressList) {
                str.Append(", ");
                str.Append(address.ToString());
            }
            return str.ToString();
        }

        /// <summary>
        /// Collate a list of entries into one big host entry
        /// </summary>
        /// <param name="address"></param>
        /// <param name="entries"></param>
        /// <returns></returns>
        internal static DnsHostEntry ToEntry(SocketAddress address, 
            IEnumerable<DnsHostEntry> entries) {
            if (entries == null || !entries.Any()) {
                return new DnsHostEntry {
                    HostName = address.ToString(),
                    Aliases = new string[0],
                    AddressList = new SocketAddress[] { address }
                };
            }

            string hostName = string.Empty;
            var addressList = new HashSet<SocketAddress>();
            var aliases = new HashSet<string>();
            foreach (var entry in entries) {
                hostName = entry.HostName;
                if (entry.AddressList != null) {
                    addressList.AddRange(entry.AddressList);
                }
                if (entry.Aliases != null) {
                    aliases.AddRange(entry.Aliases);
                }
            }
            return new DnsHostEntry {
                HostName = hostName,
                Aliases = aliases.ToArray(),
                AddressList = addressList.ToArray()
            };
        }
    }
} 