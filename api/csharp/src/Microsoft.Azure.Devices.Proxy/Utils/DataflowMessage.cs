// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace System.Threading.Tasks.Dataflow {
    using System;
    using System.Collections.Concurrent;
    using System.Collections.Generic;

    /// <summary>
    /// A dataflow block input with timeout and last exception
    /// </summary>
    public class DataflowMessage<T> : IDisposable {

        /// <summary> The message input or output content </summary>
        public T Arg { get; set; }

        /// <summary> Exceptions that occurred during processing </summary>
        public LinkedList<Exception> Exceptions { get; set; } = new LinkedList<Exception>();

        public override string ToString() => $"{Arg} ({Exceptions.Count})";

        /// <summary>
        /// Helper to reset the message
        /// </summary>
        /// <param name="arg"></param>
        /// <returns></returns>
        internal DataflowMessage<T> Reset(T arg) {
            Arg = arg;
            return this;
        }

        /// <summary>
        /// Call dispose to return the message back to the pool when done.
        /// </summary>
        public void Dispose() {
            Exceptions.Clear();
            Arg = default(T);
            _pool.Return(this);
        }

        /// <summary>
        /// Create an adapter to change T into a dataflow message
        /// </summary>
        /// <returns>The adapter block</returns>
        public static IPropagatorBlock<T, DataflowMessage<T>> CreateAdapter() =>
            new TransformBlock<T, DataflowMessage<T>>(arg => _pool.Get().Reset(arg));

        /// <summary>
        /// Create an adapter to change T into a dataflow message
        /// </summary>
        /// <param name="options">Execution options for the returned block</param>
        /// <returns>The adapter block</returns>
        public static IPropagatorBlock<T, DataflowMessage<T>> CreateAdapter(ExecutionDataflowBlockOptions options) =>
            new TransformBlock<T, DataflowMessage<T>>(arg => _pool.Get().Reset(arg), options);


        private static ObjectPool<DataflowMessage<T>> _pool = 
            new ObjectPool<DataflowMessage<T>>(() => new DataflowMessage<T>());
    }
}