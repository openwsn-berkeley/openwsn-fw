#include "opendefs.h"
#include "tohlone.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "opentcp.h"

#include "tohlone_webpages.h"

//=========================== variables =======================================

tohlone_vars_t tohlone_vars;

//=========================== prototypes ======================================

void tohlone_sendpkt(void);
bool tohlone_check4chars(uint8_t c1[4], uint8_t c2[4]);

//=========================== public ==========================================

void tohlone_init() {
   tohlone_vars.httpChunk = 0;
   tohlone_vars.getRequest[0] = '/';
   tohlone_vars.getRequest[1] = ' ';
   tohlone_webpages_init();
}

bool tohlone_shouldIlisten() {
   return TRUE;
}

void tohlone_sendpkt() {
   uint8_t buffer[TCP_DEFAULT_WINDOW_SIZE];
   uint8_t buffer_len;
  
   buffer_len = tohlone_webpage(tohlone_vars.getRequest, tohlone_vars.httpChunk++, buffer);
   
   if (buffer_len == 0) {
      // No more to send
      // close TCP session, but keep listening
      tohlone_vars.getRequest[0] = '/';
      tohlone_vars.getRequest[1] = ' ';
      opentcp_close();
      return;
   }

   tohlone_vars.pkt = openqueue_getFreePacketBuffer(COMPONENT_TOHLONE);
   if (tohlone_vars.pkt==NULL) {
      openserial_printError(COMPONENT_TOHLONE,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      opentcp_close();
      return;
   }
   tohlone_vars.pkt->creator = COMPONENT_TOHLONE;
   tohlone_vars.pkt->owner   = COMPONENT_TOHLONE;
   
   packetfunctions_reserveHeaderSize(tohlone_vars.pkt, buffer_len);
   memcpy(tohlone_vars.pkt->payload, buffer, buffer_len);
   
   if ((opentcp_send(tohlone_vars.pkt))==E_FAIL) {
      openqueue_freePacketBuffer(tohlone_vars.pkt);
      opentcp_close();
   }

}

bool tohlone_check4chars(uint8_t c1[4], uint8_t c2[4]) {
  return ((c1[0] == c2[0]) && 
          (c1[1] == c2[1]) && 
          (c1[2] == c2[2]) && 
          (c1[3] == c2[3]));
}

void tohlone_receive(OpenQueueEntry_t* msg) {
   uint8_t payload_index;
   
   for (payload_index=0;payload_index<msg->length-3;payload_index++) {
      if (tohlone_check4chars(msg->payload+payload_index,(unsigned char *) "GET "))
         memcpy(tohlone_vars.getRequest, 
                msg->payload + payload_index + 4, 
                msg->length - payload_index - 4);

      if (tohlone_check4chars(msg->payload+payload_index, (unsigned char *)"\r\n\r\n")) {
         tohlone_vars.httpChunk = 0;
         tohlone_sendpkt();
         return;
      }
   }
   openserial_printData((uint8_t*)(msg->payload),msg->length);
   openqueue_freePacketBuffer(msg);
}

void tohlone_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   msg->owner = COMPONENT_TOHLONE;
   if (msg->creator!=COMPONENT_TOHLONE) {
      openserial_printError(COMPONENT_TOHLONE,ERR_UNEXPECTED_SENDDONE,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
   }
   
   tohlone_sendpkt();
   openqueue_freePacketBuffer(msg);
}

void tohlone_connectDone(owerror_t error) {
}

bool tohlone_debugPrint() {
   return FALSE;
}

//=========================== private =========================================
