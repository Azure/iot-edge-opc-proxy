// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "prx_err.h"
#include "util_mem.h"

//
// Returns the pal error as string representation
//
const char* prx_err_string(
    int32_t error
)
{
    switch (error)
    {
    case er_unknown:
        return "er_unknown";
    case er_fatal:
        return "er_fatal";
    case er_arg:
        return "er_arg";
    case er_fault:
        return "er_fault";
    case er_bad_state:
        return "er_bad_state";
    case er_out_of_memory:
        return "er_out_of_memory";
    case er_already_exists:
        return "er_already_exists";
    case er_not_found:
        return "er_not_found";
    case er_not_supported:
        return "er_not_supported";
    case er_not_impl:
        return "er_not_impl";
    case er_permission:
        return "er_permission";
    case er_retry:
        return "er_retry";
    case er_nomore:
        return "er_nomore";
    case er_network:
        return "er_network";
    case er_connecting:
        return "er_connecting";
    case er_busy:
        return "er_busy";
    case er_writing:
        return "er_writing";
    case er_reading:
        return "er_reading";
    case er_waiting:
        return "er_waiting";
    case er_timeout:
        return "er_timeout";
    case er_aborted:
        return "er_aborted";
    case er_closed:
        return "er_closed";
    case er_shutdown:
        return "er_shutdown";
    case er_refused:
        return "er_refused";
    case er_no_address:
        return "er_no_address";
    case er_no_host:
        return "er_no_host";
    case er_host_unknown:
        return "er_host_unknown";
    case er_address_family:
        return "er_address_family";
    case er_duplicate:
        return "er_duplicate";
    case er_bad_flags:
        return "er_bad_flags";
    case er_invalid_format:
        return "er_invalid_format";
    case er_disk_io:
        return "er_disk_io";
    case er_missing:
        return "er_missing";
    case er_prop_get:
        return "er_prop_get";
    case er_prop_set:
        return "er_prop_set";
    case er_reset:
        return "er_reset";
    case er_crypto:
        return "er_crypto";
    case er_comm:
        return "er_comm";
    case er_undelivered:
        return "er_undelivered";
    case er_ok:
        return "er_ok";
    }
    return "<unknown>";
}


