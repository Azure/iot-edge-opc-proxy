// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.Model {
    using MsgPack;
    using System;
    using System.Threading;
    using System.Threading.Tasks;

    /// <summary>
    /// Custom Message serializer to use type id as discriminator
    /// </summary>
    internal class MessageSerializer : Serializer<Message> {

        public override async Task<Message> ReadAsync(MsgPackReader reader,
            SerializerContext context, CancellationToken ct) {

            Message message = new Message();
            await reader.ReadObjectHeaderAsync(ct);
            ushort version = await reader.ReadUInt16Async(ct);
            if (message.Version != version) {
                throw new FormatException($"Bad message version {version}");
            }

            message.Source = await context.Get<Reference>().ReadAsync(
                reader, context, ct);
            message.Proxy = await context.Get<Reference>().ReadAsync(
                reader, context, ct);
            message.Target = await context.Get<Reference>().ReadAsync(
                reader, context, ct);

            message.Error = await reader.ReadInt32Async(ct);

            message.IsResponse = await reader.ReadBoolAsync(ct);

            message.TypeId = await reader.ReadUInt32Async(ct);

            message.Content = null;

            if (message.IsResponse) {
                /**/ if (message.TypeId == MessageContent.Data)
                    message.Content = await context.Get<DataMessage>().ReadAsync(
                        reader, context, ct);
                else if (message.TypeId == MessageContent.Ping)
                    message.Content = await context.Get<PingResponse>().ReadAsync(
                        reader, context, ct);
                else if (message.TypeId == MessageContent.Resolve)
                    message.Content = await context.Get<ResolveResponse>().ReadAsync(
                        reader, context, ct);
                else if (message.TypeId == MessageContent.Link)
                    message.Content = await context.Get<LinkResponse>().ReadAsync(
                        reader, context, ct);
                else if (message.TypeId == MessageContent.GetOpt)
                    message.Content = await context.Get<GetOptResponse>().ReadAsync(
                        reader, context, ct);
                else if (message.TypeId == MessageContent.Close)
                    message.Content = await context.Get<CloseResponse>().ReadAsync(
                        reader, context, ct);
                else
                    await reader.ReadAsync(ct);

            }
            else {
                /**/ if (message.TypeId == MessageContent.Data)
                    message.Content = await context.Get<DataMessage>().ReadAsync(
                        reader, context, ct);
                else if (message.TypeId == MessageContent.Ping)
                    message.Content = await context.Get<PingRequest>().ReadAsync(
                        reader, context, ct);
                else if (message.TypeId == MessageContent.Resolve)
                    message.Content = await context.Get<ResolveRequest>().ReadAsync(
                        reader, context, ct);
                else if (message.TypeId == MessageContent.Link)
                    message.Content = await context.Get<LinkRequest>().ReadAsync(
                        reader, context, ct);
                else if (message.TypeId == MessageContent.SetOpt)
                    message.Content = await context.Get<SetOptRequest>().ReadAsync(
                        reader, context, ct);
                else if (message.TypeId == MessageContent.GetOpt)
                    message.Content = await context.Get<GetOptRequest>().ReadAsync(
                        reader, context, ct);
                else if (message.TypeId == MessageContent.Open)
                    message.Content = await context.Get<OpenRequest>().ReadAsync(
                        reader, context, ct);
                else
                    await reader.ReadAsync(ct);
            }
            return message;
        }

        public override async Task WriteAsync(MsgPackWriter writer, Message message,
            SerializerContext context, CancellationToken ct) {

            if (message == null) {
                await writer.WriteNilAsync(ct);
                return;
            }

            await writer.WriteObjectHeaderAsync(8, ct);

            await writer.WriteAsync(message.Version, ct);                              

            await context.Get<Reference>().WriteAsync(                                 
                writer, message.Source, context, ct);
            await context.Get<Reference>().WriteAsync(                                 
                writer, message.Proxy, context, ct);
            await context.Get<Reference>().WriteAsync(                                 
                writer, message.Target, context, ct);

            await writer.WriteAsync(message.Error, ct);                                

            await writer.WriteAsync(message.IsResponse, ct);                           

            await writer.WriteAsync(message.TypeId, ct);                               

            /**/ if (message.Content is DataMessage)                                   
                await context.Get<DataMessage>().WriteAsync(
                    writer, (DataMessage)message.Content, context, ct);                          
            else if (message.Content is PingRequest)
                await context.Get<PingRequest>().WriteAsync(
                    writer, (PingRequest)message.Content, context, ct);
            else if (message.Content is PingResponse)
                await context.Get<PingResponse>().WriteAsync(
                    writer, (PingResponse)message.Content, context, ct);
            else if (message.Content is LinkRequest)
                await context.Get<LinkRequest>().WriteAsync(
                    writer, (LinkRequest)message.Content, context, ct);
            else if (message.Content is LinkResponse)
                await context.Get<LinkResponse>().WriteAsync(
                    writer, (LinkResponse)message.Content, context, ct);
            else if (message.Content is ResolveResponse)
                await context.Get<ResolveResponse>().WriteAsync(
                    writer, (ResolveResponse)message.Content, context, ct);
            else if (message.Content is ResolveRequest)
                await context.Get<ResolveRequest>().WriteAsync(
                    writer, (ResolveRequest)message.Content, context, ct);
            else if (message.Content is SetOptRequest)
                await context.Get<SetOptRequest>().WriteAsync(
                    writer, (SetOptRequest)message.Content, context, ct);
            else if (message.Content is GetOptRequest)
                await context.Get<GetOptRequest>().WriteAsync(
                    writer, (GetOptRequest)message.Content, context, ct);
            else if (message.Content is GetOptResponse)
                await context.Get<GetOptResponse>().WriteAsync(
                    writer, (GetOptResponse)message.Content, context, ct);
            else if (message.Content is OpenRequest)
                await context.Get<OpenRequest>().WriteAsync(
                    writer, (OpenRequest)message.Content, context, ct);
            else if (message.Content is CloseResponse)
                await context.Get<CloseResponse>().WriteAsync(
                    writer, (CloseResponse)message.Content, context, ct);
            else if (message.Content is VoidMessage)
                await writer.WriteNilAsync(ct);
            else
                throw new FormatException("Bad type in content");
        }
    }

    /// <summary>
    /// Custom socket address serializer to ensure format expected in native code
    /// </summary>
    internal class SocketAddressSerializer : Serializer<SocketAddress> {

        public async override Task<SocketAddress> ReadAsync(MsgPackReader reader,
            SerializerContext context, CancellationToken ct) {
            SocketAddress result = null;
            int members = await reader.ReadObjectHeaderAsync(ct);
            if (members < 1) {
                throw new FormatException(
                    $"Unexpected number of properties {members}");
            }
            AddressFamily family = (AddressFamily)await reader.ReadInt32Async(ct);
            if (family == AddressFamily.Unspecified) {
                result = new NullSocketAddress();
            }
            else if (family == AddressFamily.Unix) {
                if (members < 2) {
                    throw new FormatException(
                        $"Unexpected number of properties {members}");
                }
                string path = await reader.ReadStringAsync(ct);
                result = new UnixSocketAddress(path);
            }
            else if (
                family != AddressFamily.Proxy &&
                family != AddressFamily.InterNetwork &&
                family != AddressFamily.InterNetworkV6) {
                throw new FormatException($"Bad address family {family}");
            }
            else {
                if (members < 4) {
                    throw new FormatException(
                        $"Unexpected number of properties {members}");
                }
                uint flow = await reader.ReadUInt32Async(ct);
                ushort port = await reader.ReadUInt16Async(ct);
                byte[] address;
                switch (family) {
                    case AddressFamily.InterNetwork:
                        address = await reader.ReadBinAsync(ct);
                        if (address.Length != 4) {
                            throw new FormatException(
                                $"Bad v4 address size {address.Length}");
                        }
                        result = new Inet4SocketAddress(address, port, flow);
                        break;
                    case AddressFamily.InterNetworkV6:
                        address = await reader.ReadBinAsync(ct);
                        if (address.Length != 16) {
                            throw new FormatException(
                                $"Bad v6 address size {address.Length}");
                        }
                        if (members < 5) {
                            throw new FormatException(
                                $"Unexpected number of properties {members}");
                        }
                        uint scopeId = await reader.ReadUInt32Async(ct);
                        result = new Inet6SocketAddress(address, port, flow, scopeId);
                        break;
                    case AddressFamily.Proxy:
                    default:
                        string host = await reader.ReadStringAsync(ct);
                        result = new ProxySocketAddress(host, port, flow);
                        break;
                }
            }
            return result;
        }

        public override async Task WriteAsync(MsgPackWriter writer,
            SocketAddress addr, SerializerContext context, CancellationToken ct) {
            if (addr == null) {
                await writer.WriteNilAsync(ct);
                return;
            }

            switch (addr.Family) {
                case AddressFamily.InterNetwork:
                    await writer.WriteObjectHeaderAsync(4, ct);
                    break;
                case AddressFamily.InterNetworkV6:
                    await writer.WriteObjectHeaderAsync(5, ct);
                    break;
                case AddressFamily.Proxy:
                    await writer.WriteObjectHeaderAsync(4, ct);
                    break;
                case AddressFamily.Unix:
                    await writer.WriteObjectHeaderAsync(2, ct);
                    break;
                case AddressFamily.Unspecified:
                    await writer.WriteObjectHeaderAsync(1, ct);
                    break;
                default:
                    throw new NotSupportedException();
            }

            await writer.WriteAsync((int)addr.Family, ct);
            if (addr.Family == AddressFamily.Unix) {
                await writer.WriteAsync(((UnixSocketAddress)addr).Path, ct);
            }
            else if (addr.Family != AddressFamily.Unspecified) {
                await writer.WriteAsync(((InetSocketAddress)addr).Flow, ct);
                await writer.WriteAsync(((InetSocketAddress)addr).Port, ct);
                switch (addr.Family) {
                    case AddressFamily.InterNetwork:
                        await writer.WriteAsync(((Inet4SocketAddress)addr).Address, ct);
                        break;
                    case AddressFamily.InterNetworkV6:
                        await writer.WriteAsync(((Inet6SocketAddress)addr).Address, ct);
                        await writer.WriteAsync(((Inet6SocketAddress)addr).ScopeId, ct);
                        break;
                    case AddressFamily.Proxy:
                        await writer.WriteAsync(((ProxySocketAddress)addr).Host, ct);
                        break;
                }
            }
        }
    }
}
