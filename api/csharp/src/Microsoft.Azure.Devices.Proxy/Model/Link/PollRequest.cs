// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System.Runtime.Serialization;

    /// <summary>
    /// Request to poll data
    /// </summary>
    [DataContract]
    public class PollRequest : Poco<PollRequest>, IMessageContent, IRequest {

        /// <summary>
        /// Sequence number of the data message to retrieve.
        /// </summary>
        [DataMember(Name = "sequence_number", Order = 1)]
        public ulong SequenceNumber {
            get; set;
        }

        /// <summary>
        /// How long to wait in milliseconds
        /// </summary>
        [DataMember(Name = "timeout", Order = 2)]
        public ulong Timeout {
            get; set;
        }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="timeout"></param>
        public static PollRequest Create(ulong timeout, ulong sequenceNumber = 0) {
            var request = Get();
            request.Timeout = timeout;
            request.SequenceNumber = sequenceNumber;
            return request;
        }

        public IMessageContent Clone() => Create(Timeout, SequenceNumber);

        public override bool IsEqual(PollRequest that) {
            return 
                IsEqual(Timeout, that.Timeout) && 
                IsEqual(SequenceNumber, that.SequenceNumber);
        }

        protected override void SetHashCode() {
            MixToHash(Timeout.GetHashCode());
            MixToHash(SequenceNumber.GetHashCode());
        }

        public override string ToString() =>
            $"{SequenceNumber} Timeout: {Timeout}";
    }
}