// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

namespace Microsoft.Azure.Devices.Proxy {
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Diagnostics.Tracing;
    using System.Globalization;
    using Microsoft.Azure.Devices.Proxy;

    /// <summary>
    /// EventSource for the new Dynamic EventSource type of Microsoft-Azure-Devices-Proxy traces.
    /// </summary>
    [EventSource(Name = "Microsoft-Azure-Devices-Proxy")]
    public class ProxyEventSource : EventSource {
        public static readonly ProxyEventSource Log = new ProxyEventSource();

        // Prevent additional instances other than ProxyEventSource.Log
        ProxyEventSource() {
        }

        public class Keywords   // This is a bitvector
        {
            public const EventKeywords Client = (EventKeywords)0x0001;
            public const EventKeywords Proxy = (EventKeywords)0x0002;
        }


        [Event(40190, Message = "Listener started: Source: {0}.")]
        public void LocalListenerStarted(object source) {
            if (this.IsEnabled()) {
                this.WriteEvent(40190, CreateSourceString(source));
            }
            Trace.TraceInformation($"Listener started ({source}).");
        }

        [Event(40191, Message = "Connection accepted: {0}.")]
        public void ConnectionAccepted(object source) {
            if (this.IsEnabled()) {
                this.WriteEvent(40191, CreateSourceString(source));
            }
        }

        [Event(40192, Message = "Connection rejected: {0} - {1}.")]
        public void ConnectionRejected(object source, Exception e) {
            if (this.IsEnabled()) {
                this.WriteEvent(40192, CreateSourceString(source), ExceptionToString(e));
            }
            Trace.TraceError($"Connection rejected... ({source}). {ExceptionToString(e)}");
        }

        [Event(40193, Message = "Stream Exception: {0} - {1} - {2}.")]
        public void StreamException(object source, object stream, Exception e) {
            if (this.IsEnabled()) {
                this.WriteEvent(40193, CreateSourceString(source), 
                    CreateSourceString(stream), ExceptionToString(e));
            }
            Trace.TraceInformation($"Stream error: {stream} ... ({source}). {ExceptionToString(e)}");
        }

        [Event(40194, Message = "Stream opened: {0} - {1}.")]
        public void StreamOpened(object source, object stream) {
            if (this.IsEnabled()) {
                this.WriteEvent(40194, CreateSourceString(source), CreateSourceString(stream));
            }
            Trace.TraceInformation($"Stream opened: {stream} ... ({source}).");
        }

        [Event(40195, Message = "Stream closing: {0} - {1}.")]
        public void StreamClosing(object source, object stream) {
            if (this.IsEnabled()) {
                this.WriteEvent(40195, CreateSourceString(source), CreateSourceString(stream));
            }
            Trace.TraceInformation($"Stream closing: {stream} ... ({source}).");
        }

        [Event(40196, Message = "Stream closed: {0} - {1}.")]
        public void StreamClosed(object source, object stream) {
            if (this.IsEnabled()) {
                this.WriteEvent(40196, CreateSourceString(source), CreateSourceString(stream));
            }
            Trace.TraceInformation($"Stream closed: {stream} ... ({source}).");
        }

        // 40197 - 40198 Available

        [Event(40199, Message = "Listener closed: Source: {0}.")]
        public void LocalListenerClosed(object source) {
            if (this.IsEnabled()) {
                this.WriteEvent(40199, CreateSourceString(source));
            }
            Trace.TraceInformation($"Listener closed ({source}).");
        }


        // 40200 - 40220 Available

        [Event(40221, Message = "{0} A ping to {1} did not find {2} ({3}).")]
        public void PingFailure(object source, object proxy, object address, object response) {
            if (this.IsEnabled()) {
                this.WriteEvent(40221, CreateSourceString(source), proxy, address);
            }
            Trace.TraceInformation($"A ping to {proxy} did not find {address} ({response})...");
        }

        [Event(40222, Message = "{0} start linking through {1} to {2}.")]
        public void LinkCreate(object source, object proxy, object info) {
            if (this.IsEnabled()) {
                this.WriteEvent(40222, CreateSourceString(source), proxy, info);
            }
            Trace.TraceInformation($"Begin linking through {proxy} to {info}...");
        }

        [Event(40223, Message = "{0} opening link through {1} to {2}.")]
        public void LinkOpen(object source, object proxy, object info) {
            if (this.IsEnabled()) {
                this.WriteEvent(40223, CreateSourceString(source), proxy, info);
            }
            Trace.TraceInformation($"Opening link through {proxy} to {info}...");
        }

        [Event(40224, Message = "{0} failed to link through {1} to {2}")]
        public void LinkFailure(object source, object proxy, object info) {
            if (this.IsEnabled()) {
                this.WriteEvent(40224, CreateSourceString(source), proxy, info);
            }
            Trace.TraceError($"Failed to link through {proxy} to {info} ...");
        }

