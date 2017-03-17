
namespace Microsoft.Azure.Devices.Proxy.Model {
    using System;
    using System.IO;
    using System.Diagnostics;
    using System.Runtime.InteropServices;
    using VisualStudio.TestTools.UnitTesting;

    /// <summary>
    /// Native interop 
    /// </summary>
    internal static partial class Interop {
        internal static class Libraries {
            internal const string LibCodec = "libproto.dll";
        }

        [DllImport(Libraries.LibCodec, EntryPoint = "io_message_decode_encode", CallingConvention = CallingConvention.Cdecl)]
        internal static extern unsafe int io_message_decode_encode(int codecId, int numMsgs, byte* inBuf, int inLen, byte* outBuf, int outLen);

        internal static unsafe byte[] MessageDecodeEncode(CodecId codecId, int numMsgs, byte[] inBuf, long inLen, byte[] outBuf, long outLen) {
            int error;
            fixed (byte* rawOut = outBuf, rawIn = inBuf) {
                error = io_message_decode_encode((int)codecId, numMsgs, rawIn, (int)inLen, rawOut, (int)outLen);
            }
            if (error != 0)
                throw new InvalidDataException();
            return outBuf;
        }
    }

    [TestClass]
    public class MessageTests {

        public byte[] buffer = new byte[0x4000];

        [TestMethod]
        public void TestMpackPingRequestWriteRead() {
            // Arrange
            ProxySocketAddress address = new ProxySocketAddress();
            address.Flow = 1;
            address.Port = 777;
            address.Host = "localhost";

            Message request = new Message(null, null, null, new PingRequest(address));
            MemoryStream stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Mpack);
            
            
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Mpack);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestMpackPingResponseWriteRead() {
            // Arrange
            ProxySocketAddress address = new ProxySocketAddress();
            address.Flow = 0xff;
            address.Port = 65500;
            address.Host = "rawrawrawraw";

            PingResponse args = new PingResponse(address);
            args.PhysicalAddress[3] = 3;
            args.PhysicalAddress[5] = 5;
            args.TimeMs = 1333333;

            Message response = new Message(null, null, null, args);
            MemoryStream stream = new MemoryStream();

            // Act
            response.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Mpack);

