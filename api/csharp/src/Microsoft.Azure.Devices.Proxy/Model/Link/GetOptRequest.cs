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
    public class GetOptRequest : Poco<GetOptRequest>, IMessageContent, IRequest {

        /// <summary>
        /// Option to get value for
        /// </summary>
        [DataMember(Name = "so_opt", Order = 1)]
        public SocketOption Option {
            get; set;
        }

        /// <summary>
        /// Create request
        /// </summary>
        /// <param name="option"></param>
        /// <returns></returns>
        public static GetOptRequest Create(SocketOption option) {
            var request = Get();
            request.Option = option;
            return request;
        }

        public IMessageContent Clone() => Create(Option);

        public override bool IsEqual(GetOptRequest that) =>
            IsEqual(Option, that.Option);

        protected override void SetHashCode() =>
            MixToHash(Option);

        public override string ToString() =>
            Option.ToString();
    }
}