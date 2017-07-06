// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
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
        abstract class BrowserAsyncEnumerator<T> : IAsyncEnumerator<T>, IDisposable {
            internal BrowseSocket Socket {
                get; set;
            }

            internal bool CacheOnly {
                get; set;
            }

            internal bool Throw {
                get; set;
            }

            /// <summary>
            /// Current item
            /// </summary>
            public T Current {
                get => _cur;
            }

            /// <summary>
            /// Move to next record
            /// </summary>
            /// <param name="ct"></param>
            /// <returns></returns>
            public async Task<bool> MoveNextAsync(CancellationToken ct) {
                if (_disposed) {
                    throw new ObjectDisposedException(this.GetType().Name);
                }
                if (_done) {
                    return false;
                }
                while (true) {
                    var response = await Socket.BrowseNextAsync(ct).ConfigureAwait(false);
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
                            if (Throw) {
                                throw new BrowseException((SocketError)response.Error);
                            }
                         //   ProxyEventSource
                            _done = true;
                            return false;
                        }
                    }
                    else {
                        _cur = Yield(response);
                        if (_cur != null) {
                            return true;
                        }
                    }
                }
            }

            /// <summary>
            /// Dispose of enumerator
            /// </summary>
            public void Dispose() {
                if (!_disposed) {
                    _disposed = true;
                    try {
                        Socket.CloseAsync(CancellationToken.None).Wait();
                    }
                    catch { }
                    Socket.Dispose();
                }
            }

            protected abstract T Yield(BrowseResponse response);

            private T _cur;
            private bool _done;
            private bool _disposed = false;
        }

        /// <summary>
        /// Enumerator for service records
        /// </summary>
        class ServiceRecordBrowser : BrowserAsyncEnumerator<DnsServiceRecord>, 
            IDnsServiceBrowser {
            protected override DnsServiceRecord Yield(BrowseResponse response) =>
                DnsServiceRecord.FromSocketAddress(response.Item as ProxySocketAddress, 
                    response.Interface, (response.Flags & BrowseResponse.Removed) != 0);
        }

        /// <summary>
        /// Enumerator for service entries (hostname:port)
        /// </summary>
        class ServiceRecordResolver : BrowserAsyncEnumerator<DnsServiceEntry>,
            IDnsServiceResolver {
            private readonly DnsServiceRecord _service;
            internal ServiceRecordResolver(DnsServiceRecord service) {
                _service = service;
            }
            protected override DnsServiceEntry Yield(BrowseResponse response) =>
                DnsServiceEntry.Create(
                    new BoundSocketAddress(response.Interface, response.Item),
                    DnsServiceRecord.Create(_service, response.Interface, 
                        (response.Flags & BrowseResponse.Removed) != 0
                    ),
                    response.Properties
                        .Where(p => p.Type == (uint)DnsRecordType.Txt)
                        .Select(p => DnsTxtRecord.Create(((Property<byte[]>)p).Value)).ToArray()
                );
        }

        /// <summary>
        /// Enumerator for resolved hosts/addresses
        /// </summary>
        class HostEntryResolver : BrowserAsyncEnumerator<DnsHostEntry>,
            IDnsHostEntryResolver {
            private readonly ProxySocketAddress _host;
            internal HostEntryResolver(ProxySocketAddress host) {
                _host = host;
            }
            protected override DnsHostEntry Yield(BrowseResponse response) =>
                DnsHostEntry.Create(
                    _host.Host,
                    response.Properties
                        .Where(p => p.Type == (uint)PropertyType.AddressInfo)
                        .Select(p => ((Property<AddressInfo>)p).Value.CanonicalName).ToArray(),
                    new SocketAddress[] {
                        new BoundSocketAddress(response.Interface, response.Item)
                    },
                    response.Interface
                );
        }

        /// <summary>
        /// Directory enumerator
        /// </summary>
        class DirectoryBrowser : BrowserAsyncEnumerator<FileEntry>,
            IDirectoryBrowser {
            protected override FileEntry Yield(BrowseResponse response) =>
                FileEntry.Create(
                    response.Item.AsProxySocketAddress().Host,
                    ((Property<FileInfo>)response.Properties.FirstOrDefault())?.Value,
                    response.Interface
                );
        }

        /// <summary>
        /// Create service record resolving enumerator
        /// </summary>
        /// <param name="proxy">The proxy endpoint(s) to use</param>
        /// <param name="record">The service record to resolve</param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public static async Task<IDnsServiceResolver> CreateServiceRecordResolverAsync(
            SocketAddress proxy, DnsServiceRecord record, bool cacheOnly, CancellationToken ct) {
            if (string.IsNullOrEmpty(record.Name) || string.IsNullOrEmpty(record.Type) ||
                record.Removed) {
                throw new ArgumentException("Invalid service, no name, not type or been removed",
                    nameof(record));
            }
            var socket = new BrowseSocket(Socket.Provider);
            try { 
                await socket.ConnectAsync(proxy ?? record.Interface, ct).ConfigureAwait(false);
                await socket.BrowseBeginAsync(record.ToSocketAddress(), BrowseRequest.Service, 
                    ct).ConfigureAwait(false);
                return new ServiceRecordResolver(record) { Socket = socket, CacheOnly = cacheOnly };
            }
            catch(Exception) {
                socket.Dispose();
                throw;
            }
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
        public static async Task<IDnsServiceBrowser> CreateServiceRecordBrowserAsync(
            SocketAddress proxy, string type, string domain, bool cacheOnly, CancellationToken ct) {
            var socket = new BrowseSocket(Socket.Provider);
            try {
                await socket.ConnectAsync(proxy, ct).ConfigureAwait(false);
                using (var record = DnsServiceRecord.Create(null,
                    string.IsNullOrEmpty(type) ? null : type,
                    string.IsNullOrEmpty(domain) ? null : domain, null)) {
                    using (var address = record.ToSocketAddress()) {
                        await socket.BrowseBeginAsync(address, BrowseRequest.Service, 
                            ct).ConfigureAwait(false);
                    }
                }
                return new ServiceRecordBrowser() { Socket = socket, CacheOnly = cacheOnly };
            }
            catch(Exception) {
                socket.Dispose();
                throw;
            }
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
            var socket = new BrowseSocket(Socket.Provider);
            try {
                await socket.ConnectAsync(proxy, ct).ConfigureAwait(false);
                await socket.BrowseBeginAsync(address, BrowseRequest.Resolve,
                    ct).ConfigureAwait(false);
                return new HostEntryResolver(address) { Socket = socket, CacheOnly = cacheOnly };
            }
            catch (Exception) {
                socket.Dispose();
                throw;
            }
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
            var socket = new BrowseSocket(Socket.Provider);
            try {
                await socket.ConnectAsync(proxy, ct).ConfigureAwait(false);
                await socket.BrowseBeginAsync(new ProxySocketAddress(folder ?? ""), BrowseRequest.Dirpath, 
                    ct).ConfigureAwait(false);
                return new DirectoryBrowser() { Socket = socket, CacheOnly = cacheOnly, Throw = true };
            }
            catch(Exception) {
                socket.Dispose();
                throw;
            }
        }
    }
}
