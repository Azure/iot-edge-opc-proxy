// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os.h"
#include "util_mem.h"

#include "pal_ev.h"
#include "pal_sk.h"
#include "pal_scan.h"
#include "pal_net.h"
#include "pal_err.h"
#include "pal_time.h"
#include "pal_types.h"

#include "util_string.h"
#include "util_misc.h"

#include <stdio.h>

#if !defined(UNIT_TEST)
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#endif

//
// Scan task represents an individual probe
//
typedef struct pal_scan_probe pal_scan_probe_t;

//
// Tristate of scan probe
//
typedef enum pal_scan_probe_state
{
    pal_scan_probe_idle = 1,
    pal_scan_probe_working,
    pal_scan_probe_done
}
pal_scan_probe_state_t;

//
// Scan task represents an individual probe
//
struct pal_scan_probe
{
#define PROBE_TIMEOUT 600
    pal_scan_t* scan;                         // Owning scan session
    ticks_t probe_start;          // Start time of probe for timeout
    pal_scan_probe_state_t state;              // State of the probe
    int itf_index;
    struct sockaddr_in6 from;                      // Source address
    struct sockaddr_in6 to;                   // Destination address
    int sock_fd;                          // Socket used for probing
    uintptr_t event_handle;        // in progress event registration
};

//
// The context for ip and port scanning
//
struct pal_scan
{
    int32_t flags;
    struct sockaddr_in6 address;    // Address of host for port scan
    uint16_t port;  // Port or 0 if only addresses are to be scanned
    pal_scan_cb_t cb;
    void* context;
    uintptr_t event_port; // Event port to use for all notifications
    int32_t timeout;                      // Next event loop timeout

    uint32_t ip_scan_itf;
    uint32_t ip_scan_cur;         // Current ip address to be probed
    uint32_t ip_scan_end;             // End ip address + 1 to probe
    uint16_t port_scan_cur;             // Current port to be probed
    uint32_t port_scan_end;                 // End port + 1 to probe

#define MAX_PROBES 1024
    pal_scan_probe_t tasks[MAX_PROBES];            // Probe contexts
    struct ifaddrs* ifaddr;                    // Allocated if infos
    int netlink_fd;               // For neighbor table notification
    uintptr_t netlink_events;          // netlink event registration
    bool destroy;                // Whether the scan should be freed

    struct ifaddrs* ifcur;               // Iterate through adapters
    bool cache_exhausted;        // Whether all lists were exhausted
    log_t log;
};

//
// Notify user of new result
//
static void pal_scan_notify(
    pal_scan_t* scan,
    int itf_index,
    struct sockaddr* to
)
{
    int32_t result;
    prx_socket_address_t prx_addr;
    char buf[128];

    dbg_assert_ptr(scan);
    dbg_assert_ptr(to);

    result = pal_os_to_prx_socket_address(__sa_base(to), __sa_size(to),
        &prx_addr);
    if (result != er_ok)
    {
        log_error(scan->log, "Failed to convert address (%s)",
            prx_err_string(result));
        return;
    }
    if (scan->port == 0)
        prx_addr.un.ip.port = 0;

    if (prx_addr.un.family == prx_address_family_inet6)
    {
        log_debug(scan->log, "Found " __prx_sa_in6_fmt,
            __prx_sa_in6_args(&prx_addr));
    }
    else
    {
        dbg_assert(prx_addr.un.family == prx_address_family_inet,
            "af wrong");
        log_debug(scan->log, "Found " __prx_sa_in4_fmt,
            __prx_sa_in4_args(&prx_addr));
    }

    buf[0] = 0;
    if (0 == (scan->flags & pal_scan_no_name_lookup))
    {
        if (__sa_base(&scan->address)->sa_family == AF_UNSPEC)
        {
            (void)getnameinfo(__sa_base(to), __sa_size(to),
                buf, sizeof(buf), NULL, 0, 0);
        }
        else
        {
            (void)getnameinfo(__sa_base(to), __sa_size(to),
                NULL, 0, buf, sizeof(buf), 0);
        }
    }

    dbg_assert_ptr(scan->cb);
    scan->cb(scan->context, itf_index, er_ok, &prx_addr,
        buf[0] ? buf : NULL);
}

