// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Runtime.Serialization;
    /// <summary>
    /// Ping request
    /// </summary>
    [DataContract]
    public class PingRequest : Poco<PingRequest>, IMessageContent, IRequest {

        /// <summary>
        /// Socket address to ping, typically proxy address
        /// </summary>
        [DataMember(Name = "address", Order = 1)]
        public SocketAddress SocketAddress {
            get; set;
        }

        /// <summary>
        /// Create request
        /// </summary>
        /// <param name="address"></param>
        public static PingRequest Create(SocketAddress socketAddress) {
            var request = Get();
            request.SocketAddress = socketAddress;
            return request;
        }

        public IMessageContent Clone() => Create(SocketAddress);

        public override bool IsEqual(PingRequest that) =>
            IsEqual(SocketAddress, that.SocketAddress);

        protected override void SetHashCode() =>
            MixToHash(SocketAddress);

        public override string ToString() => 
            $"{SocketAddress}";
    }
}