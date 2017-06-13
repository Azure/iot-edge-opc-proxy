// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;

    /// <summary>
    /// A browsed file entry in a directory.
    /// </summary>
    public class FileEntry : IEquatable<FileEntry> {

        /// <summary>
        /// Full file name (includes path)
        /// </summary>
        public string FileName { get; internal set; }

        /// <summary>
        /// File info for this file entry.  Includes file type.
        /// </summary>
        public FileInfo Info { get; internal set; }

        /// <summary>
        /// Proxy on which this file entry is valid.
        /// </summary>
        public SocketAddress Interface { get; internal set; }

        /// <summary>
        /// Return object as string
        /// </summary>
        /// <returns></returns>
        public override string ToString() => $"{FileName} ({Info})";

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(Object obj) {
            return this.Equals(obj as FileEntry);
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(FileEntry that) {
            if (that == null) {
                return false;
            }
            return
                this.FileName.Equals(that.FileName) &&
                this.Info.Equals(that.Info) &&
                this.Interface.Equals(that.Interface);
        }

        /// <summary>
        /// Returns hash code
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return ((
                FileName.GetHashCode() * 31) ^
                Info.GetHashCode() * 31) ^
                Interface.GetHashCode();
        }
    }
} 