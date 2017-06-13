// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System.Runtime.Serialization;

    /// <summary>
    /// Void args or return
    /// </summary>
    [DataContract]
    public abstract class VoidMessage : Serializable<VoidMessage>, IMessageContent {
        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool IsEqual(VoidMessage that) {
            return true;
        }

        protected override void SetHashCode() {
            MixToHash(0);
        }
    }
}