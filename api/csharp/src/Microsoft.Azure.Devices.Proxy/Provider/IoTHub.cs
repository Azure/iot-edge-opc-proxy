// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Provider {
    using System.Threading.Tasks;
    using System.Collections.Generic;
    using System.Text;
    using System.Threading;
    using System.IO;
    using System.Net.Http;
    using System.Net;
    using System.Runtime.Serialization;
    using System.Linq;
    using System.Security.Cryptography;
    using Newtonsoft.Json;
    using Newtonsoft.Json.Linq;
    using System;
    using Model;

    /// <summary>
    /// Resolves names through registry manager and provides remoting
    /// through devices methods.
    /// </summary>
    public class IoTHub : ResultCache, IRemotingService, INameService {

        private static readonly string _apiVersion = "2016-11-14";
        private static readonly string _clientId = "Microsoft.Azure.Devices/1.1.4";

        /// <summary>
        /// Wrapper around device twin result
        /// </summary>
        class IoTHubNameRecord : INameRecord {

            /// <summary>
            /// Device id as unique identifier of the record on the hub
            /// </summary>
            public string Id { get; set; }

            /// <summary>
            /// Returns the address of the record
            /// </summary>
            public Reference Address {
                get {
                    return Reference.Parse(_tags["id"].ToString());
                }
                set {
                    _tags["id"] = value.ToString();
                }
            }

            /// <summary>
            /// Returns the name of the record
            /// </summary>
            public string Name {
                get {
                    return _tags["name"].ToString();
                }
                set {
                    _tags["name"] = value;
                }
            }

            /// <summary>
            /// The type of the record
            /// </summary>
            public RecordType Type {
                get {
                    return (RecordType)int.Parse(
                        _tags["type"].ToString());
                }
                set {
                    _tags["type"] = ((Int32)value).ToString();
                    // No support for bit queries ...
                    if (0 == (value & RecordType.Proxy))
                        _tags["proxy"] = null;
                    else
                        _tags["proxy"] = "1";
                    if (0 == (value & RecordType.Host))
                        _tags["host"] = null;
                    else
                        _tags["host"] = "1";
                    if (0 == (value & RecordType.Link))
                        _tags["link"] = null;
                    else
                        _tags["link"] = "1";
                }
            }

            /// <summary>
            /// Device twin tags based record
            /// </summary>
            public IoTHubNameRecord(string deviceId, JObject tags) {
                Id = deviceId;
                _tags = tags;
            }
            private JObject _tags;
        }

        private ConnectionString _hubConnectionString;
        private Http _http = new Http();

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="elem"></param>
        public IoTHub(ConnectionString hubConnectionString) {
            _hubConnectionString = hubConnectionString;
            StartTimer(TimeSpan.FromMinutes(5));
        }

        /// <summary>
        /// Invoke method on proxy
        /// </summary>
        /// <param name="request"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async Task<Message> CallAsync(INameRecord proxy, Message request, 
            CancellationToken ct) {
            try {
                return await Retry.Do(ct,
                    () => InvokeDeviceMethodAsync(proxy, request, TimeSpan.FromMinutes(3), ct),
                    (e) => true, Retry.NoBackoff, int.MaxValue);
            }
            catch (Exception e) {
                ProxyEventSource.Log.HandledExceptionAsError(this, e);
                return null;
            }
        }

        /// <summary>
        /// Broadcast to all proxies
        /// </summary>
        /// <param name="message"></param>
        /// <param name="handler"></param>
        /// <param name="last"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async Task BroadcastAsync(Message message, 
            Func<Message, INameRecord, CancellationToken, Task<Disposition>> handler,
            Action<Exception> last, CancellationToken ct) {

            var cts = new CancellationTokenSource();
            ct.Register(() => {
                cts?.Cancel();
            });

            try {
                string sql = "SELECT * FROM devices WHERE tags.proxy=1";
                var results = await LookupAsync(sql, ct);
                if (!results.Any()) {
                    ProxyEventSource.Log.NoProxyInstalled(this);
                    return;
                }

                var proxyList = new List<INameRecord>(results);
                for (int attempts = 0; attempts < 20 && !ct.IsCancellationRequested; attempts++) {
                    var tasks = new Dictionary<Task<Message>, INameRecord>();
                    foreach (var proxy in proxyList) {
                        tasks.Add(TryInvokeDeviceMethodAsync(proxy, message, 
                            TimeSpan.FromMinutes(5), cts.Token), proxy);
                    }
                    proxyList.Clear();

                    while (!ct.IsCancellationRequested && tasks.Any()) {
                        // Now link one by one
                        var method = await Task.WhenAny(tasks.Keys);
                        var record = tasks[method];
                        tasks.Remove(method);
                        if (method.IsFaulted) {
                            proxyList.Add(record);
                            continue;
                        }
                        var disp = await handler(method.Result, record, cts.Token);
                        /**/ if (disp == Disposition.Done) {
                            return;
                        }
                        else if (disp == Disposition.Retry) {
                            proxyList.Add(record);
                        }
                    }
                    await Task.Delay(3 * attempts, cts.Token);
                }
                ct.ThrowIfCancellationRequested();
                last(null);
            }
            catch (OperationCanceledException) {
            }
            catch (Exception e) {
                last(e);
                throw ProxyEventSource.Log.Rethrow(e, this);
            }
            finally {
                try {
                    cts.Cancel(); // Cancel the remaining tasks...
                }
                catch (Exception ex) {
                    ProxyEventSource.Log.HandledExceptionAsInformation(this, ex);
                }
            }
        }

        /// <summary>
        /// Lookup entry by id
        /// </summary>
        /// <param name="id"></param>
        /// <returns></returns>
        public async Task<IEnumerable<INameRecord>> LookupAsync(Reference id, RecordType type,
            CancellationToken ct) {
            var sql = new StringBuilder("SELECT * FROM devices WHERE ");
            sql.Append("tags.id=");
            sql.Append(id.ToString().ToLower());
            sql.Append(RecordTypeExpression(" AND", type));
            return await LookupAsync(sql.ToString(), ct);
        }

        /// <summary>
        /// Lookup entry by record type
        /// </summary>
        /// <param name="type"></param>
        /// <returns></returns>
        public async Task<IEnumerable<INameRecord>> LookupAsync(string name, RecordType type,
            CancellationToken ct) {
            var sql = new StringBuilder("SELECT * FROM devices WHERE ");
            sql.Append("tags.name=");
            sql.Append(name.ToLower());
            sql.Append(RecordTypeExpression(" AND", type));
            return await LookupAsync(sql.ToString(), ct);
        }

        /// <summary>
        /// Returns all results from a query
        /// </summary>
        /// <param name="sql"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        private Task<IEnumerable<INameRecord>> LookupAsync(
            string sql, CancellationToken ct) =>
            Call((s, c) => LookupUncachedAsync(s, c), sql, ct);

        /// <summary>
        /// Returns all results from a query
        /// </summary>
        /// <param name="sql"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        private async Task<IEnumerable<INameRecord>> LookupUncachedAsync(
            string sql, CancellationToken ct) {
            var response = await PagedLookupWithRetryAsync(sql, null, ct);
            if (string.IsNullOrEmpty(response.Item1))
                return response.Item2;
            var results = new List<INameRecord>(response.Item2);
            do {
                response = await PagedLookupWithRetryAsync(sql, response.Item1, ct);
                results.Concat(response.Item2);
            }
            while (!string.IsNullOrEmpty(response.Item1) && !ct.IsCancellationRequested);
            ct.ThrowIfCancellationRequested();
            return results;
        }


        /// <summary>
        /// Returns the query results async, but retries with exponential backoff when failure.
        /// </summary>
        /// <param name="message"></param>
        /// <returns></returns>
        private async Task<Tuple<string, IEnumerable<INameRecord>>> PagedLookupWithRetryAsync(
            string sql, string continuation, CancellationToken ct) {
            try {
                return await Retry.WithLinearBackoff(ct,
                    () => PagedLookupAsync(sql, continuation, ct),
                    (e) => true);
            }
            catch (Exception e) {
                ProxyEventSource.Log.HandledExceptionAsError(this, e);
                return null;
            }
        }

        /// <summary>
        /// Returns the query results async
        /// </summary>
        /// <param name="message"></param>
        /// <returns></returns>
        private async Task<Tuple<string, IEnumerable<INameRecord>>> PagedLookupAsync(
            string sql, string continuation, CancellationToken ct) {
            if (string.IsNullOrEmpty(sql)) {
                throw ProxyEventSource.Log.ArgumentNull("sql");
            }
            try {
                var uri = new UriBuilder("https", _hubConnectionString.HostName);
                uri.Path = "/devices/query";
                uri.Query = "api-version=" + _apiVersion;

                var stream = await _http.StreamAsync(uri.Uri, HttpMethod.Post, async h => {
                    h.Add(HttpRequestHeader.Authorization.ToString(), await GetSASTokenAsync(3600));
                    h.Add(HttpRequestHeader.UserAgent.ToString(), _clientId);

                    // Add previous continuation if any provided
                    if (!string.IsNullOrEmpty(continuation)) {
                        h.Add("x-ms-continuation", continuation);
                    }
                    }, h => {
                        // get continuation returned if any
                        IEnumerable<string> values;
                        if (h.TryGetValues("x-ms-continuation", out values)) {
                            continuation = values.FirstOrDefault();
                        }
                        else {
                            continuation = null;
                        }
                    }, ct, @"{""query"":""" + sql + @"""}", "application/json");

                using (stream)
                using (StreamReader sr = new StreamReader(stream))
                using (JsonReader reader = new JsonTextReader(sr)) {
                    var results = JToken.ReadFrom(reader);
                    return Tuple.Create(continuation, results.Select(j => (INameRecord)
                        new IoTHubNameRecord((string)j["deviceId"], j["tags"] as JObject)));
                }
            }
            catch (Exception e) {
                throw ProxyEventSource.Log.Rethrow(e, this);
            }
        }

        /// <summary>
        /// Create record type expression from type
        /// </summary>
        /// <param name="type"></param>
        /// <returns></returns>
        private static string RecordTypeExpression(string logicalConcat, RecordType type) {
            var sql = new StringBuilder(logicalConcat);
            sql.Append(" (");
            bool concat = false;

            // No support for bit queries ...
            if (0 != (type & RecordType.Proxy)) {
                sql.Append("tags.proxy=1");
                concat = true;
            }
            if (0 != (type & RecordType.Host)) {
                if (concat) sql.Append(" OR ");
                sql.Append("tags.host=1");
                concat = true;
            }
            if (0 != (type & RecordType.Link)) {
                if (concat) sql.Append(" OR ");
                sql.Append("tags.link=1");
                concat = true;
            }
            if (!concat) {
                return "";
            }
            sql.Append(")");
            return sql.ToString();
        }

        /// <summary>
        /// Envelope for message call
        /// </summary>
        [DataContract]
        internal class MethodCallRequest : Serializable<MethodCallRequest> {
            /// <summary>
            /// Method to run
            /// </summary>
            [DataMember(Name = "methodName", Order = 1)]
            public string MethodName { get; set; } = "proxy";

            /// <summary>
            /// Method timeout in seconds
            /// </summary>
            [DataMember(Name = "responseTimeoutInSeconds", Order = 2)]
            public int ResponseTimeoutInSeconds { get; set; }

            /// <summary>
            /// Method payload
            /// </summary>
            [DataMember(Name = "payload", Order = 3)]
            public Message Payload { get; set; }

            /// <summary>
            /// Constructor
            /// </summary>
            /// <param name="message"></param>
            /// <param name="methodName"></param>
            /// <param name="responseTimeout"></param>
            public MethodCallRequest(Message message, TimeSpan responseTimeout) {
                if (responseTimeout == TimeSpan.MaxValue)
                    responseTimeout = TimeSpan.FromMinutes(5); // Max value for iothub is 5 minutes
                this.ResponseTimeoutInSeconds = (int)responseTimeout.TotalSeconds;
                this.Payload = message;
            }
        }

        /// <summary>
        /// Envelope for message response
        /// </summary>
        [DataContract]
        internal class MethodCallResponse : Serializable<MethodCallResponse> {
            /// <summary>
            /// Gets or sets the status of device method invocation.
            /// </summary>
            [DataMember(Name = "status")]
            public int Status { get; set; }

            /// <summary>
            /// Method payload
            /// </summary>
            [DataMember(Name = "payload")]
            public Message Payload { get; set; }
        }

        /// <summary>
        /// Makes a method request from a message
        /// </summary>
        /// <param name="message"></param>
        /// <param name="timeout"></param>
        /// <returns></returns>
        private async Task<Message> InvokeDeviceMethodAsync(Message message, 
            TimeSpan timeout, CancellationToken ct) {
            if (string.IsNullOrEmpty(message.DeviceId)) {
                throw ProxyEventSource.Log.ArgumentNull("deviceId");
            }
            var buffer = new MemoryStream();
            var request = new MethodCallRequest(message, timeout);
            request.Encode(buffer, CodecId.Json);

            var uri = new UriBuilder("https", _hubConnectionString.HostName);
            uri.Path = $"/twins/{WebUtility.UrlEncode(message.DeviceId)}/methods";
            uri.Query = "api-version=" + _apiVersion;
            var stream = await _http.StreamAsync(uri.Uri, HttpMethod.Post, async h => {
                h.Add(HttpRequestHeader.Authorization.ToString(), await GetSASTokenAsync(3600));
                h.Add(HttpRequestHeader.UserAgent.ToString(), _clientId);
            }, _ => {}, ct, Encoding.UTF8.GetString(buffer.ToArray()), "application/json");
            using (stream) {
                var response = MethodCallResponse.Decode(stream, CodecId.Json);
                if (response.Status != 200) {
                    if (response.Status == 0) {
                        throw new ProxyException($"Remote proxy not connected!");
                    }
                    throw ProxyEventSource.Log.Rethrow(
                        new ProxyException($"Unexpected status {response.Status}"));
                }
                return response.Payload;
            }
        }

        /// <summary>
        /// Invoke method and return response
        /// </summary>
        /// <param name="record"></param>
        /// <param name="request"></param>
        /// <param name="timeout"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        private async Task<Message> InvokeDeviceMethodAsync(INameRecord record, Message request,
            TimeSpan timeout, CancellationToken ct) {
            request.Proxy = record.Address;
            request.DeviceId = record.Id;
            return await InvokeDeviceMethodAsync(request, timeout, ct);
        }

        /// <summary>
        /// Invoke method and return response
        /// </summary>
        /// <param name="record"></param>
        /// <param name="request"></param>
        /// <param name="timeout"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        private async Task<Message> TryInvokeDeviceMethodAsync(INameRecord record, Message request,
            TimeSpan timeout, CancellationToken ct) {
            try {
                return await InvokeDeviceMethodAsync(record, request, timeout, ct);
            }
            catch (ProxyException) {
            }
            catch (Exception e) {
                ProxyEventSource.Log.HandledExceptionAsWarning(this, e);
            }
            return null;
        }

        /// <summary>
        /// Create a token for iothub.
        /// </summary>
        /// <param name="validityPeriodInSeconds"></param>
        /// <returns></returns>
        private Task<string> GetSASTokenAsync(int validityPeriodInSeconds) {
            // http://msdn.microsoft.com/en-us/library/azure/dn170477.aspx
            // signature is computed from joined encoded request Uri string and expiry string
            DateTime expiryTime = DateTime.UtcNow + TimeSpan.FromSeconds(validityPeriodInSeconds);
            var expiry = ((long)(expiryTime - 
                new DateTime(1970, 1, 1, 0, 0, 0, 0, DateTimeKind.Utc)).TotalSeconds).ToString();
            string encodedScope = Uri.EscapeDataString(_hubConnectionString.HostName);
            string sig;
            // the connection string signature is base64 encoded
            var key = Convert.FromBase64String(_hubConnectionString.SharedAccessKey);
            using (var hmac = new HMACSHA256(key)) {
                sig = Convert.ToBase64String(hmac.ComputeHash(
                    Encoding.UTF8.GetBytes(encodedScope + "\n" + expiry)));
            }
            return Task.FromResult(string.Format("SharedAccessSignature sr={0}&sig={1}&se={2}&skn={3}",
                encodedScope, Uri.EscapeDataString(sig), Uri.EscapeDataString(expiry),
                Uri.EscapeDataString(_hubConnectionString.SharedAccessKeyName)));
        }
    }
}