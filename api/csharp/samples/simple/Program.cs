// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Samples {
    using System;
    using System.Collections.Generic;
    using System.Text;
    using System.Threading.Tasks;
    using System.Linq;
    using Microsoft.Azure.Devices.Proxy;

    class Program {

        static string host_name;

        static void Main(string[] args) {

            if (args.Length > 0)
                host_name = args[0];
            else
                host_name = System.Net.Dns.GetHostName();

            //
            // Simple TCP/IP Services
            //
            // Port 7:  Echo http://tools.ietf.org/html/rfc862
            // Port 19: Chargen http://tools.ietf.org/html/rfc864
            // Port 13: daytime http://tools.ietf.org/html/rfc867
            // Port 17: quotd http://tools.ietf.org/html/rfc865
            //
            for (int j = 1; ; j++) {
                Console.Clear();
                Console.Out.WriteLine($"#{j} Sync tests...");
#if !PERF
                for (int i = 0; i < j + 1; i++) {
                    try {
                        SendReceive(7, Encoding.UTF8.GetBytes("Simple test to echo server"));
                        Receive(19);
                        Receive(13);
                        Receive(17);
                        Receive(19);
                        EchoLoop(j+1);
                    }
                    catch (Exception e) {
                        Console.Out.WriteLine(e.ToString());
                    }
                }
#endif
                Console.Clear();
                Console.Out.WriteLine($"#{j} Async tests...");
                try {
                    var tasks = new List<Task>();

                    for (int i = 0; i < j + 1; i++) {
                        tasks.Add(ReceiveAsync(i, 19));
                        tasks.Add(ReceiveAsync(i, 13));
                        tasks.Add(ReceiveAsync(i, 17));
                        tasks.Add(EchoLoopAsync(i, 5));
                    }
                    while (tasks.Any()) {
                        int index = Task.WaitAny(tasks.ToArray());
                        var task = tasks.ElementAt(index);
                        tasks.RemoveAt(index);
                        if (task.IsFaulted) {
                            Console.Out.WriteLine(task.Exception.ToString());
                        }
                    }
                }
                catch (Exception e) {
                    Console.Out.WriteLine(e.ToString());
                }
            }
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
                await s.ConnectAsync(host_name, 7);
                Console.Out.WriteLine($"EchoLoopAsync #{index}: Connected!");
                for (int i = 0; i < loops; i++) {
                    await EchoLoopAsync1(index, s, i);
                }
                await s.CloseAsync();
            }
            Console.Out.WriteLine($"EchoLoopAsync #{index}.  Done!");
        }

        public static async Task ReceiveAsync(int index, int port) {
            byte[] buffer = new byte[1024];
            using (TcpClient client = new TcpClient()) {
                await client.ConnectAsync(host_name, port);
                Console.Out.WriteLine($"ReceiveAsync #{index}: Connected to port {port}!.  Read ...");
                using (NetworkStream str = client.GetStream()) {
                    int read = await str.ReadAsync(buffer);
                    Console.Out.WriteLine($"{Encoding.UTF8.GetString(buffer, 0, read)}     #{index}");
                }
            }
            Console.Out.WriteLine($"ReceiveAsync #{index} port {port}.  Done!");
        }

        public static async Task SendReceiveAsync(int index, int port, byte[] buffer) {
            using (TcpClient client = new TcpClient()) {
                await client.ConnectAsync(host_name, port);
                Console.Out.WriteLine($"SendReceiveAsync #{index}: Connected to port {port}!.  Write ...");
                NetworkStream str = client.GetStream();
                await str.WriteAsync(buffer);
                int read = await str.ReadAsync(buffer);
                Console.Out.WriteLine(Encoding.UTF8.GetString(buffer, 0, read));
            }
            Console.Out.WriteLine($"SendReceiveAsync #{index} port {port}.  Done!");
        }

        private static async Task EchoLoopAsync1(int index, Socket s, int i) {
            var msg = Encoding.UTF8.GetBytes(string.Format("{0,6} async loop #{1} to echo server", i, index));
            await s.SendAsync(msg);
            byte[] buffer = new byte[msg.Length];
            int count = await s.ReceiveAsync(buffer);
            Console.Out.WriteLine("({1,6}) received '{0}' ... (#{2})",
                Encoding.UTF8.GetString(buffer, 0, count), i, index);
        }
    }
}
