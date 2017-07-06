// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System.Runtime.Serialization;

    /// <summary>
    /// Set option request
    /// </summary>
    [DataContract]
    public class SetOptRequest : Poco<SetOptRequest>, IMessageContent, IRequest {

        /// <summary>
        /// Option and Value to set
        /// </summary>
        [DataMember(Name = "so_val", Order = 1)]
        public IProperty OptionValue {
            get; set;
        }

        /// <summary>
        /// Create request
        /// </summary>
        public static SetOptRequest Create(IProperty optionValue) {
            var request = Get();
            request.OptionValue = optionValue;
            return request;
        }

        public IMessageContent Clone() => Create(OptionValue);

        public override bool IsEqual(SetOptRequest that) =>
            IsEqual(OptionValue, that.OptionValue);

        protected override void SetHashCode() => 
            MixToHash(OptionValue);

        public override string ToString() => 
            OptionValue.ToString();
    }
}