/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:11:02.989130.
*/
#include "openwsn_obj.h"
#include "ohlone_obj.h"
#include "openqueue_obj.h"
#include "packetfunctions_obj.h"
#include "openserial_obj.h"
#include "opentcp_obj.h"

#include "ohlone_webpages_obj.h"

//=========================== variables =======================================

// declaration of global variable _ohlone_vars_ removed during objectification.

//=========================== prototypes ======================================

void ohlone_sendpkt(OpenMote* self);
bool ohlone_check4chars(OpenMote* self, uint8_t c1[4], uint8_t c2[4]);

//=========================== public ==========================================

void ohlone_init(OpenMote* self) {
   (self->ohlone_vars).httpChunk = 0;
   (self->ohlone_vars).getRequest[0] = '/';
   (self->ohlone_vars).getRequest[1] = ' ';
   ohlone_webpages_init();
}

bool ohlone_shouldIlisten(OpenMote* self) {
   return TRUE;
}

void ohlone_sendpkt(OpenMote* self) {
   uint8_t buffer[TCP_DEFAULT_WINDOW_SIZE];
   uint8_t buffer_len;
  
   buffer_len = ohlone_webpage((self->ohlone_vars).getRequest, (self->ohlone_vars).httpChunk++, buffer);
   
   if (buffer_len == 0) {
      // No more to send
      // close TCP session, but keep listening
      (self->ohlone_vars).getRequest[0] = '/';
      (self->ohlone_vars).getRequest[1] = ' ';
 opentcp_close(self);
      return;
   }

   (self->ohlone_vars).pkt = openqueue_getFreePacketBuffer(self, COMPONENT_OHLONE);
   if ((self->ohlone_vars).pkt==NULL) {
 openserial_printError(self, COMPONENT_OHLONE,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
 opentcp_close(self);
      return;
   }
   (self->ohlone_vars).pkt->creator = COMPONENT_OHLONE;
   (self->ohlone_vars).pkt->owner   = COMPONENT_OHLONE;
   
 packetfunctions_reserveHeaderSize(self, (self->ohlone_vars).pkt, buffer_len);
   memcpy((self->ohlone_vars).pkt->payload, buffer, buffer_len);
   
   if (( opentcp_send(self, (self->ohlone_vars).pkt))==E_FAIL) {
 openqueue_freePacketBuffer(self, (self->ohlone_vars).pkt);
 opentcp_close(self);
   }

}

bool ohlone_check4chars(OpenMote* self, uint8_t c1[4], uint8_t c2[4]) {
  return ((c1[0] == c2[0]) && 
          (c1[1] == c2[1]) && 
          (c1[2] == c2[2]) && 
          (c1[3] == c2[3]));
}

void ohlone_receive(OpenMote* self, OpenQueueEntry_t* msg) {
   uint8_t payload_index;
   
   for (payload_index=0;payload_index<msg->length-3;payload_index++) {
      if ( ohlone_check4chars(self, msg->payload+payload_index,(unsigned char *) "GET "))
         memcpy((self->ohlone_vars).getRequest, 
                msg->payload + payload_index + 4, 
                msg->length - payload_index - 4);

      if ( ohlone_check4chars(self, msg->payload+payload_index, (unsigned char *)"\r\n\r\n")) {
         (self->ohlone_vars).httpChunk = 0;
 ohlone_sendpkt(self);
         return;
      }
   }
 openserial_printData(self, (uint8_t*)(msg->payload),msg->length);
 openqueue_freePacketBuffer(self, msg);
}

void ohlone_sendDone(OpenMote* self, OpenQueueEntry_t* msg, owerror_t error) {
   msg->owner = COMPONENT_OHLONE;
   if (msg->creator!=COMPONENT_OHLONE) {
 openserial_printError(self, COMPONENT_OHLONE,ERR_UNEXPECTED_SENDDONE,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
   }
   
 ohlone_sendpkt(self);
 openqueue_freePacketBuffer(self, msg);
}

void ohlone_connectDone(OpenMote* self, owerror_t error) {
}

bool ohlone_debugPrint(OpenMote* self) {
   return FALSE;
}

//=========================== private =========================================
