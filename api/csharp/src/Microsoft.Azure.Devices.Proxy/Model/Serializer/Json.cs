// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using Newtonsoft.Json.Linq;
    using Newtonsoft.Json;
    using Newtonsoft.Json.Serialization;
    using System.Reflection;
    using System.Runtime.Serialization;

    /// <summary>
    /// Resolver implementation for the Proxy object model.
    /// </summary>
    public class CustomResolver : DefaultContractResolver {
        public static readonly CustomResolver Instance = new CustomResolver();

        protected override JsonProperty CreateProperty(MemberInfo member, 
            MemberSerialization memberSerialization) {
            JsonProperty property = base.CreateProperty(member, memberSerialization);

            var dataMember = member.GetCustomAttribute<DataMemberAttribute>();
            property.ShouldSerialize = (i) => {
                return dataMember != null;
            };
            property.PropertyName = dataMember != null ? dataMember.Name : "";
            return property;
        }

        protected override JsonContract CreateContract(Type objectType) {
            JsonContract contract = base.CreateContract(objectType);
            /**/ if (typeof(Reference).IsAssignableFrom(objectType)) {
                contract.Converter = new ReferenceConverter();
            }
            else if (typeof(SocketAddress).IsAssignableFrom(objectType)) {
                contract.Converter = new SocketAddressConverter();
            }
            else if (typeof(IProperty).IsAssignableFrom(objectType)) {
                contract.Converter = new PropertyConverter();
            }
            else if (typeof(IMulticastOption).IsAssignableFrom(objectType)) {
                contract.Converter = new MulticastOptionConverter();
            }
            else if (typeof(Message).IsAssignableFrom(objectType)) {
                contract.Converter = new MessageConverter();
            }
            else if (typeof(IVoidMessage).IsAssignableFrom(objectType)) {
                contract.Converter = new VoidConverter();
            }
            else if (typeof(IMessageContent).IsAssignableFrom(objectType)) {
                // Must come last for void messages to be properly serialized
                contract.Converter = new MessageContentConverter();
            }
            return contract;
        }

        class MessageReferencePlaceHolder : IMessageContent {
            public Message Ref { get; set; }

            public IMessageContent Clone() {
                throw new NotImplementedException();
            }
            public void Dispose() {
                throw new NotImplementedException();
            }
        }

        class MessageConverter : JsonConverter {
            public override bool CanConvert(Type objectType) {
                return false;
            }
            public override object ReadJson(JsonReader reader, Type objectType, 
                object existingValue, JsonSerializer serializer) {
                var message = Message.Get();
                // Hand this message to the content converter as existing object
                message.Content = new MessageReferencePlaceHolder { Ref = message };
                serializer.Populate(reader, message);
                if ((message.Version >> 16) != (VersionEx.Assembly.ToUInt() >> 16)) {
                    throw new FormatException($"Bad message version {message.Version}");
                }
                return message;
            }

            public override void WriteJson(JsonWriter writer, object value,
                JsonSerializer serializer) {
                throw new NotImplementedException();
            }
            public override bool CanWrite {
                get { return false; }
            }
        }

        class MessageContentConverter : JsonConverter {
            public override bool CanConvert(Type objectType) {
                return false;
            }
            private object Deserialize(Type t, JsonReader reader, JsonSerializer serializer) {
                var content = Activator.CreateInstance(t);

                // TODO: Create empty

                if (reader.TokenType != JsonToken.Null)
                    serializer.Populate(reader, content);
                return content;
            }
            public override object ReadJson(JsonReader reader, Type objectType,
                object existingValue, JsonSerializer serializer) {
                var reference = ((MessageReferencePlaceHolder)existingValue).Ref;
                return Deserialize(MessageContent.TypeOf(reference.TypeId,
                    reference.IsResponse), reader, serializer);
            }
            public override void WriteJson(JsonWriter writer, object value, JsonSerializer serializer) {
                throw new NotImplementedException();
            }
            public override bool CanWrite {
                get { return false; }
            }
        }

        class VoidConverter : MessageContentConverter {
            public override void WriteJson(JsonWriter writer, 
                object value, JsonSerializer serializer) {
                writer.WriteNull();
            }
            public override bool CanWrite {
                get { return true; }
            }
        }

        class ReferenceConverter : JsonConverter {
            public override bool CanConvert(Type objectType) {
                return false;
            }
            public override object ReadJson(JsonReader reader, Type objectType,
                object existingValue, JsonSerializer serializer) {
                Reference address = null;
                if (reader.TokenType == JsonToken.StartObject) {
                    do {
                        reader.Read();
                        if (reader.TokenType == JsonToken.PropertyName &&
                            reader.Value.ToString().Equals("id",
                                StringComparison.CurrentCultureIgnoreCase)) {
                            reader.Read();
                            if (reader.TokenType == JsonToken.String)
                                address = Reference.Parse(reader.Value.ToString());
                            else
                                throw new InvalidDataContractException();
                        }
                    } while (reader.TokenType != JsonToken.EndObject);
                }
                return address;
            }

            public override void WriteJson(JsonWriter writer, object value, 
                JsonSerializer serializer) {
                writer.WriteStartObject();
                writer.WritePropertyName("id");
                if (value == null) {
                    writer.WriteNull();
                }
                else {
                    writer.WriteValue(value.ToString());
                }
                writer.WriteEndObject();
            }
        }

        class SocketAddressConverter : JsonConverter {
            public override bool CanConvert(Type objectType) {
                return false;
            }

            public override object ReadJson(JsonReader reader, Type objectType, 
                object existingValue, JsonSerializer serializer) {
                JObject jsonObject = JObject.Load(reader);
                AddressFamily family = (AddressFamily)jsonObject.Value<int>("family");
                switch(family) {
                    case AddressFamily.Unspecified:
                        return new AnySocketAddress();
                    case AddressFamily.Unix:
                        return jsonObject.ToObject<UnixSocketAddress>();
                    case AddressFamily.Proxy:
                        return jsonObject.ToObject<ProxySocketAddress>();
                    case AddressFamily.InterNetwork:
                        return jsonObject.ToObject<Inet4SocketAddress>();
                    case AddressFamily.InterNetworkV6:
                        return jsonObject.ToObject<Inet6SocketAddress>();
                    default:
                        throw new SerializationException($"Bad socket address family {family}");
                }
            }
            public override void WriteJson(JsonWriter writer, object value, 
                JsonSerializer serializer) {
                throw new NotImplementedException();
            }
            public override bool CanWrite {
                get { return false; }
            }
        }

        class MulticastOptionConverter : JsonConverter {
            public override bool CanConvert(Type objectType) {
                return false;
            }

            public override object ReadJson(JsonReader reader, Type objectType,
                object existingValue, JsonSerializer serializer) {
                var jsonObject = JObject.Load(reader);
                var family = (AddressFamily)jsonObject.Value<int>("family");
                switch (family) {
                    case AddressFamily.InterNetwork:
                        return jsonObject.ToObject<Inet4MulticastOption>();
                    case AddressFamily.InterNetworkV6:
                        return jsonObject.ToObject<Inet6MulticastOption>();
                    default:
                        throw new SerializationException($"Bad address family {family}");
                }
            }
            public override void WriteJson(JsonWriter writer, object value,
                JsonSerializer serializer) {
                throw new NotImplementedException();
            }
            public override bool CanWrite {
                get { return false; }
            }
        }

        class PropertyConverter : JsonConverter {
            public override bool CanConvert(Type objectType) {
                return false;
            }

            public override object ReadJson(JsonReader reader, Type objectType,
                object existingValue, JsonSerializer serializer) {
                JObject jsonObject = JObject.Load(reader);
                uint type = jsonObject.Value<uint>("type");
                if (type == (uint)SocketOption.IpMulticastJoin ||
                    type == (uint)SocketOption.IpMulticastLeave) {
                    return Property<IMulticastOption>.Create(type,
                        jsonObject.GetValue("property").ToObject<IMulticastOption>(serializer));
                }
                else if (type == (uint)PropertyType.FileInfo) {
                    return Property<FileInfo>.Create(type,
                        jsonObject.GetValue("property").ToObject<FileInfo>(serializer));
                }
                else if (type == (uint)PropertyType.AddressInfo) {
                    return Property<AddressInfo>.Create(type,
                        jsonObject.GetValue("property").ToObject<AddressInfo>(serializer));
                }
                else if (type == (uint)PropertyType.InterfaceInfo) {
                    return Property<InterfaceInfo>.Create(type,
                        jsonObject.GetValue("property").ToObject<InterfaceInfo>(serializer));
                }
                else if (type >= (uint)DnsRecordType.Simple &&
                         type < (uint)DnsRecordType.__prx_record_max) {
                    return jsonObject.ToObject<Property<byte[]>>();
                }
                else if (type < (uint)SocketOption.__prx_so_max) {
                    return jsonObject.ToObject<Property<ulong>>();
                }
                else {
                    throw new FormatException($"Bad type encountered {type}");
                }
            }
            public override void WriteJson(JsonWriter writer, object value,
                JsonSerializer serializer) {
                throw new NotImplementedException();
            }
            public override bool CanWrite {
                get { return false; }
            }
        }
    }
}
