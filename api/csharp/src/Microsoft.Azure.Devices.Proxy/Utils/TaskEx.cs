// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace System.Threading.Tasks {
    using System;
    using System.Threading;

    public static class TaskEx {
        public static Task Completed { get; } = Task.FromResult(true);

        public static void OnFault(this Task task, Action<Task> faultAction) {
            switch (task.Status) {
                case TaskStatus.RanToCompletion:
                case TaskStatus.Canceled:
                    break;
                case TaskStatus.Faulted:
                    faultAction(task);
                    break;
                default:
                    task.ContinueWith(faultAction,
                        TaskContinuationOptions.OnlyOnFaulted |
                        TaskContinuationOptions.ExecuteSynchronously);
                    break;
            }
        }

        public static void OnFault(this Task task, Action<Task, object> faultAction, object state) {
            switch (task.Status) {
                case TaskStatus.RanToCompletion:
                case TaskStatus.Canceled:
                    break;
                case TaskStatus.Faulted:
                    faultAction(task, state);
                    break;
                default:
                    task.ContinueWith(faultAction, state,
                        TaskContinuationOptions.OnlyOnFaulted |
                        TaskContinuationOptions.ExecuteSynchronously);
                    break;
            }
        }
    }
}