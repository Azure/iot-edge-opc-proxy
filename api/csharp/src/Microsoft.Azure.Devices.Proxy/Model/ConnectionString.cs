// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy.Model {
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Runtime.Serialization;
    using System.Text;

    /// <summary>
    /// Connection string
    /// </summary>
    [DataContract]
    public class ConnectionString : IEquatable<ConnectionString> {

        public enum Id {
            HostName,
            DeviceId,
            SharedAccessKeyName,
            SharedAccessKey,
            Endpoint,
            ConsumerGroup,
            PartitionCount,
            Entity,
            EndpointName,
            SharedAccessToken
        }

        /// <summary>
        /// All items to be serialized
        /// </summary>
        [DataMember (Order = 1)]
        public Dictionary<Id, string> items;

        /// <summary>
        /// Get hub name from connection string
        /// </summary>
        public string HubName {
            get {
                int idx = HostName.IndexOf('.');
                if (idx == -1)
                    throw new InvalidDataContractException("No hub name");
                return HostName.Substring(idx);
            }
        }

        /// <summary>
        /// Get host name from connection string
        /// </summary>
        public string HostName {
            get {
                return this[Id.HostName];
            }
        }

        /// <summary>
        /// Get device id
        /// </summary>
        public string DeviceId {
            get {
                return this[Id.DeviceId];
            }
        }

        /// <summary>
        /// Get shared access key name
        /// </summary>
        public string SharedAccessKeyName {
            get {
                return this[Id.SharedAccessKeyName];
            }
        }

        /// <summary>
        /// Get shared access key
        /// </summary>
        public string SharedAccessKey {
            get {
                return this[Id.SharedAccessKey];
            }
        }

        /// <summary>
        /// Get shared access key
        /// </summary>
        public string SharedAccessToken {
            get {
                return this[Id.SharedAccessToken];
            }
        }

        /// <summary>
        /// Get Endpoint address
        /// </summary>
        public Uri Endpoint {
            get {
                return new Uri(this[Id.Endpoint]);
            }
        }

        /// <summary>
        /// Get Endpoint address
        /// </summary>
        public string Entity {
            get {
                return this[Id.Entity];
            }
        }

        /// <summary>
        /// Indexer
        /// </summary>
        /// <param name="id"></param>
        /// <returns></returns>
        private string this [Id id] {
            get {
                string value;
                if (!items.TryGetValue(id, out value))
                    return null;
                return value;
            }
            set {
                items.Add(id, value);
            }
        }

        /// <summary>
        /// Create connection string
        /// </summary>
        public ConnectionString() {
            items = new Dictionary<Id, string>();
        }

        /// <summary>
        /// Create connection string
        /// </summary>
        public ConnectionString(string host, string deviceId, string keyName, string key) 
            : this() {
            items[Id.HostName] = host;
            items[Id.DeviceId] = deviceId;
            items[Id.SharedAccessKeyName] = keyName;
            items[Id.SharedAccessKey] = key;
        }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="endpoint"></param>
        /// <param name="keyName"></param>
        /// <param name="token"></param>
        public ConnectionString(Uri endpoint, string keyName, string token)
            : this() {
            items[Id.HostName] = endpoint.DnsSafeHost;
            items[Id.EndpointName] = endpoint.AbsolutePath.TrimStart('/');
            items[Id.SharedAccessKeyName] = keyName;
            items[Id.SharedAccessToken] = token;
        }

        /// <summary>
        /// Parse connection string
        /// </summary>
        /// <param name="connectionString"></param>
        /// <returns></returns>
        public static ConnectionString Parse(string connectionString) {
            if (connectionString == null)
                throw new ArgumentException("Connection string must be non null");
            ConnectionString cs = new ConnectionString();
            foreach (var elem in connectionString.Split(';')) {
                int i = elem.IndexOf("=");
                if (i < 0)
                    throw new InvalidDataContractException("Bad key value pair.");
                // Throws argument if already exists or parse fails...
                cs.items.Add((Id)Enum.Parse(typeof(Id), elem.Substring(0, i), true), elem.Substring(i + 1));
            }
            return cs;
        }

        /// <summary>
        /// Converts to string
        /// </summary>
        /// <returns></returns>
        public override string ToString() {
            StringBuilder b = new StringBuilder();
            foreach (var kv in items) {
                b.Append(kv.Key.ToString());
                b.Append("=");
                b.Append(kv.Value.ToString());
                b.Append(";");
            }
            return b.ToString().TrimEnd(';');
        }

        /// <summary>
        /// Equality compare
        /// </summary>
        /// <param name="obj"></param>
        /// <returns></returns>
        public override bool Equals(object obj) {
            return Equals(obj as ConnectionString);
        }

        /// <summary>
        /// Returns hash code
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return items.GetHashCode();
        }

        /// <summary>
        /// Type safe compare
        /// </summary>
        /// <param name="other"></param>
        /// <returns></returns>
        public bool Equals(ConnectionString other) {
            if (other == null)
                return false;
            return items.SequenceEqual(other.items);
        }
    }
}
