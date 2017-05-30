// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {

    using System.Threading;
    using System.Threading.Tasks;

    /// <summary>
    /// Async enumerator interface - todo: remove for ix
    /// </summary>
    /// <typeparam name="T"></typeparam>
    public interface IAsyncEnumerator<T> {

        /// <summary>
        /// Current element
        /// </summary>
        T Current { get; }

        /// <summary>
        /// Move to next entry
        /// </summary>
        /// <param name="ct"></param>
        /// <returns>true if next entry, false if done</returns>
        Task<bool> MoveNextAsync(CancellationToken ct);
    }
}
