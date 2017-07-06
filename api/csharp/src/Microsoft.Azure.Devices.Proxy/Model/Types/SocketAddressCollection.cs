// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Linq;
    using System.Collections.Generic;
    using System.Runtime.Serialization;
    using System.Text;

    /// <summary>
    /// A list of socket addresses
    /// </summary>
    [DataContract]
    public class SocketAddressCollection : SocketAddress, IEquatable<SocketAddressCollection> {

        [DataMember(Name = "family", Order = 1)]
        public override AddressFamily Family {
            get => AddressFamily.Collection;
        }

        /// <summary>
        /// Storage of socket addresses
        /// </summary>
        [DataMember(Name = "includes", Order = 2)]
        HashSet<SocketAddress> Inner {
            get; set;
        }

        /// <summary>
        /// Create an address from a list
        /// </summary>
        /// <param name="addresses"></param>
        /// <returns></returns>
        public static SocketAddress Create(IEnumerable<SocketAddress> addresses) {
            var set = new HashSet<SocketAddress>(addresses);
            if (!set.Any()) {
                return null;
            } else {
                var address = new SocketAddressCollection {
                    Inner = set
                };
                try {
                    return address.Addresses().Single();
                } catch {
                    return address;
                }
            }
        }

        /// <summary>
        /// Return flattened list of addresses
        /// </summary>
        /// <returns></returns>
        public IEnumerable<SocketAddress> Addresses() {
            foreach(var address in Inner) {
                var collection = address as SocketAddressCollection;
                if (collection != null) {
                    foreach (var r in collection.Addresses()) {
                        yield return r;
                    }
                }
                else if (!(address is AnySocketAddress)) {
                    yield return address;
                }
            }
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(SocketAddressCollection that) {
            if (that == null) {
                return false;
            }
            return
                IsEqual(that as SocketAddress) &&
                Inner.SetEquals(that.Inner);
        }

        public override bool IsEqual(object that) => Equals(that as SocketAddressCollection);

        protected override void SetHashCode() {
            MixToHash(Family);
            MixToHash(Inner);
        }

        /// <summary>
        /// Stringify address
        /// </summary>
        /// <returns></returns>
        public override string ToString() {
            var bld = new StringBuilder();
            foreach(var a in Addresses()) {
                bld.AppendLine(a.ToString());
            }
            return bld.ToString();
        }


        /// <summary>
        /// Returns the first address if any as proxy address
        /// </summary>
        /// <returns></returns>
        public override ProxySocketAddress AsProxySocketAddress() =>
            Addresses().First().AsProxySocketAddress();

    }
}