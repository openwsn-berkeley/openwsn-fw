#include "openwsn.h"
#include "opencoap.h"
#include "openudp.h"
#include "openqueue.h"
#include "openserial.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void opencoap_init() {
}

void opencoap_receive(OpenQueueEntry_t* msg) {
   uint16_t temp_l4_destination_port;
   msg->owner   = COMPONENT_OPENCOAP;
   //reply with the same OpenQueueEntry_t
   msg->creator                       = COMPONENT_OPENCOAP;
   msg->l4_protocol                   = IANA_UDP;
   temp_l4_destination_port           = msg->l4_destination_port;
   msg->l4_destination_port           = msg->l4_sourcePortORicmpv6Type;
   msg->l4_sourcePortORicmpv6Type     = temp_l4_destination_port;
   if ((openudp_send(msg))==E_FAIL) {
      openqueue_freePacketBuffer(msg);
   }
}

void opencoap_sendDone(OpenQueueEntry_t* msg, error_t error) {
   msg->owner = COMPONENT_OPENCOAP;
   if (msg->creator!=COMPONENT_OPENCOAP) {
      openserial_printError(COMPONENT_OPENCOAP,ERR_UNEXPECTED_SENDDONE,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
   }
   openqueue_freePacketBuffer(msg);
}

//=========================== private =========================================