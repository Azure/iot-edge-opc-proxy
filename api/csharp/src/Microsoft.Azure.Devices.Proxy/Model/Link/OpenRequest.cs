// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Runtime.Serialization;

    /// <summary>
    /// Open request - opens the link or updates credits
    /// </summary>
    [DataContract]
    public class OpenRequest : Serializable<OpenRequest>, IMessageContent, IRequest {

        /// <summary>
        /// Subscribed channel for data messages
        /// </summary>
        [DataMember(Name = "stream_id", Order = 1)]
        public Reference StreamId { get; set; }

        /// <summary>
        /// What type of connection string is used - default 0
        /// </summary>
        [DataMember(Name = "type", Order = 2)]
        public int Type { get; set; }

        /// <summary>
        /// Send credit, causes receive up to credit exhaustion
        /// </summary>
        [DataMember(Name = "connection-string", Order = 3)]
        public string ConnectionString { get; set; }

        /// <summary>
        /// Whether the stream is going to be polled
        /// </summary>
        [DataMember(Name = "polled", Order = 4)]
        public bool IsPolled { get; set; }

        /// <summary>
        /// Largest receive buffer (to us) = default 0 = auto
        /// </summary>
        [DataMember(Name = "max_recv", Order = 5)]
        public uint MaxReceiveBuffer { get; set; } = 0;

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool IsEqual(OpenRequest that) {
            return
                IsEqual(StreamId, that.StreamId) &&
                IsEqual(Type, that.Type) &&
                IsEqual(IsPolled, that.IsPolled) &&
                IsEqual(ConnectionString, that.ConnectionString);
        }

        protected override void SetHashCode() {
            MixToHash(Type);
            MixToHash(IsPolled);
            MixToHash(StreamId);
            MixToHash(ConnectionString);
        }
    }
}