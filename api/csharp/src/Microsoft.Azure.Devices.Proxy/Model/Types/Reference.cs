// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Linq;
    using System.Runtime.Serialization;

    /// <summary>
    /// A reference identifies entities in the proxy
    /// </summary>
    [DataContract]
    public class Reference : Poco<Reference> {

        /// <summary>
        /// Null reference
        /// </summary>
        public static readonly Reference Null = new Reference(0x0);

        /// <summary>
        /// All references (Broadcast)
        /// </summary>
        public static readonly Reference All = new Reference(0xff);

        /// <summary>
        /// Internal address storage
        /// </summary>
        internal Guid storage;

        /// <summary>
        /// Serialization only
        /// </summary>
        [DataMember(Name = "addr", Order = 1)]
        public byte[] Address {
            get {
                return this.storage.ToByteArray();
            }
            set {
                storage = new Guid(value);
            }
        }

        /// <summary>
        /// Creates a new reference address
        /// </summary>
        public Reference() : this(Guid.NewGuid()) {}

        /// <summary>
        /// Creates a reference from a string, throws if string is not an address
        /// </summary>
        public static Reference Parse(string addressString) =>
            new Reference(Guid.Parse(addressString));

        /// <summary>
        /// Tries to create a reference from a string
        /// </summary>
        public static bool TryParse(string addressString, out Reference address) {
            Guid guid;
            if (!Guid.TryParse(addressString, out guid)) {
                address = null;
                return false;
            }
            address = new Reference(guid);
            return true;
        }

        internal Reference(byte fill) =>
            storage = new Guid(Enumerable.Repeat(fill, 16).ToArray());

        internal Reference(byte[] buf) : this(new Guid(buf)) {}

        internal Reference(Guid guid) => storage = guid;

        /// <summary>
        /// Returns whether this is a null reference
        /// </summary>
        /// <returns></returns>
        public bool IsNull() => Equals(Null);

        /// <summary>
        /// Returns whether this is a broadcast address
        /// </summary>
        /// <returns></returns>
        public bool IsBroadcast() => Equals(All);

        /// <summary>
        /// Converts this reference to a socket address
        /// </summary>
        /// <returns></returns>
        public SocketAddress ToSocketAddress() =>
            new Inet6SocketAddress(this.storage.ToByteArray());

        /// <summary>
        /// Converts this reference to a socket address
        /// </summary>
        /// <returns></returns>
        public static Reference FromSocketAddress(Inet6SocketAddress address) =>
            new Reference(address.Address);

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="obj"></param>
        /// <returns></returns>
        public override bool IsEqual(Reference address) =>
            IsEqual(storage, address.storage);

        /// <summary>
        /// Returns reference as string
        /// </summary>
        /// <returns></returns>
        public override string ToString() => storage.ToString("D");

        /// <summary>
        /// Returns hash of reference
        /// </summary>
        /// <returns></returns>
        protected override void SetHashCode() => MixToHash(storage);
    }
}
