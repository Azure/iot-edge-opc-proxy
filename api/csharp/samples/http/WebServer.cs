// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Samples {
    using System.IO;
    using Microsoft.Extensions.Logging;
    using Microsoft.Extensions.Configuration;
    using Microsoft.AspNetCore.Builder;
    using Microsoft.AspNetCore.Hosting;
    using Microsoft.Azure.Devices.Proxy;

    /// <summary>
    /// Web app startup 
    /// </summary>
    public class Startup {

        /// <summary>
        /// App configuration
        /// </summary>
        public IConfigurationRoot Configuration { get; private set; }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="env"></param>
        public Startup(IHostingEnvironment env) {
            Configuration = new ConfigurationBuilder().AddEnvironmentVariables().Build();
        }

        /// <summary>
        /// Configure app - sample forwards http content between remote server and web browser...
        /// </summary>
        /// <param name="app"></param>
        /// <param name="loggerFactory"></param>
        public void Configure(IApplicationBuilder app, ILoggerFactory loggerFactory) {
            loggerFactory.AddConsole();
            Socket.RemoteTimeout = 120 * 1000;
            app.UseMiddleware<ReverseProxy>(app, "proxy");
        }

        /// <summary>
        /// Console server starting point
        /// </summary>
        /// <param name="args"></param>
        /// <returns></returns>
        public static int Main(string[] args) {
            var config = new ConfigurationBuilder()
                .AddCommandLine(args)
                .Build();
            var builder = new WebHostBuilder()
                .UseContentRoot(Directory.GetCurrentDirectory())
                .UseConfiguration(config)
                .UseStartup<Startup>()
                .UseKestrel(options => {
                    options.NoDelay = true;
                    options.UseHttps("testCert.pfx", "testPassword");
                    options.UseConnectionLogging();
                    if (config["threadCount"] != null) {
                        options.ThreadCount = int.Parse(config["threadCount"]);
                    }
                })
                .UseUrls("http://*:8080", "https://*:8081")
                ;
            var host = builder.Build();
            host.Run();
            return 0;
        }
    }
}
