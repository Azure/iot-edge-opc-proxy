// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Runtime.Serialization;

    /// <summary>
    /// Stream data message payload
    /// </summary>
    [DataContract]
    public class DataMessage : Serializable<DataMessage>, IMessageContent {

        /// <summary>
        /// Source address if udp socket
        /// </summary>
        [DataMember(Name = "source_address", Order = 1)]
        public SocketAddress Source { get; set; }

        /// <summary>
        /// Buffer content
        /// </summary>
        [DataMember(Name = "buffer", Order = 2)]
        public byte[] Payload { get; set; }

        /// <summary>
        /// Control buffer
        /// </summary>
        [DataMember(Name = "control_buffer", Order = 3)]
        public byte[] Control { get; set; }

        /// <summary>
        /// Default constructor
        /// </summary>
        public DataMessage() {
            // no op
        }

        public DataMessage(byte[] payload, SocketAddress endpoint = null) {
            Payload = payload;
            Source = endpoint;
            Control = new byte[0];
        }

        /// <summary>
        /// Convinience constructor
        /// </summary>
        /// <param name="buffer"></param>
        /// <param name="endpoint"></param>
        public DataMessage(ArraySegment<byte> buffer, SocketAddress endpoint) {
            Payload = new byte[buffer.Count];
            Buffer.BlockCopy(buffer.Array, buffer.Offset, Payload, 0, buffer.Count);
            Source = endpoint;
            Control = new byte[0];
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool IsEqual(DataMessage that) {
            return
                IsEqual(Payload, that.Payload) &&
                IsEqual(Control, that.Control) &&
                IsEqual(Source, that.Source);
        }

        protected override void SetHashCode() {
            MixToHash(Payload);
            MixToHash(Control);
            MixToHash(Source);
        }
    }
}