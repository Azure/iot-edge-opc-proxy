// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _os_mock_h_
#define _os_mock_h_

//
// Posix types and defines to get posix code to compile
//

#define _os_posix_h_ 1

typedef long long ticks_t;

typedef unsigned long u_long;

// pthread.h

typedef int pthread_rwlock_t;
typedef int pthread_rwlockattr_t;


// poll.h
struct pollfd {
    int   fd;
    short events;
    short revents;
};

enum __POLLEVENT
{
    POLLRDNORM = 0x10,
    POLLRDBAND = 0x20,
    POLLWRNORM = 0x40,
    POLLERR = 0x80,
    POLLHUP = 0x100
};

// socket
enum __AF
{
    AF_RANGE_BEGIN = 0,

    AF_UNSPEC,
    AF_INET,
    AF_INET6,
    AF_UNIX,

    AF_RANGE_END
};

enum __SOCK
{
    SOCK_RANGE_BEGIN = 0,

    SOCK_DGRAM,
    SOCK_STREAM,
    SOCK_RAW,
    SOCK_SEQPACKET,
    SOCK_RDM,

    SOCK_RANGE_END
};

enum __IPPROTO
{
    IPPROTO_RANGE_BEGIN = 0,
    
    IPPROTO_UDP,
    IPPROTO_TCP,
    IPPROTO_IP,
    IPPROTO_IPV6,
    IPPROTO_ICMP,
    IPPROTO_ICMPV6,

    IPPROTO_RANGE_END 
};

struct sockaddr
{
    int sa_family;
};

struct in6_addr
{
    uint8_t s6_addr[16];
};

struct in_addr
{
    uint32_t s_addr;
};

struct sockaddr_in
{
    int sin_family;
    short sin_port;
    struct in_addr sin_addr;
};

struct sockaddr_in6
{
    int sin6_family;
    short sin6_port;
    int sin6_flowinfo;
    int sin6_scope_id;
    struct in6_addr sin6_addr;
};

struct addrinfo
{
    int ai_flags; 
    int ai_family;
    int ai_socktype;
    int ai_protocol; 
    size_t ai_addrlen; 
    char* ai_canonname; 
    struct sockaddr* ai_addr;
    struct addrinfo* ai_next; 
};

struct linger
{
    int l_onoff;
    int l_linger;
};

#define AI_CANONNAME 0x2
#define AI_NUMERICHOST 0x4
#define AI_PASSIVE 0x8

#define NI_NUMERICHOST 0x2
#define NI_NAMEREQD 0x4
#define NI_NUMERICSERV 0x8
#define NI_MAXSERV 32
#define SOMAXCONN 0x7fffffff

#define INADDR_ANY 0xffffffff

enum __SO
{
    SO_RANGE_BEGIN = 100,

#define SOL_SOCKET 22
    SO_DEBUG,
    SO_ACCEPTCONN,
    SO_REUSEADDR,
    SO_KEEPALIVE,
    SO_DONTROUTE,
    SO_BROADCAST,
    SO_LINGER,
    SO_OOBINLINE,
    SO_SNDBUF,
    SO_RCVBUF,
    SO_SNDLOWAT,
    SO_RCVLOWAT,
    SO_SNDTIMEO,
    SO_RCVTIMEO,
    SO_ERROR,
    SO_TYPE,
    SO_UPDATE_ACCEPT_CONTEXT,
    SO_UPDATE_CONNECT_CONTEXT,
    IP_OPTIONS,
    IP_HDRINCL,
    IP_TOS,
    IP_TTL,
    IP_MULTICAST_TTL,
    IP_MULTICAST_LOOP,
    IP_PKTINFO,
    IPV6_HOPLIMIT,
    IPV6_V6ONLY,
    TCP_NODELAY,

#define SOL_TCP 23

    _TCP_USER_TIMEOUT,
#define TCP_USER_TIMEOUT _TCP_USER_TIMEOUT

    SO_RANGE_END
};

enum __SHUT
{
    SHUT_RANGE_BEGIN = 1,

    SHUT_RD,
    SHUT_WR,
    SHUT_RDWR,

    SHUT_RANGE_END
};

struct timespec;

