using System;
// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Model {
    using System;
    using System.Collections.Generic;
    using System.Threading;
    using System.Threading.Tasks;

    public interface INameService {
        /// <summary>
        /// Lookup record by name
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        Task<IEnumerable<INameRecord>> LookupAsync(string name, RecordType type,
             CancellationToken ct);

        /// <summary>
        /// Lookup record by address
        /// </summary>
        /// <param name="address"></param>
        /// <returns></returns>
        Task<IEnumerable<INameRecord>> LookupAsync(Reference address, RecordType type,
            CancellationToken ct);
    }
}