//
// Begin next probe on the probe task
//
static void pal_scan_next(
    pal_scan_probe_t* task
);

//
// Complete the probe task - this is called on event port thread.
//
static void pal_scan_probe_complete(
    pal_scan_probe_t* task
)
{
    uintptr_t evt_handle;
    dbg_assert_ptr(task);

    if (task->state == pal_scan_probe_done)
        pal_scan_notify(task->scan, task->itf_index, __sa_base(&task->to));

    evt_handle = task->event_handle;
    if (evt_handle == 0)
    {
        //
        // If not registered with the event port, need to close manually
        // and continue.
        //
        if (task->sock_fd != -1)
            close(task->sock_fd);
        task->sock_fd = -1;
        pal_scan_next(task);
        return;
    }

    //
    // Otherwise, close event.  We will get a destroy callback once
    // unregistered and socket is closed.  Note that we are running on
    // the same thread as the reschedule thread, so this provides
    // threadsafe access to the event handle (to avoid double close).
    //
    task->event_handle = 0;
    task->sock_fd = -1;
    pal_event_close(evt_handle, true);
}

//
// Called by event port when event occurred on registered socket
//
static int32_t pal_scan_probe_cb(
    void* context,
    pal_event_type_t event_type,
    int32_t error_code
)
{
    int error;
    pal_scan_probe_t* task = (pal_scan_probe_t*)context;
    socklen_t len = sizeof(error);
    dbg_assert_ptr(task);

    switch (event_type)
    {
    case pal_event_type_read:
    case pal_event_type_write:
        if (error_code == er_ok)
        {
            len = sizeof(error);
            if (0 == getsockopt(task->sock_fd, SOL_SOCKET, SO_ERROR,
                (sockbuf_t*)&error, &len) &&
                error == 0)
            {
                task->state = pal_scan_probe_done;
            }
        }
        pal_scan_probe_complete(task);
        return er_aborted;
    case pal_event_type_error:
    case pal_event_type_close:
        pal_scan_probe_complete(task);
        return er_aborted;
    case pal_event_type_destroy:
        // Completed closing - fd is now closed as well
        dbg_assert(task->event_handle == 0, "Unexpected handle %d",
            task->event_handle);
        dbg_assert(task->sock_fd == -1, "Unexpected fd %d",
            task->sock_fd);
        pal_scan_next(task);
        return er_ok;
    case pal_event_type_unknown:
    default:
        dbg_assert(0, "Unknown event type %d", event_type);
        return er_bad_state;
    }
}

//
// Probes tcp port - this is called on event port thread.
//
void pal_scan_probe_begin(
    pal_scan_probe_t* task
)
{
    int32_t result;
    dbg_assert_ptr(task);
    do
    {
        task->probe_start = ticks_get();
        task->sock_fd = socket(__sa_base(&task->from)->sa_family,
            SOCK_STREAM, IPPROTO_TCP);
        if (task->sock_fd == -1)
        {
            result = pal_os_last_net_error_as_prx_error();
            break;
        }

        if (0 != bind(task->sock_fd, __sa_base(&task->from),
            __sa_size(&task->from)))
        {
            result = pal_os_last_net_error_as_prx_error();
            break;
        }


        // Register with event port
        result = pal_event_port_register(task->scan->event_port,
            task->sock_fd, pal_scan_probe_cb, task, &task->event_handle);
        if (result != er_ok)
            break;

        if (task->scan->port == 0)
        {
            if (__sa_is_in4(&task->to))
                __sa_as_in4(&task->to)->sin_port = swap_16(40000);
            else
                __sa_as_in6(&task->to)->sin6_port = swap_16(40000);
        }

        if (0 == connect(task->sock_fd, __sa_base(&task->to),
            __sa_size(&task->to)))
        {
            // Connected synchronously
            task->state = pal_scan_probe_done;
            break;
        }

        result = pal_os_last_net_error_as_prx_error();
        if (result != er_waiting && result != er_retry)
        {
            log_error(task->scan->log, "Failed to probe port (%s)",
                prx_err_string(result));
            break;
        }

        // Select write callback
        result = pal_event_select(task->event_handle, pal_event_type_write);
        if (result != er_ok)
            break;

        // Wait for completion indicated by write event or timeout.
        return;
    }
    while (0);

    // Probe completed synchronously or failed, complete and continue...
    pal_scan_probe_complete(task);
}

