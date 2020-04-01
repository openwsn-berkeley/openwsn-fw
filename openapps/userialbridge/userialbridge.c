#include "config.h"

#if defined(OPENWSN_USERIALBRIDGE_C)

#include "opendefs.h"
#include "userialbridge.h"
#include "openqueue.h"
#include "opentimers.h"
#include "openserial.h"
#include "packetfunctions.h"
#include "scheduler.h"
#include "IEEE802154E.h"
#include "idmanager.h"

//=========================== variables =======================================

userialbridge_vars_t userialbridge_vars;

static const uint8_t userialbridge_dst_addr[]   = {
    0xbb, 0xbb, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
}; 

//=========================== prototypes ======================================

void userialbridge_task_cb(void);

//=========================== public ==========================================

void userialbridge_init(void) {
    
    // clear local variables
    memset(&userialbridge_vars,0,sizeof(userialbridge_vars_t));
    
    // register at UDP stack
    userialbridge_vars.desc.port              = WKP_UDP_SERIALBRIDGE;
    userialbridge_vars.desc.callbackReceive   = NULL;
    userialbridge_vars.desc.callbackSendDone  = &userialbridge_sendDone;
    openudp_register(&userialbridge_vars.desc);
}

void userialbridge_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
    openqueue_freePacketBuffer(msg);
}

//=========================== private =========================================

void userialbridge_triggerData(void) {
    
    // store payload to send
    userialbridge_vars.txbufLen = openserial_getInputBuffer(
        &userialbridge_vars.txbuf[0],
        USERIALBRIDGE_MAXPAYLEN
    );
    
    // push task
    scheduler_push_task(userialbridge_task_cb,TASKPRIO_COAP);
}

void userialbridge_task_cb(void) {
    OpenQueueEntry_t*    pkt;
    
    // don't run if not synch
    if (ieee154e_isSynch() == FALSE) return;
    
    // if you get here, send a packet
    
    // get a free packet buffer
    pkt = openqueue_getFreePacketBuffer(COMPONENT_USERIALBRIDGE);
    if (pkt==NULL) {
        openserial_printError(
            COMPONENT_USERIALBRIDGE,
            ERR_NO_FREE_PACKET_BUFFER,
            (errorparameter_t)0,
            (errorparameter_t)0
        );
        return;
    }
    
    pkt->owner                         = COMPONENT_USERIALBRIDGE;
    pkt->creator                       = COMPONENT_USERIALBRIDGE;
    pkt->l4_protocol                   = IANA_UDP;
    pkt->l4_destination_port           = WKP_UDP_SERIALBRIDGE;
    pkt->l4_sourcePortORicmpv6Type     = WKP_UDP_SERIALBRIDGE;
    pkt->l3_destinationAdd.type        = ADDR_128B;
    memcpy(&pkt->l3_destinationAdd.addr_128b[0],userialbridge_dst_addr,16);
    
    packetfunctions_reserveHeaderSize(pkt,userialbridge_vars.txbufLen);
    memcpy(&pkt->payload[0],&userialbridge_vars.txbuf[0],userialbridge_vars.txbufLen);
    
    if ((openudp_send(pkt))==E_FAIL) {
        openqueue_freePacketBuffer(pkt);
    }
}

#endif /* OPENWSN_USERIALBRIDGE_C */
