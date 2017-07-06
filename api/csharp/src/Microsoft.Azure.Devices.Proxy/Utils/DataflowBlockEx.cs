// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace System.Threading.Tasks.Dataflow {
    using System;

    public static class DataflowBlockEx {

        public static bool Push<T>(this ITargetBlock<DataflowMessage<T>> block,
            DataflowMessage<T> message, Exception exception) {
            message.Exceptions.AddFirst(exception);
            return block.Post(message);
        }

        public static bool Push<T>(this ITargetBlock<DataflowMessage<T>> block,
            DataflowMessage<T> message) {
            message.Exceptions.Clear();
            return block.Post(message);
        }

        private static DataflowLinkOptions _propagateOption = new DataflowLinkOptions { 
            PropagateCompletion = true,
            Append = true
        };

        public static IDisposable ConnectTo<TOutput>(
            this ISourceBlock<TOutput> source, ITargetBlock<TOutput> target) {
            return source.LinkTo(target, _propagateOption
#if DEBUG_PIPE
                ,(message) => {
                    System.Diagnostics.Trace.TraceInformation(
                        $"  -- {message.ToString()} from {source} to {target}");
                    return true;
                }
#endif
            );
        }

        public static IPropagatorBlock<TInput, TOutput> Encapsulate<TInput, TOutput>(
            ITargetBlock<TInput> target, ISourceBlock<TOutput> source) =>
            new EncapsulatingPropagator<TInput, TOutput>(target, source);

        private sealed class EncapsulatingPropagator<TInput, TOutput> : 
            IPropagatorBlock<TInput, TOutput> {

            public EncapsulatingPropagator(ITargetBlock<TInput> target, ISourceBlock<TOutput> source) {
                _target = target ?? throw new ArgumentNullException(nameof(target));
                _source = source ?? throw new ArgumentNullException(nameof(source));
            }

            public void Complete() => _target.Complete();

            void IDataflowBlock.Fault(Exception exception) =>
                _target.Fault(exception ?? throw new ArgumentNullException(nameof(exception)));

            public DataflowMessageStatus OfferMessage(DataflowMessageHeader messageHeader,
                TInput messageValue, ISourceBlock<TInput> source, bool consumeToAccept) =>
                _target.OfferMessage(messageHeader, messageValue, source, consumeToAccept);

            public Task Completion { get => _source.Completion; }

            public IDisposable LinkTo(ITargetBlock<TOutput> target, DataflowLinkOptions linkOptions) =>
                _source.LinkTo(target, linkOptions);

            public override string ToString() => _source.ToString();

            public TOutput ConsumeMessage(DataflowMessageHeader messageHeader,
                ITargetBlock<TOutput> target, out Boolean messageConsumed) =>
                _source.ConsumeMessage(messageHeader, target, out messageConsumed);

            public bool ReserveMessage(DataflowMessageHeader messageHeader, ITargetBlock<TOutput> target) =>
                _source.ReserveMessage(messageHeader, target);

            public void ReleaseReservation(DataflowMessageHeader messageHeader, ITargetBlock<TOutput> target) =>
                _source.ReleaseReservation(messageHeader, target);

            private readonly ITargetBlock<TInput> _target;
            private readonly ISourceBlock<TOutput> _source;
        }
    }
}