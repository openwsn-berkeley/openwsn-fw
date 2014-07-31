/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:02:15.123502.
*/
#include "openwsn_obj.h"
#include "IEEE802154E_obj.h"
#include "radio_obj.h"
#include "radiotimer_obj.h"
#include "IEEE802154_obj.h"
#include "openqueue_obj.h"
#include "idmanager_obj.h"
#include "openserial_obj.h"
#include "schedule_obj.h"
#include "packetfunctions_obj.h"
#include "scheduler_obj.h"
#include "leds_obj.h"
#include "neighbors_obj.h"
#include "debugpins_obj.h"
#include "sixtop_obj.h"
#include "adaptive_sync_obj.h"
#include "processIE_obj.h"
//START OF TELEMATICS CODE
#include "security.h"
//END OF TELEMATICS CODE

//=========================== variables =======================================

// declaration of global variable _ieee154e_vars_ removed during objectification.
// declaration of global variable _ieee154e_stats_ removed during objectification.
// declaration of global variable _ieee154e_dbg_ removed during objectification.

//=========================== prototypes ======================================

// SYNCHRONIZING
void activity_synchronize_newSlot(OpenMote* self);
void activity_synchronize_startOfFrame(OpenMote* self, PORT_RADIOTIMER_WIDTH capturedTime);
void activity_synchronize_endOfFrame(OpenMote* self, PORT_RADIOTIMER_WIDTH capturedTime);
// TX
void activity_ti1ORri1(OpenMote* self);
void activity_ti2(OpenMote* self);
void activity_tie1(OpenMote* self);
void activity_ti3(OpenMote* self);
void activity_tie2(OpenMote* self);
void activity_ti4(OpenMote* self, PORT_RADIOTIMER_WIDTH capturedTime);
void activity_tie3(OpenMote* self);
void activity_ti5(OpenMote* self, PORT_RADIOTIMER_WIDTH capturedTime);
void activity_ti6(OpenMote* self);
void activity_tie4(OpenMote* self);
void activity_ti7(OpenMote* self);
void activity_tie5(OpenMote* self);
void activity_ti8(OpenMote* self, PORT_RADIOTIMER_WIDTH capturedTime);
void activity_tie6(OpenMote* self);
void activity_ti9(OpenMote* self, PORT_RADIOTIMER_WIDTH capturedTime);
// RX
void activity_ri2(OpenMote* self);
void activity_rie1(OpenMote* self);
void activity_ri3(OpenMote* self);
void activity_rie2(OpenMote* self);
void activity_ri4(OpenMote* self, PORT_RADIOTIMER_WIDTH capturedTime);
void activity_rie3(OpenMote* self);
void activity_ri5(OpenMote* self, PORT_RADIOTIMER_WIDTH capturedTime);
void activity_ri6(OpenMote* self);
void activity_rie4(OpenMote* self);
void activity_ri7(OpenMote* self);
void activity_rie5(OpenMote* self);
void activity_ri8(OpenMote* self, PORT_RADIOTIMER_WIDTH capturedTime);
void activity_rie6(OpenMote* self);
void activity_ri9(OpenMote* self, PORT_RADIOTIMER_WIDTH capturedTime);
// frame validity check

bool isValidRxFrame(OpenMote* self, ieee802154_header_iht* ieee802514_header);
bool isValidAck(OpenMote* self, ieee802154_header_iht*     ieee802514_header,
                    OpenQueueEntry_t*          packetSent);
// IEs Handling
bool ieee154e_processIEs(OpenMote* self, OpenQueueEntry_t* pkt, uint16_t* lenIE);
void ieee154e_processSlotframeLinkIE(OpenMote* self, OpenQueueEntry_t* pkt,uint8_t * ptr);
// ASN handling
void incrementAsnOffset(OpenMote* self);
void asnStoreFromAdv(OpenMote* self, uint8_t* asn);
void joinPriorityStoreFromAdv(OpenMote* self, uint8_t jp);
// synchronization
void synchronizePacket(OpenMote* self, PORT_RADIOTIMER_WIDTH timeReceived);
void synchronizeAck(OpenMote* self, PORT_SIGNED_INT_WIDTH timeCorrection);
void changeIsSync(OpenMote* self, bool newIsSync);
// notifying upper layer
void notif_sendDone(OpenMote* self, OpenQueueEntry_t* packetSent, owerror_t error);
void notif_receive(OpenMote* self, OpenQueueEntry_t* packetReceived);
// statistics
void resetStats(OpenMote* self);
void updateStats(OpenMote* self, PORT_SIGNED_INT_WIDTH timeCorrection);
// misc
uint8_t calculateFrequency(OpenMote* self, uint8_t channelOffset);
void changeState(OpenMote* self, ieee154e_state_t newstate);
void endSlot(OpenMote* self);
bool debugPrint_asn(OpenMote* self);
bool debugPrint_isSync(OpenMote* self);

//=========================== admin ===========================================

/**
\brief This function initializes this module.

Call this function once before any other function in this module, possibly
during boot-up.
*/
void ieee154e_init(OpenMote* self) {
   
   // initialize variables
   memset(&(self->ieee154e_vars),0,sizeof(ieee154e_vars_t));
   memset(&(self->ieee154e_dbg),0,sizeof(ieee154e_dbg_t));
   
   if ( idmanager_getIsDAGroot(self)==TRUE) {
 changeIsSync(self, TRUE);
   } else {
 changeIsSync(self, FALSE);
   }
   
 resetStats(self);
   (self->ieee154e_stats).numDeSync                 = 0;
   
   // switch radio on
 radio_rfOn(self);
   
   // set callback functions for the radio
 radio_setOverflowCb(self, isr_ieee154e_newSlot);
 radio_setCompareCb(self, isr_ieee154e_timer);
 radio_setStartFrameCb(self, ieee154e_startOfFrame);
 radio_setEndFrameCb(self, ieee154e_endOfFrame);
   // have the radio start its timer
 radio_startTimer(self, TsSlotDuration);
}

//=========================== public ==========================================

/**
/brief Difference between some older ASN and the current ASN.

\param[in] someASN some ASN to compare to the current

\returns The ASN difference, or 0xffff if more than 65535 different
*/
PORT_RADIOTIMER_WIDTH ieee154e_asnDiff(OpenMote* self, asn_t* someASN) {
   PORT_RADIOTIMER_WIDTH diff;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   if ((self->ieee154e_vars).asn.byte4 != someASN->byte4) {
      ENABLE_INTERRUPTS();
      return (PORT_RADIOTIMER_WIDTH)0xFFFFFFFF;;
   }
   
   diff = 0;
   if ((self->ieee154e_vars).asn.bytes2and3 == someASN->bytes2and3) {
      ENABLE_INTERRUPTS();
      return (self->ieee154e_vars).asn.bytes0and1-someASN->bytes0and1;
   } else if ((self->ieee154e_vars).asn.bytes2and3-someASN->bytes2and3==1) {
      diff  = (self->ieee154e_vars).asn.bytes0and1;
      diff += 0xffff-someASN->bytes0and1;
      diff += 1;
   } else {
      diff = (PORT_RADIOTIMER_WIDTH)0xFFFFFFFF;;
   }
   ENABLE_INTERRUPTS();
   return diff;
}

//======= events

/**
\brief Indicates a new slot has just started.

This function executes in ISR mode, when the new slot timer fires.
*/
void isr_ieee154e_newSlot(OpenMote* self) {
 radio_setTimerPeriod(self, TsSlotDuration);
   if ((self->ieee154e_vars).isSync==FALSE) {
      if ( idmanager_getIsDAGroot(self)==TRUE) {
    	  //START OF TELEMATICS CODE
			 //If I'm the DAG Root, here I can store the Key
 scheduler_push_task(self, coordinator_init,TASKPRIO_SIXTOP_NOTIF_RX);
			 //END OF TELEMATICS CODE
 changeIsSync(self, TRUE);
      } else {
 activity_synchronize_newSlot(self);
      }
   } else {
     // adaptive synchronization
 adaptive_sync_countCompensationTimeout(self);
 activity_ti1ORri1(self);
   }
   (self->ieee154e_dbg).num_newSlot++;
}

/**
\brief Indicates the FSM timer has fired.

This function executes in ISR mode, when the FSM timer fires.
*/
void isr_ieee154e_timer(OpenMote* self) {
   switch ((self->ieee154e_vars).state) {
      case S_TXDATAOFFSET:
 activity_ti2(self);
         break;
      case S_TXDATAPREPARE:
 activity_tie1(self);
         break;
      case S_TXDATAREADY:
 activity_ti3(self);
         break;
      case S_TXDATADELAY:
 activity_tie2(self);
         break;
      case S_TXDATA:
 activity_tie3(self);
         break;
      case S_RXACKOFFSET:
 activity_ti6(self);
         break;
      case S_RXACKPREPARE:
 activity_tie4(self);
         break;
      case S_RXACKREADY:
 activity_ti7(self);
         break;
      case S_RXACKLISTEN:
 activity_tie5(self);
         break;
      case S_RXACK:
 activity_tie6(self);
         break;
      case S_RXDATAOFFSET:
 activity_ri2(self); 
         break;
      case S_RXDATAPREPARE:
 activity_rie1(self);
         break;
      case S_RXDATAREADY:
 activity_ri3(self);
         break;
      case S_RXDATALISTEN:
 activity_rie2(self);
         break;
      case S_RXDATA:
 activity_rie3(self);
         break;
      case S_TXACKOFFSET: 
 activity_ri6(self);
         break;
      case S_TXACKPREPARE:
 activity_rie4(self);
         break;
      case S_TXACKREADY:
 activity_ri7(self);
         break;
      case S_TXACKDELAY:
 activity_rie5(self);
         break;
      case S_TXACK:
 activity_rie6(self);
         break;
      default:
         // log the error
 openserial_printError(self, COMPONENT_IEEE802154E,ERR_WRONG_STATE_IN_TIMERFIRES,
                               (errorparameter_t)(self->ieee154e_vars).state,
                               (errorparameter_t)(self->ieee154e_vars).slotOffset);
         // abort
 endSlot(self);
         break;
   }
   (self->ieee154e_dbg).num_timer++;
}

