// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Runtime.Serialization;

    /// <summary>
    // Link response or error
    /// </summary>
    [DataContract]
    public class LinkResponse : Poco<LinkResponse>, IMessageContent, IResponse {
        /// <summary>
        /// Remote link address 
        /// </summary>
        [DataMember(Name = "link_id", Order = 1)]
        public Reference LinkId {
            get; set;
        }

        /// <summary>
        /// Interface address assigned on remote
        /// </summary>
        [DataMember(Name = "local_address", Order = 2)]
        public SocketAddress LocalAddress {
            get; set;
        }

        /// <summary>
        /// Peer IP address proxy connected to
        /// </summary>
        [DataMember(Name = "peer_address", Order = 3)]
        public SocketAddress PeerAddress {
            get; set;
        }

        /// <summary>
        /// Create response
        /// </summary>
        /// <param name="linkId"></param>
        /// <param name="localAddress"></param>
        /// <param name="peerAddress"></param>
        /// <returns></returns>
        public static LinkResponse Create(Reference linkId, 
            SocketAddress localAddress, SocketAddress peerAddress) {
            var response = Get();
            response.LinkId = linkId;
            response.LocalAddress = localAddress;
            response.PeerAddress = peerAddress;
            return response;
        }

        public IMessageContent Clone() => Create(LinkId, LocalAddress, PeerAddress);

        public override bool IsEqual(LinkResponse that) {
            return IsEqual(LinkId, that.LinkId) &&
                IsEqual(LocalAddress, that.LocalAddress) &&
                IsEqual(PeerAddress, that.PeerAddress);
        }

        protected override void SetHashCode() {
            MixToHash(LinkId);
            MixToHash(LocalAddress);
            MixToHash(PeerAddress);
        }

        public override string ToString() =>
            $"{LinkId} (Local: {LocalAddress}, Peer: {PeerAddress})";
    }
}