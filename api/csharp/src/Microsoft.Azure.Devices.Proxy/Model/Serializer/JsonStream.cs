// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using Newtonsoft.Json;
    using System.Threading.Tasks;
    using System.Threading;
    using System.IO;

    /// <summary>
    /// Facade on top of msg pack reader/writer to read streams of 
    /// Proxy model objects.
    /// </summary>
    public class JsonStream<S> : ICodecStream<S> where S : Stream {

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="owner"></param>
        public JsonStream(bool owner = false) {
            _owner = owner;
            _serializer = new JsonSerializer() {
                ContractResolver = CustomResolver.Instance
            };
        }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="strm"></param>
        /// <param name="owner"></param>
        public JsonStream(S strm, bool owner = false) : this (owner) =>
            Stream = strm;


        /// <summary>
        /// The wrapped stream
        /// </summary>
        public S Stream {
            get => _strm;

            set {
                _strm = value;

                if (_strm != null) {
                    if (_strm.CanRead) {
                        _reader = new JsonTextReader(new StreamReader(_strm)) {
                            CloseInput = _owner
                        };
                    }
                    if (_strm.CanWrite) {
                        _writer = new JsonTextWriter(new StreamWriter(_strm)) {
                            CloseOutput = _owner
                        };
                    }
                }
            }
        }

        /// <summary>
        /// Read object from stream
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="ct"></param>
        /// <returns></returns>
        public Task<T> ReadAsync<T>(CancellationToken ct) => 
            Task.Run(() => Read<T>(), ct);

        /// <summary>
        /// Write object to stream
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="obj"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public Task WriteAsync<T>(T obj, CancellationToken ct) => 
            Task.Run(() => Write(obj), ct);

        /// <summary>
        /// Read object from stream
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <returns></returns>
        public T Read<T>() => 
            _serializer.Deserialize<T>(_reader);

        /// <summary>
        /// Write object to stream
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="obj"></param>
        /// <returns></returns>
        public void Write<T>(T obj) {
            _serializer.Serialize(_writer, obj);
            _writer?.Flush();
        }

        public void Dispose() {

            var reader = _reader as IDisposable;
            _reader = null;

            var writer = _writer as IDisposable;
            _writer = null;

            reader?.Dispose();
            writer?.Dispose();

            if (_owner) {
                ((IDisposable)_strm).Dispose();
                _strm = null;
            }
        }

        private readonly JsonSerializer _serializer;
        private JsonTextReader _reader;
        private JsonTextWriter _writer;
        private S _strm;
        private bool _owner;
    }

    /// <summary>
    /// Non generic version.
    /// </summary>
    public class JsonStream : JsonStream<Stream>, ICodecStream {
        public JsonStream(Stream strm, bool owner = false) :
            base(strm, owner) {
        }
    }
}