            // Assert
            Assert.IsTrue(response.Equals(returned));
        }

        [TestMethod]
        public void TestMpackResolveRequestWriteRead() {
            // Arrange
            ResolveRequest args = new ResolveRequest();
            args.Family = AddressFamily.InterNetworkV6;
            args.Host = "testhosterfoobarlongandagainsomevaluehereohiamnotdoneyetoknowiamdone";
            args.Port = 443;
            args.Flags = (UInt32)GetAddrInfoFlag.Passive;

            Message request = new Message(null, null, null, args);
            MemoryStream stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Mpack);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestMpackResolveResponseWriteRead() {
            // Arrange
            ProxySocketAddress address = new ProxySocketAddress();
            address.Flow = 0xff;
            address.Port = 65500;
            address.Host = "rawrawrawraw";

            ResolveResponse args = new ResolveResponse();
            for (UInt32 i = 0; i < 10; i++) {
                var ai = new AddressInfo();
                ai.CanonicalName = String.Format("name{0}", i);
                var sa = new Inet4SocketAddress(i | 0xf0000, (UInt16)i, i + 10);
                ai.SocketAddress = sa;
                args.Results.Add(ai);
            }

            Message response = new Message(null, null, null, args);
            MemoryStream stream = new MemoryStream();

            // Act
            response.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Mpack);

            // Assert
            Assert.IsTrue(response.Equals(returned));
        }

        [TestMethod]
        public void TestMpackLinkRequestWriteRead1() {
            // Arrange
            ProxySocketAddress address = new ProxySocketAddress();
            address.Port = 443;
            address.Host = "myhosttolinkto";

            SocketInfo props = new SocketInfo();
            props.Address = address;
            props.Family = AddressFamily.InterNetwork;
            props.Protocol = ProtocolType.Udp;
            props.Type = SocketType.Stream;
            props.Flags = (UInt32)SocketFlags.Passive;
            // no options

            Message request = new Message(null, null, null, new LinkRequest(props));
            MemoryStream stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Mpack);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestMpackLinkRequestWriteRead2() {
            // Arrange
            ProxySocketAddress address = new ProxySocketAddress();
            address.Port = 443;
            address.Host = "myhosttolinkto";

            SocketInfo props = new SocketInfo();
            props.Address = address;
            props.Family = AddressFamily.InterNetwork;
            props.Protocol = ProtocolType.Udp;
            props.Type = SocketType.Stream;
            props.Flags = (UInt32)SocketFlags.Passive;

            for (Int32 i = 0; i < 10; i++) {
                var ov = new SocketOptionValue();
                ov.Option = (SocketOption)(13 - i);
                ov.Value = (UInt64)i;
                props.Options.Add(ov);
            }

            Message request = new Message(null, null, null, new LinkRequest(props));
            MemoryStream stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Mpack);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }


        [TestMethod]
        public void TestMpackLinkResponseWriteRead() {
            // Arrange
            Inet4SocketAddress sal = new Inet4SocketAddress(0xffaabbcc, 9643, 1);
            var ab = new byte[16];
            new Random().NextBytes(ab);
            Inet6SocketAddress sap = new Inet6SocketAddress(ab, 443, 1, 0);

            LinkResponse args = new LinkResponse();
            args.LocalAddress = sal;
            args.PeerAddress = sap;
            args.LinkId = new Reference();

            Message response = new Message(null, null, null, args);
            MemoryStream stream = new MemoryStream();

            // Act
            response.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Mpack);
           
            // Assert
            Assert.IsTrue(response.Equals(returned));
        }

        [TestMethod]
        public void TestMpackSetOptRequestWriteRead() {
            // Arrange
            SocketOptionValue optionValue = new SocketOptionValue();
            optionValue.Option = SocketOption.Broadcast;
            optionValue.Value = 1;

            Message request = new Message(null, null, null, new SetOptRequest(optionValue));
            MemoryStream stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Mpack);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestMpackSetOptResponseWriteRead() {
            // Arrange
            Message response = new Message(null, null, null, new SetOptResponse());
            MemoryStream stream = new MemoryStream();

            // Act
            response.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Mpack);
            
            // Assert
            Assert.IsTrue(response.Equals(returned));
        }

        [TestMethod]
        public void TestMpackGetOptRequestWriteRead() {
            // Arrange
            Message request = new Message(null, null, null, new GetOptRequest(SocketOption.Broadcast));
            MemoryStream stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Mpack);
 
            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestMpackGetOptResponseWriteRead() {
            // Arrange
            SocketOptionValue optionValue = new SocketOptionValue();
            optionValue.Option = SocketOption.Broadcast;
            optionValue.Value = 1;

            Message response = new Message(null, null, null, new GetOptResponse(optionValue));
            MemoryStream stream = new MemoryStream();

            // Act
            response.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Mpack);

            // Assert
            Assert.IsTrue(response.Equals(returned));
        }

        [TestMethod]
        public void TestMpackOpenRequestWriteRead() {

            // Arrange
            OpenRequest args = new OpenRequest();
            args.StreamId = new Reference();
            args.ConnectionString = "dfksjaödfjkasdfölskajdfölsadfjkslöajksadlöjksdlöfsjkadflösdajkfösdlafj";
            args.IsPolled = true;

            Message request = new Message(null, null, null, args);
            MemoryStream stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Mpack);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestMpackOpenResponseWriteRead() {

            // Arrange
            Message response = new Message(null, null, null, new OpenResponse());
            MemoryStream stream = new MemoryStream();

            // Act
            response.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Mpack);

            // Assert
            Assert.IsTrue(response.Equals(returned));
        }

        [TestMethod]
        public void TestMpackPollRequestWriteRead() {
            // Arrange
            Message request = new Message(null, null, null, new PollRequest(100000));
            MemoryStream stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Mpack);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestMpackDataWriteRead1() {
            // Arrange
            Inet4SocketAddress source = new Inet4SocketAddress(0xffaabbcc, 9643, 1);

            DataMessage args = new DataMessage();
            args.Source = source;
            args.Payload = new byte[145];
            new Random().NextBytes(args.Payload);

            Message datagramIn = new Message(null, null, null, args);
            MemoryStream stream = new MemoryStream();

            // Act
            datagramIn.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Mpack);

            // Assert
            Assert.IsTrue(datagramIn.Equals(returned));
        }

        [TestMethod]
        public void TestMpackDataWriteRead2() {

            // Arrange
            DataMessage args = new DataMessage();
            args.Source = new NullSocketAddress();
            args.Payload = new byte[600];
            new Random().NextBytes(args.Payload);

            Message datagramIn = new Message(null, null, null, args);
            MemoryStream stream = new MemoryStream();

            // Act
            datagramIn.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Mpack);

            // Assert
            Assert.IsTrue(datagramIn.Equals(returned));
        }

        [TestMethod]
        public void TestMpackOpenPlusDataWriteRead() {

            // Arrange
            OpenRequest args1 = new OpenRequest();
            args1.StreamId = new Reference();
            args1.ConnectionString = "dfksjaödfjkasdfölskajdfölsadfjkslöajksadlöjksdlöfsjkadflösdajkfösdlafj";
            args1.IsPolled = false;
            DataMessage args2 = new DataMessage();
            args2.Source = new NullSocketAddress();
            args2.Payload = new byte[600];
            new Random().NextBytes(args2.Payload);

            Message datagramIn1 = new Message(null, null, null, args1);
            Message datagramIn2 = new Message(null, null, null, args2);
            MemoryStream stream = new MemoryStream();

            // Act
            datagramIn1.Encode(stream, CodecId.Mpack);
            datagramIn2.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 2, stream.GetBuffer(), stream.Length, buffer, buffer.Length);

            stream = new MemoryStream(buf);
            Message returned1 = Message.Decode(stream, CodecId.Mpack);
            Message returned2 = Message.Decode(stream, CodecId.Mpack);

            // Assert
            Assert.IsTrue(datagramIn1.Equals(returned1));
            Assert.IsTrue(datagramIn2.Equals(returned2));
        }

        [TestMethod]
        public void TestMpackDataPlusDataWriteRead() {

            // Arrange
            DataMessage args1 = new DataMessage();
            args1.Source = new NullSocketAddress();
            args1.Payload = new byte[145];
            new Random().NextBytes(args1.Payload);
            DataMessage args2 = new DataMessage();
            args2.Source = new NullSocketAddress();
            args2.Payload = new byte[600];
            new Random().NextBytes(args2.Payload);
            DataMessage args3 = new DataMessage();
            args3.Source = new NullSocketAddress();
            args3.Payload = new byte[400];
            new Random().NextBytes(args3.Payload);

            Message datagramIn1 = new Message(null, null, null, args1);
            Message datagramIn2 = new Message(null, null, null, args2);
            Message datagramIn3 = new Message(null, null, null, args3);
            MemoryStream stream = new MemoryStream();

            // Act
            datagramIn1.Encode(stream, CodecId.Mpack);
            datagramIn2.Encode(stream, CodecId.Mpack);
            datagramIn3.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 3, stream.GetBuffer(), stream.Length, buffer, buffer.Length);

            stream = new MemoryStream(buf);
            Message returned1 = Message.Decode(stream, CodecId.Mpack);
            Message returned2 = Message.Decode(stream, CodecId.Mpack);
            Message returned3 = Message.Decode(stream, CodecId.Mpack);

            // Assert
            Assert.IsTrue(datagramIn1.Equals(returned1));
            Assert.IsTrue(datagramIn2.Equals(returned2));
            Assert.IsTrue(datagramIn3.Equals(returned3));
        }

        [TestMethod]
        public void TestMpackCloseRequestWriteRead() {
            // Arrange
            Message request = new Message(null, null, null, new CloseRequest());
            MemoryStream stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Mpack);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestMpackCloseResponseWriteRead() {
            // Arrange
            CloseResponse args = new CloseResponse();
            args.ErrorCode = (Int32)SocketError.Closed;
            args.TimeOpenInMilliseconds = 1000;
            args.BytesSent = 200;
            args.BytesReceived = 1;

            Message response = new Message(null, null, null, args);
            MemoryStream stream = new MemoryStream();

            // Act
            response.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Mpack);

            // Assert
            Assert.IsTrue(response.Equals(returned));
        }

        [TestMethod]
        public void TestJsonPingRequestWriteRead() {
            // Arrange
            ProxySocketAddress address = new ProxySocketAddress();
            address.Flow = 1;
            address.Port = 777;
            address.Host = "localhost";

            Message request = new Message(null, null, null, new PingRequest(address));
            MemoryStream stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestJsonPingResponseWriteRead() {
            // Arrange
            ProxySocketAddress address = new ProxySocketAddress();
            address.Flow = 0xff;
            address.Port = 65500;
            address.Host = "rawrawrawraw";

            PingResponse args = new PingResponse(address);
            args.PhysicalAddress[3] = 3;
            args.PhysicalAddress[5] = 5;
            args.TimeMs = 1333333;

            Message response = new Message(null, null, null, args);
            MemoryStream stream = new MemoryStream();

            // Act
            response.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(response.Equals(returned));
        }

        [TestMethod]
        public void TestJsonResolveRequestWriteRead() {
            // Arrange
            ResolveRequest args = new ResolveRequest();
            args.Family = AddressFamily.InterNetworkV6;
            args.Host = "testhosterfoobarlongandagainsomevaluehereohiamnotdoneyetoknowiamdone";
            args.Port = 443;
            args.Flags = (UInt32)GetAddrInfoFlag.Passive;

            Message request = new Message(null, null, null, args);
            MemoryStream stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestJsonResolveResponseWriteRead() {
            // Arrange
            ProxySocketAddress address = new ProxySocketAddress();
            address.Flow = 0xff;
            address.Port = 65500;
            address.Host = "rawrawrawraw";

            ResolveResponse args = new ResolveResponse();
            for (UInt32 i = 0; i < 10; i++) {
                var ai = new AddressInfo();
                ai.CanonicalName = String.Format("name{0}", i);
                var sa = new Inet4SocketAddress(i | 0xf0000, (UInt16)i, i + 10);
                ai.SocketAddress = sa;
                args.Results.Add(ai);
            }

            Message response = new Message(null, null, null, args);
            MemoryStream stream = new MemoryStream();

            // Act
            response.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(response.Equals(returned));
        }

        [TestMethod]
        public void TestJsonLinkRequestWriteRead1() {
            // Arrange
            ProxySocketAddress address = new ProxySocketAddress();
            address.Port = 443;
            address.Host = "myhosttolinkto";

            SocketInfo props = new SocketInfo();
            props.Address = address;
            props.Family = AddressFamily.InterNetwork;
            props.Protocol = ProtocolType.Udp;
            props.Type = SocketType.Stream;
            props.Flags = (UInt32)SocketFlags.Passive;
            // no options

            Message request = new Message(null, null, null, new LinkRequest(props));
            MemoryStream stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestJsonLinkRequestWriteRead2() {
            // Arrange
            ProxySocketAddress address = new ProxySocketAddress();
            address.Port = 443;
            address.Host = "myhosttolinkto";

            SocketInfo props = new SocketInfo();
            props.Address = address;
            props.Family = AddressFamily.InterNetwork;
            props.Protocol = ProtocolType.Udp;
            props.Type = SocketType.Stream;
            props.Flags = (UInt32)SocketFlags.Passive;

            for (Int32 i = 0; i < 10; i++) {
                var ov = new SocketOptionValue();
                ov.Option = (SocketOption)(13 - i);
                ov.Value = (UInt64)i;
                props.Options.Add(ov);
            }

            Message request = new Message(null, null, null, new LinkRequest(props));
            MemoryStream stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }


        [TestMethod]
        public void TestJsonLinkResponseWriteRead() {
            // Arrange
            Inet4SocketAddress sal = new Inet4SocketAddress(0xffaabbcc, 9643, 1);
            var ab = new byte[16];
            new Random().NextBytes(ab);
            Inet6SocketAddress sap = new Inet6SocketAddress(ab, 443, 1, 0);

            LinkResponse args = new LinkResponse();
            args.LocalAddress = sal;
            args.PeerAddress = sap;
            args.LinkId = new Reference();

            Message response = new Message(null, null, null, args);
            MemoryStream stream = new MemoryStream();

            // Act
            response.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(response.Equals(returned));
        }

        [TestMethod]
        public void TestJsonSetOptRequestWriteRead() {
            // Arrange
            SocketOptionValue optionValue = new SocketOptionValue();
            optionValue.Option = SocketOption.Broadcast;
            optionValue.Value = 1;

            Message request = new Message(null, null, null, new SetOptRequest(optionValue));
            MemoryStream stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestJsonSetOptResponseWriteRead() {
            // Arrange
            Message response = new Message(null, null, null, new SetOptResponse());
            MemoryStream stream = new MemoryStream();

            // Act
            response.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(response.Equals(returned));
        }

        [TestMethod]
        public void TestJsonGetOptRequestWriteRead() {
            // Arrange
            Message request = new Message(null, null, null, new GetOptRequest(SocketOption.Broadcast));
            MemoryStream stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestJsonGetOptResponseWriteRead() {
            // Arrange
            SocketOptionValue optionValue = new SocketOptionValue();
            optionValue.Option = SocketOption.Broadcast;
            optionValue.Value = 1;

            Message response = new Message(null, null, null, new GetOptResponse(optionValue));
            MemoryStream stream = new MemoryStream();

            // Act
            response.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(response.Equals(returned));
        }

        [TestMethod]
        public void TestJsonOpenRequestWriteRead() {

            // Arrange
            OpenRequest args = new OpenRequest();
            args.StreamId = new Reference();
            args.ConnectionString = "dfksjaödfjkasdfölskajdfölsadfjkslöajksadlöjksdlöfsjkadflösdajkfösdlafj";
            args.IsPolled = true;

            Message request = new Message(null, null, null, args);
            MemoryStream stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestJsonOpenResponseWriteRead() {

            // Arrange
            Message response = new Message(null, null, null, new OpenResponse());
            MemoryStream stream = new MemoryStream();

            // Act
            response.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(response.Equals(returned));
        }

        [TestMethod]
        public void TestJsonPollRequestWriteRead() {
            // Arrange
            Message request = new Message(null, null, null, new PollRequest(100000));
            MemoryStream stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestJsonDataWriteRead1() {
            // Arrange
            Inet4SocketAddress source = new Inet4SocketAddress(0xffaabbcc, 9643, 1);

            DataMessage args = new DataMessage();
            args.Source = source;
            args.Payload = new byte[145];
            new Random().NextBytes(args.Payload);

            Message datagramIn = new Message(null, null, null, args);
            MemoryStream stream = new MemoryStream();

            // Act
            datagramIn.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(datagramIn.Equals(returned));
        }

        [TestMethod]
        public void TestJsonDataWriteRead2() {

            // Arrange
            DataMessage args = new DataMessage();
            args.Source = new NullSocketAddress();
            args.Payload = new byte[600];
            new Random().NextBytes(args.Payload);

            Message datagramIn = new Message(null, null, null, args);
            MemoryStream stream = new MemoryStream();

            // Act
            datagramIn.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(datagramIn.Equals(returned));
        }

        [TestMethod]
        public void TestJsonCloseRequestWriteRead() {
            // Arrange
            Message request = new Message(null, null, null, new CloseRequest());
            MemoryStream stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestJsonCloseResponseWriteRead() {
            // Arrange
            CloseResponse args = new CloseResponse();
            args.ErrorCode = (Int32)SocketError.Closed;
            args.TimeOpenInMilliseconds = 1000;
            args.BytesSent = 200;
            args.BytesReceived = 1;

            Message response = new Message(null, null, null, args);
            MemoryStream stream = new MemoryStream();

            // Act
            response.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            Message returned = Message.Decode(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(response.Equals(returned));
        }
    } // class
}
