// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    /// <summary>
    /// Dns host name resolver interface - resolve host names or addresses
    /// </summary>
    public interface IDnsHostEntryResolver : 
        IAsyncEnumerator<DnsHostEntry>, IDisposable { }
} 