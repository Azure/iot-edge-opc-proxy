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
    public class PingRequest : Serializable<PingRequest>, IMessageContent, IRequest {

        /// <summary>
        /// Socket address to ping, typically proxy address
        /// </summary>
        [DataMember(Name = "address", Order = 1)]
        public SocketAddress SocketAddress { get; set; }

        /// <summary>
        /// Default constructor
        /// </summary>
        public PingRequest() {
            // Noop
        }

        /// <summary>
        /// Convinience constructor
        /// </summary>
        /// <param name="address"></param>
        public PingRequest(SocketAddress socketAddress) {
            this.SocketAddress = socketAddress;
        }


        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool IsEqual(PingRequest that) =>
            IsEqual(SocketAddress, that.SocketAddress);

        protected override void SetHashCode() =>
            MixToHash(SocketAddress);
    }
}