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
    public class GetOptResponse : Poco<GetOptResponse>, IMessageContent, IResponse {

        /// <summary>
        /// Option value returned
        /// </summary>
        [DataMember(Name = "so_val", Order = 1)]
        public IProperty OptionValue {
            get; set;
        }

        /// <summary>
        /// Create response
        /// </summary>
        /// <param name="optionValue"></param>
        /// <returns></returns>
        public static GetOptResponse Create(IProperty optionValue) {
            var response = Get();
            response.OptionValue = optionValue;
            return response;
        }

        public IMessageContent Clone() => Create(OptionValue);

        public override bool IsEqual(GetOptResponse that) =>
            IsEqual(OptionValue, that.OptionValue);

        protected override void SetHashCode() =>
            MixToHash(OptionValue);

        public override string ToString() =>
            OptionValue.ToString();
    }
}