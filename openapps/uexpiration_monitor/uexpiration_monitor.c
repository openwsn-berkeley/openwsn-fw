#include "config.h"

#if OPENWSN_UEXP_MONITOR_C

#include "opendefs.h"
#include "uexpiration_monitor.h"
#include "sock.h"
#include "async.h"
#include "openserial.h"
#include "packetfunctions.h"

#ifdef DEADLINE_OPTION
#include "iphc.h"
#endif

//=========================== variables =======================================

static sock_udp_t _sock;

//=========================== prototypes ======================================

//=========================== private =========================================

void _sock_handler(sock_udp_t *sock, sock_async_flags_t type, void *arg) {
    (void) arg;

    uint8_t buf[50];
    if (type & SOCK_ASYNC_MSG_RECV) {
        sock_udp_ep_t remote;
            size_t len = 0;
        if (sock_udp_recv(sock, buf, sizeof(buf), 0, &remote) >= 0) {
#ifdef DEADLINE_OPTION
            monitor_expiration_vars_t deadline = { 0 };
            iphc_getDeadlineInfo(&deadline);
            memcpy(&buf[0], &deadline.time_elapsed, sizeof(uint16_t));
            memcpy(&buf[2], &deadline.time_left, sizeof(int16_t));
            len += sizeof(uint16_t);
            len += sizeof(int16_t);
#endif
            if (sock_udp_send(sock, (char*) buf, len, &remote) < 0) {
                LOG_ERROR(COMPONENT_UMONITOR, ERR_PUSH_LOWER_LAYER,
                          (errorparameter_t) 0,
                          (errorparameter_t) 0);
            }
        }
    }
}

//=========================== public ==========================================

void umonitor_init(void)
{
    // clear local variables
    memset(&_sock, 0, sizeof(sock_udp_t));

    sock_udp_ep_t local;
    local.family = AF_INET6;
    local.port = WKP_UDP_MONITOR;

    if (sock_udp_create(&_sock, &local, NULL, 0) < 0) {
        LOG_ERROR(COMPONENT_UMONITOR, ERR_INVALID_PARAM,
                (errorparameter_t) 0,
                (errorparameter_t) 0);
        return;
    }

    sock_udp_set_cb(&_sock, _sock_handler, NULL);
}

#endif /* OPEWSN_UEXP_MONITOR_C */