/**
\brief Indicates the radio just received the first byte of a packet.

This function executes in ISR mode.
*/
void ieee154e_startOfFrame(OpenMote* self, PORT_RADIOTIMER_WIDTH capturedTime) {
   if ((self->ieee154e_vars).isSync==FALSE) {
 activity_synchronize_startOfFrame(self, capturedTime);
   } else {
      switch ((self->ieee154e_vars).state) {
         case S_TXDATADELAY:   
 activity_ti4(self, capturedTime);
            break;
         case S_RXACKREADY:
            /*
            It is possible to receive in this state for radio where there is no
            way of differentiated between "ready to listen" and "listening"
            (e.g. CC2420). We must therefore expect to the start of a packet in
            this "ready" state.
            */
            // no break!
         case S_RXACKLISTEN:
 activity_ti8(self, capturedTime);
            break;
         case S_RXDATAREADY:
            /*
            Similarly as above.
            */
            // no break!
         case S_RXDATALISTEN:
 activity_ri4(self, capturedTime);
            break;
         case S_TXACKDELAY:
 activity_ri8(self, capturedTime);
            break;
         default:
            // log the error
 openserial_printError(self, COMPONENT_IEEE802154E,ERR_WRONG_STATE_IN_NEWSLOT,
                                  (errorparameter_t)(self->ieee154e_vars).state,
                                  (errorparameter_t)(self->ieee154e_vars).slotOffset);
            // abort
 endSlot(self);
            break;
      }
   }
   (self->ieee154e_dbg).num_startOfFrame++;
}

/**
\brief Indicates the radio just received the last byte of a packet.

This function executes in ISR mode.
*/
void ieee154e_endOfFrame(OpenMote* self, PORT_RADIOTIMER_WIDTH capturedTime) {
   if ((self->ieee154e_vars).isSync==FALSE) {
 activity_synchronize_endOfFrame(self, capturedTime);
   } else {
      switch ((self->ieee154e_vars).state) {
         case S_TXDATA:
 activity_ti5(self, capturedTime);
            break;
         case S_RXACK:
 activity_ti9(self, capturedTime);
            break;
         case S_RXDATA:
 activity_ri5(self, capturedTime);
            break;
         case S_TXACK:
 activity_ri9(self, capturedTime);
            break;
         default:
            // log the error
 openserial_printError(self, COMPONENT_IEEE802154E,ERR_WRONG_STATE_IN_ENDOFFRAME,
                                  (errorparameter_t)(self->ieee154e_vars).state,
                                  (errorparameter_t)(self->ieee154e_vars).slotOffset);
            // abort
 endSlot(self);
            break;
      }
   }
   (self->ieee154e_dbg).num_endOfFrame++;
}

//======= misc

/**
\brief Trigger this module to print status information, over serial.

debugPrint_* functions are used by the openserial module to continuously print
status information about several modules in the OpenWSN stack.

\returns TRUE if this function printed something, FALSE otherwise.
*/
//bool debugPrint_asn(OpenMote* self) {
//   asn_t output;
//   output.byte4         =  (self->ieee154e_vars).asn.byte4;
//   output.bytes2and3    =  (self->ieee154e_vars).asn.bytes2and3;
//   output.bytes0and1    =  (self->ieee154e_vars).asn.bytes0and1;
// openserial_printStatus(self, STATUS_ASN,(uint8_t*)&output,sizeof(output));
//   return TRUE;
//}

/**
\brief Trigger this module to print status information, over serial.

debugPrint_* functions are used by the openserial module to continuously print
status information about several modules in the OpenWSN stack.

\returns TRUE if this function printed something, FALSE otherwise.
*/
//bool debugPrint_isSync(OpenMote* self) {
//   uint8_t output=0;
//   output = (self->ieee154e_vars).isSync;
// openserial_printStatus(self, STATUS_ISSYNC,(uint8_t*)&output,sizeof(uint8_t));
//   return TRUE;
//}

/**
\brief Trigger this module to print status information, over serial.

debugPrint_* functions are used by the openserial module to continuously print
status information about several modules in the OpenWSN stack.

\returns TRUE if this function printed something, FALSE otherwise.
*/
//bool debugPrint_macStats(OpenMote* self) {
//   // send current stats over serial
// openserial_printStatus(self, STATUS_MACSTATS,(uint8_t*)&(self->ieee154e_stats),sizeof(ieee154e_stats_t));
//   return TRUE;
//}

//=========================== private =========================================

//======= SYNCHRONIZING

port_INLINE void activity_synchronize_newSlot(OpenMote* self) {
   // I'm in the middle of receiving a packet
   if ((self->ieee154e_vars).state==S_SYNCRX) {
      return;
   }
   
   // if this is the first time I call this function while not synchronized,
   // switch on the radio in Rx mode
   if ((self->ieee154e_vars).state!=S_SYNCLISTEN) {
      // change state
 changeState(self, S_SYNCLISTEN);
      
      // turn off the radio (in case it wasn't yet)
 radio_rfOff(self);
      
      // configure the radio to listen to the default synchronizing channel
 radio_setFrequency(self, SYNCHRONIZING_CHANNEL);
      
      // update record of current channel
      (self->ieee154e_vars).freq = SYNCHRONIZING_CHANNEL;
      
      // switch on the radio in Rx mode.
 radio_rxEnable(self);
      (self->ieee154e_vars).radioOnInit= radio_getTimerValue(self);
      (self->ieee154e_vars).radioOnThisSlot=TRUE;
 radio_rxNow(self);
   }
   
   // increment ASN (used only to schedule serial activity)
 incrementAsnOffset(self);
   
   // to be able to receive and transmist serial even when not synchronized
   // take turns every 8 slots sending and receiving
   if        (((self->ieee154e_vars).asn.bytes0and1&0x000f)==0x0000) {
 openserial_stop(self);
 openserial_startOutput(self);
   } else if (((self->ieee154e_vars).asn.bytes0and1&0x000f)==0x0008) {
 openserial_stop(self);
 openserial_startInput(self);
   }
}

port_INLINE void activity_synchronize_startOfFrame(OpenMote* self, PORT_RADIOTIMER_WIDTH capturedTime) {
   
   // don't care about packet if I'm not listening
   if ((self->ieee154e_vars).state!=S_SYNCLISTEN) {
      return;
   }
   
   // change state
 changeState(self, S_SYNCRX);
   
   // stop the serial
 openserial_stop(self);
   
   // record the captured time 
   (self->ieee154e_vars).lastCapturedTime = capturedTime;
   
   // record the captured time (for sync)
   (self->ieee154e_vars).syncCapturedTime = capturedTime;
}

