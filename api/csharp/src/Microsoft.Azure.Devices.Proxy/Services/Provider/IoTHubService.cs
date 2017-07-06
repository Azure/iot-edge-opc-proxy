// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Provider {
    using System;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Threading.Tasks.Dataflow;
    using System.Collections.Generic;
    using System.Text;
    using System.IO;
    using System.Net;
    using System.Net.Http.Headers;
    using System.Runtime.Serialization;
    using System.Linq;
    using System.Collections.Concurrent;
    using System.Security.Cryptography;
    using Newtonsoft.Json;
    using Newtonsoft.Json.Linq;

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
            InitUpdateBlock();
            StartTimer(registryRefresh);
        }

        #region INameService

        /// <summary>
        /// Internal query implementation - caches a record if found in local cache
        /// </summary>
        struct IoTHubNameServiceQuery : IQuery {
            public string sql;
            public INameRecord cached;

            public override string ToString() => sql != null ? sql : cached.ToString();
        }

        /// <summary>
        /// Create new IoTHubNameServiceQuery based on name and type
        /// </summary>
        /// <param name="name"></param>
        /// <param name="type"></param>
        /// <returns></returns>
        public IQuery NewQuery(string name, NameRecordType type) {
            if (string.IsNullOrEmpty(name)) {
                throw new ArgumentException(nameof(name));
            }
            name = name.ToLowerInvariant();

            // Lookup cached record here for efficiency
            if (NameRecordType.Host == (type & NameRecordType.Host)) {
                // Use cache to look up a host record
                if (_cache.TryGetValue(name, out INameRecord record) && record.References.Any()) {
                    return new IoTHubNameServiceQuery { cached = record };
                }
            }

            var sql = new StringBuilder("SELECT * FROM devices WHERE ");
            sql.Append("(deviceId = '");
            sql.Append(name);

            if (NameRecordType.Proxy == (type & NameRecordType.Proxy)) {
                sql.Append("' OR tags.name = '");
                sql.Append(name);
            }

            if (Reference.TryParse(name, out Reference address)) {
                sql.Append("' OR tags.address = '");
                sql.Append(address.ToString().ToLower());
            }

            sql.Append("') AND ");
            sql.Append(IoTHubRecord.CreateTypeQueryString(type));
            if (NameRecordType.Proxy == (type & NameRecordType.Proxy)) {
                sql.Append(" AND properties.reported.alive = 1");
            }
            return new IoTHubNameServiceQuery { sql = sql.ToString() };
        }

        /// <summary>
        /// Create new query based on address
        /// </summary>
        /// <param name="address"></param>
        /// <param name="type"></param>
        /// <returns></returns>
        public IQuery NewQuery(Reference address, NameRecordType type) {
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
            if (NameRecordType.Proxy == (type & NameRecordType.Proxy)) {
                sql.Append(" AND properties.reported.alive = 1");
            }
            return new IoTHubNameServiceQuery { sql = sql.ToString() };
        }

        /// <summary>
        /// Returns a lookup block that allows passing queries to
        /// </summary>
        /// <param name="results"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public IPropagatorBlock<IQuery, INameRecord> Lookup(ExecutionDataflowBlockOptions options) {
            if (options == null) {
                throw new ArgumentNullException(nameof(options));
            }
            var ct = options.CancellationToken;
            if (ct == null) {
                throw new ArgumentNullException(nameof(ct));
            }

            var results = new BufferBlock<INameRecord>(options);
            //
            // Create a completion action to complete the results when 
            // all queries complete, i.e. no more results will be pending.
            //
            var ca = new CompletionAction(() => results.Complete());
            var lookup = new ActionBlock<IQuery>(async input => {
                var query = (IoTHubNameServiceQuery)input;
                ca.Begin();
                try {
                    if (query.cached != null) {
                        await results.SendAsync(query.cached).ConfigureAwait(false);
                    }
                    else {
                        await LookupAsync(query.sql, results, ct).ConfigureAwait(false);
                    }
                }
                finally {
                    ca.End();
                }
            }, options);

            lookup.Completion.ContinueWith(t => {
                if (t.IsFaulted) {
                    ((IDataflowBlock)results).Fault(t.Exception);
                }
                ca.Dispose();
            });
            return DataflowBlockEx.Encapsulate(lookup, results);
        }

        /// <summary>
        /// Returns all results from a query
        /// </summary>
        /// <param name="sql"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        internal async Task LookupAsync(string sql, ITargetBlock<INameRecord> target, 
            CancellationToken ct) {
            string continuation = null;
            do {
                var response = await PagedLookupAsync(sql, continuation, ct).ConfigureAwait(false);
                foreach (var result in response.Item2) {
                    await target.SendAsync(result, ct).ConfigureAwait(false);
                }
                continuation = response.Item1;
            }
            while (!string.IsNullOrEmpty(continuation) && !ct.IsCancellationRequested);
        }

        /// <summary>
        /// Lookup record using sql query string
        /// </summary>
        /// <param name="sql"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        internal Task<Tuple<string, IEnumerable<INameRecord>>> PagedLookupAsync(
            string sql, string continuation, CancellationToken ct) =>
            Call(PagedLookupUncachedWithRetryAsync, sql, continuation, ct);

        /// <summary>
        /// Returns the query results async, but retries with exponential backoff when failure.
        /// </summary>
        /// <param name="sql"></param>
        /// <param name="continuation"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        private async Task<Tuple<string, IEnumerable<INameRecord>>> PagedLookupUncachedWithRetryAsync(
            string sql, string continuation, CancellationToken ct) {
#if DEBUG 
            var sw = System.Diagnostics.Stopwatch.StartNew();
#endif
            var result = await Retry.WithLinearBackoff(ct, () => 
                    PagedLookupUncachedAsync(sql, continuation, ct),
                _ => !ct.IsCancellationRequested).ConfigureAwait(false);
#if DEBUG
            System.Diagnostics.Trace.TraceInformation($" > {sw.Elapsed} < for Lookup of " +
                $"{sql} (cont: {continuation ?? "<none>"}) returned {result.Item2.Count()} records");
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
        private async Task<Tuple<string, IEnumerable<INameRecord>>> PagedLookupUncachedAsync(
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
        /// The update target block exposed by the name service.
        /// </summary>
        public ITargetBlock<Tuple<INameRecord, bool>> Update { get; private set; }

        /// <summary>
        /// Initializes the update target.  If the tuple item 2 is false removes 
        /// a record from cache and registry and invalidates results cache. If 
        /// true, registers a new record with device registry.  Host records are 
        /// synchronized lazily at every cache refresh to avoid spamming the 
        /// registry...
        /// </summary>
        private void InitUpdateBlock() {
            Update = new ActionBlock<Tuple<INameRecord, bool>>(async (tup) => {
                if (tup.Item2) {
                    if (NameRecordType.Host == (tup.Item1.Type & NameRecordType.Host)) {
                        _cache.AddOrUpdate(tup.Item1.Id, s => tup.Item1, (s, r) => r.Assign(tup.Item1));
                        return;
                    }
                    else {
                        await UpdateRecordAsync(tup.Item1, _close.Token).ConfigureAwait(false);
                    }
                }
                else {
                    _cache.TryRemove(tup.Item1.Id, out INameRecord record);
                    await RemoveRecordAsync(record, _close.Token).ConfigureAwait(false);
                }
                base.Invalidate();
            },
            new ExecutionDataflowBlockOptions {
                CancellationToken = _close.Token
            });
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
                var result = await GetRecordAsync(record, ct).ConfigureAwait(false);
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
                hubRecord = await AddRecordAsync(record, ct).ConfigureAwait(false);
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
                        return await GetRecordAsync(record, ct).ConfigureAwait(false);
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
                    var record = await UpdateRecordAsync(kv.Value, 
                        CancellationToken.None).ConfigureAwait(false);
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

        #endregion

        #region IStreamService

        /// <summary>
        /// Create stream connection through iot hub methods.
        /// </summary>
        /// <param name="streamId">Local reference address of the stream</param>
        /// <param name="remoteId">Remote reference of link</param>
        /// <param name="proxy">The proxy server</param>
        /// <returns></returns>
        public Task<IConnection> CreateConnectionAsync(Reference streamId,
            Reference remoteId, INameRecord proxy, CodecId encoding) {

            IConnection conn = new IoTHubStream(this, streamId, remoteId, proxy, null);

            // TODO: Revisit:  At this point we could either a) look up a host from the registry
            // then use it to create a dedicated stream with connection string or b) create an 
            // adhoc dr stream record in the registry. 

            return Task.FromResult(conn);
        }

        #endregion

        #region IRemotingService

        /// <summary>
        /// Invoke method on proxy
        /// </summary>
        /// <param name="proxy"></param>
        /// <param name="request"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public Task<Message> CallAsync(INameRecord proxy, Message request, TimeSpan timeout,
            CancellationToken ct) => InvokeDeviceMethodAsync(proxy, request, timeout, ct);

        /// <summary>
        /// Envelope for message call
        /// </summary>
        [DataContract]
        internal class MethodCallRequest : Poco<MethodCallRequest> {
            /// <summary>
            /// Method to run
            /// </summary>
            [DataMember(Name = "methodName", Order = 1)]
            public string MethodName { get; private set; } = "proxy";

            /// <summary>
            /// Method timeout in seconds
            /// </summary>
            [DataMember(Name = "responseTimeoutInSeconds", Order = 2)]
            public int ResponseTimeoutInSeconds { get; private set; }

            /// <summary>
            /// Method payload
            /// </summary>
            [DataMember(Name = "payload", Order = 3)]
            public Message Payload { get; private set; }

            /// <summary>
            /// Create request
            /// </summary>
            /// <param name="message"></param>
            /// <param name="responseTimeout"></param>
            /// <param name="methodName"></param>
            /// <returns></returns>
            public static MethodCallRequest Create(Message message, TimeSpan responseTimeout, 
                string methodName = "proxy") {
                var request = Get();

                if (responseTimeout > _maxTimeout) {
                    responseTimeout = _maxTimeout;
                }
                else if (responseTimeout < _minTimeout) {
                    responseTimeout = _minTimeout;
                }

                request.ResponseTimeoutInSeconds = (int)responseTimeout.TotalSeconds;
                request.Payload = message;
                request.MethodName = methodName;
                return request;
            }

            public override bool IsEqual(MethodCallRequest that) {
                return
                    IsEqual(Payload, that.Payload) &&
                    IsEqual(ResponseTimeoutInSeconds, that.ResponseTimeoutInSeconds) &&
                    IsEqual(MethodName, that.MethodName);
            }

            protected override void SetHashCode() {
                MixToHash(MethodName);
                MixToHash(ResponseTimeoutInSeconds);
                MixToHash(Payload);
            }

            // Max value for iothub is 5 minutes
            private static readonly TimeSpan _maxTimeout = TimeSpan.FromMinutes(5);
            private static readonly TimeSpan _minTimeout = TimeSpan.FromSeconds(5);
        }

        /// <summary>
        /// Envelope for message response
        /// </summary>
        [DataContract]
        internal class MethodCallResponse : Poco<MethodCallResponse> {
            /// <summary>
            /// Gets or sets the status of device method invocation.
            /// </summary>
            [DataMember(Name = "status")]
            public int Status { get; private set; }

            /// <summary>
            /// Method payload
            /// </summary>
            [DataMember(Name = "payload")]
            public Message Payload { get; private set; }

            /// <summary>
            /// Create response
            /// </summary>
            /// <param name="status"></param>
            /// <param name="payload"></param>
            /// <returns></returns>
            public static MethodCallResponse Create(int status, Message payload) {
                var response = Get();
                response.Status = status;
                response.Payload = payload;
                return response;
            }

            public override bool IsEqual(MethodCallResponse that) {
                return
                    IsEqual(Payload, that.Payload) &&
                    IsEqual(Status, that.Status);
            }

            protected override void SetHashCode() {
                MixToHash(Status);
                MixToHash(Payload);
            }
        }

#if PERF
        private long _calls;
        private long _failed;
        private System.Diagnostics.Stopwatch _callsw = System.Diagnostics.Stopwatch.StartNew();

        internal async Task<Message> InvokeDeviceMethodAsync(Message message, 
            TimeSpan timeout, CancellationToken ct) {
            bool log = true;
            try {
                message = await InvokeDeviceMethodInternalAsync(message, timeout, 
                    ct).ConfigureAwait(false);
                _calls++;
                return message;
            }
            catch (OperationCanceledException) {
                log = false; throw;
            }
            catch (ProxyNotFound) {
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
#if LOG_MESSAGES
            var sw = System.Diagnostics.Stopwatch.StartNew();
#endif
#endif
            if (string.IsNullOrEmpty(message.DeviceId)) {
                throw ProxyEventSource.Log.ArgumentNull("deviceId");
            }
            var buffer = new MemoryStream();
            var request = MethodCallRequest.Create(message, timeout);
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
                    response = Serializable.Decode<MethodCallResponse>(stream, CodecId.Json);
                }
            }
            catch (HttpResponseException hex) {
                /**/ if (hex.StatusCode == HttpStatusCode.NotFound) {
                    throw new ProxyNotFound(hex);
                }
                else if (hex.StatusCode == HttpStatusCode.Forbidden) {
                    throw new ProxyPermission(hex);
                }
                else if (hex.StatusCode == HttpStatusCode.GatewayTimeout) {
                    throw new ProxyTimeout(hex.Message, hex);
                }
                else {
                    ProxyEventSource.Log.HandledExceptionAsError(this, hex);
                    throw new ProxyException(
                        $"Remote proxy device method communication failure: {hex.StatusCode}.", hex);
                }
            }
            catch (OperationCanceledException) {
                throw;
            }
            catch (Exception ex) {
                ProxyEventSource.Log.HandledExceptionAsError(this, ex);
                throw new ProxyException($"Unknown proxy failure.", ex);
            }
#if LOG_MESSAGES
            finally {
                System.Diagnostics.Trace.TraceInformation(
                    $" > {sw.Elapsed} < for InvokeDeviceMethodAsync({message})");
            }
#endif
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
            using (var message = request.Clone()) {
                message.Proxy = record.Address;
                message.DeviceId = record.Id;
                return await InvokeDeviceMethodAsync(message, timeout, ct).ConfigureAwait(false);
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
        internal async Task<Message> TryInvokeDeviceMethodAsync(INameRecord record, Message request,
            TimeSpan timeout, CancellationToken ct) {
            try {
                return await InvokeDeviceMethodAsync(record, request, timeout, ct).ConfigureAwait(false);
            }
            catch (OperationCanceledException) {
                throw;
            }
            catch {}
            return null;
        }

        /// <summary>
        /// Try until cancelled or successful...
        /// </summary>
        /// <param name="record"></param>
        /// <param name="request"></param>
        /// <param name="timeout"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        internal Task<Message> TryInvokeDeviceMethodWithRetryAsync(
            INameRecord record, Message request, TimeSpan timeout, CancellationToken ct) =>
            Retry.Do(ct, () => InvokeDeviceMethodAsync(record, request, timeout, ct),
                (e) => !ct.IsCancellationRequested, Retry.NoBackoff, int.MaxValue);

        #endregion

        /// <summary>
        /// Make a uri to the service
        /// </summary>
        /// <param name="path"></param>
        /// <returns></returns>
        private Uri CreateUri(string path) => new UriBuilder {
                Scheme = "https",
                Host = _hubConnectionString.HostName,
                Path = path,
                Query = "api-version=" + _apiVersion
            }.Uri;
        
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
        private CancellationTokenSource _close = new CancellationTokenSource();
    }
}