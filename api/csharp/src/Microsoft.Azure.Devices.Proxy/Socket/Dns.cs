// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System.Threading.Tasks;
    using System;
    using System.Threading;
    using System.Collections.Generic;

    /// <summary>
    /// Provides name resolution api
    /// </summary>
    public static class Dns {

        #region Browse

        /// <summary>
        /// Resolve service record to a host name, port, plus txt records.
        /// </summary>
        /// <param name="proxy"></param>
        /// <param name="service"></param>
        /// <param name="cached">Only return items until cache is exhausted</param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public static Task<IDnsServiceResolver> BrowseAsync(SocketAddress proxy, DnsServiceRecord service, 
            bool cached, CancellationToken ct) =>
            BrowseClient.CreateServiceRecordResolverAsync(proxy, service, cached, ct);

        //
        // Resolve service record to a host name, port, plus txt records.
        //
        public static Task<IDnsServiceResolver> BrowseAsync(SocketAddress proxy, DnsServiceRecord record,
            CancellationToken ct) =>
            BrowseAsync(proxy, record, false, ct);

        //
        // Resolve service record to a host name, port, plus txt records.
        //
        public static Task<IDnsServiceResolver> BrowseAsync(DnsServiceRecord record,
            bool cached, CancellationToken ct) =>
            BrowseAsync(null, record, cached, ct);

        //
        // Resolve service record to a host name, port, plus txt records.
        //
        public static Task<IDnsServiceResolver> BrowseAsync(DnsServiceRecord record,
            CancellationToken ct) =>
            BrowseAsync(record, false, ct);

        /// <summary>
        /// Browse for service names on a particular proxy
        /// </summary>
        /// <param name="proxy"></param>
        /// <param name="serviceType"></param>
        /// <param name="domain"></param>
        /// <param name="cached">Only return items until cache is exhausted</param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public static Task<IDnsServiceBrowser> BrowseAsync(SocketAddress proxy, string serviceType, string domain,
            bool cached, CancellationToken ct) =>
            BrowseClient.CreateServiceRecordBrowserAsync(proxy, serviceType, domain ?? "local", cached, ct);

        //
        // Browse in cache
        //
        public static Task<IDnsServiceBrowser> BrowseAsync(SocketAddress proxy, string serviceType, string domain,
            CancellationToken ct) =>
            BrowseAsync(proxy, serviceType, domain, false, ct);

        //
        // Browse for service names with specific type in a domain
        //
        public static Task<IDnsServiceBrowser> BrowseAsync(string serviceType, string domain,
            bool cached, CancellationToken ct) =>
            BrowseAsync(null, serviceType, domain, cached, ct);

        //
        // Browse for service names with specific type in a domain
        //
        public static Task<IDnsServiceBrowser> BrowseAsync(string serviceType, string domain,
            CancellationToken ct) =>
            BrowseAsync(null, serviceType, domain, false, ct);

        //
        // Browse for service types in a domain on a proxy
        //
        public static Task<IDnsServiceBrowser> BrowseAsync(SocketAddress proxy, string domain,
            CancellationToken ct) =>
            BrowseAsync(proxy, null, domain, ct);

        //
        // Browse for service types in a domain
        //
        public static Task<IDnsServiceBrowser> BrowseAsync(string domain, 
            CancellationToken ct) =>
            BrowseAsync(null, null, domain, ct);
        
        //
        // Browse all domains on a particular proxy
        //
        public static Task<IDnsServiceBrowser> BrowseAsync(SocketAddress proxy, 
            CancellationToken ct) =>
            BrowseClient.CreateServiceRecordBrowserAsync(proxy, null, null, true, ct);

        //
        // Browse all domains
        //
        public static Task<IDnsServiceBrowser> BrowseAsync(CancellationToken ct) =>
            BrowseClient.CreateServiceRecordBrowserAsync(null, null, null, true, ct);

        #endregion

        #region Resolve

        /// <summary>
        /// Resolve dns address into dns host entries
        /// </summary>
        /// <param name="proxy"></param>
        /// <param name="address"></param>
        /// <param name="cached"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public static Task<IDnsHostEntryResolver> ResolveAsync(SocketAddress proxy, SocketAddress address, bool cached, 
            CancellationToken ct) {
            return BrowseClient.CreateHostEntryResolverAsync(proxy, address, cached, ct);
        }

        //
        // Resolve dns address entries
        //
        public static Task<IDnsHostEntryResolver> ResolveAsync(SocketAddress proxy, SocketAddress address, 
            CancellationToken ct) =>
            ResolveAsync(proxy, address, false, CancellationToken.None);

        //
        // Resolve dns address entries
        //
        public static Task<IDnsHostEntryResolver> ResolveAsync(SocketAddress proxy, SocketAddress address) =>
            ResolveAsync(proxy, address, CancellationToken.None);

        //
        // Resolve dns address entries
        //
        public static Task<IDnsHostEntryResolver> ResolveAsync(SocketAddress address, bool cached, CancellationToken ct) =>
            ResolveAsync(null, address, cached, ct);

        //
        // Resolve dns address entries
        //
        public static Task<IDnsHostEntryResolver> ResolveAsync(SocketAddress address, CancellationToken ct) =>
            ResolveAsync(null, address, ct);

        //
        // Resolve dns address entries
        //
        public static Task<IDnsHostEntryResolver> ResolveAsync(string hostName, ushort port, CancellationToken ct) =>
            ResolveAsync(new ProxySocketAddress(hostName, port), ct);

        //
        // Alias for GetHostEntryAsync
        //
        public static Task<DnsHostEntry> ResolveAsync(string hostName, CancellationToken ct) =>
            GetHostEntryAsync(hostName, ct);

        //
        // Alias for GetHostEntryAsync
        //
        public static Task<DnsHostEntry> ResolveAsync(string hostName) =>
            GetHostEntryAsync(hostName);

        //
        // Alias for GetHostEntry
        //
        public static DnsHostEntry Resolve(string hostName) =>
            GetHostEntry(hostName);

        //
        // Alias for BeginGetHostEntry
        //
        public static IAsyncResult BeginResolve(string hostName, AsyncCallback callback, object state) =>
            BeginGetHostEntry(hostName, callback, state);

        //
        // Alias for EndGetHostEntry
        //
        public static DnsHostEntry EndResolve(IAsyncResult result) =>
            EndGetHostEntry(result);

        #endregion

        #region GetHostEntry

        /// <summary>
        /// Collate returned entries into a list from cache
        /// </summary>
        /// <param name="address"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public static async Task<DnsHostEntry> GetHostEntryAsync(SocketAddress address, CancellationToken ct) {
            var resolver = await ResolveAsync(null, address, true, ct).ConfigureAwait(false);
            var entries = new List<DnsHostEntry>();
            while(await resolver.MoveNextAsync(ct).ConfigureAwait(false)) {
                entries.Add(resolver.Current);
            }
            return DnsHostEntry.ToEntry(address, entries);
        }

        /// <summary>
        /// Returns a host entry for a name
        /// </summary>
        /// <param name="hostName"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public static Task<DnsHostEntry> GetHostEntryAsync(string hostName, CancellationToken ct) =>
            GetHostEntryAsync(new ProxySocketAddress(hostName, 0), ct);

        //
        // Version of GetHostEntry without cancellation token
        //
        public static Task<DnsHostEntry> GetHostEntryAsync(string hostName) =>
            GetHostEntryAsync(hostName, CancellationToken.None);

        //
        // Version of GetHostEntry without cancellation token
        //
        public static Task<DnsHostEntry> GetHostEntryAsync(SocketAddress address) =>
            GetHostEntryAsync(address, CancellationToken.None);

        //
        // Sync version of GetHostEntry
        //
        public static DnsHostEntry GetHostEntry(string hostName) =>
            TaskToSync.Run(() => GetHostEntryAsync(hostName));

        //
        // Sync version of GetHostEntry
        //
        public static DnsHostEntry GetHostEntry(SocketAddress address) =>
            TaskToSync.Run(() => GetHostEntryAsync(address));

        //
        // Begin GetHostEntry
        //
        public static IAsyncResult BeginGetHostEntry(string hostName, AsyncCallback callback, object state) =>
            TaskToApm.Begin(GetHostEntryAsync(hostName), callback, state);

        //
        // Begin GetHostEntry
        //
        public static IAsyncResult BeginGetHostEntry(SocketAddress address, AsyncCallback callback, object state) =>
            TaskToApm.Begin(GetHostEntryAsync(address), callback, state);

        //
        // End GetHostEntry
        //
        public static DnsHostEntry EndGetHostEntry(IAsyncResult result) =>
            TaskToApm.End<DnsHostEntry>(result);

        #endregion

        #region GetHostByName

        //
        // Alias for GetHostEntryAsync
        //
        public static Task<DnsHostEntry> GetHostByNameAsync(string hostName, CancellationToken ct) =>
            GetHostEntryAsync(hostName, ct);

        //
        // Alias for GetHostEntryAsync
        //
        public static Task<DnsHostEntry> GetHostByNameAsync(string hostName) =>
            GetHostEntryAsync(hostName);

        //
        // Alias for GetHostEntry
        //
        public static DnsHostEntry GetHostByName(string hostName) =>
            GetHostEntry(hostName);

        //
        // Alias for BeginGetHostEntry
        //
        public static IAsyncResult BeginGetHostByName(string hostName, AsyncCallback callback, object state) =>
            BeginGetHostEntry(hostName, callback, state);

        //
        // Alias for EndGetHostEntry
        //
        public static DnsHostEntry EndGetHostByName(IAsyncResult result) =>
            EndGetHostEntry(result);

        #endregion

        #region GetHostAddresses

        //
        // Get addresses for host
        //
        public static async Task<SocketAddress[]> GetHostAddressesAsync(string hostName, CancellationToken ct) {
            var result = await GetHostByNameAsync(hostName, ct).ConfigureAwait(false);
            return result.AddressList;
        }

        //
        // Get addresses for host
        //
        public static Task<SocketAddress[]> GetHostAddressesAsync(string hostName) =>
            GetHostAddressesAsync(hostName, CancellationToken.None);

        //
        // Begin GetHostAddresses
        //
        public static IAsyncResult BeginGetHostAddresses(string hostName, AsyncCallback callback, object state) =>
            TaskToApm.Begin(GetHostAddressesAsync(hostName), callback, state);

        //
        // End GetHostAddresses
        //
        public static SocketAddress[] EndGetHostAddresses(IAsyncResult result) =>
            TaskToApm.End<SocketAddress[]>(result);

        //
        // Get addresses for host
        //
        public static SocketAddress[] GetHostAddresses(string hostName) {
            return GetHostByName(hostName).AddressList;
        }

        #endregion

        #region GetHostByAddress

        //
        // Alias for GetHostEntryAsync
        //
        public static Task<DnsHostEntry> GetHostByAddressAsync(SocketAddress address, CancellationToken ct) =>
            GetHostEntryAsync(address, ct);

        //
        // Alias for GetHostEntryAsync
        //
        public static Task<DnsHostEntry> GetHostByAddressAsync(SocketAddress address) =>
            GetHostEntryAsync(address);

        //
        // Alias for GetHostEntry
        //
        public static DnsHostEntry GetHostByAddress(SocketAddress address) =>
            GetHostEntry(address);

        //
        // Alias for BeginGetHostEntry
        //
        public static IAsyncResult BeginGetHostByAddress(SocketAddress address, AsyncCallback callback, object state) =>
            BeginGetHostEntry(address, callback, state);

        //
        // Alias for EndGetHostEntry
        //
        public static DnsHostEntry EndGetHostByAddress(IAsyncResult result) =>
            EndGetHostEntry(result);

        #endregion
    }
}