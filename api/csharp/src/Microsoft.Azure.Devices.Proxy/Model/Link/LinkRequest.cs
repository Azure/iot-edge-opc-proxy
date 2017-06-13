// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Runtime.Serialization;

    /// <summary>
    /// Link request - create socket link 
    /// </summary>
    [DataContract]
    public class LinkRequest : Serializable<LinkRequest>, IMessageContent, IRequest {

        public static readonly byte LINK_VERSION = 7;

        /// <summary>
        /// Version number
        /// </summary>
        [DataMember(Name = "version", Order = 1)]
        public Byte Version { get; set; }

        /// <summary>
        /// Socket Properties
        /// </summary>
        [DataMember(Name = "props", Order = 2)]
        public SocketInfo Properties { get; set; }

        /// <summary>
        /// Default constructor
        /// </summary>
        public LinkRequest() {
            // no-op
        }

        /// <summary>
        /// Convinience constructor
        /// </summary>
        /// <param name="properties"></param>
        public LinkRequest(SocketInfo properties) {
            Version = LINK_VERSION;
            Properties = properties;
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool IsEqual(LinkRequest that) {
            return IsEqual(Version, that.Version) &&
                IsEqual(Properties, that.Properties);
        }

        protected override void SetHashCode() {
            MixToHash(Version);
            MixToHash(Properties);
        }
    }
}