// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System.Runtime.Serialization;

    /// <summary>
    /// Open request - opens the link or updates credits
    /// </summary>
    [DataContract]
    public class OpenRequest : Poco<OpenRequest>, IMessageContent, IRequest {

        /// <summary>
        /// Subscribed channel for data messages
        /// </summary>
        [DataMember(Name = "stream_id", Order = 1)]
        public Reference StreamId {
            get; set;
        }

        /// <summary>
        /// Encoding to use on stream
        /// </summary>
        [DataMember(Name = "encoding", Order = 2)]
        public int Encoding {
            get; set;
        }

        /// <summary>
        /// Send credit, causes receive up to credit exhaustion
        /// </summary>
        [DataMember(Name = "connection-string", Order = 3)]
        public string ConnectionString {
            get; set;
        }

        /// <summary>
        /// What type of connection string is used - default 0
        /// </summary>
        [DataMember(Name = "type", Order = 4)]
        public int Type {
            get; set;
        }

        /// <summary>
        /// Whether the stream is going to be polled
        /// </summary>
        [DataMember(Name = "polled", Order = 5)]
        public bool IsPolled {
            get; set;
        }

        /// <summary>
        /// Largest receive buffer (to us) = default 0 = auto
        /// </summary>
        [DataMember(Name = "max_recv", Order = 6)]
        public uint MaxReceiveBuffer {
            get; set;
        }

        /// <summary>
        /// Create request
        /// </summary>
        /// <param name="streamId"></param>
        /// <param name="type"></param>
        /// <param name="connectionString"></param>
        /// <param name="isPolled"></param>
        /// <param name="maxReceiveBuffer"></param>
        /// <returns></returns>
        public static OpenRequest Create(Reference streamId, int encoding, 
            string connectionString, int type, bool isPolled, uint maxReceiveBuffer = 0) {
            var request = Get();
            request.StreamId = streamId;
            request.Encoding = encoding;
            request.ConnectionString = connectionString;
            request.Type = type;
            request.IsPolled = isPolled;
            request.MaxReceiveBuffer = maxReceiveBuffer;
            return request;
        }

        public IMessageContent Clone() => 
            Create(StreamId, Encoding, ConnectionString, Type, IsPolled, MaxReceiveBuffer);

        public override bool IsEqual(OpenRequest that) {
            return
                IsEqual(StreamId, that.StreamId) &&
                IsEqual(Encoding, that.Encoding) &&
                IsEqual(ConnectionString, that.ConnectionString) &&
                IsEqual(Type, that.Type) &&
                IsEqual(IsPolled, that.IsPolled) &&
                IsEqual(MaxReceiveBuffer, that.MaxReceiveBuffer);
        }

        protected override void SetHashCode() {
            MixToHash(StreamId);
            MixToHash(Encoding);
            MixToHash(ConnectionString);
            MixToHash(Type);
            MixToHash(IsPolled);
        }

        public override string ToString() =>
            $"{StreamId} (Polled: {IsPolled})";
    }
}