/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:11:13.620260.
*/
#include "openwsn_obj.h"
#include "udpinject_obj.h"
#include "openudp_obj.h"
#include "openqueue_obj.h"
#include "openserial_obj.h"
#include "packetfunctions_obj.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void udpinject_init(OpenMote* self) {
}

void udpinject_trigger(OpenMote* self) {
   OpenQueueEntry_t* pkt;
   uint8_t number_bytes_from_input_buffer;
   uint8_t input_buffer[18];
   //get command from OpenSerial (16B IPv6 destination address, 2B destination port)
   number_bytes_from_input_buffer = openserial_getInputBuffer(self, &(input_buffer[0]),sizeof(input_buffer));
   if (number_bytes_from_input_buffer!=sizeof(input_buffer)) {
 openserial_printError(self, COMPONENT_UDPINJECT,ERR_INPUTBUFFER_LENGTH,
                            (errorparameter_t)number_bytes_from_input_buffer,
                            (errorparameter_t)0);
      return;
   };
   //prepare packet
   pkt = openqueue_getFreePacketBuffer(self, COMPONENT_UDPINJECT);
   if (pkt==NULL) {
 openserial_printError(self, COMPONENT_UDPINJECT,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      return;
   }
   pkt->creator                     = COMPONENT_UDPINJECT;
   pkt->owner                       = COMPONENT_UDPINJECT;
   pkt->l4_protocol                 = IANA_UDP;
   pkt->l4_sourcePortORicmpv6Type   = WKP_UDP_INJECT;
   pkt->l4_destination_port         = packetfunctions_ntohs(self, &(input_buffer[16]));
   pkt->l3_destinationAdd.type = ADDR_128B;
   memcpy(&(pkt->l3_destinationAdd.addr_128b[0]),&(input_buffer[0]),16);
 packetfunctions_reserveHeaderSize(self, pkt,6);
   ((uint8_t*)pkt->payload)[0]      = 'p';
   ((uint8_t*)pkt->payload)[1]      = 'o';
   ((uint8_t*)pkt->payload)[2]      = 'i';
   ((uint8_t*)pkt->payload)[3]      = 'p';
   ((uint8_t*)pkt->payload)[4]      = 'o';
   ((uint8_t*)pkt->payload)[5]      = 'i';
   //send packet
   if (( openudp_send(self, pkt))==E_FAIL) {
 openqueue_freePacketBuffer(self, pkt);
   }
}

void udpinject_sendDone(OpenMote* self, OpenQueueEntry_t* msg, owerror_t error) {
   msg->owner = COMPONENT_UDPINJECT;
   if (msg->creator!=COMPONENT_UDPINJECT) {
 openserial_printError(self, COMPONENT_UDPINJECT,ERR_UNEXPECTED_SENDDONE,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
   }
 openqueue_freePacketBuffer(self, msg);
}

void udpinject_receive(OpenMote* self, OpenQueueEntry_t* msg) {
 openqueue_freePacketBuffer(self, msg);
}

bool udpinject_debugPrint(OpenMote* self) {
   return FALSE;
}

//=========================== private =========================================