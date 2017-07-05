// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Provider {
    using System;
    using System.IO;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Collections.Generic;
    using System.Xml;
    using Relay;
#if NET45 || NET46
    using ServiceBus.Messaging;
#endif
    using System.Net.Http;
    using System.Net;

    public class ServiceBusNamespace {

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="connectionString"></param>
        public ServiceBusNamespace(ConnectionString connectionString) {
            _connectionString = connectionString;
            _tokenProvider = TokenProvider.CreateSharedAccessSignatureTokenProvider(
                _connectionString.SharedAccessKeyName, _connectionString.SharedAccessKey);
        }

        /// <summary>
        /// Delete connection in service bus namespace
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        public Task<string> DeleteConnectionAsync(string name) {
            return DoRequestAsync(GetManagementUri(name), HttpMethod.Delete, null);
        }

        /// <summary>
        /// Get key for connection, or if not exist, creates it.
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        public async Task<string> GetConnectionKeyAsync(string name) {
            string response;
            try {
                response = await GetConnectionAsync(name).ConfigureAwait(false);
                return GetKeyFromResponseXml(response);
            }
            catch (KeyNotFoundException e) {
                // Recreate the listener with a new key
                ProxyEventSource.Log.HandledExceptionAsInformation(this, e);
                try {
                    await DeleteConnectionAsync(name).ConfigureAwait(false);
                }
                catch { }
                response = await CreateConnectionAsync(name).ConfigureAwait(false);
            }
            catch (Exception ex) {
                ProxyEventSource.Log.HandledExceptionAsInformation(this, ex);
                response = await CreateConnectionAsync(name).ConfigureAwait(false);
            }
            return GetKeyFromResponseXml(response);
        }

        /// <summary>
        /// Get uri for named hybrid connection to open listener with
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        public Uri GetConnectionUri(string name) {
            var builder = new UriBuilder("sb", _connectionString.Endpoint.DnsSafeHost);
            builder.Path = name;
            return builder.Uri;
        }

        /// <summary>
        /// Create connection in service bus name space
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        protected Task<string> CreateConnectionAsync(string name) {
#if NET45 || NET46
            var key = SharedAccessAuthorizationRule.GenerateRandomKey();
            var putData = 
              @"<entry xmlns=""http://www.w3.org/2005/Atom"">
                    <title type=""text"">" + name + @"</title>
                    <content type=""application/xml"">
                        <HybridConnectionDescription xmlns:i=""http://www.w3.org/2001/XMLSchema-instance"" 
                                xmlns=""http://schemas.microsoft.com/netservices/2010/10/servicebus/connect"">
                            <RequiresClientAuthorization>true</RequiresClientAuthorization>
                            <AuthorizationRules>
                                <AuthorizationRule i:type=""SharedAccessAuthorizationRule"">
                                    <ClaimType>SharedAccessKey</ClaimType>
                                    <ClaimValue>None</ClaimValue>
                                    <Rights>
                                        <AccessRights>Send</AccessRights>
                                        <AccessRights>Listen</AccessRights>
                                    </Rights>
                                    <KeyName>proxy</KeyName>
                                    <PrimaryKey>" + key + @"</PrimaryKey>
                                </AuthorizationRule>
                            </AuthorizationRules>      
                        </HybridConnectionDescription>  
                    </content>
                </entry>";
            return DoRequestAsync(GetManagementUri(name), HttpMethod.Put, putData);
#else
            throw new NotSupportedException("Must create relay entity first and specifiy in connection string");
#endif
        }

        /// <summary>
        /// Get connection description in service bus name space
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        protected Task<string> GetConnectionAsync(string name) {
            return DoRequestAsync(GetManagementUri(name), HttpMethod.Get, null);
        }

        /// <summary>
        /// Extract key from xml, throws if not found
        /// </summary>
        /// <param name="response"></param>
        /// <returns></returns>
        private static string GetKeyFromResponseXml(string response) {
            var document = new XmlDocument();
            document.Load(new StringReader(response));
            XmlNamespaceManager manager = new XmlNamespaceManager(document.NameTable);
            manager.AddNamespace("ns", "http://schemas.microsoft.com/netservices/2010/10/servicebus/connect");
            var node = document.SelectSingleNode("//ns:PrimaryKey", manager);
            if (node == null)
                throw new KeyNotFoundException();
            return node.InnerText;
        }

        /// <summary>
        /// Create a token for service bus.  
        /// </summary>
        /// <param name="validityPeriodInSeconds"></param>
        /// <returns></returns>
        private async Task<string> GetSasTokenAsync(int validityPeriodInSeconds) {
            string baseAddress = new UriBuilder("https", _connectionString.Endpoint.DnsSafeHost).Uri.ToString();
            var token = await _tokenProvider.GetTokenAsync(baseAddress, new TimeSpan(0, 0, validityPeriodInSeconds));
            return token.TokenString;
        }

        /// <summary>
        /// Get uri for management operations
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        private Uri GetManagementUri(string name) {
            var builder = new UriBuilder("https", _connectionString.Endpoint.DnsSafeHost);
            builder.Path = name;
            builder.Query = "api-version=2016-07";
            return builder.Uri;
        }

        /// <summary>
        /// Helper to do rest call
        /// </summary>
        /// <param name="uri"></param>
        /// <param name="method"></param>
        /// <param name="payload"></param>
        /// <returns></returns>
        private Task<string> DoRequestAsync(Uri uri, HttpMethod method, string payload) =>
            _http.CallAsync(uri, method, async h => 
                h.Add(HttpRequestHeader.Authorization.ToString(), 
                    await GetSasTokenAsync(3600).ConfigureAwait(false)), 
                (s, h) => { }, CancellationToken.None, payload);

        private readonly ConnectionString _connectionString;
        private readonly TokenProvider _tokenProvider;
        private readonly Http _http = new Http();
    }
}