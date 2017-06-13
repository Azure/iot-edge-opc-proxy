// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    /// <summary>
    /// Browse files in directory
    /// </summary>
    public interface IDirectoryBrowser :
        IAsyncEnumerator<FileEntry>, IDisposable { }
} 