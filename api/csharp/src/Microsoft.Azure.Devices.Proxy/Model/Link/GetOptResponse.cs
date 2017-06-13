// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System.Runtime.Serialization;

    /// <summary>
    /// Get option response, returns option value
    /// </summary>
    [DataContract]
    public class GetOptResponse : Serializable<GetOptResponse>, IMessageContent, IResponse {

        /// <summary>
        /// Option value returned
        /// </summary>
        [DataMember(Name = "so_val", Order = 1)]
        public PropertyBase OptionValue { get; set; }

        /// <summary>
        /// Default constructor
        /// </summary>
        public GetOptResponse() {
            // no-op
        }

        /// <summary>
        /// Convinience constructor
        /// </summary>
        public GetOptResponse(PropertyBase optionValue) {
            OptionValue = optionValue;
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool IsEqual(GetOptResponse that) =>
            IsEqual(OptionValue, that.OptionValue);

        protected override void SetHashCode() =>
            MixToHash(OptionValue);
    }
}