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
    /// Dns host name resolver interface - resolve host names or addresses
    /// </summary>
    public interface IDnsHostEntryResolver : 
        IAsyncEnumerator<DnsHostEntry>, IDisposable { }

    /// <summary>
    /// Container class for host name information
    /// </summary>
    public class DnsHostEntry : IEquatable<DnsHostEntry> {

        /// <summary>
        /// Contains host name
        /// </summary>
        public string HostName { get; internal set; }

        /// <summary>
        /// Other dns names for this host
        /// </summary>
        public string[] Aliases { get; internal set; }

        /// <summary>
        /// Addresses for this host
        /// </summary>
        public SocketAddress[] AddressList { get; internal set; }

        /// <summary>
        /// Proxy on which this record is valid.
        /// </summary>
        public SocketAddress Interface { get; internal set; }

        /// <summary>
        /// Return object as string
        /// </summary>
        /// <returns></returns>
        public override string ToString() {
            var str = new StringBuilder();
            str.Append(HostName);
            foreach (var alias in Aliases) {
                str.Append(", ");
                str.Append(alias);
            }
            str.AppendLine();
            foreach (var address in AddressList) {
                str.Append("  ");
                str.AppendLine(address.ToString());
            }
            return str.ToString();
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(Object obj) {
            return this.Equals(obj as DnsHostEntry);
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(DnsHostEntry that) {
            if (that == null) {
                return false;
            }
            return
                this.HostName.Equals(that.HostName) &&
                this.Aliases.SequenceEqual(that.Aliases) &&
                this.Interface.Equals(that.Interface) &&
                this.AddressList.SequenceEqual(that.AddressList);
        }

        /// <summary>
        /// Returns hash code
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            int result = HostName.GetHashCode();
            foreach (var address in AddressList) {
                result = (result * 31) ^ address.GetHashCode();
            }
            result ^= Interface.GetHashCode();
            return result;
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
            var addressList = new List<SocketAddress>();
            var aliases = new List<string>();
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