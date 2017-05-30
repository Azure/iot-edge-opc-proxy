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
            None, Browse, Resolve, Dir, Fs
        }

        static void Main(string[] args) {
            Op op = Op.None;
            bool cache = false;
            int period = 60 * 1000;
            DnsServiceRecord record = null;
            ProxySocketAddress address = null;
            // Parse command line
            try {
                for (int i = 0; i < args.Length; i++) {
                        switch (args[i]) {
                        case "-s":
                        case "--services":
                            i++;
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
                            op = Op.Resolve;
                            if (i < args.Length) {
                                address = ProxySocketAddress.Parse(args[i]);
                            }
                            break;
                        case "-d":
                        case "--dir":
                            i++;
                            op = Op.Dir;
                            if (i < args.Length) {
                                address = ProxySocketAddress.Parse(args[i]);
                            }
                            break;
                        case "--fs":
                            op = Op.Fs;
                            break;
                        case "--use-cache":
                            cache = true;
                            break;
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
-s --services   Without further argument browses all domains.  Otherwise
                browses for service types or service names in a domain.
                If service name is provided, resolves to host:port and txt.
-t --timeout    Timeout in ms to use for each browse step.
-r --resolve    Resolve host to address (getaddrbyhost) or address to host 
                (gethostbyaddr)
-d --dir        Browse a folder.
   --fs         Browse entire file system.
   --use-cache  Return data from cache.
-h --help       Help.
"
                    );
                return;
            }

            if (cache) {
                Console.WriteLine("Cache not supported");
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
                var entries = ResolveAddressAsync(address, period, cache).Result;
            }
            else if (op == Op.Dir) {
                var files = BrowseFilesAsync(null, address?.Host, period, cache).Result;
                Console.WriteLine($"{files.Count} files/folders found!!!");
            }
            else if (op == Op.Fs) {
                BrowseFilesRecursiveAsync(null, null, period, cache).Wait();
            }
            else {
                Console.WriteLine("Browse and resolve all services");
                var entries = ResolveServiceNamesAsync(period).Result;
                Console.WriteLine($"{entries.Count} entries resolved!!!");
            }
        }

        /// <summary>
        /// Browse folders
        /// </summary>
        /// <param name="period"></param>
        /// <returns></returns>
        static async Task<List<Model.FileEntry>> BrowseFilesAsync(SocketAddress proxy, string folder,
            int period, bool cache) {
            Console.WriteLine($"Listing {folder??"<root>"} ...");
            var cts = new CancellationTokenSource(period);
            var files = new List<Model.FileEntry>();
            try {
                using (var browser = await Model.BrowseClient.CreateDirectoryBrowserAsync(
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
                        catch(Model.BrowseException e) {
                            Console.WriteLine($"Browse error {e.Message}");
                        }
                    }
                }
            }
            catch (OperationCanceledException) { }
            return files;
        }

        /// <summary>
        /// When all service types were browsed in all domains for period milliseconds, 
        /// browse all service records 
        /// </summary>
        /// <param name="period"></param>
        /// <returns></returns>
        static async Task BrowseFilesRecursiveAsync(SocketAddress proxy, string folder,
            int period, bool cache) {
            var files = await BrowseFilesAsync(proxy, folder, period, cache);
            foreach (var result in files) {
                if (result.Info.Type == (int)Model.FileType.Directory) {
                    await BrowseFilesRecursiveAsync(result.Interface, result.FileName, period, cache);
                }
            }
        }

        /// <summary>
        /// Browse domains
        /// </summary>
        /// <param name="period"></param>
        /// <returns></returns>
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
        /// <param name="type"></param>
        /// <param name="domain"></param>
        /// <param name="period"></param>
        /// <returns></returns>
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
        /// Browse service names or types
        /// </summary>
        /// <param name="type"></param>
        /// <param name="domain"></param>
        /// <param name="period"></param>
        /// <returns></returns>
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
        /// Resolve address
        /// </summary>
        /// <param name="addr"></param>
        /// <param name="period"></param>
        /// <returns></returns>
        static async Task<IEnumerable<DnsHostEntry>> ResolveAddressAsync(ProxySocketAddress addr, 
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
        /// <param name="period"></param>
        /// <returns></returns>
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
        /// <param name="period"></param>
        /// <returns></returns>
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
        /// When all service types were browsed in all domains for period milliseconds, 
        /// browse all service records 
        /// </summary>
        /// <param name="period"></param>
        /// <returns></returns>
        static async Task<HashSet<DnsServiceEntry>> ResolveServiceNamesAsync(int period) {
            var entries = new HashSet<DnsServiceEntry>();
            var results = await Task.WhenAll(BrowseServiceNamesAsync(period).Result.Select(
                s => {
                    return ResolveServiceAsync(s, period, true);
                }).ToArray());
            foreach (var result in results) {
                entries.AddRange(result);
            }
            return entries;
        }
    }
}
