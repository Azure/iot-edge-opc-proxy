// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {

    using System;
    using System.Collections.Concurrent;

    /// <summary>
    /// Helper class to cache delegates. 
    /// </summary>
    public class ResultCache {

        /// <summary>
        /// Concrete func translation to cached func
        /// </summary>
        private readonly ConcurrentDictionary<Delegate, Delegate> funcs
            = new ConcurrentDictionary<Delegate, Delegate>();

        /// <summary>
        /// Clear all funcs
        /// </summary>
        public void Invalidate() =>
            funcs.Clear();

        private static Func<T1, TResult> Cached<T1, TResult>(
          Func<T1, TResult> func) {
            var cache = new ConcurrentDictionary<T1, TResult>();
            return (arg) => cache.GetOrAdd(arg, func(arg));
        }

        /// <summary>
        /// Call method
        /// </summary>
        /// <typeparam name="T1"></typeparam>
        /// <typeparam name="TResult"></typeparam>
        /// <param name="func"></param>
        /// <param name="arg"></param>
        /// <returns></returns>
        public TResult Call<T1, TResult>(Func<T1, TResult> func, T1 arg) =>
            ((Func<T1, TResult>)funcs.GetOrAdd(func, Cached(func)))(arg);
        

        /// <summary>
        /// Invalidate results
        /// </summary>
        /// <typeparam name="T1"></typeparam>
        /// <typeparam name="TResult"></typeparam>
        /// <param name="func"></param>
        public void Invalidate<T1, TResult>(Func<T1, TResult> func) =>
            funcs.TryRemove(func, out tmp);


        private static Func<T1, T2, TResult> Cached<T1, T2, TResult>(Func<T1, T2, TResult> func) {
            var cache = new ConcurrentDictionary<Tuple<T1, T2>, TResult>();
            return (arg1, arg2) => cache.GetOrAdd(
                Tuple.Create(arg1, arg2), ck => func(arg1, arg2));
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
        /// <returns></returns>
        public TResult Call<T1, T2, TResult>(Func<T1, T2, TResult> func, T1 arg1, T2 arg2) =>
            ((Func<T1, T2, TResult>)funcs.GetOrAdd(func, Cached(func)))(arg1, arg2);
        
        /// <summary>
        /// Invalidate method results
        /// </summary>
        /// <typeparam name="T1"></typeparam>
        /// <typeparam name="T2"></typeparam>
        /// <typeparam name="TResult"></typeparam>
        /// <param name="func"></param>
        public void Invalidate<T1, T2, TResult>(Func<T1, T2, TResult> func) =>
            funcs.TryRemove(func, out tmp);


        private static Func<T1, T2, T3, TResult> Cached<T1, T2, T3, TResult>(
            Func<T1, T2, T3, TResult> func) {
            var cache = new ConcurrentDictionary<Tuple<T1, T2, T3>, TResult>();
            return (arg1, arg2, arg3) => cache.GetOrAdd(
                Tuple.Create(arg1, arg2, arg3), ck => func(arg1, arg2, arg3));
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
        /// <returns></returns>
        public TResult Call<T1, T2, T3, TResult>(Func<T1, T2, T3, TResult> func, T1 arg1, T2 arg2, T3 arg3) =>
            ((Func<T1, T2, T3, TResult>)funcs.GetOrAdd(func, Cached(func)))(arg1, arg2, arg3);

        /// <summary>
        /// Invalidate method results
        /// </summary>
        /// <typeparam name="T1"></typeparam>
        /// <typeparam name="T2"></typeparam>
        /// <typeparam name="T3"></typeparam>
        /// <typeparam name="TResult"></typeparam>
        /// <param name="func"></param>
        public void Invalidate<T1, T2, T3, TResult>(Func<T1, T2, T3, TResult> func) =>
            funcs.TryRemove(func, out tmp);

        private static Func<T1, T2, T3, T4, TResult> Cached<T1, T2, T3, T4, TResult>(
                Func<T1, T2, T3, T4, TResult> func) {
            var cache = new ConcurrentDictionary<Tuple<T1, T2, T3, T4>, TResult>();
            return (arg1, arg2, arg3, arg4) => cache.GetOrAdd(
                Tuple.Create(arg1, arg2, arg3, arg4), ck => func(arg1, arg2, arg3, arg4));
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
        /// <returns></returns>
        public TResult Call<T1, T2, T3, T4, TResult>(Func<T1, T2, T3, T4, TResult> func, T1 arg1, T2 arg2, T3 arg3, T4 arg4) =>
            ((Func<T1, T2, T3, T4, TResult>)funcs.GetOrAdd(func, Cached(func)))(arg1, arg2, arg3, arg4);

        /// <summary>
        /// Invalidate method results
        /// </summary>
        /// <typeparam name="T1"></typeparam>
        /// <typeparam name="T2"></typeparam>
        /// <typeparam name="T3"></typeparam>
        /// <typeparam name="T4"></typeparam>
        /// <typeparam name="TResult"></typeparam>
        /// <param name="func"></param>
        public void Invalidate<T1, T2, T3, T4, TResult>(Func<T1, T2, T3, T4, TResult> func) =>
            funcs.TryRemove(func, out tmp);

        private static Func<T1, T2, T3, T4, T5, TResult> Cached<T1, T2, T3, T4, T5, TResult>(
                Func<T1, T2, T3, T4, T5, TResult> func) {
            var cache = new ConcurrentDictionary<Tuple<T1, T2, T3, T4, T5>, TResult>();
            return (arg1, arg2, arg3, arg4, arg5) => cache.GetOrAdd(
                Tuple.Create(arg1, arg2, arg3, arg4, arg5), ck => func(arg1, arg2, arg3, arg4, arg5));
        }

        /// <summary>
        /// Call method
        /// </summary>
        /// <typeparam name="T1"></typeparam>
        /// <typeparam name="T2"></typeparam>
        /// <typeparam name="T3"></typeparam>
        /// <typeparam name="T4"></typeparam>
        /// <typeparam name="T5"></typeparam>
        /// <typeparam name="TResult"></typeparam>
        /// <param name="func"></param>
        /// <param name="arg1"></param>
        /// <param name="arg2"></param>
        /// <param name="arg3"></param>
        /// <param name="arg4"></param>
        /// <param name="arg5"></param>
        /// <returns></returns>
        public TResult Call<T1, T2, T3, T4, T5, TResult>(Func<T1, T2, T3, T4, T5, TResult> func, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5) =>
            ((Func<T1, T2, T3, T4, T5, TResult>)funcs.GetOrAdd(func, Cached(func)))(arg1, arg2, arg3, arg4, arg5);

        /// <summary>
        /// Invalidate method results
        /// </summary>
        /// <typeparam name="T1"></typeparam>
        /// <typeparam name="T2"></typeparam>
        /// <typeparam name="T3"></typeparam>
        /// <typeparam name="T4"></typeparam>
        /// <typeparam name="T5"></typeparam>
        /// <typeparam name="TResult"></typeparam>
        /// <param name="func"></param>
        public void Invalidate<T1, T2, T3, T4, T5, TResult>(Func<T1, T2, T3, T4, T5, TResult> func) =>
            funcs.TryRemove(func, out tmp);

        Delegate tmp;
    }
}
