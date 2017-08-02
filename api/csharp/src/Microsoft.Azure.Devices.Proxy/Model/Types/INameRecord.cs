// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Collections.Generic;

    /// <summary>
    /// Name record interface
    /// </summary>
    public interface INameRecord : IEquatable<INameRecord> {

        /// <summary>
        /// Name
        /// </summary>
        string Name {
            get; set;
        }

        /// <summary>
        /// Domain
        /// </summary>
        string Domain {
            get; set;
        }

        /// <summary>
        /// Address of the record
        /// </summary>
        Reference Address {
            get; set;
        }

        /// <summary>
        /// Unique id of the record
        /// </summary>
        string Id {
            get; set;
        }

        /// <summary>
        /// Record type
        /// </summary>
        NameRecordType Type {
            get; set;
        }

        /// <summary>
        /// Returns list of references for this record
        /// </summary>
        IEnumerable<Reference> References {
            get;
        }

        /// <summary>
        /// Time the item behind the record had activity
        /// </summary>
        DateTime LastActivity {
            get; set;
        }

        /// <summary>
        /// Add a address to another record
        /// </summary>
        /// <param name="address"></param>
        void AddReference(Reference address);

        /// <summary>
        /// Remove a address
        /// </summary>
        /// <param name="address"></param>
        void RemoveReference(Reference address);

        /// <summary>
        /// Assign name record to this record
        /// </summary>
        /// <param name="record"></param>
        /// <returns></returns>
        INameRecord Assign(INameRecord record);
    }
}