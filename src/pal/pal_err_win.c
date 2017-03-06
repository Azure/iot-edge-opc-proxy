// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os.h"
#include "util_mem.h"
#include "pal_err.h"
#include "pal_types.h"

//
// Convert an error code
//
int32_t pal_os_to_prx_error(
    int32_t error
)
{
    switch(error)
    {
        // Win32
    case ERROR_SUCCESS:                return er_ok;
    case ERROR_NO_DATA:                return er_not_found;
    case ERROR_INVALID_PARAMETER:      return er_arg;
    case ERROR_NOT_ENOUGH_MEMORY:      return er_out_of_memory;
    case ERROR_ADDRESS_NOT_ASSOCIATED: return er_not_found;
    case ERROR_BUFFER_OVERFLOW:        return er_fault;
    case ERROR_INVALID_OPERATION:      return er_aborted;
    case ERROR_OPERATION_ABORTED:      return er_aborted;
    case ERROR_INVALID_HANDLE:         return er_arg;
    case ERROR_IO_PENDING:             return er_waiting;
    case ERROR_NOT_FOUND:              return er_not_found;
        // Nt
    case STATUS_IO_TIMEOUT:            return er_timeout;
    case STATUS_CANCELLED:             return er_aborted;
    case STATUS_CONNECTION_ABORTED:    return er_aborted;
    case STATUS_REQUEST_ABORTED:       return er_aborted;
    case STATUS_CONNECTION_RESET:      return er_reset;
    default:
        return pal_os_to_prx_net_error(error);
    }
}

//
// Convert an error code
//
int pal_os_from_prx_error(
    int32_t error
)
{
    switch (error)
    {
    case er_ok:                return ERROR_SUCCESS;
    case er_not_found:         return ERROR_NO_DATA;
    case er_arg:               return ERROR_INVALID_PARAMETER;
    case er_out_of_memory:     return ERROR_NOT_ENOUGH_MEMORY;
    case er_fault:             return ERROR_BUFFER_OVERFLOW;
    case er_aborted:           return ERROR_OPERATION_ABORTED;
    case er_waiting:           return ERROR_IO_PENDING;
    default:
        dbg_assert(0, "Unknown pi error %d", error);
    }
    return pal_os_from_prx_net_error(error);
}

//
// Return general last error
//
int32_t pal_os_last_error_as_prx_error(
    void
)
{
    int32_t error;
    char* message = NULL;

    error = GetLastError();

    if (error != ERROR_SUCCESS)
    {
        FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (char*)&message, 0, NULL);

        log_error(NULL, "Platform error code %d (%s)", 
            error, message ? message : "<unknown>");
        LocalFree(message);
    }

    return pal_os_to_prx_error(error);
}

//
// Set general last error
//
void pal_os_set_error_as_prx_error(
    int32_t error
)
{
    int32_t result;

    if (error != er_ok)
    {
        log_debug(NULL, "Error occurred: %s", prx_err_string(error));
    }

    result = pal_os_from_prx_error(error);

    SetLastError(result);
}

//
// Initialize err 
//
int32_t pal_err_init(
    void
)
{
    return er_ok;
}

//
// Destroy err
//
void pal_err_deinit(
    void
)
{
    // no-op
}
