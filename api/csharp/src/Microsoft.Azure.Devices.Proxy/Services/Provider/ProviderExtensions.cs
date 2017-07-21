// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Linq.Expressions;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Threading.Tasks.Dataflow;

    internal static class ProviderExtensions {

        /// <summary>
        /// Lookup records with a name service query
        /// </summary>
        /// <param name="query"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public static async Task<IEnumerable<INameRecord>> LookupAsync(this INameService service,
            Expression<Func<INameRecord, bool>> query, CancellationToken ct) {
            var result = new List<INameRecord>();
            var lookup = service.Read(new ExecutionDataflowBlockOptions { CancellationToken = ct });
            await lookup.SendAsync(query).ConfigureAwait(false);
            lookup.Complete();
            while (await lookup.OutputAvailableAsync(ct).ConfigureAwait(false)) {
                result.Add(await lookup.ReceiveAsync(ct).ConfigureAwait(false));
            }
            return result;
        }

        /// <summary>
        /// Lookup records by name
        /// </summary>
        /// <param name="name"></param>
        /// <param name="type"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public static Task<IEnumerable<INameRecord>> LookupAsync(this INameService service,
            string name, NameRecordType type, CancellationToken ct) =>
            LookupAsync(service, r => r.Matches(name, type), ct);

        /// <summary>
        /// Lookup records by address
        /// </summary>
        /// <param name="address"></param>
        /// <param name="type"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public static Task<IEnumerable<INameRecord>> LookupAsync(this INameService service,
            Reference address, NameRecordType type, CancellationToken ct) =>
            LookupAsync(service, r => r.Matches(address, type), ct);

        /// <summary>
        /// Adds or updates a record in the name service
        /// </summary>
        /// <param name="service"></param>
        /// <param name="record"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public static Task AddOrUpdateAsync(this INameService service,
            INameRecord record, CancellationToken ct) =>
            service.Write.SendAsync(Tuple.Create(record, NameServiceOperation.Update), ct);

        /// <summary>
        /// Removes a record in the name service
        /// </summary>
        /// <param name="service"></param>
        /// <param name="record"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public static Task RemoveAsync(this INameService service,
            INameRecord record, CancellationToken ct) =>
            service.Write.SendAsync(Tuple.Create(record, NameServiceOperation.Remove), ct);

        /// <summary>
        /// Match record against addres and type
        /// </summary>
        /// <param name="record"></param>
        /// <param name="address"></param>
        /// <param name="type"></param>
        /// <returns></returns>
        public static bool Matches(this INameRecord record, Reference address, NameRecordType type) {
            if (!record.Type.HasFlag(type)) {
                return false;
            }
            return (address == Reference.All || record.Address.Equals(address));
        }

        /// <summary>
        /// Match record against addres and type
        /// </summary>
        /// <param name="record"></param>
        /// <param name="addresses"></param>
        /// <param name="type"></param>
        /// <returns></returns>
        public static bool Matches(this INameRecord record, IEnumerable<Reference> addresses,
            NameRecordType type) =>
            addresses.Any(a => record.Matches(a, type));

        /// <summary>
        /// Match record against name and type
        /// </summary>
        /// <param name="record"></param>
        /// <param name="name"></param>
        /// <param name="type"></param>
        /// <returns></returns>
        public static bool Matches(this INameRecord record, string name, NameRecordType type) {
            if (!record.Type.HasFlag(type)) {
                return false;
            }
            return name == null ||
               (record.Name.Equals(name, StringComparison.CurrentCultureIgnoreCase) ||
                record.Id.Equals(name, StringComparison.CurrentCultureIgnoreCase));
        }

        /// <summary>
        /// Match record against address
        /// </summary>
        /// <param name="record"></param>
        /// <param name="address"></param>
        /// <param name="type"></param>
        /// <returns></returns>
        public static bool Matches(this INameRecord record, SocketAddress address,
            NameRecordType type) {
            if (address.Family == AddressFamily.Collection) {
                return Matches(record, (SocketAddressCollection)address, type);
            }
            if (Matches(record, address.ToString(), type)) {
                return true;
            }
            if (address.Family == AddressFamily.InterNetworkV6) {
                return Matches(record, Reference.FromSocketAddress((Inet6SocketAddress)address), type);
            }
            return false;
        }

        /// <summary>
        /// Match record against list of addresses
        /// </summary>
        /// <param name="record"></param>
        /// <param name="addresses"></param>
        /// <param name="type"></param>
        /// <returns></returns>
        public static bool Matches(this INameRecord record, SocketAddressCollection addresses,
            NameRecordType type) =>
            addresses.Addresses().Any(a => record.Matches(a, type));
    }
}