//
// Get next port to be scanned
//
static int32_t pal_scan_get_next_port(
    pal_scan_t* scan,
    struct sockaddr* from,
    struct sockaddr* to
)
{
    uint16_t port;

    dbg_assert_ptr(scan);

    if (scan->cache_exhausted)
        return er_nomore;

    // Select next port
    if (scan->port_scan_cur >= scan->port_scan_end)
    {
        // No more candidates
        // Notify we are done.
        scan->cache_exhausted = true;
        scan->cb(scan->context, 0, er_nomore, NULL, NULL);
        log_trace(scan->log, "Port scan completed.");
        return er_nomore;
    }

    port = (uint16_t)scan->port_scan_cur++;

    // Perform actual scan action - update address with target and port
    memset(from, 0, sizeof(struct sockaddr_in6));
    memcpy(to, &scan->address, sizeof(struct sockaddr_in6));

    // Connect to port
    __sa_base(from)->sa_family = __sa_base(to)->sa_family;
    if (__sa_is_in6(to))
        __sa_as_in6(to)->sin6_port = port;
    else
        __sa_as_in4(to)->sin_port = port;
    return er_ok;
}

//
// Get next address to be scanned
//
static int32_t pal_scan_get_next_address(
    pal_scan_t* scan,
    struct sockaddr* from,
    struct sockaddr* to
)
{
    uint32_t subnet_mask;
    dbg_assert_ptr(scan);

    if (scan->cache_exhausted)
        return er_nomore;

    while (true)
    {
        if (scan->ip_scan_cur < scan->ip_scan_end)
        {
            __sa_base(to)->sa_family = AF_INET;
            __sa_as_in4(to)->sin_addr.s_addr = swap_32(scan->ip_scan_cur);

            __sa_base(from)->sa_family = AF_INET;
            __sa_as_in4(from)->sin_addr.s_addr = scan->ip_scan_itf;
            __sa_as_in4(from)->sin_port = 0;

            // Update address to add any port
            __sa_as_in4(to)->sin_port = swap_16(scan->port);
            scan->ip_scan_cur++;
            return er_ok;
        }

        // See if we can set next range from current unicast address
        if (scan->ifcur)
        {
            log_trace(scan->log, "-> %s (flags:%x)", scan->ifcur->ifa_name,
                scan->ifcur->ifa_flags);
            if (0 != (scan->ifcur->ifa_flags & IFF_UP) &&
                0 == (scan->ifcur->ifa_flags & IFF_LOOPBACK) &&
                __sa_is_in4(scan->ifcur->ifa_addr))
            {
                scan->ip_scan_itf = __sa_as_in4(scan->ifcur->ifa_addr)->sin_addr.s_addr;
                subnet_mask = __sa_as_in4(scan->ifcur->ifa_netmask)->sin_addr.s_addr;

                log_trace(scan->log, "Scanning " __sa_in4_fmt " (" __sa_in4_fmt ")",
                    __sa_in4_args(scan->ifcur->ifa_addr),
                    __sa_in4_args(scan->ifcur->ifa_netmask));

                scan->ip_scan_end = scan->ip_scan_itf | ~subnet_mask;
                scan->ip_scan_cur = scan->ip_scan_itf & subnet_mask;

                scan->ip_scan_cur = swap_32(scan->ip_scan_cur);
                scan->ip_scan_end = swap_32(scan->ip_scan_end);
                scan->ip_scan_end++;
            }
            scan->ifcur = scan->ifcur->ifa_next;
            continue;
        }

        // Notify we are done.
        scan->cache_exhausted = true;
        scan->cb(scan->context, 0, er_nomore, NULL, NULL);
        log_trace(scan->log, "IP scan completed.");
        return er_nomore;
    }
}

