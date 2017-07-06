
namespace Microsoft.Azure.Devices.Proxy.Model {
    using System;
    using System.IO;
    using System.Diagnostics;
    using System.Runtime.InteropServices;
    using VisualStudio.TestTools.UnitTesting;
    using System.Text;

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

        [DllImport(Libraries.LibCodec, EntryPoint = "io_type_decode_encode", CallingConvention = CallingConvention.Cdecl)]
        internal static extern unsafe int io_type_decode_encode(int codecId, int type, byte* inBuf, int inLen, byte* outBuf, int outLen);

        internal static unsafe byte[] TypeDecodeEncode(CodecId codecId, int type, byte[] inBuf, long inLen, byte[] outBuf, long outLen) {
            int error;
            fixed (byte* rawOut = outBuf, rawIn = inBuf) {
                error = io_type_decode_encode((int)codecId, type, rawIn, (int)inLen, rawOut, (int)outLen);
            }
            if (error != 0)
                throw new InvalidDataException();
            return outBuf;
        }
    }


    [TestClass]
    public class TypeTests {
        public byte[] buffer = new byte[0x4000];

        private readonly int BrowseRequestType = 1;
        private readonly int BrowseResponseType = 2;

        [TestMethod]
        public void TestMpackBrowseRequestWriteRead() {
            // Arrange
            var request = new BrowseRequest {
                Flags = 0,
                Handle = new Reference(),
                Item = new ProxySocketAddress {
                    Flags = 1,
                    Port = 1,
                    Host = ""
                },
                Type = BrowseRequest.Service
            };
            var stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Mpack);

            byte[] buf = Interop.TypeDecodeEncode(CodecId.Mpack, BrowseRequestType, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<BrowseRequest>(new MemoryStream(buf), CodecId.Mpack);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestMpackBrowseResponseWriteRead1() {
            // Arrange
            var response = new BrowseResponse {
                Flags = 0,
                Handle = new Reference(),
                Item = new ProxySocketAddress {
                    Flags = 1,
                    Port = 1,
                    Host = "test._test._tcp.longdomain12345"
                },
                Error = 0
            };

            response.Properties.Add(Property<byte[]>.Create(
                (uint)DnsRecordType.Txt,
                 Encoding.UTF8.GetBytes("some test = test")
            ));
            response.Properties.Add(Property<byte[]>.Create(
                (uint)DnsRecordType.Txt,
                Encoding.UTF8.GetBytes("a record")
            ));
            response.Properties.Add(Property<byte[]>.Create(
                (uint)DnsRecordType.Txt,
                Encoding.UTF8.GetBytes("spasdfsdafsda")
            ));
            response.Properties.Add(Property<byte[]>.Create(
                (uint)DnsRecordType.Txt,
                Encoding.UTF8.GetBytes("asd=3433434")
            ));
            response.Properties.Add(Property<byte[]>.Create(
                (uint)DnsRecordType.Txt,
                Encoding.UTF8.GetBytes("FPOÜDFD")
            ));
            response.Properties.Add(Property<byte[]>.Create(
                (uint)DnsRecordType.Txt,
                Encoding.UTF8.GetBytes("some test = test")
            ));
            response.Properties.Add(Property<byte[]>.Create(
                (uint)DnsRecordType.Txt,
                Encoding.UTF8.GetBytes("a record")
            ));
            response.Properties.Add(Property<byte[]>.Create(
                (uint)DnsRecordType.Txt,
                Encoding.UTF8.GetBytes("spasdfsdafsda")
            ));
            response.Properties.Add(Property<byte[]>.Create(
                (uint)DnsRecordType.Txt,
                Encoding.UTF8.GetBytes("asd=3433434")
            ));
            response.Properties.Add(Property<byte[]>.Create(
                (uint)DnsRecordType.Txt,
                Encoding.UTF8.GetBytes("FPOÜDFD")
            ));

            var stream = new MemoryStream();

            // Act
            response.Encode(stream, CodecId.Mpack);

            byte[] buf = Interop.TypeDecodeEncode(CodecId.Mpack, BrowseResponseType, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<BrowseResponse>(new MemoryStream(buf), CodecId.Mpack);

            // Assert
            Assert.IsTrue(response.Equals(returned));
        }

        [TestMethod]
        public void TestMpackBrowseResponseWriteRead2() {
            // Arrange
            var response = new BrowseResponse {
                Flags = 0,
                Handle = new Reference(),
                Item = new ProxySocketAddress {
                    Flags = 1,
                    Port = 1,
                    Host = "/test/test/test"
                },
                Error = 0
            };
            var stream = new MemoryStream();

            // Act
            response.Encode(stream, CodecId.Mpack);

            byte[] buf = Interop.TypeDecodeEncode(CodecId.Mpack, BrowseResponseType, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode <BrowseResponse>(new MemoryStream(buf), CodecId.Mpack);

            // Assert
            Assert.IsTrue(response.Equals(returned));
        }


        [TestMethod]
        public void TestJsonBrowseRequestWriteRead() {
            // Arrange
            var request = new BrowseRequest {
                Flags = 0,
                Handle = new Reference(),
                Item = new ProxySocketAddress {
                    Flags = 1,
                    Port = 1,
                    Host = ""
                },
                Type = BrowseRequest.Service
            };
            var stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Json);

            byte[] buf = Interop.TypeDecodeEncode(CodecId.Json, BrowseRequestType, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<BrowseRequest>(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestJsonBrowseResponseWriteRead1() {
            // Arrange
            var response = new BrowseResponse {
                Flags = 0,
                Handle = new Reference(),
                Item = new ProxySocketAddress {
                    Flags = 1,
                    Port = 1,
                    Host = "test._test._tcp.longdomain12345"
                },
                Error = 0
            };
            var stream = new MemoryStream();

            response.Properties.Add(Property<byte[]>.Create(
                (uint)DnsRecordType.Txt,
                Encoding.UTF8.GetBytes("some test = test")
            ));
            response.Properties.Add(Property<byte[]>.Create(
                (uint)DnsRecordType.Txt,
                Encoding.UTF8.GetBytes("a record")
            ));
            response.Properties.Add(Property<byte[]>.Create(
                (uint)DnsRecordType.Txt,
                Encoding.UTF8.GetBytes("spasdfsdafsda")
            ));
            response.Properties.Add(Property<byte[]>.Create(
                (uint)DnsRecordType.Txt,
                Encoding.UTF8.GetBytes("asd=3433434")
            ));
            response.Properties.Add(Property<byte[]>.Create(
                (uint)DnsRecordType.Txt,
                Encoding.UTF8.GetBytes("FPOÜDFD")
            ));
            response.Properties.Add(Property<byte[]>.Create(
                (uint)DnsRecordType.Txt,
                Encoding.UTF8.GetBytes("some test = test")
            ));
            response.Properties.Add(Property<byte[]>.Create(
                (uint)DnsRecordType.Txt,
                Encoding.UTF8.GetBytes("a record")
            ));
            response.Properties.Add(Property<byte[]>.Create(
                (uint)DnsRecordType.Txt,
                Encoding.UTF8.GetBytes("spasdfsdafsda")
            ));
            response.Properties.Add(Property<byte[]>.Create(
                (uint)DnsRecordType.Txt,
                Encoding.UTF8.GetBytes("asd=3433434")
            ));
            response.Properties.Add(Property<byte[]>.Create(
                (uint)DnsRecordType.Txt,
                Encoding.UTF8.GetBytes("FPOÜDFD")
            ));

            // Act
            response.Encode(stream, CodecId.Json);

            byte[] buf = Interop.TypeDecodeEncode(CodecId.Json, BrowseResponseType, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<BrowseResponse>(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(response.Equals(returned));
        }

        [TestMethod]
        public void TestJsonBrowseResponseWriteRead2() {
            // Arrange
            var response = new BrowseResponse {
                Flags = 0,
                Handle = new Reference(),
                Item = new ProxySocketAddress {
                    Flags = 1,
                    Port = 1,
                    Host = "/test/test/test"
                },
                Error = 0
            };
            var stream = new MemoryStream();

            // Act
            response.Encode(stream, CodecId.Json);

            byte[] buf = Interop.TypeDecodeEncode(CodecId.Json, BrowseResponseType, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<BrowseResponse>(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(response.Equals(returned));
        }
    }

    [TestClass]
    public class MessageTests {

        public byte[] buffer = new byte[0x4000];

        [TestMethod]
        public void TestMpackPingRequestWriteRead() {
            // Arrange
            var address = new ProxySocketAddress();
            address.Flags = 1;
            address.Port = 777;
            address.Host = "localhost";

            var request = Message.Create(null, null, null, PingRequest.Create(address));
            var stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Mpack);
            
            
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Mpack);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestMpackPingResponseWriteRead() {
            // Arrange
            var address = new ProxySocketAddress();
            address.Flags = 0xff;
            address.Port = 65500;
            address.Host = "rawrawrawraw";

            var args = PingResponse.Create(address);
            args.PhysicalAddress[3] = 3;
            args.PhysicalAddress[5] = 5;
            args.TimeMs = 1333333;

            var response = Message.Create(null, null, null, args);
            var stream = new MemoryStream();

            // Act
            response.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Mpack);

            // Assert
            Assert.IsTrue(response.Equals(returned));
        }

        [TestMethod]
        public void TestMpackLinkRequestWriteRead1() {
            // Arrange
            var address = new ProxySocketAddress();
            address.Port = 443;
            address.Host = "myhosttolinkto";

            var props = new SocketInfo();
            props.Address = address;
            props.Family = AddressFamily.InterNetwork;
            props.Protocol = ProtocolType.Udp;
            props.Type = SocketType.Stream;
            props.Flags = (uint)SocketFlags.Passive;
            // no options

            var request = Message.Create(null, null, null, LinkRequest.Create(props));
            var stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Mpack);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestMpackLinkRequestWriteRead2() {
            // Arrange
            var address = new ProxySocketAddress();
            address.Port = 443;
            address.Host = "myhosttolinkto";

            var props = new SocketInfo();
            props.Address = address;
            props.Family = AddressFamily.InterNetwork;
            props.Protocol = ProtocolType.Udp;
            props.Type = SocketType.Stream;
            props.Flags = (uint)SocketFlags.Passive;

            for (Int32 i = 0; i < 10; i++) {
                props.Options.Add(Property<ulong>.Create((uint)(13 - i), (ulong)i));
            }

            var request = Message.Create(null, null, null, LinkRequest.Create(props));
            var stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Mpack);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestMpackLinkRequestWriteRead3() {
            // Arrange
            var address = new ProxySocketAddress();
            address.Port = 1;
            address.Host = "";

            var props = new SocketInfo();
            props.Address = address;
            props.Family = AddressFamily.Proxy;
            props.Protocol = ProtocolType.Unspecified;
            props.Type = SocketType.Dgram;
            props.Flags = (uint)SocketFlags.Internal;

            for (Int32 i = 0; i < 3; i++) {
                props.Options.Add(Property<ulong>.Create((uint)(13 - i), (ulong)i));
            }

            props.Options.Add(Property<IMulticastOption>.Create((uint)SocketOption.IpMulticastJoin,
                new Inet4MulticastOption { InterfaceIndex = 5, Address = BitConverter.GetBytes((int)234) }));
            var ab = new byte[16];
            new Random().NextBytes(ab);
            props.Options.Add(Property<IMulticastOption>.Create((uint)SocketOption.IpMulticastLeave,
                new Inet6MulticastOption { InterfaceIndex = 5, Address = ab }));

            var request = Message.Create(null, null, null, LinkRequest.Create(props));
            var stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Mpack);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestMpackLinkResponseWriteRead() {
            // Arrange
            Inet4SocketAddress sal = new Inet4SocketAddress(0xffaabbcc, 9643);
            var ab = new byte[16];
            new Random().NextBytes(ab);
            Inet6SocketAddress sap = new Inet6SocketAddress(ab, 443, 1, 0);

            var args = LinkResponse.Create(new Reference(), sal, sap);
            var response = Message.Create(null, null, null, args);
            var stream = new MemoryStream();

            // Act
            response.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Mpack);
           
            // Assert
            Assert.IsTrue(response.Equals(returned));
        }

        [TestMethod]
        public void TestMpackSetOptRequestWriteRead() {
            // Arrange
            var optionValue = Property<ulong>.Create((uint)SocketOption.Broadcast, 1);
            var request = Message.Create(null, null, null, SetOptRequest.Create(optionValue));
            var stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Mpack);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestMpackSetOptResponseWriteRead() {
            // Arrange
            var response = Message.Create(null, null, null, SetOptResponse.Create());
            var stream = new MemoryStream();

            // Act
            response.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Mpack);
            
            // Assert
            Assert.IsTrue(response.Equals(returned));
        }

        [TestMethod]
        public void TestMpackGetOptRequestWriteRead() {
            // Arrange
            var request = Message.Create(null, null, null, GetOptRequest.Create(SocketOption.Broadcast));
            var stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Mpack);
 
            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestMpackGetOptResponseWriteRead() {
            // Arrange
            var optionValue = new Property<ulong>();
            optionValue.Type = (uint)SocketOption.Broadcast;
            optionValue.Value = 1;

            var response = Message.Create(null, null, null, GetOptResponse.Create(optionValue));
            var stream = new MemoryStream();

            // Act
            response.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Mpack);

            // Assert
            Assert.IsTrue(response.Equals(returned));
        }

        [TestMethod]
        public void TestMpackOpenRequestWriteRead() {

            // Arrange
            var args = OpenRequest.Create(new Reference(), (int)CodecId.Mpack,
                "dfksjaödfjkasdfölskajdfölsadfjkslöajksadlöjksdlöfsjkadflösdajkfösdlafj", 0, true);
            var request = Message.Create(null, null, null, args);
            var stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Mpack);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestMpackOpenResponseWriteRead() {

            // Arrange
            var response = Message.Create(null, null, null, OpenResponse.Create());
            var stream = new MemoryStream();

            // Act
            response.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Mpack);

            // Assert
            Assert.IsTrue(response.Equals(returned));
        }

        [TestMethod]
        public void TestMpackPollRequestWriteRead() {
            // Arrange
            var request = Message.Create(null, null, null, PollRequest.Create(100000));
            var stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Mpack);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestMpackPollResponseWriteRead() {
            // Arrange
            var response = Message.Create(null, null, null, PollResponse.Create());
            var stream = new MemoryStream();

            // Act
            response.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Mpack);

            // Assert
            Assert.IsTrue(response.Equals(returned));
        }

        [TestMethod]
        public void TestMpackDataWriteRead1() {
            // Arrange
            var source = new Inet4SocketAddress(0xffaabbcc, 9643);
            var args = DataMessage.Create(new byte[145], source, 12);
            new Random().NextBytes(args.Payload);

            var datagramIn = Message.Create(null, null, null, args);
            var stream = new MemoryStream();

            // Act
            datagramIn.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Mpack);

            // Assert
            Assert.IsTrue(datagramIn.Equals(returned));
        }

        [TestMethod]
        public void TestMpackDataWriteRead2() {

            // Arrange
            var args = DataMessage.Create(new byte[600], new AnySocketAddress(), new byte[50], 12);
            new Random().NextBytes(args.Payload);
            new Random().NextBytes(args.Control);

            var datagramIn = Message.Create(null, null, null, args);
            var stream = new MemoryStream();

            // Act
            datagramIn.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Mpack);

            // Assert
            Assert.IsTrue(datagramIn.Equals(returned));
        }

        [TestMethod]
        public void TestMpackOpenPlusDataWriteRead() {

            // Arrange
            var args1 = OpenRequest.Create(new Reference(), (int)CodecId.Mpack, 
                "dfksjaödfjkasdfölskajdfölsadfjkslöajksadlöjksdlöfsjkadflösdajkfösdlafj", 0, false);
            var args2 = DataMessage.Create(new byte[600], new AnySocketAddress(), 12);
            new Random().NextBytes(args2.Payload);

            var datagramIn1 = Message.Create(null, null, null, args1);
            var datagramIn2 = Message.Create(null, null, null, args2);
            var stream = new MemoryStream();

            // Act
            datagramIn1.Encode(stream, CodecId.Mpack);
            datagramIn2.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 2, stream.GetBuffer(), stream.Length, buffer, buffer.Length);

            stream = new MemoryStream(buf);
            Message returned1 = Serializable.Decode<Message>(stream, CodecId.Mpack);
            Message returned2 = Serializable.Decode<Message>(stream, CodecId.Mpack);

            // Assert
            Assert.IsTrue(datagramIn1.Equals(returned1));
            Assert.IsTrue(datagramIn2.Equals(returned2));
        }

        [TestMethod]
        public void TestMpackDataPlusDataWriteRead() {

            // Arrange
            var args1 = DataMessage.Create(new byte[145], new AnySocketAddress(), 11);
            new Random().NextBytes(args1.Payload);
            var args2 = DataMessage.Create(new byte[600], new AnySocketAddress(), 12);
            new Random().NextBytes(args2.Payload);
            var args3 = DataMessage.Create(new byte[400], new AnySocketAddress(), new byte[30], 13);
            new Random().NextBytes(args3.Payload);
            new Random().NextBytes(args3.Control);

            var datagramIn1 = Message.Create(null, null, null, args1);
            var datagramIn2 = Message.Create(null, null, null, args2);
            var datagramIn3 = Message.Create(null, null, null, args3);
            var stream = new MemoryStream();

            // Act
            datagramIn1.Encode(stream, CodecId.Mpack);
            datagramIn2.Encode(stream, CodecId.Mpack);
            datagramIn3.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 3, stream.GetBuffer(), stream.Length, buffer, buffer.Length);

            stream = new MemoryStream(buf);
            Message returned1 = Serializable.Decode<Message>(stream, CodecId.Mpack);
            Message returned2 = Serializable.Decode<Message>(stream, CodecId.Mpack);
            Message returned3 = Serializable.Decode<Message>(stream, CodecId.Mpack);

            // Assert
            Assert.IsTrue(datagramIn1.Equals(returned1));
            Assert.IsTrue(datagramIn2.Equals(returned2));
            Assert.IsTrue(datagramIn3.Equals(returned3));
        }

        [TestMethod]
        public void TestMpackCloseRequestWriteRead() {
            // Arrange
            var request = Message.Create(null, null, null, new CloseRequest());
            var stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Mpack);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestMpackCloseResponseWriteRead() {
            // Arrange
            var args = CloseResponse.Create(1000, 3000, 200, (int)SocketError.Closed);
            var response = Message.Create(null, null, null, args);
            var stream = new MemoryStream();

            // Act
            response.Encode(stream, CodecId.Mpack);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Mpack, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Mpack);

            // Assert
            Assert.IsTrue(response.Equals(returned));
        }

        [TestMethod]
        public void TestJsonPingRequestWriteRead() {
            // Arrange
            var address = new ProxySocketAddress();
            address.Flags = 1;
            address.Port = 777;
            address.Host = "localhost";

            var request = Message.Create(null, null, null, PingRequest.Create(address));
            var stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestJsonPingResponseWriteRead() {
            // Arrange
            var address = new ProxySocketAddress();
            address.Flags = 0xff;
            address.Port = 65500;
            address.Host = "rawrawrawraw";

            var args = PingResponse.Create(address);
            args.PhysicalAddress[3] = 3;
            args.PhysicalAddress[5] = 5;
            args.TimeMs = 1333333;

            var response = Message.Create(null, null, null, args);
            var stream = new MemoryStream();

            // Act
            response.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(response.Equals(returned));
        }
        
        [TestMethod]
        public void TestJsonLinkRequestWriteRead1() {
            // Arrange
            var address = new ProxySocketAddress();
            address.Port = 443;
            address.Host = "myhosttolinkto";

            var props = new SocketInfo();
            props.Address = address;
            props.Family = AddressFamily.InterNetwork;
            props.Protocol = ProtocolType.Udp;
            props.Type = SocketType.Stream;
            props.Flags = (UInt32)SocketFlags.Passive;
            // no options

            var request = Message.Create(null, null, null, LinkRequest.Create(props));
            var stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestJsonLinkRequestWriteRead2() {
            // Arrange
            var address = new ProxySocketAddress();
            address.Port = 443;
            address.Host = "myhosttolinkto";

            var props = new SocketInfo();
            props.Address = address;
            props.Family = AddressFamily.InterNetwork;
            props.Protocol = ProtocolType.Udp;
            props.Type = SocketType.Stream;
            props.Flags = (uint)SocketFlags.Passive;

            for (Int32 i = 0; i < 10; i++) {
                props.Options.Add(Property<ulong>.Create((uint)(13 - i), (ulong)i));
            }

            var request = Message.Create(null, null, null, LinkRequest.Create(props));
            var stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestJsonLinkRequestWriteRead3() {
            // Arrange
            var address = new ProxySocketAddress();
            address.Port = 1;
            address.Host = "";

            var props = new SocketInfo();
            props.Address = address;
            props.Family = AddressFamily.Proxy;
            props.Protocol = ProtocolType.Unspecified;
            props.Type = SocketType.Dgram;
            props.Flags = (uint)SocketFlags.Internal;

            for (Int32 i = 0; i < 3; i++) {
                props.Options.Add(Property<ulong>.Create((uint)(13 - i), (ulong)i));
            }

            props.Options.Add(Property<IMulticastOption>.Create((uint)SocketOption.IpMulticastJoin,
                new Inet4MulticastOption { InterfaceIndex = 5, Address = BitConverter.GetBytes((int)234) }));
            var ab = new byte[16];
            new Random().NextBytes(ab);
            props.Options.Add(Property<IMulticastOption>.Create((uint)SocketOption.IpMulticastLeave,
                new Inet6MulticastOption { InterfaceIndex = 5, Address = ab }));

            var request = Message.Create(null, null, null, LinkRequest.Create(props));
            var stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestJsonLinkResponseWriteRead() {
            // Arrange
            var sal = new Inet4SocketAddress(0xffaabbcc, 9643);
            var ab = new byte[16];
            new Random().NextBytes(ab);
            var sap = new Inet6SocketAddress(ab, 443, 1, 0);

            var args = LinkResponse.Create(new Reference(), sal, sap);
            var response = Message.Create(null, null, null, args);
            var stream = new MemoryStream();

            // Act
            response.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(response.Equals(returned));
        }

        [TestMethod]
        public void TestJsonSetOptRequestWriteRead() {
            // Arrange
            var optionValue = Property<ulong>.Create((uint)SocketOption.Broadcast, 1);
            var request = Message.Create(null, null, null, SetOptRequest.Create(optionValue));
            var stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestJsonSetOptResponseWriteRead() {
            // Arrange
            var response = Message.Create(null, null, null, SetOptResponse.Create());
            var stream = new MemoryStream();

            // Act
            response.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(response.Equals(returned));
        }

        [TestMethod]
        public void TestJsonGetOptRequestWriteRead() {
            // Arrange
            var request = Message.Create(null, null, null, GetOptRequest.Create(SocketOption.Broadcast));
            var stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestJsonGetOptResponseWriteRead() {
            // Arrange
            var optionValue = Property<ulong>.Create((uint)SocketOption.Broadcast, 1);
            var response = Message.Create(null, null, null, GetOptResponse.Create(optionValue));
            var stream = new MemoryStream();

            // Act
            response.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(response.Equals(returned));
        }

        [TestMethod]
        public void TestJsonOpenRequestWriteRead() {

            // Arrange
            var args = OpenRequest.Create(new Reference(), (int)CodecId.Mpack,
                "dfksjaödfjkasdfölskajdfölsadfjkslöajksadlöjksdlöfsjkadflösdajkfösdlafj", 0, true);
            var request = Message.Create(null, null, null, args);
            var stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestJsonOpenResponseWriteRead() {

            // Arrange
            var response = Message.Create(null, null, null, OpenResponse.Create());
            var stream = new MemoryStream();

            // Act
            response.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(response.Equals(returned));
        }

        [TestMethod]
        public void TestJsonPollRequestWriteRead() {
            // Arrange
            var request = Message.Create(null, null, null, PollRequest.Create(100000));
            var stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestJsonPollResponseWriteRead() {
            // Arrange
            var response = Message.Create(null, null, null, PollResponse.Create());
            var stream = new MemoryStream();

            // Act
            response.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(response.Equals(returned));
        }

        [TestMethod]
        public void TestJsonDataWriteRead1() {
            // Arrange
            var source = new Inet4SocketAddress(0xffaabbcc, 9643);
            var args = DataMessage.Create(new byte[145], source, 12);
            new Random().NextBytes(args.Payload);

            var datagramIn = Message.Create(null, null, null, args);
            var stream = new MemoryStream();

            // Act
            datagramIn.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(datagramIn.Equals(returned));
        }

        [TestMethod]
        public void TestJsonDataWriteRead2() {

            // Arrange
            var args = DataMessage.Create(new byte[600], new AnySocketAddress(), new byte[50], 12);
            new Random().NextBytes(args.Payload);
            new Random().NextBytes(args.Control);

            var datagramIn = Message.Create(null, null, null, args);
            var stream = new MemoryStream();

            // Act
            datagramIn.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(datagramIn.Equals(returned));
        }

        [TestMethod]
        public void TestJsonCloseRequestWriteRead() {
            // Arrange
            var request = Message.Create(null, null, null, new CloseRequest());
            var stream = new MemoryStream();

            // Act
            request.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(request.Equals(returned));
        }

        [TestMethod]
        public void TestJsonCloseResponseWriteRead() {
            // Arrange
            var args = CloseResponse.Create(1000, 3000, 200, (int)SocketError.Closed);
            var response = Message.Create(null, null, null, args);
            var stream = new MemoryStream();

            // Act
            response.Encode(stream, CodecId.Json);
            byte[] buf = Interop.MessageDecodeEncode(CodecId.Json, 1, stream.GetBuffer(), stream.Length, buffer, buffer.Length);
            var returned = Serializable.Decode<Message>(new MemoryStream(buf), CodecId.Json);

            // Assert
            Assert.IsTrue(response.Equals(returned));
        }
    } // class
}
