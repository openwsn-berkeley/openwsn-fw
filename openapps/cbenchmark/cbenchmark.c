/**
\brief App that generates packets as instructed over serial.
*/

#include "opendefs.h"
#include "opencoap.h"
#include "opentimers.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "openrandom.h"
#include "scheduler.h"
#include "idmanager.h"
#include "eui64.h"
#include "neighbors.h"
#include "cbenchmark.h"

//=========================== defines =========================================

const uint8_t cbenchmark_path0[] = "b";

//=========================== variables =======================================

cbenchmark_vars_t cbenchmark_vars;

//=========================== prototypes ======================================

owerror_t cbenchmark_receive(OpenQueueEntry_t* msg,
        coap_header_iht*  coap_header,
        coap_option_iht*  coap_incomingOptions,
        coap_option_iht*  coap_outgoingOptions,
        uint8_t*          coap_outgoingOptionsLen);
void cbenchmark_sendDone(OpenQueueEntry_t* msg, owerror_t error);
void cbenchmark_sendPacket(open_addr_t *, bool, uint8_t, uint8_t*, uint8_t, uint8_t);

//=========================== public ==========================================

void cbenchmark_init() {
   // prepare the resource descriptor for the /b path
   cbenchmark_vars.desc.path0len                        = sizeof(cbenchmark_path0)-1;
   cbenchmark_vars.desc.path0val                        = (uint8_t*)(&cbenchmark_path0);
   cbenchmark_vars.desc.path1len                        = 0;
   cbenchmark_vars.desc.path1val                        = NULL;
   cbenchmark_vars.desc.componentID                     = COMPONENT_CBENCHMARK;
   cbenchmark_vars.desc.securityContext                 = NULL;
   cbenchmark_vars.desc.discoverable                    = TRUE;
   cbenchmark_vars.desc.callbackRx                      = &cbenchmark_receive;
   cbenchmark_vars.desc.callbackSendDone                = &cbenchmark_sendDone;

   opencoap_register(&cbenchmark_vars.desc);

   // Register sendPacket handler callback
   openserial_registerSendPacketCb(&cbenchmark_sendPacket);

   cbenchmark_vars.timerId = opentimers_create(TIMER_GENERAL_PURPOSE, TASKPRIO_COAP);

}

//=========================== private =========================================
owerror_t cbenchmark_receive(OpenQueueEntry_t* msg,
        coap_header_iht*  coap_header,
        coap_option_iht*  coap_incomingOptions,
        coap_option_iht*  coap_outgoingOptions,
        uint8_t*          coap_outgoingOptionsLen) {

    return E_SUCCESS;
}

void cbenchmark_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
    openqueue_freePacketBuffer(msg);
}

void cbenchmark_sendPacket(open_addr_t *dest, bool con, uint8_t numPackets, uint8_t* token, uint8_t tokenLen, uint8_t payloadLen) {

}

