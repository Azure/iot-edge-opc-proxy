// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace System.Threading.Tasks {
    using System;
    using System.Threading;

    public static class TaskEx {
        public static Task Completed { get; } = Task.FromResult(true);
    }
}