enum __ERRNO
{
    ERRNO_RANGE_BEGIN,
#undef EPERM
#undef ENOENT
#undef ESRCH
#undef EINTR
#undef EIO
#undef ENXIO
#undef E2BIG
#undef ENOEXEC
#undef EBADF
#undef ECHILD
#undef EAGAIN
#undef ENOMEM
#undef EACCES
#undef EFAULT
#undef EBUSY
#undef EEXIST
#undef EXDEV
#undef ENODEV
#undef ENOTDIR
#undef EISDIR
#undef EINVAL
#undef ENFILE
#undef EMFILE
#undef ENOTTY
#undef EFBIG
#undef ENOSPC
#undef ESPIPE
#undef EROFS
#undef EMLINK
#undef EPIPE
#undef EDOM
#undef ERANGE
#undef EDEADLK
#undef ENAMETOOLONG
#undef ENOLCK
#undef ENOSYS
#undef ENOTEMPTY
#undef EILSEQ
#undef EADDRINUSE
#undef EADDRNOTAVAIL
#undef EAFNOSUPPORT
#undef EALREADY
#undef EBADMSG
#undef ECANCELED
#undef ECONNABORTED
#undef ECONNREFUSED
#undef ECONNRESET
#undef EDESTADDRREQ
#undef EHOSTUNREACH
#undef EIDRM
#undef EINPROGRESS
#undef EISCONN
#undef ELOOP
#undef EMSGSIZE
#undef ENETDOWN
#undef ENETRESET
#undef ENETUNREACH
#undef ENOBUFS
#undef ENODATA
#undef ENOLINK
#undef ENOMSG
#undef ENOPROTOOPT
#undef ENOSR
#undef ENOSTR
#undef ENOTCONN
#undef ENOTRECOVERABLE
#undef ENOTSOCK
#undef ENOTSUP
#undef EOPNOTSUPP
#undef EOTHER
#undef EOVERFLOW
#undef EOWNERDEAD
#undef EPROTO
#undef EPROTONOSUPPORT
#undef EPROTOTYPE
#undef ETIME
#undef ETIMEDOUT
#undef ETXTBSY
#undef EWOULDBLOCK
#undef ESHUTDOWN
#undef EBADFD
    EPERM,
    ENOENT,
    ESRCH,
    EINTR,
    EIO,
    ENXIO,
    E2BIG,
    ENOEXEC,
    EBADF,
    ECHILD,
    EAGAIN,
    ENOMEM,
    EACCES,
    EFAULT,
    ENOTBLK,
    EBUSY,
    EEXIST,
    EXDEV,
    ENODEV,
    ENOTDIR,
    EISDIR,
    EINVAL,
    ENFILE,
    EMFILE,
    ENOTTY,
    ETXTBSY,
    EFBIG,
    ENOSPC,
    ESPIPE,
    EROFS,
    EMLINK,
    EPIPE,
    EDOM,
    ERANGE,
    EDEADLK,
    ENAMETOOLONG,
    ENOLCK,
    ENOSYS,
    ENOTEMPTY,
    ELOOP,
    ENOMSG,
    EIDRM,
    ECHRNG,
    EL2NSYNC,
    EL3HLT,
    EL3RST,
    ELNRNG,
    EUNATCH,
    ENOCSI,
    EL2HLT,
    EBADE,
    EBADR,
    EXFULL,
    ENOANO,
    EBADRQC,
    EBADSLT,
    EBFONT,
    ENOSTR,
    ENODATA,
    ETIME,
    ENOSR,
    ENONET,
    ENOPKG,
    EREMOTE,
    ENOLINK,
    EADV,
    ESRMNT,
    ECOMM,
    EPROTO,
    EMULTIHOP,
    EDOTDOT,
    EBADMSG,
    EOVERFLOW,
    ENOTUNIQ,
    EBADFD,
    EREMCHG,
    ELIBACC,
    ELIBBAD,
    ELIBSCN,
    ELIBMAX,
    ELIBEXEC,
    EILSEQ,
    ERESTART,
    ESTRPIPE,
    EUSERS,
    ENOTSOCK,
    EDESTADDRREQ,
    EMSGSIZE,
    EPROTOTYPE,
    ENOPROTOOPT,
    EPROTONOSUPPORT,
    ESOCKTNOSUPPORT,
    EOPNOTSUPP,
    EPFNOSUPPORT,
    EAFNOSUPPORT,
    EADDRINUSE,
    EADDRNOTAVAIL,
    ENETDOWN,
    ENETUNREACH,
    ENETRESET,
    ECONNABORTED,
    ECONNRESET,
    ENOBUFS,
    EISCONN,
    ENOTCONN,
    ESHUTDOWN,
    ETOOMANYREFS,
    ETIMEDOUT,
    ECONNREFUSED,
    EHOSTDOWN,
    EHOSTUNREACH,
    EALREADY,
    EINPROGRESS,
    ESTALE,
    EUCLEAN,
    ENOTNAM,
    ENAVAIL,
    EISNAM,
    EREMOTEIO,
    EDQUOT,
    ENOMEDIUM,
    EMEDIUMTYPE,
    ECANCELED,
    ENOKEY,
    EKEYEXPIRED,
    EKEYREVOKED,
    EKEYREJECTED,
    EOWNERDEAD,
    ENOTRECOVERABLE,
    EWOULDBLOCK,

