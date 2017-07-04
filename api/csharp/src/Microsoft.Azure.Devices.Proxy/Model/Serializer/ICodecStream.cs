// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.IO;
    using System.Threading;
    using System.Threading.Tasks;

    public interface ICodecStream<S> : IDisposable where S : Stream {

        /// <summary>
        /// Underlying stream
        /// </summary>
        S Stream {
            get; set;
        }

        /// <summary>
        /// Read type T from stream.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="ct"></param>
        /// <returns></returns>
        Task<T> ReadAsync<T>(CancellationToken ct);

        /// <summary>
        /// Write type T to stream
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="obj"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        Task WriteAsync<T>(T obj, CancellationToken ct);

        /// <summary>
        /// Read type T from stream.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <returns></returns>
        T Read<T>();

        /// <summary>
        /// Write type T to stream
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="obj"></param>
        /// <returns></returns>
        void Write<T>(T obj);
    }

    /// <summary>
    /// Non generic version
    /// </summary>
    public interface ICodecStream : ICodecStream<Stream> { }
}