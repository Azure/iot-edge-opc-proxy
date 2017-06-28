// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Text;
    using System.Threading.Tasks;

    public static class UtilsExtensions {
        public static Task<Byte[]> ToBytes(this Stream stream) {
            byte[] bytes = new byte[stream.Length];
            int read, current = 0;
            while ((read = stream.Read(bytes, current, bytes.Length - current)) > 0) {
                current += read;
            }

            return Task.FromResult(bytes);
        }

        private static readonly Random rng = new Random();
        public static void Shuffle<T>(this IList<T> list) {
            int n = list.Count;
            while (n > 1) {
                n--;
                int k = rng.Next(n + 1);
                T value = list[k];
                list[k] = list[n];
                list[n] = value;
            }
        }

        public static void AddRange<T>(this ISet<T> set, IEnumerable<T> enumerable) {
            foreach (var item in enumerable) {
                set.Add(item);
            }
        }

        public static IEnumerable<T> AsEnumerable<T>(this T obj) {
            yield return obj;
        }

        public static bool SameAs<T>(this IEnumerable<T> enumerable1, IEnumerable<T> enumerable2) {
            return new HashSet<T>(enumerable1).SetEquals(enumerable2);
        }

        public static string GetCombinedExceptionMessage(this AggregateException ae) {
            StringBuilder sb = new StringBuilder();
            foreach (Exception e in ae.InnerExceptions) {
                sb.AppendLine(string.Concat("E: ", e.Message));
            }

            return sb.ToString();
        }

        public static string GetCombinedExceptionStackTrace(this AggregateException ae) {
            StringBuilder sb = new StringBuilder();
            foreach (Exception e in ae.InnerExceptions) {
                sb.AppendLine(string.Concat("StackTrace: ", e.StackTrace));
            }

            return sb.ToString();
        }

        public static SocketError GetSocketError(this Exception ex) {
            SocketError error = SocketError.Fatal;
            if (ex is SocketException) {
                return ((SocketException)ex).Error;
            }
            else if (ex is AggregateException) {
                foreach (Exception e in ((AggregateException)ex).InnerExceptions) {
                    error = e.GetSocketError();
                    if (error != SocketError.Fatal)
                        break;
                }
            }
            return error;
        }
    }
}