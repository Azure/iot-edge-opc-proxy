// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Runtime.Serialization;

    /// <summary>
    /// Response for close request, or notification when close occurred
    /// </summary>
    [DataContract]
    public class CloseResponse : Poco<CloseResponse>, IMessageContent, IResponse  {
        
        /// <summary>
        /// How long the link was open
        /// </summary>
        [DataMember(Name = "time_open", Order = 1)]
        public ulong TimeOpenInMilliseconds {
            get; set;
        }

        /// <summary>
        /// How many bytes were sent
        /// </summary>
        [DataMember(Name = "bytes_sent", Order = 2)]
        public ulong BytesSent {
            get; set;
        }

        /// <summary>
        /// How many received
        /// </summary>
        [DataMember(Name = "bytes_received", Order = 3)]
        public ulong BytesReceived {
            get; set;
        }

        /// <summary>
        /// Last error code on the link, e.g. reason link closed.
        /// </summary>
        [DataMember(Name = "error_code", Order = 4)]
        public int ErrorCode {
            get; set;
        }

        /// <summary>
        /// Create response
        /// </summary>
        /// <param name="timeOpenInMilliseconds"></param>
        /// <param name="bytesSent"></param>
        /// <param name="bytesReceived"></param>
        /// <param name="errorCode"></param>
        /// <returns></returns>
        public static CloseResponse Create(ulong timeOpenInMilliseconds,
            ulong bytesSent, ulong bytesReceived, int errorCode) {
            var response = Get();
            response.TimeOpenInMilliseconds = timeOpenInMilliseconds;
            response.BytesSent = bytesSent;
            response.BytesReceived = bytesReceived;
            response.ErrorCode = errorCode;
            return response;
        }

        public IMessageContent Clone() =>
            Create(TimeOpenInMilliseconds, BytesSent, BytesReceived, ErrorCode);

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool IsEqual(CloseResponse that) {
            return
                IsEqual(TimeOpenInMilliseconds, that.TimeOpenInMilliseconds) &&
                IsEqual(BytesSent, that.BytesSent) &&
                IsEqual(BytesReceived, that.BytesReceived) &&
                IsEqual(ErrorCode, that.ErrorCode);
        }

        protected override void SetHashCode() {
            MixToHash(TimeOpenInMilliseconds);
            MixToHash(BytesSent);
            MixToHash(BytesReceived);
            MixToHash(ErrorCode);
        }

        public override string ToString() =>
            $"Open for {TimeSpan.FromMilliseconds(TimeOpenInMilliseconds)} " +
            $"(Received/Sent: {BytesReceived}/{BytesSent}, CloseReason: {ErrorCode})";
    }
}