// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "util_mem.h"
#include "prx_sched.h"
#include "prx_buffer.h"
#include "util_signal.h"
#include "pal_mt.h"
#include "pal_time.h"

#include "azure_c_shared_utility/refcount.h"
#include "azure_c_shared_utility/doublylinkedlist.h"
#include "azure_c_shared_utility/threadapi.h"

#define INITIAL_POOL_SIZE 128

// #define LOG_VERBOSE

//
// Schedules ...
//
struct prx_scheduler
{
    prx_buffer_factory_t* task_pool;       // Pool of task entries
    DLIST_ENTRY now;     // Immediately queued tasks in fifo order
    DLIST_ENTRY later;         // queued tasks ordered by deadline
    rw_lock_t lock;
    THREAD_HANDLE worker_thread;
    signal_t* wakeup;                     // Idle interrupt signal
    bool should_run;     // Whether the schedule thread is running
    log_t log;
};

DEFINE_REFCOUNT_TYPE(prx_scheduler_t);

//
// ... entries in task queue
//
typedef struct prx_task_entry
{
    const char* name;                // Name of task for debugging
    ticks_t queued;       // When the task was queued to scheduler
    ticks_t deadline;                   // When it needs to be run
    prx_task_t task;                               // Task handler
    void* context;                    // this pointer passed to it
    DLIST_ENTRY link;
#ifdef DEBUG
    const char* func;
    int line;
#endif
}
prx_task_entry_t;

//
// Remove a task from the a queue by id
//
static prx_task_entry_t* prx_scheduler_remove_by_id(
    PDLIST_ENTRY queue,
    intptr_t id
)
{
    prx_task_entry_t* next, *found = NULL;

    for (PDLIST_ENTRY p = queue->Flink;
        p != queue; p = p->Flink)
    {
        next = containingRecord(p, prx_task_entry_t, link);
        if (next == (prx_task_entry_t*)id)
        {
            DList_RemoveEntryList(&next->link);
            found = next;
            break;
        }
    }
    return found;
}

//
// Remove by context and return in passed in list
//
static void prx_scheduler_remove_by_task_type(
    PDLIST_ENTRY queue,
    prx_task_t task,
    void* context,
    PDLIST_ENTRY result
)
{
    prx_task_entry_t* next;
    for (PDLIST_ENTRY p = queue->Flink; p != queue; )
    {
        next = containingRecord(p, prx_task_entry_t, link);
        if ((!context || next->context == context) &&
            (!task || next->task == task))
        {
            p = p->Flink;
            DList_RemoveEntryList(&next->link);
            DList_InsertTailList(result, &next->link);
            continue;
        }
        p = p->Flink;
    }
}

//
// Debug log dump contents of task queue
//
static void prx_scheduler_log_queue(
    log_t log,
    PDLIST_ENTRY queue
)
{
    prx_task_entry_t* next;
    int32_t index = 0;
    for (PDLIST_ENTRY p = queue->Flink; p != queue; p = p->Flink)
    {
        next = containingRecord(p, prx_task_entry_t, link);
#if defined(UNIT_TEST)
        (void)log;
#elif defined(DEBUG)
        log_debug(log, "%d [%s(%p)] (%s:%d) due:%u added:%u",
            index, next->name, next->context, next->func, next->line, 
            next->deadline, next->queued);
#else
        log_debug(log, "%d [%s(%p)] due:%u added:%u",
            index, next->name, next->context,
            next->deadline, next->queued);
#endif
        index++;
    }
}

//
// Exit idle
//
static void prx_scheduler_interrupt(
    prx_scheduler_t* scheduler
)
{
    (void)signal_set(scheduler->wakeup);
}

//
// Enter idle
//
static void prx_scheduler_idle(
    prx_scheduler_t* scheduler,
    int32_t delay
)
{
    // prx_scheduler_log_queue(scheduler->log, &scheduler->later);
    (void)signal_wait(scheduler->wakeup, (uint32_t)delay);
}