//
// Begin next probe on the probe task - this is called on event port thread.
//
static void pal_scan_next(
    pal_scan_probe_t* task
)
{
    int32_t result;
    dbg_assert_ptr(task);

    while (!task->scan->destroy) // Fill as many tasks as possible until er_nomore
    {
        if (task->scan->address.sin6_family == AF_UNSPEC)
        {
            result = pal_scan_get_next_address(task->scan,
                __sa_base(&task->from), __sa_base(&task->to));
        }
        else
        {
            result = pal_scan_get_next_port(task->scan,
                __sa_base(&task->from), __sa_base(&task->to));
        }

        if (result == er_nomore)
        {
            task->state = pal_scan_probe_done;
            return;
        }

        if (result == er_ok)
        {
            // Update address to add any port
            if (__sa_is_in6(&task->to))
            {
                log_debug(task->scan->log,
                    "Probe on " __sa_in6_fmt " for " __sa_in6_fmt,
                    __sa_in6_args(&task->from), __sa_in6_args(&task->to));
            }
            else
            {
                log_debug(task->scan->log,
                    "Probe on " __sa_in4_fmt " for " __sa_in4_fmt,
                    __sa_in4_args(&task->from), __sa_in4_args(&task->to));
            }

            task->state = pal_scan_probe_working;
            // Perform actual scan action
            pal_scan_probe_begin(task);
            return;
        }
    }
}

//
// Arp cache polling - correlates the current arp cache to ongoing probes.
//
static void pal_scan_arp_cache_check(
    pal_scan_t* scan
)
{
    FILE *fp;
    char *pos, *line = NULL; // NULL so that getline allocates and reallocates.
    ssize_t read;
    size_t len = 0;
    int flags;
    uint32_t ip;
    const char* mac, *dev;

    dbg_assert_ptr(scan);
    fp = fopen("/proc/scan/arp", "r");
    if (!fp)
        return;
    read = getline(&line, &len, fp); // Skip header of arp cache
    while (read >= 0 && line)
    {
        // Parse arp cache line by line.
        read = getline(&line, &len, fp);
        if (read < 0 || !line)
            continue;
        // Parse ipv4 address
        ((uint8_t*)&ip)[0] = (uint8_t)strtol(line, &pos, 10);
        if (*pos++ != '.') continue;
        ((uint8_t*)&ip)[1] = (uint8_t)strtol(pos, &pos, 10);
        if (*pos++ != '.') continue;
        ((uint8_t*)&ip)[2] = (uint8_t)strtol(pos, &pos, 10);
        if (*pos++ != '.') continue;
        ((uint8_t*)&ip)[3] = (uint8_t)strtol(pos, &pos, 10);
        while (*pos == ' ') pos++; // Skip spaces
        if (!*pos) continue;
        // Parse type
        flags = strtol(pos, &pos, 0x10);
        if (!*pos) continue;
        while (*pos == ' ') pos++; // Skip spaces
        // Parse flags
        flags = strtol(pos, &pos, 0x10);
        if (!flags || !*pos) // flags == 0, incomplete, else == complete.
            continue; // Save ourselves from parsing more
        while (*pos == ' ') pos++; // Skip spaces
        mac = pos;  // Save mac
        while (*pos != ' ') pos++;
        *pos++ = 0;
        while (*pos == ' ') pos++; // Skip spaces
        while (*pos != ' ') pos++; // Skip mask
        while (*pos == ' ') pos++; // Skip spaces
        dev = pos;  // Save dev
        while (*pos && *pos != '\n') pos++;
        *pos = 0;
        for (size_t i = 0; i < _countof(scan->tasks); i++)
        {
            // Complete the probe entry that matches the ip address.
            if (scan->tasks[i].state == pal_scan_probe_working &&
                __sa_is_in4(&scan->tasks[i].to) &&
                __sa_as_in4(&scan->tasks[i].to)->sin_addr.s_addr == ip)
            {
                if (scan->tasks[i].itf_index != 0 &&
                    scan->tasks[i].itf_index != (int)if_nametoindex(dev))
                    continue;

                scan->tasks[i].state = pal_scan_probe_done;
                pal_scan_probe_complete(&scan->tasks[i]);
            }
        }
    }
    if (line)
        free(line);
    fclose(fp);
}

