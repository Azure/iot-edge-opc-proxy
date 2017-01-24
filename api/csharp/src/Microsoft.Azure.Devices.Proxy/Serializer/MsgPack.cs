// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Model {

    using System;
    using System.IO;
    using System.Threading;
    using System.Threading.Tasks;
    using MsgPack;
    using MsgPack.Serialization;

    /// <summary>
    /// Notes for mpack-cli serialization format and Api usage:
    /// 
    /// Framing is supported with array method (sequential properties, no names) or 
    /// map method (lookup by property name key). For that each object is preceeded
    /// with a array header or map header.  Array for obvious reasons is more efficient
    /// both in format and decoding, but difficult to version. Currently only array
    /// method is supported, although native code could be easily adopted to map as 
    /// well. Each array header is followed by a "MemberPlaceHolder" in msgpack-cli for 
    /// whatever reason, which is written as Nil, thus does not provide any value, just 
    /// consumes 8 bits. Native cmp codec also conforms to this protocol.
    /// 
    /// DataMember "Order" must be contigouus from base class to derived class, e.g.
    /// A : B then properties must be ordered as A1, A2, B3, B4 (see below).  Also
    /// if a order value is skipped, msgpack-cli inserts a null entry.
    /// 
    /// Serializers are generated and cached in the context, so we try to only create 
    /// a context once for this process (see singleton below).  
    /// Polymorphism is handled by serializing type names, however, in the case of 
    /// SocketAddress and MulticastOptions, the discriminator is AddressFamily, thus
    /// below implements a custom serializer for both types (todo for mcastopt).
    /// </summary>
    internal class MpackContext {

        private static SerializationContext instance;

        /// <summary>
        /// Set up serialization context
        /// </summary>
        /// <returns></returns>
        public static SerializationContext Get() {

            if (instance != null) {
                return instance;
            }

            // Lazily create and then keep around

            var context = new SerializationContext();
            context.SerializationMethod = SerializationMethod.Array;
            context.EnumSerializationMethod = EnumSerializationMethod.ByUnderlyingValue;
            context.CompatibilityOptions.PackerCompatibilityOptions = PackerCompatibilityOptions.None;

            context.Serializers.RegisterOverride(new MessageSerializer(context));
            context.Serializers.RegisterOverride(new SocketAddressSerializer(context));

            // Register custom handlers
            instance = context;
            return context;
        }

        /// <summary>
        /// Custom Message serializer to use type id as discriminator
        /// </summary>
        internal class MessageSerializer : MessagePackSerializer<Message> {

            public MessageSerializer(SerializationContext ownerContext)
                : base(ownerContext) { }

            protected override void PackToCore(Packer packer, Message addr) {
                PackToAsyncCore(packer, addr, CancellationToken.None).GetAwaiter().GetResult();
            }

            protected override Message UnpackFromCore(Unpacker unpacker) {
                Message message = new Message();

                if (!unpacker.IsArrayHeader) {
                    SerializationExceptions.ThrowIsNotArrayHeader(unpacker);
                }

                if (!unpacker.Read()) {
                    // Read null member value
                    SerializationExceptions.ThrowMissingItem(1, unpacker);
                }

                UInt16 version;
                if (!unpacker.ReadUInt16(out version)) {
                    SerializationExceptions.ThrowMissingItem(2, unpacker);
                }
                if (message.Version != version) {
                    throw new FormatException($"Bad message version {version}");
                }

                if (!unpacker.Read()) {
                    SerializationExceptions.ThrowMissingItem(3, unpacker);
                }
                message.Source = unpacker.Unpack<Reference>(OwnerContext);

                if (!unpacker.Read()) {
                    SerializationExceptions.ThrowMissingItem(4, unpacker);
                }
                message.Proxy = unpacker.Unpack<Reference>(OwnerContext);

                if (!unpacker.Read()) {
                    SerializationExceptions.ThrowMissingItem(5, unpacker);
                }
                message.Target = unpacker.Unpack<Reference>(OwnerContext);

                Int32 error;
                if (!unpacker.ReadInt32(out error)) {
                    SerializationExceptions.ThrowMissingItem(6, unpacker);
                }
                message.Error = error;

                bool isResponse;
                if (!unpacker.ReadBoolean(out isResponse)) {
                    SerializationExceptions.ThrowMissingItem(7, unpacker);
                }
                message.IsResponse = isResponse;

                UInt32 type;
                if (!unpacker.ReadUInt32(out type)) {
                    SerializationExceptions.ThrowMissingItem(8, unpacker);
                }
                message.TypeId = type;
                message.Content = null;
                if (!unpacker.Read()) {
                    SerializationExceptions.ThrowMissingItem(9, unpacker);
                }
                if (isResponse) {                                             
                    /**/ if (type == MessageContent.Data)
                        message.Content = unpacker.Unpack<DataMessage>(OwnerContext);
                    else if (type == MessageContent.Ping)
                        message.Content = unpacker.Unpack<PingResponse>(OwnerContext);
                    else if (type == MessageContent.Resolve)
                        message.Content = unpacker.Unpack<ResolveResponse>(OwnerContext);
                    else if (type == MessageContent.Link)
                        message.Content = unpacker.Unpack<LinkResponse>(OwnerContext);
              //    else if (type == MessageContent.SetOpt)
              //        message.Content = unpacker.Unpack<SetOptResponse>(OwnerContext);
                    else if (type == MessageContent.GetOpt)
                        message.Content = unpacker.Unpack<GetOptResponse>(OwnerContext);
              //      else if (type == MessageContent.Open)
              //          message.Content = unpacker.Unpack<OpenResponse>(OwnerContext);
                    else if (type == MessageContent.Close)
                        message.Content = unpacker.Unpack<CloseResponse>(OwnerContext);
                }
                else {
                    /**/ if (type == MessageContent.Data)
                        message.Content = unpacker.Unpack<DataMessage>(OwnerContext);
                    else if (type == MessageContent.Ping)
                        message.Content = unpacker.Unpack<PingRequest>(OwnerContext);
                    else if (type == MessageContent.Resolve)
                        message.Content = unpacker.Unpack<ResolveRequest>(OwnerContext);
                    else if (type == MessageContent.Link)
                        message.Content = unpacker.Unpack<LinkRequest>(OwnerContext);
                    else if (type == MessageContent.SetOpt)
                        message.Content = unpacker.Unpack<SetOptRequest>(OwnerContext);
                    else if (type == MessageContent.GetOpt)
                        message.Content = unpacker.Unpack<GetOptRequest>(OwnerContext);
                    else if (type == MessageContent.Open)
                        message.Content = unpacker.Unpack<OpenRequest>(OwnerContext);
              //    else if (type == MessageContent.Close)
              //        message.Content = unpacker.Unpack<CloseRequest>(OwnerContext);
                }
                unpacker.Drain();
                return message;
            }

            private async Task<bool> PackContent<T>(Packer packer,
                IMessageContent content, CancellationToken cancellationToken) {
                if (!(content is T))
                    return false;
                await packer.PackAsync((T)content, OwnerContext, cancellationToken);
                return true;
            }

            protected override async Task PackToAsyncCore(Packer packer,
                Message message, CancellationToken cancellationToken) {
                await packer.PackArrayHeaderAsync(8 + 1, cancellationToken);
                await packer.PackNullAsync(cancellationToken);                  // 1
                await packer.PackAsync(
                    message.Version, OwnerContext, cancellationToken);          // 2
                await packer.PackAsync(
                    message.Source, OwnerContext, cancellationToken);           // 3
                await packer.PackAsync(
                    message.Proxy, OwnerContext, cancellationToken);            // 4
                await packer.PackAsync(
                    message.Target, OwnerContext, cancellationToken);           // 5
                await packer.PackAsync(
                    message.Error, OwnerContext, cancellationToken);            // 6
                await packer.PackAsync(
                    message.IsResponse, OwnerContext, cancellationToken);       // 7
                await packer.PackAsync(
                    message.TypeId, OwnerContext, cancellationToken);           // 8

                if (message.Content is VoidMessage ||
                   (!await PackContent<DataMessage>(                            // 9
                        packer, message.Content, cancellationToken) &&
                    !await PackContent<PingRequest>(
                        packer, message.Content, cancellationToken) &&
                    !await PackContent<PingResponse>(
                        packer, message.Content, cancellationToken) &&
                    !await PackContent<ResolveRequest>(
                        packer, message.Content, cancellationToken) &&
                    !await PackContent<ResolveResponse>(
                        packer, message.Content, cancellationToken) &&
                    !await PackContent<LinkRequest>(
                        packer, message.Content, cancellationToken) &&
                    !await PackContent<LinkResponse>(
                        packer, message.Content, cancellationToken) &&
                    !await PackContent<SetOptRequest>(
                        packer, message.Content, cancellationToken) &&
                    !await PackContent<GetOptRequest>(
                        packer, message.Content, cancellationToken) &&
                    !await PackContent<GetOptResponse>(
                        packer, message.Content, cancellationToken) &&
                    !await PackContent<OpenRequest>(
                        packer, message.Content, cancellationToken) &&
                    !await PackContent<CloseResponse>(
                        packer, message.Content, cancellationToken))) {
                    await packer.PackNullAsync(cancellationToken);
                }
            }

            protected override Task<Message> UnpackFromAsyncCore(
                Unpacker unpacker, CancellationToken cancellationToken) {
                var tcs = new TaskCompletionSource<Message>();
                try {
                    tcs.SetResult(this.UnpackFromCore(unpacker));
                }
                catch (Exception ex) {
                    tcs.SetException(ex);
                }
                return tcs.Task;
            }
        }

        /// <summary>
        /// Custom socket address serializer to ensure format expected in native code
        /// </summary>
        internal class SocketAddressSerializer : MessagePackSerializer<SocketAddress> {

            public SocketAddressSerializer(SerializationContext ownerContext) 
                : base( ownerContext ) { }

            protected override void PackToCore(Packer packer, SocketAddress addr) {
                PackToAsyncCore(packer, addr, CancellationToken.None).GetAwaiter().GetResult();
            }

            protected override SocketAddress UnpackFromCore(Unpacker unpacker) {

                SocketAddress result = null;

                if (!unpacker.IsArrayHeader) {
                    SerializationExceptions.ThrowIsNotArrayHeader(unpacker);
                }

                if (!unpacker.Read()) {
                    // Read null member value
                    SerializationExceptions.ThrowMissingItem(0, unpacker);
                }

                Int32 af;
                if (!unpacker.ReadInt32(out af)) {
                    SerializationExceptions.ThrowMissingItem(1, unpacker);
                }

                AddressFamily family = (AddressFamily)af;

                if (family == AddressFamily.Unspecified) {
                    result = new NullSocketAddress();
                }
                else if (family == AddressFamily.Unix) {
                    string path;
                    if (!unpacker.ReadString(out path)) {
                        SerializationExceptions.ThrowMissingItem(2, unpacker);
                    }
                    result = new UnixSocketAddress(path);
                }
                else if (
                    family != AddressFamily.Proxy &&
                    family != AddressFamily.InterNetwork &&
                    family != AddressFamily.InterNetworkV6) {
                    SerializationExceptions.ThrowMissingItem(1, unpacker);
                }
                else {
                    UInt32 flow;
                    if (!unpacker.ReadUInt32(out flow)) {
                        SerializationExceptions.ThrowMissingItem(2, unpacker);
                    }

                    UInt16 port;
                    if (!unpacker.ReadUInt16(out port)) {
                        SerializationExceptions.ThrowMissingItem(3, unpacker);
                    }

                    byte[] address;
                    switch (family) {
                        case AddressFamily.InterNetwork:
                            if (!unpacker.ReadBinary(out address) || address.Length != 4) {
                                SerializationExceptions.ThrowMissingItem(4, unpacker);
                            }
                            result = new Inet4SocketAddress(
                                address, port, flow);
                            break;
                        case AddressFamily.InterNetworkV6:
                            if (!unpacker.ReadBinary(out address) || address.Length != 16) {
                                SerializationExceptions.ThrowMissingItem(4, unpacker);
                            }
                            UInt32 scopeId;
                            if (!unpacker.ReadUInt32(out scopeId)) {
                                SerializationExceptions.ThrowMissingItem(5, unpacker);
                            }
                            result = new Inet6SocketAddress(
                                address, port, flow, scopeId);
                            break;
                        case AddressFamily.Proxy:
                        default:
                            string host;
                            if (!unpacker.ReadString(out host)) {
                                SerializationExceptions.ThrowMissingItem(4, unpacker);
                            }
                            result = new ProxySocketAddress(host, port, flow);
                            break;
                    }
                }
                return result;
            }

            protected override async Task PackToAsyncCore(Packer packer,
                SocketAddress addr, CancellationToken cancellationToken) {
                switch (addr.Family) {
                    case AddressFamily.InterNetwork:
                        await packer.PackArrayHeaderAsync(5, cancellationToken); break;
                    case AddressFamily.InterNetworkV6:
                        await packer.PackArrayHeaderAsync(6, cancellationToken); break;
                    case AddressFamily.Proxy:
                        await packer.PackArrayHeaderAsync(5, cancellationToken); break;
                    case AddressFamily.Unix:
                        await packer.PackArrayHeaderAsync(3, cancellationToken); break;
                    case AddressFamily.Unspecified:
                        await packer.PackArrayHeaderAsync(2, cancellationToken); break;
                    default:
                        throw new NotSupportedException();
                }
                await packer.PackNullAsync(cancellationToken);
                await packer.PackAsync((Int32)addr.Family, 
                    OwnerContext, cancellationToken);
                if (addr.Family == AddressFamily.Unix) {
                    await packer.PackAsync(((UnixSocketAddress)addr).Path, 
                        OwnerContext, cancellationToken);
                }
                else if (addr.Family != AddressFamily.Unspecified) {
                    await packer.PackAsync(((InetSocketAddress)addr).Flow, 
                        OwnerContext, cancellationToken);
                    await packer.PackAsync(((InetSocketAddress)addr).Port, 
                        OwnerContext, cancellationToken);
                    switch (addr.Family) {
                        case AddressFamily.InterNetwork:
                            await packer.PackAsync(((Inet4SocketAddress)addr).Address, 
                                OwnerContext, cancellationToken);
                            break;
                        case AddressFamily.InterNetworkV6:
                            await packer.PackAsync(((Inet6SocketAddress)addr).Address, 
                                OwnerContext, cancellationToken);
                            await packer.PackAsync(((Inet6SocketAddress)addr).ScopeId, 
                                OwnerContext, cancellationToken);
                            break;
                        case AddressFamily.Proxy:
                            await packer.PackAsync(((ProxySocketAddress)addr).Host, 
                                OwnerContext, cancellationToken);
                            break;
                    }
                }
            }

            protected override Task<SocketAddress> UnpackFromAsyncCore(
                Unpacker unpacker, CancellationToken cancellationToken) {
                var tcs = new TaskCompletionSource<SocketAddress>();
                try {
                    tcs.SetResult(this.UnpackFromCore(unpacker));
                }
                catch (Exception ex) {
                    tcs.SetException(ex);
                }
                return tcs.Task;
            }
        }
    }
}
