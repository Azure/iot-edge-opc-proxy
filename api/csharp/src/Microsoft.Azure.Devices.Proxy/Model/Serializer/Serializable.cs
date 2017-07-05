// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.IO;
    using System.Threading.Tasks;
    using System.Threading;
    using Newtonsoft.Json;

    public static class Serializable {

        /// <summary>
        /// Convert stream to codec stream
        /// </summary>
        /// <param name="stream"></param>
        /// <param name="codecId"></param>
        /// <param name="reader">As reader or writer stream</param>
        /// <returns></returns>
        public static ICodecStream AsCodecStream(this Stream stream, CodecId codecId,
            bool owner = false) {
            switch (codecId) {
                case CodecId.Json:
                    return new JsonStream(stream, owner);
                case CodecId.Mpack:
                    return new MsgPackStream(stream,  owner);
                default:
                    throw new NotSupportedException();
            }
        }

        /// <summary>
        /// Convert stream to codec stream
        /// </summary>
        /// <param name="stream"></param>
        /// <param name="codecId"></param>
        /// <param name="reader">As reader or writer stream</param>
        /// <returns></returns>
        public static ICodecStream<S> AsCodecStream<S>(this S stream, CodecId codecId, 
            bool owner = false) where S : Stream {
            switch (codecId) {
                case CodecId.Json:
                    return new JsonStream<S>(stream, owner);
                case CodecId.Mpack:
                    return new MsgPackStream<S>(stream,  owner);
                default:
                    throw new NotSupportedException();
            }
        }

        /// <summary>
        /// Create message from stream using specified codec.
        /// </summary>
        /// <param name="stream"></param>
        /// <param name="codecId"></param>
        /// <returns></returns>
        public static T Decode<T>(Stream stream, CodecId codecId) {
            using (var reader = stream.AsCodecStream(codecId)) {
                return reader.Read<T>();
            }
        }

        /// <summary>
        /// Create message from stream using specified codec.
        /// </summary>
        /// <param name="stream"></param>
        /// <param name="codecId"></param>
        /// <returns></returns>
        public static async Task<T> DecodeAsync<T>(Stream stream, CodecId codecId, CancellationToken ct) {
            using (var reader = stream.AsCodecStream(codecId)) {
                return await reader.ReadAsync<T>(ct).ConfigureAwait(false);
            }
        }

        /// <summary>
        /// Encode 
        /// </summary>
        /// <param name="stream"></param>
        /// <param name=""></param>
        /// <param name="codecId"></param>
        public static void Encode<T>(this T ziss, Stream stream, CodecId codecId) where T : Poco {
            using (var writer = stream.AsCodecStream(codecId)) {
                writer.Write(ziss);
            }
        }

        /// <summary>
        /// Encode 
        /// </summary>
        /// <param name="stream"></param>
        /// <param name=""></param>
        /// <param name="codecId"></param>
        public static async Task EncodeAsync<T>(this T ziss, Stream stream, CodecId codecId, 
            CancellationToken ct) where T : Poco {
            using (var writer = stream.AsCodecStream(codecId)) {
                await writer.WriteAsync(ziss, ct).ConfigureAwait(false);
            }
        }
    }
}
