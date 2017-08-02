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
    switch (error)
    {
    case 0:
        //   0   Success
        return er_ok;
    case EPERM:
        //   1   Operation not permitted
        return er_permission;
    case ENOENT:
        //   2   No such file or directory
        return er_not_found;
    case ESRCH:
        //   3   No such process
        break;
    case EINTR:
        //   4   Interrupted system call
        return er_aborted;
    case EIO:
        //   5   I/O error
        return er_disk_io;
    case ENXIO:
        //   6   No such device or address
        return er_not_found;
    case E2BIG:
        //   7   Argument list too long
        return er_arg;
    case ENOEXEC:
        //   8   Exec format error
        return er_invalid_format;
    case EBADF:
        //   9   Bad file number
        return er_arg;
    case ECHILD:
        //  10   No child processes
        break;
    case EAGAIN:
        //  11   Try again (==EWOULDBLOCK Operation would block)
        return er_retry;
    case ENOMEM:
        //  12   Out of memory
        return er_out_of_memory;
    case EACCES:
        //  13   Permission denied
        return er_permission;
    case EFAULT:
        //  14   Bad address
        return er_fault;
    case ENOTBLK:
        //  15   Block device required
        return er_disk_io;
    case EBUSY:
        //  16   Device or resource busy
        return er_busy;
    case EEXIST:
        //  17   File exists
        return er_already_exists;
    case EXDEV:
        //  18   Cross-device link
        break;
    case ENODEV:
        //  19   No such device
        return er_not_found;
    case ENOTDIR:
        //  20   Not a directory
        return er_disk_io;
    case EISDIR:
        //  21   Is a directory
        return er_disk_io;
    case EINVAL:
        //  22   Invalid argument
        return er_arg;
    case ENFILE:
        //  23   File table overflow
        return er_disk_io;
    case EMFILE:
        //  24   Too many open files
        return er_out_of_memory;
    case ENOTTY:
        //  25   Not a typewriter
        break;
    case ETXTBSY:
        //  26   Text file busy
        return er_disk_io;
    case EFBIG:
        //  27   File too large
        return er_disk_io;
    case ENOSPC:
        //  28   No space left on device
        return er_disk_io;
    case ESPIPE:
        //  29   Illegal seek
        return er_disk_io;
    case EROFS:
        //  30   Read-only file system
        return er_disk_io;
    case EMLINK:
        //  31   Too many links
        return er_disk_io;
    case EPIPE:
        //  32   Broken pipe
        return er_reset;
    case EDOM:
        //  33   Math argument out of domain of func
        return er_arg;
    case ERANGE:
        //  34   Math result not representable
        return er_arg;
    case EDEADLK:
        //  35   Resource deadlock would occur
        break;
    case ENAMETOOLONG:
        //  36   File name too long
        return er_arg;
    case ENOLCK:
        //  37   No record locks available
        break;
    case ENOSYS:
        //  38   Function not implemented
        return er_not_impl;
    case ENOTEMPTY:
        //  39   Directory not empty
        return er_disk_io;
    case ELOOP:
        //  40   Too many symbolic links encountered
        return er_disk_io;
    case ENOMSG:
        //  42   No message of desired type
        break;
    case EIDRM:
        //  43   Identifier removed
        break;
    case ECHRNG:
        //  44   Channel number out of range
        return er_arg;
    case EL2NSYNC:
        //  45   Level 2 not synchronized
        break;
    case EL3HLT:
        //  46   Level 3 halted
        break;
    case EL3RST:
        //  47   Level 3 reset
        break;
    case ELNRNG:
        //  48   Link number out of range
        break;
    case EUNATCH:
        //  49   Protocol driver not attached
        break;
    case ENOCSI:
        //  50   No CSI structure available
        break;
    case EL2HLT:
        //  51   Level 2 halted
        break;
    case EBADE:
        //  52   Invalid exchange
        break;
    case EBADR:
        //  53   Invalid request descriptor
        break;
    case EXFULL:
        //  54   Exchange full
        break;
    case ENOANO:
        //  55   No anode
        break;
    case EBADRQC:
        //  56   Invalid request code
        break;
    case EBADSLT:
        //  57   Invalid slot
        break;
    case EBFONT:
        //  59   Bad font file format
        break;
    case ENOSTR:
        //  60   Device not a stream
        break;
    case ENODATA:
        //  61   No data available
        return er_nomore;
    case ETIME:
        //  62   Timer expired
        return er_timeout;
    case ENOSR:
        //  63   Out of streams resources
        return er_out_of_memory;
    case ENONET:
        //  64   Machine is not on the network
        return er_network;
    case ENOPKG:
        //  65   Package not installed
        break;
    case EREMOTE:
        //  66   Object is remote
        break;
    case ENOLINK:
        //  67   Link has been severed
        break;
    case EADV:
        //  68   Advertise error
        break;
    case ESRMNT:
        //  69   Srmount error
        break;
    case ECOMM:
        //  70   Communication error on send
        return er_comm;
    case EPROTO:
        //  71   Protocol error
        return er_network;
    case EMULTIHOP:
        //  72   Multihop attempted
        return er_network;
    case EDOTDOT:
        //  73   RFS specific error
        break;
    case EBADMSG:
        //  74   Not a data message
        return er_invalid_format;
    case EOVERFLOW:
        //  75   Value too large for defined data type
        return er_fault;
    case ENOTUNIQ:
        //  76   Name not unique on network
        break;
    case EBADFD:
        //  77   File descriptor in bad state
        return er_bad_state;
    case EREMCHG:
        //  78   Remote address changed
        break;
    case ELIBACC:
        //  79   Can not access a needed shared library
        break;
    case ELIBBAD:
        //  80   Accessing a corrupted shared library
        break;
    case ELIBSCN:
        //  81   .lib section in a.out corrupted
        break;
    case ELIBMAX:
        //  82   Attempting to link in too many shared libraries
        break;
    case ELIBEXEC:
        //  83   Cannot exec a shared library directly
        break;
    case EILSEQ:
        //  84   Illegal byte sequence
        break;
    case ERESTART:
        //  85   Interrupted system call should be restarted
        return er_retry;
    case ESTRPIPE:
        //  86   Streams pipe error
        return er_comm;
    case EUSERS:
        //  87   Too many users
        break;
    case ENOTSOCK:
        //  88   Socket operation on non-socket
        return er_arg;
    case EDESTADDRREQ:
        //  89   Destination address required
        return er_arg;
    case EMSGSIZE:
        //  90   Message too long
        return er_invalid_format;
    case EPROTOTYPE:
        //  91   Protocol wrong type for socket
        return er_not_supported;
    case ENOPROTOOPT:
        //  92   Protocol not available
        return er_not_supported;
    case EPROTONOSUPPORT:
        //  93   Protocol not supported
        return er_not_supported;
    case ESOCKTNOSUPPORT:
        //  94   Socket type not supported
        return er_not_supported;
    case EOPNOTSUPP:
        //  95   Operation not supported on transport endpoint
        return er_not_supported;
    case EPFNOSUPPORT:
        //  96   Protocol family not supported
        return er_not_supported;
    case EAFNOSUPPORT:
        //  97   Address family not supported by protocol
        return er_address_family;
    case EADDRINUSE:
        //  98   Address already in use
        return er_already_exists;
    case EADDRNOTAVAIL:
        //  99   Cannot assign requested address
        return er_no_address;
    case ENETDOWN:
        //  100  Network is down
        return er_network;
    case ENETUNREACH:
        //  101  Network is unreachable
        return er_network;
    case ENETRESET:
        //  102  Network dropped connection because of reset
        return er_network;
    case ECONNABORTED:
        //  103  Software caused connection abort
        return er_connecting;
    case ECONNRESET:
        //  104  Connection reset by peer
        return er_reset;
    case ENOBUFS:
        //  105  No buffer space available
        return er_out_of_memory;
    case EISCONN:
        //  106  Transport endpoint is already connected
        return er_already_exists;
    case ENOTCONN:
        //  107  Transport endpoint is not connected
        return er_closed;
    case ESHUTDOWN:
        //  108  Cannot send after transport endpoint shutdown
        return er_shutdown;
    case ETOOMANYREFS:
        //  109  Too many references: cannot splice
        break;
    case ETIMEDOUT:
        //  110  Connection timed out
        return er_timeout;
    case ECONNREFUSED:
        //  111  Connection refused
        return er_refused;
    case EHOSTDOWN:
        //  112  Host is down
        return er_no_host;
    case EHOSTUNREACH:
        //  113  No route to host
        return er_host_unknown;
    case EALREADY:
        //  114  Operation already in progress
        return er_waiting;
    case EINPROGRESS:
        //  115  Operation now in progress
        return er_waiting;
    case ESTALE:
        //  116  Stale NFS file handle
        break;
    case EUCLEAN:
        //  117  Structure needs cleaning
        break;
    case ENOTNAM:
        //  118  Not a XENIX named type file
        break;
    case ENAVAIL:
        //  119  No XENIX semaphores available
        break;
    case EISNAM:
        //  120  Is a named type file
        break;
    case EREMOTEIO:
        //  121  Remote I/O error
        break;
    case EDQUOT:
        //  122  Quota exceeded
        break;
    case ENOMEDIUM:
        //  123  No medium found
        break;
    case EMEDIUMTYPE:
        //  124  Wrong medium type
        break;
    case ECANCELED:
        //  125  Operation Canceled
        return er_aborted;
    case ENOKEY:
        //  126  Required key not available
        break;
    case EKEYEXPIRED:
        //  127  Key has expired
        break;
    case EKEYREVOKED:
        //  128  Key has been revoked
        break;
    case EKEYREJECTED:
        //  129  Key was rejected by service
        break;
    case EOWNERDEAD:
        //  130  Owner died
        break;
    case ENOTRECOVERABLE:
        //  131  State not recoverable
        break;
    default:
        /**/ if (error == EWOULDBLOCK)
            return er_retry;
        dbg_assert(0, "Unknown os error %d", error);
        return er_unknown;
    }
    return er_fatal;
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
    case er_ok:
        return 0;
    case er_fatal:
        return ENOTRECOVERABLE;
    case er_arg:
        return EINVAL;
    case er_fault:
        return EFAULT;
    case er_bad_state:
        return EBADFD;
    case er_out_of_memory:
        return ENOMEM;
    case er_already_exists:
        return EEXIST;
    case er_not_found:
        return ENXIO;
    case er_not_supported:
        return EOPNOTSUPP;
    case er_not_impl:
        return ENOSYS;
    case er_permission:
        return EACCES;
    case er_retry:
        return EAGAIN;
    case er_nomore:
        return ENODATA;
    case er_network:
        return ENETDOWN;
    case er_connecting:
        return EHOSTUNREACH;
    case er_busy:
        return EBUSY;
    case er_writing:
        return ECOMM;
    case er_reading:
        return ECOMM;
    case er_waiting:
        return EINPROGRESS;
    case er_timeout:
        return ETIME;
    case er_aborted:
        return EINTR;
    case er_closed:
        return ENOTCONN;
    case er_shutdown:
        return ESHUTDOWN;
    case er_refused:
        return ECONNREFUSED;
    case er_no_address:
        return EADDRNOTAVAIL;
    case er_no_host:
        return EHOSTDOWN;
    case er_host_unknown:
        return EHOSTUNREACH;
    case er_address_family:
        return EAFNOSUPPORT;
    case er_duplicate:
        break;
    case er_bad_flags:
        return EINVAL;
    case er_invalid_format:
        return EINVAL;
    case er_disk_io:
        return EIO;
    case er_missing:
        break;
    case er_prop_get:
        return EPROTO;
    case er_prop_set:
        return EPROTO;
    case er_reset:
        return ECONNRESET;
    case er_undelivered:
        return ETIMEDOUT;
    case er_crypto:
        return EPERM;
    case er_comm:
        return ECOMM;
    default:
        break;
    }
    dbg_assert(0, "Unknown pi error %d", error);
    return -1;
}

