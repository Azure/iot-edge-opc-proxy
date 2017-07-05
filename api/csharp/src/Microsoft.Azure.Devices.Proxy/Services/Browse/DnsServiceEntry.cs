// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;

    /// <summary>
    /// Represents a browsed service. In the case of dns-sd, this is the 
    /// record pointed to by PTR. To ensure we are not too chatty it is 
    /// expected that all entries contain server and server meta data if
    /// available.
    /// </summary>
    public class DnsServiceEntry : Poco<DnsServiceEntry>, IEnumerable<KeyValuePair<string, string>> {

        /// <summary>
        /// In case of dns-sd, the SRV entry related to PTR record.
        /// This address should be resolved using the Host resolver
        /// or can be passed directly to BindAsync/ConnectAsync.
        /// </summary>
        public SocketAddress Address {
            get; private set;
        }

        /// <summary>
        /// In case of dns-sd, PTR entry.  This is the full record
        /// that was used to query the entry (including the proxy
        /// interface on which the entry was found.
        /// </summary>
        public DnsServiceRecord Service {
            get; private set;
        }

        /// <summary>
        /// In case of dns-sd, TXT records associated with the SRV 
        /// record. Use these to look up additional properties of
        /// the service, e.g. uri, configuration parameters, etc.
        /// </summary>
        public DnsTxtRecord[] TxtRecords {
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
        internal static DnsServiceEntry Create(SocketAddress address,
            DnsServiceRecord service, DnsTxtRecord[] txtRecords) {
            var entry = Get();
            entry.Address = address;
            entry.Service = service;
            entry.TxtRecords = txtRecords;
            return entry;
        }

        /// <summary>
        /// Returns a value for a key in the text records 
        /// </summary>
        /// <param name="key"></param>
        /// <returns></returns>
        public string this[string key] {
            get {
                foreach (var record in TxtRecords) {
                    if (record.AsKeyValuePair().Key.Equals(key)) {
                        return key;
                    }
                }
                throw new IndexOutOfRangeException($"{key} not found");
            }
        }

        /// <summary>
        /// Enumerator to iterate through text record key value pairs
        /// </summary>
        /// <returns></returns>
        public IEnumerator<KeyValuePair<string, string>> GetEnumerator() {
            foreach (var item in TxtRecords) {
                yield return item.AsKeyValuePair();
            }
        }

        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();

        public override bool IsEqual(DnsServiceEntry that) {
            return
                IsEqual(Service, that.Service) &&
                IsEqual(Address, that.Address) &&
                TxtRecords.SequenceEqual(that.TxtRecords);
        }

        protected override void SetHashCode() {
            MixToHash(Service);
            MixToHash(Address);
            MixToHash(TxtRecords);
        }

        /// <summary>
        /// Stringify entry
        /// </summary>
        /// <returns></returns>
        public override string ToString() {
            var bld = new StringBuilder();
            bld.Append(Service.ToString());
            bld.Append("@");
            bld.AppendLine(Address.ToString());
            for (int i = 0; i < TxtRecords.Length; i++) {
                bld.Append(",  [");
                bld.Append(i);
                bld.Append("] ");
                bld.AppendLine(TxtRecords[i].ToString());
            }
            return bld.ToString();
        }
    }
}