port_INLINE void activity_synchronize_endOfFrame(OpenMote* self, PORT_RADIOTIMER_WIDTH capturedTime) {
   ieee802154_header_iht ieee802514_header;
   uint16_t              lenIE;
   
   // check state
   if ((self->ieee154e_vars).state!=S_SYNCRX) {
      // log the error
 openserial_printError(self, COMPONENT_IEEE802154E,ERR_WRONG_STATE_IN_ENDFRAME_SYNC,
                            (errorparameter_t)(self->ieee154e_vars).state,
                            (errorparameter_t)0);
      // abort
 endSlot(self);
   }
   
   // change state
 changeState(self, S_SYNCPROC);
   
   // get a buffer to put the (received) frame in
   (self->ieee154e_vars).dataReceived = openqueue_getFreePacketBuffer(self, COMPONENT_IEEE802154E);
   if ((self->ieee154e_vars).dataReceived==NULL) {
      // log the error
 openserial_printError(self, COMPONENT_IEEE802154E,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      // abort
 endSlot(self);
      return;
   }
   
   // declare ownership over that packet
   (self->ieee154e_vars).dataReceived->creator = COMPONENT_IEEE802154E;
   (self->ieee154e_vars).dataReceived->owner   = COMPONENT_IEEE802154E;
   
   /*
   The do-while loop that follows is a little parsing trick.
   Because it contains a while(0) condition, it gets executed only once.
   The behavior is:
   - if a break occurs inside the do{} body, the error code below the loop
     gets executed. This indicates something is wrong with the packet being 
     parsed.
   - if a return occurs inside the do{} body, the error code below the loop
     does not get executed. This indicates the received packet is correct.
   */
   do { // this "loop" is only executed once
      
      // retrieve the received data frame from the radio's Rx buffer
      (self->ieee154e_vars).dataReceived->payload = &((self->ieee154e_vars).dataReceived->packet[FIRST_FRAME_BYTE]);
 radio_getReceivedFrame(self,        (self->ieee154e_vars).dataReceived->payload,
                                   &(self->ieee154e_vars).dataReceived->length,
                             sizeof((self->ieee154e_vars).dataReceived->packet),
                                   &(self->ieee154e_vars).dataReceived->l1_rssi,
                                   &(self->ieee154e_vars).dataReceived->l1_lqi,
                                   &(self->ieee154e_vars).dataReceived->l1_crc);
      
      // break if packet too short
      if ((self->ieee154e_vars).dataReceived->length<LENGTH_CRC || (self->ieee154e_vars).dataReceived->length>LENGTH_IEEE154_MAX) {
         // break from the do-while loop and execute abort code below
 openserial_printError(self, COMPONENT_IEEE802154E,ERR_INVALIDPACKETFROMRADIO,
                            (errorparameter_t)0,
                            (self->ieee154e_vars).dataReceived->length);
         break;
      }
      
      // toss CRC (2 last bytes)
 packetfunctions_tossFooter(self,    (self->ieee154e_vars).dataReceived, LENGTH_CRC);
      
      // break if invalid CRC
      if ((self->ieee154e_vars).dataReceived->l1_crc==FALSE) {
         // break from the do-while loop and execute abort code below
         break;
      }
      
      // parse the IEEE802.15.4 header (synchronize, end of frame)
 ieee802154_retrieveHeader(self, (self->ieee154e_vars).dataReceived,&ieee802514_header);
      
      //START OF TELEMATICS CODE
        //if I'm not the DAG Root and I'm not synch, I can store the Key
  		if( idmanager_getIsDAGroot(self)==FALSE && ieee154e_isSynch(self) == FALSE
  		   && ieee802514_header.frameType == IEEE154_TYPE_BEACON){
  			remote_init(ieee802514_header);
  	  }
  	  //END OF TELEMATICS CODE

      // break if invalid IEEE802.15.4 header
      if (ieee802514_header.valid==FALSE) {
         // break from the do-while loop and execute the clean-up code below
         break;
      }
      
      // store header details in packet buffer
      (self->ieee154e_vars).dataReceived->l2_frameType = ieee802514_header.frameType;
      (self->ieee154e_vars).dataReceived->l2_dsn       = ieee802514_header.dsn;
      memcpy(&((self->ieee154e_vars).dataReceived->l2_nextORpreviousHop),&(ieee802514_header.src),sizeof(open_addr_t));
      
      // toss the IEEE802.15.4 header -- this does not include IEs as they are processed 
      // next.
 packetfunctions_tossHeader(self, (self->ieee154e_vars).dataReceived,ieee802514_header.headerLength);
      
      // process IEs
      lenIE = 0;
      if (
            (
               ieee802514_header.valid==TRUE                                                       &&
               ieee802514_header.ieListPresent==TRUE                                               &&
               ieee802514_header.frameType==IEEE154_TYPE_BEACON                                    &&
 packetfunctions_sameAddress(self, &ieee802514_header.panid, idmanager_getMyID(self, ADDR_PANID)) &&
 ieee154e_processIEs(self, (self->ieee154e_vars).dataReceived,&lenIE)
            )==FALSE) {
         // break from the do-while loop and execute the clean-up code below
         break;
      }
      
      // turn off the radio
 radio_rfOff(self);
      
      // compute radio duty cycle
      (self->ieee154e_vars).radioOnTics += ( radio_getTimerValue(self)-(self->ieee154e_vars).radioOnInit);
      
      // toss the IEs
 packetfunctions_tossHeader(self, (self->ieee154e_vars).dataReceived,lenIE);
      
      // synchronize (for the first time) to the sender's ADV
 synchronizePacket(self, (self->ieee154e_vars).syncCapturedTime);
      
      // declare synchronized
 changeIsSync(self, TRUE);
      
      // log the info
 openserial_printInfo(self, COMPONENT_IEEE802154E,ERR_SYNCHRONIZED,
                            (errorparameter_t)(self->ieee154e_vars).slotOffset,
                            (errorparameter_t)0);
      
      // send received ADV up the stack so RES can update statistics (synchronizing)
 notif_receive(self, (self->ieee154e_vars).dataReceived);
      
      // clear local variable
      (self->ieee154e_vars).dataReceived = NULL;
      
      // official end of synchronization
 endSlot(self);
      
      // everything went well, return here not to execute the error code below
      return;
      
   } while(0);
   
   // free the (invalid) received data buffer so RAM memory can be recycled
 openqueue_freePacketBuffer(self, (self->ieee154e_vars).dataReceived);
   
   // clear local variable
   (self->ieee154e_vars).dataReceived = NULL;
   
   // return to listening state
 changeState(self, S_SYNCLISTEN);
}

port_INLINE bool ieee154e_processIEs(OpenMote* self, OpenQueueEntry_t* pkt, uint16_t* lenIE) {
   uint8_t               ptr;
   uint8_t               byte0;
   uint8_t               byte1;
   uint8_t               temp_8b;
   uint8_t               gr_elem_id;
   uint8_t               subid;
   uint16_t              temp_16b;
   uint16_t              len;
   uint16_t              sublen;
   PORT_SIGNED_INT_WIDTH timeCorrection;
   
   ptr=0;
   
   //===== header or payload IE header
   
   //candidate IE header  if type ==0 header IE if type==1 payload IE
   temp_8b    = *((uint8_t*)(pkt->payload)+ptr);
   ptr++;
   
   temp_16b   = temp_8b + ((*((uint8_t*)(pkt->payload)+ptr))<< 8);
   ptr++;
   
   *lenIE     = ptr;
   
   if ((temp_16b & IEEE802154E_DESC_TYPE_PAYLOAD_IE) == IEEE802154E_DESC_TYPE_PAYLOAD_IE){
      // payload IE
      
      len          = (temp_16b & IEEE802154E_DESC_LEN_PAYLOAD_IE_MASK)>>IEEE802154E_DESC_LEN_PAYLOAD_IE_SHIFT;
      gr_elem_id   = (temp_16b & IEEE802154E_DESC_GROUPID_PAYLOAD_IE_MASK)>>IEEE802154E_DESC_GROUPID_PAYLOAD_IE_SHIFT;
   } else {
      // header IE
      
      len          = (temp_16b & IEEE802154E_DESC_LEN_HEADER_IE_MASK)>>IEEE802154E_DESC_LEN_HEADER_IE_SHIFT;
      gr_elem_id   = (temp_16b & IEEE802154E_DESC_ELEMENTID_HEADER_IE_MASK)>>IEEE802154E_DESC_ELEMENTID_HEADER_IE_SHIFT; 
   }
   
   *lenIE         += len;
   
   //===== sub-elements
   
   switch(gr_elem_id){
      
      case IEEE802154E_MLME_IE_GROUPID:
         // MLME IE
         
         do {
            
            //read sub IE header
            temp_8b     = *((uint8_t*)(pkt->payload)+ptr);
            ptr         = ptr + 1;
            temp_16b    = temp_8b  +(*((uint8_t*)(pkt->payload)+ptr) << 8);
            ptr         = ptr + 1;
            
            len         = len - 2; //remove header fields len
            
            if ((temp_16b & IEEE802154E_DESC_TYPE_LONG) == IEEE802154E_DESC_TYPE_LONG){
               // long sub-IE
               
               sublen   = (temp_16b & IEEE802154E_DESC_LEN_LONG_MLME_IE_MASK)>>IEEE802154E_DESC_LEN_LONG_MLME_IE_SHIFT;
               subid    = (temp_16b & IEEE802154E_DESC_SUBID_LONG_MLME_IE_MASK)>>IEEE802154E_DESC_SUBID_LONG_MLME_IE_SHIFT; 
            } else {
               // short sub-IE
               
               sublen   = (temp_16b & IEEE802154E_DESC_LEN_SHORT_MLME_IE_MASK)>>IEEE802154E_DESC_LEN_SHORT_MLME_IE_SHIFT;
               subid    = (temp_16b & IEEE802154E_DESC_SUBID_SHORT_MLME_IE_MASK)>>IEEE802154E_DESC_SUBID_SHORT_MLME_IE_SHIFT; 
            }
            
            switch(subid){
               
               case IEEE802154E_MLME_SYNC_IE_SUBID:
                  // Sync IE: ASN and Join Priority 
                  
                  if ( idmanager_getIsDAGroot(self)==FALSE) {
                     // ASN
 asnStoreFromAdv(self, (uint8_t*)(pkt->payload)+ptr);
                     ptr = ptr + 5;
                     // join priority
 joinPriorityStoreFromAdv(self, *((uint8_t*)(pkt->payload)+ptr));
                     ptr = ptr + 1;
                  }
                  break;
               
               case IEEE802154E_MLME_SLOTFRAME_LINK_IE_SUBID:
 processIE_retrieveSlotframeLinkIE(self, pkt,&ptr);
                  break;
               
               case IEEE802154E_MLME_TIMESLOT_IE_SUBID:
                  //TODO
                  break;
               
               default:
                  return FALSE;
                  break;
            }
            
            len = len - sublen;
         } while(len>0);
         
         break;
      
      case IEEE802154E_ACK_NACK_TIMECORRECTION_ELEMENTID:
         // timecorrection IE
         
         if (
 idmanager_getIsDAGroot(self)==FALSE &&
 neighbors_isPreferredParent(self, &(pkt->l2_nextORpreviousHop))
            ) {
            
            byte0 = *((uint8_t*)(pkt->payload)+ptr);
            ptr++;
            byte1 = *((uint8_t*)(pkt->payload)+ptr);
            ptr++;

            timeCorrection  = (int16_t)((uint16_t)byte1<<8 | (uint16_t)byte0);
            timeCorrection  = (timeCorrection / (PORT_SIGNED_INT_WIDTH)US_PER_TICK);
            timeCorrection  = -timeCorrection;
            
 synchronizeAck(self, timeCorrection);
         }
         break;
         
      default:
         *lenIE = 0; //no header or not recognized.
         return FALSE;
   }
   
   if(*lenIE>127) {
      // log the error
 openserial_printError(self, 
         COMPONENT_IEEE802154E,
         ERR_HEADER_TOO_LONG,
         (errorparameter_t)*lenIE,
         (errorparameter_t)1
      );
   }
   return TRUE;
}

//======= TX

port_INLINE void activity_ti1ORri1(OpenMote* self) {
   cellType_t  cellType;
   open_addr_t neighbor;
   uint8_t     i;
   sync_IE_ht  sync_IE;

   // increment ASN (do this first so debug pins are in sync)
 incrementAsnOffset(self);
   
   // wiggle debug pins
 debugpins_slot_toggle(self);
   if ((self->ieee154e_vars).slotOffset==0) {
 debugpins_frame_toggle(self);
   }
   
   // desynchronize if needed
   if ( idmanager_getIsDAGroot(self)==FALSE) {
      (self->ieee154e_vars).deSyncTimeout--;
      if ((self->ieee154e_vars).deSyncTimeout==0) {
         // declare myself desynchronized
 changeIsSync(self, FALSE);
        
         // log the error
 openserial_printError(self, COMPONENT_IEEE802154E,ERR_DESYNCHRONIZED,
                               (errorparameter_t)(self->ieee154e_vars).slotOffset,
                               (errorparameter_t)0);
            
         // update the statistics
         (self->ieee154e_stats).numDeSync++;
            
         // abort
 endSlot(self);
         return;
      }
   }
   
   // if the previous slot took too long, we will not be in the right state
   if ((self->ieee154e_vars).state!=S_SLEEP) {
      // log the error
 openserial_printError(self, COMPONENT_IEEE802154E,ERR_WRONG_STATE_IN_STARTSLOT,
                            (errorparameter_t)(self->ieee154e_vars).state,
                            (errorparameter_t)(self->ieee154e_vars).slotOffset);
      // abort
 endSlot(self);
      return;
   }
   
   if ((self->ieee154e_vars).slotOffset==(self->ieee154e_vars).nextActiveSlotOffset) {
      // this is the next active slot
      
      // advance the schedule
 schedule_advanceSlot(self);
      
      // find the next one
      (self->ieee154e_vars).nextActiveSlotOffset    = schedule_getNextActiveSlotOffset(self);
   } else {
      // this is NOT the next active slot, abort
      // stop using serial
 openserial_stop(self);
      // abort the slot
 endSlot(self);
      //start outputing serial
 openserial_startOutput(self);
      return;
   }
   
   // check the schedule to see what type of slot this is
   cellType = schedule_getType(self);
   switch (cellType) {
      case CELLTYPE_ADV:
         // stop using serial
 openserial_stop(self);
         // look for an ADV packet in the queue
         (self->ieee154e_vars).dataToSend = openqueue_macGetAdvPacket(self);
         if ((self->ieee154e_vars).dataToSend==NULL) {   // I will be listening for an ADV
            // change state
 changeState(self, S_RXDATAOFFSET);
            // arm rt1
 radiotimer_schedule(self, DURATION_rt1);
         } else {                                // I will be sending an ADV
            // change state
 changeState(self, S_TXDATAOFFSET);
            // change owner
            (self->ieee154e_vars).dataToSend->owner = COMPONENT_IEEE802154E;         
            //copy synch IE  -- should be Little endian???
            // fill in the ASN field of the ADV
 ieee154e_getAsn(self, sync_IE.asn);
            sync_IE.join_priority = neighbors_getMyDAGrank(self)/(2*MINHOPRANKINCREASE); //poipoi -- use dagrank(rank) 
       
            memcpy((self->ieee154e_vars).dataToSend->l2_ASNpayload,&sync_IE,sizeof(sync_IE_ht));
            
            // record that I attempt to transmit this packet
            (self->ieee154e_vars).dataToSend->l2_numTxAttempts++;
            // arm tt1
 radiotimer_schedule(self, DURATION_tt1);
         }
         break;
      case CELLTYPE_TXRX:
      case CELLTYPE_TX:
         // stop using serial
 openserial_stop(self);
         // check whether we can send
         if ( schedule_getOkToSend(self)) {
 schedule_getNeighbor(self, &neighbor);
            (self->ieee154e_vars).dataToSend = openqueue_macGetDataPacket(self, &neighbor);
         } else {
            (self->ieee154e_vars).dataToSend = NULL;
         }
         if ((self->ieee154e_vars).dataToSend!=NULL) {   // I have a packet to send
            // change state
 changeState(self, S_TXDATAOFFSET);
            // change owner
            (self->ieee154e_vars).dataToSend->owner = COMPONENT_IEEE802154E;
            // record that I attempt to transmit this packet
            (self->ieee154e_vars).dataToSend->l2_numTxAttempts++;
            // arm tt1
 radiotimer_schedule(self, DURATION_tt1);
         } else if (cellType==CELLTYPE_TX){
            // abort
 endSlot(self);
         }
         if (cellType==CELLTYPE_TX || 
             (cellType==CELLTYPE_TXRX && (self->ieee154e_vars).dataToSend!=NULL)) {
            break;
         }
      case CELLTYPE_RX:
         // stop using serial
 openserial_stop(self);
         // change state
 changeState(self, S_RXDATAOFFSET);
         // arm rt1
 radiotimer_schedule(self, DURATION_rt1);
         break;
      case CELLTYPE_SERIALRX:
         // stop using serial
 openserial_stop(self);
         // abort the slot
 endSlot(self);
         //start inputting serial data
 openserial_startInput(self);
         //this is to emulate a set of serial input slots without having the slotted structure.

 radio_setTimerPeriod(self, TsSlotDuration*(NUMSERIALRX));
         
         //increase ASN by NUMSERIALRX-1 slots as at this slot is already incremented by 1
         for (i=0;i<NUMSERIALRX-1;i++){
 incrementAsnOffset(self);
         }
         // deal with the case when schedule multi slots
 adaptive_sync_countCompensationTimeout_compoundSlots(self, NUMSERIALRX-1);
         
         break;
      case CELLTYPE_MORESERIALRX:
         // do nothing (not even endSlot(self))
         break;
      default:
         // stop using serial
 openserial_stop(self);
         // log the error
 openserial_printCritical(self, COMPONENT_IEEE802154E,ERR_WRONG_CELLTYPE,
                               (errorparameter_t)cellType,
                               (errorparameter_t)(self->ieee154e_vars).slotOffset);
         // abort
 endSlot(self);
         break;
   }
}

port_INLINE void activity_ti2(OpenMote* self) {
   // change state
 changeState(self, S_TXDATAPREPARE);
   
   // calculate the frequency to transmit on
   (self->ieee154e_vars).freq = calculateFrequency(self, schedule_getChannelOffset(self)); 
   
   // configure the radio for that frequency
 radio_setFrequency(self, (self->ieee154e_vars).freq);
   
   // load the packet in the radio's Tx buffer
 radio_loadPacket(self, (self->ieee154e_vars).dataToSend->payload,
                    (self->ieee154e_vars).dataToSend->length);
   
   // enable the radio in Tx mode. This does not send the packet.
 radio_txEnable(self);
   (self->ieee154e_vars).radioOnInit= radio_getTimerValue(self);
   (self->ieee154e_vars).radioOnThisSlot=TRUE;
   // arm tt2
 radiotimer_schedule(self, DURATION_tt2);
   
   // change state
 changeState(self, S_TXDATAREADY);
}

port_INLINE void activity_tie1(OpenMote* self) {
   // log the error
 openserial_printError(self, COMPONENT_IEEE802154E,ERR_MAXTXDATAPREPARE_OVERFLOW,
                         (errorparameter_t)(self->ieee154e_vars).state,
                         (errorparameter_t)(self->ieee154e_vars).slotOffset);
   
   // abort
 endSlot(self);
}

port_INLINE void activity_ti3(OpenMote* self) {
   // change state
 changeState(self, S_TXDATADELAY);
   
   // arm tt3
 radiotimer_schedule(self, DURATION_tt3);
   
   // give the 'go' to transmit
 radio_txNow(self);
}

port_INLINE void activity_tie2(OpenMote* self) {
   // log the error
 openserial_printError(self, COMPONENT_IEEE802154E,ERR_WDRADIO_OVERFLOWS,
                         (errorparameter_t)(self->ieee154e_vars).state,
                         (errorparameter_t)(self->ieee154e_vars).slotOffset);
   
   // abort
 endSlot(self);
}

//start of frame interrupt
port_INLINE void activity_ti4(OpenMote* self, PORT_RADIOTIMER_WIDTH capturedTime) {
   // change state
 changeState(self, S_TXDATA);
   
   // cancel tt3
 radiotimer_cancel(self);
   
   // record the captured time
   (self->ieee154e_vars).lastCapturedTime = capturedTime;
   
   // arm tt4
 radiotimer_schedule(self, DURATION_tt4);
}

port_INLINE void activity_tie3(OpenMote* self) {
   // log the error
 openserial_printError(self, COMPONENT_IEEE802154E,ERR_WDDATADURATION_OVERFLOWS,
                         (errorparameter_t)(self->ieee154e_vars).state,
                         (errorparameter_t)(self->ieee154e_vars).slotOffset);
   
   // abort
 endSlot(self);
}

port_INLINE void activity_ti5(OpenMote* self, PORT_RADIOTIMER_WIDTH capturedTime) {
   bool listenForAck;
   
   // change state
 changeState(self, S_RXACKOFFSET);
   
   // cancel tt4
 radiotimer_cancel(self);
   
   // turn off the radio
 radio_rfOff(self);
   (self->ieee154e_vars).radioOnTics+=( radio_getTimerValue(self)-(self->ieee154e_vars).radioOnInit);
   
   // record the captured time
   (self->ieee154e_vars).lastCapturedTime = capturedTime;
   
   // decides whether to listen for an ACK
   if ( packetfunctions_isBroadcastMulticast(self, &(self->ieee154e_vars).dataToSend->l2_nextORpreviousHop)==TRUE) {
      listenForAck = FALSE;
   } else {
      listenForAck = TRUE;
   }
   
   if (listenForAck==TRUE) {
      // arm tt5
 radiotimer_schedule(self, DURATION_tt5);
   } else {
      // indicate succesful Tx to schedule to keep statistics
 schedule_indicateTx(self, &(self->ieee154e_vars).asn,TRUE);
      // indicate to upper later the packet was sent successfully
 notif_sendDone(self, (self->ieee154e_vars).dataToSend,E_SUCCESS);
      // reset local variable
      (self->ieee154e_vars).dataToSend = NULL;
      // abort
 endSlot(self);
   }
}

port_INLINE void activity_ti6(OpenMote* self) {
   // change state
 changeState(self, S_RXACKPREPARE);
   
   // calculate the frequency to transmit on
   (self->ieee154e_vars).freq = calculateFrequency(self, schedule_getChannelOffset(self)); 
   
   // configure the radio for that frequency
 radio_setFrequency(self, (self->ieee154e_vars).freq);
   
   // enable the radio in Rx mode. The radio is not actively listening yet.
 radio_rxEnable(self);
   //caputre init of radio for duty cycle calculation
   (self->ieee154e_vars).radioOnInit= radio_getTimerValue(self);
   (self->ieee154e_vars).radioOnThisSlot=TRUE;
   // arm tt6
 radiotimer_schedule(self, DURATION_tt6);
   
   // change state
 changeState(self, S_RXACKREADY);
}

port_INLINE void activity_tie4(OpenMote* self) {
   // log the error
 openserial_printError(self, COMPONENT_IEEE802154E,ERR_MAXRXACKPREPARE_OVERFLOWS,
                         (errorparameter_t)(self->ieee154e_vars).state,
                         (errorparameter_t)(self->ieee154e_vars).slotOffset);
   
   // abort
 endSlot(self);
}

port_INLINE void activity_ti7(OpenMote* self) {
   // change state
 changeState(self, S_RXACKLISTEN);
   
   // start listening
 radio_rxNow(self);
   
   // arm tt7
 radiotimer_schedule(self, DURATION_tt7);
}

port_INLINE void activity_tie5(OpenMote* self) {
   // indicate transmit failed to schedule to keep stats
 schedule_indicateTx(self, &(self->ieee154e_vars).asn,FALSE);
   
   // decrement transmits left counter
   (self->ieee154e_vars).dataToSend->l2_retriesLeft--;
   
   if ((self->ieee154e_vars).dataToSend->l2_retriesLeft==0) {
      // indicate tx fail if no more retries left
 notif_sendDone(self, (self->ieee154e_vars).dataToSend,E_FAIL);
   } else {
      // return packet to the virtual COMPONENT_SIXTOP_TO_IEEE802154E component
      (self->ieee154e_vars).dataToSend->owner = COMPONENT_SIXTOP_TO_IEEE802154E;
   }
   
   // reset local variable
   (self->ieee154e_vars).dataToSend = NULL;
   
   // abort
 endSlot(self);
}

port_INLINE void activity_ti8(OpenMote* self, PORT_RADIOTIMER_WIDTH capturedTime) {
   // change state
 changeState(self, S_RXACK);
   
   // cancel tt7
 radiotimer_cancel(self);
   
   // record the captured time
   (self->ieee154e_vars).lastCapturedTime = capturedTime;
   
   // arm tt8
 radiotimer_schedule(self, DURATION_tt8);
}

port_INLINE void activity_tie6(OpenMote* self) {
   // abort
 endSlot(self);
}

port_INLINE void activity_ti9(OpenMote* self, PORT_RADIOTIMER_WIDTH capturedTime) {
   ieee802154_header_iht     ieee802514_header;
   uint16_t                  lenIE;
   
   // change state
 changeState(self, S_TXPROC);
   
   // cancel tt8
 radiotimer_cancel(self);
   
   // turn off the radio
 radio_rfOff(self);
   //compute tics radio on.
   (self->ieee154e_vars).radioOnTics+=( radio_getTimerValue(self)-(self->ieee154e_vars).radioOnInit);
   
   // record the captured time
   (self->ieee154e_vars).lastCapturedTime = capturedTime;
   
   // get a buffer to put the (received) ACK in
   (self->ieee154e_vars).ackReceived = openqueue_getFreePacketBuffer(self, COMPONENT_IEEE802154E);
   if ((self->ieee154e_vars).ackReceived==NULL) {
      // log the error
 openserial_printError(self, COMPONENT_IEEE802154E,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      // abort
 endSlot(self);
      return;
   }
   
   // declare ownership over that packet
   (self->ieee154e_vars).ackReceived->creator = COMPONENT_IEEE802154E;
   (self->ieee154e_vars).ackReceived->owner   = COMPONENT_IEEE802154E;
   
   /*
   The do-while loop that follows is a little parsing trick.
   Because it contains a while(0) condition, it gets executed only once.
   Below the do-while loop is some code to cleans up the ack variable.
   Anywhere in the do-while loop, a break statement can be called to jump to
   the clean up code early. If the loop ends without a break, the received
   packet was correct. If it got aborted early (through a break), the packet
   was faulty.
   */
   do { // this "loop" is only executed once
      
      // retrieve the received ack frame from the radio's Rx buffer
      (self->ieee154e_vars).ackReceived->payload = &((self->ieee154e_vars).ackReceived->packet[FIRST_FRAME_BYTE]);
 radio_getReceivedFrame(self,        (self->ieee154e_vars).ackReceived->payload,
                                   &(self->ieee154e_vars).ackReceived->length,
                             sizeof((self->ieee154e_vars).ackReceived->packet),
                                   &(self->ieee154e_vars).ackReceived->l1_rssi,
                                   &(self->ieee154e_vars).ackReceived->l1_lqi,
                                   &(self->ieee154e_vars).ackReceived->l1_crc);
      
      // break if wrong length
      if ((self->ieee154e_vars).ackReceived->length<LENGTH_CRC || (self->ieee154e_vars).ackReceived->length>LENGTH_IEEE154_MAX) {
         // break from the do-while loop and execute the clean-up code below
 openserial_printError(self, COMPONENT_IEEE802154E,ERR_INVALIDPACKETFROMRADIO,
                            (errorparameter_t)1,
                            (self->ieee154e_vars).ackReceived->length);
        
         break;
      }
      
      // toss CRC (2 last bytes)
 packetfunctions_tossFooter(self,    (self->ieee154e_vars).ackReceived, LENGTH_CRC);
   
      // break if invalid CRC
      if ((self->ieee154e_vars).ackReceived->l1_crc==FALSE) {
         // break from the do-while loop and execute the clean-up code below
         break;
      }
      
      // parse the IEEE802.15.4 header (RX ACK)
 ieee802154_retrieveHeader(self, (self->ieee154e_vars).ackReceived,&ieee802514_header);
      
      // break if invalid IEEE802.15.4 header
      if (ieee802514_header.valid==FALSE) {
         // break from the do-while loop and execute the clean-up code below
         break;
      }
      
      //START OF TELEMATICS CODE
      if((self->ieee154e_vars).ackReceived->l2_security== TRUE){
         security_incomingFrame((self->ieee154e_vars).ackReceived);
      }
      //END OF TELEMATICS CODE


      // store header details in packet buffer
      (self->ieee154e_vars).ackReceived->l2_frameType  = ieee802514_header.frameType;
      (self->ieee154e_vars).ackReceived->l2_dsn        = ieee802514_header.dsn;
      memcpy(&((self->ieee154e_vars).ackReceived->l2_nextORpreviousHop),&(ieee802514_header.src),sizeof(open_addr_t));
      
      // toss the IEEE802.15.4 header
 packetfunctions_tossHeader(self, (self->ieee154e_vars).ackReceived,ieee802514_header.headerLength);
      
      // break if invalid ACK
      if ( isValidAck(self, &ieee802514_header,(self->ieee154e_vars).dataToSend)==FALSE) {
         // break from the do-while loop and execute the clean-up code below
         break;
      }
      //hanlde IEs --xv poipoi
      if (ieee802514_header.ieListPresent==FALSE){
         break; //ack should contain IEs.
      }
      
      if ( ieee154e_processIEs(self, (self->ieee154e_vars).ackReceived,&lenIE)==FALSE){
        // invalid IEs in ACK
        break;
      }
 
      // toss the IEs
 packetfunctions_tossHeader(self, (self->ieee154e_vars).ackReceived,lenIE);
      
      // inform schedule of successful transmission
 schedule_indicateTx(self, &(self->ieee154e_vars).asn,TRUE);
      
      // inform upper layer
 notif_sendDone(self, (self->ieee154e_vars).dataToSend,E_SUCCESS);
      (self->ieee154e_vars).dataToSend = NULL;
      
      // in any case, execute the clean-up code below (processing of ACK done)
   } while (0);
   
   // free the received ack so corresponding RAM memory can be recycled
 openqueue_freePacketBuffer(self, (self->ieee154e_vars).ackReceived);
   
   // clear local variable
   (self->ieee154e_vars).ackReceived = NULL;
   
   // official end of Tx slot
 endSlot(self);
}

//======= RX

port_INLINE void activity_ri2(OpenMote* self) {
   // change state
 changeState(self, S_RXDATAPREPARE);
   
   // calculate the frequency to transmit on
   (self->ieee154e_vars).freq = calculateFrequency(self, schedule_getChannelOffset(self)); 
   
   // configure the radio for that frequency
 radio_setFrequency(self, (self->ieee154e_vars).freq);
   
   // enable the radio in Rx mode. The radio does not actively listen yet.
 radio_rxEnable(self);
   (self->ieee154e_vars).radioOnInit= radio_getTimerValue(self);
   (self->ieee154e_vars).radioOnThisSlot=TRUE;
   
   // arm rt2
 radiotimer_schedule(self, DURATION_rt2);
       
   // change state
 changeState(self, S_RXDATAREADY);
}

port_INLINE void activity_rie1(OpenMote* self) {
   // log the error
 openserial_printError(self, COMPONENT_IEEE802154E,ERR_MAXRXDATAPREPARE_OVERFLOWS,
                         (errorparameter_t)(self->ieee154e_vars).state,
                         (errorparameter_t)(self->ieee154e_vars).slotOffset);
   
   // abort
 endSlot(self);
}

port_INLINE void activity_ri3(OpenMote* self) {
   // change state
 changeState(self, S_RXDATALISTEN);
   
   // give the 'go' to receive
 radio_rxNow(self);
   
   // arm rt3 
 radiotimer_schedule(self, DURATION_rt3);
}

port_INLINE void activity_rie2(OpenMote* self) {
   // abort
 endSlot(self);
}

port_INLINE void activity_ri4(OpenMote* self, PORT_RADIOTIMER_WIDTH capturedTime) {

   // change state
 changeState(self, S_RXDATA);
   
   // cancel rt3
 radiotimer_cancel(self);
   
   // record the captured time
   (self->ieee154e_vars).lastCapturedTime = capturedTime;
   
   // record the captured time to sync
   (self->ieee154e_vars).syncCapturedTime = capturedTime;

 radiotimer_schedule(self, DURATION_rt4);
}

port_INLINE void activity_rie3(OpenMote* self) {
     
   // log the error
 openserial_printError(self, COMPONENT_IEEE802154E,ERR_WDDATADURATION_OVERFLOWS,
                         (errorparameter_t)(self->ieee154e_vars).state,
                         (errorparameter_t)(self->ieee154e_vars).slotOffset);
   
   // abort
 endSlot(self);
}

port_INLINE void activity_ri5(OpenMote* self, PORT_RADIOTIMER_WIDTH capturedTime) {
   ieee802154_header_iht ieee802514_header;
   uint16_t lenIE=0;
   
   // change state
 changeState(self, S_TXACKOFFSET);
   
   // cancel rt4
 radiotimer_cancel(self);

   // turn off the radio
 radio_rfOff(self);
   (self->ieee154e_vars).radioOnTics+= radio_getTimerValue(self)-(self->ieee154e_vars).radioOnInit;
   // get a buffer to put the (received) data in
   (self->ieee154e_vars).dataReceived = openqueue_getFreePacketBuffer(self, COMPONENT_IEEE802154E);
   if ((self->ieee154e_vars).dataReceived==NULL) {
      // log the error
 openserial_printError(self, COMPONENT_IEEE802154E,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      // abort
 endSlot(self);
      return;
   }
   
   // declare ownership over that packet
   (self->ieee154e_vars).dataReceived->creator = COMPONENT_IEEE802154E;
   (self->ieee154e_vars).dataReceived->owner   = COMPONENT_IEEE802154E;

   /*
   The do-while loop that follows is a little parsing trick.
   Because it contains a while(0) condition, it gets executed only once.
   The behavior is:
   - if a break occurs inside the do{} body, the error code below the loop
     gets executed. This indicates something is wrong with the packet being 
     parsed.
   - if a return occurs inside the do{} body, the error code below the loop
     does not get executed. This indicates the received packet is correct.
   */
   do { // this "loop" is only executed once
      
      // retrieve the received data frame from the radio's Rx buffer
      (self->ieee154e_vars).dataReceived->payload = &((self->ieee154e_vars).dataReceived->packet[FIRST_FRAME_BYTE]);
 radio_getReceivedFrame(self,        (self->ieee154e_vars).dataReceived->payload,
                                   &(self->ieee154e_vars).dataReceived->length,
                             sizeof((self->ieee154e_vars).dataReceived->packet),
                                   &(self->ieee154e_vars).dataReceived->l1_rssi,
                                   &(self->ieee154e_vars).dataReceived->l1_lqi,
                                   &(self->ieee154e_vars).dataReceived->l1_crc);
      
      // break if wrong length
      if ((self->ieee154e_vars).dataReceived->length<LENGTH_CRC || (self->ieee154e_vars).dataReceived->length>LENGTH_IEEE154_MAX ) {
         // jump to the error code below this do-while loop
 openserial_printError(self, COMPONENT_IEEE802154E,ERR_INVALIDPACKETFROMRADIO,
                            (errorparameter_t)2,
                            (self->ieee154e_vars).dataReceived->length);
         break;
      }
      
      // toss CRC (2 last bytes)
 packetfunctions_tossFooter(self,    (self->ieee154e_vars).dataReceived, LENGTH_CRC);
      
      // if CRC doesn't check, stop
      if ((self->ieee154e_vars).dataReceived->l1_crc==FALSE) {
         // jump to the error code below this do-while loop
         break;
      }
      
      // parse the IEEE802.15.4 header (RX DATA)
 ieee802154_retrieveHeader(self, (self->ieee154e_vars).dataReceived,&ieee802514_header);
      
      // break if invalid IEEE802.15.4 header
      if (ieee802514_header.valid==FALSE) {
         // break from the do-while loop and execute the clean-up code below
         break;
      }
      
      // store header details in packet buffer
      (self->ieee154e_vars).dataReceived->l2_frameType      = ieee802514_header.frameType;
      (self->ieee154e_vars).dataReceived->l2_dsn            = ieee802514_header.dsn;
      (self->ieee154e_vars).dataReceived->l2_IEListPresent  = ieee802514_header.ieListPresent;
      memcpy(&((self->ieee154e_vars).dataReceived->l2_nextORpreviousHop),&(ieee802514_header.src),sizeof(open_addr_t));
      
      // toss the IEEE802.15.4 header
 packetfunctions_tossHeader(self, (self->ieee154e_vars).dataReceived,ieee802514_header.headerLength);
      
      // handle IEs xv poipoi
      // reset join priority 
      // retrieve IE in sixtop
      if ((ieee802514_header.valid==TRUE &&
          ieee802514_header.ieListPresent==TRUE && 
          ieee802514_header.frameType==IEEE154_TYPE_BEACON && // if it is not a beacon and have ie, the ie will be processed in sixtop
 packetfunctions_sameAddress(self, &ieee802514_header.panid, idmanager_getMyID(self, ADDR_PANID)) && 
 ieee154e_processIEs(self, (self->ieee154e_vars).dataReceived,&lenIE))==FALSE) {
          //log  that the packet is not carrying IEs
      }
      
      // toss the IEs including Synch
 packetfunctions_tossHeader(self, (self->ieee154e_vars).dataReceived,lenIE);
            
      // record the captured time
      (self->ieee154e_vars).lastCapturedTime = capturedTime;
      
      // if I just received an invalid frame, stop
      if ( isValidRxFrame(self, &ieee802514_header)==FALSE) {
         // jump to the error code below this do-while loop
         break;
      }
      
      // check if ack requested
      if (ieee802514_header.ackRequested==1) {
         // arm rt5
 radiotimer_schedule(self, DURATION_rt5);
      } else {
         // synchronize to the received packet iif I'm not a DAGroot and this is my preferred parent
         if ( idmanager_getIsDAGroot(self)==FALSE && neighbors_isPreferredParent(self, &((self->ieee154e_vars).dataReceived->l2_nextORpreviousHop))) {
 synchronizePacket(self, (self->ieee154e_vars).syncCapturedTime);
         }
         // indicate reception to upper layer (no ACK asked)
 notif_receive(self, (self->ieee154e_vars).dataReceived);
         // reset local variable
         (self->ieee154e_vars).dataReceived = NULL;
         // abort
 endSlot(self);
      }
      
      // everything went well, return here not to execute the error code below
      return;
      
   } while(0);
   
   // free the (invalid) received data so RAM memory can be recycled
 openqueue_freePacketBuffer(self, (self->ieee154e_vars).dataReceived);
   
   // clear local variable
   (self->ieee154e_vars).dataReceived = NULL;
   
   // abort
 endSlot(self);
}

port_INLINE void activity_ri6(OpenMote* self) {
   PORT_SIGNED_INT_WIDTH timeCorrection;
   header_IE_ht header_desc;
   
   // change state
 changeState(self, S_TXACKPREPARE);
   
   // get a buffer to put the ack to send in
   (self->ieee154e_vars).ackToSend = openqueue_getFreePacketBuffer(self, COMPONENT_IEEE802154E);
   if ((self->ieee154e_vars).ackToSend==NULL) {
      // log the error
 openserial_printError(self, COMPONENT_IEEE802154E,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      // indicate we received a packet anyway (we don't want to loose any)
 notif_receive(self, (self->ieee154e_vars).dataReceived);
      // free local variable
      (self->ieee154e_vars).dataReceived = NULL;
      // abort
 endSlot(self);
      return;
   }
   
   // declare ownership over that packet
   (self->ieee154e_vars).ackToSend->creator = COMPONENT_IEEE802154E;
   (self->ieee154e_vars).ackToSend->owner   = COMPONENT_IEEE802154E;
   
   // calculate the time timeCorrection (this is the time when the packet arrive w.r.t the time it should be.
   timeCorrection = (PORT_SIGNED_INT_WIDTH)((PORT_SIGNED_INT_WIDTH)(self->ieee154e_vars).syncCapturedTime-(PORT_SIGNED_INT_WIDTH)TsTxOffset);
    
   // add the payload to the ACK (i.e. the timeCorrection)
 packetfunctions_reserveHeaderSize(self, (self->ieee154e_vars).ackToSend,sizeof(timecorrection_IE_ht));
   timeCorrection  = -timeCorrection;
   timeCorrection *= US_PER_TICK;
   (self->ieee154e_vars).ackToSend->payload[0] = (uint8_t)((((uint16_t)timeCorrection)   ) & 0xff);
   (self->ieee154e_vars).ackToSend->payload[1] = (uint8_t)((((uint16_t)timeCorrection)>>8) & 0xff);
   
   // add header IE header -- xv poipoi -- pkt is filled in reverse order..
 packetfunctions_reserveHeaderSize(self, (self->ieee154e_vars).ackToSend,sizeof(header_IE_ht));
   //create the header for ack IE
   header_desc.length_elementid_type=(sizeof(timecorrection_IE_ht)<< IEEE802154E_DESC_LEN_HEADER_IE_SHIFT)|
                                     (IEEE802154E_ACK_NACK_TIMECORRECTION_ELEMENTID << IEEE802154E_DESC_ELEMENTID_HEADER_IE_SHIFT)|
                                     IEEE802154E_DESC_TYPE_SHORT; 
   memcpy((self->ieee154e_vars).ackToSend->payload,&header_desc,sizeof(header_IE_ht));
   
   //START OF TELEMATICS CODE
      (self->ieee154e_vars).ackToSend->l2_security = TRUE;
      (self->ieee154e_vars).ackToSend->l2_securityLevel = 5;
      (self->ieee154e_vars).ackToSend->l2_keyIdMode = 3;
      if( idmanager_getIsDAGroot(self)){
       open_addr_t* temp_addr;
       temp_addr = idmanager_getMyID(self, ADDR_64B);
       memcpy(&((self->ieee154e_vars).ackToSend->l2_keySource), temp_addr, sizeof(open_addr_t));
      }else{
 neighbors_getPreferredParentEui64(self, &((self->ieee154e_vars).ackToSend->l2_keySource));
      }
      (self->ieee154e_vars).ackToSend->l2_keyIndex = 1;
      //END OF TELEMATICS CODE

   // openserial_printError(self, COMPONENT_SIXTOP,ERR_OK,
   //    						(errorparameter_t)(self->ieee154e_vars).ackToSend->payload[14],
   //    						(errorparameter_t)501);
   // openserial_printError(self, COMPONENT_SIXTOP,ERR_OK,
   //    						(errorparameter_t)(self->ieee154e_vars).ackToSend->payload[15],
   //    						(errorparameter_t)502);

   // prepend the IEEE802.15.4 header to the ACK
   (self->ieee154e_vars).ackToSend->l2_frameType = IEEE154_TYPE_ACK;
   (self->ieee154e_vars).ackToSend->l2_dsn       = (self->ieee154e_vars).dataReceived->l2_dsn;

   //START OF TELEMATICS CODE
      if((self->ieee154e_vars).ackToSend->l2_security == IEEE154_SEC_YES_SECURITY){

       	   security_outgoingFrame((self->ieee154e_vars).ackToSend,
       			   	   	   	   	  (self->ieee154e_vars).ackToSend->l2_securityLevel,
       			   	   	          (self->ieee154e_vars).ackToSend->l2_keyIdMode,
       			   	   	          &(self->ieee154e_vars).ackToSend->l2_keySource,
       			   	   	          (self->ieee154e_vars).ackToSend->l2_keyIndex);
          }
      //END OF TELEMATICS CODE
 ieee802154_prependHeader(self, (self->ieee154e_vars).ackToSend,
                            (self->ieee154e_vars).ackToSend->l2_frameType,
                            IEEE154_IELIST_YES,//ie in ack
                            IEEE154_FRAMEVERSION,//enhanced ack
                            //START OF TELEMATICS CODE
							//IEEE154_SEC_NO_SECURITY,
							(self->ieee154e_vars).ackToSend->l2_security,
							//END OF TELEMATICS CODE
                            (self->ieee154e_vars).dataReceived->l2_dsn,
                            &((self->ieee154e_vars).dataReceived->l2_nextORpreviousHop)
                            );
   
   // space for 2-byte CRC
 packetfunctions_reserveFooterSize(self, (self->ieee154e_vars).ackToSend,2);
   
    // calculate the frequency to transmit on
   (self->ieee154e_vars).freq = calculateFrequency(self, schedule_getChannelOffset(self)); 
   
   // configure the radio for that frequency
 radio_setFrequency(self, (self->ieee154e_vars).freq);
   
   // load the packet in the radio's Tx buffer
 radio_loadPacket(self, (self->ieee154e_vars).ackToSend->payload,
                    (self->ieee154e_vars).ackToSend->length);
   
   // enable the radio in Tx mode. This does not send that packet.
 radio_txEnable(self);
   (self->ieee154e_vars).radioOnInit= radio_getTimerValue(self);
   (self->ieee154e_vars).radioOnThisSlot=TRUE;
   // arm rt6
 radiotimer_schedule(self, DURATION_rt6);
   
   // change state
 changeState(self, S_TXACKREADY);
}

port_INLINE void activity_rie4(OpenMote* self) {
   // log the error
 openserial_printError(self, COMPONENT_IEEE802154E,ERR_MAXTXACKPREPARE_OVERFLOWS,
                         (errorparameter_t)(self->ieee154e_vars).state,
                         (errorparameter_t)(self->ieee154e_vars).slotOffset);
   
   // abort
 endSlot(self);
}

port_INLINE void activity_ri7(OpenMote* self) {
   // change state
 changeState(self, S_TXACKDELAY);
   
   // arm rt7
 radiotimer_schedule(self, DURATION_rt7);
   
   // give the 'go' to transmit
 radio_txNow(self); 
}

port_INLINE void activity_rie5(OpenMote* self) {
   // log the error
 openserial_printError(self, COMPONENT_IEEE802154E,ERR_WDRADIOTX_OVERFLOWS,
                         (errorparameter_t)(self->ieee154e_vars).state,
                         (errorparameter_t)(self->ieee154e_vars).slotOffset);
   
   // abort
 endSlot(self);
}

port_INLINE void activity_ri8(OpenMote* self, PORT_RADIOTIMER_WIDTH capturedTime) {
   // change state
 changeState(self, S_TXACK);
   
   // cancel rt7
 radiotimer_cancel(self);
   
   // record the captured time
   (self->ieee154e_vars).lastCapturedTime = capturedTime;
   
   // arm rt8
 radiotimer_schedule(self, DURATION_rt8);
}

port_INLINE void activity_rie6(OpenMote* self) {
   // log the error
 openserial_printError(self, COMPONENT_IEEE802154E,ERR_WDACKDURATION_OVERFLOWS,
                         (errorparameter_t)(self->ieee154e_vars).state,
                         (errorparameter_t)(self->ieee154e_vars).slotOffset);
   
   // abort
 endSlot(self);
}

port_INLINE void activity_ri9(OpenMote* self, PORT_RADIOTIMER_WIDTH capturedTime) {
   // change state
 changeState(self, S_RXPROC);
   
   // cancel rt8
 radiotimer_cancel(self);
  
   // record the captured time
   (self->ieee154e_vars).lastCapturedTime = capturedTime;
   
   // free the ack we just sent so corresponding RAM memory can be recycled
 openqueue_freePacketBuffer(self, (self->ieee154e_vars).ackToSend);
   
   // clear local variable
   (self->ieee154e_vars).ackToSend = NULL;
   
   // synchronize to the received packet
   if ( idmanager_getIsDAGroot(self)==FALSE && neighbors_isPreferredParent(self, &((self->ieee154e_vars).dataReceived->l2_nextORpreviousHop))) {
 synchronizePacket(self, (self->ieee154e_vars).syncCapturedTime);
   }
   
   // inform upper layer of reception (after ACK sent)
 notif_receive(self, (self->ieee154e_vars).dataReceived);
   
   // clear local variable
   (self->ieee154e_vars).dataReceived = NULL;
   
   // official end of Rx slot
 endSlot(self);
}

//======= frame validity check

/**
\brief Decides whether the packet I just received is valid received frame.

A valid Rx frame satisfies the following constraints:
- its IEEE802.15.4 header is well formatted
- it's a DATA of BEACON frame (i.e. not ACK and not COMMAND)
- it's sent on the same PANid as mine
- it's for me (unicast or broadcast)

\param[in] ieee802514_header IEEE802.15.4 header of the packet I just received

\returns TRUE if packet is valid received frame, FALSE otherwise
*/
port_INLINE bool isValidRxFrame(OpenMote* self, ieee802154_header_iht* ieee802514_header) {
   return ieee802514_header->valid==TRUE                                                           && \
          (
             ieee802514_header->frameType==IEEE154_TYPE_DATA                   ||
             ieee802514_header->frameType==IEEE154_TYPE_BEACON
          )                                                                                        && \
 packetfunctions_sameAddress(self, &ieee802514_header->panid, idmanager_getMyID(self, ADDR_PANID))     && \
          (
 idmanager_isMyAddress(self, &ieee802514_header->dest)                   ||
 packetfunctions_isBroadcastMulticast(self, &ieee802514_header->dest)
          );
}

/**
\brief Decides whether the packet I just received is a valid ACK.

A packet is a valid ACK if it satisfies the following conditions:
- the IEEE802.15.4 header is valid
- the frame type is 'ACK'
- the sequence number in the ACK matches the sequence number of the packet sent
- the ACK contains my PANid
- the packet is unicast to me
- the packet comes from the neighbor I sent the data to

\param[in] ieee802514_header IEEE802.15.4 header of the packet I just received
\param[in] packetSent points to the packet I just sent

\returns TRUE if packet is a valid ACK, FALSE otherwise.
*/
port_INLINE bool isValidAck(OpenMote* self, ieee802154_header_iht* ieee802514_header, OpenQueueEntry_t* packetSent) {
   /*
   return ieee802514_header->valid==TRUE                                                           && \
          ieee802514_header->frameType==IEEE154_TYPE_ACK                                           && \
          ieee802514_header->dsn==packetSent->l2_dsn                                               && \
 packetfunctions_sameAddress(self, &ieee802514_header->panid, idmanager_getMyID(self, ADDR_PANID))     && \
 idmanager_isMyAddress(self, &ieee802514_header->dest)                                          && \
 packetfunctions_sameAddress(self, &ieee802514_header->src,&packetSent->l2_nextORpreviousHop);
   */
   // poipoi don't check for seq num
   return ieee802514_header->valid==TRUE                                                           && \
          ieee802514_header->frameType==IEEE154_TYPE_ACK                                           && \
 packetfunctions_sameAddress(self, &ieee802514_header->panid, idmanager_getMyID(self, ADDR_PANID))     && \
 idmanager_isMyAddress(self, &ieee802514_header->dest)                                          && \
 packetfunctions_sameAddress(self, &ieee802514_header->src,&packetSent->l2_nextORpreviousHop);
}

//======= ASN handling

port_INLINE void incrementAsnOffset(OpenMote* self) {
   // increment the asn
   (self->ieee154e_vars).asn.bytes0and1++;
   if ((self->ieee154e_vars).asn.bytes0and1==0) {
      (self->ieee154e_vars).asn.bytes2and3++;
      if ((self->ieee154e_vars).asn.bytes2and3==0) {
         (self->ieee154e_vars).asn.byte4++;
      }
   }
   // increment the offsets
   (self->ieee154e_vars).slotOffset  = ((self->ieee154e_vars).slotOffset+1)% schedule_getFrameLength(self);
   (self->ieee154e_vars).asnOffset   = ((self->ieee154e_vars).asnOffset+1)%16;
}

//from upper layer that want to send the ASN to compute timing or latency
port_INLINE void ieee154e_getAsn(OpenMote* self, uint8_t* array) {
   array[0]         = ((self->ieee154e_vars).asn.bytes0and1     & 0xff);
   array[1]         = ((self->ieee154e_vars).asn.bytes0and1/256 & 0xff);
   array[2]         = ((self->ieee154e_vars).asn.bytes2and3     & 0xff);
   array[3]         = ((self->ieee154e_vars).asn.bytes2and3/256 & 0xff);
   array[4]         =  (self->ieee154e_vars).asn.byte4;
}

port_INLINE void joinPriorityStoreFromAdv(OpenMote* self, uint8_t jp){
  (self->ieee154e_vars).dataReceived->l2_joinPriority = jp;
  (self->ieee154e_vars).dataReceived->l2_joinPriorityPresent = TRUE;     
}


port_INLINE void asnStoreFromAdv(OpenMote* self, uint8_t* asn) {
   
   // store the ASN
   (self->ieee154e_vars).asn.bytes0and1   =     asn[0]+
                                    256*asn[1];
   (self->ieee154e_vars).asn.bytes2and3   =     asn[2]+
                                    256*asn[3];
   (self->ieee154e_vars).asn.byte4        =     asn[4];
   
   // determine the current slotOffset
   /*
   Note: this is a bit of a hack. Normally, slotOffset=ASN%slotlength. But since
   the ADV is exchanged in slot 0, we know that we're currently at slotOffset==0
   */
   (self->ieee154e_vars).slotOffset       = 0;
 schedule_syncSlotOffset(self, (self->ieee154e_vars).slotOffset);
   (self->ieee154e_vars).nextActiveSlotOffset = schedule_getNextActiveSlotOffset(self);
   
   /* 
   infer the asnOffset based on the fact that
   (self->ieee154e_vars).freq = 11 + (asnOffset + channelOffset)%16 
   */
   (self->ieee154e_vars).asnOffset = (self->ieee154e_vars).freq - 11 - schedule_getChannelOffset(self);
}

//======= synchronization

void synchronizePacket(OpenMote* self, PORT_RADIOTIMER_WIDTH timeReceived) {
   PORT_SIGNED_INT_WIDTH timeCorrection;
   PORT_RADIOTIMER_WIDTH newPeriod;
   PORT_RADIOTIMER_WIDTH currentValue;
   PORT_RADIOTIMER_WIDTH currentPeriod;
   
   // record the current timer value and period
   currentValue                   = radio_getTimerValue(self);
   currentPeriod                  = radio_getTimerPeriod(self);
   
   // calculate new period
   timeCorrection                 =  (PORT_SIGNED_INT_WIDTH)((PORT_SIGNED_INT_WIDTH)timeReceived-(PORT_SIGNED_INT_WIDTH)TsTxOffset);

   newPeriod                      =  TsSlotDuration;
   
   // detect whether I'm too close to the edge of the slot, in that case,
   // skip a slot and increase the temporary slot length to be 2 slots long
   if (currentValue<timeReceived || currentPeriod-currentValue<RESYNCHRONIZATIONGUARD) {
      newPeriod                  +=  TsSlotDuration;
 incrementAsnOffset(self);
   }
   newPeriod                      =  (PORT_RADIOTIMER_WIDTH)((PORT_SIGNED_INT_WIDTH)newPeriod+timeCorrection);
   
   // resynchronize by applying the new period
 radio_setTimerPeriod(self, newPeriod);
   
   // reset the de-synchronization timeout
   (self->ieee154e_vars).deSyncTimeout    = DESYNCTIMEOUT;
   
   // log a large timeCorrection
   if (
         (self->ieee154e_vars).isSync==TRUE &&
         (
            timeCorrection<-LIMITLARGETIMECORRECTION ||
            timeCorrection> LIMITLARGETIMECORRECTION
         )
      ) {
 openserial_printError(self, COMPONENT_IEEE802154E,ERR_LARGE_TIMECORRECTION,
                            (errorparameter_t)timeCorrection,
                            (errorparameter_t)0);
   }
   
   // update the stats
   (self->ieee154e_stats).numSyncPkt++;
 updateStats(self, timeCorrection);

 adaptive_sync_preprocess(self, timeCorrection, (self->ieee154e_vars).dataReceived->l2_nextORpreviousHop);
   
#ifdef OPENSIM
 debugpins_syncPacket_set(self);
 debugpins_syncPacket_clr(self);
#endif
}

void synchronizeAck(OpenMote* self, PORT_SIGNED_INT_WIDTH timeCorrection) {
   PORT_RADIOTIMER_WIDTH newPeriod;
   PORT_RADIOTIMER_WIDTH currentPeriod;
   
   // calculate new period
   currentPeriod                  = radio_getTimerPeriod(self);
   newPeriod                      =  (PORT_RADIOTIMER_WIDTH)((PORT_SIGNED_INT_WIDTH)currentPeriod-timeCorrection);

   // resynchronize by applying the new period
 radio_setTimerPeriod(self, newPeriod);
   // reset the de-synchronization timeout
   (self->ieee154e_vars).deSyncTimeout    = DESYNCTIMEOUT;
   // log a large timeCorrection
   if (
         (self->ieee154e_vars).isSync==TRUE &&
         (
            timeCorrection<-LIMITLARGETIMECORRECTION ||
            timeCorrection> LIMITLARGETIMECORRECTION
         )
      ) {
 openserial_printError(self, COMPONENT_IEEE802154E,ERR_LARGE_TIMECORRECTION,
                            (errorparameter_t)timeCorrection,
                            (errorparameter_t)1);
   }
   // update the stats
   (self->ieee154e_stats).numSyncAck++;
 updateStats(self, timeCorrection);

   // update last asn when need sync.
 adaptive_sync_preprocess(self, (-timeCorrection), (self->ieee154e_vars).ackReceived->l2_nextORpreviousHop);
   
#ifdef OPENSIM
 debugpins_syncAck_set(self);
 debugpins_syncAck_clr(self);
#endif
}

void changeIsSync(OpenMote* self, bool newIsSync) {
   (self->ieee154e_vars).isSync = newIsSync;
   
   if ((self->ieee154e_vars).isSync==TRUE) {
 leds_sync_on(self);
 resetStats(self);
   } else {
 leds_sync_off(self);
 schedule_resetBackoff(self);
   }
}

//======= notifying upper layer

void notif_sendDone(OpenMote* self, OpenQueueEntry_t* packetSent, owerror_t error) {
   // record the outcome of the trasmission attempt
   packetSent->l2_sendDoneError   = error;
   // record the current ASN
   memcpy(&packetSent->l2_asn,&(self->ieee154e_vars).asn,sizeof(asn_t));
   // associate this packet with the virtual component
   // COMPONENT_IEEE802154E_TO_RES so RES can knows it's for it
   packetSent->owner              = COMPONENT_IEEE802154E_TO_SIXTOP;
   // post RES's sendDone task
 scheduler_push_task(self, task_sixtopNotifSendDone,TASKPRIO_SIXTOP_NOTIF_TXDONE);
   // wake up the scheduler
   SCHEDULER_WAKEUP();
}

void notif_receive(OpenMote* self, OpenQueueEntry_t* packetReceived) {
   // record the current ASN
   memcpy(&packetReceived->l2_asn, &(self->ieee154e_vars).asn, sizeof(asn_t));
   // indicate reception to the schedule, to keep statistics
 schedule_indicateRx(self, &packetReceived->l2_asn);
   // associate this packet with the virtual component
   // COMPONENT_IEEE802154E_TO_SIXTOP so sixtop can knows it's for it
   packetReceived->owner          = COMPONENT_IEEE802154E_TO_SIXTOP;

   // post RES's Receive task
 scheduler_push_task(self, task_sixtopNotifReceive,TASKPRIO_SIXTOP_NOTIF_RX);
   // wake up the scheduler
   SCHEDULER_WAKEUP();
}

//======= stats

port_INLINE void resetStats(OpenMote* self) {
   (self->ieee154e_stats).numSyncPkt      =    0;
   (self->ieee154e_stats).numSyncAck      =    0;
   (self->ieee154e_stats).minCorrection   =  127;
   (self->ieee154e_stats).maxCorrection   = -127;
   (self->ieee154e_stats).numTicsOn       =    0;
   (self->ieee154e_stats).numTicsTotal    =    0;
   // do not reset the number of de-synchronizations
}

void updateStats(OpenMote* self, PORT_SIGNED_INT_WIDTH timeCorrection) {
   // update minCorrection
   if (timeCorrection<(self->ieee154e_stats).minCorrection) {
     (self->ieee154e_stats).minCorrection = timeCorrection;
   }
   // update maxConnection
   if(timeCorrection>(self->ieee154e_stats).maxCorrection) {
     (self->ieee154e_stats).maxCorrection = timeCorrection;
   }
}

//======= misc

/**
\brief Calculates the frequency channel to transmit on, based on the 
absolute slot number and the channel offset of the requested slot.

During normal operation, the frequency used is a function of the 
channelOffset indicating in the schedule, and of the ASN of the
slot. This ensures channel hopping, consecutive packets sent in the same slot
in the schedule are done on a difference frequency channel.

During development, you can force single channel operation by having this
function return a constant channel number (between 11 and 26). This allows you
to use a single-channel sniffer; but you can not schedule two links on two
different channel offsets in the same slot.

\param[in] channelOffset channel offset for the current slot

\returns The calculated frequency channel, an integer between 11 and 26.
*/
port_INLINE uint8_t calculateFrequency(OpenMote* self, uint8_t channelOffset) {
   // comment the following line out to disable channel hopping
   return SYNCHRONIZING_CHANNEL; // single channel
   //return 11+((self->ieee154e_vars).asnOffset+channelOffset)%16; //channel hopping
}

/**
\brief Changes the state of the IEEE802.15.4e FSM.

Besides simply updating the state global variable,
this function toggles the FSM debug pin.

\param[in] newstate The state the IEEE802.15.4e FSM is now in.
*/
void changeState(OpenMote* self, ieee154e_state_t newstate) {
   // update the state
   (self->ieee154e_vars).state = newstate;
   // wiggle the FSM debug pin
   switch ((self->ieee154e_vars).state) {
      case S_SYNCLISTEN:
      case S_TXDATAOFFSET:
 debugpins_fsm_set(self);
         break;
      case S_SLEEP:
      case S_RXDATAOFFSET:
 debugpins_fsm_clr(self);
         break;
      case S_SYNCRX:
      case S_SYNCPROC:
      case S_TXDATAPREPARE:
      case S_TXDATAREADY:
      case S_TXDATADELAY:
      case S_TXDATA:
      case S_RXACKOFFSET:
      case S_RXACKPREPARE:
      case S_RXACKREADY:
      case S_RXACKLISTEN:
      case S_RXACK:
      case S_TXPROC:
      case S_RXDATAPREPARE:
      case S_RXDATAREADY:
      case S_RXDATALISTEN:
      case S_RXDATA:
      case S_TXACKOFFSET:
      case S_TXACKPREPARE:
      case S_TXACKREADY:
      case S_TXACKDELAY:
      case S_TXACK:
      case S_RXPROC:
 debugpins_fsm_toggle(self);
         break;
   }
}

/**
\brief Housekeeping tasks to do at the end of each slot.

This functions is called once in each slot, when there is nothing more
to do. This might be when an error occured, or when everything went well.
This function resets the state of the FSM so it is ready for the next slot.

Note that by the time this function is called, any received packet should already
have been sent to the upper layer. Similarly, in a Tx slot, the sendDone
function should already have been done. If this is not the case, this function
will do that for you, but assume that something went wrong.
*/
void endSlot(OpenMote* self) {
  
   // turn off the radio
 radio_rfOff(self);
   // compute the duty cycle if radio has been turned on
   if ((self->ieee154e_vars).radioOnThisSlot==TRUE){  
      (self->ieee154e_vars).radioOnTics+=( radio_getTimerValue(self)-(self->ieee154e_vars).radioOnInit);
   }
   // clear any pending timer
 radiotimer_cancel(self);
   
   // reset capturedTimes
   (self->ieee154e_vars).lastCapturedTime = 0;
   (self->ieee154e_vars).syncCapturedTime = 0;
   
   //computing duty cycle.
   (self->ieee154e_stats).numTicsOn+=(self->ieee154e_vars).radioOnTics;//accumulate and tics the radio is on for that window
   (self->ieee154e_stats).numTicsTotal+= radio_getTimerPeriod(self);//increment total tics by timer period.

   if ((self->ieee154e_stats).numTicsTotal>DUTY_CYCLE_WINDOW_LIMIT){
      (self->ieee154e_stats).numTicsTotal = (self->ieee154e_stats).numTicsTotal>>1;
      (self->ieee154e_stats).numTicsOn    = (self->ieee154e_stats).numTicsOn>>1;
   }

   //clear vars for duty cycle on this slot   
   (self->ieee154e_vars).radioOnTics=0;
   (self->ieee154e_vars).radioOnThisSlot=FALSE;
   
   // clean up dataToSend
   if ((self->ieee154e_vars).dataToSend!=NULL) {
      // if everything went well, dataToSend was set to NULL in ti9
      // getting here means transmit failed
      
      // indicate Tx fail to schedule to update stats
 schedule_indicateTx(self, &(self->ieee154e_vars).asn,FALSE);
      
      //decrement transmits left counter
      (self->ieee154e_vars).dataToSend->l2_retriesLeft--;
      
      if ((self->ieee154e_vars).dataToSend->l2_retriesLeft==0) {
         // indicate tx fail if no more retries left
 notif_sendDone(self, (self->ieee154e_vars).dataToSend,E_FAIL);
      } else {
         // return packet to the virtual COMPONENT_SIXTOP_TO_IEEE802154E component
         (self->ieee154e_vars).dataToSend->owner = COMPONENT_SIXTOP_TO_IEEE802154E;
      }
      
      // reset local variable
      (self->ieee154e_vars).dataToSend = NULL;
   }
   
   // clean up dataReceived
   if ((self->ieee154e_vars).dataReceived!=NULL) {
      // assume something went wrong. If everything went well, dataReceived
      // would have been set to NULL in ri9.
      // indicate  "received packet" to upper layer since we don't want to loose packets
 notif_receive(self, (self->ieee154e_vars).dataReceived);
      // reset local variable
      (self->ieee154e_vars).dataReceived = NULL;
   }
   
   // clean up ackToSend
   if ((self->ieee154e_vars).ackToSend!=NULL) {
      // free ackToSend so corresponding RAM memory can be recycled
 openqueue_freePacketBuffer(self, (self->ieee154e_vars).ackToSend);
      // reset local variable
      (self->ieee154e_vars).ackToSend = NULL;
   }
   
   // clean up ackReceived
   if ((self->ieee154e_vars).ackReceived!=NULL) {
      // free ackReceived so corresponding RAM memory can be recycled
 openqueue_freePacketBuffer(self, (self->ieee154e_vars).ackReceived);
      // reset local variable
      (self->ieee154e_vars).ackReceived = NULL;
   }
   
   
   // change state
 changeState(self, S_SLEEP);
}

bool ieee154e_isSynch(OpenMote* self){
   return (self->ieee154e_vars).isSync;
}
