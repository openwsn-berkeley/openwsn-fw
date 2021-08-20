#include "opendefs.h"
#include "usendpacket.h"
#include "openqueue.h"
#include "openserial.h"
#include "packetfunctions.h"
#include "scheduler.h"
#include "IEEE802154E.h"
#include "schedule.h"
#include "icmpv6rpl.h"
#include "idmanager.h"

//=========================== variables =======================================

usendpacket_vars_t usendpacket_vars;

static  uint8_t usendpacket_payload_True[]    = "0Olicke";
static  uint8_t usendpacket_payload_False[]   = "0Xlicke";
static const uint8_t usendapcket_dst_addr[]   = {
   0xbb, 0xbb, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
};
uint8_t packetCount=1;
//=========================== prototypes ======================================

void usendpacket_timer_cb(opentimers_id_t id);
void usendpacket_task_cb(bool answer);

//=========================== public ==========================================

void usendpacket_init(void) {

    // clear local variables
    memset(&usendpacket_vars,0,sizeof(usendpacket_vars_t));

    // register at UDP stack
    usendpacket_vars.desc.port              = WKP_UDP_INJECT;
    usendpacket_vars.desc.callbackReceive   = &usendpacket_receive;
    usendpacket_vars.desc.callbackSendDone  = &usendpacket_sendDone;
    openudp_register(&usendpacket_vars.desc);

    usendpacket_vars.period = USENDPACKET_PERIOD_MS;
    // start periodic timer
    usendpacket_vars.timerId = opentimers_create(TIMER_GENERAL_PURPOSE, TASKPRIO_UDP);
    /*
    opentimers_scheduleIn(
        usendpacket_vars.timerId,
        USENDPACKET_PERIOD_MS,
        TIME_MS,
        TIMER_PERIODIC,
        usendpacket_timer_cb
    );
    */
}

void usendpacket_sendDone(OpenQueueEntry_t* msg, owerror_t error) {

    // free the packet buffer entry
    openqueue_freePacketBuffer(msg);

    // allow send next uinject packet
    usendpacket_vars.busySendingUsendpacket = FALSE;
}

void usendpacket_receive(OpenQueueEntry_t* pkt) {

    openqueue_freePacketBuffer(pkt);

    openserial_printError(
        COMPONENT_UINJECT,
        ERR_RCVD_ECHO_REPLY,
        (errorparameter_t)0,
        (errorparameter_t)0
    );
}

//=========================== private =========================================
/*
void usendpacket_timer_cb(opentimers_id_t id){
    // calling the task directly as the timer_cb function is executed in
    // task mode by opentimer already
    usendpacket_task_cb();
}
*/

void usendpacket_task_cb(bool answer) {
    
    //if(!trueClicked && !falseClicked) { return; }

    OpenQueueEntry_t*    pkt;
    uint8_t              asnArray[5];
    open_addr_t          parentNeighbor;
    bool                 foundNeighbor;

    // don't run if not synch
    if (ieee154e_isSynch() == FALSE) {
        return;
    }

    // don't run on dagroot
    if (idmanager_getIsDAGroot()) {
        opentimers_destroy(usendpacket_vars.timerId);
        return;
    }

    foundNeighbor = icmpv6rpl_getPreferredParentEui64(&parentNeighbor);
    if (foundNeighbor==FALSE) {
        return;
    }

    if (schedule_hasManagedTxCellToNeighbor(&parentNeighbor) == FALSE) {
        return;
    }

    if (usendpacket_vars.busySendingUsendpacket==TRUE) {
        // don't continue if I'm still sending a previous uinject packet
        return;
    }

    // if you get here, send a packet
    openserial_printInfo(COMPONENT_UINJECT, 255, answer, packetCount);
    
    // get a free packet buffer
    pkt = openqueue_getFreePacketBuffer(COMPONENT_UINJECT);
    if (pkt==NULL) {
        openserial_printError(
            COMPONENT_UINJECT,
            ERR_NO_FREE_PACKET_BUFFER,
            (errorparameter_t)0,
            (errorparameter_t)0
        );
        return;
    }

    pkt->owner                         = COMPONENT_UINJECT;
    pkt->creator                       = COMPONENT_UINJECT;
    pkt->l4_protocol                   = IANA_UDP;
    pkt->l4_destination_port           = WKP_UDP_INJECT;
    pkt->l4_sourcePortORicmpv6Type     = WKP_UDP_INJECT;
    pkt->l3_destinationAdd.type        = ADDR_128B;
    memcpy(&pkt->l3_destinationAdd.addr_128b[0],usendapcket_dst_addr,16);

    // add payload
    if (answer) {
        usendpacket_payload_True[0] = packetCount++;
        packetfunctions_reserveHeaderSize(pkt,sizeof(usendpacket_payload_True)-1);
        memcpy(&pkt->payload[0],usendpacket_payload_True,sizeof(usendpacket_payload_True)-1);
    }
    else {
        usendpacket_payload_False[0] = packetCount++;
        packetfunctions_reserveHeaderSize(pkt,sizeof(usendpacket_payload_False)-1);
        memcpy(&pkt->payload[0],usendpacket_payload_False,sizeof(usendpacket_payload_False)-1);

    }
    packetfunctions_reserveHeaderSize(pkt,sizeof(uint16_t));
    pkt->payload[1] = (uint8_t)((usendpacket_vars.counter & 0xff00)>>8);
    pkt->payload[0] = (uint8_t)(usendpacket_vars.counter & 0x00ff);
    usendpacket_vars.counter++;

    packetfunctions_reserveHeaderSize(pkt,sizeof(asn_t));
    ieee154e_getAsn(asnArray);
    pkt->payload[0] = asnArray[0];
    pkt->payload[1] = asnArray[1];
    pkt->payload[2] = asnArray[2];
    pkt->payload[3] = asnArray[3];
    pkt->payload[4] = asnArray[4];

    if ((openudp_send(pkt))==E_FAIL) {
        openqueue_freePacketBuffer(pkt);
    } else {
        // set busySending to TRUE
        usendpacket_vars.busySendingUsendpacket = TRUE;
    }
 //   trueClicked = FALSE;
//    falseClicked = FALSE;
}


