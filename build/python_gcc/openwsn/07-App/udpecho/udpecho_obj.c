/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:11:12.639297.
*/
#include "openwsn_obj.h"
#include "udpecho_obj.h"
#include "openudp_obj.h"
#include "openqueue_obj.h"
#include "openserial_obj.h"
#include "packetfunctions_obj.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void udpecho_init(OpenMote* self) {
}

void udpecho_receive(OpenMote* self, OpenQueueEntry_t* request) {
   uint16_t          temp_l4_destination_port;
   OpenQueueEntry_t* reply;
   
   reply = openqueue_getFreePacketBuffer(self, COMPONENT_UDPECHO);
   if (reply==NULL) {
 openserial_printError(self, 
         COMPONENT_UDPLATENCY,
         ERR_NO_FREE_PACKET_BUFFER,
         (errorparameter_t)0,
         (errorparameter_t)0
      );
      return;
   }
   
   reply->owner                         = COMPONENT_UDPECHO;
   
   // reply with the same OpenQueueEntry_t
   reply->creator                       = COMPONENT_UDPECHO;
   reply->l4_protocol                   = IANA_UDP;
   temp_l4_destination_port           = request->l4_destination_port;
   reply->l4_destination_port           = request->l4_sourcePortORicmpv6Type;
   reply->l4_sourcePortORicmpv6Type     = temp_l4_destination_port;
   reply->l3_destinationAdd.type        = ADDR_128B;
   
   // copy source to destination to echo.
   memcpy(&reply->l3_destinationAdd.addr_128b[0],&request->l3_sourceAdd.addr_128b[0],16);
   
 packetfunctions_reserveHeaderSize(self, reply,request->length);
   memcpy(&reply->payload[0],&request->payload[0],request->length);
 openqueue_freePacketBuffer(self, request);
   
   if (( openudp_send(self, reply))==E_FAIL) {
 openqueue_freePacketBuffer(self, reply);
   }
}

void udpecho_sendDone(OpenMote* self, OpenQueueEntry_t* msg, owerror_t error) {
 openqueue_freePacketBuffer(self, msg);
}

bool udpecho_debugPrint(OpenMote* self) {
   return FALSE;
}

//=========================== private =========================================