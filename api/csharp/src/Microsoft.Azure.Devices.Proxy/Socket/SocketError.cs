// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

// Keep in sync with native layer, in particular order of members!
namespace Microsoft.Azure.Devices.Proxy {
    /// <summary>
    /// Error numbers, e.g. as part of message
    /// </summary>
    public enum SocketError {
        Unknown = -1001,
        Fatal,
        Arg,
        Fault,
        BadState,
        OutOfMemory,
        Already_exists,
        NotFound,
        NotSupported,
        NotImpl,
        Permission,
        Retry,
        Nomore,
        Network,
        Connecting,
        Busy,
        Writing,
        Reading,
        Waiting,
        Timeout,
        Aborted,
        Closed,
        Shutdown,
        Refused,
        No_address,
        No_host,
        Host_unknown,
        Address_family,
        Reserved2,
        Bad_flags,
        Invalid_format,
        Disk_io,
        Reserved1,
        PropGet,
        PropSet,
        Reset,
        Undelivered,
        Crypto,
        Comm,
        Success = 0
    }
}