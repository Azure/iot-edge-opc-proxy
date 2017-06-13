// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    //
    // Proxy protocol, defines messages sent to rpc socket communication.
    //
    using System;
    using System.Runtime.Serialization;
    /// <summary>
    // Get option request, request value for specified option
    /// </summary>
    [DataContract]
    public class GetOptRequest : Serializable<GetOptRequest>, IMessageContent, IRequest {

        /// <summary>
        /// Option to get value for
        /// </summary>
        [DataMember(Name = "so_opt", Order = 1)]
        public SocketOption Option { get; set; }

        /// <summary>
        /// Default constructor
        /// </summary>
        public GetOptRequest() {
            // no-op
        }

        /// <summary>
        /// Convinience constructor
        /// </summary>
        public GetOptRequest(SocketOption option) {
            Option = option;
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool IsEqual(GetOptRequest that) =>
            IsEqual(Option, that.Option);

        protected override void SetHashCode() =>
            MixToHash(Option);
    }
}