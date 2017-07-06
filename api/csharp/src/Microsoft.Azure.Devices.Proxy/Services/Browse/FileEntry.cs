// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;

    /// <summary>
    /// A browsed file entry in a directory.
    /// </summary>
    public class FileEntry : Poco<FileEntry> {

        /// <summary>
        /// Full file name (includes path)
        /// </summary>
        public string FileName {
            get; private set;
        }

        /// <summary>
        /// File info for this file entry.  Includes file type.
        /// </summary>
        public FileInfo Info {
            get; private set;
        }

        /// <summary>
        /// Proxy on which this file entry is valid.
        /// </summary>
        public SocketAddress Interface {
            get; private set;
        }

        /// <summary>
        /// Create file entry
        /// </summary>
        /// <param name="fileName"></param>
        /// <param name="info"></param>
        /// <param name="interface"></param>
        /// <returns></returns>
        internal static FileEntry Create(string fileName, FileInfo info, 
            SocketAddress @interface) {
            var entry = Get();
            entry.FileName = fileName;
            entry.Info = info;
            entry.Interface = @interface;
            return entry;
        }

        public override bool IsEqual(FileEntry that) {
            return
                IsEqual(FileName, that.FileName) &&
                IsEqual(Info, that.Info) &&
                IsEqual(Interface, that.Interface);
        }

        protected override void SetHashCode() {
            MixToHash(FileName);
            MixToHash(Info);
            MixToHash(Interface);
        }

        public override string ToString() => $"{FileName} ({Info})";
    }
} 