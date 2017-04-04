// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {

    using System;
    using System.Collections.Concurrent;
    using System.Threading;
    using System.Threading.Tasks;

    /// <summary>
    /// Helper class to cache delegates. 
    /// </summary>
    public class ResultCache {

        /// <summary>
        /// Concrete func translation to cached func
        /// </summary>
        private readonly ConcurrentDictionary<Delegate, Delegate> _funcs
            = new ConcurrentDictionary<Delegate, Delegate>();

        /// <summary>
        /// Clear all funcs
        /// </summary>
        public virtual void Invalidate() =>
            _funcs.Clear();

        /// <summary>
        /// Start invalidate timer
        /// </summary>
        /// <param name="period"></param>
        public void StartTimer(TimeSpan period) {
            if (_invalidateTimer != null) {
                _invalidateTimer.Change(period, period);
            }
            else {
                _invalidateTimer = new Timer(_ => Invalidate(), null, period, period);
            }
        }

        /// <summary>
        /// Stop invalidate timer
        /// </summary>
        public void StopTimer() {
            if (_invalidateTimer != null) {
                _invalidateTimer.Dispose();
                _invalidateTimer = null;
            }
        }

        /// <summary>
        /// Call method
        /// </summary>
        /// <typeparam name="T1"></typeparam>
        /// <typeparam name="TResult"></typeparam>
        /// <param name="func"></param>
        /// <param name="arg"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public Task<TResult> Call<T1, TResult>(Func<T1, CancellationToken, Task<TResult>> func, T1 arg, CancellationToken ct) =>
            ((Func<T1, CancellationToken, Task<TResult>>)_funcs.GetOrAdd(func, Cached(func)))(arg, ct);
        private static Func<T1, CancellationToken, Task<TResult>> Cached<T1, TResult>(
            Func<T1, CancellationToken, Task<TResult>> func) {
            var cache = new ConcurrentDictionary<T1, Task<TResult>>();
            return (arg, ct) => {
                var k = arg;
                var t = cache.GetOrAdd(k, ck => func(ck, ct));
                if (t.IsCompleted) {
                    return t;
                }
                cache.TryRemove(k, out t);
                ct.ThrowIfCancellationRequested();
                return cache.GetOrAdd(k, ck => func(ck, ct));
            };
        }

        /// <summary>
        /// Call method
        /// </summary>
        /// <typeparam name="T1"></typeparam>
        /// <typeparam name="T2"></typeparam>
        /// <typeparam name="TResult"></typeparam>
        /// <param name="func"></param>
        /// <param name="arg1"></param>
        /// <param name="arg2"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public Task<TResult> Call<T1, T2, TResult>(Func<T1, T2, CancellationToken, Task<TResult>> func, T1 arg1, T2 arg2, CancellationToken ct) =>
            ((Func<T1, T2, CancellationToken, Task<TResult>>)_funcs.GetOrAdd(func, Cached(func)))(arg1, arg2, ct);
        private static Func<T1, T2, CancellationToken, Task<TResult>> Cached<T1, T2, TResult>(
            Func<T1, T2, CancellationToken, Task<TResult>> func) {
            var cache = new ConcurrentDictionary<Tuple<T1, T2>, Task<TResult>>();
            return (arg1, arg2, ct) => {
                var k = Tuple.Create(arg1, arg2);
                var t = cache.GetOrAdd(k, ck =>  func(ck.Item1, ck.Item2, ct));
                if (t.IsCompleted) {
                    return t;
                }
                cache.TryRemove(k, out t);
                ct.ThrowIfCancellationRequested();
                return cache.GetOrAdd(k, ck => func(ck.Item1, ck.Item2, ct));
            };
        }

        /// <summary>
        /// Call method
        /// </summary>
        /// <typeparam name="T1"></typeparam>
        /// <typeparam name="T2"></typeparam>
        /// <typeparam name="T3"></typeparam>
        /// <typeparam name="TResult"></typeparam>
        /// <param name="func"></param>
        /// <param name="arg1"></param>
        /// <param name="arg2"></param>
        /// <param name="arg3"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public Task<TResult> Call<T1, T2, T3, TResult>(Func<T1, T2, T3, CancellationToken, Task<TResult>> func, T1 arg1, T2 arg2, T3 arg3, CancellationToken ct) =>
            ((Func<T1, T2, T3, CancellationToken, Task<TResult>>)_funcs.GetOrAdd(func, Cached(func)))(arg1, arg2, arg3, ct);
        private static Func<T1, T2, T3, CancellationToken, Task<TResult>> Cached<T1, T2, T3, TResult>(
            Func<T1, T2, T3, CancellationToken, Task<TResult>> func) {
            var cache = new ConcurrentDictionary<Tuple<T1, T2, T3>, Task<TResult>>();
            return (arg1, arg2, arg3, ct) => {
                var k = Tuple.Create(arg1, arg2, arg3);
                var t = cache.GetOrAdd(k, ck => func(ck.Item1, ck.Item2, ck.Item3, ct));
                if (t.IsCompleted) {
                    return t;
                }
                cache.TryRemove(k, out t);
                ct.ThrowIfCancellationRequested();
                return cache.GetOrAdd(k, ck => func(ck.Item1, ck.Item2, ck.Item3, ct));
            };
        }

        /// <summary>
        /// Call method
        /// </summary>
        /// <typeparam name="T1"></typeparam>
        /// <typeparam name="T2"></typeparam>
        /// <typeparam name="T3"></typeparam>
        /// <typeparam name="T4"></typeparam>
        /// <typeparam name="TResult"></typeparam>
        /// <param name="func"></param>
        /// <param name="arg1"></param>
        /// <param name="arg2"></param>
        /// <param name="arg3"></param>
        /// <param name="arg4"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public Task<TResult> Call<T1, T2, T3, T4, TResult>(Func<T1, T2, T3, T4, CancellationToken, Task<TResult>> func, T1 arg1, T2 arg2, T3 arg3, T4 arg4, CancellationToken ct) =>
            ((Func<T1, T2, T3, T4, CancellationToken, Task<TResult>>)_funcs.GetOrAdd(func, Cached(func)))(arg1, arg2, arg3, arg4, ct);
        private static Func<T1, T2, T3, T4, CancellationToken, Task<TResult>> Cached<T1, T2, T3, T4, TResult>(
            Func<T1, T2, T3, T4, CancellationToken, Task<TResult>> func) {
            var cache = new ConcurrentDictionary<Tuple<T1, T2, T3, T4>, Task<TResult>>();
            return (arg1, arg2, arg3, arg4, ct) => {
                var k = Tuple.Create(arg1, arg2, arg3, arg4);
                var t = cache.GetOrAdd(k, ck => func(ck.Item1, ck.Item2, ck.Item3, ck.Item4, ct));
                if (t.IsCompleted) {
                    return t;
                }
                cache.TryRemove(k, out t);
                ct.ThrowIfCancellationRequested();
                return cache.GetOrAdd(k, ck => func(ck.Item1, ck.Item2, ck.Item3, ck.Item4, ct));
            };
        }

        private Timer _invalidateTimer;
    }
}
