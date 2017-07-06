// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Linq;
    using System.Collections.Generic;
    using System.Runtime.Serialization;

    /// <summary>
    /// Browse rpc response
    /// </summary>
    [DataContract]
    public class BrowseResponse : Poco<BrowseResponse> {

        /// <summary>
        /// Original request handle 
        /// </summary>
        [DataMember(Name = "handle", Order = 1)]
        public Reference Handle {
            get; set;
        }

        //
        // Response flags denoting the type of response or event occurred
        //
        public static readonly int Eos = 0x1;        // End of stream reached
        public static readonly int Removed = 0x2;             // Item removed
        public static readonly int AllForNow = 0x4;        // Cache exhausted
        public static readonly int Empty = 0x8;             // Empty response

        /// <summary>
        /// Flags
        /// </summary>
        [DataMember(Name = "flags", Order = 2)]
        public int Flags {
            get; set;
        }

        /// <summary>
        /// Error - Results in exception if not Ok.
        /// </summary>
        [DataMember(Name = "error_code", Order = 3)]
        public int Error {
            get; set;
        }

        /// <summary>
        /// Browsed item or null if response is empty.
        /// </summary>
        [DataMember(Name = "item", Order = 4)]
        public SocketAddress Item {
            get; set;
        }

        /// <summary>
        /// Properties of item
        /// </summary>
        [DataMember(Name = "props", Order = 5)]
        public List<IProperty> Properties {
            get; set;
        } = new List<IProperty>();

        /// <summary>
        /// Proxy from which the response was received
        /// </summary>
        public SocketAddress Interface {
            get; internal set;
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool IsEqual(BrowseResponse that) {
            return
                IsEqual(Handle, that.Handle) &&
                IsEqual(Error, that.Error) &&
                IsEqual(Flags, that.Flags) &&
                IsEqual(Item, that.Item) &&
                Properties.SequenceEqual(that.Properties);
        }

        protected override void SetHashCode() {
            MixToHash(Handle);
            MixToHash(Error);
            MixToHash(Flags);
            MixToHash(Item);
            MixToHash(Properties);
        }
    }
} 