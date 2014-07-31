/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:11:11.654790.
*/
#include "openwsn_obj.h"
#include "tcpinject_obj.h"
#include "openserial_obj.h"
#include "packetfunctions_obj.h"
#include "opentcp_obj.h"
#include "openqueue_obj.h"

//=========================== variables =======================================

// declaration of global variable _tcpinject_vars_ removed during objectification.

//=========================== prototypes ======================================

//=========================== public ==========================================

void tcpinject_init(OpenMote* self) {
}

bool tcpinject_shouldIlisten(OpenMote* self) {
   return FALSE;
}

void tcpinject_trigger(OpenMote* self) {
   uint8_t number_bytes_from_input_buffer;
   uint8_t input_buffer[18];
   //get command from OpenSerial (16B IPv6 destination address, 2B destination port)
   number_bytes_from_input_buffer = openserial_getInputBuffer(self, &(input_buffer[0]),sizeof(input_buffer));
   if (number_bytes_from_input_buffer!=sizeof(input_buffer)) {
 openserial_printError(self, COMPONENT_TCPINJECT,ERR_INPUTBUFFER_LENGTH,
                            (errorparameter_t)number_bytes_from_input_buffer,
                            (errorparameter_t)0);
      return;
   };
   (self->tcpinject_vars).hisAddress.type = ADDR_128B;
   memcpy(&((self->tcpinject_vars).hisAddress.addr_128b[0]),&(input_buffer[0]),16);
   (self->tcpinject_vars).hisPort = packetfunctions_ntohs(self, &(input_buffer[16]));
   //connect
 opentcp_connect(self, &(self->tcpinject_vars).hisAddress,(self->tcpinject_vars).hisPort,WKP_TCP_INJECT);
}

void tcpinject_connectDone(OpenMote* self, owerror_t error) {
   if (error==E_SUCCESS) {
      (self->tcpinject_vars).pkt = openqueue_getFreePacketBuffer(self, COMPONENT_TCPINJECT);
      if ((self->tcpinject_vars).pkt==NULL) {
 openserial_printError(self, COMPONENT_TCPINJECT,ERR_NO_FREE_PACKET_BUFFER,
                               (errorparameter_t)0,
                               (errorparameter_t)0);
         return;
      }
      (self->tcpinject_vars).pkt->creator                      = COMPONENT_TCPINJECT;
      (self->tcpinject_vars).pkt->owner                        = COMPONENT_TCPINJECT;
      (self->tcpinject_vars).pkt->l4_protocol                  = IANA_UDP;
      (self->tcpinject_vars).pkt->l4_sourcePortORicmpv6Type    = WKP_TCP_INJECT;
      (self->tcpinject_vars).pkt->l4_destination_port          = (self->tcpinject_vars).hisPort;
      memcpy(&((self->tcpinject_vars).pkt->l3_destinationAdd),&(self->tcpinject_vars).hisAddress,sizeof(open_addr_t));
 packetfunctions_reserveHeaderSize(self, (self->tcpinject_vars).pkt,6);
      ((uint8_t*)(self->tcpinject_vars).pkt->payload)[0] = 'p';
      ((uint8_t*)(self->tcpinject_vars).pkt->payload)[1] = 'o';
      ((uint8_t*)(self->tcpinject_vars).pkt->payload)[2] = 'i';
      ((uint8_t*)(self->tcpinject_vars).pkt->payload)[3] = 'p';
      ((uint8_t*)(self->tcpinject_vars).pkt->payload)[4] = 'o';
      ((uint8_t*)(self->tcpinject_vars).pkt->payload)[5] = 'i';
      if ( opentcp_send(self, (self->tcpinject_vars).pkt)==E_FAIL) {
 openqueue_freePacketBuffer(self, (self->tcpinject_vars).pkt);
      }
      return;
   }
}

void tcpinject_sendDone(OpenMote* self, OpenQueueEntry_t* msg, owerror_t error) {
   msg->owner = COMPONENT_TCPINJECT;
   if (msg->creator!=COMPONENT_TCPINJECT) {
 openserial_printError(self, COMPONENT_TCPINJECT,ERR_UNEXPECTED_SENDDONE,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
   }
 opentcp_close(self);
 openqueue_freePacketBuffer(self, msg);
}

void tcpinject_receive(OpenMote* self, OpenQueueEntry_t* msg) {
}

bool tcpinject_debugPrint(OpenMote* self) {
   return FALSE;
}

//=========================== private =========================================