// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {

    /// <summary>
    /// Property interface - wrapper for serveral property types, 
    /// including socket option values and dns records. Describes
    /// properties of a remote item (e.g. socket, file, etc.)
    /// </summary>
    public interface IProperty {
        /// <summary>
        /// Property type
        /// </summary>
        uint Type {
            get;
        }
    }

    /// <summary>
    /// Typed property includes value
    /// </summary>
    public interface IProperty<T> : IProperty {

        /// <summary>
        /// Property value
        /// </summary>
        T Value {
            get;
        }
    }
}