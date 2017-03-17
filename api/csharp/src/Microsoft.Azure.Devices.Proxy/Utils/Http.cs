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
        public override String ToString() => $"{StatusCode}: {Response}";
    }

    public class Http {
        private HttpClient _client;

        /// <summary>
        /// Default constructor
        /// </summary>
        public Http() {
            _client = new HttpClient();
        }


        /// <summary>
        /// Helper to do rest call
        /// </summary>
        /// <param name="uri"></param>
        /// <param name="method"></param>
        /// <param name="addHeaders"></param>
        /// <param name="ct"></param>
        /// <param name="payload"></param>
        /// <param name="contentType"></param>
        /// <returns></returns>
        public async Task<string> CallAsync(Uri uri, HttpMethod method,
            Func<HttpRequestHeaders, Task> addHeaders, Action<HttpResponseHeaders> queryHeaders,
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
                    var resp = await _client.SendAsync(msg, ct).ConfigureAwait(false);
                    if ((int)resp.StatusCode < 200 || (int)resp.StatusCode > 300) {
                        throw new HttpResponseException(resp.StatusCode, await resp.Content.ReadAsStringAsync());
                    }
                    queryHeaders(resp.Headers);
                    return await resp.Content.ReadAsStringAsync().ConfigureAwait(false); 
                }
                catch (TaskCanceledException ex) {
                    if (!ct.IsCancellationRequested) {
                        throw new TimeoutException("SendAsync timed out", ex);
                    }
                    else {
                        throw;
                    }
                }
            }
        }

        /// <summary>
        /// Without cancellation token
        /// </summary>
        /// <param name="uri"></param>
        /// <param name="method"></param>
        /// <param name="addHeaders"></param>
        /// <param name="payload"></param>
        /// <param name="contentType"></param>
        /// <returns></returns>
        public Task<string> CallAsync(Uri uri, HttpMethod method,
            Func<HttpRequestHeaders, Task> addHeaders, Action<HttpResponseHeaders> queryHeaders,
            string payload = null, string contentType = null) =>
            CallAsync(uri, method, addHeaders, queryHeaders, CancellationToken.None, payload, contentType);


        /// <summary>
        /// Helper to do rest call that returns a stream
        /// </summary>
        /// <param name="uri"></param>
        /// <param name="method"></param>
        /// <param name="addHeaders"></param>
        /// <param name="ct"></param>
        /// <param name="payload"></param>
        /// <param name="contentType"></param>
        /// <returns></returns>
        public async Task<Stream> StreamAsync(Uri uri, HttpMethod method,
            Func<HttpRequestHeaders, Task> addHeaders, Action<HttpResponseHeaders> queryHeaders,
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
                    var resp = await _client.SendAsync(msg, ct).ConfigureAwait(false);
                    if ((int)resp.StatusCode < 200 || (int)resp.StatusCode > 300) {
                        throw new HttpResponseException(resp.StatusCode, await resp.Content.ReadAsStringAsync());
                    }
                    else {
                        queryHeaders(resp.Headers);
                        return await resp.Content.ReadAsStreamAsync().ConfigureAwait(false);
                    }
                }
                catch (TaskCanceledException ex) {
                    if (!ct.IsCancellationRequested) {
                        throw new TimeoutException("SendAsync timed out", ex);
                    }
                    else {
                        throw;
                    }
                }
            }
        }

        /// <summary>
        /// Without cancellation token
        /// </summary>
        /// <param name="uri"></param>
        /// <param name="method"></param>
        /// <param name="addHeaders"></param>
        /// <param name="payload"></param>
        /// <param name="contentType"></param>
        /// <returns></returns>
        public Task<Stream> StreamAsync(Uri uri, HttpMethod method,
            Func<HttpRequestHeaders, Task> addHeaders, Action<HttpResponseHeaders> queryHeaders,
            string payload = null, string contentType = null) =>
            StreamAsync(uri, method, addHeaders, queryHeaders, CancellationToken.None, payload, contentType);
    }
}
