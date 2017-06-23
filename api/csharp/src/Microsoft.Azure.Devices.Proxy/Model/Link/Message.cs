// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Runtime.Serialization;

    /// <summary>
    /// A serializable message sent between proxy servers and clients
    /// </summary>
    [DataContract]
    public class Message : Serializable<Message> {

        /// <summary>
        /// Version validation field
        /// </summary>
        [DataMember(Name = "version", Order = 1)]
        public ushort Version { get; set; } = (ushort)
            ((VersionEx.Assembly.Major << 6) | (VersionEx.Assembly.Minor + 5));

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
        public int Error { get; set; }

        /// <summary>
        /// Whether the message is response to a request
        /// </summary>
        [DataMember(Name = "is_response", Order = 6)]
        public bool IsResponse { get; set; }

        /// <summary>
        /// Content type
        /// </summary>
        [DataMember(Name = "type", Order = 7)]
        public uint TypeId { get; set; }

        /// <summary>
        /// Content type
        /// </summary>
        [DataMember(Name = "content", Order = 8)]
        public IMessageContent Content { get; set; }

        internal string DeviceId { get; set; }

        /// <summary>
        /// Create an empty message 
        /// </summary>
        public Message() {
        }

        /// <summary>
        /// Create a shallow clone - does not include content
        /// </summary>
        /// <param name="message"></param>
        internal Message(Message message) : 
            this(message.Source, message.Target, message.Proxy, message.Content) {
            DeviceId = message.DeviceId;
        }

        /// <summary>
        /// Create message with specific buffer
        /// </summary>
        /// <param name="buf"></param>
        public Message(Reference source, Reference target, IMessageContent content) :
            this(source, target, Reference.Null, content) {
        }

        /// <summary>
        /// Create message with specific buffer
        /// </summary>
        /// <param name="buf"></param>
        public Message(Reference source, Reference target, Reference proxy, IMessageContent content) : this() {
            Content = content ?? throw new ArgumentException("content was null");
            TypeId = MessageContent.GetId(content);
            IsResponse = MessageContent.IsResponse(content);
            Source = source ?? new Reference();
            Target = target ?? new Reference();
            Proxy = proxy ?? new Reference();
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="obj"></param>
        /// <returns></returns>
        public override bool IsEqual(Message msg) {
            return
                IsEqual(Error, msg.Error) &&
                IsEqual(IsResponse, msg.IsResponse) &&
                IsEqual(TypeId, msg.TypeId) &&
                IsEqual(Target, msg.Target) &&
                IsEqual(Proxy, msg.Proxy) &&
                IsEqual(Source, msg.Source) &&
                IsEqual(Version, msg.Version) &&
                IsEqual(Content, msg.Content);
        }

        protected override void SetHashCode() {
            MixToHash(Error);
            MixToHash(IsResponse);
            MixToHash(TypeId);
            MixToHash(Target);
            MixToHash(Proxy);
            MixToHash(Source);
            MixToHash(Version);
            MixToHash(Content);
        }
    }
}
