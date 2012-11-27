#include "openwsn.h"
#include "openbridge.h"
#include "openserial.h"
#include "packetfunctions.h"
#include "iphc.h"
#include "idmanager.h"
#include "openqueue.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void openbridge_init() {
}

void openbridge_trigger() {
   uint8_t           input_buffer[136];//worst case: 8B of next hop + 128B of data
   OpenQueueEntry_t* pkt;
   uint8_t           numDataBytes;
   numDataBytes = openserial_getNumDataBytes();
   openserial_getInputBuffer(&(input_buffer[0]),numDataBytes);
   
  //this is a temporal workaround as we are never supposed to get chunks of data
   //longer than input buffer size.. I assume that HDLC will solve that.
   
   if (numDataBytes>136){
       openserial_printError(COMPONENT_OPENBRIDGE,ERR_INPUTBUFFER_LENGTH,
                   (errorparameter_t)0,
                   (errorparameter_t)numDataBytes);
       //return.
       //poipoi xv test that..
       numDataBytes=sizeof(input_buffer);
   }
   
   if (idmanager_getIsBridge()==TRUE && numDataBytes>0) {
      pkt = openqueue_getFreePacketBuffer(COMPONENT_OPENBRIDGE);
      if (pkt==NULL) {
         openserial_printError(COMPONENT_OPENBRIDGE,ERR_NO_FREE_PACKET_BUFFER,
                               (errorparameter_t)0,
                               (errorparameter_t)0);
         return;
      }
      //admin
      pkt->creator  = COMPONENT_OPENBRIDGE;
      pkt->owner    = COMPONENT_OPENBRIDGE;
      //l2
      pkt->l2_nextORpreviousHop.type = ADDR_64B;
      memcpy(&(pkt->l2_nextORpreviousHop.addr_64b[0]),&(input_buffer[0]),8);
      //payload
      packetfunctions_reserveHeaderSize(pkt,numDataBytes-8);
      memcpy(pkt->payload,&(input_buffer[8]),numDataBytes-8);
      //send
      if ((iphc_sendFromBridge(pkt))==E_FAIL) {
         openqueue_freePacketBuffer(pkt);
      }
   }
}

void openbridge_sendDone(OpenQueueEntry_t* msg, error_t error) {
   msg->owner = COMPONENT_OPENBRIDGE;
   if (msg->creator!=COMPONENT_OPENBRIDGE) {
      openserial_printError(COMPONENT_OPENBRIDGE,ERR_UNEXPECTED_SENDDONE,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
   }
   openqueue_freePacketBuffer(msg);
}

void openbridge_receive(OpenQueueEntry_t* msg) {
  //XV sending src and dest.
   uint8_t total;
   uint8_t size=sizeof(msg->l2_nextORpreviousHop.addr_64b);
   total=size;
   packetfunctions_reserveHeaderSize(msg,size);
   memcpy(msg->payload,msg->l2_nextORpreviousHop.addr_64b,size);
   
   size=sizeof(idmanager_getMyID(ADDR_64B)->addr_64b);
   total+=size;
   packetfunctions_reserveHeaderSize(msg,size);

   memcpy(msg->payload,idmanager_getMyID(ADDR_64B)->addr_64b,size);
  
  
   openserial_printData((uint8_t*)(msg->payload),msg->length);
   openqueue_freePacketBuffer(msg);
}

//=========================== private =========================================
