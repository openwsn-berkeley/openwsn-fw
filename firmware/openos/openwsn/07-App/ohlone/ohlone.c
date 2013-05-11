#include "openwsn.h"
#include "ohlone.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "opentcp.h"

#include "ohlone_webpages.h"

//=========================== variables =======================================

ohlone_vars_t ohlone_vars;

//=========================== prototypes ======================================

void ohlone_sendpkt();
bool ohlone_check4chars(uint8_t c1[4], uint8_t c2[4]);

//=========================== public ==========================================

void ohlone_init() {
   ohlone_vars.httpChunk = 0;
   ohlone_vars.getRequest[0] = '/';
   ohlone_vars.getRequest[1] = ' ';
   ohlone_webpages_init();
}

bool ohlone_shouldIlisten() {
   return TRUE;
}

void ohlone_sendpkt() {
   uint8_t buffer[TCP_DEFAULT_WINDOW_SIZE];
   uint8_t buffer_len;
  
   buffer_len = ohlone_webpage(ohlone_vars.getRequest, ohlone_vars.httpChunk++, buffer);
   
   if (buffer_len == 0) {
      // No more to send
      // close TCP session, but keep listening
      ohlone_vars.getRequest[0] = '/';
      ohlone_vars.getRequest[1] = ' ';
      opentcp_close();
      return;
   }

   ohlone_vars.pkt = openqueue_getFreePacketBuffer(COMPONENT_OHLONE);
   if (ohlone_vars.pkt==NULL) {
      openserial_printError(COMPONENT_OHLONE,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      opentcp_close();
      return;
   }
   ohlone_vars.pkt->creator = COMPONENT_OHLONE;
   ohlone_vars.pkt->owner   = COMPONENT_OHLONE;
   
   packetfunctions_reserveHeaderSize(ohlone_vars.pkt, buffer_len);
   memcpy(ohlone_vars.pkt->payload, buffer, buffer_len);
   
   if ((opentcp_send(ohlone_vars.pkt))==E_FAIL) {
      openqueue_freePacketBuffer(ohlone_vars.pkt);
      opentcp_close();
   }

}

bool ohlone_check4chars(uint8_t c1[4], uint8_t c2[4]) {
  return ((c1[0] == c2[0]) && 
          (c1[1] == c2[1]) && 
          (c1[2] == c2[2]) && 
          (c1[3] == c2[3]));
}

void ohlone_receive(OpenQueueEntry_t* msg) {
   uint8_t payload_index;
   
   for (payload_index=0;payload_index<msg->length-3;payload_index++) {
      if (ohlone_check4chars(msg->payload+payload_index,(unsigned char *) "GET "))
         memcpy(ohlone_vars.getRequest, 
                msg->payload + payload_index + 4, 
                msg->length - payload_index - 4);

      if (ohlone_check4chars(msg->payload+payload_index, (unsigned char *)"\r\n\r\n")) {
         ohlone_vars.httpChunk = 0;
         ohlone_sendpkt();
         return;
      }
   }
   openserial_printData((uint8_t*)(msg->payload),msg->length);
   openqueue_freePacketBuffer(msg);
}

void ohlone_sendDone(OpenQueueEntry_t* msg, error_t error) {
   msg->owner = COMPONENT_OHLONE;
   if (msg->creator!=COMPONENT_OHLONE) {
      openserial_printError(COMPONENT_OHLONE,ERR_UNEXPECTED_SENDDONE,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
   }
   
   ohlone_sendpkt();
   openqueue_freePacketBuffer(msg);
}

void ohlone_connectDone(error_t error) {
}

bool ohlone_debugPrint() {
   return FALSE;
}

//=========================== private =========================================