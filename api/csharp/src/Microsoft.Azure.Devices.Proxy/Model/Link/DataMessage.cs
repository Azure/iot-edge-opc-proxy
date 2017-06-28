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
        /// Sequence number of the data message 
        /// </summary>
        [DataMember(Name = "sequence_number", Order = 1)]
        public ulong SequenceNumber { get; set; }

        /// <summary>
        /// Source address if udp socket
        /// </summary>
        [DataMember(Name = "source_address", Order = 2)]
        public SocketAddress Source { get; set; }

        /// <summary>
        /// Buffer content
        /// </summary>
        [DataMember(Name = "buffer", Order = 3)]
        public byte[] Payload { get; set; }

        /// <summary>
        /// Control buffer
        /// </summary>
        [DataMember(Name = "control_buffer", Order = 4)]
        public byte[] Control { get; set; }

        /// <summary>
        /// Default constructor
        /// </summary>
        public DataMessage() {
            // no op
        }

        public DataMessage(byte[] payload, SocketAddress endpoint = null, ulong sequenceNumber = 0) {
            SequenceNumber = sequenceNumber;
            Payload = payload;
            Source = endpoint;
            Control = new byte[0];
        }

        /// <summary>
        /// Convinience constructor
        /// </summary>
        /// <param name="buffer"></param>
        /// <param name="endpoint"></param>
        public DataMessage(ArraySegment<byte> buffer, SocketAddress endpoint, ulong sequenceNumber = 0) {
            SequenceNumber = sequenceNumber;
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
                IsEqual(SequenceNumber, that.SequenceNumber) &&
                IsEqual(Payload, that.Payload) &&
                IsEqual(Control, that.Control) &&
                IsEqual(Source, that.Source);
        }

        protected override void SetHashCode() {
            MixToHash(SequenceNumber);
            MixToHash(Payload);
            MixToHash(Control);
            MixToHash(Source);
        }
    }
}