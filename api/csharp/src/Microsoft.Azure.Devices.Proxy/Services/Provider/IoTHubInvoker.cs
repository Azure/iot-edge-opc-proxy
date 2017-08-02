// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Provider {
    using System;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Text;
    using System.IO;
    using System.Net;
    using System.Runtime.Serialization;

    /// <summary>
    /// Invokes iot hub device methods on a proxy
    /// </summary>
    internal class IoTHubInvoker : IDisposable {

        /// <summary>
        /// The connection string
        /// </summary>
        internal ConnectionString ConnectionString { get; }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="connectionString"></param>
        internal IoTHubInvoker(ConnectionString connectionString) {
            ConnectionString = connectionString;
        }

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

        internal async Task<Message> CallAsync(Message message,
            TimeSpan timeout, CancellationToken ct) {
            bool log = true;
            try {
                message = await CallInternalAsync(message, timeout,
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
        internal async Task<Message> CallInternalAsync(Message message,
            TimeSpan timeout, CancellationToken ct) {
#else
        /// <summary>
        /// Makes a method request from a message
        /// </summary>
        /// <param name="message"></param>
        /// <param name="timeout"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async Task<Message> CallAsync(Message message,
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
                Host = ConnectionString.HostName,
                Path = $"/twins/{WebUtility.UrlEncode(message.DeviceId)}/methods",
                Query = "api-version=" + IoTHubService._apiVersion
            };
            MethodCallResponse response;
            try {
                var stream = await _http.StreamAsync(uri.Uri, Http.Post, async h => {
                    h.Add(HttpRequestHeader.Authorization.ToString(),
                        await IoTHubService.GetSasTokenAsync(ConnectionString,
                            3600).ConfigureAwait(false));
                    h.Add(HttpRequestHeader.UserAgent.ToString(), IoTHubService._clientId);
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
        public async Task<Message> CallAsync(INameRecord record, Message request,
            TimeSpan timeout, CancellationToken ct) {
            using (var message = request.Clone()) {
                message.Proxy = record.Address;
                message.DeviceId = record.Id;
                return await CallAsync(message, timeout, ct).ConfigureAwait(false);
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
        public async Task<Message> TryCallAsync(INameRecord record, Message request,
            TimeSpan timeout, CancellationToken ct) {
            try {
                return await CallAsync(record, request, timeout, ct).ConfigureAwait(false);
            }
            catch (OperationCanceledException) {
                throw;
            }
            catch { }
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
        public Task<Message> TryCallWithRetryAsync(
            INameRecord record, Message request, TimeSpan timeout, int max, CancellationToken ct) =>
            Retry.Do(ct, () => CallAsync(record, request, timeout, ct),
                (e) => !ct.IsCancellationRequested, Retry.NoBackoff, max);


        /// <summary>
        /// Dispose underlying http client
        /// </summary>
        public void Dispose() => _http.Dispose();

        private readonly Http _http = new Http();
    }
}