    ERRNO_RANGE_END
};

enum __EAI
{
    EAI_RANGE_BEGIN = 200,

#undef EAI_AGAIN
#undef EAI_BADFLAGS
#undef EAI_FAMILY
#undef EAI_NONAME
#undef EAI_NODATA
#undef EAI_FAIL
    EAI_AGAIN,
    EAI_BADFLAGS,
    EAI_FAMILY,
    EAI_NONAME,
    EAI_NODATA,
    EAI_FAIL,

    EAI_RANGE_END,
};


enum __H_ERRNO
{
    H_ERRNO_RANGE_BEGIN = 300,

#undef HOST_NOT_FOUND
#undef TRY_AGAIN
#undef NO_RECOVERY
#undef NO_DATA
    HOST_NOT_FOUND,
    TRY_AGAIN,
    NO_RECOVERY,
    NO_DATA,

    H_ERRNO_RANGE_END,
};

#define MSG_OOB 0x1
#define MSG_PEEK 0x2
#define MSG_DONTROUTE 0x4
#define MSG_TRUNC 0x8
#define MSG_CTRUNC 0x10

//
// Windows types and defines to get windows os code to compile
//
#define _os_win_h_ 1

// Windows
typedef wchar_t WCHAR, *LPWSTR;
typedef char *LPSTR;
typedef unsigned char BYTE;
typedef const wchar_t *LPCWSTR;
typedef const char *LPCSTR, *PCSTR;
typedef unsigned int UINT, DWORD, *LPDWORD;
typedef unsigned long ULONG, *PULONG;
typedef unsigned long long ULONGLONG;
typedef bool BOOL, BOOLEAN, *LPBOOL;
typedef void* WSAEVENT;
typedef void VOID, *LPVOID, *PVOID, *HANDLE, *HMODULE;
typedef const void* LPCVOID;
typedef HANDLE *PHANDLE;
typedef unsigned short WORD, USHORT, INTERNET_PORT;
typedef void* DWORD_PTR, *HLOCAL;
#define INVALID_HANDLE_VALUE (HANDLE)-1
#define TRUE true
#define FALSE false
#define WT_EXECUTEDEFAULT 1
#define INFINITE (DWORD)-1
#define WINBASEAPI
#define WINAPI
#define MAX_PATH 256
#define WAIT_OBJECT_0 0
typedef struct _SECURITY_ATTRIBUTES SECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;
#define MAKEWORD(a, b) (WORD)((a << 8) + b)
#define rsize_t size_t

enum __ERRORS
{
    WIN_ERRORS_RANGE_BEGIN,

    WIN32_ERRORS_RANGE_BEGIN = WIN_ERRORS_RANGE_BEGIN,

    ERROR_SUCCESS,
    ERROR_NO_DATA,
    ERROR_INVALID_PARAMETER,
    ERROR_NOT_ENOUGH_MEMORY,
    ERROR_ADDRESS_NOT_ASSOCIATED,
    ERROR_BUFFER_OVERFLOW,
    ERROR_INVALID_OPERATION,
    ERROR_OPERATION_ABORTED,
    ERROR_INVALID_HANDLE,
    ERROR_IO_PENDING,
    ERROR_NOT_FOUND,

    WIN32_ERRORS_RANGE_END,

    NTSTATUS_ERRORS_RANGE_BEGIN = 200,

    STATUS_IO_TIMEOUT,
    STATUS_CANCELLED,
    STATUS_CONNECTION_ABORTED,
    STATUS_REQUEST_ABORTED,
    STATUS_CONNECTION_RESET,
    STATUS_CONNECTION_REFUSED,