//
// Scan timer callback - this is called on event port thread.
//
static int32_t pal_scan_scheduler(
    void* context,
    bool no_events
)
{
    pal_scan_t* scan = (pal_scan_t*)context;
    ticks_t now, tmp;
    int32_t reschedule;
    uintptr_t evt_handle;

    dbg_assert_ptr(scan);
    (void)no_events;

    // Synchronize tasks with arp cache
    if (scan->port == 0 && !scan->destroy)
        pal_scan_arp_cache_check(scan);

    // Run through all tasks and determine whether they are old
    now = ticks_get();
    reschedule = PROBE_TIMEOUT;
    for (size_t i = 0; i < _countof(scan->tasks); i++)
    {
        if (scan->destroy)
            return -1;

        // Find next non-pending task
        if (scan->tasks[i].state == pal_scan_probe_idle)
        {
            pal_scan_next(&scan->tasks[i]);
            continue;
        }

        if (scan->tasks[i].sock_fd != -1 &&
            scan->tasks[i].probe_start + PROBE_TIMEOUT < now)
        {
            dbg_assert(scan->tasks[i].event_handle != 0, "no reg?");

            //
            // Probe timed out - unregister task which will get
            // destroy callback and which will schedule next.
            // Run on same kq/poll/epoll thread that will receive
            // Event callback, so threadsafe.
            //
            evt_handle = scan->tasks[i].event_handle;
            scan->tasks[i].event_handle = 0;
            scan->tasks[i].sock_fd = -1;
            if (evt_handle)
                pal_event_close(evt_handle, true);
            continue;
        }

        tmp = now - scan->tasks[i].probe_start + PROBE_TIMEOUT;
        if ((int32_t)tmp < reschedule)
            reschedule = (int32_t)tmp;
    }

    dbg_assert(reschedule > 0, "Should have some time between.");
    return reschedule;
}

//
// Close netlink registration
//
static bool pal_scan_netlink_close(
    pal_scan_t* scan
)
{
    uintptr_t evt_handle;
    dbg_assert_ptr(scan);

    evt_handle = scan->netlink_events;
    scan->netlink_events = 0;

    if (evt_handle)
    {
        scan->netlink_fd = -1;
        pal_event_close(evt_handle, true);
        return false;
    }

    if (scan->netlink_fd != -1)
    {
        close(scan->netlink_fd);
        scan->netlink_fd = -1;
    }
    return true;
}

//
// Handle new address
//
static void pal_scan_netlink_on_addr(
    pal_scan_t* scan,
    struct nlmsghdr *nlh
)
{
    struct ifaddrmsg *ifa_msg = (struct ifaddrmsg*)NLMSG_DATA(nlh);
    int len = RTM_PAYLOAD(nlh);
    void* addr = NULL;
    struct sockaddr_in6 sa;

    dbg_assert_ptr(scan);
    dbg_assert_ptr(nlh);
    dbg_assert(nlh->nlmsg_type != RTM_GETADDR, "Should not be get");

    // Parse attributes
    for (struct rtattr* rta = RTM_RTA(ifa_msg); RTA_OK(rta, len);
        rta = RTA_NEXT(rta, len))
    {
        /**/ if (rta->rta_type == IFA_ADDRESS)
            addr = RTA_DATA(ifa_msg);
        else if (rta->rta_type > IFA_MAX)
        {
            log_error(scan->log, "Unexpected ifa type parsed %d",
                rta->rta_type);
        }
        else
        {
            log_debug(scan->log, "Parsed %d", rta->rta_type);
        }
    }

    memset(&sa, 0, sizeof(sa));
    __sa_base(&sa)->sa_family = ifa_msg->ifa_family;

    /**/ if (__sa_is_in6(&sa))
    {
        memcpy(&(__sa_as_in6(&sa)->sin6_addr), addr, 16);
        log_trace(scan->log, "%s address: " __sa_in6_fmt,
            nlh->nlmsg_type == RTM_NEWADDR ? "New" : "Lost",
            __sa_in6_args(&sa));
    }
    else if (__sa_is_in4(&sa))
    {
        __sa_as_in4(&sa)->sin_addr.s_addr = *((int32_t*)addr);
        log_debug(scan->log, "%s address: " __sa_in4_fmt,
            nlh->nlmsg_type == RTM_NEWADDR ? "New" : "Lost",
            __sa_in4_args(&sa));
    }
    else
    {
        log_error(scan->log, "Unknown address family in ifa message %d",
            ifa_msg->ifa_family);
        return;
    }

    if (nlh->nlmsg_type == RTM_NEWADDR)
        pal_scan_notify(scan, ifa_msg->ifa_index, __sa_base(&sa));
}

