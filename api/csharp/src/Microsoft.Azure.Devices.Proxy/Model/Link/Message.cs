// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Runtime.Serialization;
    using System.Text;
    using System.Threading;

    /// <summary>
    /// A serializable message sent between proxy servers and clients
    /// </summary>
    [DataContract]
    public class Message : Poco<Message> {

        /// <summary>
        /// Version validation field
        /// </summary>
        [DataMember(Name = "version", Order = 1)]
        public uint Version {
            get; set;
        }

        /// <summary>
        /// Source address
        /// </summary>
        [DataMember(Name = "source_id", Order = 2)]
        public Reference Source {
            get; set;
        }

        /// <summary>
        /// Proxy address
        /// </summary>
        [DataMember(Name = "proxy_id", Order = 3)]
        public Reference Proxy {
            get; set;
        }

        /// <summary>
        /// Target object 
        /// </summary>
        [DataMember(Name = "target_id", Order = 4)]
        public Reference Target {
            get; set;
        }

        /// <summary>
        /// Sequence id
        /// </summary>
        [DataMember(Name = "seq_id", Order = 5)]
        public uint SequenceId {
            get; set;
        }

        /// <summary>
        /// Error code if this is an error message
        /// </summary>
        [DataMember(Name = "error_code", Order = 6)]
        public int Error {
            get; set;
        }

        /// <summary>
        /// Whether the message is response to a request
        /// </summary>
        [DataMember(Name = "is_response", Order = 7)]
        public bool IsResponse {
            get; set;
        }

        /// <summary>
        /// Content type
        /// </summary>
        [DataMember(Name = "type", Order = 8)]
        public uint TypeId {
            get; set;
        }

        /// <summary>
        /// Content type
        /// </summary>
        [DataMember(Name = "content", Order = 9)]
        public IMessageContent Content {
            get; set;
        }


        internal string DeviceId {
            get; set;
        }


        /// <summary>
        /// Create message with specific buffer
        /// </summary>
        /// <param name="buf"></param>
        public static Message Create(Reference source, Reference target,
            IMessageContent content) =>
            Create(source, target, Reference.Null, content);

        /// <summary>
        /// Create message with specific buffer
        /// </summary>
        /// <param name="buf"></param>
        public static Message Create(Reference source, Reference target,
            Reference proxy, IMessageContent content) =>
            Create((uint)Interlocked.Increment(ref _counter), source, target, proxy, content);

        /// <summary>
        /// Create message with specific buffer
        /// </summary>
        /// <param name="buf"></param>
        protected static Message Create(uint sequenceId, Reference source,
            Reference target, Reference proxy, IMessageContent content, string deviceId = null) {
            var message = Get();
            message.Version = VersionEx.Assembly.ToUInt();
            message.SequenceId = sequenceId;
            message.Content = content ?? throw new ArgumentException("content was null");
            message.TypeId = MessageContent.GetId(content);
            message.IsResponse = MessageContent.IsResponse(content);
            message.Source = source ?? new Reference();
            message.Target = target ?? new Reference();
            message.Proxy = proxy ?? new Reference();
            message.DeviceId = deviceId;
            return message;
        }

        /// <summary>
        /// Create a clone of the message including the owned content
        /// </summary>
        /// <param name="message"></param>
        public Message Clone() => Create(SequenceId, Source, Target,
            Proxy, Content.Clone(), DeviceId);

        public override void Dispose() {
            var content = Content;
            Content = null;
            content?.Dispose();
            base.Dispose();
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="obj"></param>
        /// <returns></returns>
        public override bool IsEqual(Message msg) {
            return
                IsEqual(Version, msg.Version) &&
                IsEqual(SequenceId, msg.SequenceId) &&
                IsEqual(Error, msg.Error) &&
                IsEqual(IsResponse, msg.IsResponse) &&
                IsEqual(TypeId, msg.TypeId) &&
                IsEqual(Target, msg.Target) &&
                IsEqual(Proxy, msg.Proxy) &&
                IsEqual(Source, msg.Source) &&
                IsEqual(Content, msg.Content);
        }

        protected override void SetHashCode() {
            MixToHash(Version);
            MixToHash(SequenceId);
            MixToHash(Error);
            MixToHash(IsResponse);
            MixToHash(TypeId);
            MixToHash(Target);
            MixToHash(Proxy);
            MixToHash(Source);
            MixToHash(Content);
        }

        public override string ToString() {
            var bld = new StringBuilder();
            bld.Append("[");
            bld.Append(MessageContent.TypeOf(TypeId, IsResponse).Name);
            bld.Append(" #");
            bld.Append(SequenceId);
            bld.Append("] ");
            bld.Append(Content == null ? "null" : Content.ToString());
            bld.Append($"({Error}) [{Source}=>{Target}(Proxy:{Proxy})]");
            return bld.ToString();
        }

        private static int _counter;
    }
}
