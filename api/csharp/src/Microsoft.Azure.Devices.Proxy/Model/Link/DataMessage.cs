// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Linq;
    using System.Runtime.Serialization;
    using System.Text;

    /// <summary>
    /// Stream data message payload
    /// </summary>
    [DataContract]
    public class DataMessage : Poco<DataMessage>, IMessageContent {

        /// <summary>
        /// Sequence number of the data message 
        /// </summary>
        [DataMember(Name = "sequence_number", Order = 1)]
        public ulong SequenceNumber {
            get; set;
        }

        /// <summary>
        /// Source address if udp socket
        /// </summary>
        [DataMember(Name = "source_address", Order = 2)]
        public SocketAddress Source {
            get; set;
        }

        /// <summary>
        /// Buffer content
        /// </summary>
        [DataMember(Name = "buffer", Order = 3)]
        public byte[] Payload {
            get; set;
        }

        /// <summary>
        /// Control buffer
        /// </summary>
        [DataMember(Name = "control_buffer", Order = 4)]
        public byte[] Control {
            get; set;
        }

        /// <summary>
        /// Create data message
        /// </summary>
        /// <param name="payload"></param>
        /// <param name="endpoint"></param>
        /// <param name="sequenceNumber"></param>
        /// <returns></returns>
        public static DataMessage Create(byte[] payload, SocketAddress endpoint = null,
            ulong sequenceNumber = 0) =>
            Create(payload, endpoint, new byte[0], sequenceNumber);

        /// <summary>
        /// Create data message
        /// </summary>
        /// <param name="buffer"></param>
        /// <param name="endpoint"></param>
        public static DataMessage Create(ArraySegment<byte> buffer, SocketAddress endpoint, 
            ulong sequenceNumber = 0) {
            var payload = new byte[buffer.Count];
            Buffer.BlockCopy(buffer.Array, buffer.Offset, payload, 0, buffer.Count);
            return Create(payload, endpoint, new byte[0], sequenceNumber);
        }

        /// <summary>
        /// Create message
        /// </summary>
        /// <param name="payload"></param>
        /// <param name="endpoint"></param>
        /// <param name="control"></param>
        /// <param name="sequenceNumber"></param>
        /// <returns></returns>
        public static DataMessage Create(byte[] payload, SocketAddress endpoint,
            byte[] control, ulong sequenceNumber) {
            var message = Get();
            message.SequenceNumber = sequenceNumber;
            message.Payload = payload;
            message.Source = endpoint;
            message.Control = control;
            return message;
        }

        public IMessageContent Clone() => Create(Payload, Source, Control, SequenceNumber);

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

        public override void Dispose() {
            Payload = null;
            Control = null;
            base.Dispose();
        }

        public override string ToString() {
#if !DEBUG
            return $"[{SequenceNumber}]";
#else
            var bld = new StringBuilder();
            bld.Append("[");
            bld.Append(SequenceNumber);
            bld.Append("] ");
            bld.Append("'");
            if (Payload == null) {
                bld.Append("null");
            }
            else {
                bld.Append(Payload, 32);
            }
            bld.Append("'");
            return bld.ToString();
#endif
        }
    }
}