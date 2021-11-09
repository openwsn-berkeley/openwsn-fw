#include "config.h"

#if OPENWSN_UINJECT_C

#include "opendefs.h"
#include "uinject.h"
#include "sock.h"
#include "async.h"
#include "openserial.h"
#include "packetfunctions.h"
#include "scheduler.h"
#include "IEEE802154E.h"
#include "schedule.h"
#include "icmpv6rpl.h"
#include "idmanager.h"
#include "openrandom.h"

#include "msf.h"

//=========================== defines =========================================

#define UINJECT_TRAFFIC_RATE 2 ///> the value X indicates 1 packet/X minutes

//=========================== variables =======================================

static sock_udp_t _sock;
static uinject_vars_t uinject_vars;

static const uint8_t uinject_payload[] = "uinject";
static const uint8_t uinject_dst_addr[] = {
        0xbb, 0xbb, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
};

//=========================== prototypes ======================================

void uinject_sock_handler(sock_udp_t *sock, sock_async_flags_t type, void *arg);

void _uinject_timer_cb(opentimers_id_t id);

void _uinject_task_cb(void);

//=========================== public ==========================================

void uinject_init(void) {

    // clear local variables
    memset(&_sock, 0, sizeof(sock_udp_t));
    memset(&uinject_vars, 0, sizeof(uinject_vars_t));

    sock_udp_ep_t local;
    local.family = AF_INET6;
    local.port = WKP_UDP_INJECT;

    if (sock_udp_create(&_sock, &local, NULL, 0) < 0) {
        openserial_printf("Could not create socket\n");
        return;
    }

    openserial_printf("Created a UDP socket\n");

    sock_udp_set_cb(&_sock, uinject_sock_handler, NULL);

    // start periodic timer
    uinject_vars.period = UINJECT_PERIOD_MS;
    uinject_vars.timerId = opentimers_create(TIMER_GENERAL_PURPOSE, TASKPRIO_UDP);
    opentimers_scheduleIn(
            uinject_vars.timerId,
            UINJECT_PERIOD_MS,
            TIME_MS,
            TIMER_PERIODIC,
            _uinject_timer_cb
    );
}

//=========================== private =========================================



void uinject_sock_handler(sock_udp_t *sock, sock_async_flags_t type, void *arg) {
    (void) arg;

    char buf[50];

    if (type & SOCK_ASYNC_MSG_RECV) {
        sock_udp_ep_t remote;
        int16_t res;

        if ((res = sock_udp_recv(sock, buf, sizeof(buf), 0, &remote)) >= 0) {
            openserial_printf("Received %d bytes from remote endpoint:\n", res);
            openserial_printf(" - port: %d", remote.port);
            openserial_printf(" - addr: ", remote.port);
            for(int i=0; i < 16; i ++)
                openserial_printf("%x ", remote.addr.ipv6[i]);

            openserial_printf("\n\n");
            openserial_printf("Msg received: %s\n\n", buf);
        }
    }

    if (type & SOCK_ASYNC_MSG_SENT) {
        owerror_t error = *(uint8_t*)arg;
        if (error == E_FAIL) {
            LOG_ERROR(COMPONENT_UINJECT, ERR_MAXRETRIES_REACHED,
                    (errorparameter_t) uinject_vars.counter,
                    (errorparameter_t) 0);
        }
        // allow send next uinject packet
        uinject_vars.busySendingUinject = FALSE;
    }
}


void _uinject_timer_cb(opentimers_id_t id) {
    // calling the task directly as the timer_cb function is executed in
    // task mode by opentimer already
    if (openrandom_get16b() < (0xffff / UINJECT_TRAFFIC_RATE)) {
        _uinject_task_cb();
    }
}

void _uinject_task_cb(void) {
    uint8_t asnArray[5];
    open_addr_t parentNeighbor;
    bool foundNeighbor;

    // don't run if not synch
    if (ieee154e_isSynch() == FALSE) {
        return;
    }

    // don't run on dagroot
    if (idmanager_getIsDAGroot()) {
        opentimers_destroy(uinject_vars.timerId);
        return;
    }

    foundNeighbor = icmpv6rpl_getPreferredParentEui64(&parentNeighbor);
    if (foundNeighbor == FALSE) {
        return;
    }

    if (schedule_hasNegotiatedCellToNeighbor(&parentNeighbor, CELLTYPE_TX) == FALSE) {
        return;
    }

    if (uinject_vars.busySendingUinject == TRUE) {
        // don't continue if I'm still sending a previous uinject packet
        return;
    }

    // if you get here, send a packet
    sock_udp_ep_t remote;
    remote.port = WKP_UDP_INJECT;
    remote.family = AF_INET6;
    memcpy(remote.addr.ipv6, uinject_dst_addr, sizeof(uinject_dst_addr));

    uint8_t payload[50];
    uint8_t len = 0;
    // add 'uinject' string
    memcpy(&payload[len], uinject_payload, sizeof(uinject_payload) - 1);
    len += sizeof(uinject_payload) - 1;
    // add counter
    payload[len++] = (uint8_t)(uinject_vars.counter & 0x00ff);
    payload[len++] = (uint8_t)((uinject_vars.counter & 0xff00) >> 8);
    uinject_vars.counter++;
    // add asn
    ieee154e_getAsn(asnArray);
    memcpy(&payload[len], asnArray, sizeof(asnArray));
    len += sizeof(asnArray);
    // add tx cells used
    payload[len++] = msf_getPreviousNumCellsUsed(CELLTYPE_TX);
    // add rx cells used
    payload[len++] = msf_getPreviousNumCellsUsed(CELLTYPE_RX);
    // add 16b addr
    payload[len++] = (uint8_t)(idmanager_getMyID(ADDR_16B)->addr_16b[1]);
    payload[len++] = (uint8_t)(idmanager_getMyID(ADDR_16B)->addr_16b[0]);
    // add ticks info
    uint32_t ticksOn;
    uint32_t ticksInTotal;
    ieee154e_getTicsInfo(&ticksOn, &ticksInTotal);
    memcpy(&payload[len],  &ticksOn, sizeof(ticksOn));
    len += sizeof(ticksOn);
    memcpy(&payload[len],  &ticksInTotal, sizeof(ticksInTotal));
    len += sizeof(ticksInTotal);

    if (sock_udp_send(&_sock, payload, len, &remote) > 0) {
        // set busySending to TRUE
        uinject_vars.busySendingUinject = TRUE;
    }
}

#endif /* OPENWSN_UINJECT_C */
