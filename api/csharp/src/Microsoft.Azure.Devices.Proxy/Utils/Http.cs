// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.IO;
    using System.Net;
    using System.Net.Http;
    using System.Net.Http.Headers;
    using System.Text;
    using System.Threading;
    using System.Threading.Tasks;

    /// <summary>
    /// Http class exceptions for status != 200
    /// </summary>
    public class HttpResponseException : Exception {

        /// <summary>
        /// Failed status
        /// </summary>
        public HttpStatusCode StatusCode { get; private set; }

        /// <summary>
        /// Response content
        /// </summary>
        public string Response { get; private set; }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="status"></param>
        /// <param name="response"></param>
        internal HttpResponseException(HttpStatusCode status, string response) {
            StatusCode = status;
            Response = response;
        }

        /// <summary>
        /// Stringify exception
        /// </summary>
        /// <returns></returns>
        public override string ToString() => $"{StatusCode}: {Response}";
    }

    public class Http {
        private readonly HttpClient _client;

        public static readonly HttpMethod Patch = new HttpMethod("PATCH");
        public static readonly HttpMethod Get = HttpMethod.Get;
        public static readonly HttpMethod Put = HttpMethod.Put;
        public static readonly HttpMethod Post = HttpMethod.Post;
        public static readonly HttpMethod Delete = HttpMethod.Delete;

        /// <summary>
        /// Default constructor
        /// </summary>
        public Http() {
            _client = new HttpClient { Timeout = TimeSpan.FromMinutes(10) };
        }


        /// <summary>
        /// Helper to do rest call
        /// </summary>
        /// <param name="uri"></param>
        /// <param name="method"></param>
        /// <param name="addHeaders"></param>
        /// <param name="queryHeaders"></param>
        /// <param name="ct"></param>
        /// <param name="payload"></param>
        /// <param name="contentType"></param>
        /// <returns></returns>
        public async Task<string> CallAsync(Uri uri, HttpMethod method,
            Func<HttpRequestHeaders, Task> addHeaders, Action<HttpStatusCode, HttpResponseHeaders> queryHeaders,
            CancellationToken ct, string payload = null, string contentType = null) {
            using (var msg = new HttpRequestMessage(method, uri)) {
                if (addHeaders != null) {
                    await addHeaders(msg.Headers).ConfigureAwait(false);
                }
                if (contentType == null) {
                    if (payload != null) {
                        msg.Content = new StringContent(payload);
                    }
                }
                else {
                    msg.Headers.Accept.Add(new MediaTypeWithQualityHeaderValue(contentType));
                    if (payload != null) {
                        msg.Content = new StringContent(payload, Encoding.UTF8, contentType);
                    }
                }
                try {
#if !NET_STANDARD2_0 && !NET45 && !NET46
                    // https://github.com/dotnet/corefx/issues/10040 causes deadlocks
                    ct = CancellationToken.None;
#endif
#if PERF 
                    var sw = System.Diagnostics.Stopwatch.StartNew();
#endif
                    var resp = await _client.SendAsync(msg, HttpCompletionOption.ResponseHeadersRead, 
                        ct).ConfigureAwait(false);
#if PERF
                    System.Diagnostics.Trace.TraceInformation($"CallAsync to {uri} took {sw.Elapsed}");
#endif
                    queryHeaders(resp.StatusCode, resp.Headers);
                    if ((int)resp.StatusCode < 200 || (int)resp.StatusCode > 300) {
                        throw new HttpResponseException(resp.StatusCode, await resp.Content.ReadAsStringAsync());
                    }
                    return await resp.Content.ReadAsStringAsync().ConfigureAwait(false); 
                }
                catch (TaskCanceledException ex) {
                    if (!ct.IsCancellationRequested) {
                        throw new TimeoutException("SendAsync timed out", ex);
                    }
                    throw;
                }
            }
        }


        /// <summary>
        /// Helper to do rest call that returns a stream
        /// </summary>
        /// <param name="uri"></param>
        /// <param name="method"></param>
        /// <param name="addHeaders"></param>
        /// <param name="queryHeaders"></param>
        /// <param name="ct"></param>
        /// <param name="payload"></param>
        /// <param name="contentType"></param>
        /// <returns></returns>
        public async Task<Stream> StreamAsync(Uri uri, HttpMethod method,
            Func<HttpRequestHeaders, Task> addHeaders, Action<HttpStatusCode, HttpResponseHeaders> queryHeaders,
            CancellationToken ct, string payload = null, string contentType = null) {
            using (var msg = new HttpRequestMessage(method, uri)) {
                if (addHeaders != null)
                    await addHeaders(msg.Headers).ConfigureAwait(false);
                if (contentType == null) {
                    if (payload != null) {
                        msg.Content = new StringContent(payload);
                    }
                }
                else {
                    msg.Headers.Accept.Add(new MediaTypeWithQualityHeaderValue(contentType));
                    if (payload != null) {
                        msg.Content = new StringContent(payload, Encoding.UTF8, contentType);
                    }
                }
                try {
#if !NET_STANDARD2_0 && !NET45 && !NET46
                    // https://github.com/dotnet/corefx/issues/10040 causes deadlocks
                    ct = CancellationToken.None;
#endif
#if PERF 
                    var sw = System.Diagnostics.Stopwatch.StartNew();
#endif
                    var resp = await _client.SendAsync(msg, HttpCompletionOption.ResponseHeadersRead, 
                        ct).ConfigureAwait(false);
#if PERF
                    System.Diagnostics.Trace.TraceInformation($"StreamAsync to {uri} took {sw.Elapsed}");
#endif
                    queryHeaders(resp.StatusCode, resp.Headers);
                    if ((int)resp.StatusCode < 200 || (int)resp.StatusCode >= 300) {
                        throw new HttpResponseException(resp.StatusCode, await resp.Content.ReadAsStringAsync());
                    }
                    return await resp.Content.ReadAsStreamAsync().ConfigureAwait(false);
                }
                catch (TaskCanceledException ex) {
                    if (!ct.IsCancellationRequested) {
                        throw new TimeoutException("SendAsync timed out", ex);
                    }
                    throw;
                }
            }
        }
    }
}
