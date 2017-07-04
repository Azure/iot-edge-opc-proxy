// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Collections;
    using System.Collections.Concurrent;
    using System.Collections.Generic;

    /// <summary>
    /// Generic base class for any object in the object model.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    public abstract class Poco<T> : Poco, IEquatable<T> where T : Poco, new() {
        public bool Equals(T other) => IsEqual(other);

        public abstract bool IsEqual(T that);

        public override bool IsEqual(object that) {
            var val = that as T;
            return val == null ? false : IsEqual(val);
        }

        public override void Dispose() {
            ResetHash();
            _pool.Return(this);
        }

        public static T Get() => _pool.Get() as T;
        private static ObjectPool<Poco> _pool = new ObjectPool<Poco>(() => new T());
    }

    /// <summary>
    /// Utility base class for any object in the object model.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    public abstract class Poco : IDisposable {

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(object that) => IsEqual(that);

        /// <summary>
        /// Returns hash for efficient lookup in sets, etc.
        /// The hash is calculated lazily at first use (i.e. insertion
        /// into a map or set, etc.).  This is so that deserialized 
        /// objects can be constructed without the need to be finalized
        /// </summary>
        /// <returns>hash code</returns>
        public override int GetHashCode() {
            if (_hash == null) {
                lock (this) {
                    if (_hash == null) {
                        SetHashCode();
                        if (_hash == null) {
                            _hash = _offsetBasis;
                        }
                    }
                }
            }
            return (int)_hash;
        }

        /// <summary>
        /// Helper to mix a hash into the hash value
        /// </summary>
        /// <param name="hash"></param>
        protected void MixToHash(int hash) {
            if (hash == 0) {
                return;
            }
            _hash = ((_hash ?? _offsetBasis) ^ hash) * _prime;
        }

        /// <summary>
        /// Helper to mix an array of items into the hash value
        /// </summary>
        /// <param name="hash"></param>
        protected void MixToHash(IEnumerable enumeration) {
            foreach (var item in enumeration) {
                MixToHash(item);
            }
        }

        /// <summary>
        /// Helper to mix an items hash code
        /// </summary>
        /// <param name="hash"></param>
        protected void MixToHash(object item) {
            if (item is IEnumerable) {
                MixToHash((IEnumerable)item);
            }
            else if (item != null) {
                MixToHash(item.GetHashCode());
            }
        }

        /// <summary>
        /// Should never be needed, but for the case of re-use?
        /// </summary>
        protected void ResetHash() {
            _hash = null;
        }

        /// <summary>
        /// Safe compare two pocos
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="ziss"></param>
        /// <param name="zat"></param>
        /// <returns></returns>
        protected static bool IsEqual(Poco ziss, Poco zat) {
            if (ziss == null) {
                return (zat == null);
            }
            else if (zat != null) {
                if (ziss.GetHashCode() == zat.GetHashCode() &&
                    ziss.IsEqual(zat)) {
                    return true;
                }
            }
            return false;
        }

        /// <summary>
        /// Safe compare anything else
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="ziss"></param>
        /// <param name="zat"></param>
        /// <returns></returns>
        protected static bool IsEqual (object ziss, object zat) {
            if (ziss == null) {
                return (zat == null);
            }
            else if (zat != null) {
                if (ziss.GetHashCode() == zat.GetHashCode() &&
                    ziss.Equals(zat)) {
                    return true;
                }
            }
            return false;
        }

        /// <summary>
        /// Safe compare two enumerables
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="ziss"></param>
        /// <param name="zat"></param>
        /// <returns></returns>
        protected static bool IsEqual<T>(IEnumerable<T> ziss, IEnumerable<T> zat) {
            if (ziss == null) {
                return (zat == null);
            }
            else if (zat != null) {
                using (var zatEnumerator = zat.GetEnumerator()) {
                    foreach (var item in ziss) {
                        if (!zatEnumerator.MoveNext() ||
                            !IsEqual(item, zatEnumerator.Current)) {
                            return false;
                        }
                    }
                    if( !zatEnumerator.MoveNext()) {
                        return true;
                    }
                }
            }
            return false;
        }

        /// <summary>
        /// Safe compare
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="ziss"></param>
        /// <param name="zat"></param>
        /// <returns></returns>
        protected static bool IsEqual(object[] ziss, object[] zat) {
            if (ziss == null) {
                return (zat == null);
            }
            else if (zat != null && ziss.Length == zat.Length) {
                for(int i = 0; i < zat.Length; i++) {
                    if (!IsEqual(ziss[i], zat[i])) {
                        return false;
                    }
                }
                return true;
            }
            return false;
        }


        /// <summary>
        /// Called to set the objects hashcode
        /// </summary>
        protected abstract void SetHashCode();

        /// <summary>
        /// Must be implemented for equality
        /// </summary>
        /// <param name="other"></param>
        /// <returns></returns>
        public abstract bool IsEqual(object that);

        public abstract void Dispose();

        // FNV-1a hash
        private int? _hash = null;
        private static readonly int _offsetBasis = unchecked((int)2166136261);
        private static readonly int _prime = 16777619;
    }
}
