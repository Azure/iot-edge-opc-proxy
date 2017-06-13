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
    /// Txt record entry - represents meta data of service record
    /// </summary>
    public class DnsTxtRecord : IEquatable<DnsTxtRecord> {

        /// <summary>
        /// Value of the record as binary buffer.
        /// </summary>
        public byte[] Value { get; internal set; }

        /// <summary>
        /// Helper constructor
        /// </summary>
        /// <param name="key"></param>
        /// <param name="valueRaw"></param>
        internal DnsTxtRecord(byte[] valueRaw) {
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
            return this.Equals(obj as DnsTxtRecord);
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
        public bool Equals(DnsTxtRecord that) {
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
}
