// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _prx_err_h_
#define _prx_err_h_

#include "common.h"


//
// Keep in sync with managed layer, in particular order of members!
//

typedef enum prx_error
{
    er_unknown = -1001,
    er_fatal,
    er_arg,
    er_fault,
    er_bad_state,
    er_out_of_memory,
    er_already_exists,
    er_not_found,
    er_not_supported,
    er_not_impl,
    er_permission,
    er_retry,
    er_nomore,
    er_network,
    er_connecting,
    er_busy,
    er_writing,
    er_reading,
    er_waiting,
    er_timeout,
    er_aborted,
    er_closed,
    er_shutdown,
    er_refused,
    er_no_address,
    er_no_host,
    er_host_unknown,
    er_address_family,
    er_duplicate,
    er_bad_flags,
    er_invalid_format,
    er_disk_io,
    er_missing,
    er_prop_get,
    er_prop_set,
    er_reset,
    er_undelivered,
    er_crypto,
    er_comm,
    er_ok = 0
}
prx_error_t;

//
// Returns the error as string representation
//
decl_internal_1(const char*, prx_err_string,
    int32_t, error
);


#endif // _prx_err_h_