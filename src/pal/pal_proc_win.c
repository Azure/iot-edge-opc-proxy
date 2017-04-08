// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os.h"
#include "util_mem.h"
#include "util_string.h"
#include "pal_err.h"
#include "pal_proc.h"

//
// Process object
//
struct process
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    STRING_HANDLE cmd_line;
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
    chk_arg_fault_return(created);
    proc = mem_zalloc_type(process_t);
    if (!proc)
        return er_out_of_memory;
    do
    {
        result = er_ok;
        proc->cmd_line = STRING_new();
        if (!proc->cmd_line)
        {
            result = er_out_of_memory;
            break;
        }
        if (image)
        {
            if (0 != STRING_concat(proc->cmd_line, "\"") ||
                0 != STRING_concat(proc->cmd_line, image) ||
                0 != STRING_concat(proc->cmd_line, "\""))
            {
                result = er_out_of_memory;
                break;
            }
        }
        for (int i = 0; i < argc; i++)
        {
            if (0 == STRING_concat(proc->cmd_line, " ") &&
                0 == STRING_concat(proc->cmd_line, argv[i]))
                continue;
            result = er_out_of_memory;
            break;
        }
        if (result != er_ok)
            break;

        proc->si.cb = sizeof(proc->si);
        __analysis_suppress(6335)
        if (!CreateProcessA(NULL, (LPSTR)STRING_c_str(proc->cmd_line),
            NULL, NULL, FALSE,
            CREATE_NEW_CONSOLE | NORMAL_PRIORITY_CLASS,
            NULL, NULL, &proc->si, &proc->pi))
        {
            result = pal_os_last_error_as_prx_error();
            log_error(NULL, "CreateProcess(%s) failed (%s).",
                STRING_c_str(proc->cmd_line), prx_err_string(result));
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
    TerminateProcess(proc->pi.hProcess, 0);
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

    if (proc->pi.hProcess)
    {
        result = WaitForSingleObject(proc->pi.hProcess, INFINITE);
        if (result != WAIT_OBJECT_0)
            log_error(NULL, "WaitForSingleObject failed %d\n", result);
        CloseHandle(proc->pi.hProcess);
    }
    if (proc->pi.hThread)
        CloseHandle(proc->pi.hThread);

    if (proc->cmd_line)
        STRING_delete(proc->cmd_line);
    mem_free_type(process_t, proc);
}
