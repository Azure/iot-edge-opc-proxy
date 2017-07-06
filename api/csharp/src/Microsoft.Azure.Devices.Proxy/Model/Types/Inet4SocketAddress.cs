// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Runtime.Serialization;
    /// <summary>
    /// IPv4 socket address, base class
    /// </summary>
    [DataContract]
    public class Inet4SocketAddress : InetSocketAddress, IEquatable<Inet4SocketAddress> {

        [DataMember(Name = "family", Order = 1)]
        public override AddressFamily Family => AddressFamily.InterNetwork;

        /// <summary>
        /// Returns the 32 bit address as a buffer
        /// </summary>
        [DataMember(Name = "addr", Order = 3)]
        public byte[] Address {
            get; set;
        }

        /// <summary>
        /// Default constructor
        /// </summary>
        public Inet4SocketAddress() {
        }

        /// <summary>
        /// Sets property values
        /// </summary>
        public Inet4SocketAddress(uint address, ushort port) :
            this(BitConverter.GetBytes(address), port) {
        }

        /// <summary>
        /// Sets property values
        /// </summary>
        public Inet4SocketAddress(byte[] address, ushort port) : this() {
            Address = address;
            Port = port;
        }

        /// <summary>
        /// Returns address as 32 bit int
        /// </summary>
        /// <returns></returns>
        public uint AsUInt32() => BitConverter.ToUInt32(Address, 0);

        /// <summary>
        /// Stringify address
        /// </summary>
        /// <returns></returns>
        public string AsString() => 
            $"{Address[0]}.{Address[1]}.{Address[2]}.{Address[3]}";

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(Inet4SocketAddress that) {
            if (that == null) {
                return false;
            }
            return
                IsEqual(that as InetSocketAddress) &&
                IsEqual(AsUInt32(), that.AsUInt32());
        }

        public override bool IsEqual(object that) => Equals(that as Inet4SocketAddress);

        protected override void SetHashCode() {
            base.SetHashCode();
            MixToHash(AsUInt32());
        }

        /// <summary>
        /// Stringify address
        /// </summary>
        /// <returns></returns>
        public override string ToString() {
            return AsString() + $":{Port}";
        }

        /// <summary>
        /// Parse a string into a inet 4 address
        /// </summary>
        /// <param name="address"></param>
        /// <param name="parsed"></param>
        /// <returns></returns>
        public static bool TryParse(string address, out Inet4SocketAddress parsed) {
            var labels = address.Split(':');
            if (labels.Length >= 1) {
                var bytes = labels[0].Split('.');
                if (bytes.Length == 4) {
                    var addressBuffer = new byte[4];
                    if (byte.TryParse(bytes[0], out addressBuffer[0]) &&
                        byte.TryParse(bytes[1], out addressBuffer[1]) &&
                        byte.TryParse(bytes[2], out addressBuffer[2]) &&
                        byte.TryParse(bytes[3], out addressBuffer[3])) {

                        // Now parse port if we have one, otherwise set to 0.
                        ushort port = 0;
                        if (labels.Length == 1 ||
                           (labels.Length == 2 && 
                            ushort.TryParse(labels[1], out port))) {
                            parsed = new Inet4SocketAddress(addressBuffer, port);
                            return true;
                        }
                    }
                }
            }
            parsed = null;
            return false;
        }

        public override ProxySocketAddress AsProxySocketAddress() =>
            new ProxySocketAddress(AsString(), Port);
    }
}