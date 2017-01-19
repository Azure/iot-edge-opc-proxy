// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Diagnostics.Tracing;
    using System.Globalization;
    using System.IO;
    using System.Threading.Tasks;
    using Provider;
    using Model;
    using Relay;

    /// <summary>
    /// EventSource for the new Dynamic EventSource type of Microsoft-Azure-Devices-Proxy traces.
    /// </summary>
    [EventSource(Name = "Microsoft-Azure-Devices-Proxy")]
    internal class ProxyEventSource : EventSource {
        public static readonly ProxyEventSource Log = new ProxyEventSource();

        // Prevent additional instances other than ProxyEventSource.Log
        ProxyEventSource() {
        }

        public class Keywords   // This is a bitvector
        {
            //public const EventKeywords Client = (EventKeywords)0x0001;
            //public const EventKeywords Proxy = (EventKeywords)0x0002;
        }


        [Event(40191, Message = "Stream accepted and connected: Target: {0}.")]
        public void StreamAcceptedAndConnected(object source) {
            if (this.IsEnabled()) {
                this.WriteEvent(40191, CreateSourceString(source));
            }
        }

        [Event(40192, Message = "Stream accepted but not connected: Target: {0}.")]
        public void StreamAcceptedNotConnected(object source) {
            if (this.IsEnabled()) {
                this.WriteEvent(40192, CreateSourceString(source));
            }
        }

        [Event(40193, Message = "Stream rejected: Target: {0}.")]
        public void StreamRejected(object source) {
            if (this.IsEnabled()) {
                this.WriteEvent(40193, CreateSourceString(source));
            }
        }

        [Event(40194, Message = "Remote proxy closed: Exception: {0}.")]
        public void RemoteProxyClosed(object source, Exception e) {
            if (this.IsEnabled()) {
                this.WriteEvent(40199, CreateSourceString(source), ExceptionToString(e));
            }
        }

        // 40195 - 40199 Available

        [Event(40200, Message = "Listener started: Source: {0}.")]
        public void LocalListenerStarted(object source) {
            if (this.IsEnabled()) {
                this.WriteEvent(40200, CreateSourceString(source));
            }
        }

        [Event(40201, Message = "Listener accepted message: Source: {0}, Connection {1}.")]
        public void ConnectionAccepted(object source, HybridConnectionStream relayConnection) {
            if (this.IsEnabled()) {
                this.WriteEvent(40201, CreateSourceString(source), relayConnection.ToString());
            }
        }

        [Event(40202, Message = "Listener closed: Source: {0}.")]
        public void LocalListenerClosed(object source) {
            if (this.IsEnabled()) {
                this.WriteEvent(40202, CreateSourceString(source));
            }
        }

        // 40203 - 40247 Available

        [Event(40248, Level = EventLevel.Warning, Message = "{0} Retrying: {1}")]
        public void Retry(object source, int k) {
            if (this.IsEnabled()) {
                this.WriteEvent(40248, source, k);
            }
        }

        // Not the actual event definition since we're using object and Exception types
        [NonEvent]
        public void HandledExceptionAsInformation(object source, Exception exception) {
            if (this.IsEnabled()) {
                this.HandledExceptionAsInformation(CreateSourceString(source), ExceptionToString(exception));
            }
        }

        [Event(40249, Message = "{0} Handled Exception: {1}")]
        void HandledExceptionAsInformation(string source, string exception) {
            this.WriteEvent(40249, source, exception);
        }

        // Not the actual event definition since we're using object and Exception types
        [NonEvent]
        public void HandledExceptionAsWarning(object source, Exception exception) {
            if (this.IsEnabled()) {
                this.HandledExceptionAsWarning(CreateSourceString(source), ExceptionToString(exception));
            }
        }

        [Event(40250, Level = EventLevel.Warning, Message = "{0} Handled Exception: {1}")]
        void HandledExceptionAsWarning(string source, string exception) {
            this.WriteEvent(40250, source, exception);
        }

        // Not the actual event definition since we're using object and Exception types
        [NonEvent]
        public void HandledExceptionAsError(object source, Exception exception) {
            if (this.IsEnabled()) {
                this.HandledExceptionAsError(CreateSourceString(source), ExceptionToString(exception));
            }
        }

        [Event(40251, Level = EventLevel.Error, Message = "{0} Handled Exception: {1}")]
        void HandledExceptionAsError(string source, string exception) {
            this.WriteEvent(40251, source, exception);
        }

        [NonEvent]
        public void GetTokenStart(object source) {
            if (this.IsEnabled()) {
                this.GetTokenStart(CreateSourceString(source));
            }
        }

        [Event(40255, Level = EventLevel.Informational, Message = "GetToken start. Source: {0}")]
        void GetTokenStart(string source) {
            this.WriteEvent(40255, source);
        }

        [NonEvent]
        public void GetTokenStop(DateTime tokenExpiry) {
            if (this.IsEnabled()) {
                this.GetTokenStop(DateTimeToString(tokenExpiry));
            }
        }

        [Event(40256, Level = EventLevel.Informational, Message = "GetToken stop. New token expires at {0}.")]
        void GetTokenStop(string tokenExpiry) {
            this.WriteEvent(40256, tokenExpiry);
        }


        [NonEvent]
        public ArgumentNullException ArgumentNull(string paramName, object source = null, EventLevel level = EventLevel.Error) {
            return this.Rethrow(new ArgumentNullException(paramName), source, level);
        }

        [NonEvent]
        public ArgumentException Argument(string paramName, string message, object source = null, EventLevel level = EventLevel.Error) {
            return this.Rethrow(new ArgumentException(message, paramName), source, level);
        }

        [NonEvent]
        public ArgumentOutOfRangeException ArgumentOutOfRange(string paramName, object actualValue, string message, object source = null, EventLevel level = EventLevel.Error) {
            return this.Rethrow(new ArgumentOutOfRangeException(paramName, actualValue, message), source, level);
        }

        [NonEvent]
        public TimeoutException Timeout(string message, object source = null, EventLevel level = EventLevel.Error) {
            return this.Rethrow(new TimeoutException(message), source, level);
        }

        [NonEvent]
        public TException Rethrow<TException>(TException exception, object source = null, EventLevel level = EventLevel.Error)
            where TException : Exception {
            // Avoid converting ToString, etc. if ETW tracing is not enabled.
            if (this.IsEnabled()) {
                switch (level) {
                    case EventLevel.Critical:
                    case EventLevel.LogAlways:
                    case EventLevel.Error:
                        this.ThrowingExceptionError(CreateSourceString(source), ExceptionToString(exception));
                        break;
                    case EventLevel.Warning:
                        this.ThrowingExceptionWarning(CreateSourceString(source), ExceptionToString(exception));
                        break;
                    case EventLevel.Informational:
                    case EventLevel.Verbose:
                    default:
                        this.ThrowingExceptionInfo(CreateSourceString(source), ExceptionToString(exception));
                        break;
                }
            }

            // This allows "throw ServiceBusEventSource.Log.ThrowingException(..."
            return exception;
        }

        [Event(40262, Level = EventLevel.Error, Message = "{0} Throwing an Exception: {1}")]
        void ThrowingExceptionError(string source, string exception) {
            // The IsEnabled() check is in the [NonEvent] Wrapper method
            this.WriteEvent(40262, source, exception);
        }

        [Event(40263, Level = EventLevel.Warning, Message = "{0} Throwing an Exception: {1}")]
        void ThrowingExceptionWarning(string source, string exception) {
            // The IsEnabled() check is in the [NonEvent] Wrapper method
            this.WriteEvent(40263, source, exception);
        }

        [Event(40264, Level = EventLevel.Informational, Message = "{0} Throwing an Exception: {1}")]
        void ThrowingExceptionInfo(string source, string exception) {
            // The IsEnabled() check is in the [NonEvent] Wrapper method
            this.WriteEvent(40264, source, exception);
        }

        [NonEvent]
        internal static string CreateSourceString(object source) {
            Type type;
            string s;
            if (source == null) {
                return string.Empty;
            }
            else if ((type = source as Type) != null) {
                return type.Name;
            }
            else if ((s = source as string) != null) {
                return s;
            }

            return source.ToString();
        }

        [NonEvent]
        static string ExceptionToString(Exception exception) {
            return exception?.ToString() ?? string.Empty;
        }

        [NonEvent]
        static string DateTimeToString(DateTime dateTime) {
            return dateTime.ToString(CultureInfo.InvariantCulture);
        }
    }
}