//
// Handle new neighbor address
//
static void pal_scan_netlink_on_neigh(
    pal_scan_t* scan,
    struct nlmsghdr* nlh
)
{
    struct ndmsg *ndm = (struct ndmsg*)NLMSG_DATA(nlh);
    int len = RTM_PAYLOAD(nlh);
    void* addr = NULL;
    struct sockaddr_in6 sa;

    dbg_assert_ptr(scan);
    dbg_assert_ptr(nlh);
    log_debug(scan->log, "neighbor message state %x", ndm->ndm_state);
    dbg_assert(nlh->nlmsg_type != RTM_GETNEIGH, "Should not be get");

    if (nlh->nlmsg_type == RTM_NEWNEIGH && 0 == (ndm->ndm_state & NUD_REACHABLE))
        return;

    // Parse attributes
    for (struct rtattr* rta = RTM_RTA(ndm); RTA_OK(rta, len);
        rta = RTA_NEXT(rta, len))
    {
        /**/ if (rta->rta_type == NDA_DST)
            addr = RTA_DATA(rta);
        else if (rta->rta_type > IFA_MAX)
        {
            log_error(scan->log, "Unexpected ndm type parsed %d",
                rta->rta_type);
        }
        else
        {
            log_debug(scan->log, "Parsed %d", rta->rta_type);
        }
    }
    if (len > 0)
        log_error(scan->log, "%d unparsed in RTA", len);

    memset(&sa, 0, sizeof(sa));
    __sa_base(&sa)->sa_family = ndm->ndm_family;

    /**/ if (__sa_is_in6(&sa))
    {
        memcpy(&(__sa_as_in6(&sa)->sin6_addr), addr, 16);
        log_trace(scan->log, "%s neighbor: " __sa_in6_fmt " (state: %x)",
            nlh->nlmsg_type == RTM_NEWNEIGH ? "New" : "Lost",
            __sa_in6_args(&sa), ndm->ndm_state);
    }
    else if (__sa_is_in4(&sa))
    {
        __sa_as_in4(&sa)->sin_addr.s_addr = *((int32_t*)addr);
        log_debug(scan->log, "%s neighbor: " __sa_in4_fmt " (state: %x)",
            nlh->nlmsg_type == RTM_NEWNEIGH ? "New" : "Lost",
            __sa_in4_args(&sa), ndm->ndm_state);
    }
    else
    {
        log_error(scan->log, "Unknown address family in neigh message %d",
            ndm->ndm_family);
        return;
    }

    if (nlh->nlmsg_type == RTM_NEWNEIGH)
        pal_scan_notify(scan, ndm->ndm_ifindex, __sa_base(&sa));
}

//
// Receive neighbor messages
//
static int32_t pal_scan_netlink_recv(
    pal_scan_t* scan
)
{
    ssize_t buf_size;
    char buf[4096];
    struct nlmsghdr *nlh = (struct nlmsghdr*)buf;

    while (!scan->destroy)
    {
        buf_size = recv(scan->netlink_fd, buf, sizeof(buf), 0);
        if (buf_size <= 0)
            return buf_size == 0 ? er_closed : pal_os_last_net_error_as_prx_error();
        for (; NLMSG_OK(nlh, buf_size) && !scan->destroy;
            nlh = NLMSG_NEXT(nlh, buf_size))
        {
            switch (nlh->nlmsg_type)
            {
            case RTM_NEWADDR:
            case RTM_DELADDR:
                pal_scan_netlink_on_addr(scan, nlh);
                break;
            case RTM_GETADDR:
                break;
            case RTM_NEWNEIGH:
            case RTM_DELNEIGH:
                pal_scan_netlink_on_neigh(scan, nlh);
                break;
            case RTM_GETNEIGH:
                break;
            case NLMSG_DONE:
                log_info(scan->log, "End of multi-part message.");
                break;
            case NLMSG_ERROR:
                log_error(scan->log, "Netlink error message.");
                return er_comm;
            default:
                log_debug(scan->log, "Unknown message %d.", nlh->nlmsg_type);
                break;
            }
        }
    }
    return er_aborted;
}