//
// Get next entry from task queue
//
static prx_task_entry_t* prx_scheduler_get_next(
    prx_scheduler_t* scheduler,
    ticks_t now,
    int32_t* idle
)
{
    prx_task_entry_t* candidate = NULL;

    rw_lock_enter(scheduler->lock);
    do
    {
        *idle = -1;

        if (!DList_IsListEmpty(&scheduler->later))
        {
            candidate = containingRecord(
                scheduler->later.Flink, prx_task_entry_t, link);
            dbg_assert_ptr(candidate);

            *idle = (int32_t)(candidate->deadline - now);

            if (*idle <= 0)
            {
                // Scheduled item hit deadline, run now
                DList_RemoveEntryList(&candidate->link);

                log_debug(scheduler->log, "Task %s with skew: %u", 
                    candidate->name, (uint32_t)abs(*idle));
                *idle = 0;
                break;
            }

            // Get next idle wait
            candidate = NULL;
        }

        if (!DList_IsListEmpty(&scheduler->now))
        {
            *idle = 0;
            candidate = containingRecord(
                DList_RemoveHeadList(&scheduler->now), prx_task_entry_t, link);
            log_debug(scheduler->log, "Task %s with latency: %u",
                candidate->name, (uint32_t)(now - candidate->queued));
            dbg_assert_ptr(candidate);
            break;
        }
    }
    while (0);

    rw_lock_exit(scheduler->lock);
    return candidate;
}

// 
// Runs the scheduler tasks queued
//
static int32_t prx_scheduler_work(
    void* context
)
{
    ticks_t now;
    int32_t idle_timeout;
    prx_task_entry_t* next;
    prx_scheduler_t* scheduler = (prx_scheduler_t*)context;
    dbg_assert_ptr(scheduler);

    while (true)
    {
        now = ticks_get();
        next = prx_scheduler_get_next(scheduler, now, &idle_timeout);
        if (next)
        {
            next->task(next->context);
            log_debug(scheduler->log, "Task %s took %u ms",
                next->name, (uint32_t)(ticks_get() - now));
            prx_buffer_release(scheduler->task_pool, next);
            mem_check();
        }
        else if (!scheduler->should_run && idle_timeout == -1)
        {
            log_debug(scheduler->log, "Exit scheduler...");
            break;
        }
        else
        {
            log_debug(scheduler->log, "Entering idle for %u ms (%u)",
                (uint32_t)idle_timeout, (uint32_t)now);
            prx_scheduler_idle(scheduler, idle_timeout);
            log_debug(scheduler->log, "Exit idle - idled for %u ms",
                (uint32_t)(ticks_get() - now));
        }
    }
    mem_check();
    return 0;
}

//
// Create scheduler
//
int32_t prx_scheduler_create(
    prx_scheduler_t* parent,
    prx_scheduler_t** created
)
{
    int32_t result;
    prx_pool_config_t config;
    THREADAPI_RESULT thread_result;
    prx_scheduler_t* scheduler;

    if (parent)
    {
        // Take a ref count on parent and return
        *created = parent;
        INC_REF(prx_scheduler_t, parent);
        return er_ok;
    }

    scheduler = REFCOUNT_TYPE_CREATE(prx_scheduler_t);
    if (!scheduler)
        return er_out_of_memory;
    memset(scheduler, 0, sizeof(prx_scheduler_t));
    do
    {
        scheduler->log = log_get("scheduler");
        DList_InitializeListHead(&scheduler->now);
        DList_InitializeListHead(&scheduler->later);

        memset(&config, 0, sizeof(config));
        config.initial_count = INITIAL_POOL_SIZE;

        result = prx_fixed_pool_create("scheduler", sizeof(prx_task_entry_t), 
            &config, &scheduler->task_pool);
        if (result != er_ok)
            break;

        result = rw_lock_create(&scheduler->lock);
        if (result != er_ok)
            break;

        result = signal_create(false, false, &scheduler->wakeup);
        if (result != er_ok)
            break;

        scheduler->should_run = true;
        thread_result = ThreadAPI_Create(
            &scheduler->worker_thread, prx_scheduler_work, scheduler);
        if (THREADAPI_OK != thread_result)
        {
            result = er_fatal;
            scheduler->should_run = false;
            break;
        }

        *created = scheduler;
        return er_ok;
    } 
    while (0);

    prx_scheduler_release(scheduler, NULL);
    return result;
}

