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
    public class VoidMessage<T> : Poco<T>, IVoidMessage where T : Poco, new() {

        public static T Create() => Get();

        public IMessageContent Clone() => (IMessageContent)Get();

        public override bool IsEqual(T that) => true;

        protected override void SetHashCode() => MixToHash(0);

        public override string ToString() => "void";
    }
}