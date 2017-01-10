
namespace simple_client {
    using System;
    using System.Collections.Generic;
    using System.Text;
    using System.Threading.Tasks;
    using Microsoft.Azure.Devices.Proxy;

    class Program {

        static string host_name;

        static void Main(string[] args) {

            Microsoft.Azure.Devices.Proxy.Provider.DefaultProvider.Instance.Init().Wait();

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
            for (int i = 0; i < 10; i++) {
                try {
                    Receive(19);
                }
                catch (Exception e) {
                    Console.Out.WriteLine(e.ToString());
                }
            }

            // Sync tests
            for (int i = 0; i < 3; i++) {
                try {
          
                    SendReceive(7, Encoding.UTF8.GetBytes("Simple test to echo server"));
          
                    Receive(19);
                    Receive(13);
                    Receive(17);
            
                    EchoLoop(100);
                }
                catch (Exception e) {
                    Console.Out.WriteLine(e.ToString());
                }
            }

            // async tests
            try {
                var tasks = new List<Task>();

                for (int i = 0; i < 1000; i++) {

                    tasks.Add(ReceiveAsync(19));
                    tasks.Add(ReceiveAsync(13));
                    tasks.Add(ReceiveAsync(17));

                    tasks.Add(EchoLoopAsync(100));
                }
                Task.WaitAll(tasks.ToArray());
            }
            catch (Exception e) {
                Console.Out.WriteLine(e.ToString());
            }
            Console.Out.WriteLine("Done");
        }

        public static void Receive(int port) {
            byte[] buffer = new byte[1024];
            using (Socket s = new Socket(SocketType.Stream, ProtocolType.Tcp)) {
                s.Connect(host_name, port);
                Console.Out.WriteLine("Connected!.  Receiving...");
                int count =  s.Receive(buffer);
                Console.Out.WriteLine(Encoding.UTF8.GetString(buffer, 0, count));
                s.Close();
            }
        }

        public static void EchoLoop(int loops) {
            using (Socket s = new Socket(SocketType.Stream, ProtocolType.Tcp)) {
                s.Connect(host_name, 7);
                Console.Out.WriteLine("Connected!.  Sending ...");

                for (int i = 0; i < loops; i++) {
                    s.Send(Encoding.UTF8.GetBytes(String.Format("{0} sync loop to echo server", i)));
                    byte[] buffer = new byte[1024];
                    Console.Out.WriteLine("Receiving...");
                    int count = s.Receive(buffer);
                    Console.Out.WriteLine(Encoding.UTF8.GetString(buffer, 0, count));
                }
                s.Close();
            }
        }

        public static void Send(int port, byte[] buffer, int iterations) {
            using (Socket s = new Socket(SocketType.Stream, ProtocolType.Tcp)) {
                s.Connect(host_name, port);
                Console.Out.WriteLine("Connected!.  Sending ...");
                for (int i = 0; i < iterations; i++)
                    s.Send(buffer);
                s.Close();
            }
        }

        public static void SendReceive(int port, byte[] buffer) {
            using (Socket s = new Socket(SocketType.Stream, ProtocolType.Tcp)) {
                s.Connect(host_name, port);
                Console.Out.WriteLine("Connected!.  Sending ...");
                s.Send(buffer);
                buffer = new byte[1024];
                Console.Out.WriteLine("Receiving sync...");
                int count = s.Receive(buffer);
                Console.Out.WriteLine(Encoding.UTF8.GetString(buffer, 0, count));
                s.Close();
            }
        }

        public static async Task EchoLoopAsync(int loops) {
            using (Socket s = new Socket(SocketType.Stream, ProtocolType.Tcp)) {
                await s.ConnectAsync(host_name, 7);
                Console.Out.WriteLine("Connected!.  Sending ...");
                for (int i = 0; i < loops; i++) {
                    await EchoLoopAsync1(s, i);
                }
                await s.CloseAsync();
            }
        }

        public static async Task ReceiveAsync(int port) {
            byte[] buffer = new byte[1024];
            using (TcpClient client = new TcpClient()) {
                await client.ConnectAsync(host_name, port);
                NetworkStream str = client.GetStream();
                int read = await str.ReadAsync(buffer);
                Console.Out.WriteLine(Encoding.UTF8.GetString(buffer, 0, read));
            }
        }

        public static async Task SendReceiveAsync(int port, byte[] buffer) {
            using (TcpClient client = new TcpClient()) {
                await client.ConnectAsync(host_name, port);
                NetworkStream str = client.GetStream();
                await str.WriteAsync(buffer);
                int read = await str.ReadAsync(buffer);
                Console.Out.WriteLine(Encoding.UTF8.GetString(buffer, 0, read));
            }
        }

        private static async Task EchoLoopAsync1(Socket s, int i) {
            var msg = Encoding.UTF8.GetBytes(String.Format("{0,6} async loop to echo server", i));
            await s.SendAsync(msg);
            byte[] buffer = new byte[msg.Length];
            int count = await s.ReceiveAsync(buffer);
            Console.Out.WriteLine("({1,6}) received '{0}' ...", Encoding.UTF8.GetString(buffer, 0, count), i);
        }
    }
}
