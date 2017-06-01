// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;

    /// <summary>
    /// Service browser interface - enumerate service records
    /// </summary>
    public interface IDnsServiceRecordResolver :
        IAsyncEnumerator<DnsServiceEntry>, IDisposable { }

    /// <summary>
    /// Txt record entry - represents meta data of service record
    /// </summary>
    public class TxtRecord : IEquatable<TxtRecord> {

        /// <summary>
        /// Value of the record as binary buffer.
        /// </summary>
        public byte[] Value { get; internal set; }

        /// <summary>
        /// Helper constructor
        /// </summary>
        /// <param name="key"></param>
        /// <param name="valueRaw"></param>
        internal TxtRecord(byte[] valueRaw) {
            Value = valueRaw;
        }

        /// <summary>
        /// Return object as string - assumption is that it is a utf-8 txt record.
        /// </summary>
        /// <returns></returns>
        public override string ToString() => Encoding.UTF8.GetString(Value);

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(Object obj) {
            return this.Equals(obj as TxtRecord);
        }

        /// <summary>
        /// Returns this record as key value pair
        /// </summary>
        /// <returns></returns>
        public KeyValuePair<string, string> AsKeyValuePair() {
            var pair = ToString().Split(new char[] { '=' }, 2);
            if (pair.Length == 2) {
                return new KeyValuePair<string, string>(pair[0].Trim(), pair[1].Trim());
            }
            else {
                return new KeyValuePair<string, string>("", pair[0].Trim());
            }
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(TxtRecord that) {
            if (that == null) {
                return false;
            }
            return
                this.Value.SequenceEqual(that.Value);
        }

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return ToString().GetHashCode();
        }
    }

    /// <summary>
    /// Represents a browsed service. In the case of dns-sd, this is the 
    /// record pointed to by PTR. To ensure we are not too chatty it is 
    /// expected that all entries contain server and server meta data if
    /// available.
    /// </summary>
    public class DnsServiceEntry : IEquatable<DnsServiceEntry> {

        /// <summary>
        /// In case of dns-sd, the SRV entry related to PTR record.
        /// This address should be resolved using the Host resolver
        /// or can be passed directly to BindAsync/ConnectAsync.
        /// </summary>
        public ProxySocketAddress Address { get; internal set; }

        // TODO: Need to have interface info in here!

        /// <summary>
        /// In case of dns-sd, PTR entry.  This is the full record
        /// that was used to query the entry (including the proxy
        /// interface on which the entry was found.
        /// </summary>
        public DnsServiceRecord Service { get; internal set; }

        /// <summary>
        /// In case of dns-sd, TXT records associated with the SRV 
        /// record. Use these to look up additional properties of
        /// the service, e.g. uri, configuration parameters, etc.
        /// </summary>
        public TxtRecord[] TxtRecords { get; internal set; }

        /// <summary>
        /// Internal constructor
        /// </summary>
        internal DnsServiceEntry() { }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(DnsServiceEntry that) {
            if (that == null) {
                return false;
            }
            return
                this.Service.Equals(that.Service) &&
                this.Address.Equals(that.Address) &&
                this.TxtRecords.SequenceEqual(that.TxtRecords);
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(object that) {
            return Equals(that as DnsServiceEntry);
        }

        /// <summary>
        /// Stringify address
        /// </summary>
        /// <returns></returns>
        public override string ToString() {
            var bld = new StringBuilder();
            bld.AppendLine(Service.ToString());
            bld.AppendLine(Address.ToString());
            for (int i = 0; i < TxtRecords.Length; i++) {
                bld.Append("  [");
                bld.Append(i);
                bld.Append("] ");
                bld.AppendLine(TxtRecords[i].ToString());
            }
            return bld.ToString();
        }

        /// <summary>
        /// Returns hash 
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return
                ToString().GetHashCode();
        }
    }
}
