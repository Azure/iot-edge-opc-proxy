// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy.Model {
    //
    // Proxy protocol, defines messages sent to rpc socket communication.
    //
    using System;
    using System.Linq;
    using System.Runtime.Serialization;

    /// <summary>
    /// Message content
    /// </summary>
    public interface IMessageContent { }

    /// <summary>
    /// Response tag
    /// </summary>
    public interface IResponse : IMessageContent { };

    /// <summary>
    /// Request tag
    /// </summary>
    public interface IRequest : IMessageContent { };

    /// <summary>
    /// Type of message
    /// </summary>
    public static class MessageContent {

        public static readonly uint Ping            = 10;
        public static readonly uint Link            = 12;
        public static readonly uint SetOpt          = 13;
        public static readonly uint GetOpt          = 14;
        public static readonly uint Open            = 20;
        public static readonly uint Close           = 21;
        public static readonly uint Data            = 30;
        public static readonly uint Poll            = 31;
        public static readonly uint Custom          =  0;

        /// <summary>
        /// Returns id for type
        /// </summary>
        /// <param name="content"></param>
        /// <returns></returns>
        public static uint GetId(IMessageContent content) {
            /**/ if (content is DataMessage)
                return Data;
            else if (content is PingRequest)
                return Ping;
            else if (content is PollRequest)
                return Poll;
            else if (content is PingResponse)
                return Ping;
            else if (content is LinkRequest)
                return Link;
            else if (content is LinkResponse)
                return Link;
            else if (content is SetOptRequest)
                return SetOpt;
            else if (content is SetOptResponse)
                return SetOpt;
            else if (content is GetOptRequest)
                return GetOpt;
            else if (content is GetOptResponse)
                return GetOpt;
            else if (content is OpenRequest)
                return Open;
            else if (content is OpenResponse)
                return Open;
            else if (content is CloseRequest)
                return Close;
            else if (content is CloseResponse)
                return Close;
            else
                return Custom;
        }

        /// <summary>
        /// Returns whether the object is a response
        /// </summary>
        /// <param name="content"></param>
        /// <returns></returns>
        public static bool IsResponse(IMessageContent content) {
            return content is IResponse;
        }

        /// <summary>
        /// Returns id for type
        /// </summary>
        /// <param name="content"></param>
        /// <returns></returns>
        public static Type TypeOf(uint type, bool isResponse) {
            if (type == Ping)
                return isResponse ? typeof(PingResponse) : typeof(PingRequest);
            if (type == Link)
                return isResponse ? typeof(LinkResponse) : typeof(LinkRequest);
            if (type == SetOpt)
                return isResponse ? typeof(SetOptResponse) : typeof(SetOptRequest);
            if (type == GetOpt)
                return isResponse ? typeof(GetOptResponse) : typeof(GetOptRequest);
            if (type == Open)
                return isResponse ? typeof(OpenResponse) : typeof(OpenRequest);
            if (type == Close)
                return isResponse ? typeof(CloseResponse) : typeof(CloseRequest);
            if (type == Poll)
                return typeof(PollRequest);
            if (type == Data)
                return typeof(DataMessage);
            else
                return null;
        }
    }

    /// <summary>
    /// Ping request
    /// </summary>
    [DataContract]
    public class PingRequest : IMessageContent, IRequest, IEquatable<PingRequest> {

        /// <summary>
        /// Socket address to ping, typically proxy address
        /// </summary>
        [DataMember(Name = "address", Order = 1)]
        public SocketAddress SocketAddress { get; set; }

        /// <summary>
        /// Default constructor
        /// </summary>
        public PingRequest() {
            // Noop
        }

        /// <summary>
        /// Convinience constructor
        /// </summary>
        /// <param name="address"></param>
        public PingRequest(SocketAddress socketAddress) {
            this.SocketAddress = socketAddress;
        }


        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(PingRequest that) {
            if (that == null) {
                return false;
            }
            return
                this.SocketAddress.Equals(that.SocketAddress);
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(Object that) {
            return Equals(that as PingRequest);
        }

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return
                SocketAddress.GetHashCode();
        }
    }

    /// <summary>
    // Ping response
    /// </summary>
    [DataContract]
    public class PingResponse : IMessageContent, IResponse, IEquatable<PingResponse> {

        /// <summary>
        /// Address pingged, in form of local address
        /// </summary>
        [DataMember(Name = "address", Order = 1)]
        public SocketAddress SocketAddress { get; set; }

        /// <summary>
        /// Mac address of machine 
        /// </summary>
        [DataMember(Name = "physical_address", Order = 2)]
        public byte[] PhysicalAddress { get; set; } = new byte[8];

        /// <summary>
        /// Time ping took
        /// </summary>
        [DataMember(Name = "time_ms", Order = 3)]
        public uint TimeMs { get; set; }


        /// <summary>
        /// Default constructor
        /// </summary>
        public PingResponse() {
            // Noop
        }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="socketAddress"></param>
        public PingResponse(SocketAddress socketAddress) {
            SocketAddress = socketAddress;
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(PingResponse that) {
            if (that == null) {
                return false;
            }
            return
                this.SocketAddress.Equals(that.SocketAddress) &&
                this.PhysicalAddress.SequenceEqual(that.PhysicalAddress) &&
                this.TimeMs.Equals(that.TimeMs);
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(Object that) {
            return Equals(that as PingResponse);
        }

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return base.GetHashCode();
        }
    }

    /// <summary>
    /// Link request - create socket link 
    /// </summary>
    [DataContract]
    public class LinkRequest : IMessageContent, IRequest, IEquatable<LinkRequest> {

        public static readonly byte LINK_VERSION = 7;

        /// <summary>
        /// Version number
        /// </summary>
        [DataMember(Name = "version", Order = 1)]
        public Byte Version { get; set; }

        /// <summary>
        /// Socket Properties
        /// </summary>
        [DataMember(Name = "props", Order = 2)]
        public SocketInfo Properties { get; set; }

        /// <summary>
        /// Default constructor
        /// </summary>
        public LinkRequest() {
            // no-op
        }

        /// <summary>
        /// Convinience constructor
        /// </summary>
        /// <param name="properties"></param>
        public LinkRequest(SocketInfo properties) {
            Version = LINK_VERSION;
            Properties = properties;
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(LinkRequest that) {
            if (that == null) {
                return false;
            }
            return
                this.Version.Equals(that.Version) &&
                this.Properties.Equals(that.Properties);
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(Object that) {
            return Equals(that as LinkRequest);
        }

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return base.GetHashCode();
        }
    }

    /// <summary>
    // Link response or error
    /// </summary>
    [DataContract]
    public class LinkResponse : IMessageContent, IResponse, IEquatable<LinkResponse> {
        /// <summary>
        /// Remote link address 
        /// </summary>
        [DataMember(Name = "link_id", Order = 1)]
        public Reference LinkId { get; set; }

        /// <summary>
        /// Interface address assigned on remote
        /// </summary>
        [DataMember(Name = "local_address", Order = 2)]
        public SocketAddress LocalAddress { get; set; }

        /// <summary>
        /// Peer IP address proxy connected to
        /// </summary>
        [DataMember(Name = "peer_address", Order = 3)]
        public SocketAddress PeerAddress { get; set; }

        /// <summary>
        /// Default constructor
        /// </summary>
        public LinkResponse() {
            // no-op
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(LinkResponse that) {
            if (that == null) {
                return false;
            }
            return
                this.LinkId.Equals(that.LinkId) && 
                this.LocalAddress.Equals(that.LocalAddress) &&
                this.PeerAddress.Equals(that.PeerAddress);
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(Object that) {
            return Equals(that as LinkResponse);
        }

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return base.GetHashCode();
        }
    }

    /// <summary>
    /// Set option request
    /// </summary>
    [DataContract]
    public class SetOptRequest : IMessageContent, IRequest, IEquatable<SetOptRequest> {

        /// <summary>
        /// Option and Value to set
        /// </summary>
        [DataMember(Name = "so_val", Order = 1)]
        public PropertyBase OptionValue { get; set; }

        /// <summary>
        /// Default constructor
        /// </summary>
        public SetOptRequest() {
            // no-op
        }

        /// <summary>
        /// Convinience constructor
        /// </summary>
        public SetOptRequest(PropertyBase optionValue) {
            OptionValue = optionValue;
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(SetOptRequest that) {
            if (that == null) {
                return false;
            }
            return
                this.OptionValue.Equals(that.OptionValue);
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(Object that) {
            return Equals(that as SetOptRequest);
        }

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return base.GetHashCode();
        }
    }

    /// <summary>
    /// Void response for set option requests
    /// </summary>
    [DataContract]
    public class SetOptResponse : VoidMessage, IResponse {
    }

    /// <summary>
    // Get option request, request value for specified option
    /// </summary>
    [DataContract]
    public class GetOptRequest : IMessageContent, IRequest, IEquatable<GetOptRequest> {

        /// <summary>
        /// Option to get value for
        /// </summary>
        [DataMember(Name = "so_opt", Order = 1)]
        public SocketOption Option { get; set; }

        /// <summary>
        /// Default constructor
        /// </summary>
        public GetOptRequest() {
            // no-op
        }

        /// <summary>
        /// Convinience constructor
        /// </summary>
        public GetOptRequest(SocketOption option) {
            Option = option;
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(GetOptRequest that) {
            if (that == null) {
                return false;
            }
            return
                this.Option.Equals(that.Option);
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(Object that) {
            return Equals(that as GetOptRequest);
        }

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return base.GetHashCode();
        }
    }

    /// <summary>
    /// Get option response, returns option value
    /// </summary>
    [DataContract]
    public class GetOptResponse : IMessageContent, IResponse, IEquatable<GetOptResponse> {

        /// <summary>
        /// Option value returned
        /// </summary>
        [DataMember(Name = "so_val", Order = 1)]
        public PropertyBase OptionValue { get; set; }

        /// <summary>
        /// Default constructor
        /// </summary>
        public GetOptResponse() {
            // no-op
        }

        /// <summary>
        /// Convinience constructor
        /// </summary>
        public GetOptResponse(PropertyBase optionValue) {
            OptionValue = optionValue;
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(GetOptResponse that) {
            if (that == null) {
                return false;
            }
            return
                this.OptionValue.Equals(that.OptionValue);
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(Object that) {
            return Equals(that as GetOptResponse);
        }

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return base.GetHashCode();
        }
    }

    /// <summary>
    /// Open request - opens the link or updates credits
    /// </summary>
    [DataContract]
    public class OpenRequest : IMessageContent, IRequest, IEquatable<OpenRequest> {

        /// <summary>
        /// Subscribed channel for data messages
        /// </summary>
        [DataMember(Name = "stream_id", Order = 1)]
        public Reference StreamId { get; set; }

        /// <summary>
        /// What type of connection string is used - default 0
        /// </summary>
        [DataMember(Name = "type", Order = 2)]
        public int Type { get; set; }

        /// <summary>
        /// Send credit, causes receive up to credit exhaustion
        /// </summary>
        [DataMember(Name = "connection-string", Order = 3)]
        public string ConnectionString { get; set; }

        /// <summary>
        /// Whether the stream is going to be polled
        /// </summary>
        [DataMember(Name = "polled", Order = 4)]
        public bool IsPolled { get; set; }

        /// <summary>
        /// Largest receive buffer (to us) = default 0 = auto
        /// </summary>
        [DataMember(Name = "max_recv", Order = 5)]
        public uint MaxReceiveBuffer { get; set; } = 0;

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(OpenRequest that) {
            if (that == null) {
                return false;
            }
            return
                this.StreamId.Equals(that.StreamId) &&
                this.Type == that.Type &&
                this.IsPolled == that.IsPolled &&
                this.ConnectionString.Equals(that.ConnectionString);
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(Object that) {
            return Equals(that as OpenRequest);
        }

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return base.GetHashCode();
        }
    }

    /// <summary>
    /// Void response for open requests
    /// </summary>
    [DataContract]
    public class OpenResponse : VoidMessage, IResponse {
    }

    /// <summary>
    /// Request to poll data
    /// </summary>
    [DataContract]
    public class PollRequest : IMessageContent, IRequest, IEquatable<PollRequest> {

        /// <summary>
        /// How long to wait in milliseconds
        /// </summary>
        [DataMember(Name = "timeout", Order = 1)]
        public ulong Timeout { get; set; }

        /// <summary>
        /// Default constructor
        /// </summary>
        /// <param name="timeout"></param>
        public PollRequest() {
            Timeout = 60000;
        }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="timeout"></param>
        public PollRequest(ulong timeout) {
            Timeout = timeout;
        }

        /// <summary>
        /// Returns whether 2 request are equal.
        /// </summary>
        /// <param name="other"></param>
        /// <returns></returns>
        public bool Equals(PollRequest that) {
            if (that == null) {
                return false;
            }
            return
                this.Timeout.Equals(that.Timeout);
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(Object that) {
            return Equals(that as PollRequest);
        }

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return base.GetHashCode();
        }
    }

    /// <summary>
    /// Void response for poll requests
    /// </summary>
    [DataContract]
    public class PollResponse : VoidMessage, IResponse {
    }

    /// <summary>
    /// Stream data message payload
    /// </summary>
    [DataContract]
    public class DataMessage : IMessageContent, IEquatable<DataMessage> {

        /// <summary>
        /// Default constructor
        /// </summary>
        public DataMessage() {
            // no op
        }

        public DataMessage(byte[] payload, SocketAddress endpoint = null) {
            Payload = payload;
            Source = endpoint;
            Control = new byte[0];
        }

        /// <summary>
        /// Convinience constructor
        /// </summary>
        /// <param name="buffer"></param>
        /// <param name="endpoint"></param>
        public DataMessage(ArraySegment<byte> buffer, SocketAddress endpoint) {
            Payload = new byte[buffer.Count];
            Buffer.BlockCopy(buffer.Array, buffer.Offset, Payload, 0, buffer.Count);
            Source = endpoint;
            Control = new byte[0];
        }

        /// <summary>
        /// Source address if udp socket
        /// </summary>
        [DataMember(Name = "source_address", Order = 1)]
        public SocketAddress Source { get; set; }

        /// <summary>
        /// Buffer content
        /// </summary>
        [DataMember(Name = "buffer", Order = 2)]
        public byte[] Payload { get; set; }

        /// <summary>
        /// Control buffer
        /// </summary>
        [DataMember(Name = "control_buffer", Order = 3)]
        public byte[] Control { get; set; }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(DataMessage that) {
            if (that == null) {
                return false;
            }
            return
                this.Payload.SequenceEqual(that.Payload) &&
                this.Control.SequenceEqual(that.Control) &&
                this.Source.Equals(that.Source);
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(Object that) {
            return Equals(that as DataMessage);
        }

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return base.GetHashCode();
        }
    }

    /// <summary>
    /// Request to close a link identified by message channel id
    /// </summary>
    [DataContract]
    public class CloseRequest : VoidMessage, IRequest {
    }

    /// <summary>
    /// Response for close request, or notification when close occurred
    /// </summary>
    [DataContract]
    public class CloseResponse : IMessageContent, IResponse, IEquatable<CloseResponse> {
        /// <summary>
        /// How long the link was open
        /// </summary>
        [DataMember(Name = "time_open", Order = 1)]
        public ulong TimeOpenInMilliseconds { get; set; }

        /// <summary>
        /// How many bytes were sent
        /// </summary>
        [DataMember(Name = "bytes_sent", Order = 2)]
        public ulong BytesSent { get; set; }

        /// <summary>
        /// How many received
        /// </summary>
        [DataMember(Name = "bytes_received", Order = 3)]
        public ulong BytesReceived { get; set; }

        /// <summary>
        /// Last error code on the link, e.g. reason link closed.
        /// </summary>
        [DataMember(Name = "error_code", Order = 4)]
        public int ErrorCode { get; set; }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(CloseResponse that) {
            if (that == null) {
                return false;
            }
            return
                this.TimeOpenInMilliseconds.Equals(that.TimeOpenInMilliseconds) &&
                this.BytesSent.Equals(that.BytesSent) &&
                this.BytesReceived.Equals(that.BytesReceived) &&
                this.ErrorCode.Equals(that.ErrorCode);
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(Object that) {
            return Equals(that as CloseResponse);
        }

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return base.GetHashCode();
        }
    }

    /// <summary>
    /// Void args or return
    /// </summary>
    [DataContract]
    public abstract class VoidMessage : IMessageContent, IEquatable<VoidMessage> {
        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public bool Equals(VoidMessage that) {
            return true;
        }

        /// <summary>
        /// Comparison
        /// </summary>
        /// <param name="that"></param>
        /// <returns></returns>
        public override bool Equals(Object that) {
            return true;
        }

        /// <summary>
        /// Returns hash for efficient lookup in list
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode() {
            return 12345678;
        }
    }
}