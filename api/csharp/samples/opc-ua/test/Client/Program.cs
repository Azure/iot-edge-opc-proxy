// Copyright (c) 1996-2016, OPC Foundation. All rights reserved.
// The source code in this file is covered under a dual-license scenario:
//   - RCL: for OPC Foundation members in good-standing
//   - GPL V2: everybody else
// RCL license terms accompanied with this source code. See http://opcfoundation.org/License/RCL/1.00/
// GNU General Public License as published by the Free Software Foundation;
// version 2 of the License are accompanied with this source code. See http://opcfoundation.org/License/GPLv2
// This source code is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//

using System;
using System.Diagnostics;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using Opc.Ua;
using Opc.Ua.Client;
using Opc.Ua.Bindings;
using Opc.Ua.Bindings.Proxy;
using System.Security.Cryptography.X509Certificates;

namespace NetCoreConsoleClient {

    public class Program {

        private const String _clientName = ".Net Core OPC UA Console Client";

        enum Op {
            Browse, Reconnect, Subscribe, All, None
        }

        public static void Main(string[] args) {
            Console.WriteLine(_clientName);
            var endpointURL = "opc.tcp://" + Utils.GetHostName() + ":51210/UA/SampleServer";
            Op op = Op.None;
            int count = 0;

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
                        case "--subscribe":
                            if (op != Op.None) {
                                throw new ArgumentException("Operations are mutual exclusive");
                            }
                            op = Op.Subscribe;
                            break;
                        case "-b":
                        case "--browse":
                            if (op != Op.None) {
                                throw new ArgumentException("Operations are mutual exclusive");
                            }
                            op = Op.Browse;
                            break;
                        case "-r":
                        case "--reconnect":
                            if (op != Op.None) {
                                throw new ArgumentException("Operations are mutual exclusive");
                            }
                            op = Op.Reconnect;
                            break;
                        case "-c":
                        case "--count":
                            i++;
                            if (i >= args.Length || !int.TryParse(args[i], out count)) {
                                throw new ArgumentException($"Bad -v arg");
                            }
                            break;
                        case "-u":
                        case "--url":
                            i++;
                            if (i >= args.Length) {
                                throw new ArgumentException($"Bad -u arg");
                            }
                            endpointURL = args[i];
                            break;
                        case "-R":
                        case "--relay":
                            Microsoft.Azure.Devices.Proxy.Socket.Provider =
                                Microsoft.Azure.Devices.Proxy.Provider.RelayProvider.CreateAsync().Result;
                            break;
                        case "-W":
                        case "--websocket":
                            Microsoft.Azure.Devices.Proxy.Provider.WebSocketProvider.Create();
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
usage:       NetCoreConsoleClient [options] operation [args]

Options:
    --relay
     -R      Use relay provider instead of default provider.
    --websocket
     -W      Use websocket kestrel provider.
    --count
     -c      Number of operations to run.
             Defaults to 0 == infinite or 1 in case of --all.
    --url
     -u      Url to connect to - defaults to " + endpointURL +
                    @"

    --help
     -?
     -h      Prints out this help.

Operations (Mutually exclusive):
    --all
             Run all tests once (default).
    --subscribe
     -s      Run subscription tests
    --browse
     -b      Run browse tests
    --reconnect
     -r      Run reconnect tests.
"
                    );
                return;
            }

            if (op == Op.None) {
                op = Op.All;
            }
            if (count == 0) {
                count = op == Op.All ? 1 : int.MaxValue;
            }

