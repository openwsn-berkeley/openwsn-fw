#include "openwsn.h"
#include "udpecho.h"
#include "openudp.h"
#include "openqueue.h"
#include "openserial.h"
#include "packetfunctions.h"
//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void udpecho_init() {
}

void udpecho_receive(OpenQueueEntry_t* msg) {
   uint16_t temp_l4_destination_port;
   
   OpenQueueEntry_t * pkt = openqueue_getFreePacketBuffer(COMPONENT_UDPECHO);
   if (pkt==NULL) {
      openserial_printError(COMPONENT_UDPLATENCY,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      return;
   }
   
   pkt->owner   = COMPONENT_UDPECHO;
   //reply with the same OpenQueueEntry_t
   pkt->creator                       = COMPONENT_UDPECHO;
   pkt->l4_protocol                   = IANA_UDP;
   temp_l4_destination_port           = msg->l4_destination_port;
   pkt->l4_destination_port           = msg->l4_sourcePortORicmpv6Type;
   pkt->l4_sourcePortORicmpv6Type     = temp_l4_destination_port;
   pkt->l3_destinationAdd.type = ADDR_128B;
   //copy source to destination to echo.
   memcpy(&pkt->l3_destinationAdd.addr_128b[0],&msg->l3_sourceAdd.addr_128b[0],16);
   
   packetfunctions_reserveHeaderSize(pkt,msg->length);
   memcpy(&pkt->payload[0],&msg->payload[0],msg->length);
   openqueue_freePacketBuffer(msg);
   
   if ((openudp_send(pkt))==E_FAIL) {
      openqueue_freePacketBuffer(pkt);
   }
}

void udpecho_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
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