        [Event(40225, Message = "{0} linked through {1} to {2}.")]
        public void LinkComplete(object source, object proxy, object info) {
            if (this.IsEnabled()) {
                this.WriteEvent(40225, CreateSourceString(source), proxy, info);
            }
            Trace.TraceInformation($"Linked through {proxy} to {info}...");
        }

        [Event(40226, Message = "{0} record {1} added...")]
        public void RecordAdded(object source, object record) {
            if (this.IsEnabled()) {
                this.WriteEvent(40226, CreateSourceString(source), record);
            }
            Trace.TraceInformation($"Record {record} added...");
        }

        [Event(40227, Message = "{0} patching record {1} : {2}...")]
        public void PatchingRecord(object source, object record, object patch) {
            if (this.IsEnabled()) {
                this.WriteEvent(40227, CreateSourceString(source), record, patch);
            }
        }

        [Event(40228, Message = "{0} record {1} patched : {2}.")]
        public void RecordPatched(object source, object record, object patch) {
            if (this.IsEnabled()) {
                this.WriteEvent(40228, CreateSourceString(source), record, patch);
            }
            Trace.TraceInformation($"Record {record} patched: {patch}...");
        }

        [Event(40229, Message = "{0} record {1} removed.")]
        public void RecordRemoved(object source, object record) {
            if (this.IsEnabled()) {
                this.WriteEvent(40229, CreateSourceString(source), record);
            }
            Trace.TraceInformation($"Record {record} removed...");
        }

        // 40230 - 40246 Available

        [Event(40247, Message = "{0} destroyed.")]
        public void ObjectDestroyed(object source)
        {
            if (this.IsEnabled())
            {
                this.WriteEvent(40247, CreateSourceString(source));
            }
            Trace.TraceInformation($"{source} destroyed...");
        }

        [Event(40248, Level = EventLevel.Warning, Message = "{0} Retry {1} after exception: {2}")]
        public void Retry(object source, int k, Exception ex) {
            if (this.IsEnabled()) {
                this.WriteEvent(40248, source, k, ExceptionToString(ex));
            }
            Trace.TraceInformation($"{source} Retry {k} after exception: {ex.ToString()}");
        }

        // Not the actual event definition since we're using object and Exception types
        [NonEvent]
        public void HandledExceptionAsInformation(object source, Exception exception) {
            this.HandledExceptionAsInformation(CreateSourceString(source), ExceptionToString(exception));
        }

        [Event(40249, Message = "{0} Handled Exception: {1}")]
        void HandledExceptionAsInformation(string source, string exception) {
            if (this.IsEnabled()) {
                this.WriteEvent(40249, source, exception);
            }
            Trace.TraceInformation($"INFO CONTINUE: {exception} ({source}).");
        }

        // Not the actual event definition since we're using object and Exception types
        [NonEvent]
        public void HandledExceptionAsWarning(object source, Exception exception) {
            this.HandledExceptionAsWarning(CreateSourceString(source), ExceptionToString(exception));
        }

        [Event(40250, Level = EventLevel.Warning, Message = "{0} Handled Exception: {1}")]
        void HandledExceptionAsWarning(string source, string exception) {
            if (this.IsEnabled()) {
                this.WriteEvent(40250, source, exception);
            }
            Trace.TraceWarning($"WARNING CONTINUE: {exception} ({source}).");
        }

        // Not the actual event definition since we're using object and Exception types
        [NonEvent]
        public void HandledExceptionAsError(object source, Exception exception) {
            this.HandledExceptionAsError(CreateSourceString(source), ExceptionToString(exception));
        }

        [Event(40251, Level = EventLevel.Error, Message = "{0} Handled Exception: {1}")]
        void HandledExceptionAsError(string source, string exception) {
            if (this.IsEnabled()) {
                this.WriteEvent(40251, source, exception);
            }
            Trace.TraceError($"ERROR CONTINUE: {exception} ({source}).");
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
                if (exception is OperationCanceledException) {
                    // Do not pollute log with cancellation exceptions
                    level = EventLevel.Verbose;
                }
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
            return exception;
        }

        [Event(40262, Level = EventLevel.Error, Message = "{0} Throwing an Exception: {1}")]
        public void ThrowingExceptionError(string source, string exception) {
            if (this.IsEnabled()) {
                this.WriteEvent(40262, source, exception);
            }
        }

        [Event(40263, Level = EventLevel.Warning, Message = "{0} Throwing an Exception: {1}")]
        public void ThrowingExceptionWarning(string source, string exception) {
            if (this.IsEnabled()) {
                this.WriteEvent(40263, source, exception);
            }
        }

        [Event(40264, Level = EventLevel.Informational, Message = "{0} Throwing an Exception: {1}")]
        public void ThrowingExceptionInfo(string source, string exception) {
            if (this.IsEnabled()) {
                this.WriteEvent(40264, source, exception);
            }
        }

        [Event(40265, Level = EventLevel.Verbose , Message = "{0}")]
        public void TraceVerbose(string message) {
            if (this.IsEnabled()) {
                this.WriteEvent(40265, message);
            }
            Trace.WriteLine(message);
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