    NTSTATUS_ERRORS_RANGE_END,

    WIN_ERRORS_RANGE_END = NTSTATUS_ERRORS_RANGE_END
};

#define TerminateProcess __TerminateProcess
#define MultiByteToWideChar __MultiByteToWideChar
#define GetLastError __GetLastError
#define FreeLibrary __FreeLibrary

#define CP_UTF8 1

#define FORMAT_MESSAGE_ALLOCATE_BUFFER 0x1
#define FORMAT_MESSAGE_FROM_SYSTEM 0x2
#define FORMAT_MESSAGE_IGNORE_INSERTS 0x4
#define FORMAT_MESSAGE_FROM_HMODULE 0x8
#define FORMAT_MESSAGE_MAX_WIDTH_MASK 0

#define LANG_NEUTRAL 1
#define SUBLANG_DEFAULT 2
#define MAKELANGID(a, b) (DWORD)((a << 16) + b)

#define INVALID_FILE_ATTRIBUTES 0xffff
#define FILE_ATTRIBUTE_DIRECTORY 0x2

// synchapi.h
typedef int SRWLOCK, *PSRWLOCK;

// process.h
typedef struct _STARTUPINFO 
{
    DWORD   cb;
} 
STARTUPINFO, *LPSTARTUPINFO;

typedef struct _PROCESS_INFORMATION 
{
    HANDLE hProcess;
    HANDLE hThread;
} 
PROCESS_INFORMATION, *LPPROCESS_INFORMATION;

#define CREATE_NEW_CONSOLE 0x10
#define NORMAL_PRIORITY_CLASS 0x20

typedef struct _OVERLAPPED {
    void* Internal;
    void* InternalHigh;
    union {
        struct {
            DWORD Offset;
            DWORD OffsetHigh;
        } DUMMYSTRUCTNAME;
        PVOID Pointer;
    } DUMMYUNIONNAME;

    HANDLE  hEvent;
} OVERLAPPED, *LPOVERLAPPED;

typedef void(*LPOVERLAPPED_COMPLETION_ROUTINE)(
    DWORD error, DWORD bytes, LPOVERLAPPED ov
    );

BOOL WINAPI HasOverlappedIoCompleted(
    LPOVERLAPPED lpOverlapped
);

// Wincrypt.h

typedef void* HCRYPTPROV;
#define PROV_RSA_FULL 1
#define CRYPT_VERIFYCONTEXT     0x10
#define CRYPT_SILENT            0x40

// Winsock
typedef intptr_t SOCKET;
#define WSAPoll poll
#define WSAAPI
#define CALLBACK
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define WSA_WAIT_EVENT_0 0
typedef struct sockaddr SOCKADDR;
typedef struct addrinfo ADDRINFOA, *PADDRINFOA;
typedef struct _SOCKADDR_STORAGE
{
    int ss_family;
    char reserved[127];
}
SOCKADDR_STORAGE;

typedef struct _SOCKET_ADDRESS
{
    struct sockaddr* lpSockaddr;
    int iSockaddrLength;
} 
SOCKET_ADDRESS, *LPSOCKET_ADDRESS;

#define SIO_GET_EXTENSION_FUNCTION_POINTER 1
typedef int GUID;
typedef
BOOL (*LPFN_ACCEPTEX)(SOCKET sListenSocket, SOCKET sAcceptSocket, PVOID lpOutputBuffer, 
    DWORD dwReceiveDataLength, DWORD dwLocalAddressLength, DWORD dwRemoteAddressLength,
    LPDWORD lpdwBytesReceived, LPOVERLAPPED lpOverlapped
    );
#define WSAID_ACCEPTEX 1
typedef BOOL (*LPFN_CONNECTEX) (SOCKET s, const struct sockaddr* name, int namelen, 
    PVOID lpSendBuffer, DWORD dwSendDataLength, LPDWORD lpdwBytesSent, LPOVERLAPPED lpOverlapped
    );
#define WSAID_CONNECTEX 2

typedef void (*LPFN_GETACCEPTEXSOCKADDRS)(PVOID lpOutputBuffer, DWORD dwReceiveDataLength,
    DWORD dwLocalAddressLength, DWORD dwRemoteAddressLength, struct sockaddr **LocalSockaddr,
    int* LocalSockaddrLength, struct sockaddr **RemoteSockaddr, int* RemoteSockaddrLength
    );
