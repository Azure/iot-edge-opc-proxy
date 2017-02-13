//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace System.Threading.Tasks {
    using System;
    using Threading;

    public static class TaskToSync {
        /// <summary>
        /// Execute's an async Task<T> method which has a void return value synchronously
        /// </summary>
        /// <param name="task">Task<T> method to execute</param>
        public static void Run(Func<Task> task) {
            Task.Factory.StartNew(() => task().Wait(), CancellationToken.None, 
                TaskCreationOptions.None, TaskScheduler.Default).ConfigureAwait(false).GetAwaiter().GetResult();
        }

        /// <summary>
        /// Execute's an async Task<T> method which has a T return type synchronously
        /// </summary>
        /// <typeparam name="T">Return Type</typeparam>
        /// <param name="task">Task<T> method to execute</param>
        /// <returns></returns>
        public static T Run<T>(Func<Task<T>> task) {
            return Task.Factory.StartNew(() => task().Result, CancellationToken.None,
               TaskCreationOptions.None, TaskScheduler.Default).ConfigureAwait(false).GetAwaiter().GetResult();
        }
    }
}