            WcfChannelBase.g_CustomTransportChannel = new ProxyTransportChannelFactory();
            try {
                ConsoleSampleClient(endpointURL, op, count).Wait();
            }
            catch (AggregateException ae) {
                ae.Flatten();
                Console.Error.WriteLine("One or more error(s) occurred:");
                foreach (var e in ae.InnerExceptions) {
                    Console.Error.WriteLine($"   -> {e.Message}");
                }
            }
            catch (Exception e) {
                Console.Error.WriteLine($"An error occurred: {e.Message}");
            }
            Console.Out.WriteLine("Press any key to exit");
            Console.ReadKey();
        }

        static async Task ConsoleSampleClient(string endpointURL, Op op, int count) {
            Console.WriteLine("1 - Create an Application Configuration.");
            var config = new ApplicationConfiguration {
                ApplicationName = "UA Core Sample Client",
                ApplicationType = ApplicationType.Client,
                ApplicationUri = "urn:" + Utils.GetHostName() + ":OPCFoundation:CoreSampleClient",
                SecurityConfiguration = new SecurityConfiguration {
                    ApplicationCertificate = new CertificateIdentifier {
                        StoreType = "Directory",
                        StorePath = "OPC Foundation/CertificateStores/MachineDefault",
                        SubjectName = "UA Core Sample Client"
                    },
                    TrustedPeerCertificates = new CertificateTrustList {
                        StoreType = "Directory",
                        StorePath = "OPC Foundation/CertificateStores/UA Applications",
                    },
                    TrustedIssuerCertificates = new CertificateTrustList {
                        StoreType = "Directory",
                        StorePath = "OPC Foundation/CertificateStores/UA Certificate Authorities",
                    },
                    RejectedCertificateStore = new CertificateTrustList {
                        StoreType = "Directory",
                        StorePath = "OPC Foundation/CertificateStores/RejectedCertificates",
                    },
                    NonceLength = 32,
                    AutoAcceptUntrustedCertificates = true
                },
                TransportConfigurations = new TransportConfigurationCollection(),
                TransportQuotas = new TransportQuotas { OperationTimeout = 120000 },
                ClientConfiguration = new ClientConfiguration { DefaultSessionTimeout = 120000 }
            };

            await config.Validate(ApplicationType.Client);

            bool haveAppCertificate = config.SecurityConfiguration.ApplicationCertificate.Certificate != null;

            if (!haveAppCertificate) {
                Console.WriteLine("    INFO: Creating new application certificate: {0}", config.ApplicationName);

                X509Certificate2 certificate = CertificateFactory.CreateCertificate(
                    config.SecurityConfiguration.ApplicationCertificate.StoreType,
                    config.SecurityConfiguration.ApplicationCertificate.StorePath,
                    null,
                    config.ApplicationUri,
                    config.ApplicationName,
                    config.SecurityConfiguration.ApplicationCertificate.SubjectName,
                    null,
                    CertificateFactory.defaultKeySize,
                    DateTime.UtcNow - TimeSpan.FromDays(1),
                    CertificateFactory.defaultLifeTime,
                    CertificateFactory.defaultHashSize,
                    false,
                    null,
                    null
                    );

                config.SecurityConfiguration.ApplicationCertificate.Certificate = certificate;
            }

            haveAppCertificate = config.SecurityConfiguration.ApplicationCertificate.Certificate != null;

            if (haveAppCertificate) {
                config.ApplicationUri = Utils.GetApplicationUriFromCertificate(
                    config.SecurityConfiguration.ApplicationCertificate.Certificate);

                if (config.SecurityConfiguration.AutoAcceptUntrustedCertificates) {
                    config.CertificateValidator.CertificateValidation += CertificateValidator_CertificateValidation;
                }
            }
            else {
                Console.WriteLine("    WARN: missing application certificate, using unsecure connection.");
            }

            int reconnectLoops = op == Op.All ? 1 : count;
            for (int i = 1; i <= reconnectLoops; i++) {

                Console.WriteLine("2 - Discover endpoints of {0}.", endpointURL);
                Uri endpointURI = new Uri(endpointURL);
                var endpointCollection = DiscoverEndpoints(config, endpointURI, 10);
                var selectedEndpoint = SelectUaTcpEndpoint(endpointCollection, haveAppCertificate);
                Console.WriteLine("    Selected endpoint uses: {0}",
                    selectedEndpoint.SecurityPolicyUri.Substring(selectedEndpoint.SecurityPolicyUri.LastIndexOf('#') + 1));

                Console.WriteLine("3 - Create session with OPC UA server.");
                var endpointConfiguration = EndpointConfiguration.Create(config);
                var endpoint = new ConfiguredEndpoint(selectedEndpoint.Server, endpointConfiguration);
                endpoint.Update(selectedEndpoint);
                var session = await CreateSessionAsync(config, endpoint);

                if (op == Op.All || op == Op.Browse) {
                    Console.WriteLine("4 - Browse the OPC UA server namespace.");
                    int j = 0;

                    while (true) {
                        Stopwatch w = Stopwatch.StartNew();
                        byte[] continuationPoint;
                        ReferenceDescriptionCollection references;

                        var stack = new Stack<Tuple<string, ReferenceDescription>>();
                        session.Browse(
                            null,
                            null,
                            ObjectIds.ObjectsFolder,
                            0u,
                            BrowseDirection.Forward,
                            ReferenceTypeIds.HierarchicalReferences,
                            true,
                            (uint)NodeClass.Variable | (uint)NodeClass.Object | (uint)NodeClass.Method,
                            out continuationPoint,
                            out references);

                        Console.WriteLine(" DisplayName, BrowseName, NodeClass");
                        references.Reverse();
                        foreach (var rd in references) {
                            stack.Push(Tuple.Create("", rd));
                        }

                        while (stack.Count > 0) {
                            var browsed = stack.Pop();
                            session.Browse(
                                null,
                                null,
                                ExpandedNodeId.ToNodeId(browsed.Item2.NodeId, session.NamespaceUris),
                                0u,
                                BrowseDirection.Forward,
                                ReferenceTypeIds.HierarchicalReferences,
                                true,
                                (uint)NodeClass.Variable | (uint)NodeClass.Object | (uint)NodeClass.Method,
                                out continuationPoint,
                                out references);

                            references.Reverse();
                            foreach (var rd in references) {
                                stack.Push(Tuple.Create(browsed.Item1 + "   ", rd));
                            }
                            Console.WriteLine($"{browsed.Item1}{(references.Count == 0 ? "-" : "+")} "+
                                $"{browsed.Item2.DisplayName}, {browsed.Item2.BrowseName}, {browsed.Item2.NodeClass}");
                        }
                        Console.WriteLine($"   ....        took {w.ElapsedMilliseconds} ms...");
                        if (++j <= count)
                            break;

                        // Reconnect
                        session.Close();
                        session = await CreateSessionAsync(config, endpoint);
                    }
                }

                if (op == Op.All || op == Op.Subscribe) {

                    Console.WriteLine("5 - Create a subscription with publishing interval of 1 second.");
                    var subscription = new Subscription(session.DefaultSubscription) { PublishingInterval = 1000 };

                    Console.WriteLine("6 - Add a list of items (server current time and status) to the subscription.");
                    var list = new List<MonitoredItem> {
                        new MonitoredItem(subscription.DefaultItem) {
                            DisplayName = "ServerStatusCurrentTime", StartNodeId = "i=2258"
                        }
                    };
                    list.ForEach(m => m.Notification += OnNotification);
                    subscription.AddItems(list);

                    Console.WriteLine("7 - Add the subscription to the session.");
                    session.AddSubscription(subscription);
                    subscription.Create();

                    await Task.Delay(1000 * (count + 1));
                    subscription.Delete(false);
                    subscription.Dispose();
                }

                session.Close();
            }
        }

        private static async Task<Session> CreateSessionAsync(ApplicationConfiguration config, ConfiguredEndpoint endpoint) {
            var session = await Session.Create(config, endpoint, true,
                _clientName, 60000, null, null);

            // Access underlying proxy socket
            var channel = session.TransportChannel as IMessageSocketChannel;
            var socket = channel.Socket as ProxyMessageSocket;
            var proxySocket = socket.ProxySocket;
            Console.WriteLine("    Connected through proxy {0}.", proxySocket.LocalEndPoint.ToString());
            return session;
        }

        private static void OnNotification(MonitoredItem item, MonitoredItemNotificationEventArgs e) {
            foreach (var value in item.DequeueValues()) {
                Console.WriteLine("{0}: {1}, {2}, {3}", item.DisplayName, value.Value, value.SourceTimestamp, value.StatusCode);
            }
        }

        private static void CertificateValidator_CertificateValidation(CertificateValidator validator,
            CertificateValidationEventArgs e) {
            Console.WriteLine("Accepted Certificate: {0}", e.Certificate.Subject);
            e.Accept = (e.Error.StatusCode == StatusCodes.BadCertificateUntrusted);
        }

        private static EndpointDescriptionCollection DiscoverEndpoints(ApplicationConfiguration config,
            Uri discoveryUrl, int timeout) {
            // use a short timeout.
            EndpointConfiguration configuration = EndpointConfiguration.Create(config);
            configuration.OperationTimeout = timeout;

            using (DiscoveryClient client = DiscoveryClient.Create(
                discoveryUrl,
                EndpointConfiguration.Create(config))) {
                try {
                    EndpointDescriptionCollection endpoints = client.GetEndpoints(null);
                    ReplaceLocalHostWithRemoteHost(endpoints, discoveryUrl);
                    return endpoints;
                }
                catch (Exception e) {
                    Console.WriteLine("Could not fetch endpoints from url: {0}", discoveryUrl);
                    Console.WriteLine("Reason = {0}", e.Message);
                    throw e;
                }
            }
        }

        private static EndpointDescription SelectUaTcpEndpoint(EndpointDescriptionCollection endpointCollection,
            bool haveCert) {
            EndpointDescription bestEndpoint = null;
            foreach (EndpointDescription endpoint in endpointCollection) {
                if (endpoint.TransportProfileUri == Profiles.UaTcpTransport) {
                    if (bestEndpoint == null ||
                        haveCert && (endpoint.SecurityLevel > bestEndpoint.SecurityLevel) ||
                        !haveCert && (endpoint.SecurityLevel < bestEndpoint.SecurityLevel)) {
                        bestEndpoint = endpoint;
                    }
                }
            }
            return bestEndpoint;
        }

        private static void ReplaceLocalHostWithRemoteHost(EndpointDescriptionCollection endpoints, Uri discoveryUrl) {
            foreach (EndpointDescription endpoint in endpoints) {
                endpoint.EndpointUrl = Utils.ReplaceLocalhost(endpoint.EndpointUrl, discoveryUrl.DnsSafeHost);
                StringCollection updatedDiscoveryUrls = new StringCollection();
                foreach (string url in endpoint.Server.DiscoveryUrls) {
                    updatedDiscoveryUrls.Add(Utils.ReplaceLocalhost(url, discoveryUrl.DnsSafeHost));
                }
                endpoint.Server.DiscoveryUrls = updatedDiscoveryUrls;
            }
        }
    }
}