#define WSAID_GETACCEPTEXSOCKADDRS 3

typedef struct _WSADATA
{
    int reserved;
}
WSADATA,  *LPWSADATA;
typedef OVERLAPPED WSAOVERLAPPED, *LPWSAOVERLAPPED;
typedef LPOVERLAPPED_COMPLETION_ROUTINE LPWSAOVERLAPPED_COMPLETION_ROUTINE;

typedef int GROUP;
typedef struct _WSAPROTOCOL_INFO
{
    int reserved;
}
WSAPROTOCOL_INFO, *LPWSAPROTOCOL_INFO;
#define WSA_FLAG_OVERLAPPED 102

typedef struct _WSABUF 
{
    ULONG len;     
    char *buf; 
} 
WSABUF, *LPWSABUF;

struct ip_mreq 
{
    struct in_addr imr_multiaddr; 
    struct in_addr imr_interface;
};

enum __WSAERRORS
{
    WSA_ERRORS_RANGE_BEGIN = 1000,
    WSAEINTR,
    WSAEBADF,
    WSAEACCES,
    WSAEFAULT,
    WSAEINVAL,
    WSAEMFILE,
    WSAEWOULDBLOCK,
    WSAEINPROGRESS,
    WSAEALREADY,
    WSAENOTSOCK,
    WSAEDESTADDRREQ,
    WSAEMSGSIZE,
    WSAEPROTOTYPE,
    WSAENOPROTOOPT,
    WSAEPROTONOSUPPORT,
    WSAEOPNOTSUPP,
    WSAEAFNOSUPPORT,
    WSAEADDRINUSE,
    WSAEADDRNOTAVAIL,
    WSAHOST_NOT_FOUND,
    WSAENETDOWN,
    WSAENETUNREACH,
    WSAENETRESET,
    WSAECONNABORTED,
    WSAECONNRESET,
    WSAENOBUFS,
    WSAEISCONN,
    WSAENOTCONN,
    WSAETIMEDOUT,
    WSAECONNREFUSED,
    WSAELOOP,
    WSAENAMETOOLONG,
    WSAEHOSTUNREACH,
    WSAENOTEMPTY,
    WSA_IO_PENDING,
    WSA_IO_INCOMPLETE,
    WSA_INVALID_HANDLE,
    WSA_INVALID_PARAMETER,
    WSA_NOT_ENOUGH_MEMORY,
    WSA_OPERATION_ABORTED,
    WSAESOCKTNOSUPPORT,
    WSAEPFNOSUPPORT,
    WSAESHUTDOWN,
    WSAETOOMANYREFS,
    WSAEHOSTDOWN,
    WSAEPROCLIM,
    WSAEUSERS,
    WSAEDQUOT,
    WSAESTALE,
    WSAEREMOTE,
    WSAEDISCON,
    WSAENOMORE,
    WSAECANCELLED,
    WSAEREFUSED,
    WSANOTINITIALISED,
    WSASYSCALLFAILURE,
    WSA_ERRORS_RANGE_END,
};


// iphlpapi.h
#define GAA_FLAG_INCLUDE_PREFIX 0x1
#define GAA_FLAG_SKIP_DNS_SERVER 0x2
#define GAA_FLAG_SKIP_FRIENDLY_NAME 0x4
#define GAA_FLAG_SKIP_ANYCAST 0x8
#define GAA_FLAG_SKIP_MULTICAST 0x10

typedef struct _IP_ADAPTER_UNICAST_ADDRESS 
{
    struct _IP_ADAPTER_UNICAST_ADDRESS *Next;
    SOCKET_ADDRESS Address;
    uint8_t OnLinkPrefixLength;
}
IP_ADAPTER_UNICAST_ADDRESS, *PIP_ADAPTER_UNICAST_ADDRESS;

typedef enum __IF_OPER_STATUS
{
    IfOperStatusUp = 1,
} 
IF_OPER_STATUS;

typedef struct _IP_ADAPTER_ADDRESSES 
{
    ULONG IfIndex;
    struct _IP_ADAPTER_ADDRESSES *Next;
    char* AdapterName;
    IF_OPER_STATUS OperStatus;
    DWORD IfType;
    DWORD Flags;
    PIP_ADAPTER_UNICAST_ADDRESS FirstUnicastAddress;
} 
IP_ADAPTER_ADDRESSES, *PIP_ADAPTER_ADDRESSES;

