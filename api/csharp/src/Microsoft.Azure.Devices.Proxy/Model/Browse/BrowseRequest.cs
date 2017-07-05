// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Runtime.Serialization;

    /// <summary>
    /// Browse rpc request
    /// </summary>
    [DataContract]
    public class BrowseRequest : Poco<BrowseRequest> {

        /// <summary>
        /// Request id
        /// </summary>
        [DataMember(Name = "handle", Order = 1)]
        public Reference Handle {
            get; set;
        }

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
        public static readonly int Resolve = 1;    // Resolve host to address
        public static readonly int Service = 2;            // Browse services
        public static readonly int Dirpath = 3;         // Recurse a dir path

        /// <summary>
        /// Type of request
        /// </summary>
        [DataMember(Name = "type", Order = 3)]
        public int Type {
            get; set;
        }

        /// <summary>
        /// Flags - 0 for now
        /// </summary>
        [DataMember(Name = "flags", Order = 4)]
        public int Flags {
            get; set;
        }

        /// <summary>
        /// Item to browse
        /// </summary>
        [DataMember(Name = "item", Order = 5)]
        public SocketAddress Item {
            get; set;
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool IsEqual(BrowseRequest that) {
            return
                IsEqual(Version, that.Version) &&
                IsEqual(Type, that.Type) &&
                IsEqual(Flags, that.Flags) &&
                IsEqual(Handle, that.Handle) &&
                IsEqual(Item, that.Item);
        }

        protected override void SetHashCode() {
            MixToHash(Version);
            MixToHash(Type);
            MixToHash(Flags);
            MixToHash(Handle);
            MixToHash(Item);
        }
    }
} 