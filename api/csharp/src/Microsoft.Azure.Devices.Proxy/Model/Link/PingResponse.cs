// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Linq;
    using System.Runtime.Serialization;

    /// <summary>
    // Ping response
    /// </summary>
    [DataContract]
    public class PingResponse : Poco<PingResponse>, IMessageContent, IResponse {

        /// <summary>
        /// Address pingged, in form of local address
        /// </summary>
        [DataMember(Name = "address", Order = 1)]
        public SocketAddress SocketAddress {
            get; set;
        }

        /// <summary>
        /// Mac address of machine 
        /// </summary>
        [DataMember(Name = "physical_address", Order = 2)]
        public byte[] PhysicalAddress {
            get; set;
        }

        /// <summary>
        /// Time ping took
        /// </summary>
        [DataMember(Name = "time_ms", Order = 3)]
        public uint TimeMs {
            get; set;
        }

        /// <summary>
        /// Create response
        /// </summary>
        /// <param name="socketAddress"></param>
        public static PingResponse Create(SocketAddress socketAddress, 
            byte[] physicalAddress = null, uint timeMs = 0) {
            var response = Get();
            response.SocketAddress = socketAddress;
            response.PhysicalAddress = physicalAddress ?? new byte[8];
            response.TimeMs = timeMs;
            return response;
        }

        public IMessageContent Clone() => Create(SocketAddress, PhysicalAddress, TimeMs);

        public override bool IsEqual(PingResponse that) {
            return
                IsEqual(SocketAddress, that.SocketAddress) &&
                IsEqual(PhysicalAddress, that.PhysicalAddress) &&
                IsEqual(TimeMs, that.TimeMs);
        }

        protected override void SetHashCode() {
            MixToHash(SocketAddress);
            MixToHash(PhysicalAddress);
            MixToHash(TimeMs);
        }

        public override string ToString() => $"{SocketAddress}";
    }
}