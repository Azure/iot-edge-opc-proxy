// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os.h"
#include "util_mem.h"
#include "util_string.h"
#include "pal_proc.h"
#include "pal_mt.h"

#include <stdio.h>
#if !defined(UNIT_TEST)
#include <sys/wait.h>
#include <signal.h>
#endif

struct process
{
    char** cmd_line;
    pid_t pid;
};

//
// Spwans a process with specified command line
//
int32_t pal_process_spawn(
    const char* image,
    int32_t argc,
    const char* argv[],
    process_t** created
)
{
    int32_t result;
    process_t* proc;

    if (!created)
        return er_fault;
    proc = mem_zalloc_type(process_t);
    if (!proc)
        return er_out_of_memory;
    do
    {
        result = er_ok;
        proc->cmd_line = (char**)mem_zalloc(sizeof(char*) * (argc + 1));
        if (!proc->cmd_line)
        {
            result = er_out_of_memory;
            break;
        }
        for (int i = 0; i < argc; i++)
        {
            result = string_clone(argv[i], &proc->cmd_line[i]);
            if (result != er_ok)
                break;
        }
        if (result != er_ok)
            break;

        fflush(NULL);
        proc->pid = fork();
        switch (proc->pid)
        {
        case -1:
            result = er_out_of_memory;
            break;
        case 0:
            // New process, execute entry point
            execvpe(image, proc->cmd_line, environ);
            // Only returns when error occurs
            log_error(NULL, "failed execvpe with error %d", errno);
            exit(-1);
            break;
        }
        *created = proc;
        return er_ok;
    } while (0);

    pal_process_yield(proc);
    return result;
}

//
// Kills a process
//
void pal_process_kill(
    process_t* proc
)
{
    if (!proc)
        return;
    (void)kill(proc->pid, SIGINT);
}

//
// Waits for a process to exit
//
void pal_process_yield(
    process_t* proc
)
{
    int32_t result;
    if (!proc)
        return;

    fflush(NULL);

    if (0 != waitpid(proc->pid, &result, 0))
        log_error(NULL, "Wait pid returned with error %d\n", errno);

    if (proc->cmd_line)
    {
        for (int i = 0; proc->cmd_line[i]; i++)
            mem_free(proc->cmd_line[i]);
        mem_free(proc->cmd_line);
    }
    mem_free_type(process_t, proc);
}
