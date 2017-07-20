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
        /// Link transport capabilities
        /// </summary>
        [DataMember(Name = "transport_caps", Order = 4)]
        public uint TransportCaps {
            get; set;
        }

        /// <summary>
        /// Max buffer to send on this link = 0 means 64k
        /// </summary>
        [DataMember(Name = "max_send", Order = 5)]
        public uint MaxSendBuffer {
            get; set;
        }

        /// <summary>
        /// Create response
        /// </summary>
        /// <param name="linkId"></param>
        /// <param name="localAddress"></param>
        /// <param name="peerAddress"></param>
        /// <param name="transportCaps"></param>
        /// <returns></returns>
        public static LinkResponse Create(Reference linkId,
            SocketAddress localAddress, SocketAddress peerAddress, uint transportCaps) {
            var response = Get();
            response.LinkId = linkId;
            response.LocalAddress = localAddress;
            response.PeerAddress = peerAddress;
            response.TransportCaps = transportCaps;
            return response;
        }

        public IMessageContent Clone() =>
            Create(LinkId, LocalAddress, PeerAddress, TransportCaps);

        public override bool IsEqual(LinkResponse that) {
            return IsEqual(LinkId, that.LinkId) &&
                IsEqual(LocalAddress, that.LocalAddress) &&
                IsEqual(PeerAddress, that.PeerAddress) &&
                IsEqual(TransportCaps, that.TransportCaps);
        }

        protected override void SetHashCode() {
            MixToHash(LinkId);
            MixToHash(LocalAddress);
            MixToHash(PeerAddress);
            MixToHash(TransportCaps);
        }

        public override string ToString() =>
            $"{LinkId} (Local: {LocalAddress}, Peer: {PeerAddress}, Caps: {TransportCaps})";
    }
}