#define IF_TYPE_SOFTWARE_LOOPBACK 100
#define IP_ADAPTER_NO_MULTICAST 0x10

// winhttp.h
typedef void* HINTERNET;

typedef void (*WINHTTP_STATUS_CALLBACK)(
    HINTERNET hInternet,
    DWORD_PTR dwContext,
    DWORD dwInternetStatus,
    LPVOID lpvStatusInformation,
    DWORD dwStatusInformationLength
);
#define WINHTTP_INVALID_STATUS_CALLBACK ((WINHTTP_STATUS_CALLBACK)(-1L))
#define HTTP_STATUS_SWITCH_PROTOCOLS 101

enum __WINHTTPERRORS
{
    WINHTTP_ERRORS_RANGE_BEGIN = 3000,
    WINHTTP_ERROR_BASE = WINHTTP_ERRORS_RANGE_BEGIN,

    ERROR_WINHTTP_OUT_OF_HANDLES,
    ERROR_WINHTTP_TIMEOUT,
    ERROR_WINHTTP_INTERNAL_ERROR,
    ERROR_WINHTTP_INVALID_URL,
    ERROR_WINHTTP_UNRECOGNIZED_SCHEME,
    ERROR_WINHTTP_NAME_NOT_RESOLVED,
    ERROR_WINHTTP_SHUTDOWN,
    ERROR_WINHTTP_LOGIN_FAILURE,
    ERROR_WINHTTP_OPERATION_CANCELLED,
    ERROR_WINHTTP_INCORRECT_HANDLE_TYPE,
    ERROR_WINHTTP_INCORRECT_HANDLE_STATE,
    ERROR_WINHTTP_CANNOT_CONNECT,
    ERROR_WINHTTP_CONNECTION_ERROR,
    ERROR_WINHTTP_RESEND_REQUEST,
    ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED,
    ERROR_WINHTTP_INVALID_SERVER_RESPONSE,
    ERROR_WINHTTP_SECURE_FAILURE,

    WINHTTP_ERRORS_RANGE_END,
    WINHTTP_ERROR_LAST = WINHTTP_ERRORS_RANGE_END
};


typedef enum _WINHTTP_CALLBACK_STATUS
{
    WINHTTP_CALLBACK_STATUS_RESOLVING_NAME,
    WINHTTP_CALLBACK_STATUS_NAME_RESOLVED,
    WINHTTP_CALLBACK_STATUS_CONNECTING_TO_SERVER,
    WINHTTP_CALLBACK_STATUS_CONNECTED_TO_SERVER,
    WINHTTP_CALLBACK_STATUS_SENDING_REQUEST,
    WINHTTP_CALLBACK_STATUS_REQUEST_SENT,
    WINHTTP_CALLBACK_STATUS_RECEIVING_RESPONSE,
    WINHTTP_CALLBACK_STATUS_RESPONSE_RECEIVED,
    WINHTTP_CALLBACK_STATUS_CLOSING_CONNECTION,
    WINHTTP_CALLBACK_STATUS_CONNECTION_CLOSED,
    WINHTTP_CALLBACK_STATUS_HANDLE_CREATED,
    WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING,
    WINHTTP_CALLBACK_STATUS_DETECTING_PROXY,
    WINHTTP_CALLBACK_STATUS_REDIRECT,
    WINHTTP_CALLBACK_STATUS_INTERMEDIATE_RESPONSE,
    WINHTTP_CALLBACK_STATUS_SECURE_FAILURE,
    WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE,
    WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE,
    WINHTTP_CALLBACK_STATUS_READ_COMPLETE,
    WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE,
    WINHTTP_CALLBACK_STATUS_REQUEST_ERROR,
    WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE,
    WINHTTP_CALLBACK_STATUS_GETPROXYFORURL_COMPLETE,
    WINHTTP_CALLBACK_STATUS_CLOSE_COMPLETE,
    WINHTTP_CALLBACK_STATUS_SHUTDOWN_COMPLETE
}
WINHTTP_CALLBACK_STATUS;

#define WINHTTP_WEB_SOCKET_ABORTED_CLOSE_STATUS 22
#define WINHTTP_WEB_SOCKET_ENDPOINT_TERMINATED_CLOSE_STATUS 33

