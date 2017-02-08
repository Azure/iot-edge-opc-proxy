// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Model {
    using System;
    using System.IO;
    using System.Runtime.Serialization;
    using System.Threading.Tasks;
    using System.Threading;
    using Newtonsoft.Json;

    /// <summary>
    /// Known codecs
    /// </summary>
    public enum CodecId {
        Mpack = 1,
        Json,
        //...
        Unknown = 0
    }


    /// <summary>
    /// Base class for serializable object
    /// </summary>
    /// <typeparam name="T"></typeparam>
    [DataContract]
    public abstract class Serializable<T> where T : class {

        /// <summary>
        /// Create message from stream using specified codec.
        /// </summary>
        /// <param name="stream"></param>
        /// <param name="codecId"></param>
        /// <returns></returns>
        public static T Decode(Stream stream, CodecId codecId) {
            T decoded;
            switch (codecId) {
                case CodecId.Json:
                    using (var reader = new JsonTextReader(new StreamReader(stream)) {
                        CloseInput = false
                    }) {
                        decoded = new JsonSerializer() {
                            ContractResolver = CustomResolver.Instance
                        }.Deserialize<T>(reader);
                    }
                    break;
                case CodecId.Mpack:
                    using (var reader = new MsgPackStream(stream)) {
                        decoded = reader.ReadAsync<T>(CancellationToken.None).GetAwaiter().GetResult();
                    }
                    break;
                default:
                    throw new NotSupportedException();
            }
            return decoded;
        }

        /// <summary>
        /// Create message from stream using specified codec.
        /// </summary>
        /// <param name="stream"></param>
        /// <param name="codecId"></param>
        /// <returns></returns>
        public static Task<T> DecodeAsync(Stream stream, CodecId codecId, CancellationToken ct) {
            switch (codecId) {
                case CodecId.Json:
                    var tcs = new TaskCompletionSource<T>();
                    Task.Run(() => {
                        using (var reader = new JsonTextReader(new StreamReader(stream)) {
                            CloseInput = false
                        }) {
                            tcs.TrySetResult(new JsonSerializer() {
                                ContractResolver = CustomResolver.Instance
                            }.Deserialize<T>(reader));
                        }
                    }, ct);
                    return tcs.Task;
                case CodecId.Mpack:
                    using (var reader = new MsgPackStream(stream)) {
                        return reader.ReadAsync<T>(ct);
                    }
                default:
                    throw new NotSupportedException();
            }
        }

        /// <summary>
        /// Encode 
        /// </summary>
        /// <param name="stream"></param>
        /// <param name=""></param>
        /// <param name="codecId"></param>
        public void Encode(Stream stream, CodecId codecId) {
            switch (codecId) {
                case CodecId.Json:
                    using (var writer = new JsonTextWriter(new StreamWriter(stream)) {
                        CloseOutput = false
                    }) {
                        new JsonSerializer() {
                            ContractResolver = CustomResolver.Instance
                        }.Serialize(writer, this as T);
                        writer.Flush();
                    }
                    break;
                case CodecId.Mpack:
                    using (var writer = new MsgPackStream(stream)) {
                        writer.WriteAsync<T>(this as T, CancellationToken.None).GetAwaiter().GetResult();
                    }
                    break;
                default:
                    throw new NotSupportedException();
            }
        }

        /// <summary>
        /// Encode 
        /// </summary>
        /// <param name="stream"></param>
        /// <param name=""></param>
        /// <param name="codecId"></param>
        public async Task EncodeAsync(Stream stream, CodecId codecId, CancellationToken ct) {
            switch (codecId) {
                case CodecId.Json:
                    await Task.Run(() => {
                        using (var writer = new JsonTextWriter(new StreamWriter(stream)) {
                            CloseOutput = false
                        }) {
                            new JsonSerializer() {
                                ContractResolver = CustomResolver.Instance 
                            }.Serialize(writer, this as T);
                            writer.Flush();
                        }
                    }, ct);
                    break;
                case CodecId.Mpack:
                    using (var writer = new MsgPackStream(stream)) {
                        await writer.WriteAsync<T>(this as T, ct);
                    }
                    break;
                default:
                    throw new NotSupportedException();
            }
        }
    }
}
