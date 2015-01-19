#include "opendefs.h"
#include "uecho.h"
#include "openudp.h"
#include "openqueue.h"
#include "openserial.h"
#include "packetfunctions.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void uecho_init() {
}

void uecho_receive(OpenQueueEntry_t* request) {
   uint16_t          temp_l4_destination_port;
   OpenQueueEntry_t* reply;
   
   reply = openqueue_getFreePacketBuffer(COMPONENT_UECHO);
   if (reply==NULL) {
      openserial_printError(
         COMPONENT_UECHO,
         ERR_NO_FREE_PACKET_BUFFER,
         (errorparameter_t)0,
         (errorparameter_t)0
      );
      openqueue_freePacketBuffer(request); //clear the request packet as well
      return;
   }
   
   reply->owner                         = COMPONENT_UECHO;
   
   // reply with the same OpenQueueEntry_t
   reply->creator                       = COMPONENT_UECHO;
   reply->l4_protocol                   = IANA_UDP;
   temp_l4_destination_port           = request->l4_destination_port;
   reply->l4_destination_port           = request->l4_sourcePortORicmpv6Type;
   reply->l4_sourcePortORicmpv6Type     = temp_l4_destination_port;
   reply->l3_destinationAdd.type        = ADDR_128B;
   
   // copy source to destination to echo.
   memcpy(&reply->l3_destinationAdd.addr_128b[0],&request->l3_sourceAdd.addr_128b[0],16);
   
   packetfunctions_reserveHeaderSize(reply,request->length);
   memcpy(&reply->payload[0],&request->payload[0],request->length);
   openqueue_freePacketBuffer(request);
   
   if ((openudp_send(reply))==E_FAIL) {
      openqueue_freePacketBuffer(reply);
   }
}

void uecho_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}

bool uecho_debugPrint() {
   return FALSE;
}

//=========================== private =========================================
