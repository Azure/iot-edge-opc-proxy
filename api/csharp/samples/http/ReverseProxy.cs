// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Samples {
    using System;
    using System.Net;
    using System.IO;
    using System.IO.Compression;
    using System.Net.Security;
    using System.Text;
    using System.Diagnostics;
    using System.Text.RegularExpressions;
    using System.Threading.Tasks;
    using Microsoft.AspNetCore.Builder;
    using Microsoft.AspNetCore.Http;
    using Microsoft.AspNetCore.Hosting.Server.Features;
    using Microsoft.Azure.Devices.Proxy;

    /// <summary>
    /// Forwarder middleware
    /// </summary>
    public class ReverseProxy {
        private readonly RequestDelegate _next;
        private readonly IApplicationBuilder _app;
        private readonly string _root;

        /// <summary>
        /// Creates middleware to forward requests to proxy
        /// </summary>
        /// <param name="next"></param>
        /// <param name="app"></param>
        public ReverseProxy(RequestDelegate next, IApplicationBuilder app, string root) {
            _next = next;
            _app = app;
            _root = root;
        }

        /// <summary>
        /// Middleware invoke entry point which forwards to proxy
        /// </summary>
        /// <param name="context"></param>
        /// <returns></returns>
        public async Task Invoke(HttpContext context) {
            var realUri = GetRealUri(context.Request);
            if (realUri != null) {
                await ServeAsync(context, realUri, val => {
                    // Rewrite uris
                    Uri parsed;
                    try {
                        if (Uri.TryCreate(val, UriKind.RelativeOrAbsolute, out parsed)) {
                            var mappedUri = new UriBuilder();
                            // Handle relative paths...
                            if (!parsed.IsAbsoluteUri || string.IsNullOrEmpty(parsed.Scheme)) {
                                mappedUri.Scheme = realUri.Scheme;
                            }
                            else {
                                mappedUri.Scheme = parsed.Scheme;
                            }

                            mappedUri.Host = context.Request.Host.Host;
                            mappedUri.Port = GetPort(mappedUri.Scheme);

                            // Everything else stays as is...
                            var newUri = mappedUri.ToString().Trim('/');
                            // Format of rewrite is http(s)://host:port/realHost/realPath
                            if (parsed.IsAbsoluteUri) {
                                newUri += $"/{_root}/{parsed.Host}/{parsed.PathAndQuery.TrimStart('/')}";
                            }
                            else {
                                newUri += $"/{_root}/{realUri.Host}/{parsed.OriginalString.TrimStart('/')}";
                            }
                            Trace.TraceInformation($"... rewriting {val} \n\t\t=> {newUri}");
                            return newUri;
                        }
                    }
                    catch { }
                    Trace.TraceError($"  FAILED to rewrite {val}!");
                    return val;
                }).ConfigureAwait(false);
            }
            else {
                await _next(context);
            }
        }

        /// <summary>
        /// Concrete fowarding implementation - service the requested resource
        /// </summary>
        /// <param name="context"></param>
        /// <param name="uri"></param>
        /// <param name="rewrite"></param>
        /// <returns></returns>
        protected async Task ServeAsync(HttpContext context, Uri uri,
            Func<string, string> rewrite) {
            // Connect a proxy socket to http endpoint

            TcpClient client = null;
            string host = uri.Host;
            string res = uri.PathAndQuery;
            Stream stream = null;
            try {
                while (true) {

                    // Connect a stream
                    if (client == null) {
                        client = new TcpClient();
                        stream = await ConnectAsync(client, uri).ConfigureAwait(false);
                    }

                    // Write request to stream
                    await WriteAsync(stream, context.Request, host, res).ConfigureAwait(false);

                    // Read from stream the response back.
                    var target = await ReadAsync(stream,
                        context.Response, host, rewrite).ConfigureAwait(false);
                    if (string.IsNullOrEmpty(target))
                        break;

                    // Handle redirect to new target - 
                    Uri redirect;
                    if (Uri.TryCreate(target, UriKind.RelativeOrAbsolute, out redirect)) {
                        res = redirect.PathAndQuery;
                        if (redirect.IsAbsoluteUri) {
                            // if it is absolute, reconnect to the new host
                            host = redirect.Host;
                        }
                        else {
                    //   // if target is relative, host is the same - see if we can reuse...
                    //   if (context.Response.Headers["Connection"] == "keep-alive") {
                    //       continue;
                    //   }
                        }
                    }
                    stream.Dispose();
                    client.Dispose();

                    client = null;
                }
            }
            catch (Exception ex) {
                Console.WriteLine($"  EXCEPTION: {ex.Message}!");
                Trace.TraceError($"  EXCEPTION: {ex.ToString()}!");
                context.Abort();
            }
            finally {
                if (stream != null) {
                    stream.Dispose();
                }
                if (client != null) {
                    client.Dispose();
                }
            }
        }

        /// <summary>
        /// Get the actual uri from the request
        /// </summary>
        /// <param name="request"></param>
        /// <returns></returns>
        protected virtual Uri GetRealUri(HttpRequest request) {

            var path = request.Path.ToUriComponent().TrimStart('/');
            var port = request.IsHttps ? 443 : 80;
            string target = null;

            if (!string.IsNullOrEmpty(path) && path.StartsWith(_root)) {
                path = path.Substring(_root.Length).TrimStart('/');

                // Parse out target, which is url encoded host:port before path?query
                var index = path.IndexOf('/');

                string dest;
                if (index >= 0) {
                    dest = path.Substring(0, index);
                    path = path.Substring(index).TrimEnd('/');
                }
                else {
                    dest = path;
                    path = "";
                }

                // Decode actual server name from dest
                var parts = WebUtility.UrlDecode(dest).Split(':');
                if (parts.Length > 1) {
                    port = int.Parse(parts[1]);
                }
                target = parts[0];
            }
            else if (request.Cookies.ContainsKey("host")) {
                // Use fallback cookie if exists...
                target = request.Cookies["host"];
            }
            else {
                return null;
            }

            var uri = new UriBuilder(request.IsHttps ? "https" : "http",
                target, port, path);
            uri.Query = request.QueryString.Value;
            return uri.Uri;
        }

        /// <summary>
        /// Connect a stream to a uri through the proxy
        /// </summary>
        /// <param name="client"></param>
        /// <param name="uri"></param>
        /// <returns></returns>
        protected async Task<Stream> ConnectAsync(TcpClient client, Uri uri) {
            await client.ConnectAsync(new ProxySocketAddress(uri.Host, uri.Port)).ConfigureAwait(false);
            var stream = (Stream)client.GetStream();
            if (!uri.Scheme.EndsWith("s")) {  // simplistic way to find if we need to connect via ssl...
                return stream;
            }
            var sslStream = new SslStream(stream, true, (o, c, ch, e) => true, null);
            await sslStream.AuthenticateAsClientAsync(uri.Host).ConfigureAwait(false);
            return sslStream;
        }

        /// <summary>
        /// Writes request to stream with named host and request target 
        /// </summary>
        /// <param name="stream"></param>
        /// <param name="request"></param>
        /// <param name="host"></param>
        /// <param name="target"></param>
        /// <returns></returns>
        protected async Task WriteAsync(Stream stream, HttpRequest request, 
            string host, string target) {
            Console.WriteLine("\nWriting request:");
            using (var bufferStream = new MemoryStream()) {
                var header = $"{request.Method} {target} {request.Protocol}\r\n";
                Console.Write(header);
                var buf = Encoding.ASCII.GetBytes(header);
                bufferStream.Write(buf, 0, buf.Length);
                foreach (var kv in request.Headers) {
                    if (kv.Key.Equals("Accept-Encoding",
                        StringComparison.CurrentCultureIgnoreCase)) {
                        string filtered = "";
                        foreach (var enc in kv.Value.ToString().Split(',')) {
                            var val = enc.ToLowerInvariant().Trim();
                            if (val.Equals("gzip") ||
                                val.Equals("deflate")) {
                                filtered += $" {val},";
                            }
                        }
                        filtered = filtered.Trim(',').Trim();
                        if (string.IsNullOrWhiteSpace(filtered)) {
                            continue; // none remaining, do not send...
                        }
                        header = $"{kv.Key}: {filtered}\r\n";
                    }
                    else if (kv.Key.Equals("Host",
                        StringComparison.CurrentCultureIgnoreCase)) {
                        header = $"Host: {host}\r\n";
                    }
                    else {
                        header = $"{kv.Key}: {kv.Value.ToString()}\r\n";
                    }
                    Console.Write(header);
                    buf = Encoding.ASCII.GetBytes(header);
                    bufferStream.Write(buf, 0, buf.Length);
                }
                bufferStream.Write(_crlf, 0, _crlf.Length);

                var buffer = bufferStream.ToArray();
                await stream.WriteAsync(buffer, 0, buffer.Length).ConfigureAwait(false);
            }
            await request.Body.CopyToAsync(stream).ConfigureAwait(false);
        }

        /// <summary>
        /// Read response from stream
        /// </summary>
        /// <param name="stream"></param>
        /// <param name="response"></param>
        /// <param name="rewrite"></param>
        /// <returns>redirected target</returns>
        protected async Task<string> ReadAsync(Stream stream, HttpResponse response,
            string host, Func<string, string> rewrite) {
            // Now read response back and write out to response
            var reader = new HttpResponseReader(stream);

            bool isChunked = false;
            string contentEncoding = null;
            var headers = new HeaderDictionary();

            // Parse headers and status code
            Console.WriteLine();
            var s = await reader.ReadLineAsync().ConfigureAwait(false); // header
            if (string.IsNullOrEmpty(s)) {
                return null;  // throw
            }
            Console.WriteLine(s);

            int statusCode = int.Parse(s.Split(' ')[1]);
            string target = null;
            while (true) {
                s = await reader.ReadLineAsync().ConfigureAwait(false);
                if (string.IsNullOrEmpty(s)) {
                    break;
                }
                Console.WriteLine(s);
                var index = s.IndexOf(':');
                if (index < 0) {
                    continue;
                }
                var key = s.Substring(0, index).Trim();
                var val = s.Substring(index + 1).Trim();

                /**/ if (key.Equals("Transfer-Encoding",
                    StringComparison.CurrentCultureIgnoreCase)) {
                    isChunked = val.Equals("chunked", StringComparison.CurrentCultureIgnoreCase);
                }
                else if (key.Equals("Content-Encoding",
                    StringComparison.CurrentCultureIgnoreCase)) {
                    contentEncoding = val.ToLowerInvariant();
                }
                else if (key.Equals("Set-Cookie")) {
                    // TODO: 
                    // continue;
                }
                else {
                    if (key.Equals("Location",
                        StringComparison.CurrentCultureIgnoreCase)) {
                        target = val;
                    }
                    val = _urls.Replace(val, (f) => rewrite(f.Value));
                }
                if (headers.ContainsKey(key)) {
                    headers.Remove(key);
                }
                headers.Add(key, val);
            }

            if (statusCode >= StatusCodes.Status300MultipleChoices ||
                statusCode < StatusCodes.Status400BadRequest) {

                if (!string.IsNullOrEmpty(target)) {
                    // Redirect to new location
                    return target;
                }
            }

            // Copy headers to response
            response.StatusCode = statusCode;
            response.Headers.Clear();
            foreach(var entry in headers) {
                response.Headers.Add(entry.Key, entry.Value);
            }

            Encoding encoder;
            if (response.StatusCode == StatusCodes.Status204NoContent ||
                response.StatusCode == StatusCodes.Status205ResetContent ||
                response.StatusCode == StatusCodes.Status304NotModified){
                response.ContentLength = 0;
                await response.Body.FlushAsync();
            }
            else if (IsTextContent(response.ContentType, out encoder)) {
                
                // Remember host as fallback
                response.Cookies.Append("host", host);

                await reader.ReadBodyAsync(response, isChunked, (body, len) => {
                    if (!string.IsNullOrWhiteSpace(contentEncoding)) {
                        var mem = new MemoryStream(body, 0, len);
                        /**/ if (contentEncoding.Equals("gzip")) {
                            using (var gzip = new GZipStream(mem,
                                CompressionMode.Decompress, false)) {
                                s = gzip.ToString(encoder);
                            }
                        }
                        else if (contentEncoding.Equals("deflate")) {
                            using (var deflate = new DeflateStream(mem,
                                CompressionMode.Decompress, false)) {
                                s = deflate.ToString(encoder);
                            }
                        }
                        else {
                            throw new Exception("Unexpected content encoding");
                        }
                    }
                    else {
                        s = encoder.GetString(body, 0, len);
                    }

                    // We use a very simple algorithm here, one could make 
                    // this a lot more elaborate...

                    // 1. find any fully qualified urls
                    s = _urls.Replace(s, (f) => rewrite(f.Value));

                    // 2. find any href="/ and src="/ and rewrite content.
                    s = _rels.Replace(s, (f) => {
                        var val = f.Groups["url"].Value.TrimStart().TrimStart('.');
                        if (!val.StartsWith("/") || val.StartsWith("//")) {
                            return f.Value;
                        }
                        return f.Value.Replace(f.Groups["url"].Value, rewrite(val));
                    });

                    // Console.WriteLine(s);

                    body = encoder.GetBytes(s);
                    if (!string.IsNullOrWhiteSpace(contentEncoding)) {

                        var mem = new MemoryStream();
                        /**/ if (contentEncoding.Equals("gzip")) {
                            using (var gzip = new GZipStream(mem,
                                CompressionMode.Compress, true)) {
                                gzip.Write(body, 0, body.Length);
                            }
                        }
                        else if (contentEncoding.Equals("deflate")) {
                            using (var deflate = new DeflateStream(mem,
                                CompressionMode.Compress, true)) {
                                deflate.Write(body, 0, body.Length);
                            }
                        }
                        body = mem.ToArray();
                    }
                    return body;
                }).ConfigureAwait(false);
            }
            else {
                // Pass through
                await reader.ReadBodyAsync(response, isChunked, null);
            }
            return null;
        }

        /// <summary>
        /// Return port for this webapp and desired scheme
        /// </summary>
        /// <param name="app"></param>
        /// <returns></returns>
        private int GetPort(string scheme) {
            var addresses = _app.ServerFeatures.Get<IServerAddressesFeature>();
            foreach (var addr in addresses.Addresses) {
                if (addr.StartsWith(scheme)) {
                    var parts = addr.Split(':');
                    if (parts.Length > 2) {
                        return int.Parse(parts[2]);
                    }
                }
            }
            // Oops, no scheme...?
            return 0;
        }

        /// <summary>
        /// Parse content type to find out if this is a text type
        /// </summary>
        /// <param name="contentType"></param>
        /// <param name="encoder"></param>
        /// <returns></returns>
        private static bool IsTextContent(string contentType, out Encoding encoder) {
            encoder = null;
            if (contentType == null) {
                return false;
            }
            // Parse content type
            var index = contentType.IndexOf('/');
            if (index <= 0) {
                return false;
            }
            // Parse top level and sublevel media type
            var toplevel = contentType.Substring(0, index).ToLowerInvariant();
            var subtype = contentType.Substring(index + 1).ToLowerInvariant();

            // and any optional params
            string[] parameters = null;
            index = subtype.IndexOf(';');
            if (index > 0) {
                parameters = subtype.Substring(index + 1).Split(';', '=');
                subtype = subtype.Substring(0, index);
            }
            subtype = subtype.Trim();

            bool isText = toplevel.Equals("text");
            if (!isText) {
                if (toplevel.Equals("application") &&
                    (subtype.Equals("json") ||
                     subtype.Equals("javascript") ||
                     subtype.EndsWith("+xml") ||
                     subtype.EndsWith("+json"))) {
                    isText = true;  // also text
                }
                // TODO: Multipart support?
            }
            if (isText) {
                if (parameters != null) {
                    for (index = 0; index < parameters.Length / 2; index++) {
                        if (parameters[index * 2].Trim().Equals("charset")) {
                            encoder = Encoding.GetEncoding(
                                parameters[index * 2 + 1].Trim());
                        }
                    }
                }
                if (encoder == null) {
                    encoder = Encoding.UTF8;
                }
            }
            return isText;
        }

        /// <summary>
        /// Helper parser / reader for reading a response from stream.
        /// </summary>
        private class HttpResponseReader {

            public HttpResponseReader(Stream stream) {
                _stream = stream;
            }

            /// <summary>
            /// Read header line by line
            /// </summary>
            /// <returns></returns>
            internal async Task<string> ReadLineAsync() {
                var buf = new byte[1024];
                int offset = 0;
                while(true) {
                    var read = await _stream.ReadAsync(buf, offset, 1).ConfigureAwait(false);
                    if (read != 1) {
                        break;
                    }
                    if (buf[offset++] == (byte)'\n') {
                        break;
                    }
                    if (offset == buf.Length) {
                        // Grow
                        var newbuf = new byte[buf.Length * 2];
                        Buffer.BlockCopy(buf, 0, newbuf, 0, buf.Length);
                        buf = newbuf;
                    }
                }
                return Encoding.ASCII.GetString(buf, 0, offset).Trim();
            }

            /// <summary>
            /// Read body with filter, read as chunked if needed...
            /// </summary>
            /// <param name="contentLength"></param>
            /// <param name="chunked"></param>
            /// <param name="filter"></param>
            internal async Task ReadBodyAsync(HttpResponse response, bool chunked, 
                Func<byte[], int, byte[]> filter) {
                if (chunked) {
                    await ReadChunkedAsync(response, filter);
                }
                else if (response.ContentLength == null) {
                    await ReadAllAsync(response, filter);
                }
                else {
                    var body = await ReadBodyAsync((int)response.ContentLength, filter);
                    response.ContentLength = body.Length;
                    if (body.Length > 0) {
                        await response.Body.WriteAsync(body, 0, body.Length).ConfigureAwait(false);
                    }
                }
                await response.Body.FlushAsync().ConfigureAwait(false);
            }

            /// <summary>
            /// Read until end of stream
            /// </summary>
            /// <param name="response"></param>
            /// <param name="filter"></param>
            /// <returns></returns>
            private async Task ReadAllAsync(HttpResponse response, 
                Func<byte[], int, byte[]> filter) {
                // Try to read as much as possible
                var body = new byte[1024];
                int offset = 0;
                try {
                    while (true) {
                        int read = await _stream.ReadAsync(body, offset,
                            body.Length - offset).ConfigureAwait(false);
                        if (read == 0)
                            break;
                        offset += read;
                        if (offset == body.Length) {
                            // Grow
                            var newbuf = new byte[body.Length * 2];
                            Buffer.BlockCopy(body, 0, newbuf, 0, body.Length);
                            body = newbuf;
                        }
                        Console.WriteLine($" {offset} bytes read... ");
                    }
                }
                catch (IOException) {
                }
                if (filter != null) {
                    body = filter(body, offset);
                    await response.Body.WriteAsync(body, 0, body.Length).ConfigureAwait(false);
                }
                else {
                    await response.Body.WriteAsync(body, 0, offset).ConfigureAwait(false);
                }
            }

            /// <summary>
            /// Read all chunks into response
            /// </summary>
            /// <param name="response"></param>
            /// <param name="filter"></param>
            /// <returns></returns>
            private async Task ReadChunkedAsync(HttpResponse response, 
                Func<byte[], int, byte[]> filter) {
                var content = filter == null ? null : new MemoryStream();
                byte[] body;
                while (true) {
                    // Read each chunk and filter it
                    var line = await ReadLineAsync();
                    body = await ReadBodyAsync(Convert.ToInt32(line, 16), null);
                    line = await ReadLineAsync();
                    if (!string.IsNullOrEmpty(line)) {
                        throw new IOException("Malformed chunk");
                    }

                    if (content == null) {
                        // Write chunks one by one 
                        var buf = Encoding.ASCII.GetBytes($"{body.Length:X}\r\n");
                        await response.Body.WriteAsync(
                            buf, 0, buf.Length).ConfigureAwait(false);
                        if (body.Length > 0) {
                            await response.Body.WriteAsync(
                                body, 0, body.Length).ConfigureAwait(false);
                        }
                        await response.Body.WriteAsync(
                            _crlf, 0, _crlf.Length).ConfigureAwait(false);
                    }
                    else if (body.Length > 0) {
                        content.Write(body, 0, body.Length);
                    }
                    if (body.Length == 0) {
                        break;
                    }
                }
                if (filter != null) {
                    // Write full filtered body, remove transfer encoding...
                    body = content.ToArray();
                    body = filter(body, body.Length);
                    response.ContentLength = body.Length;
                    response.Headers.Remove("Transfer-Encoding");
                    if (body.Length > 0) {
                        await response.Body.WriteAsync(
                            body, 0, body.Length).ConfigureAwait(false);
                    }
                }
            }

            /// <summary>
            /// Read x amount of bytes from buffer
            /// </summary>
            /// <param name="contentLength"></param>
            /// <returns></returns>
            private async Task<byte[]> ReadBodyAsync(int contentLength,
                Func<byte[], int, byte[]> filter) {
                var buf = new byte[contentLength];
                int offset = 0;
                while (offset < buf.Length) {
                    offset += await _stream.ReadAsync(buf, offset,
                        buf.Length - offset).ConfigureAwait(false);
                    Console.WriteLine($" {(offset * 100 / buf.Length)}% read... ");
                }
                if (filter != null && contentLength > 0) {
                    return filter(buf, buf.Length);
                }
                return buf;
            }

            private Stream _stream;
        }

        private static readonly byte[] _crlf = new byte[] { (byte)'\r', (byte)'\n' };
        private static Regex _rels = new Regex(
            @"(?<n>(a)|img|(script))\b[^>]*?\b(?<t>(?(1)href|src))\s*=\s*(?:" +
            @"""(?<url>(?:\\""|[^""])*)""|"+ 
              @"'(?<url>(?:\\'|[^'])*)')",
            RegexOptions.IgnoreCase);
        private static Regex _urls = new Regex(
            @"http(s)?://([\w+?\.\w+])+([a-zA-Z0-9\~\!\@\#\$\%\^\&amp;" +
            @"\*\(\)_\-\=\+\\\/\?\.\:\;\'\,]*([a-zA-Z0-9\?\#\=\/]){1})?", 
            RegexOptions.IgnoreCase);
    }


    public static class Extensions {
        /// <summary>
        /// Helper extension to convert an entire stream into a string...
        /// </summary>
        /// <param name="stream"></param>
        /// <param name="encoder"></param>
        /// <returns></returns>
        public static string ToString(this Stream stream, Encoding encoder) {
            // Try to read as much as possible
            var body = new byte[1024];
            int offset = 0;
            try {
                while (true) {
                    int read = stream.Read(body, offset, body.Length - offset);
                    if (read == 0)
                        break;
                    offset += read;
                    if (offset == body.Length) {
                        // Grow
                        var newbuf = new byte[body.Length * 2];
                        Buffer.BlockCopy(body, 0, newbuf, 0, body.Length);
                        body = newbuf;
                    }
                }
            }
            catch (IOException) { }
            return encoder.GetString(body, 0, offset);
        }
    }
}
