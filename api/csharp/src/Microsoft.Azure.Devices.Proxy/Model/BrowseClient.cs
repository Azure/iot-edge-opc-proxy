// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Model {
    using System;
    using System.Linq;
    using System.Text;
    using System.Threading;
    using System.Threading.Tasks;

    /// <summary>
    /// Actual browsing implementation, uses browse socket per request
    /// </summary>
    public class BrowseClient {

        /// <summary>
        /// Base enumerator implementation wrapping the browse socket
        /// </summary>
        abstract class BrowseSocketAsyncEnumerator<T> : IAsyncEnumerator<T> {
            internal BrowseSocket Socket { get; set; }
            internal bool CacheOnly { get; set; }

            /// <summary>
            /// Current item
            /// </summary>
            public T Current { get { return _cur; } }

            /// <summary>
            /// Move to next record
            /// </summary>
            /// <param name="ct"></param>
            /// <returns></returns>
            public async Task<bool> MoveNextAsync(CancellationToken ct) {
                if (_done)
                    return false;
                while (true) {
                    var response = await Socket.BrowseNextAsync(ct);
                    if (response == null) {
                        _done = true;
                        return false;
                    }
                    _done = (response.Flags & BrowseResponse.Eos) != 0;
                    if (!_done && CacheOnly) {
                        if ((response.Flags & BrowseResponse.AllForNow) != 0) {
                            _done = true;
                        }
                    }
                    if ((response.Flags & BrowseResponse.Empty) != 0) {
                        if (_done) {
                            return false;
                        }
                        if (response.Error != 0) {
                            throw new BrowseException((SocketError)response.Error);
                        }
                    }
                    else {
                        _cur = Yield(response);
                        if (_cur != null)
                            return true;
                    }
                }
            }

            /// <summary>
            /// Dispose of enumerator
            /// </summary>
            public void Dispose() {
                Socket.CloseAsync(CancellationToken.None).Wait();
            }

            protected abstract T Yield(BrowseResponse response);

            private T _cur;
            private bool _done;
        }

        /// <summary>
        /// Enumerator for service records
        /// </summary>
        class ServiceRecordBrowser : BrowseSocketAsyncEnumerator<DnsServiceRecord>, 
            IDnsServiceRecordBrowser {
            protected override DnsServiceRecord Yield(BrowseResponse response) {
                var record = DnsServiceRecord.FromSocketAddress(response.Item as ProxySocketAddress);
                record.Interface = response.Interface;
                record.Removed = (response.Flags & BrowseResponse.Removed) != 0;
                return record;
            }
        }

        /// <summary>
        /// Enumerator for service entries (hostname:port)
        /// </summary>
        class ServiceRecordResolver : BrowseSocketAsyncEnumerator<DnsServiceEntry>,
            IDnsServiceRecordResolver {
            private readonly DnsServiceRecord _service;
            internal ServiceRecordResolver(DnsServiceRecord service) {
                _service = service;
            }
            protected override DnsServiceEntry Yield(BrowseResponse response) =>
                new DnsServiceEntry {
                    Address = response.Item as ProxySocketAddress,
                    Service = new DnsServiceRecord(_service) {
                        Interface = response.Interface,
                        Removed = (response.Flags & BrowseResponse.Removed) != 0
                    },
                    TxtRecords = response.Properties
                        .Where(p => p.Type == (uint)DnsRecordType.Txt)
                        .Select(p => new TxtRecord(((Property<byte[]>)p).Value)).ToArray()
                };
        }

        /// <summary>
        /// Enumerator for resolved hosts/addresses
        /// </summary>
        class HostEntryResolver : BrowseSocketAsyncEnumerator<DnsHostEntry>,
            IDnsHostEntryResolver {
            private readonly ProxySocketAddress _host;
            internal HostEntryResolver(ProxySocketAddress host) {
                _host = host;
            }
            protected override DnsHostEntry Yield(BrowseResponse response) =>
                new DnsHostEntry {
                    AddressList = new SocketAddress[] { response.Item },
                    HostName = _host.Host,
                    Interface = response.Interface,
                    Aliases = response.Properties
                        .Where(p => p.Type == (uint)DnsRecordType.Simple)
                        .Select(p => Encoding.UTF8.GetString(((Property<byte[]>)p).Value)).ToArray()
                };
        }

        /// <summary>
        /// Directory enumerator
        /// </summary>
        class DirectoryBrowser : BrowseSocketAsyncEnumerator<FileEntry>,
            IDirectoryBrowser {
            protected override FileEntry Yield(BrowseResponse response) =>
                new FileEntry {
                    FileName = response.Item.AsProxySocketAddress().Host,
                    Interface = response.Interface,
                    Info = ((Property<FileInfo>)response.Properties.First()).Value
                };
        }

        /// <summary>
        /// Create service record resolving enumerator
        /// </summary>
        /// <param name="proxy">The proxy endpoint(s) to use</param>
        /// <param name="record">The service record to resolve</param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public static async Task<IDnsServiceRecordResolver> CreateServiceRecordResolverAsync(
            SocketAddress proxy, DnsServiceRecord record, bool cacheOnly, CancellationToken ct) {
            if (string.IsNullOrEmpty(record.Name) || string.IsNullOrEmpty(record.Type) ||
                record.Removed) {
                throw new ArgumentException("Invalid service, no name, not type or been removed",
                    nameof(record));
            }
            var socket = new BrowseSocket(Socket.Provider, BrowseRequest.Service);
            await socket.ConnectAsync(proxy ?? record.Interface, ct);
            await socket.BrowseBeginAsync(record.ToSocketAddress(), ct);
            return new ServiceRecordResolver(record) { Socket = socket, CacheOnly = cacheOnly };
        }

        /// <summary>
        /// Create service browser - returns service records, which identifiy either 
        /// - domains (if both domain and type are null)
        /// - service types (if only domain are given)
        /// - Service names - with type and domain (if both type and domain are provded)
        /// </summary>
        /// <param name="proxy">Proxy endpoint(s) to use for resolve</param>
        /// <param name="type">service type to browse or null</param>
        /// <param name="domain">domain to browse or null</param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public static async Task<IDnsServiceRecordBrowser> CreateServiceRecordBrowserAsync(
            SocketAddress proxy, string type, string domain, bool cacheOnly, CancellationToken ct) {
            var socket = new BrowseSocket(Socket.Provider, BrowseRequest.Service);
            await socket.ConnectAsync(proxy, ct);
            await socket.BrowseBeginAsync(new DnsServiceRecord {
                Type = string.IsNullOrEmpty(type) ? null : type,
                Domain = string.IsNullOrEmpty(domain) ? null : domain
            }.ToSocketAddress(), ct);
            return new ServiceRecordBrowser() { Socket = socket, CacheOnly = cacheOnly };
        }

        /// <summary>
        /// Create host resolving enumerator (enumerates all addresses for a particular host address).
        /// Performs gethostbyaddr and gethostbyname type functionality (i.e. getaddrinfo).
        /// </summary>
        /// <param name="proxy">Proxy endpoint(s) to use for resolve</param>
        /// <param name="host">Address to resolve</param>
        /// <param name="cacheOnly">Only deliver from cache</param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public static async Task<IDnsHostEntryResolver> CreateHostEntryResolverAsync(
            SocketAddress proxy, SocketAddress host, bool cacheOnly, CancellationToken ct) {
            var address = host.AsProxySocketAddress();
            if (address == null) {
                throw new ArgumentException(nameof(host));
            }
            var socket = new BrowseSocket(Socket.Provider, BrowseRequest.Resolve);
            await socket.ConnectAsync(proxy, ct);
            await socket.BrowseBeginAsync(address, ct);
            return new HostEntryResolver(address) { Socket = socket, CacheOnly = cacheOnly };
        }

        /// <summary>
        /// Create directory browser to browse remote directories.  Catch BrowseException
        /// and check code to understand e.g. permission issues, etc. 
        /// </summary>
        /// <param name="folder">Folder name</param>
        /// <param name="cacheOnly">Only deliver from cache</param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public static async Task<IDirectoryBrowser> CreateDirectoryBrowserAsync(
            SocketAddress proxy, string folder, bool cacheOnly, CancellationToken ct) {
            var socket = new BrowseSocket(Socket.Provider, BrowseRequest.Dirpath);
            await socket.ConnectAsync(proxy, ct);
            await socket.BrowseBeginAsync(new ProxySocketAddress(folder ?? ""), ct);
            return new DirectoryBrowser() { Socket = socket, CacheOnly = cacheOnly };
        }
    }
}
