// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace WebServerSample {
    using System;
    using System.Net;
    using System.IO;
    using System.IO.Compression;
    using System.Net.Security;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Threading.Tasks;
    using Microsoft.Extensions.Logging;
    using Microsoft.Extensions.Configuration;
    using Microsoft.AspNetCore.Builder;
    using Microsoft.AspNetCore.Hosting;
    using Microsoft.AspNetCore.Http;
    using Microsoft.AspNetCore.Hosting.Server.Features;
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

            app.Use(async (context, next) => {
                Uri realUri = context.Request.GetRealUri();
                if (realUri == null) {
                    await next();
                    return;
                }

                // Connect a proxy socket to http endpoint
                using (var client = new TcpClient()) {
                    using (var stream = await client.ConnectAsync(realUri, 
                        context.Request.IsHttps).ConfigureAwait(false)) {
                        try {
                            // Write request to stream
                            await stream.WriteAsync(context.Request, realUri).ConfigureAwait(false);
                            // Read from stream the response back.

                            await stream.ReadAsync(context.Response, val => {
                                var mappedUri = new UriBuilder(val);

                                // Handle relative paths...
                                if (string.IsNullOrEmpty(mappedUri.Host)) {
                                    mappedUri.Host = realUri.Host;
                                }
                                if (string.IsNullOrEmpty(mappedUri.Scheme) ||
                                    realUri.Scheme.Equals("https")) { // Do not allow downgrade
                                    mappedUri.Scheme = realUri.Scheme;
                                }

                                // Format of rewrite is http(s)://host:port/realHost/realPath
                                mappedUri.Path = $"{mappedUri.Host}/{mappedUri.Path.Trim('/')}";
                                mappedUri.Host = context.Request.Host.Host;
                                mappedUri.Port = app.GetPort(mappedUri.Scheme);

                                // Everything else stays as is...
                                var newUri = mappedUri.ToString();
                                Console.WriteLine($"=>   {val}: {newUri}");
                                return newUri;

                            }).ConfigureAwait(false);
                        }
                        catch(Exception ex) {
                            Console.WriteLine(ex.ToString());
                            context.Abort();
                        }
                    }
                }
            });
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
                    // options.ThreadCount = 4;
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


    /// <summary>
    /// Http extensions
    /// </summary>
    public static class Extensions {
        /// <summary>
        /// Connect a stream to a uri through the proxy
        /// </summary>
        /// <param name="client"></param>
        /// <param name="uri"></param>
        /// <param name="ssl"></param>
        /// <returns></returns>
        public static async Task<Stream> ConnectAsync(this TcpClient client, Uri uri, bool ssl) {
            await client.ConnectAsync(new ProxySocketAddress(uri.Host, uri.Port)).ConfigureAwait(false);
            var stream = (Stream)client.GetStream();
            if (!ssl) {
                return stream;
            }
            var sslStream = new SslStream(stream, true, (o, c, ch, e) => true, null);
            await sslStream.AuthenticateAsClientAsync(uri.Host).ConfigureAwait(false);
            return sslStream;
        }

        /// <summary>
        /// Return secure port for this webapp
        /// </summary>
        /// <param name="app"></param>
        /// <returns></returns>
        public static int GetPort(this IApplicationBuilder app, string scheme) {
            var addresses = app.ServerFeatures.Get<IServerAddressesFeature>();
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
        /// Get the actual uri from the request
        /// </summary>
        /// <param name="request"></param>
        /// <returns></returns>
        public static Uri GetRealUri(this HttpRequest request) {

            var path = request.Path.ToUriComponent().TrimStart('/');
            if (!string.IsNullOrEmpty(path)) {

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
                var target = WebUtility.UrlDecode(dest).Split(':');
                var port = request.IsHttps ? 443 : 80;
                if (target.Length > 1) {
                    port = int.Parse(target[1]);
                }

                var uri = new UriBuilder(request.IsHttps ? "https" : "http",
                    target[0], port, path);

                uri.Query = request.QueryString.Value;
                return uri.Uri;
            }
            return null;
        }

        /// <summary>
        /// Write request to stream
        /// </summary>
        /// <param name="stream"></param>
        /// <param name="request"></param>
        /// <param name="uri"></param>
        /// <returns></returns>
        public static async Task WriteAsync(this Stream stream, HttpRequest request, Uri uri) {
            Console.WriteLine("\nWriting request:");
            var header = $"{request.Method} {uri.Scheme}://{uri.Host}{uri.PathAndQuery} {request.Protocol}\r\n";
            Console.Write(header);

            using (var bufferStream = new MemoryStream()) {
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
                        filtered = filtered.Trim(',');
                        if (string.IsNullOrWhiteSpace(header)) {
                            continue; // none remaining, do not send...
                        }
                        header = $"{kv.Key}: {filtered}\r\n";
                    }
                    else if (kv.Key.Equals("Host",
                        StringComparison.CurrentCultureIgnoreCase)) {
                        header = $"Host: {uri.Host}\r\n";
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
        /// <returns></returns>
        public static async Task ReadAsync(this Stream stream, HttpResponse response,
            Func<string, string> rewrite) {
            // Now read response back and write out to response
            var reader = new HttpResponseReader(stream);

            bool isChunked = false;
            string contentEncoding = null;

            Console.WriteLine();
            var s = await reader.ReadLineAsync().ConfigureAwait(false); // header
            if (string.IsNullOrEmpty(s)) {
                return;  // throw
            }
            Console.WriteLine(s);
            response.StatusCode = int.Parse(s.Split(' ')[1]);
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

                /**/ if (key.Equals("Location",
                    StringComparison.CurrentCultureIgnoreCase)) {
                    val = rewrite(val);
                }
                else if (key.Equals("Transfer-Encoding",
                    StringComparison.CurrentCultureIgnoreCase)) {
                    isChunked = val.Equals("chunked", StringComparison.CurrentCultureIgnoreCase);
                }
                else if (key.Equals("Content-Encoding",
                    StringComparison.CurrentCultureIgnoreCase)) {
                    contentEncoding = val.ToLowerInvariant();
                }
                if (response.Headers.ContainsKey(key)) {
                    response.Headers.Remove(key);
                }
                response.Headers.Add(key, val);
            }

            Encoding encoder;
            if (IsTextContent(response.ContentType, out encoder)) {
                await reader.ReadBodyAsync(response, isChunked,
                    (body, len) => {

                        if (!string.IsNullOrWhiteSpace(contentEncoding)) {
                            using (var mem = new MemoryStream(body, 0, len)) {
                                /**/ if (contentEncoding.Equals("gzip")) {
                                    var gzip = new GZipStream(new MemoryStream(body, 0, len),
                                        CompressionMode.Decompress);
                                    s = gzip.ToString(encoder);
                                }
                                else if (contentEncoding.Equals("deflate")) {
                                    var deflate = new DeflateStream(new MemoryStream(body, 0, len),
                                        CompressionMode.Decompress);
                                    s = deflate.ToString(encoder);
                                }
                                else {
                                    throw new Exception("Unexpected content encoding");
                                }
                            }
                        }
                        else {
                            s = encoder.GetString(body, 0, len);
                        }

                        // We use a very stupid algorithm here, one could make 
                        // this a lot better

                        // 1. find any fully qualified urls
                        s = _urls.Replace(s, (f) => {
                            return rewrite(f.Value);
                        });
                        // 2. find any href="/ and src="/ and rewrite content.

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
                    }

                ).ConfigureAwait(false);
                return;
            }
            else {
                // Pass through
                await reader.ReadBodyAsync(response, isChunked, null);
            }
        }

        /// <summary>
        /// Convert an entire stream into a string...
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
            catch (IOException) {}
            return encoder.GetString(body, 0, offset);
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
                        continue;
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
                while (true) {
                    // Read each chunk and filter it
                    var line = await ReadLineAsync();
                    var body = await ReadBodyAsync(Convert.ToInt32(line, 16), filter);
                    line = await ReadLineAsync();
                    if (!string.IsNullOrEmpty(line)) {
                        throw new IOException("Malformed chunk");
                    }

                    // Write chunk
                    var buf = Encoding.ASCII.GetBytes($"{body.Length:X}\r\n");
                    await response.Body.WriteAsync(
                        buf, 0, buf.Length).ConfigureAwait(false);
                    if (body.Length > 0) {
                        await response.Body.WriteAsync(
                            body, 0, body.Length).ConfigureAwait(false);
                    }
                    await response.Body.WriteAsync(
                        _crlf, 0, _crlf.Length).ConfigureAwait(false);
                    if (body.Length == 0) {
                        break;
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
        private static Regex _urls = new Regex(@"
((www\.|(http|https)+\:\/\/)[&#95;.a-z0-9-]
+\.[a-z0-9\/&#95;:@=.+?,##%&~-]*[^.|\'|\# |!|\(|?|,| |>|<|;|\)])", RegexOptions.IgnoreCase);
    }
}