//
// Queue a task
//
intptr_t prx_scheduler_queue(
    prx_scheduler_t* scheduler,
    const char* name,
    prx_task_t task,
    void* context,
    uint32_t delay,
    const char* func,
    int line
)
{
    bool inserted;
    prx_task_entry_t* entry, *next;

    chk_arg_fault_return(scheduler);
    chk_arg_fault_return(task);

    entry = (prx_task_entry_t*)prx_buffer_alloc(scheduler->task_pool, NULL);
    if (!entry)
    {
        dbg_assert(0, "Too many tasks queued, ran out of memory");
        return -1;
    }

    entry->name = name;
    entry->context = context;
    entry->task = task;
    entry->queued = ticks_get();
    entry->deadline = entry->queued + delay;
#ifdef DEBUG
    entry->func = func;
    entry->line = line;
#else
    (void)func;
    (void)line;
#endif
    rw_lock_enter_w(scheduler->lock);
    if (!delay)
    {
        // Insert at end of now list
        DList_InsertTailList(&scheduler->now, &entry->link);
    }
    else
    {
        // Insert ordered
        inserted = false;

        for (PDLIST_ENTRY p = scheduler->later.Flink; 
            p != &scheduler->later; p = p->Flink)
        {
            next = containingRecord(p, prx_task_entry_t, link);
            if (next->deadline > entry->deadline)
            {
                // this task has earlier deadline so insert before
                DList_InsertTailList(&next->link, &entry->link);
                inserted = true;
                break;
            }
        }

        if (!inserted)
        {
            // Insert at end
            DList_InsertTailList(&scheduler->later, &entry->link);
        }
    }
    rw_lock_exit_w(scheduler->lock);

#ifdef LOG_VERBOSE
    prx_scheduler_log_queue(scheduler->log, &scheduler->now);
    prx_scheduler_log_queue(scheduler->log, &scheduler->later);
#endif
    prx_scheduler_interrupt(scheduler);
    return (intptr_t)entry;
}

//
// Kill a task by id
//
int32_t prx_scheduler_kill(
    prx_scheduler_t* scheduler,
    intptr_t id
)
{
    prx_task_entry_t* next;

    chk_arg_fault_return(scheduler);

    rw_lock_enter_w(scheduler->lock);
    next = prx_scheduler_remove_by_id(&scheduler->now, id);
    if (!next)
        next = prx_scheduler_remove_by_id(&scheduler->later, id);
    rw_lock_exit(scheduler->lock);

    if (next)
    {
        prx_buffer_release(scheduler->task_pool, next);
        return er_ok;
    }

    return er_not_found;
}

//
// Removes all tasks with given context and/or callback
//
void prx_scheduler_clear(
    prx_scheduler_t* scheduler,
    prx_task_t task,
    void* context
)
{
    prx_task_entry_t* next;
    DLIST_ENTRY free_list;

    if (!scheduler)
        return;

    DList_InitializeListHead(&free_list);

    rw_lock_enter_w(scheduler->lock);
    prx_scheduler_remove_by_task_type(&scheduler->now, task, context, &free_list);
    prx_scheduler_remove_by_task_type(&scheduler->later, task, context, &free_list);
    rw_lock_exit_w(scheduler->lock);

    while(!DList_IsListEmpty(&free_list))
    {
        next = containingRecord(
            DList_RemoveHeadList(&free_list), prx_task_entry_t, link);
        prx_buffer_release(scheduler->task_pool, next);
    }
}

//
// Release scheduler, also clears scheduler of context
//
void prx_scheduler_release(
    prx_scheduler_t* scheduler,
    void* context
)
{
    if (!scheduler)
        return;

    log_debug(scheduler->log, "Releasing scheduler queue for %p", context);
#ifdef LOG_VERBOSE
    prx_scheduler_log_queue(scheduler->log, &scheduler->now);
    prx_scheduler_log_queue(scheduler->log, &scheduler->later);
#endif

    if (context)
        prx_scheduler_clear(scheduler, NULL, context);

    if (DEC_REF(prx_scheduler_t, scheduler) == DEC_RETURN_ZERO)
    {
        scheduler->should_run = false;
        prx_scheduler_interrupt(scheduler);
    }
}

//
// Wait before exiting program
//
void prx_scheduler_at_exit(
    prx_scheduler_t* scheduler
)
{
    int32_t result;
    if (!scheduler)
        return;

    if (scheduler->worker_thread)
    {
        if (THREADAPI_OK != ThreadAPI_Join(scheduler->worker_thread, &result))
        {
            log_error(scheduler->log, "Failed stopping scheduler thread");
        }
    }

    if (scheduler->wakeup)
        signal_free(scheduler->wakeup);

    if (scheduler->task_pool)
    {
        prx_scheduler_clear(scheduler, NULL, NULL);
        prx_buffer_factory_free(scheduler->task_pool);
    }

    if (scheduler->lock)
        rw_lock_free(scheduler->lock);

    REFCOUNT_TYPE_FREE(prx_scheduler_t, scheduler);
}