typedef enum _WINHTTP_WEB_SOCKET_BUFFER_TYPE
{
    WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE,
    WINHTTP_WEB_SOCKET_BINARY_FRAGMENT_BUFFER_TYPE,
    WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE,
    WINHTTP_WEB_SOCKET_UTF8_FRAGMENT_BUFFER_TYPE,
    WINHTTP_WEB_SOCKET_CLOSE_BUFFER_TYPE
} 
WINHTTP_WEB_SOCKET_BUFFER_TYPE;
typedef struct _WINHTTP_WEB_SOCKET_STATUS
{
    DWORD dwBytesTransferred;
    WINHTTP_WEB_SOCKET_BUFFER_TYPE eBufferType;
} 
WINHTTP_WEB_SOCKET_STATUS;

#define WINHTTP_OPTION_CONTEXT_VALUE                    45
#define WINHTTP_OPTION_CLIENT_CERT_CONTEXT              47
#define WINHTTP_OPTION_CONNECT_RETRIES                  53
#define WINHTTP_OPTION_ASSURED_NON_BLOCKING_CALLBACKS  188
#define WINHTTP_OPTION_MAX_CONNS_PER_SERVER             55
#define WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET           114
#define WINHTTP_OPTION_PROXY_USERNAME                    6
#define WINHTTP_OPTION_PROXY_PASSWORD                   73
#define WINHTTP_ACCESS_TYPE_NAMED_PROXY                 33
#define WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY              4

#define WINHTTP_QUERY_STATUS_CODE                       19 
#define WINHTTP_QUERY_FLAG_NUMBER               0x40000000
#define WINHTTP_ADDREQ_FLAG_ADD                 0x20000000
#define WINHTTP_ADDREQ_FLAG_REPLACE             0x80000000
#define WINHTTP_FLAG_SECURE                     0x00001000
#define WINHTTP_FLAG_ASYNC                      0x10000000 

typedef struct _WINHTTP_ASYNC_RESULT
{
    DWORD_PTR dwResult;  
    DWORD dwError;
}
WINHTTP_ASYNC_RESULT, *LPWINHTTP_ASYNC_RESULT;

//
// Linux types and defines to get linux functionality to build
//

#define _os_linux_h_ 1

// unistd.h
#define F_SETFL 1 
#define F_GETFL 1 
#define O_NONBLOCK 0x1

#define _PC_PATH_MAX 100
#define F_OK 5

// time.h
#define CLOCK_MONOTONIC 4
#if !defined(__clockid_t_defined)
typedef int clockid_t;
#endif

// sys/types.h
typedef int pid_t;

// signal.h
#define SIGINT 2

// syscall.h
#define SYS_getrandom 111

// epoll.h
typedef union epoll_data 
{
    void    *ptr;
    int      fd;
    uint32_t u32;
    uint64_t u64;
} 
epoll_data_t;

struct epoll_event 
{
    uint32_t     events; 
    epoll_data_t data; 
};

#define EPOLL_CLOEXEC 1

enum __EPOLL_CTL
{
    EPOLL_CTL_ADD,
    EPOLL_CTL_MOD,
    EPOLL_CTL_DEL
};

enum __EPOLLEVENT
{
    EPOLLIN = 0x200000,
    EPOLLOUT = 0x400000,
    EPOLLERR = 0x800000,
    EPOLLHUP = 0x1000000,
    EPOLLET = 0x2000000,
    EPOLLONESHOT = 0x4000000
};

// ifaddrs.h
struct ifaddrs 
{
    struct ifaddrs *ifa_next; 
    char *ifa_name;
    unsigned int ifa_flags; 
    struct sockaddr *ifa_addr; 
    struct sockaddr *ifa_netmask;
    union {
        struct sockaddr *ifu_broadaddr;
        struct sockaddr *ifu_dstaddr;
    } ifa_ifu;
#define ifa_broadaddr ifa_ifu.ifu_broadaddr
#define ifa_dstaddr   ifa_ifu.ifu_dstaddr
    void *ifa_data;
};

#define IFF_UP 0x1
#define IFF_LOOPBACK 0x2
#define IFF_MULTICAST 0x4


