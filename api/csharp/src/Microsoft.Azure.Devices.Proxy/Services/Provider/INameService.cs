// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Threading;
    using System.Threading.Tasks.Dataflow;

    /// <summary>
    /// Interface for name services exposed by provider object.  
    /// </summary>
    public interface INameService {
        /// <summary>
        /// Creates a query based on name. The name can also be a stringified 
        /// address, or an alias name of the record. It cannot be null or empty.
        /// </summary>
        /// <param name="name">The name to query for</param>
        /// <param name="type">The type of record</param>
        /// <returns></returns>
        IQuery NewQuery(string name, NameRecordType type);

        /// <summary>
        /// Creates a query based on address. It is valid to specify Reference.All 
        /// to receive all records of a particular type. It is not valid to specify 
        /// null or Reference.Null as address.
        /// </summary>
        /// <param name="address">The address to query for</param>
        /// <param name="type">The type of records</param>
        /// <returns></returns>
        IQuery NewQuery(Reference address, NameRecordType type);

        /// <summary>
        /// Produces name records for a set of queries posted to the returned Lookup 
        /// block.  Results will be send to the results block.
        /// </summary>
        /// <param name="results">target block to post results to</param>
        /// <param name="ct">Cancel the current query in progress</param>
        /// <returns></returns>
        IPropagatorBlock<IQuery, INameRecord> Lookup(ExecutionDataflowBlockOptions options);

        /// <summary>
        /// Post name record to add or update (true) the record in the name service
        /// or remove it (false). Add, update, remove are all done in the 
        /// </summary>
        ITargetBlock<Tuple<INameRecord, bool>> Update { get; }
    }
}
