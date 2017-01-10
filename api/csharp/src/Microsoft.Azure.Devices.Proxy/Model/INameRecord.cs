// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Model {

    /// <summary>
    /// Name record interface
    /// </summary>
    public interface INameRecord {

        /// <summary>
        /// Name
        /// </summary>
        string Name { get; set; }

        /// <summary>
        /// Address of the record
        /// </summary>
        Reference Address { get; set; }

        /// <summary>
        /// Unique id of the record
        /// </summary>
        string Id { get; set; }

        /// <summary>
        /// Record type
        /// </summary>
        RecordType Type { get; set; }
    }
}