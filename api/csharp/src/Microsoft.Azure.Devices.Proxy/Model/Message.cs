// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy.Model {
    using System;
    using System.Runtime.Serialization;

    /// <summary>
    /// A serializable message sent between proxy servers and clients
    /// </summary>
    [DataContract]
    public class Message : Serializable<Message>, IEquatable<Message> {

        /// <summary>
        /// Version validation field
        /// </summary>
        [DataMember(Name = "version", Order = 1)]
        public UInt16 Version { get; set; } = 0x5;

        /// <summary>
        /// Source address
        /// </summary>
        [DataMember(Name = "source_id", Order = 2)]
        public Reference Source { get; set; }

        /// <summary>
        /// Proxy address
        /// </summary>
        [DataMember(Name = "proxy_id", Order = 3)]
        public Reference Proxy { get; set; }

        /// <summary>
        /// Target object 
        /// </summary>
        [DataMember(Name = "target_id", Order = 4)]
        public Reference Target { get; set; }


        /// <summary>
        /// Error code if this is an error message
        /// </summary>
        [DataMember(Name = "error_code", Order = 5)]
        public Int32 Error { get; set; }

        /// <summary>
        /// Whether the message is response to a request
        /// </summary>
        [DataMember(Name = "is_response", Order = 6)]
        public bool IsResponse { get; set; }

        /// <summary>
        /// Content type
        /// </summary>
        [DataMember(Name = "type", Order = 7)]
        public UInt32 TypeId { get; set; }

        /// <summary>
        /// Content type
        /// </summary>
        [DataMember(Name = "content", Order = 8)]
        public IMessageContent Content { get; set; }
        public string DeviceId { get; internal set; }

        /// <summary>
        /// Create an empty message 
        /// </summary>
        public Message() {
        }

        /// <summary>
        /// Create message with specific buffer
        /// </summary>
        /// <param name="buf"></param>
        public Message(Reference source, Reference target, Reference proxy, IMessageContent content) : this() {
            if (content == null) throw new ArgumentException("content was null");
            Content = content;
            TypeId = MessageContent.GetId(content);
            IsResponse = MessageContent.IsResponse(content);
            Source = source ?? new Reference();
            Target = target ?? new Reference();
            Proxy = proxy ?? new Reference();
        }

        /// <summary>
        /// Create message with specific buffer
        /// </summary>
        /// <param name="buf"></param>
        public Message(Reference source, Reference target, IMessageContent content) : 
            this(source, target, Reference.Null, content) {
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="obj"></param>
        /// <returns></returns>
        public bool Equals(Message msg) {
            if (msg == null) {
                return false;
            }
            return
                this.Error.Equals(msg.Error) &&
                this.IsResponse.Equals(msg.IsResponse) &&
                this.TypeId.Equals(msg.TypeId) &&
                this.Target.Equals(msg.Target) &&
                this.Proxy.Equals(msg.Proxy) &&
                this.Source.Equals(msg.Source) &&
                this.Version == msg.Version && 
                this.Content.Equals(msg.Content);
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(Object that) {
            return Equals(that as Message);
        }

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return
                Content.GetHashCode();
        }
    }
}
