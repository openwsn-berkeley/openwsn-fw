#include "openwsn.h"
#include "udpecho.h"
#include "openudp.h"
#include "openqueue.h"
#include "openserial.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void udpecho_init() {
}

void udpecho_receive(OpenQueueEntry_t* msg) {
   uint16_t temp_l4_destination_port;
   msg->owner   = COMPONENT_UDPECHO;
   //reply with the same OpenQueueEntry_t
   msg->creator                       = COMPONENT_UDPECHO;
   msg->l4_protocol                   = IANA_UDP;
   temp_l4_destination_port           = msg->l4_destination_port;
   msg->l4_destination_port           = msg->l4_sourcePortORicmpv6Type;
   msg->l4_sourcePortORicmpv6Type     = temp_l4_destination_port;
   msg->l3_destinationORsource.type = ADDR_128B;
   memcpy(&msg->l3_destinationORsource.addr_128b[0],&msg->l3_sourceAdd.addr_128b[0],16);
   
   if ((openudp_send(msg))==E_FAIL) {
      openqueue_freePacketBuffer(msg);
   }
}

void udpecho_sendDone(OpenQueueEntry_t* msg, error_t error) {
   msg->owner = COMPONENT_UDPECHO;
   if (msg->creator!=COMPONENT_UDPECHO) {
      openserial_printError(COMPONENT_UDPECHO,ERR_UNEXPECTED_SENDDONE,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
   }
   openqueue_freePacketBuffer(msg);
}

bool udpecho_debugPrint() {
   return FALSE;
}

//=========================== private =========================================