//
// Called by event port when event occurred on netlink socket
//
static int32_t pal_scan_netlink_event(
    void* context,
    pal_event_type_t event_type,
    int32_t error_code
)
{
    pal_scan_t* scan = (pal_scan_t*)context;
    dbg_assert_ptr(scan);
    switch (event_type)
    {
    case pal_event_type_read:
        if (error_code == er_ok)
            return pal_scan_netlink_recv(scan);
    case pal_event_type_write:
    case pal_event_type_error:
    case pal_event_type_close:
        return error_code != er_ok ? error_code : er_closed;
    case pal_event_type_destroy:
        if (!scan->destroy) // Should never get here for now.
            pal_scan_netlink_close(scan);
        return er_ok;
    case pal_event_type_unknown:
    default:
        dbg_assert(0, "Unknown event type %d", event_type);
        return er_bad_state;
    }
}

//
// Open netlink socket and register for netlink multicast groups
//
static int32_t pal_scan_netlink_listen(
    pal_scan_t* scan,
    int32_t nl_groups
)
{
    struct sockaddr_nl sa;
    int32_t result;
    int val;

    dbg_assert_ptr(scan);
    dbg_assert(nl_groups, "Bad arg");

    // nl_groups |= RTMGRP_LINK;
    // nl_groups |= RTMGRP_IPV4_IFADDR;
    // nl_groups |= RTMGRP_IPV6_IFADDR;
    // nl_groups |= RTMGRP_IPV6_IFINFO;

    scan->netlink_fd = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);
    if (scan->netlink_fd == -1)
        return pal_os_last_net_error_as_prx_error();
    do
    {
        // Subscribe to neighbor updates
        memset(&sa, 0, sizeof(sa));
        sa.nl_family = AF_NETLINK;
        sa.nl_groups = nl_groups;

        if (0 != bind(scan->netlink_fd, __sa_base(&sa), sizeof(sa)))
        {
            result = pal_os_last_net_error_as_prx_error();
            break;
        }

        val = 0x10000;
        if (0 != setsockopt(scan->netlink_fd, SOL_SOCKET,
            SO_RCVBUF, (const sockbuf_t*)&val, sizeof(val)))
        {
            result = pal_os_last_net_error_as_prx_error();
            break;
        }

        val = 1;
#ifndef SOL_NETLINK
#define SOL_NETLINK 270
#endif
        if (0 != setsockopt(scan->netlink_fd, SOL_NETLINK,
            NETLINK_NO_ENOBUFS, (const sockbuf_t*)&val, sizeof(val)))
        {
            result = pal_os_last_net_error_as_prx_error();
            break;
        }

        result = pal_event_port_register(scan->event_port, scan->netlink_fd,
            pal_scan_netlink_event, scan, &scan->netlink_events);
        if (result != er_ok)
            break;

        result = pal_event_select(scan->netlink_events, pal_event_type_read);
        if (result != er_ok)
            break;

        return er_ok;
    }
    while (0);
    return result;
}

//
// Create and initialize scan context
//
static int32_t pal_scan_create(
    int32_t flags,
    pal_scan_cb_t cb,
    void* context,
    pal_scan_t** created
)
{
    pal_scan_t* scan;

    chk_arg_fault_return(cb);

    scan = (pal_scan_t*)mem_zalloc_type(pal_scan_t);
    if (!scan)
        return er_out_of_memory;

    // Initialize scan
    scan->log = log_get("pal.scan");
    scan->flags = flags;
    scan->cb = cb;
    scan->context = context;
    scan->netlink_fd = -1;

    for (size_t i = 0; i < _countof(scan->tasks); i++)
    {
        scan->tasks[i].state = pal_scan_probe_idle;
        scan->tasks[i].sock_fd = -1;
        scan->tasks[i].scan = scan;
    }

    *created = scan;
    return er_ok;
}

