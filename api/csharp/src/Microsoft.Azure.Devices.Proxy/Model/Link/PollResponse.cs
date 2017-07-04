// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System.Runtime.Serialization;

    /// <summary>
    /// Void response for poll requests
    /// </summary>
    [DataContract]
    public class PollResponse :  VoidMessage<PollResponse>, IResponse {
    }
}