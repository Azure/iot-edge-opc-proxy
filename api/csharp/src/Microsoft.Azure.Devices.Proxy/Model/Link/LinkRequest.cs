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
    public class LinkRequest : Poco<LinkRequest>, IMessageContent, IRequest {

        public static readonly byte LINK_VERSION = 7;

        /// <summary>
        /// Version number
        /// </summary>
        [DataMember(Name = "version", Order = 1)]
        public Byte Version {
            get; set;
        }

        /// <summary>
        /// Socket Properties
        /// </summary>
        [DataMember(Name = "props", Order = 2)]
        public SocketInfo Properties {
            get; set;
        }

        /// <summary>
        /// Create request
        /// </summary>
        /// <param name="properties"></param>
        public static LinkRequest Create(SocketInfo properties) {
            var request = Get();
            request.Version = LINK_VERSION;
            request.Properties = properties;
            return request;
        }

        public IMessageContent Clone() => Create(Properties);

        public override bool IsEqual(LinkRequest that) {
            return 
                IsEqual(Version, that.Version) &&
                IsEqual(Properties, that.Properties);
        }

        protected override void SetHashCode() {
            MixToHash(Version);
            MixToHash(Properties);
        }

        public override string ToString() => Properties.ToString();
    }
}