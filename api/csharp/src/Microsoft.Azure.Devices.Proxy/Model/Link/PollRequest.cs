// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System.Runtime.Serialization;

    /// <summary>
    /// Request to poll data
    /// </summary>
    [DataContract]
    public class PollRequest : Serializable<PollRequest>, IMessageContent, IRequest {

        /// <summary>
        /// How long to wait in milliseconds
        /// </summary>
        [DataMember(Name = "timeout", Order = 1)]
        public ulong Timeout { get; set; }

        /// <summary>
        /// Default constructor
        /// </summary>
        /// <param name="timeout"></param>
        public PollRequest() {
            Timeout = 60000;
        }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="timeout"></param>
        public PollRequest(ulong timeout) {
            Timeout = timeout;
        }

        /// <summary>
        /// Returns whether 2 request are equal.
        /// </summary>
        /// <param name="other"></param>
        /// <returns></returns>
        public override bool IsEqual(PollRequest that) =>
            IsEqual(Timeout, that.Timeout);
        

        protected override void SetHashCode() =>
            MixToHash(Timeout.GetHashCode());
    }
}