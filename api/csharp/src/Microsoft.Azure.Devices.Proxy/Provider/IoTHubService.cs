// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Provider {
    using System;
    using System.Threading.Tasks;
    using System.Collections.Generic;
    using System.Text;
    using System.Threading;
    using System.IO;
    using System.Net;
    using System.Net.Http.Headers;
    using System.Runtime.Serialization;
    using System.Linq;
    using System.Collections.Concurrent;
    using System.Security.Cryptography;
    using Newtonsoft.Json;
    using Newtonsoft.Json.Linq;
    using Model;

    /// <summary>
    /// Resolves names through registry manager and provides remoting and streaming
    /// through devices methods.
    /// </summary>
    public partial class IoTHubService : ResultCache, IRemotingService, INameService, IStreamService {

        private static readonly string _apiVersion = "2016-11-14";
        private static readonly string _clientId = $"Microsoft.Azure.Devices.Proxy/{VersionEx.Assembly}";

        /// <summary>
        /// Constructor for IotHub service implementation
        /// </summary>
        /// <param name="hubConnectionString"></param>
        /// <param name="registryRefresh"></param>
        public IoTHubService(ConnectionString hubConnectionString, TimeSpan registryRefresh) {
            _hubConnectionString = hubConnectionString;
            StartTimer(registryRefresh);
        }

        /// <summary>
        /// Create stream connection through iot hub methods.
        /// </summary>
        /// <param name="streamId">Local reference address of the stream</param>
        /// <param name="remoteId">Remote reference of link</param>
        /// <param name="proxy">The proxy server</param>
        /// <returns></returns>
        public Task<IConnection> CreateConnectionAsync(Reference streamId,
            Reference remoteId, INameRecord proxy) {

            IConnection conn = new IoTHubStream(this, streamId, remoteId, proxy, null);

            // TODO: Revisit:  At this point we could either a) look up a host from the registry
            // then use it to create a dedicated stream with connection string or b) create an 
            // adhoc dr stream record in the registry. 

            return Task.FromResult(conn);
        }

        /// <summary>
        /// Invoke method on proxy
        /// </summary>
        /// <param name="proxy"></param>
        /// <param name="request"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async Task<Message> CallAsync(INameRecord proxy, Message request, 
            CancellationToken ct) {
            try {
                return await InvokeDeviceMethodAsync(proxy, request, TimeSpan.FromMinutes(3), 
                    ct).ConfigureAwait(false);
            }
            catch (OperationCanceledException) {
            }
            catch (Exception e) {
                ProxyEventSource.Log.HandledExceptionAsError(this, e);
            }
            return null;
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
                cts.Cancel();
            });

            try {
                string sql = "SELECT * FROM devices WHERE tags.proxy=1";
                var results = await LookupAsync(sql, ct).ConfigureAwait(false);
                var proxyList = new List<INameRecord>(results);
                if (!proxyList.Any()) {
                    ProxyEventSource.Log.NoProxyInstalled(this);
                }

                for (int attempts = 1; !ct.IsCancellationRequested && proxyList.Any(); attempts++) {
                    var tasks = new Dictionary<Task<Message>, INameRecord>();

                    proxyList.Shuffle();
                    foreach (var proxy in proxyList) {
                        // 3 second initial timeout on method call here to keep broadcasts moving
                        // We retry all failed invocations if none responds within 3 seconds
                        tasks.Add(TryInvokeDeviceMethodAsync(proxy, message, 
                            TimeSpan.FromSeconds(3 * attempts), cts.Token), proxy);
                    }
                    proxyList.Clear();

                    while (!ct.IsCancellationRequested && tasks.Any()) {
                        // Now link one by one
                        var method = await Task.WhenAny(tasks.Keys).ConfigureAwait(false);
                        var record = tasks[method];
                        tasks.Remove(method);
                        if (method.IsFaulted) {
                            proxyList.Add(record);
                            continue;
                        }
                        var response = await method;
                        if (response == null) {
                            proxyList.Add(record);
                            continue;
                        }
                        var disp = await handler(response, record, ct).ConfigureAwait(false);
                        /**/ if (disp == Disposition.Done) {
                            return;
                        }
                        else if (disp == Disposition.Retry) {
                            proxyList.Add(record);
                        }
                    }

                    ProxyEventSource.Log.BroadcastRetry(this, attempts, proxyList.Count);
                    if (!proxyList.Any()) {
                        break;
                    }
                }
                if (!ct.IsCancellationRequested) {
                    last(null);
                }
            }
            catch (OperationCanceledException) {
            }
            catch (Exception e) {
                if (!ct.IsCancellationRequested) {
                    last(e);
                    throw ProxyEventSource.Log.Rethrow(e, this);
                }
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

        //
        // To speed up access to host names, we cache them locally, resync 
        // them every so often with device registry...
        //
        private readonly ConcurrentDictionary<string, INameRecord> _cache = 
            new ConcurrentDictionary<string, INameRecord>();

        /// <summary>
        /// Override to also kick off a resync of the lookaside cache
        /// </summary>
        public override void Invalidate() {
            SynchronizeCacheAsync();
            base.Invalidate();
        }

        /// <summary>
        /// Registers a new record with device registry.  Host records are 
        /// synchronized lazily at every cache refresh to avoid spamming the 
        /// registry...
        /// </summary>
        /// <param name="proxy"></param>
        /// <param name="name"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async  Task AddOrUpdateAsync(INameRecord record, CancellationToken ct) {
            if (RecordType.Host == (record.Type & RecordType.Host)) {
                _cache.AddOrUpdate(record.Id, s => record, (s, r) => r.Assign(record));
            }
            else {
                await UpdateRecordAsync(record, ct);
                base.Invalidate();
            }
        }

        /// <summary>
        /// Removes a record from cache and registry and invalidates results cache.
        /// </summary>
        /// <param name="record"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async Task RemoveAsync(INameRecord record, CancellationToken ct) {
            _cache.TryRemove(record.Id, out record);
            await RemoveRecordAsync(record, ct);
            base.Invalidate();
        }

        /// <summary>
        /// Lookup record by record type
        /// </summary>
        /// <param name="name"></param>
        /// <param name="type"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async Task<IEnumerable<INameRecord>> LookupAsync(string name, RecordType type,
            CancellationToken ct) {
            if (string.IsNullOrEmpty(name)) {
                throw new ArgumentException(nameof(name));
            }
            name = name.ToLowerInvariant();
            if (RecordType.Host == (type & RecordType.Host)) {
                // Use cache to look up a host record
                INameRecord record;
                if (_cache.TryGetValue(name, out record) && record.References.Any()) {
                    return new Record(record).AsEnumerable();
                }
            }

            var sql = new StringBuilder("SELECT * FROM devices WHERE ");
            sql.Append("(deviceId = '");
            sql.Append(name);

            if (RecordType.Proxy == (type & RecordType.Proxy)) {
                sql.Append("' OR tags.name = '");
                sql.Append(name);
            }

            Reference address;
            if (Reference.TryParse(name, out address)) {
                sql.Append("' OR tags.address = '");
                sql.Append(address.ToString().ToLower());
            }

            sql.Append("') AND ");
            sql.Append(IoTHubRecord.CreateTypeQueryString(type));
            return await LookupAsync(sql.ToString(), ct).ConfigureAwait(false);
        }

        /// <summary>
        /// Lookup record by address
        /// </summary>
        /// <param name="address"></param>
        /// <param name="type"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async Task<IEnumerable<INameRecord>> LookupAsync(Reference address, RecordType type,
            CancellationToken ct) {
            if (address == null || address == Reference.Null) {
                throw new ArgumentException(nameof(address));
            }
            var sql = new StringBuilder("SELECT * FROM devices WHERE ");
            if (address != Reference.All) {
                sql.Append("tags.address = '");
                sql.Append(address.ToString().ToLower());
                sql.Append("' AND ");
            }
            sql.Append(IoTHubRecord.CreateTypeQueryString(type));
            return await LookupAsync(sql.ToString(), ct).ConfigureAwait(false);
        }

        /// <summary>
        /// Lookup record using sql query string
        /// </summary>
        /// <param name="sql"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        internal Task<IEnumerable<INameRecord>> LookupAsync(string sql, CancellationToken ct) =>
            Call(LookupUncachedAsync, sql, ct);

        /// <summary>
        /// Returns all results from a query
        /// </summary>
        /// <param name="sql"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        internal async Task<IEnumerable<INameRecord>> LookupUncachedAsync(
            string sql, CancellationToken ct) {
            var response = await PagedLookupWithRetryAsync(sql, null, ct).ConfigureAwait(false);
            if (string.IsNullOrEmpty(response.Item1))
                return response.Item2;
            var results = new List<INameRecord>(response.Item2);
            do {
                response = await PagedLookupWithRetryAsync(sql, response.Item1, ct).ConfigureAwait(false);
                results.AddRange(response.Item2);
            }
            while (!string.IsNullOrEmpty(response.Item1) && !ct.IsCancellationRequested);
            ct.ThrowIfCancellationRequested();
            return results;
        }

        /// <summary>
        /// Returns the query results async, but retries with exponential backoff when failure.
        /// </summary>
        /// <param name="sql"></param>
        /// <param name="continuation"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        private async Task<Tuple<string, IEnumerable<INameRecord>>> PagedLookupWithRetryAsync(
            string sql, string continuation, CancellationToken ct) {
#if PERF
            var sw = System.Diagnostics.Stopwatch.StartNew();
#endif
            var result = await Retry.WithLinearBackoff(ct, () => PagedLookupAsync(sql, continuation, ct),
                _ => !ct.IsCancellationRequested).ConfigureAwait(false);
#if PERF
            System.Diagnostics.Trace.TraceInformation($"Time for lookup took {sw.Elapsed.ToString()}");
#endif
            return result;
        }

        /// <summary>
        /// Returns the query results async
        /// </summary>
        /// <param name="sql"></param>
        /// <param name="continuation"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        private async Task<Tuple<string, IEnumerable<INameRecord>>> PagedLookupAsync(
            string sql, string continuation, CancellationToken ct) {
            if (string.IsNullOrEmpty(sql)) {
                throw ProxyEventSource.Log.ArgumentNull("sql");
            }
            try {
                var uri = new UriBuilder {
                    Scheme = "https",
                    Host = _hubConnectionString.HostName,
                    Path = "/devices/query",
                    Query = "api-version=" + _apiVersion
                };
                var stream = await _http.StreamAsync(
                    CreateUri("/devices/query"), Http.Post, 
                    async h => {
                        h.Add(HttpRequestHeader.Authorization.ToString(), 
                            await GetSasTokenAsync(3600).ConfigureAwait(false));
                        h.Add(HttpRequestHeader.UserAgent.ToString(), _clientId);

                        // Add previous continuation if any provided
                        if (!string.IsNullOrEmpty(continuation)) {
                            h.Add("x-ms-continuation", continuation);
                        }
                    }, (s, h) => {
                        // get continuation returned if any
                        IEnumerable<string> values;
                        if (h.TryGetValues("x-ms-continuation", out values)) {
                            continuation = values.FirstOrDefault();
                        }
                        else {
                            continuation = null;
                        }
                    }, ct, 
                    @"{""query"":""" + sql + @"""}", "application/json").ConfigureAwait(false);

                using (stream)
                using (StreamReader sr = new StreamReader(stream))
                using (JsonReader reader = new JsonTextReader(sr)) {
                    var results = JToken.ReadFrom(reader);
                    return Tuple.Create(continuation, results.Select(j =>
                        (INameRecord)new IoTHubRecord((string)j["deviceId"], 
                            (string)j["etag"], j["tags"] as JObject)));
                }
            }
            catch (Exception e) {
                throw ProxyEventSource.Log.Rethrow(e, this);
            }
        }

        /// <summary>
        /// Creates a device in the IoT Hub device registry 
        /// </summary>
        /// <param name="record"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        internal async Task<IoTHubRecord> AddRecordAsync(INameRecord record, 
            CancellationToken ct) {
            try {
                await _http.CallAsync(CreateUri("/devices/" + record.Id.ToLowerInvariant()), Http.Put,
                    async h => {
                        h.Add(HttpRequestHeader.Authorization.ToString(),
                            await GetSasTokenAsync(3600).ConfigureAwait(false));
                        h.Add(HttpRequestHeader.UserAgent.ToString(), _clientId);
                    }, (sc, h) => {
                        if (sc == HttpStatusCode.Conflict || sc == HttpStatusCode.PreconditionFailed) {
                            throw new TransientException();
                        }
                    }, ct, 
                    @"{""deviceId"": """ + record.Id + @"""}", "application/json").ConfigureAwait(false);

                ProxyEventSource.Log.RecordAdded(this, record);
                return new IoTHubRecord(record);
            }
            catch (TransientException) {
                // Retrieve the twin object and update it
                var result = await GetRecordAsync(record, ct);
                if (result == null) {
                    return null;
                }
                result.Assign(record);
                return result;
            }
            catch (Exception e) {
                throw ProxyEventSource.Log.Rethrow(e, this);
            }
        }

        /// <summary>
        /// Retrieves a single device twin based record from IoT Hub 
        /// </summary>
        /// <param name="record"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        internal async Task<IoTHubRecord> GetRecordAsync(INameRecord record,
            CancellationToken ct) {
            try {
                var stream = await _http.StreamAsync(
                    CreateUri("/twins/" + record.Id.ToLowerInvariant()), Http.Get,
                    async h => {
                        h.Add(HttpRequestHeader.Authorization.ToString(),
                            await GetSasTokenAsync(3600).ConfigureAwait(false));
                        h.Add(HttpRequestHeader.UserAgent.ToString(), _clientId);
                    }, (sc, h) => {
                        if (sc == HttpStatusCode.NotFound) {
                            throw new TransientException();
                        }
                    }, ct).ConfigureAwait(false);

                using (stream)
                using (StreamReader sr = new StreamReader(stream))
                using (JsonReader reader = new JsonTextReader(sr)) {
                    var j = JToken.ReadFrom(reader);
                    return new IoTHubRecord((string)j["deviceId"],
                        (string)j["etag"], j["tags"] as JObject);
                }
            }
            catch (TransientException) {
                ProxyEventSource.Log.RecordRemoved(this, record);
                return null;
            }
            catch (Exception e) {
                throw ProxyEventSource.Log.Rethrow(e, this);
            }
        }

        /// <summary>
        /// Deletes an record in the IoT Hub device registry 
        /// </summary>
        /// <param name="record"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        internal async Task RemoveRecordAsync(INameRecord record,
            CancellationToken ct) {
            try {
                await _http.CallAsync(
                    CreateUri("/devices/" + record.Id.ToLowerInvariant()), Http.Delete, 
                    async h => {
                        h.Add(HttpRequestHeader.Authorization.ToString(),
                            await GetSasTokenAsync(3600).ConfigureAwait(false));
                        h.Add(HttpRequestHeader.UserAgent.ToString(), _clientId);
                        h.IfMatch.Add(new EntityTagHeaderValue(@"""*"""));
                    }, 
                    (sc, h) => { }, ct).ConfigureAwait(false);
            }
            catch (Exception e) {
                ProxyEventSource.Log.HandledExceptionAsInformation(this, e);
            }
        }

        /// <summary>
        /// Updates a record in the device registry
        /// </summary>
        /// <param name="record"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        private async Task<INameRecord> UpdateRecordAsync(
            INameRecord record, CancellationToken ct) {

            string json;
            var hubRecord = record as IoTHubRecord;

            //
            // If the record is a generic record, add it first and retrieve the resulting
            // twin record from the registry.  Then assign the generic record to it and 
            // see if anything needs patching...
            //
            if (hubRecord == null) {
                // Create and convert generic record into hub record
                hubRecord = await AddRecordAsync(record, ct);
                if (hubRecord == null) {
                    return record;
                }
                json = hubRecord.Log;
                if (string.IsNullOrEmpty(json)) {
                    return hubRecord;
                }
            }
            else {
                json = hubRecord.Log;
                //
                // If this is already a twin record, check if there is anything to patch
                // if there is not, just attempt to retrieve the most recent one now...
                // 
                if (string.IsNullOrEmpty(json)) {
                    try {
                        return await GetRecordAsync(record, ct);
                    }
                    catch {
                        return record;
                    }
                }
            }
            //
            // If we logged changes to the record use the resulting patch json to patch 
            // up the twin record.  If we do not find the record anymore as part of 
            // patching it, then it was deleted, in which case return null.  Otherwise 
            // return the returned patched up twin record
            //
            ProxyEventSource.Log.PatchingRecord(this, record, json);
            try {
                var stream = await _http.StreamAsync(
                    CreateUri("/twins/" + hubRecord.Id.ToLowerInvariant()), Http.Patch, 
                    async h => {
                        h.Add(HttpRequestHeader.Authorization.ToString(),
                            await GetSasTokenAsync(3600).ConfigureAwait(false));
                        h.Add(HttpRequestHeader.UserAgent.ToString(), _clientId);
                        h.IfMatch.Add(new EntityTagHeaderValue(@"""*"""));
                    }, 
                    (sc, h) => {
                        if (sc == HttpStatusCode.NotFound) {
                            throw new TransientException();
                        }
                    }, ct, json, "application/json").ConfigureAwait(false);
                using (stream)
                using (StreamReader sr = new StreamReader(stream))
                using (JsonReader reader = new JsonTextReader(sr)) {
                    var j = JToken.ReadFrom(reader);
                    hubRecord = new IoTHubRecord((string)j["deviceId"],
                        (string)j["etag"], j["tags"] as JObject);
                }
            }
            catch (TransientException) {
                ProxyEventSource.Log.RecordRemoved(this, record);
                hubRecord = null;
            }
            catch (Exception e) {
                throw ProxyEventSource.Log.Rethrow(e, this);
            }
            ProxyEventSource.Log.RecordPatched(this, record, json);
            return hubRecord;
        }

        /// <summary>
        /// Try to synchronize the cache with device registry. 
        /// </summary>
        private async void SynchronizeCacheAsync() {
            foreach (var kv in _cache) {
                try {
                    var record = await UpdateRecordAsync(kv.Value, CancellationToken.None);
                    if (record != null) {
                        if (!record.Equals(kv.Value)) {
                            _cache.TryUpdate(kv.Key, record, kv.Value);
                        }
                    }
                    else {
                        _cache.TryRemove(kv.Key, out record);
                    }
                }
                catch (Exception ex) {
                    ProxyEventSource.Log.HandledExceptionAsInformation(this, ex);
                }
            }
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
            /// <param name="responseTimeout"></param>
            public MethodCallRequest(Message message, TimeSpan responseTimeout) {
                if (responseTimeout > TimeSpan.FromMinutes(5))
                    responseTimeout = TimeSpan.FromMinutes(5); // Max value for iothub is 5 minutes
                else if (responseTimeout < TimeSpan.FromSeconds(5))
                    responseTimeout = TimeSpan.FromSeconds(5);
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

#if PERF
        private long _calls;
        private long _failed;
        private System.Diagnostics.Stopwatch _callsw = System.Diagnostics.Stopwatch.StartNew();

        internal async Task<Message> InvokeDeviceMethodAsync(Message message, 
            TimeSpan timeout, CancellationToken ct) {
            bool log = true;
            try {
                message = await InvokeDeviceMethodInternalAsync(message, timeout, ct);
                _calls++;
                return message;
            }
            catch (OperationCanceledException) {
                log = false; throw;
            }
            catch (ProxyNotFoundException) {
                log = false; throw;
            }
            catch (Exception) {
                _failed++; throw;
            }
            finally {
                if (log) {
                    var persec = _calls / _callsw.Elapsed.TotalSeconds;
                    Console.CursorLeft = 0; Console.CursorTop = 1;
                    Console.WriteLine(
                        $"{_calls} calls [{_failed} failed] ({persec} calls/sec)");
                }
            }
        }

        /// <summary>
        /// Makes a method request from a message
        /// </summary>
        /// <param name="message"></param>
        /// <param name="timeout"></param>
        /// <returns></returns>
        internal async Task<Message> InvokeDeviceMethodInternalAsync(Message message, 
            TimeSpan timeout, CancellationToken ct) {
#else
        /// <summary>
        /// Makes a method request from a message
        /// </summary>
        /// <param name="message"></param>
        /// <param name="timeout"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        internal async Task<Message> InvokeDeviceMethodAsync(Message message, 
            TimeSpan timeout, CancellationToken ct) {
#endif
            if (string.IsNullOrEmpty(message.DeviceId)) {
                throw ProxyEventSource.Log.ArgumentNull("deviceId");
            }
            var buffer = new MemoryStream();
            var request = new MethodCallRequest(message, timeout);
            request.Encode(buffer, CodecId.Json);

            var uri = new UriBuilder {
                Scheme = "https",
                Host = _hubConnectionString.HostName,
                Path = $"/twins/{WebUtility.UrlEncode(message.DeviceId)}/methods",
                Query = "api-version=" + _apiVersion
            };
            MethodCallResponse response;
            try {
                var stream = await _http.StreamAsync(uri.Uri, Http.Post, async h => {
                    h.Add(HttpRequestHeader.Authorization.ToString(),
                        await GetSasTokenAsync(3600).ConfigureAwait(false));
                    h.Add(HttpRequestHeader.UserAgent.ToString(), _clientId);
                }, (s, h) => { }, ct, Encoding.UTF8.GetString(buffer.ToArray()),
                    "application/json").ConfigureAwait(false);
                using (stream) {
                    response = MethodCallResponse.Decode(stream, CodecId.Json);
                }
            }
            catch (HttpResponseException hex) {
                if (hex.StatusCode != HttpStatusCode.NotFound) {
                    ProxyEventSource.Log.HandledExceptionAsError(this, hex);
                    throw new ProxyException(
                        $"Remote proxy device method communication failure: {hex.StatusCode}.", hex);
                }
                else {
                    ProxyEventSource.Log.HandledExceptionAsInformation(this, hex);
                    throw new ProxyNotFoundException(hex);
                }
            }
            catch (OperationCanceledException) {
                throw;
            }
            catch (Exception ex) {
                ProxyEventSource.Log.HandledExceptionAsError(this, ex);
                throw new ProxyException($"Unknown proxy failure.", ex);
            }
            if (response.Status != 200) {
                throw ProxyEventSource.Log.Rethrow(
                    new ProxyException($"Unexpected status {response.Status} returned by proxy."));
            }
            return response.Payload;
        }

        /// <summary>
        /// Invoke method and return response
        /// </summary>
        /// <param name="record"></param>
        /// <param name="request"></param>
        /// <param name="timeout"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        internal async Task<Message> InvokeDeviceMethodAsync(INameRecord record, Message request,
            TimeSpan timeout, CancellationToken ct) {
            request.Proxy = record.Address;
            request.DeviceId = record.Id;
            return await InvokeDeviceMethodAsync(request, timeout, ct).ConfigureAwait(false);
        }

        /// <summary>
        /// Invoke method and return response
        /// </summary>
        /// <param name="record"></param>
        /// <param name="request"></param>
        /// <param name="timeout"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        internal async Task<Message> TryInvokeDeviceMethodAsync(INameRecord record, Message request,
            TimeSpan timeout, CancellationToken ct) {
            try {
                return await InvokeDeviceMethodAsync(
                    record, request, timeout, ct).ConfigureAwait(false);
            }
            catch (OperationCanceledException) {
                throw;
            }
            catch {
                return null;
            }
        }

        /// <summary>
        /// Make a uri to the service
        /// </summary>
        /// <param name="path"></param>
        /// <returns></returns>
        private Uri CreateUri(string path) {
            return new UriBuilder {
                Scheme = "https",
                Host = _hubConnectionString.HostName,
                Path = path,
                Query = "api-version=" + _apiVersion
            }.Uri;
        }

        /// <summary>
        /// Create a token for iothub.
        /// </summary>
        /// <param name="validityPeriodInSeconds"></param>
        /// <returns></returns>
        private Task<string> GetSasTokenAsync(int validityPeriodInSeconds) {
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
            return Task.FromResult($"SharedAccessSignature sr={encodedScope}" +
                $"&sig={Uri.EscapeDataString(sig)}&se={Uri.EscapeDataString(expiry)}" + 
                $"&skn={Uri.EscapeDataString(_hubConnectionString.SharedAccessKeyName)}");
        }

        private readonly ConnectionString _hubConnectionString;
        private readonly Http _http = new Http();
    }
}