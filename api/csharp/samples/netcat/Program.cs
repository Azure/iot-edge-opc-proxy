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

    class Program {

        /// <summary>
        /// Print help
        /// </summary>
        static void PrintHelp() {
            Console.WriteLine(
                @"
PNetCat - Proxy .net Netcat.  
usage:       
             PNetCat [options] host (port1 [...portN] | portlo-porthi)

             Tries to connect to a port on the named host and sends data from
             stdin to the port and and receives back to stdout.  If more than
             one port is provided all ports are tried, until one connects 
             successfully. Alternatively to specifying each port individually 
             a range of ports can be provided as final argument.

options:

     -d      Do not attempt to read from stdin.

    --help
     -?
     -h      Prints out help.

    --close
     -c      Close and exit when EOF received on stdin.

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
                                    optc = optl[0];
                                    break;
                                case "version":
                                case "relay":
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
                            default:
                                throw new ArgumentException($"Unknown option {opt}.");
                        }

                        continue;
                    }
                    else if (string.IsNullOrEmpty(prog.Host)) {
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
        /// Validate all properties
        /// </summary>
        public void Validate() {
            if (string.IsNullOrEmpty(Host)) {
                throw new ArgumentException($"Missing host argument.");
            }
            if (Ports.Count == 0) {
                throw new ArgumentException($"Missing port argument(s).");
            }
        }

        /// <summary>
        /// Cats from std in to network stream and back out to std out.
        /// </summary>
        public async Task RunAsync(CancellationToken ct) {

            Validate();

            if (UseRelay) {
                Socket.Provider = await Provider.RelayProvider.CreateAsync();
            }

            foreach (int port in Ports) {
                var client = new TcpClient();
                if (Timeout.HasValue) {
                    client.Socket.ConnectTimeout = TimeSpan.FromMilliseconds(Timeout.Value);
                    client.Socket.ReceiveTimeout = Timeout.Value;
                    client.Socket.SendTimeout = Timeout.Value;
                }
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
                        tasks.Add(StdInReader(stream, cts.Token));
                    }
                    tasks.Add(StdOutWriter(stream, cts.Token));

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
                catch {
                    Console.Error.WriteLine($"... FAILED!");
                }
                finally {
                    client.Dispose();
                }
            }
        }

        /// <summary>
        /// Reads from stdin and copies to network stream.
        /// </summary>
        /// <param name="stream"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        private async Task StdInReader(NetworkStream stream, CancellationToken ct) {
            do {
                Stream stdin = Console.OpenStandardInput();
                try {
                    await stdin.CopyToAsync(stream, 0x10000, ct);
                }
                catch (OperationCanceledException) {
                    break;
                }
                catch (Exception ex) {
                    if (ct.IsCancellationRequested) {
                        break;
                    }

                    Console.Error.WriteLine($"Exception occurred on stdin {ex}.");
                    // Continue
                }
                finally {
                    stdin.Dispose();
                    stdin = null;
                }
            }
            while (!CloseOnEof && !ct.IsCancellationRequested);
            Console.Error.WriteLine("... stdin closed");
        }

        /// <summary>
        /// Reads from stdin and copies to network stream.
        /// </summary>
        /// <param name="stream"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        private async Task StdOutWriter(NetworkStream stream, CancellationToken ct) {
            try {
                using (Stream stdout = Console.OpenStandardOutput()) {
                    await stream.CopyToAsync(stdout, 0x10000, ct);
                }
            }
            catch (OperationCanceledException) {
                Console.Error.WriteLine();
            }
            catch (Exception ex) {
                if (ex.GetSocketError() == SocketError.Closed) {
                    Console.Error.Write($"Remote side closed ");
                }
                else if (!ct.IsCancellationRequested) {
                    Console.Error.Write($"Exception occurred on stdout {ex} ");
                }
            }
            Console.Error.WriteLine("... stdout closed");
        }
    }
}
