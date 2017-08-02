// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Samples {
    using System;
    using System.Linq;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Collections.Generic;
    using Microsoft.Azure.Devices.Proxy;

    class Program {

        enum Op {
            None, Browse, Resolve, Dir, Fs, Net, Port, All
        }

        /// <summary>
        /// Main program entry point
        /// </summary>
        static void Main(string[] args) {
            Op op = Op.None;
            bool cache = false;
            int period = 60 * 1000;
            ushort port = 0;
            string host = "";
            DnsServiceRecord record = null;
            ProxySocketAddress address = null;
            // Parse command line
            try {
                for (int i = 0; i < args.Length; i++) {
                    switch (args[i]) {
                        case "-a":
                        case "--all":
                            if (op != Op.None) {
                                throw new ArgumentException("Operations are mutual exclusive");
                            }
                            op = Op.All;
                            break;
                        case "-s":
                        case "--services":
                            i++;
                            if (op != Op.None) {
                                throw new ArgumentException("Operations are mutual exclusive");
                            }
                            op = Op.Browse;
                            if (i < args.Length) {
                                record = DnsServiceRecord.Parse(args[i]);
                            }
                            break;
                        case "-t":
                        case "--timeout":
                            i++;
                            if (i >= args.Length || !int.TryParse(args[i], out period)) {
                                throw new ArgumentException($"Bad -t arg");
                            }
                            break;
                        case "-r":
                        case "--resolve":
                            i++;
                            if (op != Op.None) {
                                throw new ArgumentException("Operations are mutual exclusive");
                            }
                            op = Op.Resolve;
                            if (i < args.Length) {
                                address = ProxySocketAddress.Parse(args[i]);
                            }
                            break;
                        case "-d":
                        case "--dir":
                            i++;
                            if (op != Op.None) {
                                throw new ArgumentException("Operations are mutual exclusive");
                            }
                            op = Op.Dir;
                            if (i < args.Length) {
                                address = ProxySocketAddress.Parse(args[i]);
                            }
                            break;
                        case "--fs":
                            if (op != Op.None) {
                                throw new ArgumentException("Operations are mutual exclusive");
                            }
                            op = Op.Fs;
                            break;
                        case "-n":
                        case "--net":
                            i++;
                            if (op != Op.None) {
                                throw new ArgumentException("Operations are mutual exclusive");
                            }
                            op = Op.Net;
                            if (i < args.Length) {
                                port = ushort.Parse(args[i]);
                            }
                            break;
                        case "-p":
                        case "--port":
                            i++;
                            if (op != Op.None) {
                                throw new ArgumentException("Operations are mutual exclusive");
                            }
                            op = Op.Port;
                            if (i == args.Length) {
                                throw new ArgumentException("-p requires host name.");
                            }
                            host = args[i];
                            break;
                        case "--use-cache":
                            cache = true;
                            break;
                        case "-R":
                        case "--relay":
                            Socket.Provider = Provider.RelayProvider.CreateAsync().Result;
                            break;
                        case "-W":
                        case "--websocket":
                            Provider.WebSocketProvider.Create();
                            break;
                        case "-?":
                        case "-h":
                        case "--help":
                            throw new ArgumentException("Help");
                        default:
                            throw new ArgumentException($"Unknown {args[i]}");
                    }
                }
            }
            catch(Exception e) {
                Console.WriteLine(e.Message);
                Console.WriteLine(
                    @"
Browser - Proxy .net browser sample.
usage:       Browser [options] operation [args]

Options:
     -t
    --timeout
             Timeout in ms to use for each browse request.
    --use-cache
             Return data from cache only (meaning depends on operation).
    --relay
     -R      Use relay provider instead of default provider.
    --websocket
     -W      Use websocket kestrel provider.

    --help
     -?
     -h      Prints out this help.

Operations (Mutually exclusive):
     -s
    --services
             Without further argument browses all domains. Otherwise
             browses for service types or service names in a domain.
             If service name is provided, resolves to host:port and txt.
     -a
    --all    Browse all services in all domains on all proxies and
             resolve each one. (default!)
     -r
    --resolve
             Resolve host to address (getaddrbyhost) or address to host
             (gethostbyaddr) on all proxies.
     -d
    --dir    Browse a folder on any proxy.
    --fs     Browse entire file system on all proxies recursively.

     -p
    --ports
             Scan for open ports on specified host which is provided as
             argument.
     -n
    --net    Scan all proxy networks for available addresses (arp). If
             port is provided only returns machines with specified port 
             open.
"
                    );
                return;
            }

            if (op == Op.Browse) {
                if (record != null) {
                    if (string.IsNullOrEmpty(record.Name)) {
                        var entries = BrowseServicesAsync(
                            record.Type, record.Domain, period, cache).Result;
                    }
                    else {
                        var entries = ResolveServiceAsync(
                            record, period, cache).Result;
                    }
                }
                else {
                    var entries = BrowseDomainsAsync(period).Result;
                    Console.WriteLine($"{entries.Count} entries found!!!");
                }
            }
            else if (op == Op.Resolve) {
                var entries = ResolveHostnameAsync(address, period, cache).Result;
            }
            else if (op == Op.Dir) {
                var files = BrowseFilesAsync(null, address?.Host, period, cache).Result;
                Console.WriteLine($"{files.Count} files/folders found!!!");
            }
            else if (op == Op.Fs) {
                BrowseFilesRecursiveAsync(null, null, period, cache).Wait();
            }
            else if (op == Op.Net) {
                var addresses = BrowseNetworkAsync(null, port, period, cache).Result;
                Console.WriteLine($"{addresses.Count} addresses found!!!");
            }
            else if (op == Op.Port) {
                var ports = BrowsePortsAsync(null, host, period, cache).Result;
                Console.WriteLine($"{ports.Count} ports open!!!");
            }
            else if (op == Op.All || op == Op.None) {
                Console.WriteLine("Browse and resolve all services");
                var entries = ResolveServiceNamesAsync(period).Result;
                Console.WriteLine($"{entries.Count} entries resolved!!!");
            }

            Console.WriteLine("Press a key to exit...");
            Console.ReadKey();
        }

        /// <summary>
        /// Browse folders
        /// </summary>
        static async Task<List<FileEntry>> BrowseFilesAsync(SocketAddress proxy, string folder,
            int period, bool cache) {
            Console.WriteLine($"Listing {folder??"<root>"} ...");
            var cts = new CancellationTokenSource(period);
            var files = new List<FileEntry>();
            try {
                using (var browser = await BrowseClient.CreateDirectoryBrowserAsync(
                    proxy, folder, cache, CancellationToken.None)) {
                    cts.Token.ThrowIfCancellationRequested();
                    while (true) {
                        try {
                            cts.Token.ThrowIfCancellationRequested();
                            if (!await browser.MoveNextAsync(cts.Token))
                                break;
                            Console.WriteLine($"{DateTime.Now}: File {browser.Current} on {browser.Current.Interface} ");
                            files.Add(browser.Current);
                        }
                        catch(BrowseException e) {
                            Console.WriteLine($"Browse error {e.Message}");
                        }
                    }
                }
            }
            catch (OperationCanceledException) { }
            return files;
        }

        /// <summary>
        /// Browse files in a particular folder recursively
        /// </summary>
        static async Task BrowseFilesRecursiveAsync(SocketAddress proxy, string folder,
            int period, bool cache) {
            var files = await BrowseFilesAsync(proxy, folder, period, cache);
            foreach (var result in files) {
                if (result.Info.Type == (int)FileType.Directory) {
                    await BrowseFilesRecursiveAsync(result.Interface, result.FileName, period, cache);
                }
            }
        }

        /// <summary>
        /// Browse ports
        /// </summary>
        /// <param name="proxy"></param>
        /// <param name="host"></param>
        /// <param name="period"></param>
        /// <param name="cache"></param>
        /// <returns></returns>
        static async Task<List<ushort>> BrowsePortsAsync(SocketAddress proxy, string host,
            int period, bool cache) {
            Console.WriteLine($"Scanning {host} ...");
            var cts = new CancellationTokenSource(period);
            var addresses = new List<ushort>();
            try {
                using (var browser = await BrowseClient.CreatePortScannerAsync(
                    proxy, new ProxySocketAddress(host), cache, CancellationToken.None)) {
                    cts.Token.ThrowIfCancellationRequested();
                    while (true) {
                        try {
                            cts.Token.ThrowIfCancellationRequested();
                            if (!await browser.MoveNextAsync(cts.Token))
                                break;
                            Console.WriteLine($"{DateTime.Now}: Found {browser.Current} ");
                            addresses.Add((browser.Current.Result as InetSocketAddress).Port);
                        }
                        catch (BrowseException e) {
                            Console.WriteLine($"Browse error {e.Message}");
                        }
                    }
                }
            }
            catch (OperationCanceledException) { }
            return addresses;
        }

        /// <summary>
        /// Browse network
        /// </summary>
        /// <param name="proxy"></param>
        /// <param name="port"></param>
        /// <param name="period"></param>
        /// <param name="cache"></param>
        /// <returns></returns>
        static async Task<List<NetworkScanResult>> BrowseNetworkAsync(SocketAddress proxy, ushort port,
            int period, bool cache) {
            Console.WriteLine($"Browsing network ...");
            var cts = new CancellationTokenSource(period);
            var results = new List<NetworkScanResult>();
            try {
                using (var browser = await BrowseClient.CreateNetworkScannerAsync(
                    proxy, port, cache, CancellationToken.None)) {
                    cts.Token.ThrowIfCancellationRequested();
                    while (true) {
                        try {
                            cts.Token.ThrowIfCancellationRequested();
                            if (!await browser.MoveNextAsync(cts.Token))
                                break;
                            Console.WriteLine($"{DateTime.Now}: Found {browser.Current} ");
                            results.Add(browser.Current);
                        }
                        catch (BrowseException e) {
                            Console.WriteLine($"Browse error {e.Message}");
                        }
                    }
                }
            }
            catch (OperationCanceledException) { }
            return results;
        }

        /// <summary>
        /// Browse domains
        /// </summary>
        static async Task<List<string>> BrowseDomainsAsync(int period) {
            Console.WriteLine($"Browsing for domains for {period} ms...");
            var cts = new CancellationTokenSource(period);
            var domains = new List<string>();
            try {
                using (var browser = await Dns.BrowseAsync(cts.Token)) {
                    cts.Token.ThrowIfCancellationRequested();
                    while (await browser.MoveNextAsync(cts.Token)) {
                        cts.Token.ThrowIfCancellationRequested();
                        Console.WriteLine($"{DateTime.Now}: Domain {browser.Current.Domain} on {browser.Current.Interface} " +
                            (browser.Current.Removed ? "removed" : "found"));
                        if (browser.Current.Removed)
                            domains.Remove(browser.Current.Domain);
                        else
                            domains.Add(browser.Current.Domain);
                    }
                }
            }
            catch (OperationCanceledException) { }
            return domains;
        }

        /// <summary>
        /// Browse service names or types
        /// </summary>
        static async Task<IEnumerable<DnsServiceRecord>> BrowseServicesAsync(string type, string domain,
            int period, bool fromCache) {
            if (type != null) {
                Console.WriteLine($"Browsing for service names for type {type} in {domain ?? "local."} for {period} ms...");
            }
            else {
                Console.WriteLine($"Browsing for service types in {domain ?? "local."} for {period} ms...");
            }
            var records = new HashSet<DnsServiceRecord>();
            var cts = new CancellationTokenSource(period);
            try {
                using (var browser = await Dns.BrowseAsync(type, domain, fromCache, cts.Token)) {
                    cts.Token.ThrowIfCancellationRequested();
                    while (await browser.MoveNextAsync(cts.Token)) {
                        cts.Token.ThrowIfCancellationRequested();
                        Console.WriteLine($"{DateTime.Now}:    {browser.Current} " +
                            (browser.Current.Removed ? "removed" : "found"));
                        if (browser.Current.Removed)
                            records.Remove(browser.Current);
                        else
                            records.Add(browser.Current);
                    }
                }
            }
            catch (OperationCanceledException) { }
            return records;
        }

        /// <summary>
        /// Resolve service names
        /// </summary>
        static async Task<IEnumerable<DnsServiceEntry>> ResolveServiceAsync(DnsServiceRecord record,
            int period, bool fromCache) {
            Console.WriteLine($"Resolving {record} for {period} ms...");
            var entries = new HashSet<DnsServiceEntry>();
            var cts = new CancellationTokenSource(period);
            try {
                using (var browser = await Dns.BrowseAsync(record, fromCache, cts.Token)) {
                    cts.Token.ThrowIfCancellationRequested();
                    while (await browser.MoveNextAsync(cts.Token)) {
                        cts.Token.ThrowIfCancellationRequested();
                        Console.WriteLine($"{DateTime.Now}: {browser.Current}");
                        entries.Add(browser.Current);
                    }
                }
            }
            catch (OperationCanceledException) { }
            return entries;
        }

        /// <summary>
        /// Resolve host name
        /// </summary>
        static async Task<IEnumerable<DnsHostEntry>> ResolveHostnameAsync(SocketAddress addr,
            int period, bool cache) {
            Console.WriteLine($"Resolving {addr} for {period} ms...");
            var entries = new HashSet<DnsHostEntry>();
            var cts = new CancellationTokenSource(period);
            try {
                using (var resolver = await Dns.ResolveAsync(addr, cache, cts.Token)) {
                    cts.Token.ThrowIfCancellationRequested();
                    while (await resolver.MoveNextAsync(cts.Token)) {
                        cts.Token.ThrowIfCancellationRequested();
                        Console.WriteLine($"{DateTime.Now}: {resolver.Current}");
                        entries.Add(resolver.Current);
                    }
                }
            }
            catch (OperationCanceledException) { }
            return entries;
        }

        /// <summary>
        /// When all domains were browsed for period milliseconds,
        /// browse all service types in all found domains
        /// </summary>
        static async Task<HashSet<DnsServiceRecord>> BrowseServiceTypesAsync(int period) {
            var records = new HashSet<DnsServiceRecord>();
            var results = await Task.WhenAll(BrowseDomainsAsync(period).Result.Select(
                d => {
                    return BrowseServicesAsync(null, d, period, true);
                }).ToArray());
            foreach(var result in results) {
                records.AddRange(result);
            }
            return records;
        }

        /// <summary>
        /// When all service types were browsed in all domains for period milliseconds,
        /// browse all service records
        /// </summary>
        static async Task<HashSet<DnsServiceRecord>> BrowseServiceNamesAsync(int period) {
            var records = new HashSet<DnsServiceRecord>();
            var results = await Task.WhenAll(BrowseServiceTypesAsync(period).Result.Select(
                s => {
                    return BrowseServicesAsync(s.Type, s.Domain, period, true);
                }).ToArray());
            foreach (var result in results) {
                records.AddRange(result);
            }
            return records;
        }

        /// <summary>
        /// When all service records were browsed for period milliseconds, resolve them
        /// </summary>
        static async Task<ISet<DnsHostEntry>> ResolveServiceNamesAsync(int period) {
            var results = await Task.WhenAll(BrowseServiceNamesAsync(period).Result.Select(
                s => {
                    return ResolveServiceAsync(s, period, true);
                }).ToArray());

            var entries = new HashSet<DnsHostEntry>();
            foreach (var result in results) {
                var results2 = await Task.WhenAll(result.Select(
                    r => {
                        return ResolveHostnameAsync(r.Address, period, true);
                    }));
                foreach (var hosts in results2) {
                    entries.AddRange(hosts);
                }
            }
            return entries;
        }
    }
}
