// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy {
    using System;

    public class SocketException : Exception {

        public SocketError Error { get; private set; }

        public SocketException(string message, Exception e) 
            : base(message, e) {
            Error = SocketError.Fatal;
        }

        public SocketException(string message, 
            SocketError errorCode = SocketError.Fatal)
            : base(message) {
            Error = errorCode;
        }

        public SocketException(SocketError errorCode)
            : this(errorCode.ToString(), errorCode) {
        }

        public SocketException(Exception e)
            : this(e.ToString(), e) {
        }

        public SocketException(AggregateException e)
            : this(e.GetCombinedExceptionMessage(), e.Flatten()) {
        }
    }
}
