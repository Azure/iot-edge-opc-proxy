// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using MsgPack;
    using System.Threading.Tasks;
    using System.Threading;
    using System.IO;

    /// <summary>
    /// Facade on top of msg pack reader/writer to read streams of 
    /// Proxy model objects.
    /// </summary>
    public class MsgPackStream<S> : SerializerContext, ICodecStream<S> where S : Stream {

        /// <summary>
        /// Default constructor
        /// </summary>
        public MsgPackStream(bool owner = false) {
            _owner = owner;

            Add(new MessageSerializer());
            Add(new PropertySerializer());
            Add(new SocketAddressSerializer());
            Add(new MulticastOptionSerializer());
        }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="strm"></param>
        public MsgPackStream(S strm, bool owner = false) : this(owner) =>
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
                        _reader = new Reader(_strm);
                    }
                    if (_strm.CanWrite) {
                        _writer = new Writer(_strm);
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
            Get<T>().ReadAsync(_reader, this, ct);

        /// <summary>
        /// Write object to stream
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="obj"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public Task WriteAsync<T>(T obj, CancellationToken ct) =>
            Get<T>().WriteAsync(_writer, obj, this, ct);

        /// <summary>
        /// Read object from stream
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="ct"></param>
        /// <returns></returns>
        public T Read<T>() => 
            TaskToSync.Run(() => ReadAsync<T>(CancellationToken.None));

        /// <summary>
        /// Write object to stream
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="obj"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public void Write<T>(T obj) =>
            TaskToSync.Run(() => WriteAsync(obj, CancellationToken.None));

        public void Dispose() {
            if (_owner) {
                ((IDisposable)_strm).Dispose();
            }
            _reader = null;
            _writer = null;
        }

        private Reader _reader;
        private Writer _writer;
        private S _strm;
        private bool _owner;
    }

    /// <summary>
    /// Non generic version.
    /// </summary>
    public class MsgPackStream : MsgPackStream<Stream>, ICodecStream {
        public MsgPackStream(Stream strm, bool owner = false) :
            base(strm, owner) {
        }
    }
}
