// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Threading.Tasks;
    using System.Threading;

    /// <summary>
    /// Retry helper class with different retry policies
    /// </summary>
    internal static class Retry {

        private static readonly object Semaphore = new object();

        public static int MaxRetryCount = 1000;
        public static int MaxRetryDelay = 3 * 60 * 1000; // 3 minutes
        public static int BackoffDelta = 3;


        /// <summary>
        /// Default exponential policy with 20% jitter
        /// </summary>
        public static Func<int, int> Exponential {
            get {
                return (k) => {
                    Random r = new Random();
                    int increment = (int)((Math.Pow(2, k) - 1) *
                        r.Next((int)(BackoffDelta * 0.8), (int)(BackoffDelta * 1.2)));
                    return (int)Math.Min(increment, MaxRetryDelay);
                };
            }
        }

        /// <summary>
        /// Default linear policy
        /// </summary>
        public static Func<int, int> Linear {
            get {
                return (k) => k * 2 * BackoffDelta;
            }
        }

        /// <summary>
        /// No backoff - just wait backoff delta
        /// </summary>
        public static Func<int, int> NoBackoff {
            get {
                return (k) => BackoffDelta;
            }
        }

        /// <summary>
        /// Retries a piece of work
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="work"></param>
        /// <param name="cont"></param>
        /// <param name="backoff"></param>
        /// <param name="maxRetry"></param>
        /// <param name="tracer"></param>
        /// <returns></returns>
        public static async Task Do(CancellationToken ct, Func<Task> work, 
            Func<Exception, bool> cont, Func<int, int> policy, int maxRetry) {
            for (int k = 1; k <= maxRetry; k++) {
                if (ct.IsCancellationRequested)
                    throw new TaskCanceledException();
                try {
                    await work();
                    return;
                }
                catch (Exception ex) {
                    if (!cont(ex))
                        throw ProxyEventSource.Log.Rethrow(ex, work); 
                    ProxyEventSource.Log.Retry(work, k, ex);
                }
                int delay = policy(k);
                if (delay == 0)
                    continue;
                await Task.Delay(delay);
            }
            throw ProxyEventSource.Log.Timeout("Retry timeout exhausted.");
        }

        /// <summary>
        /// Retries a piece of work with return type
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="work"></param>
        /// <param name="cont"></param>
        /// <param name="backoff"></param>
        /// <param name="maxRetry"></param>
        /// <param name="tracer"></param>
        /// <returns></returns>
        public static async Task<T> Do<T>(CancellationToken ct, Func<Task<T>> work,
            Func<Exception, bool> cont, Func<int, int> policy, int maxRetry) {
            for (int k = 1; k <= maxRetry; k++) {
                if (ct.IsCancellationRequested)
                    throw new TaskCanceledException();
                try {
                    return await work();
                }
                catch (Exception ex) {
                    if (!cont(ex))
                        throw ProxyEventSource.Log.Rethrow(ex, work);
                    ProxyEventSource.Log.Retry(work, k, ex);
                }
                int delay = policy(k);
                if (delay == 0)
                    continue;
                await Task.Delay(delay);
            }
            throw ProxyEventSource.Log.Timeout("Retry timeout exhausted.");
        }

        /// <summary>
        /// Retry with linear backoff
        /// </summary>
        /// <param name="work"></param>
        /// <param name="cont"></param>
        /// <param name="maxRetry"></param>
        /// <param name="tracer"></param>
        /// <returns></returns>
        public static Task WithLinearBackoff(CancellationToken ct,
            Func<Task> work, Func<Exception, bool> cont) {
            return Do(ct, work, cont, Linear, MaxRetryCount);
        }

        /// <summary>
        /// Retry with linear backoff
        /// </summary>
        /// <param name="work"></param>
        /// <param name="tracer"></param>
        /// <returns></returns>
        public static Task WithLinearBackoff(CancellationToken ct, 
            Func<Task> work) {
            return WithLinearBackoff(ct, work, (ex) => ex is ITransientException);
        }

        /// <summary>
        /// Retry with linear backoff
        /// </summary>
        /// <param name="work"></param>
        /// <param name="tracer"></param>
        /// <returns></returns>
        public static Task WithLinearBackoff(Func<Task> work) {
            return WithLinearBackoff(CancellationToken.None, work);
        }

        /// <summary>
        /// Retry with linear backoff
        /// </summary>
        /// <param name="work"></param>
        /// <param name="cont"></param>
        /// <param name="maxRetry"></param>
        /// <param name="tracer"></param>
        /// <returns></returns>
        public static Task<T> WithLinearBackoff<T>(CancellationToken ct,
            Func<Task<T>> work, Func<Exception, bool> cont) {
            return Do(ct, work, cont, Linear, MaxRetryCount);
        }

        /// <summary>
        /// Retry with linear backoff
        /// </summary>
        /// <param name="work"></param>
        /// <param name="tracer"></param>
        /// <returns></returns>
        public static Task<T> WithLinearBackoff<T>(CancellationToken ct,
            Func<Task<T>> work) {
            return WithLinearBackoff(ct, work, (ex) => ex is ITransientException);
        }

        /// <summary>
        /// Retry with linear backoff
        /// </summary>
        /// <param name="work"></param>
        /// <param name="tracer"></param>
        /// <returns></returns>
        public static Task<T> WithLinearBackoff<T>(Func<Task<T>> work) {
            return WithLinearBackoff(CancellationToken.None, work);
        }

        /// <summary>
        /// Retry with exponential backoff
        /// </summary>
        /// <param name="work"></param>
        /// <param name="cont"></param>
        /// <param name="tracer"></param>
        /// <returns></returns>
        public static Task WithExponentialBackoff(CancellationToken ct,
            Func<Task> work,
            Func<Exception, bool> cont) {
            return Do(ct, work, cont, Exponential, MaxRetryCount);
        }

        /// <summary>
        /// Retry with exponential backoff
        /// </summary>
        /// <param name="work"></param>
        /// <param name="tracer"></param>
        /// <returns></returns>
        public static Task WithExponentialBackoff(CancellationToken ct,
            Func<Task> work) {
            return WithExponentialBackoff(ct, work, (ex) => ex is ITransientException);
        }

        /// <summary>
        /// Retry with exponential backoff
        /// </summary>
        /// <param name="work"></param>
        /// <param name="tracer"></param>
        /// <returns></returns>
        public static Task WithExponentialBackoff(Func<Task> work) {
            return WithExponentialBackoff(CancellationToken.None, work);
        }

        /// <summary>
        /// Retry with exponential backoff
        /// </summary>
        /// <param name="work"></param>
        /// <param name="cont"></param>
        /// <param name="tracer"></param>
        /// <returns></returns>
        public static Task<T> WithExponentialBackoff<T>(CancellationToken ct,
            Func<Task<T>> work, Func<Exception, bool> cont) {
            return Do(ct, work, cont, Exponential, MaxRetryCount);
        }

        /// <summary>
        /// Retry with exponential backoff
        /// </summary>
        /// <param name="work"></param>
        /// <param name="tracer"></param>
        /// <returns></returns>
        public static Task<T> WithExponentialBackoff<T>(CancellationToken ct,
            Func<Task<T>> work) {
            return WithExponentialBackoff(ct, work, (ex) => ex is ITransientException);
        }

        /// <summary>
        /// Retry with exponential backoff
        /// </summary>
        /// <param name="work"></param>
        /// <param name="tracer"></param>
        /// <returns></returns>
        public static Task<T> WithExponentialBackoff<T>(Func<Task<T>> work) {
            return WithExponentialBackoff(CancellationToken.None, work);
        }
    }
}