//
// Return general last error
//
int32_t pal_os_last_error_as_prx_error(
    void
)
{
    int32_t error;

    error = errno;

    if (error != 0 &&
        error != EINPROGRESS &&
        error != EWOULDBLOCK &&
        error != EINTR)
    {
        log_info(NULL, "A OS operation resulted in error %d (%s)",
            error, strerror(error));
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
        log_debug(NULL, "Setting errno to reflect error %s",
            prx_err_string(error));
    }

    result = pal_os_from_prx_error(error);
    errno = result;
}

//
// Initialize err
//
int32_t pal_err_init(
    void
)
{
#if !defined(UNIT_TEST)
    struct sigaction sig_action;

    sig_action.sa_flags = 0;
    sigemptyset(&sig_action.sa_mask);

    do
    {
        sig_action.sa_handler = SIG_DFL;
        if (0 != sigaction(SIGSEGV, &sig_action, NULL))
            break;
        sig_action.sa_handler = SIG_DFL;
        if (0 != sigaction(SIGFPE,  &sig_action, NULL))
            break;
        sig_action.sa_handler = SIG_IGN;
        if (0 != sigaction(SIGILL,  &sig_action, NULL))
            break;
        sig_action.sa_handler = SIG_IGN;
        if (0 != sigaction(SIGPIPE, &sig_action, NULL))
            break;
        sig_action.sa_handler = SIG_DFL;
        if (0 != sigaction(SIGTERM, &sig_action, NULL))
            break;
        sig_action.sa_handler = SIG_DFL;
        if (0 != sigaction(SIGABRT, &sig_action, NULL))
            break;

        return er_ok;
    }
    while(0);
    log_error(NULL, "failed to sigaction");
    return pal_os_last_error_as_prx_error();
#else
    return er_ok;
#endif
}

//
// Destroy err
//
void pal_err_deinit(
    void
)
{
    // no op
}
