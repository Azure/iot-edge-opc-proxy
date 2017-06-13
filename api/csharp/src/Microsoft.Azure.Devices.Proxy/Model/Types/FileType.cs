// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    //
    // File types
    //
    public enum FileType {
        Unknown = 0,
        File,
        Directory,
        Link,
        BlockDevice,
        CharDevice,
        Pipe,
        Socket
    };
}