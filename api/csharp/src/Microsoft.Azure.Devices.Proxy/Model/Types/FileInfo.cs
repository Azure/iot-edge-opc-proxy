// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Runtime.Serialization;

    /// <summary>
    /// Platform independent file info
    /// </summary>
    [DataContract]
    public class FileInfo : Poco<FileInfo> {

        /// <summary>
        /// Id of file
        /// </summary>
        [DataMember(Name = "inode_number", Order = 1)]
        public long NodeId {
            get; set;
        }

        /// <summary>
        /// Device the file is on
        /// </summary>
        [DataMember(Name = "device_id", Order = 2)]
        public long DeviceId {
            get; set;
        }

        /// <summary>
        /// Type of file
        /// </summary>
        [DataMember(Name = "type", Order = 3)]
        public int Type {
            get; set;
        }

        /// <summary>
        /// Size of file
        /// </summary>
        [DataMember(Name = "total_size", Order = 4)]
        public long Size {
            get; set;
        }

        /// <summary>
        /// Last time file was accessed
        /// </summary>
        [DataMember(Name = "last_atime", Order = 5)]
        public long LastAccessedAsUnixTime {
            get; set;
        }

        /// <summary>
        /// Last time the file was modified
        /// </summary>
        [DataMember(Name = "last_mtime", Order = 6)]
        public long LastModifiedAsUnixTime {
            get; set;
        }

        /// <summary>
        /// Create file info
        /// </summary>
        /// <param name="nodeId"></param>
        /// <param name="deviceId"></param>
        /// <param name="type"></param>
        /// <param name="size"></param>
        /// <param name="lastAccessedAsUnixTime"></param>
        /// <param name="lastModifiedAsUnixTime"></param>
        /// <returns></returns>
        public static FileInfo Create(long nodeId, long deviceId, int type,
            long size, long lastAccessedAsUnixTime, long lastModifiedAsUnixTime) {
            var info = Get();
            info.NodeId = nodeId;
            info.DeviceId = deviceId;
            info.Type = type;
            info.Size = size;
            info.LastAccessedAsUnixTime = lastAccessedAsUnixTime;
            info.LastModifiedAsUnixTime = lastModifiedAsUnixTime;
            return info;
        }

        public FileInfo Clone() => Create(NodeId, DeviceId, Type, Size, 
            LastAccessedAsUnixTime, LastModifiedAsUnixTime);

        /// <summary>
        /// Return object as string
        /// </summary>
        /// <returns></returns>
        public override string ToString() =>
            $"{((FileType)Type).ToString()} Size: {Size}";

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool IsEqual(FileInfo that) {
            return
                IsEqual(NodeId, that.NodeId) &&
                IsEqual(DeviceId, that.DeviceId) &&
                IsEqual(Type, that.Type) &&
                IsEqual(Size, that.Size) &&
                IsEqual(LastAccessedAsUnixTime, that.LastAccessedAsUnixTime) &&
                IsEqual(LastModifiedAsUnixTime, that.LastModifiedAsUnixTime);
        }

        protected override void SetHashCode() {
            MixToHash(NodeId);
            MixToHash(DeviceId);
            MixToHash(Type);
            MixToHash(Size);
            MixToHash(LastAccessedAsUnixTime);
            MixToHash(LastModifiedAsUnixTime);
        }
    }
}