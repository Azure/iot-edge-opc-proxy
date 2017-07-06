// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!

namespace Microsoft.Azure.Devices.Proxy {
    /// <summary>
    /// Socket option name for set and get
    /// </summary>
    public enum SocketOption {
        Unknown,
        Nonblocking,
        Available,
        Shutdown,
        Debug,
        Acceptconn,
        Reuseaddr,
        Keepalive,
        Dontroute,
        Broadcast,
        Linger,
        Oobinline,
        Sndbuf,
        Rcvbuf,
        Sndlowat,
        Rcvlowat,
        Sndtimeo,
        Rcvtimeo,
        Error,
        Type,
        IpOptions,
        IpHdrincl,
        IpTos,
        IpTtl,
        IpMulticasTtl,
        IpMulticastLoop,
        IpPktInfo,
        Ipv6Hoplimit,
        Ipv6ProtectionLevel,
        Ipv6Only,
        TcpNodelay,

        IpMulticastJoin, 
        IpMulticastLeave,

        // ...

        PropsTimeout,
        __prx_so_max
    }
}