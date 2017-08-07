// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Samples {
    using System;
    using System.IO;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Collections.Generic;
    using Microsoft.Azure.Devices.Proxy;
    using System.Linq;

    class Program {

        /// <summary>
        /// Print help
        /// </summary>
        static void PrintHelp() {
            Console.WriteLine(
                @"
PNetCat - Proxy .net Netcat.
usage:
             PNetCat [options] remote-host (port1 [...portN] | portlo-porthi)

             In client mode, tries to connect to a port on the named host and
             sends data from stdin to the port and and receives back to stdout.

             In client mode, if more than one port is provided all ports are
             tried, until one connects successfully. Alternatively to
             specifying each port individually a range of ports can be provided
             as final argument. In server mode, exactly 1 port must be provided.

             PNetCat [options] --local local-port remote-host remote-port

             In server mode (--local option) accepts a connection on the
             specified port and bridges this connection to the specified host
             and port creating a port bridge.

options:
    --help
     -?
     -h      Prints this help.

    --local
     -l port
             Enables pnetcat server mode rather than client mode. Waits for 
             connection on local port rather than opening stdin/stdout.

     -d      If in client mode, do not attempt to read from stdin.

    --close
     -c      When in client mode, close and exit when EOF received on stdin.

    --proxy
     -x proxy_address
    --source
     -s source_address
             Specifies the proxy address which is used to send the packets.

    --wait
     -w timeout
             If a connection and stdin are idle for more than timeout seconds,
             then the connection is silently closed. The default is no timeout.

    --relay
     -R      Use relay provider instead of default provider.
    --websocket
     -W      Use websocket kestrel provider.
               ");
        }

        /// <summary>
        /// Main entry point
        /// </summary>
        static void Main(string[] args) {

            var prog = new Program();
            try {
                for (int index = 0; index < args.Length; index++) {
                    int tmp;
                    string opt = args[index];
                    if (string.IsNullOrEmpty(args[index])) {
                        continue;
                    }
                    if (opt[0] == '-' || opt[0] == '/') {
                        if (opt.Length == 1) {
                            throw new ArgumentException($"Unknown option {opt}.");
                        }
                        char optc = opt[1];
                        if (optc == '-' && opt[0] != '/') {
                            // Parse -- long option
                            string optl = opt.Substring(2);
                            switch (optl) {
                                case "help":
                                case "source":
                                case "close":
                                case "wait":
                                case "local":
                                    optc = optl[0];
                                    break;
                                case "version":
                                case "relay":
                                case "websocket":
                                    optc = char.ToUpperInvariant(optl[0]);
                                    break;
                                case "proxy":
                                    optc = 'x';
                                    break;
                                default:
                                    throw new ArgumentException($"Unknown option {opt}.");
                            }
                        }
                        switch (optc) {
                            case 'd':
                                prog.NoStdIn = true;
                                break;
                            case 'c':
                                prog.CloseOnEof = true;
                                break;
                            case 'R':
                                prog.UseRelay = true;
                                break;
                            case 'W':
                                prog.UseWS = true;
                                break;
                            case '?':
                            case 'h':
                                PrintHelp();
                                return;
                            case 'x':
                            case 's':
                                index++;
                                SocketAddress proxy;
                                if (index >= args.Length ||
                                    !SocketAddress.TryParse(args[index], out proxy)) {
                                    throw new ArgumentException($"Bad value for {opt}.");
                                }
                                prog.Proxy = proxy;
                                break;
                            case 'w':
                                index++;
                                if (index >= args.Length ||
                                    !int.TryParse(args[index], out tmp)) {
                                    throw new ArgumentException($"Bad value for {opt}.");
                                }
                                prog.Timeout = tmp * 1000;
                                break;
                            case 'l':
                                index++;
                                if (index >= args.Length ||
                                    !int.TryParse(args[index], out tmp)) {
                                    throw new ArgumentException($"Bad value for {opt}.");
                                }
                                prog.LocalPort = (ushort)tmp;
                                break;
                            default:
                                throw new ArgumentException($"Unknown option {opt}.");
                        }

                        continue;
                    }

                    if (string.IsNullOrEmpty(prog.Host)) {
                        prog.Host = opt;
                    }
                    else {
                        var ports = opt.Split('-');
                        if (!int.TryParse(ports[0], out tmp)) {
                            throw new ArgumentException($"Invalid port value {opt}.");
                        }
                        prog.Ports.Add(tmp);
                        if (ports.Length == 2) {
                            int hi;
                            if (!int.TryParse(ports[1], out hi) || hi > ushort.MaxValue) {
                                throw new ArgumentException($"Invalid upper bound for port range: {opt}.");
                            }
                            while (++tmp <= hi) {
                                prog.Ports.Add(tmp);
                            }
                        }
                        else if (ports.Length > 2) {
                            throw new ArgumentException($"Invalid port range: {opt}.");
                        }
                    }
                }
                prog.Validate();
            }
            catch (ArgumentException e) {
                Console.Error.WriteLine($"Error: {e.Message}");
                PrintHelp();
                return;
            }

            // Register console cancellation
            var cts = new CancellationTokenSource();
            Console.CancelKeyPress += (o, e) => {
                Console.Error.WriteLine();
                Console.Error.WriteLine("Ctrl+C => Cancellation requested...");
                cts.Cancel();
                e.Cancel = true; // Wait for cts to kill readers and exit - do not exit immediately.
            };
            prog.RunAsync(cts.Token).Wait();
        }

        /// <summary>
        /// Host name
        /// </summary>
        internal string Host { get; set; }

        /// <summary>
        /// List of ports to try
        /// </summary>
        internal List<int> Ports { get; set; } = new List<int>();

        /// <summary>
        /// Whether to connect std in
        /// </summary>
        internal bool NoStdIn { get; set; }

        /// <summary>
        /// timeout for send/recv/connect
        /// </summary>
        internal int? Timeout { get; set; }

        /// <summary>
        /// Whether to close when eof received
        /// </summary>
        internal bool CloseOnEof { get; set; }

        /// <summary>
        /// Use relay provider
        /// </summary>
        internal bool UseRelay { get; set; }

        /// <summary>
        /// Proxy to connect through
        /// </summary>
        internal SocketAddress Proxy { get; set; }

        /// <summary>
        /// Use webprovider
        /// </summary>
        internal bool UseWS { get; set; }

        /// <summary>
        /// Port to bind to
        /// </summary>
        internal ushort LocalPort { get; set; }

        /// <summary>
        /// Validate all properties
        /// </summary>
        public void Validate() {
            if (string.IsNullOrEmpty(Host)) {
                throw new ArgumentException("Missing host argument.");
            }
            if (Ports.Count == 0) {
                throw new ArgumentException("Missing port argument(s).");
            }
        }

        /// <summary>
        /// Cats from std in to network stream and back out to std out.
        /// </summary>
        public async Task RunAsync(CancellationToken ct) {

            if (UseRelay) {
                Socket.Provider = await Provider.RelayProvider.CreateAsync();
            }
            else if (UseWS) {
                Provider.WebSocketProvider.Create();
            }

            if (LocalPort == 0) {
                await StdAsync(ct);
            }
            else {
                await TcpAsync(ct);
            }
        }

        /// <summary>
        /// Cats from std in to network stream and back out to std out.
        /// </summary>
        public async Task StdAsync(CancellationToken ct) {

            foreach (int port in Ports) {
                using (var client = new TcpClient()) {
                    if (Timeout.HasValue) {
                        client.Socket.ConnectTimeout = TimeSpan.FromMilliseconds(Timeout.Value);
                        client.Socket.ReceiveTimeout = Timeout.Value;
                        client.Socket.SendTimeout = Timeout.Value;
                    }

                    Stream input = null;
                    Stream output = null;
                    try {
                        Console.Error.Write($"Connecting to {Host}:{port} ");
                        await client.ConnectAsync(Host, port);
                        Console.Error.WriteLine($"... connected!");
                        var stream = client.GetStream();

                        // Create a local cancellation token so we can kill all tasks...
                        var cts = new CancellationTokenSource();
                        ct.Register(() => {
                            cts.Cancel();
                            client.Socket.Close();
                        });

                        // Add reader/writer tasks to our task list
                        var tasks = new List<Task>();
                        if (!NoStdIn) {
                            input = Console.OpenStandardInput();
                            tasks.Add(OutPump(input, stream, cts.Token));
                        }
                        output = Console.OpenStandardInput();
                        tasks.Add(InPump(stream, output, cts.Token));

                        // Wait for any of the tasks to complete, then cancel the others
                        await Task.WhenAny(tasks.ToArray());

                        cts.Cancel();
                        foreach (Task t in tasks) {
                            if (!t.IsCompleted) {
                                await t;
                            }
                        }
                        Console.Error.WriteLine("... Disconnected!");
                        return;
                    }
                    catch (Exception e) {
                        Console.Error.WriteLine($"... FAILED ({e.Message})!");
                    }
                    finally {
                        input?.Dispose();
                        output?.Dispose();
                    }
                }
            }
        }

        /// <summary>
        /// Opens port and tunnels stream
        /// </summary>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async Task TcpAsync(CancellationToken ct) {

            if (Ports.Count > 1) {
                throw new ArgumentException(
                    "Only one port argument allowed when using -l option!");
            }

            CloseOnEof = true;
            using (var client = new TcpClient()) {
                if (Timeout.HasValue) {
                    client.Socket.ConnectTimeout = TimeSpan.FromMilliseconds(Timeout.Value);
                    client.Socket.ReceiveTimeout = Timeout.Value;
                    client.Socket.SendTimeout = Timeout.Value;
                }
                Console.Error.Write($"Waiting on port {LocalPort} ");
                var listener = new System.Net.Sockets.TcpListener(System.Net.IPAddress.Any, LocalPort);
                do {
                    Stream networkStream = null;
                    try {
                        listener.Start();
                        var c = await listener.AcceptTcpClientAsync();
                        listener.Stop();

                        // Got stream - now connect through proxy
                        networkStream = c.GetStream();
                        Console.Error.Write($"Connecting to {Host}:{Ports.First()} ");
                        await client.ConnectAsync(Host, Ports.First()).ConfigureAwait(false);
                        Console.Error.WriteLine($"... connected!");
                        var stream = client.GetStream();

                        // Create a local cancellation token so we can kill both tasks...
                        var cts = new CancellationTokenSource();
                        ct.Register(() => {
                            cts.Cancel();
                            client.Socket.Close();
                        });

                        // Now pump
                        await Task.WhenAny(new Task[] {
                            OutPump(networkStream, stream, cts.Token),
                            InPump(stream, networkStream, cts.Token)
                        });
                        cts.Cancel();
                    }
                    catch (Exception e) {
                        Console.Error.WriteLine($"... FAILED ({e.Message})!");
                    }
                    finally {
                        networkStream?.Dispose();
                        networkStream = null;
                    }
                }
                while (!ct.IsCancellationRequested);
            }
        }

        /// <summary>
        /// Reads from input and copies to proxy network stream.
        /// </summary>
        /// <param name="input"></param>
        /// <param name="output"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        private async Task OutPump(Stream input, NetworkStream output, CancellationToken ct) {
            do {
                try {
                    await input.CopyToAsync(output, 0x10000, ct);
                }
                catch (OperationCanceledException) {
                    break;
                }
                catch (Exception ex) {
                    if (!ct.IsCancellationRequested) {
                        Console.Error.WriteLine($"Input error: {ex.Message}.");
                    }
                }
            }
            while (!CloseOnEof && !ct.IsCancellationRequested);
            Console.Error.WriteLine("... input closed");
        }

        /// <summary>
        /// Reads from proxy network stream and copies to output.
        /// </summary>
        /// <param name="input"></param>
        /// <param name="output"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        private async Task InPump(NetworkStream input, Stream output, CancellationToken ct) {
            try {
                await input.CopyToAsync(output, 0x10000, ct);
            }
            catch (OperationCanceledException) {
                Console.Error.WriteLine();
            }
            catch (Exception ex) {
                if (ex.GetSocketError() == SocketError.Closed) {
                    Console.Error.Write($"Remote side closed ");
                }
                else if (!ct.IsCancellationRequested) {
                    Console.Error.Write($"Output error: {ex.Message} ");
                }
            }
            Console.Error.WriteLine("... output closed");
        }
    }
}
