//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Provider {
    using System.IO;
    using System.Threading.Tasks;
    using Microsoft.Extensions.Configuration;
    using Microsoft.AspNetCore.Hosting;
    using Proxy;
    using System;

    /// <summary>
    /// Provider for websocket streaming based on Kestrel
    /// implementation
    /// </summary>
    public class WebSocketProvider {

        /// <summary>
        /// Create provider host from Configuration
        /// </summary>
        /// <returns></returns>
        public static void Create(bool useSsl = false) {
            Create(new UriBuilder(useSsl ? 
                "https://localhost:8081" : "http://localhost:8080").Uri);
        }

        /// <summary>
        /// Create provider host from Configuration
        /// </summary>
        /// <returns></returns>
        public static void Create(Uri url) {
            // Build configuration
            var config = new ConfigurationBuilder()
            .AddEnvironmentVariables()
            .Build();

            // Listen endpoint
            var endpoint = new UriBuilder(url);
            endpoint.Host = "*";

            // Build web host
            new WebHostBuilder()
            .UseContentRoot(Directory.GetCurrentDirectory())
            .UseConfiguration(config)
            .Configure(app => {
                var iothubowner = config.GetConnectionString("iothubowner");
                if (string.IsNullOrEmpty(iothubowner)) {
                    iothubowner = null;  // Let default provider find out...
                }
                app.ConfigureProxy(new WebSocketOptions {
                    IoTHubOwnerConnectionString = iothubowner,
                    PublicEndpoint = url
                });
            })
            .UseKestrel(options => {
                options.NoDelay = true;
                if (url.Scheme.Equals("https", StringComparison.OrdinalIgnoreCase)) {
                    options.UseHttps("testCert.pfx", "testPassword");
                }
                options.UseConnectionLogging();
                if (config["threadCount"] != null) {
                    options.ThreadCount = int.Parse(config["threadCount"]);
                }
                else {
                    options.ThreadCount = Environment.ProcessorCount / 2;
                }
                if (options.ThreadCount == 0) {
                    options.ThreadCount = 1;
                }
            })
            .UseUrls(endpoint.ToString())
            .Build()
            .Start();
        }
    }
}
