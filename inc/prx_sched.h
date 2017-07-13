// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _prx_sched_h_
#define _prx_sched_h_

#include "common.h"
#include "util_dbg.h"

//
// Schedules tasks
//
typedef struct prx_scheduler prx_scheduler_t;

//
// Task work callback
//
typedef int32_t (*prx_task_t)(
    void* context
    );

//
// Create scheduler
//
decl_internal_2(int32_t, prx_scheduler_create,
    prx_scheduler_t*, parent,
    prx_scheduler_t**, scheduler
);

//
// Queue a task for execution
//
decl_internal_7(intptr_t, prx_scheduler_queue,
    prx_scheduler_t*, scheduler,
    const char*, name,
    prx_task_t, task,
    void*, context,
    uint32_t, delay,
    const char*, func,
    int, line
);

//
// Kill a task
//
decl_internal_2(int32_t, prx_scheduler_kill,
    prx_scheduler_t*, scheduler,
    intptr_t, id
);

//
// Removes all tasks of given type and/or context
//
decl_internal_3(void, prx_scheduler_clear,
    prx_scheduler_t*, scheduler,
    prx_task_t, task,
    void*, context
);

//
// Release scheduler of context
//
decl_internal_2(void, prx_scheduler_release,
    prx_scheduler_t*, scheduler,
    void*, context
);

//
// Waits for scheduler to exit
//
decl_internal_1(void, prx_scheduler_at_exit,
    prx_scheduler_t*, scheduler
);

//
// Checks whether code is running in specified scheduler.
//
decl_internal_1(bool, prx_scheduler_runs_me,
    prx_scheduler_t*, scheduler
);

//
// Schedule immediate execution 
//
#if !defined(UNIT_TEST)
#define __do_next_s(s, t, o) \
    prx_scheduler_queue(s, #t, (prx_task_t)t, o, \
        0, __func__, __LINE__)
#else
#define __do_next_s(s, t, o) \
    do { prx_task_t _t = (prx_task_t)t; _t(o); } while(0)
#endif

//
// Schedule immediate execution, scheduler is part of object
//
#define __do_next(o, t) \
    __do_next_s(o->scheduler, t, o)

//
// Schedule delayed task execution, clearing all previous ones
//
#if !defined(UNIT_TEST)
#define __do_later_s(s, t, o, d) \
    prx_scheduler_clear(s, (prx_task_t)t, o); \
    prx_scheduler_queue(s, #t, (prx_task_t)t, o, \
        d, __func__, __LINE__)
#else
#define __do_later_s(s, t, o, d) \
    do { prx_task_t _t = (prx_task_t)t; _t(o); } while(0)
#endif

//
// Schedule later, scheduler is part of object
//
#define __do_later(o, t, d) \
    __do_later_s(o->scheduler, t, o, d)

//
// Assert we are on scheduler thread
//
#ifndef dbg_assert_is_task
#if !defined(UNIT_TEST)
#define dbg_assert_is_task(s) \
    dbg_assert(prx_scheduler_runs_me(s), "not running on scheduler")
#else
#define dbg_assert_is_task(s)
#endif
#endif


#endif // _prx_sched_h_