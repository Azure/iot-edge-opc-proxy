// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy.Model {
    //
    // Browse protocol, defines messages sent to rpc of browse server.
    //
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Runtime.Serialization;

    //
    // Dns Record types
    //
    public enum DnsRecordType {
        Simple = 100,
        Txt,
        Ptr,

        // ...
        __prx_record_max
    };


    /// <summary>
    /// Browse rpc response
    /// </summary>
    [DataContract]
    public class BrowseResponse : Serializable<BrowseResponse>, IEquatable<BrowseResponse> {

        /// <summary>
        /// Request id
        /// </summary>
        [DataMember(Name = "handle", Order = 1)]
        public Reference Handle { get; set; }

        //
        // Type of request - service, address resolver, folders
        //
        public static readonly int Eos = 0x1;        // End of stream reached
        public static readonly int Removed = 0x2;             // Item removed
        public static readonly int AllForNow = 0x4;        // Cache exhausted
        public static readonly int Empty = 0x8;             // Empty response

        /// <summary>
        /// Flags
        /// </summary>
        [DataMember(Name = "flags", Order = 2)]
        public int Flags { get; set; }

        /// <summary>
        /// Error
        /// </summary>
        [DataMember(Name = "error_code", Order = 3)]
        public int Error { get; set; }

        /// <summary>
        /// Item returned
        /// </summary>
        [DataMember(Name = "item", Order = 4)]
        public SocketAddress Item { get; set; }

        /// <summary>
        /// Properties of item
        /// </summary>
        [DataMember(Name = "props", Order = 5)]
        public HashSet<PropertyBase> Properties { get; set; } = new HashSet<PropertyBase>();

        /// <summary>
        /// Proxy from which the response was received
        /// </summary>
        public SocketAddress Interface { get; internal set; }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(Object obj) {
            return this.Equals(obj as BrowseResponse);
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(BrowseResponse that) {
            if (that == null) {
                return false;
            }
            return
                this.Handle.Equals(that.Handle) &&
                this.Error.Equals(that.Error) &&
                this.Flags.Equals(that.Flags) &&
                this.Item.Equals(that.Item) &&
                this.Properties.SequenceEqual(that.Properties);
        }

        /// <summary>
        /// Returns hash code
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            int result = 0;
            foreach (var prop in Properties)
                result = (result * 31) ^ prop.GetHashCode();
            return (result * Flags) ^
                (Handle.GetHashCode() ^ (Item.GetHashCode() * 31));
        }
    }

    /// <summary>
    /// Browse rpc request
    /// </summary>
    [DataContract]
    public class BrowseRequest : Serializable<BrowseRequest>, IEquatable<BrowseRequest> {

        /// <summary>
        /// Request id
        /// </summary>
        [DataMember(Name = "handle", Order = 1)]
        public Reference Handle { get; set; }

        public static readonly byte BROWSE_VERSION = 1;

        /// <summary>
        /// Version number, always 1
        /// </summary>
        [DataMember(Name = "version", Order = 2)]
        public Byte Version { get; internal set; } = BROWSE_VERSION;

        //
        // Type of request - service, address resolver, folders
        //
        public static readonly int Cancel = 0;  // Cancel request with handle
        public static readonly int Resolve = 1;  // Resolve string to address
        public static readonly int Service = 2;            // Browse services
        public static readonly int Dirpath = 3;         // Recurse a dir path

        /// <summary>
        /// Type of request
        /// </summary>
        [DataMember(Name = "type", Order = 3)]
        public int Type { get; set; }

        /// <summary>
        /// Flags - 0 for now
        /// </summary>
        [DataMember(Name = "flags", Order = 4)]
        public int Flags { get; set; }

        /// <summary>
        /// Item to browse
        /// </summary>
        [DataMember(Name = "item", Order = 5)]
        public SocketAddress Item { get; set; }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(Object obj) {
            return this.Equals(obj as BrowseRequest);
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(BrowseRequest that) {
            if (that == null) {
                return false;
            }
            return
                this.Version.Equals(that.Version) &&
                this.Type.Equals(that.Type) &&
                this.Flags.Equals(that.Flags) &&
                this.Handle.Equals(that.Handle) &&
                this.Item.Equals(that.Item);
        }

        /// <summary>
        /// Returns hash code
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return (Version * Type * Flags) ^
                (Handle.GetHashCode() ^ (Item.GetHashCode() * 31));
        }
    }
} 