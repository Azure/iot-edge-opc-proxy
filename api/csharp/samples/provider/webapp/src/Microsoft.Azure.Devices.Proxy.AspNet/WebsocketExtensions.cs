// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Provider {
    using Microsoft.AspNetCore.Builder;
    using Microsoft.Extensions.Options;

    /// <summary>
    /// Helper extension class
    /// </summary>
    public static class WebSocketExtensions {
        /// <summary>
        /// Attach proxy handler to application
        /// </summary>
        /// <param name="app"></param>
        /// <returns></returns>
        public static void ConfigureProxy(this IApplicationBuilder app, WebSocketOptions options) {
            app.UseWebSockets();
            app.UseMiddleware<WebSocketMiddleware>(Options.Create(options));
        }
    }
}
