// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Linq.Expressions;
    using System.Threading.Tasks.Dataflow;

    /// <summary>
    /// Interface for name services exposed by provider object.
    /// </summary>
    public interface INameService {

        /// <summary>
        /// Produces name records for a set of expression queries posted to the
        /// returned block, which source can be connected to other blocks.
        /// </summary>
        /// <param name="options">Read source execution options</param>
        /// <returns></returns>
        IPropagatorBlock<Expression<Func<INameRecord, bool>>, INameRecord> Read(
            ExecutionDataflowBlockOptions options);

        /// <summary>
        /// Singleton writer block. Post add, updates or removal operations to
        /// the block to be lazily completed.  Subscribe to the block to receive
        /// updates to the device registry broadcasted.
        /// </summary>
        IPropagatorBlock<
            Tuple<INameRecord, NameServiceOperation>,
            Tuple<INameRecord, NameServiceEvent>>
        Write {
            get;
        }
    }
}
