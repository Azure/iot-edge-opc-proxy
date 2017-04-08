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
        AlreadyExists,
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
        NoAddress,
        NoHost,
        HostUnknown,
        AddressFamily,
        Reserved2,
        BadFlags,
        InvalidFormat,
        DiskIo,
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