// in.h
struct ip_mreqn {
    struct in_addr imr_multiaddr;
    struct in_addr imr_address;
    int imr_ifindex;
};
#define IP_DROP_MEMBERSHIP 21
#define IP_ADD_MEMBERSHIP 20

// in6.h
struct ipv6_mreq 
{
    struct in6_addr ipv6mr_multiaddr;
    int ipv6mr_interface;
};

#define IPV6_ADD_MEMBERSHIP    20
#define IPV6_DROP_MEMBERSHIP    21

// ioctl.h
#define FIONREAD 1
#define FIONBIO 2

// uuid/uuid.h
typedef uint8_t uuid_t[16];

// spawn.h

typedef struct posix_spawnattr posix_spawnattr_t;
typedef struct posix_spawn_file_actions posix_spawn_file_actions_t;

// sys/event.h
struct kevent {
    uintptr_t ident;  
    int16_t filter; 
    uint16_t flags;  
    uint32_t fflags; 
    intptr_t data;   
    void *udata; 
};

enum EVFILT
{
    EVFILT_READ = 1,
    EVFILT_WRITE
};

#define EV_EOF 0x100
#define EV_ERROR 0x200
#define EV_ADD 0x400
#define EV_CLEAR 0x800
#define EV_RECEIPT 0x1000
#define EV_DELETE 0x2000

// libwebsockets.h
struct lws;
struct lws_extension;
struct lws_context;

#define LWS_LIBRARY_VERSION_MAJOR 2
#define LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT 0x1

enum lws_callback_reasons
{
    LWS_CALLBACK_CLIENT_CONNECTION_ERROR,
    LWS_CALLBACK_CLIENT_ESTABLISHED,
    LWS_CALLBACK_CLIENT_RECEIVE,
    LWS_CALLBACK_CLIENT_WRITEABLE,
    LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER,
    LWS_CALLBACK_CLOSED,
    LWS_CALLBACK_WSI_DESTROY,
    LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS,
    LWS_CALLBACK_GET_THREAD_ID,
    LWS_CALLBACK_CHANGE_MODE_POLL_FD,
    LWS_CALLBACK_LOCK_POLL,
    LWS_CALLBACK_UNLOCK_POLL
};

enum lws_close_status {
    LWS_CLOSE_STATUS_NOSTATUS,
    LWS_CLOSE_STATUS_NORMAL,
    LWS_CLOSE_STATUS_GOINGAWAY
};

struct lws_context_creation_info 
{
    int port;
    const struct lws_protocols *protocols;
    const struct lws_extension *extensions;
    const char *http_proxy_address;
    unsigned int http_proxy_port;
    int gid;
    int uid;
    void *user;
    int options;
};

typedef int (callback_function)(struct lws *wsi,
    enum lws_callback_reasons reason, void *user,
    void *in, size_t len);

typedef void(*log_emit_function)(int level, const char *line);

struct lws_protocols 
{
    const char *name;
    callback_function *callback;
    void *user;
};

#define LCCSCF_USE_SSL 0x1

struct lws_client_connect_info 
{
    struct lws_context *context;
    const char *address;
    int port;
    int ssl_connection;
    const char *path;
    const char *host;
    const char *origin;
    const char *protocol;
    int ietf_version_or_minus_one;
    void *userdata;
    struct lws **pwsi;
};

enum lws_write_protocol {
    LWS_WRITE_CONTINUATION = 0x1,
    LWS_WRITE_BINARY = 0x2,
    LWS_WRITE_TEXT = 0x4,
    LWS_WRITE_NO_FIN = 0x8
};

#define LWS_SEND_BUFFER_PRE_PADDING 4 + 10 + (2 * 4)
#define LWS_SEND_BUFFER_POST_PADDING 4
#define CONTEXT_PORT_NO_LISTEN -1
#define getdtablesize() 20

// openssl.h
typedef void X509_STORE;
typedef void BIO;
typedef void BIO_METHOD;
typedef void X509;
typedef void SSL_CTX;
typedef void pem_password_cb;

//
// Adapter types
//
typedef int fd_t;
#define _invalid_fd -1
typedef int socklen_t;
typedef int socksize_t;
typedef int sockssize_t;
typedef char sockbuf_t;
typedef unsigned long saddr_t;
typedef void ifinfo_t;
typedef void ifaddr_t;
#define EAI_NODATA 9999

#define _fd_nonblock(fd, r)

#endif // _os_mock_h_