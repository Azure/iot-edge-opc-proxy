// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

using System.Text;

namespace Microsoft.Azure.Devices.Proxy {

    /// <summary>
    /// Container class for port scan result infos
    /// </summary>
    public class PortScanResult : Poco<PortScanResult> {

        /// <summary>
        /// Address with the found port on the host
        /// </summary>
        public SocketAddress Result {
            get; private set;
        }

        /// <summary>
        /// Returns the name of the port
        /// </summary>
        public string Name {
            get; private set;
        }

        /// <summary>
        /// Proxy on which this record is valid.
        /// </summary>
        public SocketAddress Interface {
            get; private set;
        }

        /// <summary>
        /// Create entry
        /// </summary>
        /// <param name="result"></param>
        /// <param name="interface"></param>
        /// <returns></returns>
        internal static PortScanResult Create(SocketAddress result,
            Property<byte[]> property, SocketAddress @interface) {
            var entry = Get();
            entry.Result = result;
            entry.Name = property?.Value != null ?
               Encoding.UTF8.GetString(property.Value, 0, property.Value.Length - 1) : "Unknown";
            entry.Interface = @interface;
            return entry;
        }

        public override bool IsEqual(PortScanResult that) {
            return
                IsEqual(Result, that.Result) &&
                IsEqual(Interface, that.Interface);
        }

        protected override void SetHashCode() {
            MixToHash(Result);
            MixToHash(Interface);
        }

        /// <summary>
        /// Return object as string
        /// </summary>
        /// <returns></returns>
        public override string ToString() => $"{Result} ({Name})";
    }
}