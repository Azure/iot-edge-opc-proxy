// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Samples {
    using System;
    using System.Collections.Generic;
    using System.Text;
    using System.Threading.Tasks;
    using Microsoft.Azure.Devices.Proxy;
    using System.Threading;
    using System.Diagnostics;
    using System.IO;

    class Program {

        static string host_name = System.Net.Dns.GetHostName();
        static Random rand = new Random();

        enum Op {
            None, Perf, Sync, Async, All
        }

        ///
        /// <summary>
        ///
        /// Simple TCP/IP Services
        ///
        /// Port 7:  Echo http://tools.ietf.org/html/rfc862
        /// Port 19: Chargen http://tools.ietf.org/html/rfc864
        /// Port 13: daytime http://tools.ietf.org/html/rfc867
        /// Port 17: quotd http://tools.ietf.org/html/rfc865
        ///
        /// </summary>
        static void Main(string[] args) {
            Op op = Op.None;
            bool bypass = false;
            int index = 0;
            int timeout = 10;
            int bufferSize = 60000;

            // Parse command line
            try {
                for (int i = 0; i < args.Length; i++) {
                    switch (args[i]) {
                        case "--all":
                            if (op != Op.None) {
                                throw new ArgumentException("Operations are mutual exclusive");
                            }
                            op = Op.All;
                            break;
                        case "-s":
                        case "--sync":
                            if (op != Op.None) {
                                throw new ArgumentException("Operations are mutual exclusive");
                            }
                            op = Op.Sync;
                            break;
                        case "-a":
                        case "--async":
                            if (op != Op.None) {
                                throw new ArgumentException("Operations are mutual exclusive");
                            }
                            op = Op.Async;
                            break;
                        case "-p":
                        case "--perf":
                            if (op != Op.None) {
                                throw new ArgumentException("Operations are mutual exclusive");
                            }
                            op = Op.Perf;
                            break;
                        case "-t":
                        case "--timeout":
                            i++;
                            if (i >= args.Length || !int.TryParse(args[i], out timeout)) {
                                throw new ArgumentException($"Bad -t arg");
                            }
                            break;
                        case "-b":
                        case "--start-at":
                            i++;
                            if (i >= args.Length || !int.TryParse(args[i], out index)) {
                                throw new ArgumentException($"Bad -b arg");
                            }
                            break;
                        case "-z":
                        case "--buffer-size":
                            i++;
                            if (i >= args.Length || !int.TryParse(args[i], out bufferSize)) {
                                throw new ArgumentException($"Bad -z arg");
                            }
                            break;
                        case "--bypass":
                            bypass = true;
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
            catch (Exception e) {
                Console.WriteLine(e.Message);
                Console.WriteLine(
                    @"
Simple - Proxy .net simple tcp/ip sample.  
usage:       Simple [options] operation [args]

Options:
    --relay
     -R      Use relay provider instead of default provider.
    --websocket
     -W      Use websocket kestrel provider.
    --buffer-size
     -z      Specifies the buffer size for performance tests.
             Defaults to 60000 bytes.
    --start-at
     -b      Begins the test loop at this index. Defaults to 0.
    --timeout
     -t      Async tests timeout in minutes. Defaults to 10.

    --help
     -?
     -h      Prints out this help.

Operations (Mutually exclusive):
    --all    Do async and sync tests (default).
     -s 
    --sync   
             Run sync tests
     -a
    --async  Run async tests
     -p 
    --perf 
             Run performance tests.
"
                    );
                return;
            }

            if (op == Op.None) {
                op = Op.All;
            }

            if (op == Op.Perf) {
                try {
                    if (bypass) {
                        PerfLoopComparedAsync(bufferSize).Wait();
                    }
                    else {
                        PerfLoopAsync(bufferSize).Wait();
                    }
                }
                catch (Exception e) {
                    Console.Out.WriteLine($"{e.Message}");
                    Thread.Sleep(4000);
                }
            }
            else {
                for (int j = index + 1; ; j++) {
                    if (op == Op.Sync || op == Op.All) {
                        Console.Clear();
                        Console.Out.WriteLine($"#{j} Sync tests...");
                        try {
                            SendReceive(7, Encoding.UTF8.GetBytes("Simple test to echo server"));
                            Receive(19);
                            Receive(13);
                            Receive(17);
                            Receive(19);
                            EchoLoop(j + 1);
                        }
                        catch (Exception e) {
                            Console.Out.WriteLine($"{e.Message}");
                            Thread.Sleep(4000);
                        }
                    }
                    if (op == Op.Async || op == Op.All) {
                        Console.Clear();
                        Console.Out.WriteLine($"#{j} Async tests...");
                        var tasks = new List<Task>();
                        try {
                            for (int i = 0; i < j + 1; i++) {
                                tasks.Add(ReceiveAsync(i, 19));
                                tasks.Add(ReceiveAsync(i, 13));
                                tasks.Add(ReceiveAsync(i, 17));
                                tasks.Add(EchoLoopAsync(i, 5));
                            }

                            Task.WaitAll(tasks.ToArray(),
                                new CancellationTokenSource(TimeSpan.FromMinutes(timeout)).Token);
                            Console.Out.WriteLine($"#{j} ... complete!");
                        }
                        catch (OperationCanceledException) {
                            foreach (var pending in tasks) {
                                Console.Out.WriteLine(
                                    $"{pending}  did not complete after {timeout} minutes.");
                            }
                            Thread.Sleep(4000);
                        }
                        catch (Exception e) {
                            Console.Out.WriteLine($"{e.Message}");
                            Thread.Sleep(4000);
                        }
                    }
                }
            }

            Console.WriteLine("Press a key to exit...");
            Console.ReadKey();
        }

        public static void Receive(int port) {
            byte[] buffer = new byte[1024];
            using (Socket s = new Socket(SocketType.Stream, ProtocolType.Tcp)) {
                s.Connect(host_name, port);
                Console.Out.WriteLine($"Receive: Connected to {s.RemoteEndPoint} on {s.InterfaceEndPoint} via {s.LocalEndPoint}!");
                Console.Out.WriteLine("Receive: Receiving sync...");
                int count =  s.Receive(buffer);
                Console.Out.WriteLine(Encoding.UTF8.GetString(buffer, 0, count));
                s.Close();
            }
        }

        public static void EchoLoop(int loops) {
            using (Socket s = new Socket(SocketType.Stream, ProtocolType.Tcp)) {
                s.Connect(host_name, 7);
                Console.Out.WriteLine($"EchoLoop: Connected to {s.RemoteEndPoint} on {s.InterfaceEndPoint} via {s.LocalEndPoint}!");

                for (int i = 0; i < loops; i++) {
                    s.Send(Encoding.UTF8.GetBytes($"EchoLoop: {i} sync loop to echo server"));
                    byte[] buffer = new byte[1024];
                    Console.Out.WriteLine($"EchoLoop: {i}:        Receiving sync...");
                    int count = s.Receive(buffer);
                    Console.Out.WriteLine(Encoding.UTF8.GetString(buffer, 0, count));
                }
                s.Close();
            }
        }

        public static void Send(int port, byte[] buffer, int iterations) {
            using (Socket s = new Socket(SocketType.Stream, ProtocolType.Tcp)) {
                s.Connect(host_name, port);
                Console.Out.WriteLine($"Send: Connected to {s.RemoteEndPoint} on {s.InterfaceEndPoint} via {s.LocalEndPoint}!");
                Console.Out.WriteLine("Send: Sending sync ...");
                for (int i = 0; i < iterations; i++)
                    s.Send(buffer);
                s.Close();
            }
        }

        public static void SendReceive(int port, byte[] buffer) {
            using (Socket s = new Socket(SocketType.Stream, ProtocolType.Tcp)) {
                s.Connect(host_name, port);
                Console.Out.WriteLine($"SendReceive: Connected to {s.RemoteEndPoint} on {s.InterfaceEndPoint} via {s.LocalEndPoint}!");
                Console.Out.WriteLine("SendReceive: Sending sync ...");
                s.Send(buffer);
                buffer = new byte[1024];
                Console.Out.WriteLine("SendReceive: Receiving sync...");
                int count = s.Receive(buffer);
                Console.Out.WriteLine(Encoding.UTF8.GetString(buffer, 0, count));
                s.Close();
            }
        }

        public static async Task EchoLoopAsync(int index, int loops) {
            using (Socket s = new Socket(SocketType.Stream, ProtocolType.Tcp)) {
                await s.ConnectAsync(host_name, 7, CancellationToken.None);
                Console.Out.WriteLine($"EchoLoopAsync #{index}: Connected!");
                for (int i = 0; i < loops; i++) {
                    await EchoLoopAsync1(index, s, i);
                }
                await s.CloseAsync(CancellationToken.None);
            }
            Console.Out.WriteLine($"EchoLoopAsync #{index}.  Done!");
        }

        public static async Task PerfLoopAsync(int bufferSize) {
            using (var client = new TcpClient()) {
                await client.ConnectAsync(host_name, 7);
                byte[] buffer = new byte[bufferSize];
                _rand.NextBytes(buffer);
                long _received = 0;
                Stopwatch _receivedw = Stopwatch.StartNew();
                for (int i = 0; ; i++) {
                    _received += await EchoLoopAsync2(client.GetStream(), buffer);
                    Console.CursorLeft = 0; Console.CursorTop = 0;
                    Console.Out.WriteLine($"{ (_received / _receivedw.ElapsedMilliseconds) } kB/sec");
                }
            }
        }

        public static async Task PerfLoopComparedAsync(int bufferSize) {
            using (var client = new System.Net.Sockets.TcpClient()) {
                await client.ConnectAsync(host_name, 7);
                byte[] buffer = new byte[bufferSize];
                _rand.NextBytes(buffer);
                long _received = 0;
                Stopwatch _receivedw = Stopwatch.StartNew();
                for (int i = 0; ; i++) {
                    _received += await EchoLoopAsync2(client.GetStream(), buffer);
                    Console.CursorLeft = 0; Console.CursorTop = 0;
                    Console.Out.WriteLine($"{ (_received / _receivedw.ElapsedMilliseconds) } kB/sec");
                }
            }
        }

        private static async Task<int> EchoLoopAsync2(Stream stream, byte[] msg) {
            await stream.WriteAsync(msg, 0, msg.Length, CancellationToken.None);
            byte[] buffer = new byte[msg.Length];
            try {
                int offset = 0;
                while (offset < buffer.Length) {
                    offset += await stream.ReadAsync(buffer, offset, buffer.Length - offset);
                }
#if TEST
                if (!buffer.SameAs(msg)) {
                    throw new Exception("Bad echo returned!");
                }
#endif
                return offset;
            }
            catch {
                Console.Out.WriteLine("Failed to receive echo buffer");
                throw;
            }
        }

        public static async Task ReceiveAsync(int index, int port) {
            byte[] buffer = new byte[1024];
            using (TcpClient client = new TcpClient()) {
                await client.ConnectAsync(host_name, port, CancellationToken.None);
                Console.Out.WriteLine($"ReceiveAsync #{index}: Connected to port {port}!.  Read ...");
                using (NetworkStream str = client.GetStream()) {
                    int read = await str.ReadAsync(buffer, 0, buffer.Length, CancellationToken.None);
                    Console.Out.WriteLine($"{Encoding.UTF8.GetString(buffer, 0, read)}     #{index}");
                }
            }
            Console.Out.WriteLine($"ReceiveAsync #{index} port {port}.  Done!");
        }

        public static async Task SendReceiveAsync(int index, int port, byte[] buffer) {
            using (TcpClient client = new TcpClient()) {
                await client.ConnectAsync(host_name, port, CancellationToken.None);
                Console.Out.WriteLine($"SendReceiveAsync #{index}: Connected to port {port}!.  Write ...");
                NetworkStream str = client.GetStream();
                await str.WriteAsync(buffer, 0, buffer.Length, CancellationToken.None);
                int read = await str.ReadAsync(buffer, 0, buffer.Length, CancellationToken.None);
                Console.Out.WriteLine(Encoding.UTF8.GetString(buffer, 0, read));
            }
            Console.Out.WriteLine($"SendReceiveAsync #{index} port {port}.  Done!");
        }

        private static async Task EchoLoopAsync1(int index, Socket s, int i) {
            ushort id = (ushort)(short.MaxValue * _rand.NextDouble());
            var msg = Encoding.UTF8.GetBytes(
                string.Format("{0,6} async loop #{1} to echo server {2}", i, index, id));
            await s.SendAsync(msg, 0, msg.Length, CancellationToken.None);
            byte[] buffer = new byte[msg.Length];
            try {
                int count = await s.ReceiveAsync(buffer, 0, buffer.Length);
                Console.Out.WriteLine("({1,6}) received '{0}' ... (#{2}, {3})",
                    Encoding.UTF8.GetString(buffer, 0, count), i, index, id);
#if TEST
                if (!buffer.SameAs(msg)) {
                    throw new Exception("Bad echo returned!");
                }
#endif
            }
            catch {
                Console.Out.WriteLine("Failed to receive {1,6} '{0}'... (#{2}, {3})",
                    Encoding.UTF8.GetString(msg, 0, msg.Length), i, index, id);
                throw;
            }
        }
        static Random _rand = new Random();
    }
}
