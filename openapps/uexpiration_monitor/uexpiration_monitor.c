#include "config.h"

#if OPENWSN_UEXP_MONITOR_C

#include "opendefs.h"
#include "uexpiration_monitor.h"
#include "openqueue.h"
#include "openserial.h"
#include "packetfunctions.h"

#ifdef DEADLINE_OPTION_ENABLED
#include "iphc.h"
#endif

//=========================== variables =======================================
umonitor_vars_t umonitor_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void umonitor_init(void) {
    // clear local variables
    memset(&umonitor_vars, 0, sizeof(umonitor_vars_t));

    // register at UDP stack
    umonitor_vars.desc.port = WKP_UDP_MONITOR;
    umonitor_vars.desc.callbackReceive = &umonitor_receive;
    umonitor_vars.desc.callbackSendDone = &umonitor_sendDone;
    openudp_register(&umonitor_vars.desc);
}

void umonitor_receive(OpenQueueEntry_t *request) {
    uint16_t temp_l4_destination_port;
    OpenQueueEntry_t *reply;
#ifdef DEADLINE_OPTION_ENABLED
    monitor_expiration_vars_t	deadline;
#endif

    reply = openqueue_getFreePacketBuffer(COMPONENT_UMONITOR);
    if (reply == NULL) {
        LOG_ERROR(COMPONENT_UMONITOR, ERR_NO_FREE_PACKET_BUFFER, (errorparameter_t) 0, (errorparameter_t) 0);
        openqueue_freePacketBuffer(request); //clear the request packet as well
        return;
    }

    reply->owner = COMPONENT_UMONITOR;
    reply->creator = COMPONENT_UMONITOR;
    reply->l4_protocol = IANA_UDP;
    temp_l4_destination_port = request->l4_destination_port;
    reply->l4_destination_port = request->l4_sourcePortORicmpv6Type;
    reply->l4_sourcePortORicmpv6Type = temp_l4_destination_port;
    reply->l3_destinationAdd.type = ADDR_128B;
    memcpy(&reply->l3_destinationAdd.addr_128b[0], &request->l3_sourceAdd.addr_128b[0], 16);

    /*************** Packet Payload  ********************/
    // [Expiration time, Delay]
    if (packetfunctions_reserveHeader(&reply, (2 * sizeof(uint16_t))) == E_FAIL) {
        openqueue_freePacketBuffer(reply);
        return;
    }
#ifdef DEADLINE_OPTION_ENABLED
    memset(&deadline, 0, sizeof(monitor_expiration_vars_t));
    iphc_getDeadlineInfo(&deadline);
    memcpy(&reply->payload[0],&deadline.time_elapsed,sizeof(uint16_t));
    memcpy(&reply->payload[2],&deadline.time_left,sizeof(uint16_t));
#endif
    openqueue_freePacketBuffer(request);

    if ((openudp_send(reply)) == E_FAIL) {
        openqueue_freePacketBuffer(reply);
    }
}

void umonitor_sendDone(OpenQueueEntry_t *msg, owerror_t error) {
    openqueue_freePacketBuffer(msg);
}

bool umonitor_debugPrint(void) {
    return FALSE;
}

//=========================== private =========================================

#endif /* OPEWSN_UEXP_MONITOR_C */