//
// Scan for addresses with optionally open port in subnet
//
int32_t pal_scan_net(
    uint16_t port,
    int32_t flags,
    pal_scan_cb_t cb,
    void* context,
    pal_scan_t** created
)
{
    int32_t result;
    int error;
    pal_scan_t* scan;

    chk_arg_fault_return(cb);

    result = pal_scan_create(flags, cb, context, &scan);
    if (result != er_ok)
        return result;
    do
    {
        scan->port_scan_end = scan->port = scan->port_scan_cur = port;

        error = getifaddrs(&scan->ifaddr);
        if (error != 0)
        {
            result = pal_os_to_prx_error(error);
            log_error(scan->log, "Failed to get adapter infos (%x, %s).",
                error, prx_err_string(result));
            break;
        }

        scan->ifcur = scan->ifaddr;
        result = pal_event_port_create(pal_scan_scheduler, scan,
            &scan->event_port);
        if (result != er_ok)
        {
            log_error(scan->log, "Failed creating event port (%s).",
                prx_err_string(result));
            break;
        }

#if defined(IP_NEIGHBOR)
        result = pal_scan_netlink_listen(scan, RTMGRP_NEIGH);
        if (result != er_ok)
        {
            log_error(scan->log, "Failed to register for neighbors (%s).",
                prx_err_string(result));
        }
#endif
        *created = scan;
        return er_ok;
    }
    while (0);

    pal_scan_close(scan);
    return result;
}

//
// Scan for ports on address
//
int32_t pal_scan_ports(
    const prx_socket_address_t* addr,
    uint16_t port_range_low,
    uint16_t port_range_high,
    int32_t flags,
    pal_scan_cb_t cb,
    void* context,
    pal_scan_t** created
)
{
    int32_t result;
    pal_scan_t* scan;
    socklen_t sa_len;

    chk_arg_fault_return(addr);
    chk_arg_fault_return(cb);

    if (!port_range_low)
        port_range_low = 1;
    if (!port_range_high)
        port_range_high = (uint16_t)-1;
    if (port_range_high <= port_range_low)
        return er_arg;

    result = pal_scan_create(flags, cb, context, &scan);
    if (result != er_ok)
        return result;
    do
    {
        scan->port_scan_cur = port_range_low;
        scan->port_scan_end = port_range_high;

        scan->port = scan->port_scan_cur;
        scan->port_scan_end++;

        sa_len = sizeof(struct sockaddr_in6);
        result = pal_os_from_prx_socket_address(addr,
            (struct sockaddr*)&scan->address, &sa_len);
        if (result != er_ok)
            break;

        // Start scan
        result = pal_event_port_create(pal_scan_scheduler, scan,
            &scan->event_port);
        if (result != er_ok)
        {
            log_error(NULL, "FATAL: Failed creating event port.");
            break;
        }

        *created = scan;
        return er_ok;
    } while (0);

    pal_scan_close(scan);
    return result;
}

//
// Abort and close in progress scan
//
void pal_scan_close(
    pal_scan_t* scan
)
{
    int sock_fd;
    uintptr_t evt_handle;

    if (!scan)
        return;

    // Set flag that scan is closing
    scan->destroy = true;

    // Stop scanning (thread) - no more calls to functions above.
    if (scan->event_port)
        pal_event_port_stop(scan->event_port);

    (void)pal_scan_netlink_close(scan);

    // Close all events now.
    for (size_t i = 0; i < _countof(scan->tasks); i++)
    {
        evt_handle = scan->tasks[i].event_handle;
        sock_fd = scan->tasks[i].sock_fd;

        scan->tasks[i].event_handle = 0;
        scan->tasks[i].sock_fd = -1;

        if (evt_handle)
            pal_event_close(evt_handle, true);
        else if (sock_fd != -1)
            close(sock_fd);
    }

    // Close and free the event port
    if (scan->event_port)
        pal_event_port_close(scan->event_port);

    if (scan->ifaddr)
        freeifaddrs(scan->ifaddr);

    log_trace(scan->log, "Scan %p closed.", scan);
    mem_free_type(pal_scan_t, scan);
}

//
// Called before using scan layer
//
int32_t pal_scan_init(
    void
)
{
    return er_ok;
}

//
// Free scan layer
//
void pal_scan_deinit(
    void
)
{
    // No ope
}
