/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:10:35.051425.
*/
#include "openwsn_obj.h"
#include "openbridge_obj.h"
#include "openserial_obj.h"
#include "packetfunctions_obj.h"
#include "iphc_obj.h"
#include "idmanager_obj.h"
#include "openqueue_obj.h"

//=========================== variables =======================================

//=========================== prototypes ======================================
//=========================== public ==========================================

void openbridge_init(OpenMote* self) {
}

void openbridge_triggerData(OpenMote* self) {
   uint8_t           input_buffer[136];//worst case: 8B of next hop + 128B of data
   OpenQueueEntry_t* pkt;
   uint8_t           numDataBytes;
  
   numDataBytes = openserial_getNumDataBytes(self);
  
   //poipoi xv
   //this is a temporal workaround as we are never supposed to get chunks of data
   //longer than input buffer size.. I assume that HDLC will solve that.
   // MAC header is 13B + 8 next hop so we cannot accept packets that are longer than 118B
   if (numDataBytes>(136 - 21) || numDataBytes<8){
   //to prevent too short or too long serial frames to kill the stack  
// openserial_printError(self, COMPONENT_OPENBRIDGE,ERR_INPUTBUFFER_LENGTH,
//                   (errorparameter_t)numDataBytes,
//                   (errorparameter_t)0);
       return;
   }
  
   //copying the buffer once we know it is not too big
 openserial_getInputBuffer(self, &(input_buffer[0]),numDataBytes);
  
   if ( idmanager_getIsBridge(self)==TRUE && numDataBytes>0) {
      pkt = openqueue_getFreePacketBuffer(self, COMPONENT_OPENBRIDGE);
      if (pkt==NULL) {
// openserial_printError(self, COMPONENT_OPENBRIDGE,ERR_NO_FREE_PACKET_BUFFER,
//                               (errorparameter_t)0,
//                               (errorparameter_t)0);
         return;
      }
      //admin
      pkt->creator  = COMPONENT_OPENBRIDGE;
      pkt->owner    = COMPONENT_OPENBRIDGE;
      //l2
      pkt->l2_nextORpreviousHop.type = ADDR_64B;
      memcpy(&(pkt->l2_nextORpreviousHop.addr_64b[0]),&(input_buffer[0]),8);
      //payload
 packetfunctions_reserveHeaderSize(self, pkt,numDataBytes-8);
      memcpy(pkt->payload,&(input_buffer[8]),numDataBytes-8);
      
      //this is to catch the too short packet. remove it after fw-103 is solved.
      if (numDataBytes<16){
// openserial_printError(self, COMPONENT_OPENBRIDGE,ERR_INVALIDSERIALFRAME,
//                            (errorparameter_t)0,
//                            (errorparameter_t)0);
      }        
      //send
      if (( iphc_sendFromBridge(self, pkt))==E_FAIL) {
 openqueue_freePacketBuffer(self, pkt);
      }
   }
}

void openbridge_sendDone(OpenMote* self, OpenQueueEntry_t* msg, owerror_t error) {
   msg->owner = COMPONENT_OPENBRIDGE;
   if (msg->creator!=COMPONENT_OPENBRIDGE) {
// openserial_printError(self, COMPONENT_OPENBRIDGE,ERR_UNEXPECTED_SENDDONE,
//                            (errorparameter_t)0,
//                            (errorparameter_t)0);
   }
 openqueue_freePacketBuffer(self, msg);
}

/**
\brief Receive a frame at the openbridge, which sends it out over serial.
*/
void openbridge_receive(OpenMote* self, OpenQueueEntry_t* msg) {
   
   // prepend previous hop
 packetfunctions_reserveHeaderSize(self, msg,LENGTH_ADDR64b);
   memcpy(msg->payload,msg->l2_nextORpreviousHop.addr_64b,LENGTH_ADDR64b);
   
   // prepend next hop (me)
 packetfunctions_reserveHeaderSize(self, msg,LENGTH_ADDR64b);
   memcpy(msg->payload, idmanager_getMyID(self, ADDR_64B)->addr_64b,LENGTH_ADDR64b);
   
   // send packet over serial (will be memcopied into serial buffer)
 openserial_printData(self, (uint8_t*)(msg->payload),msg->length);
   
   // free packet
 openqueue_freePacketBuffer(self, msg);
}

//=========================== private =========================================
