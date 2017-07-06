// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace System.Collections.Concurrent {

    /// <summary>
    /// Provides a thread-safe object pool.
    /// </summary>
    /// <typeparam name="T">
    /// Specifies the type of the elements stored in the pool.</typeparam>
    public sealed class ObjectPool<T>  {
        private readonly Func<T> _generator;

        /// <summary>
        /// Initializes an instance of the ObjectPool class.
        /// </summary>
        /// <param name="generator">The function used to create items </param>
        public ObjectPool(Func<T> generator) : 
            this(generator, new ConcurrentQueue<T>()) {
        }

        /// <summary>
        /// Initializes an instance of the ObjectPool class.
        /// </summary>
        /// <param name="generator">The function used to create items </param>
        /// <param name="container">The collection used to store elements </param>
        public ObjectPool(Func<T> generator, IProducerConsumerCollection<T> container) {
            _generator = generator ?? throw new ArgumentNullException("generator");
            _container = container ?? throw new ArgumentNullException("container");
        }

        /// <summary>
        /// Adds the provided item into the pool.
        /// </summary>
        /// <param name="item">The item to be added.</param>
        public void Return(T item) {
            _container.TryAdd(item);
        }

        /// <summary>
        /// Gets an item from the pool. If the pool is empty, a new item will 
        /// be created and returned.
        /// </summary>
        /// <returns>The removed or created item.</returns>
        public T Get() {
            return _container.TryTake(out T value) ? value : _generator();
        }

        private readonly IProducerConsumerCollection<T> _container;
    }
}