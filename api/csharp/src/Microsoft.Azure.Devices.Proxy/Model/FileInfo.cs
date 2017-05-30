// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy.Model {
    using System;
    using System.Runtime.Serialization;

    //
    // File types
    //
    public enum FileType {
        Unknown = 0,
        File,
        Directory,
        Link,
        BlockDevice,
        CharDevice,
        Pipe,
        Socket
    };

    /// <summary>
    /// Platform independent file info
    /// </summary>
    [DataContract]
    public class FileInfo : IEquatable<FileInfo> {

        /// <summary>
        /// Id of file
        /// </summary>
        [DataMember(Name = "inode_number", Order = 1)]
        public long NodeId { get; set; }

        /// <summary>
        /// Device the file is on
        /// </summary>
        [DataMember(Name = "device_id", Order = 2)]
        public long DeviceId { get; set; }

        /// <summary>
        /// Type of file
        /// </summary>
        [DataMember(Name = "type", Order = 3)]
        public int Type { get; set; }

        /// <summary>
        /// Size of file
        /// </summary>
        [DataMember(Name = "total_size", Order = 4)]
        public long Size { get; set; }

        /// <summary>
        /// Last time file was accessed
        /// </summary>
        [DataMember(Name = "last_atime", Order = 5)]
        public long LastAccessedAsUnixTime { get; set; }

        /// <summary>
        /// Last time the file was modified
        /// </summary>
        [DataMember(Name = "last_mtime", Order = 6)]
        public long LastModifiedAsUnixTime { get; set; }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(FileInfo that) {
            if (that == null) {
                return false;
            }
            return
                this.NodeId.Equals(that.NodeId) &&
                this.DeviceId.Equals(that.DeviceId) &&
                this.Type.Equals(that.Type) &&
                this.Size.Equals(that.Size) &&
                this.LastAccessedAsUnixTime.Equals(that.LastAccessedAsUnixTime) &&
                this.LastModifiedAsUnixTime.Equals(that.LastModifiedAsUnixTime);
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(Object that) {
            return Equals(that as FileInfo);
        }

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return ((((((
                NodeId.GetHashCode() * 31) ^
                DeviceId.GetHashCode()) * 31) ^
                Type.GetHashCode() * 31) ^
                Size.GetHashCode() * 31) ^
                LastAccessedAsUnixTime.GetHashCode() * 31) ^
                LastModifiedAsUnixTime.GetHashCode();
        }
    }
}