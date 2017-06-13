// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System.Collections.Generic;
    using System.Threading;
    using System.Threading.Tasks;

    /// <summary>
    /// Interface for name services exposed by provider object
    /// </summary>
    public interface INameService {
        /// <summary>
        /// Lookup record by name
        /// </summary>
        /// <param name="name"></param>
        /// <param name="type"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        Task<IEnumerable<INameRecord>> LookupAsync(string name, NameRecordType type,
             CancellationToken ct);

        /// <summary>
        /// Lookup record by address
        /// </summary>
        /// <param name="address"></param>
        /// <param name="type"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        Task<IEnumerable<INameRecord>> LookupAsync(Reference address, NameRecordType type,
            CancellationToken ct);

        /// <summary>
        /// Adds or updates a record in the name service
        /// </summary>
        /// <param name="proxy"></param>
        /// <param name="name"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        Task AddOrUpdateAsync(INameRecord record, CancellationToken ct);

        /// <summary>
        /// Removes a record in the name service
        /// </summary>
        /// <param name="proxy"></param>
        /// <param name="name"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        Task RemoveAsync(INameRecord record, CancellationToken ct);
    }
}