#include "opendefs.h"
#include "IEEE802154E.h"
#include "radio.h"
#include "radiotimer.h"
#include "IEEE802154.h"
#include "ieee802154_security_driver.h"
#include "openqueue.h"
#include "idmanager.h"
#include "openserial.h"
#include "schedule.h"
#include "packetfunctions.h"
#include "scheduler.h"
#include "leds.h"
#include "neighbors.h"
#include "debugpins.h"
#include "sixtop.h"
#include "adaptive_sync.h"
#include "processIE.h"
#include "openrandom.h"

//=========================== variables =======================================

ieee154e_vars_t    ieee154e_vars;
ieee154e_stats_t   ieee154e_stats;
ieee154e_dbg_t     ieee154e_dbg;

//=========================== prototypes ======================================

// SYNCHRONIZING
void     activity_synchronize_newSlot(void);
void     activity_synchronize_startOfFrame(PORT_RADIOTIMER_WIDTH capturedTime);
void     activity_synchronize_endOfFrame(PORT_RADIOTIMER_WIDTH capturedTime);
// TX
void     activity_ti1ORri1(void);
void     activity_ti2(void);
void     activity_tie1(void);
void     activity_ti3(void);
void     activity_tie2(void);
void     activity_ti4(PORT_RADIOTIMER_WIDTH capturedTime);
void     activity_tie3(void);
void     activity_ti5(PORT_RADIOTIMER_WIDTH capturedTime);
void     activity_ti6(void);
void     activity_tie4(void);
void     activity_ti7(void);
void     activity_tie5(void);
void     activity_ti8(PORT_RADIOTIMER_WIDTH capturedTime);
void     activity_tie6(void);
void     activity_ti9(PORT_RADIOTIMER_WIDTH capturedTime);
// RX
void     activity_ri2(void);
void     activity_rie1(void);
void     activity_ri3(void);
void     activity_rie2(void);
void     activity_ri4(PORT_RADIOTIMER_WIDTH capturedTime);
void     activity_rie3(void);
void     activity_ri5(PORT_RADIOTIMER_WIDTH capturedTime);
void     activity_ri6(void);
void     activity_rie4(void);
void     activity_ri7(void);
void     activity_rie5(void);
void     activity_ri8(PORT_RADIOTIMER_WIDTH capturedTime);
void     activity_rie6(void);
void     activity_ri9(PORT_RADIOTIMER_WIDTH capturedTime);

// frame validity check
bool     isValidRxFrame(ieee802154_header_iht* ieee802514_header);
bool     isValidAck(ieee802154_header_iht*     ieee802514_header,
                    OpenQueueEntry_t*          packetSent);
bool     isValidJoin(OpenQueueEntry_t* eb, ieee802154_header_iht *parsedHeader); 
// IEs Handling
bool     ieee154e_processIEs(OpenQueueEntry_t* pkt, uint16_t* lenIE);
// ASN handling
void     incrementAsnOffset(void);
void     ieee154e_syncSlotOffset(void);
void     asnStoreFromEB(uint8_t* asn);
void     joinPriorityStoreFromEB(uint8_t jp);

// timeslot template handling
void     timeslotTemplateIDStoreFromEB(uint8_t id);
// channelhopping template handling
void     channelhoppingTemplateIDStoreFromEB(uint8_t id);
// synchronization
void     synchronizePacket(PORT_RADIOTIMER_WIDTH timeReceived);
void     synchronizeAck(PORT_SIGNED_INT_WIDTH timeCorrection);
void     changeIsSync(bool newIsSync);
// notifying upper layer
void     notif_sendDone(OpenQueueEntry_t* packetSent, owerror_t error);
void     notif_receive(OpenQueueEntry_t* packetReceived);
// statistics
void     resetStats(void);
void     updateStats(PORT_SIGNED_INT_WIDTH timeCorrection);
// misc
uint8_t  calculateFrequency(uint8_t channelOffset);
void     changeState(ieee154e_state_t newstate);
void     endSlot(void);
bool     debugPrint_asn(void);
bool     debugPrint_isSync(void);
void     ieee154e_sendEB(void);
// interrupts
void     isr_ieee154e_newSlot(void);
void     isr_ieee154e_timer(void);

//=========================== admin ===========================================

/**
\brief This function initializes this module.

Call this function once before any other function in this module, possibly
during boot-up.
*/
void ieee154e_init() {
   
   // initialize variables
   memset(&ieee154e_vars,0,sizeof(ieee154e_vars_t));
   memset(&ieee154e_dbg,0,sizeof(ieee154e_dbg_t));
   
   ieee154e_vars.singleChannel     = SYNCHRONIZING_CHANNEL;
   ieee154e_vars.isAckEnabled      = TRUE;
   ieee154e_vars.isSecurityEnabled = FALSE;
   // default hopping template
   memcpy(
       &(ieee154e_vars.chTemplate[0]),
       chTemplate_default,
       sizeof(ieee154e_vars.chTemplate)
   );
   
   if (idmanager_getIsDAGroot()==TRUE) {
      changeIsSync(TRUE);
   } else {
      changeIsSync(FALSE);
   }
   
   resetStats();
   ieee154e_stats.numDeSync                 = 0;
   
   // switch radio on
   radio_rfOn();
   
   // set callback functions for the radio
   radio_setOverflowCb(isr_ieee154e_newSlot);
   radio_setCompareCb(isr_ieee154e_timer);
   radio_setStartFrameCb(ieee154e_startOfFrame);
   radio_setEndFrameCb(ieee154e_endOfFrame);
   // have the radio start its timer
   radio_startTimer(TsSlotDuration);
}

//=========================== public ==========================================

/**
/brief Difference between some older ASN and the current ASN.

\param[in] someASN some ASN to compare to the current

\returns The ASN difference, or 0xffff if more than 65535 different
*/
PORT_RADIOTIMER_WIDTH ieee154e_asnDiff(asn_t* someASN) {
   PORT_RADIOTIMER_WIDTH diff;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   if (ieee154e_vars.asn.byte4 != someASN->byte4) {
      ENABLE_INTERRUPTS();
      return (PORT_RADIOTIMER_WIDTH)0xFFFFFFFF;;
   }
   
   diff = 0;
   if (ieee154e_vars.asn.bytes2and3 == someASN->bytes2and3) {
      ENABLE_INTERRUPTS();
      return ieee154e_vars.asn.bytes0and1-someASN->bytes0and1;
   } else if (ieee154e_vars.asn.bytes2and3-someASN->bytes2and3==1) {
      diff  = ieee154e_vars.asn.bytes0and1;
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
void isr_ieee154e_newSlot() {
   radio_setTimerPeriod(TsSlotDuration);
   if (ieee154e_vars.isSync==FALSE) {
      if (idmanager_getIsDAGroot()==TRUE) {
         changeIsSync(TRUE);
         incrementAsnOffset();
         ieee154e_syncSlotOffset();
         ieee154e_vars.nextActiveSlotOffset = schedule_getNextActiveSlotOffset();
      } else {
         activity_synchronize_newSlot();
      }
   } else {
#ifdef ADAPTIVE_SYNC
     // adaptive synchronization
      adaptive_sync_countCompensationTimeout();
#endif
      activity_ti1ORri1();
   }
   ieee154e_dbg.num_newSlot++;
}

/**
\brief Indicates the FSM timer has fired.

This function executes in ISR mode, when the FSM timer fires.
*/
void isr_ieee154e_timer() {
   switch (ieee154e_vars.state) {
      case S_TXDATAOFFSET:
         activity_ti2();
         break;
      case S_TXDATAPREPARE:
         activity_tie1();
         break;
      case S_TXDATAREADY:
         activity_ti3();
         break;
      case S_TXDATADELAY:
         activity_tie2();
         break;
      case S_TXDATA:
         activity_tie3();
         break;
      case S_RXACKOFFSET:
         activity_ti6();
         break;
      case S_RXACKPREPARE:
         activity_tie4();
         break;
      case S_RXACKREADY:
         activity_ti7();
         break;
      case S_RXACKLISTEN:
         activity_tie5();
         break;
      case S_RXACK:
         activity_tie6();
         break;
      case S_RXDATAOFFSET:
         activity_ri2(); 
         break;
      case S_RXDATAPREPARE:
         activity_rie1();
         break;
      case S_RXDATAREADY:
         activity_ri3();
         break;
      case S_RXDATALISTEN:
         activity_rie2();
         break;
      case S_RXDATA:
         activity_rie3();
         break;
      case S_TXACKOFFSET: 
         activity_ri6();
         break;
      case S_TXACKPREPARE:
         activity_rie4();
         break;
      case S_TXACKREADY:
         activity_ri7();
         break;
      case S_TXACKDELAY:
         activity_rie5();
         break;
      case S_TXACK:
         activity_rie6();
         break;
      default:
         // log the error
         openserial_printError(COMPONENT_IEEE802154E,ERR_WRONG_STATE_IN_TIMERFIRES,
                               (errorparameter_t)ieee154e_vars.state,
                               (errorparameter_t)ieee154e_vars.slotOffset);
         // abort
         endSlot();
         break;
   }
   ieee154e_dbg.num_timer++;
}

/**
\brief Indicates the radio just received the first byte of a packet.

This function executes in ISR mode.
*/
void ieee154e_startOfFrame(PORT_RADIOTIMER_WIDTH capturedTime) {
   if (ieee154e_vars.isSync==FALSE) {
     activity_synchronize_startOfFrame(capturedTime);
   } else {
      switch (ieee154e_vars.state) {
         case S_TXDATADELAY:   
            activity_ti4(capturedTime);
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
            activity_ti8(capturedTime);
            break;
         case S_RXDATAREADY:
            /*
            Similarly as above.
            */
            // no break!
         case S_RXDATALISTEN:
            activity_ri4(capturedTime);
            break;
         case S_TXACKDELAY:
            activity_ri8(capturedTime);
            break;
         default:
            // log the error
            openserial_printError(COMPONENT_IEEE802154E,ERR_WRONG_STATE_IN_NEWSLOT,
                                  (errorparameter_t)ieee154e_vars.state,
                                  (errorparameter_t)ieee154e_vars.slotOffset);
            // abort
            endSlot();
            break;
      }
   }
   ieee154e_dbg.num_startOfFrame++;
}

/**
\brief Indicates the radio just received the last byte of a packet.

This function executes in ISR mode.
*/
void ieee154e_endOfFrame(PORT_RADIOTIMER_WIDTH capturedTime) {
   if (ieee154e_vars.isSync==FALSE) {
      activity_synchronize_endOfFrame(capturedTime);
   } else {
      switch (ieee154e_vars.state) {
         case S_TXDATA:
            activity_ti5(capturedTime);
            break;
         case S_RXACK:
            activity_ti9(capturedTime);
            break;
         case S_RXDATA:
            activity_ri5(capturedTime);
            break;
         case S_TXACK:
            activity_ri9(capturedTime);
            break;
         default:
            // log the error
            openserial_printError(COMPONENT_IEEE802154E,ERR_WRONG_STATE_IN_ENDOFFRAME,
                                  (errorparameter_t)ieee154e_vars.state,
                                  (errorparameter_t)ieee154e_vars.slotOffset);
            // abort
            endSlot();
            break;
      }
   }
   ieee154e_dbg.num_endOfFrame++;
}

//======= misc

/**
\brief Trigger this module to print status information, over serial.

debugPrint_* functions are used by the openserial module to continuously print
status information about several modules in the OpenWSN stack.

\returns TRUE if this function printed something, FALSE otherwise.
*/
bool debugPrint_asn() {
   asn_t output;
   output.byte4         =  ieee154e_vars.asn.byte4;
   output.bytes2and3    =  ieee154e_vars.asn.bytes2and3;
   output.bytes0and1    =  ieee154e_vars.asn.bytes0and1;
   openserial_printStatus(STATUS_ASN,(uint8_t*)&output,sizeof(output));
   return TRUE;
}

/**
\brief Trigger this module to print status information, over serial.

debugPrint_* functions are used by the openserial module to continuously print
status information about several modules in the OpenWSN stack.

\returns TRUE if this function printed something, FALSE otherwise.
*/
bool debugPrint_isSync() {
   uint8_t output=0;
   output = ieee154e_vars.isSync;
   openserial_printStatus(STATUS_ISSYNC,(uint8_t*)&output,sizeof(uint8_t));
   return TRUE;
}

/**
\brief Trigger this module to print status information, over serial.

debugPrint_* functions are used by the openserial module to continuously print
status information about several modules in the OpenWSN stack.

\returns TRUE if this function printed something, FALSE otherwise.
*/
bool debugPrint_macStats() {
   // send current stats over serial
   openserial_printStatus(STATUS_MACSTATS,(uint8_t*)&ieee154e_stats,sizeof(ieee154e_stats_t));
   return TRUE;
}

//=========================== private =========================================

//======= SYNCHRONIZING

port_INLINE void activity_synchronize_newSlot() {
   // I'm in the middle of receiving a packet
   if (ieee154e_vars.state==S_SYNCRX) {
      return;
   }
   
   // if this is the first time I call this function while not synchronized,
   // switch on the radio in Rx mode
   if (ieee154e_vars.state!=S_SYNCLISTEN) {
      // change state
      changeState(S_SYNCLISTEN);
      
      // turn off the radio (in case it wasn't yet)
      radio_rfOff();
      
      // configure the radio to listen to the default synchronizing channel
      radio_setFrequency(SYNCHRONIZING_CHANNEL);
      
      // update record of current channel
      ieee154e_vars.freq = SYNCHRONIZING_CHANNEL;
      
      // switch on the radio in Rx mode.
      radio_rxEnable();
      ieee154e_vars.radioOnInit=radio_getTimerValue();
      ieee154e_vars.radioOnThisSlot=TRUE;
      radio_rxNow();
   }
   
   // if I'm already in S_SYNCLISTEN, while not synchronized,
   // but the synchronizing channel has been changed,
   // change the synchronizing channel
   if ((ieee154e_vars.state==S_SYNCLISTEN) && (ieee154e_vars.singleChannelChanged == TRUE)) {
      // turn off the radio (in case it wasn't yet)
      radio_rfOff();
      
      // update record of current channel
      ieee154e_vars.freq = calculateFrequency(ieee154e_vars.singleChannel);
      
      // configure the radio to listen to the default synchronizing channel
      radio_setFrequency(ieee154e_vars.freq);
      
      // switch on the radio in Rx mode.
      radio_rxEnable();
      ieee154e_vars.radioOnInit=radio_getTimerValue();
      ieee154e_vars.radioOnThisSlot=TRUE;
      radio_rxNow();
      ieee154e_vars.singleChannelChanged = FALSE;
   }
   
   // increment ASN (used only to schedule serial activity)
   incrementAsnOffset();
   
   // to be able to receive and transmist serial even when not synchronized
   // take turns every 8 slots sending and receiving
   if        ((ieee154e_vars.asn.bytes0and1&0x000f)==0x0000) {
      openserial_stop();
      openserial_startOutput();
   } else if ((ieee154e_vars.asn.bytes0and1&0x000f)==0x0008) {
      openserial_stop();
      openserial_startInput();
   }
}

port_INLINE void activity_synchronize_startOfFrame(PORT_RADIOTIMER_WIDTH capturedTime) {
   
   // don't care about packet if I'm not listening
   if (ieee154e_vars.state!=S_SYNCLISTEN) {
      return;
   }
   
   // change state
   changeState(S_SYNCRX);
   
   // stop the serial
   openserial_stop();
   
   // record the captured time 
   ieee154e_vars.lastCapturedTime = capturedTime;
   
   // record the captured time (for sync)
   ieee154e_vars.syncCapturedTime = capturedTime;
}

port_INLINE void activity_synchronize_endOfFrame(PORT_RADIOTIMER_WIDTH capturedTime) {
   ieee802154_header_iht ieee802514_header;
   uint16_t              lenIE;
   
   // check state
   if (ieee154e_vars.state!=S_SYNCRX) {
      // log the error
      openserial_printError(COMPONENT_IEEE802154E,ERR_WRONG_STATE_IN_ENDFRAME_SYNC,
                            (errorparameter_t)ieee154e_vars.state,
                            (errorparameter_t)0);
      // abort
      endSlot();
   }
   
   // change state
   changeState(S_SYNCPROC);
   
   // get a buffer to put the (received) frame in
   ieee154e_vars.dataReceived = openqueue_getFreePacketBuffer(COMPONENT_IEEE802154E);
   if (ieee154e_vars.dataReceived==NULL) {
      // log the error
      openserial_printError(COMPONENT_IEEE802154E,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      // abort
      endSlot();
      return;
   }
   
   // declare ownership over that packet
   ieee154e_vars.dataReceived->creator = COMPONENT_IEEE802154E;
   ieee154e_vars.dataReceived->owner   = COMPONENT_IEEE802154E;
   
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
      ieee154e_vars.dataReceived->payload = &(ieee154e_vars.dataReceived->packet[FIRST_FRAME_BYTE]);
      radio_getReceivedFrame(       ieee154e_vars.dataReceived->payload,
                                   &ieee154e_vars.dataReceived->length,
                             sizeof(ieee154e_vars.dataReceived->packet),
                                   &ieee154e_vars.dataReceived->l1_rssi,
                                   &ieee154e_vars.dataReceived->l1_lqi,
                                   &ieee154e_vars.dataReceived->l1_crc);
      
      // break if packet too short
      if (ieee154e_vars.dataReceived->length<LENGTH_CRC || ieee154e_vars.dataReceived->length>LENGTH_IEEE154_MAX) {
         // break from the do-while loop and execute abort code below
          openserial_printError(COMPONENT_IEEE802154E,ERR_INVALIDPACKETFROMRADIO,
                            (errorparameter_t)0,
                            ieee154e_vars.dataReceived->length);
         break;
      }
      
      // toss CRC (2 last bytes)
      packetfunctions_tossFooter(   ieee154e_vars.dataReceived, LENGTH_CRC);
      
      // break if invalid CRC
      if (ieee154e_vars.dataReceived->l1_crc==FALSE) {
         // break from the do-while loop and execute abort code below
         break;
      }
      
      // parse the IEEE802.15.4 header (synchronize, end of frame)
      ieee802154_retrieveHeader(ieee154e_vars.dataReceived,&ieee802514_header);
      
      // break if invalid IEEE802.15.4 header
      if (ieee802514_header.valid==FALSE) {
         // break from the do-while loop and execute the clean-up code below
         break;
      }
      
      // store header details in packet buffer
      ieee154e_vars.dataReceived->l2_frameType = ieee802514_header.frameType;
      ieee154e_vars.dataReceived->l2_dsn       = ieee802514_header.dsn;
      memcpy(&(ieee154e_vars.dataReceived->l2_nextORpreviousHop),&(ieee802514_header.src),sizeof(open_addr_t));

      if (ieee154e_vars.dataReceived->l2_securityLevel != IEEE154_ASH_SLF_TYPE_NOSEC) {
         // If we are not synced, we need to parse IEs and retrieve the ASN
         // before authenticating the beacon, because nonce is created from the ASN
         if (!ieee154e_vars.isSync && ieee802514_header.frameType == IEEE154_TYPE_BEACON) {
            if (!isValidJoin(ieee154e_vars.dataReceived, &ieee802514_header)) {
               // invalidate variables
               memset(&ieee154e_vars, 0, sizeof(ieee154e_vars_t));
               break;
            }
         }
         else if (IEEE802154_SECURITY.incomingFrame(ieee154e_vars.dataReceived) != E_SUCCESS) {
            break;
         }
      } // checked if unsecured frame should pass during header retrieval

      // toss the IEEE802.15.4 header -- this does not include IEs as they are processed
      // next.
      packetfunctions_tossHeader(ieee154e_vars.dataReceived,ieee802514_header.headerLength);
     
      // process IEs
      lenIE = 0;
      if (
            (
               ieee802514_header.valid==TRUE                                                       &&
               ieee802514_header.ieListPresent==TRUE                                               &&
               ieee802514_header.frameType==IEEE154_TYPE_BEACON                                    &&
               packetfunctions_sameAddress(&ieee802514_header.panid,idmanager_getMyID(ADDR_PANID)) &&
               ieee154e_processIEs(ieee154e_vars.dataReceived,&lenIE)
            )==FALSE) {
         // break from the do-while loop and execute the clean-up code below
         break;
      }
    
      // turn off the radio
      radio_rfOff();
      
      // compute radio duty cycle
      ieee154e_vars.radioOnTics += (radio_getTimerValue()-ieee154e_vars.radioOnInit);

      // toss the IEs
      packetfunctions_tossHeader(ieee154e_vars.dataReceived,lenIE);
      
      // synchronize (for the first time) to the sender's EB
      synchronizePacket(ieee154e_vars.syncCapturedTime);
      
      // declare synchronized
      changeIsSync(TRUE);
      
      // log the info
      openserial_printInfo(COMPONENT_IEEE802154E,ERR_SYNCHRONIZED,
                            (errorparameter_t)ieee154e_vars.slotOffset,
                            (errorparameter_t)0);
      
      // send received EB up the stack so RES can update statistics (synchronizing)
      notif_receive(ieee154e_vars.dataReceived);
      
      // clear local variable
      ieee154e_vars.dataReceived = NULL;
      
      // official end of synchronization
      endSlot();
      
      // everything went well, return here not to execute the error code below
      return;
      
   } while(0);
   
   // free the (invalid) received data buffer so RAM memory can be recycled
   openqueue_freePacketBuffer(ieee154e_vars.dataReceived);
   
   // clear local variable
   ieee154e_vars.dataReceived = NULL;
   
   // return to listening state
   changeState(S_SYNCLISTEN);
}

port_INLINE bool ieee154e_processIEs(OpenQueueEntry_t* pkt, uint16_t* lenIE) {
   uint8_t               ptr;
   uint8_t               temp_8b;
   uint8_t               gr_elem_id;
   uint8_t               subid;
   uint16_t              temp_16b;
   uint16_t              len;
   uint16_t              sublen;
   // flag used for understanding if the slotoffset should be inferred from both ASN and slotframe length
   bool                  f_asn2slotoffset;
   
   ptr=0;
   
   // payload IE header, header IE is processed before when retrieve header  
   
   //candidate IE header  if type ==0 header IE if type==1 payload IE
   temp_8b    = *((uint8_t*)(pkt->payload)+ptr);
   ptr++;
   
   temp_16b   = temp_8b + ((*((uint8_t*)(pkt->payload)+ptr)) << 8);
   ptr++;
   
   *lenIE     = ptr;
   
   if ((temp_16b & IEEE802154E_DESC_TYPE_PAYLOAD_IE) == IEEE802154E_DESC_TYPE_PAYLOAD_IE){
      // payload IE
      
      len          = temp_16b & IEEE802154E_DESC_LEN_PAYLOAD_IE_MASK;
      gr_elem_id   = (temp_16b & IEEE802154E_DESC_GROUPID_PAYLOAD_IE_MASK)>>IEEE802154E_DESC_GROUPID_PAYLOAD_IE_SHIFT;
   } else {
      // header IE
      
      len          = temp_16b & IEEE802154E_DESC_LEN_HEADER_IE_MASK;
      gr_elem_id   = (temp_16b & IEEE802154E_DESC_ELEMENTID_HEADER_IE_MASK)>>IEEE802154E_DESC_ELEMENTID_HEADER_IE_SHIFT; 
   }
   
   *lenIE         += len;
   
   //===== sub-elements
   
   switch(gr_elem_id){
      
      case IEEE802154E_MLME_IE_GROUPID:
         // MLME IE
         f_asn2slotoffset = FALSE;
         do {
            
            //read sub IE header
            temp_8b     = *((uint8_t*)(pkt->payload)+ptr);
            ptr         = ptr + 1;
            temp_16b    = temp_8b  + ((*((uint8_t*)(pkt->payload)+ptr))<<8);
            ptr         = ptr + 1;
            
            len         = len - 2; //remove header fields len
            
            if ((temp_16b & IEEE802154E_DESC_TYPE_LONG) == IEEE802154E_DESC_TYPE_LONG){
               // long sub-IE
               
               sublen   = temp_16b & IEEE802154E_DESC_LEN_LONG_MLME_IE_MASK;
               subid    = (temp_16b & IEEE802154E_DESC_SUBID_LONG_MLME_IE_MASK)>>IEEE802154E_DESC_SUBID_LONG_MLME_IE_SHIFT; 
            } else {
               // short sub-IE
               
               sublen   = temp_16b & IEEE802154E_DESC_LEN_SHORT_MLME_IE_MASK;
               subid    = (temp_16b & IEEE802154E_DESC_SUBID_SHORT_MLME_IE_MASK)>>IEEE802154E_DESC_SUBID_SHORT_MLME_IE_SHIFT; 
            }
            
            switch(subid){
               
               case IEEE802154E_MLME_SYNC_IE_SUBID:
                  // Sync IE: ASN and Join Priority 
                  
                  if (idmanager_getIsDAGroot()==FALSE) {
                     // ASN
                     asnStoreFromEB((uint8_t*)(pkt->payload)+ptr);
                     // ASN is known, but the frame length is not
                     // frame length will be known after parsing the frame and link IE
                     f_asn2slotoffset = TRUE;
                     ptr = ptr + 5;
                     // join priority
                     joinPriorityStoreFromEB(*((uint8_t*)(pkt->payload)+ptr));
                     ptr = ptr + 1;
                  }
                  break;
               
               case IEEE802154E_MLME_SLOTFRAME_LINK_IE_SUBID:
                  if ((idmanager_getIsDAGroot()==FALSE) && (ieee154e_isSynch()==FALSE)) {
                     processIE_retrieveSlotframeLinkIE(pkt,&ptr);
                  }
                  break;
               
               case IEEE802154E_MLME_TIMESLOT_IE_SUBID:
                  if (idmanager_getIsDAGroot()==FALSE) {
                      // timelsot template ID
                      timeslotTemplateIDStoreFromEB(*((uint8_t*)(pkt->payload)+ptr));
                      ptr = ptr + 1;
                  }
                  break;
                  
               case IEEE802154E_MLME_CHANNELHOPPING_IE_SUBID:
                  if (idmanager_getIsDAGroot()==FALSE) {
                      // timelsot template ID
                      channelhoppingTemplateIDStoreFromEB(*((uint8_t*)(pkt->payload)+ptr));
                      ptr = ptr + 1;
                  }
                  break;
               default:
                  return FALSE;
                  break;
            }
            
            len = len - sublen;
         } while(len>0);
         if (f_asn2slotoffset == TRUE) {
            // at this point, ASN and frame length are known
            // the current slotoffset can be inferred
            ieee154e_syncSlotOffset();
            if (schedule_syncSlotOffset(ieee154e_vars.slotOffset)==TRUE) {
               ieee154e_vars.nextActiveSlotOffset = schedule_getNextActiveSlotOffset();
            } else {
               ieee154e_vars.isUnscheduledEB = TRUE;
               ieee154e_vars.nextActiveSlotOffset = schedule_getClosestActiveSlotOffset(ieee154e_vars.slotOffset);
            }
         }
         break;
         
      default:
         *lenIE = 0; //no header or not recognized.
         return FALSE;
   }
   
   if(*lenIE>127) {
      // log the error
      openserial_printError(
         COMPONENT_IEEE802154E,
         ERR_HEADER_TOO_LONG,
         (errorparameter_t)*lenIE,
         (errorparameter_t)1
      );
   }
   return TRUE;
}

//======= TX

port_INLINE void activity_ti1ORri1() {
   cellType_t        cellType;
   open_addr_t       neighbor;
   OpenQueueEntry_t* availableEB;
   bool              f_wakeUpForUnscheduledTask;
   
   // increment ASN (do this first so debug pins are in sync)
   incrementAsnOffset();
   
   // stop serial activity
   openserial_stop();
   
   // next off cell will be used for outputting the serial if they have been inputting and viceversa
   ieee154e_vars.serialInputOutput ^= TRUE;
   
   // by default a spouriousEB will not be sent
   ieee154e_vars.isUnscheduledEB = FALSE;
   
   // wiggle debug pins
   debugpins_slot_toggle();
   if (ieee154e_vars.slotOffset==0) {
      debugpins_frame_toggle();
   }
   
   // desynchronize if needed
   if (idmanager_getIsDAGroot()==FALSE) {
      ieee154e_vars.deSyncTimeout--;
      if (ieee154e_vars.deSyncTimeout==0) {
         // declare myself desynchronized
         changeIsSync(FALSE);
        
         // log the error
         openserial_printError(COMPONENT_IEEE802154E,ERR_DESYNCHRONIZED,
                               (errorparameter_t)ieee154e_vars.slotOffset,
                               (errorparameter_t)0);
            
         // update the statistics
         ieee154e_stats.numDeSync++;
            
         // abort
         endSlot();
         return;
      }
   }
   
   // if the previous slot took too long, we will not be in the right state
   if (ieee154e_vars.state!=S_SLEEP) {
      // log the error
      openserial_printError(COMPONENT_IEEE802154E,ERR_WRONG_STATE_IN_STARTSLOT,
                            (errorparameter_t)ieee154e_vars.state,
                            (errorparameter_t)ieee154e_vars.slotOffset);
      // abort
      endSlot();
      return;
   }
   
   // check if an EB is available in the queue
   availableEB = openqueue_macGetEBPacket();
   f_wakeUpForUnscheduledTask = FALSE;
   if (ieee154e_vars.slotOffset==ieee154e_vars.nextActiveSlotOffset) {
      // This is the next active slot
      // advance the schedule
      schedule_advanceSlot();
      // find the next one
      ieee154e_vars.nextActiveSlotOffset = schedule_getNextActiveSlotOffset();
   } else {
      // Being here means that:
      // 1. this mote is not a DAGroot and an EB was present into the queue 
      //    during the previous active slot; in that slot another packet was 
      //    available to be sent, so the EB should still be present into the queue
      // 2. the mote just joined the network and the slotskip optimization has
      //    not been enabled yet
      // 3. this mote is DAGroot, so the slotskip optimization is not enabled
      // This is NOT the next active slot
      f_wakeUpForUnscheduledTask = TRUE;
      // Check if an EB is in the queue
      if (availableEB != NULL) {
         // yes, it is (case 1 and possibly 3)
         ieee154e_vars.dataToSend = availableEB;
         ieee154e_vars.isUnscheduledEB = TRUE;
         ieee154e_sendEB();
      }
   }
   
   // compute the number of slots to be skipped (if this mote is not DAGroot)
   ieee154e_vars.numSkippedSlots = 0;
   if (idmanager_getIsDAGroot()==FALSE) {
      if (ieee154e_vars.nextActiveSlotOffset <= ieee154e_vars.slotOffset) {
         ieee154e_vars.numSkippedSlots += schedule_getFrameLength();
      }
      ieee154e_vars.numSkippedSlots += 
               ieee154e_vars.nextActiveSlotOffset - ieee154e_vars.slotOffset - 1;
   }
   
   // if the mote was waken up for unscheduled tasks, this function terminates here
   if (f_wakeUpForUnscheduledTask) {
      if (availableEB == NULL) {
         // no unscheduled EB (case 2 and possibly 3)
         // abort the slot
         endSlot();
      }
      return;
   }
   
   // check the schedule to see what type of slot this is
   cellType = schedule_getType();
   switch (cellType) {
      case CELLTYPE_TXRX:
      case CELLTYPE_TX:
         // assuming that there is nothing to send
         ieee154e_vars.dataToSend = NULL;
         // check whether we can send
         if (schedule_getOkToSend()) {
            schedule_getNeighbor(&neighbor);
            ieee154e_vars.dataToSend = openqueue_macGetDataPacket(&neighbor);
            if (ieee154e_vars.dataToSend==NULL) {
               ieee154e_vars.dataToSend = availableEB;
            } else {
               // being here means that I will transmit any packet but not EBs
               if (availableEB != NULL) {
                  // compute a random timeslot among the next OFF timeslots for sending the EB
                  if (ieee154e_vars.numSkippedSlots > 0) {
                     ieee154e_vars.numSkippedSlots = openrandom_get16b() % ieee154e_vars.numSkippedSlots;
                  }
               }
               // setting availableEB to NULL, so that it can be used as "bool" variable in what follows
               availableEB = NULL;
            }
         }
         if (ieee154e_vars.dataToSend==NULL) {
            if (cellType==CELLTYPE_TX) {
               // abort
               endSlot();
               // break here if cellType is TX
               break;
            }
            // being here means that this cell is TXRX, the clause CELLTYPE_RX will be executed
         } else {
            if (availableEB != NULL) {        // I will be sending an EB
               if (cellType==CELLTYPE_TX) {
                  ieee154e_vars.isUnscheduledEB = TRUE;
               }
               ieee154e_sendEB();
            } else {
               // change state
               changeState(S_TXDATAOFFSET);
               // change owner
               ieee154e_vars.dataToSend->owner = COMPONENT_IEEE802154E;
               // record that I attempt to transmit this packet
               ieee154e_vars.dataToSend->l2_numTxAttempts++;
               // arm tt1
               radiotimer_schedule(DURATION_tt1);
            }
            break;
         }
      case CELLTYPE_RX:
         // change state
         changeState(S_RXDATAOFFSET);
         // arm rt1
         radiotimer_schedule(DURATION_rt1);
         break;
      default:
         // log the error
         openserial_printCritical(COMPONENT_IEEE802154E,ERR_WRONG_CELLTYPE,
                               (errorparameter_t)cellType,
                               (errorparameter_t)ieee154e_vars.slotOffset);
         // abort
         endSlot();
         break;
   }
}

port_INLINE void activity_ti2() {
   
   // change state
   changeState(S_TXDATAPREPARE);

   // make a local copy of the frame
   packetfunctions_duplicatePacket(&ieee154e_vars.localCopyForTransmission, ieee154e_vars.dataToSend);

   // check if packet needs to be encrypted/authenticated before transmission 
   if (ieee154e_vars.localCopyForTransmission.l2_securityLevel != IEEE154_ASH_SLF_TYPE_NOSEC) { // security enabled
      // encrypt in a local copy
      if (IEEE802154_SECURITY.outgoingFrame(&ieee154e_vars.localCopyForTransmission) != E_SUCCESS) {
         // keep the frame in the OpenQueue in order to retry later
         endSlot(); // abort
         return;
      }
   }
   
   // add 2 CRC bytes only to the local copy as we end up here for each retransmission
   packetfunctions_reserveFooterSize(&ieee154e_vars.localCopyForTransmission, 2);
   
   // calculate the frequency to transmit on
   if (ieee154e_vars.isUnscheduledEB == TRUE) {
      ieee154e_vars.freq = calculateFrequency(SCHEDULE_MINIMAL_6TISCH_CHANNELOFFSET); 
   } else {
      ieee154e_vars.freq = calculateFrequency(schedule_getChannelOffset()); 
   }
   
   // configure the radio for that frequency
   radio_setFrequency(ieee154e_vars.freq);
   
   // load the packet in the radio's Tx buffer
   radio_loadPacket(ieee154e_vars.localCopyForTransmission.payload,
                    ieee154e_vars.localCopyForTransmission.length);
   
   // enable the radio in Tx mode. This does not send the packet.
   radio_txEnable();
   ieee154e_vars.radioOnInit=radio_getTimerValue();
   ieee154e_vars.radioOnThisSlot=TRUE;
   // arm tt2
   radiotimer_schedule(DURATION_tt2);
   
   // change state
   changeState(S_TXDATAREADY);
}

port_INLINE void activity_tie1() {
   // log the error
   openserial_printError(COMPONENT_IEEE802154E,ERR_MAXTXDATAPREPARE_OVERFLOW,
                         (errorparameter_t)ieee154e_vars.state,
                         (errorparameter_t)ieee154e_vars.slotOffset);
   
   // abort
   endSlot();
}

port_INLINE void activity_ti3() {
   // change state
   changeState(S_TXDATADELAY);
   
   // arm tt3
   radiotimer_schedule(DURATION_tt3);
   
   // give the 'go' to transmit
   radio_txNow();
}

port_INLINE void activity_tie2() {
   // log the error
   openserial_printError(COMPONENT_IEEE802154E,ERR_WDRADIO_OVERFLOWS,
                         (errorparameter_t)ieee154e_vars.state,
                         (errorparameter_t)ieee154e_vars.slotOffset);
   
   // abort
   endSlot();
}

//start of frame interrupt
port_INLINE void activity_ti4(PORT_RADIOTIMER_WIDTH capturedTime) {
   // change state
   changeState(S_TXDATA);
   
   // cancel tt3
   radiotimer_cancel();
   
   // record the captured time
   ieee154e_vars.lastCapturedTime = capturedTime;
   
   // arm tt4
   radiotimer_schedule(DURATION_tt4);
}

port_INLINE void activity_tie3() {
   // log the error
   openserial_printError(COMPONENT_IEEE802154E,ERR_WDDATADURATION_OVERFLOWS,
                         (errorparameter_t)ieee154e_vars.state,
                         (errorparameter_t)ieee154e_vars.slotOffset);
   
   // abort
   endSlot();
}

port_INLINE void activity_ti5(PORT_RADIOTIMER_WIDTH capturedTime) {
   bool listenForAck;
   
   // change state
   changeState(S_RXACKOFFSET);
   
   // cancel tt4
   radiotimer_cancel();
   
   // turn off the radio
   radio_rfOff();
   ieee154e_vars.radioOnTics+=(radio_getTimerValue()-ieee154e_vars.radioOnInit);
   
   // record the captured time
   ieee154e_vars.lastCapturedTime = capturedTime;
   
   // decides whether to listen for an ACK
   if (packetfunctions_isBroadcastMulticast(&ieee154e_vars.dataToSend->l2_nextORpreviousHop)==TRUE) {
      listenForAck = FALSE;
   } else {
      listenForAck = TRUE;
   }
   
   if (listenForAck==TRUE) {
      // arm tt5
      radiotimer_schedule(DURATION_tt5);
   } else {
      if (ieee154e_vars.isUnscheduledEB == FALSE) {
         // indicate succesful Tx to schedule to keep statistics
         schedule_indicateTx(&ieee154e_vars.asn,TRUE);
      }
      // indicate to upper later the packet was sent successfully
      notif_sendDone(ieee154e_vars.dataToSend,E_SUCCESS);
      // reset local variable
      ieee154e_vars.dataToSend = NULL;
      // abort
      endSlot();
   }
}

port_INLINE void activity_ti6() {
   // change state
   changeState(S_RXACKPREPARE);
   
   // calculate the frequency to transmit on
   ieee154e_vars.freq = calculateFrequency(schedule_getChannelOffset()); 
   
   // configure the radio for that frequency
   radio_setFrequency(ieee154e_vars.freq);
   
   // enable the radio in Rx mode. The radio is not actively listening yet.
   radio_rxEnable();
   //caputre init of radio for duty cycle calculation
   ieee154e_vars.radioOnInit=radio_getTimerValue();
   ieee154e_vars.radioOnThisSlot=TRUE;
   // arm tt6
   radiotimer_schedule(DURATION_tt6);
   
   // change state
   changeState(S_RXACKREADY);
}

port_INLINE void activity_tie4() {
   // log the error
   openserial_printError(COMPONENT_IEEE802154E,ERR_MAXRXACKPREPARE_OVERFLOWS,
                         (errorparameter_t)ieee154e_vars.state,
                         (errorparameter_t)ieee154e_vars.slotOffset);
   
   // abort
   endSlot();
}

port_INLINE void activity_ti7() {
   // change state
   changeState(S_RXACKLISTEN);
   
   // start listening
   radio_rxNow();
   
   // arm tt7
   radiotimer_schedule(DURATION_tt7);
}

port_INLINE void activity_tie5() {
   // indicate transmit failed to schedule to keep stats
   schedule_indicateTx(&ieee154e_vars.asn,FALSE);
   
   // decrement transmits left counter
   ieee154e_vars.dataToSend->l2_retriesLeft--;
   
   if (ieee154e_vars.dataToSend->l2_retriesLeft==0) {
      // indicate tx fail if no more retries left
      notif_sendDone(ieee154e_vars.dataToSend,E_FAIL);
   } else {
      // return packet to the virtual COMPONENT_SIXTOP_TO_IEEE802154E component
      ieee154e_vars.dataToSend->owner = COMPONENT_SIXTOP_TO_IEEE802154E;
   }
   
   // reset local variable
   ieee154e_vars.dataToSend = NULL;
   
   // abort
   endSlot();
}

port_INLINE void activity_ti8(PORT_RADIOTIMER_WIDTH capturedTime) {
   // change state
   changeState(S_RXACK);
   
   // cancel tt7
   radiotimer_cancel();
   
   // record the captured time
   ieee154e_vars.lastCapturedTime = capturedTime;
   
   // arm tt8
   radiotimer_schedule(DURATION_tt8);
}

port_INLINE void activity_tie6() {
   // abort
   endSlot();
}

port_INLINE void activity_ti9(PORT_RADIOTIMER_WIDTH capturedTime) {
   ieee802154_header_iht     ieee802514_header;
   
   // change state
   changeState(S_TXPROC);
   
   // cancel tt8
   radiotimer_cancel();
   
   // turn off the radio
   radio_rfOff();
   //compute tics radio on.
   ieee154e_vars.radioOnTics+=(radio_getTimerValue()-ieee154e_vars.radioOnInit);
   
   // record the captured time
   ieee154e_vars.lastCapturedTime = capturedTime;
   
   // get a buffer to put the (received) ACK in
   ieee154e_vars.ackReceived = openqueue_getFreePacketBuffer(COMPONENT_IEEE802154E);
   if (ieee154e_vars.ackReceived==NULL) {
      // log the error
      openserial_printError(COMPONENT_IEEE802154E,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      // abort
      endSlot();
      return;
   }
   
   // declare ownership over that packet
   ieee154e_vars.ackReceived->creator = COMPONENT_IEEE802154E;
   ieee154e_vars.ackReceived->owner   = COMPONENT_IEEE802154E;
   
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
      ieee154e_vars.ackReceived->payload = &(ieee154e_vars.ackReceived->packet[FIRST_FRAME_BYTE]);
      radio_getReceivedFrame(       ieee154e_vars.ackReceived->payload,
                                   &ieee154e_vars.ackReceived->length,
                             sizeof(ieee154e_vars.ackReceived->packet),
                                   &ieee154e_vars.ackReceived->l1_rssi,
                                   &ieee154e_vars.ackReceived->l1_lqi,
                                   &ieee154e_vars.ackReceived->l1_crc);
      
      // break if wrong length
      if (ieee154e_vars.ackReceived->length<LENGTH_CRC || ieee154e_vars.ackReceived->length>LENGTH_IEEE154_MAX) {
         // break from the do-while loop and execute the clean-up code below
        openserial_printError(COMPONENT_IEEE802154E,ERR_INVALIDPACKETFROMRADIO,
                            (errorparameter_t)1,
                            ieee154e_vars.ackReceived->length);
        
         break;
      }
      
      // toss CRC (2 last bytes)
      packetfunctions_tossFooter(   ieee154e_vars.ackReceived, LENGTH_CRC);
   
      // break if invalid CRC
      if (ieee154e_vars.ackReceived->l1_crc==FALSE) {
         // break from the do-while loop and execute the clean-up code below
         break;
      }
      
      // parse the IEEE802.15.4 header (RX ACK)
      ieee802154_retrieveHeader(ieee154e_vars.ackReceived,&ieee802514_header);
      
      // break if invalid IEEE802.15.4 header
      if (ieee802514_header.valid==FALSE) {
         // break from the do-while loop and execute the clean-up code below
         break;
      }

      // store header details in packet buffer
      ieee154e_vars.ackReceived->l2_frameType  = ieee802514_header.frameType;
      ieee154e_vars.ackReceived->l2_dsn        = ieee802514_header.dsn;
      memcpy(&(ieee154e_vars.ackReceived->l2_nextORpreviousHop),&(ieee802514_header.src),sizeof(open_addr_t));

      // check the security level of the ACK frame and decrypt/authenticate
      if (ieee154e_vars.ackReceived->l2_securityLevel != IEEE154_ASH_SLF_TYPE_NOSEC) {
          if (IEEE802154_SECURITY.incomingFrame(ieee154e_vars.ackReceived) != E_SUCCESS) {
         	 break;
          }
      } // checked if unsecured frame should pass during header retrieval
      
      // toss the IEEE802.15.4 header
      packetfunctions_tossHeader(ieee154e_vars.ackReceived,ieee802514_header.headerLength);
    
      // break if invalid ACK
      if (isValidAck(&ieee802514_header,ieee154e_vars.dataToSend)==FALSE) {
         // break from the do-while loop and execute the clean-up code below
         break;
      }
         
      if (
            idmanager_getIsDAGroot()==FALSE &&
            neighbors_isPreferredParent(&(ieee154e_vars.ackReceived->l2_nextORpreviousHop))
         ) {
         synchronizeAck(ieee802514_header.timeCorrection);
      }
      
      // inform schedule of successful transmission
      schedule_indicateTx(&ieee154e_vars.asn,TRUE);
      
      // inform upper layer
      notif_sendDone(ieee154e_vars.dataToSend,E_SUCCESS);
      ieee154e_vars.dataToSend = NULL;
      
      // in any case, execute the clean-up code below (processing of ACK done)
   } while (0);
   
   // free the received ack so corresponding RAM memory can be recycled
   openqueue_freePacketBuffer(ieee154e_vars.ackReceived);
   
   // clear local variable
   ieee154e_vars.ackReceived = NULL;
   
   // official end of Tx slot
   endSlot();
}

//======= RX

port_INLINE void activity_ri2() {
   // change state
   changeState(S_RXDATAPREPARE);
   
   // calculate the frequency to transmit on
   ieee154e_vars.freq = calculateFrequency(schedule_getChannelOffset()); 
   
   // configure the radio for that frequency
   radio_setFrequency(ieee154e_vars.freq);
   
   // enable the radio in Rx mode. The radio does not actively listen yet.
   radio_rxEnable();
   ieee154e_vars.radioOnInit=radio_getTimerValue();
   ieee154e_vars.radioOnThisSlot=TRUE;
   
   // arm rt2
   radiotimer_schedule(DURATION_rt2);
       
   // change state
   changeState(S_RXDATAREADY);
}

port_INLINE void activity_rie1() {
   // log the error
   openserial_printError(COMPONENT_IEEE802154E,ERR_MAXRXDATAPREPARE_OVERFLOWS,
                         (errorparameter_t)ieee154e_vars.state,
                         (errorparameter_t)ieee154e_vars.slotOffset);
   
   // abort
   endSlot();
}

port_INLINE void activity_ri3() {
   // change state
   changeState(S_RXDATALISTEN);
   
   // give the 'go' to receive
   radio_rxNow();
   
   // arm rt3 
   radiotimer_schedule(DURATION_rt3);
}

port_INLINE void activity_rie2() {
   // abort
   endSlot();
}

port_INLINE void activity_ri4(PORT_RADIOTIMER_WIDTH capturedTime) {

   // change state
   changeState(S_RXDATA);
   
   // cancel rt3
   radiotimer_cancel();
   
   // record the captured time
   ieee154e_vars.lastCapturedTime = capturedTime;
   
   // record the captured time to sync
   ieee154e_vars.syncCapturedTime = capturedTime;

   radiotimer_schedule(DURATION_rt4);
}

port_INLINE void activity_rie3() {
     
   // log the error
   openserial_printError(COMPONENT_IEEE802154E,ERR_WDDATADURATION_OVERFLOWS,
                         (errorparameter_t)ieee154e_vars.state,
                         (errorparameter_t)ieee154e_vars.slotOffset);
   
   // abort
   endSlot();
}

port_INLINE void activity_ri5(PORT_RADIOTIMER_WIDTH capturedTime) {
   ieee802154_header_iht ieee802514_header;
   uint16_t lenIE=0;
   
   // change state
   changeState(S_TXACKOFFSET);
   
   // cancel rt4
   radiotimer_cancel();

   // turn off the radio
   radio_rfOff();
   ieee154e_vars.radioOnTics+=radio_getTimerValue()-ieee154e_vars.radioOnInit;
   // get a buffer to put the (received) data in
   ieee154e_vars.dataReceived = openqueue_getFreePacketBuffer(COMPONENT_IEEE802154E);
   if (ieee154e_vars.dataReceived==NULL) {
      // log the error
      openserial_printError(COMPONENT_IEEE802154E,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      // abort
      endSlot();
      return;
   }
   
   // declare ownership over that packet
   ieee154e_vars.dataReceived->creator = COMPONENT_IEEE802154E;
   ieee154e_vars.dataReceived->owner   = COMPONENT_IEEE802154E;

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
      ieee154e_vars.dataReceived->payload = &(ieee154e_vars.dataReceived->packet[FIRST_FRAME_BYTE]);
      radio_getReceivedFrame(       ieee154e_vars.dataReceived->payload,
                                   &ieee154e_vars.dataReceived->length,
                             sizeof(ieee154e_vars.dataReceived->packet),
                                   &ieee154e_vars.dataReceived->l1_rssi,
                                   &ieee154e_vars.dataReceived->l1_lqi,
                                   &ieee154e_vars.dataReceived->l1_crc);
      
      // break if wrong length
      if (ieee154e_vars.dataReceived->length<LENGTH_CRC || ieee154e_vars.dataReceived->length>LENGTH_IEEE154_MAX ) {
         // jump to the error code below this do-while loop
         openserial_printError(COMPONENT_IEEE802154E,ERR_INVALIDPACKETFROMRADIO,
                            (errorparameter_t)2,
                            ieee154e_vars.dataReceived->length);
         break;
      }
      
      // toss CRC (2 last bytes)
      packetfunctions_tossFooter(   ieee154e_vars.dataReceived, LENGTH_CRC);
      
      // if CRC doesn't check, stop
      if (ieee154e_vars.dataReceived->l1_crc==FALSE) {
         // jump to the error code below this do-while loop
         break;
      }
      
      // parse the IEEE802.15.4 header (RX DATA)
      ieee802154_retrieveHeader(ieee154e_vars.dataReceived,&ieee802514_header);
      
      // break if invalid IEEE802.15.4 header
      if (ieee802514_header.valid==FALSE) {
         // break from the do-while loop and execute the clean-up code below
         break;
      }

      // store header details in packet buffer
      ieee154e_vars.dataReceived->l2_frameType      = ieee802514_header.frameType;
      ieee154e_vars.dataReceived->l2_dsn            = ieee802514_header.dsn;
      ieee154e_vars.dataReceived->l2_IEListPresent  = ieee802514_header.ieListPresent;
      memcpy(&(ieee154e_vars.dataReceived->l2_nextORpreviousHop),&(ieee802514_header.src),sizeof(open_addr_t));

      // if security is enabled, decrypt/authenticate the frame.
      if (ieee154e_vars.dataReceived->l2_securityLevel != IEEE154_ASH_SLF_TYPE_NOSEC) {
         if (IEEE802154_SECURITY.incomingFrame(ieee154e_vars.dataReceived) != E_SUCCESS) {
            break;
         }
      } // checked if unsecured frame should pass during header retrieval

      // toss the IEEE802.15.4 header
      packetfunctions_tossHeader(ieee154e_vars.dataReceived,ieee802514_header.headerLength);

      // handle IEs xv poipoi
      // reset join priority 
      // retrieve IE in sixtop
      if ((ieee802514_header.valid==TRUE &&
          ieee802514_header.ieListPresent==TRUE && 
          ieee802514_header.frameType==IEEE154_TYPE_BEACON && // if it is not a beacon and have ie, the ie will be processed in sixtop
          packetfunctions_sameAddress(&ieee802514_header.panid,idmanager_getMyID(ADDR_PANID)) && 
          ieee154e_processIEs(ieee154e_vars.dataReceived,&lenIE))==FALSE) {
          //log  that the packet is not carrying IEs
      }
      
     // toss the IEs including Synch
      packetfunctions_tossHeader(ieee154e_vars.dataReceived,lenIE);
            
      // record the captured time
      ieee154e_vars.lastCapturedTime = capturedTime;
      
      // if I just received an invalid frame, stop
      if (isValidRxFrame(&ieee802514_header)==FALSE) {
         // jump to the error code below this do-while loop
         break;
      }
      
      // record the timeCorrection and print out at end of slot
      ieee154e_vars.dataReceived->l2_timeCorrection = (PORT_SIGNED_INT_WIDTH)((PORT_SIGNED_INT_WIDTH)TsTxOffset-(PORT_SIGNED_INT_WIDTH)ieee154e_vars.syncCapturedTime);
      
      // check if ack requested
      if (ieee802514_header.ackRequested==1 && ieee154e_vars.isAckEnabled == TRUE) {
         // arm rt5
         radiotimer_schedule(DURATION_rt5);
      } else {
         // synchronize to the received packet iif I'm not a DAGroot and this is my preferred parent
         if (idmanager_getIsDAGroot()==FALSE && neighbors_isPreferredParent(&(ieee154e_vars.dataReceived->l2_nextORpreviousHop))) {
            synchronizePacket(ieee154e_vars.syncCapturedTime);
         }
         // indicate reception to upper layer (no ACK asked)
         notif_receive(ieee154e_vars.dataReceived);
         // reset local variable
         ieee154e_vars.dataReceived = NULL;
         // abort
         endSlot();
      }
      
      // everything went well, return here not to execute the error code below
      return;
      
   } while(0);
   
   // free the (invalid) received data so RAM memory can be recycled
   openqueue_freePacketBuffer(ieee154e_vars.dataReceived);
   
   // clear local variable
   ieee154e_vars.dataReceived = NULL;
   
   // abort
   endSlot();
}

port_INLINE void activity_ri6() {
   
   // change state
   changeState(S_TXACKPREPARE);
   
   // get a buffer to put the ack to send in
   ieee154e_vars.ackToSend = openqueue_getFreePacketBuffer(COMPONENT_IEEE802154E);
   if (ieee154e_vars.ackToSend==NULL) {
      // log the error
      openserial_printError(COMPONENT_IEEE802154E,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      // indicate we received a packet anyway (we don't want to loose any)
      notif_receive(ieee154e_vars.dataReceived);
      // free local variable
      ieee154e_vars.dataReceived = NULL;
      // abort
      endSlot();
      return;
   }
   
   // declare ownership over that packet
   ieee154e_vars.ackToSend->creator = COMPONENT_IEEE802154E;
   ieee154e_vars.ackToSend->owner   = COMPONENT_IEEE802154E;
   
   // calculate the time timeCorrection (this is the time the sender is off w.r.t to this node. A negative number means
   // the sender is too late.
   ieee154e_vars.timeCorrection = (PORT_SIGNED_INT_WIDTH)((PORT_SIGNED_INT_WIDTH)TsTxOffset-(PORT_SIGNED_INT_WIDTH)ieee154e_vars.syncCapturedTime);
   
   // prepend the IEEE802.15.4 header to the ACK
   ieee154e_vars.ackToSend->l2_frameType = IEEE154_TYPE_ACK;
   ieee154e_vars.ackToSend->l2_dsn       = ieee154e_vars.dataReceived->l2_dsn;

   // To send ACK, we use the same security level (including NOSEC) and keys
   // that were present in the DATA packet.
   ieee154e_vars.ackToSend->l2_securityLevel = ieee154e_vars.dataReceived->l2_securityLevel;
   ieee154e_vars.ackToSend->l2_keyIdMode     = ieee154e_vars.dataReceived->l2_keyIdMode;
   ieee154e_vars.ackToSend->l2_keyIndex      = ieee154e_vars.dataReceived->l2_keyIndex;

   ieee802154_prependHeader(ieee154e_vars.ackToSend,
                            ieee154e_vars.ackToSend->l2_frameType,
                            FALSE,//no payloadIE in ack
                            ieee154e_vars.dataReceived->l2_dsn,
                            &(ieee154e_vars.dataReceived->l2_nextORpreviousHop)
                            );
   
   // if security is enabled, encrypt directly in OpenQueue as there are no retransmissions for ACKs
   if (ieee154e_vars.ackToSend->l2_securityLevel != IEEE154_ASH_SLF_TYPE_NOSEC) {
      if (IEEE802154_SECURITY.outgoingFrame(ieee154e_vars.ackToSend) != E_SUCCESS) {
     	   openqueue_freePacketBuffer(ieee154e_vars.ackToSend);
     	   endSlot();
     	   return;
      }
   }
    // space for 2-byte CRC
   packetfunctions_reserveFooterSize(ieee154e_vars.ackToSend,2);
  
    // calculate the frequency to transmit on
   ieee154e_vars.freq = calculateFrequency(schedule_getChannelOffset()); 
   
   // configure the radio for that frequency
   radio_setFrequency(ieee154e_vars.freq);
   
   // load the packet in the radio's Tx buffer
   radio_loadPacket(ieee154e_vars.ackToSend->payload,
                    ieee154e_vars.ackToSend->length);
   
   // enable the radio in Tx mode. This does not send that packet.
   radio_txEnable();
   ieee154e_vars.radioOnInit=radio_getTimerValue();
   ieee154e_vars.radioOnThisSlot=TRUE;
   // arm rt6
   radiotimer_schedule(DURATION_rt6);
   
   // change state
   changeState(S_TXACKREADY);
}

port_INLINE void activity_rie4() {
   // log the error
   openserial_printError(COMPONENT_IEEE802154E,ERR_MAXTXACKPREPARE_OVERFLOWS,
                         (errorparameter_t)ieee154e_vars.state,
                         (errorparameter_t)ieee154e_vars.slotOffset);
   
   // abort
   endSlot();
}

port_INLINE void activity_ri7() {
   // change state
   changeState(S_TXACKDELAY);
   
   // arm rt7
   radiotimer_schedule(DURATION_rt7);
   
   // give the 'go' to transmit
   radio_txNow(); 
}

port_INLINE void activity_rie5() {
   // log the error
   openserial_printError(COMPONENT_IEEE802154E,ERR_WDRADIOTX_OVERFLOWS,
                         (errorparameter_t)ieee154e_vars.state,
                         (errorparameter_t)ieee154e_vars.slotOffset);
   
   // abort
   endSlot();
}

port_INLINE void activity_ri8(PORT_RADIOTIMER_WIDTH capturedTime) {
   // change state
   changeState(S_TXACK);
   
   // cancel rt7
   radiotimer_cancel();
   
   // record the captured time
   ieee154e_vars.lastCapturedTime = capturedTime;
   
   // arm rt8
   radiotimer_schedule(DURATION_rt8);
}

port_INLINE void activity_rie6() {
   // log the error
   openserial_printError(COMPONENT_IEEE802154E,ERR_WDACKDURATION_OVERFLOWS,
                         (errorparameter_t)ieee154e_vars.state,
                         (errorparameter_t)ieee154e_vars.slotOffset);
   
   // abort
   endSlot();
}

port_INLINE void activity_ri9(PORT_RADIOTIMER_WIDTH capturedTime) {
   // change state
   changeState(S_RXPROC);
   
   // cancel rt8
   radiotimer_cancel();
  
   // record the captured time
   ieee154e_vars.lastCapturedTime = capturedTime;
   
   // free the ack we just sent so corresponding RAM memory can be recycled
   openqueue_freePacketBuffer(ieee154e_vars.ackToSend);
   
   // clear local variable
   ieee154e_vars.ackToSend = NULL;
   
   // synchronize to the received packet
   if (idmanager_getIsDAGroot()==FALSE && neighbors_isPreferredParent(&(ieee154e_vars.dataReceived->l2_nextORpreviousHop))) {
      synchronizePacket(ieee154e_vars.syncCapturedTime);
   }
   
   // inform upper layer of reception (after ACK sent)
   notif_receive(ieee154e_vars.dataReceived);
   
   // clear local variable
   ieee154e_vars.dataReceived = NULL;
   
   // official end of Rx slot
   endSlot();
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
port_INLINE bool isValidRxFrame(ieee802154_header_iht* ieee802514_header) {
   return ieee802514_header->valid==TRUE                                                           && \
          (
             ieee802514_header->frameType==IEEE154_TYPE_DATA                   ||
             ieee802514_header->frameType==IEEE154_TYPE_BEACON
          )                                                                                        && \
          packetfunctions_sameAddress(&ieee802514_header->panid,idmanager_getMyID(ADDR_PANID))     && \
          (
             idmanager_isMyAddress(&ieee802514_header->dest)                   ||
             packetfunctions_isBroadcastMulticast(&ieee802514_header->dest)
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
port_INLINE bool isValidAck(ieee802154_header_iht* ieee802514_header, OpenQueueEntry_t* packetSent) {
   /*
   return ieee802514_header->valid==TRUE                                                           && \
          ieee802514_header->frameType==IEEE154_TYPE_ACK                                           && \
          ieee802514_header->dsn==packetSent->l2_dsn                                               && \
          packetfunctions_sameAddress(&ieee802514_header->panid,idmanager_getMyID(ADDR_PANID))     && \
          idmanager_isMyAddress(&ieee802514_header->dest)                                          && \
          packetfunctions_sameAddress(&ieee802514_header->src,&packetSent->l2_nextORpreviousHop);
   */
   // poipoi don't check for seq num
   return ieee802514_header->valid==TRUE                                                           && \
          ieee802514_header->frameType==IEEE154_TYPE_ACK                                           && \
          packetfunctions_sameAddress(&ieee802514_header->panid,idmanager_getMyID(ADDR_PANID))     && \
          idmanager_isMyAddress(&ieee802514_header->dest)                                          && \
          packetfunctions_sameAddress(&ieee802514_header->src,&packetSent->l2_nextORpreviousHop);
}

//======= ASN handling

port_INLINE void incrementAsnOffset() {
   frameLength_t frameLength;
   
   // increment the asn
   ieee154e_vars.asn.bytes0and1++;
   if (ieee154e_vars.asn.bytes0and1==0) {
      ieee154e_vars.asn.bytes2and3++;
      if (ieee154e_vars.asn.bytes2and3==0) {
         ieee154e_vars.asn.byte4++;
      }
   }
   
   // increment the offsets
   frameLength = schedule_getFrameLength();
   if (frameLength == 0) {
      ieee154e_vars.slotOffset++;
   } else {
      ieee154e_vars.slotOffset  = (ieee154e_vars.slotOffset+1)%frameLength;
   }
   ieee154e_vars.asnOffset   = (ieee154e_vars.asnOffset+1)%16;
}

//from upper layer that want to send the ASN to compute timing or latency
port_INLINE void ieee154e_getAsn(uint8_t* array) {
   array[0]         = (ieee154e_vars.asn.bytes0and1     & 0xff);
   array[1]         = (ieee154e_vars.asn.bytes0and1/256 & 0xff);
   array[2]         = (ieee154e_vars.asn.bytes2and3     & 0xff);
   array[3]         = (ieee154e_vars.asn.bytes2and3/256 & 0xff);
   array[4]         =  ieee154e_vars.asn.byte4;
}

port_INLINE uint16_t ieee154e_getTimeCorrection() {
    int16_t returnVal;
    
    returnVal = (uint16_t)(ieee154e_vars.timeCorrection);
    
    return returnVal;
}

port_INLINE void joinPriorityStoreFromEB(uint8_t jp){
  ieee154e_vars.dataReceived->l2_joinPriority = jp;
  ieee154e_vars.dataReceived->l2_joinPriorityPresent = TRUE;     
}

// This function parses IEs from an EB to get to the ASN before security
// processing is invoked. It should be called *only* when a node has no/lost sync.
// This way, we can authenticate EBs and reject unwanted ones.
bool isValidJoin(OpenQueueEntry_t* eb, ieee802154_header_iht *parsedHeader) {
   uint16_t              lenIE;

   // toss the header in order to get to IEs
   packetfunctions_tossHeader(eb, parsedHeader->headerLength);
     
   // process IEs
   // at this stage, this can work only if EB is authenticated but not encrypted
   lenIE = 0;
   if (
         (
            parsedHeader->valid==TRUE                                                       &&
            parsedHeader->ieListPresent==TRUE                                               &&
            parsedHeader->frameType==IEEE154_TYPE_BEACON                                    &&
            packetfunctions_sameAddress(&parsedHeader->panid,idmanager_getMyID(ADDR_PANID)) &&
            ieee154e_processIEs(eb,&lenIE)
         )==FALSE) {
      return FALSE;
   }
   
   // put everything back in place in order to invoke security-incoming on the
   // correct frame length and correct pointers (first byte of the frame)
   packetfunctions_reserveHeaderSize(eb, parsedHeader->headerLength);

   // verify EB's authentication tag
   if (IEEE802154_SECURITY.incomingFrame(eb) == E_SUCCESS) {
      return TRUE;
   }

   return FALSE;
}

port_INLINE void asnStoreFromEB(uint8_t* asn) {
   
   // store the ASN
   ieee154e_vars.asn.bytes0and1   =     asn[0]+
                                    256*asn[1];
   ieee154e_vars.asn.bytes2and3   =     asn[2]+
                                    256*asn[3];
   ieee154e_vars.asn.byte4        =     asn[4];
}

port_INLINE void ieee154e_syncSlotOffset() {
   frameLength_t frameLength;
   uint32_t slotOffset;
   
   frameLength = schedule_getFrameLength();
   
   // determine the current slotOffset
   slotOffset = ieee154e_vars.asn.byte4;
   slotOffset = slotOffset % frameLength;
   slotOffset = slotOffset << 16;
   slotOffset = slotOffset + ieee154e_vars.asn.bytes2and3;
   slotOffset = slotOffset % frameLength;
   slotOffset = slotOffset << 16;
   slotOffset = slotOffset + ieee154e_vars.asn.bytes0and1;
   slotOffset = slotOffset % frameLength;
   
   ieee154e_vars.slotOffset       = (slotOffset_t) slotOffset;
   ieee154e_vars.asnOffset        = ieee154e_vars.asn.bytes0and1 & 0xf;
}

void ieee154e_setIsAckEnabled(bool isEnabled){
    ieee154e_vars.isAckEnabled = isEnabled;
}

void ieee154e_setSingleChannel(uint8_t channel){
    if (
        (channel < 11 || channel > 26) &&
         channel != 0   // channel == 0 means channel hopping is enabled
    ) {
        // log wrong channel, should be  : (0, or 11~26)
        return;
    }
    ieee154e_vars.singleChannel = channel;
    ieee154e_vars.singleChannelChanged = TRUE;
}

void ieee154e_setIsSecurityEnabled(bool isEnabled){
    ieee154e_vars.isSecurityEnabled = isEnabled;
}

// timeslot template handling
port_INLINE void timeslotTemplateIDStoreFromEB(uint8_t id){
    ieee154e_vars.tsTemplateId = id;
}

// channelhopping template handling
port_INLINE void channelhoppingTemplateIDStoreFromEB(uint8_t id){
    ieee154e_vars.chTemplateId = id;
}
//======= synchronization

void synchronizePacket(PORT_RADIOTIMER_WIDTH timeReceived) {
   PORT_SIGNED_INT_WIDTH timeCorrection;
   PORT_RADIOTIMER_WIDTH newPeriod;
   PORT_RADIOTIMER_WIDTH currentValue;
   PORT_RADIOTIMER_WIDTH currentPeriod;
   
   // record the current timer value and period
   currentValue                   =  radio_getTimerValue();
   currentPeriod                  =  radio_getTimerPeriod();
   
   // calculate new period
   timeCorrection                 =  (PORT_SIGNED_INT_WIDTH)((PORT_SIGNED_INT_WIDTH)timeReceived-(PORT_SIGNED_INT_WIDTH)TsTxOffset);

   newPeriod                      =  TsSlotDuration;
   
   // detect whether I'm too close to the edge of the slot, in that case,
   // skip a slot and increase the temporary slot length to be 2 slots long
   if (currentValue<timeReceived || currentPeriod-currentValue<RESYNCHRONIZATIONGUARD) {
      newPeriod                  +=  TsSlotDuration;
      incrementAsnOffset();
   }
   newPeriod                      =  (PORT_RADIOTIMER_WIDTH)((PORT_SIGNED_INT_WIDTH)newPeriod+timeCorrection);
   
   // resynchronize by applying the new period
   radio_setTimerPeriod(newPeriod);
#ifdef ADAPTIVE_SYNC
   // indicate time correction to adaptive sync module
   adaptive_sync_indicateTimeCorrection(timeCorrection,ieee154e_vars.dataReceived->l2_nextORpreviousHop);
#endif
   // reset the de-synchronization timeout
   ieee154e_vars.deSyncTimeout    = DESYNCTIMEOUT;
   
   // log a large timeCorrection
   if (
         ieee154e_vars.isSync==TRUE &&
         (
            timeCorrection<-LIMITLARGETIMECORRECTION ||
            timeCorrection> LIMITLARGETIMECORRECTION
         )
      ) {
      openserial_printError(COMPONENT_IEEE802154E,ERR_LARGE_TIMECORRECTION,
                            (errorparameter_t)timeCorrection,
                            (errorparameter_t)0);
   }
   
   // update the stats
   ieee154e_stats.numSyncPkt++;
   updateStats(timeCorrection);
   
#ifdef OPENSIM
   debugpins_syncPacket_set();
   debugpins_syncPacket_clr();
#endif
}

void synchronizeAck(PORT_SIGNED_INT_WIDTH timeCorrection) {
   PORT_RADIOTIMER_WIDTH newPeriod;
   PORT_RADIOTIMER_WIDTH currentPeriod;
   
   // calculate new period
   currentPeriod                  =  radio_getTimerPeriod();
   newPeriod                      =  (PORT_RADIOTIMER_WIDTH)((PORT_SIGNED_INT_WIDTH)currentPeriod+timeCorrection);

   // resynchronize by applying the new period
   radio_setTimerPeriod(newPeriod);
   
   // reset the de-synchronization timeout
   ieee154e_vars.deSyncTimeout    = DESYNCTIMEOUT;
#ifdef ADAPTIVE_SYNC
   // indicate time correction to adaptive sync module
   adaptive_sync_indicateTimeCorrection((-timeCorrection),ieee154e_vars.ackReceived->l2_nextORpreviousHop);
#endif
   // log a large timeCorrection
   if (
         ieee154e_vars.isSync==TRUE &&
         (
            timeCorrection<-LIMITLARGETIMECORRECTION ||
            timeCorrection> LIMITLARGETIMECORRECTION
         )
      ) {
      openserial_printError(COMPONENT_IEEE802154E,ERR_LARGE_TIMECORRECTION,
                            (errorparameter_t)timeCorrection,
                            (errorparameter_t)1);
   }

   // update the stats
   ieee154e_stats.numSyncAck++;
   updateStats(timeCorrection);
   
#ifdef OPENSIM
   debugpins_syncAck_set();
   debugpins_syncAck_clr();
#endif
}

void changeIsSync(bool newIsSync) {
   ieee154e_vars.isSync = newIsSync;
   
   if (ieee154e_vars.isSync==TRUE) {
      leds_sync_on();
      resetStats();
   } else {
      leds_sync_off();
      schedule_resetBackoff();
   }
}

//======= notifying upper layer

void notif_sendDone(OpenQueueEntry_t* packetSent, owerror_t error) {
   // record the outcome of the trasmission attempt
   packetSent->l2_sendDoneError   = error;
   // record the current ASN
   memcpy(&packetSent->l2_asn,&ieee154e_vars.asn,sizeof(asn_t));
   // associate this packet with the virtual component
   // COMPONENT_IEEE802154E_TO_RES so RES can knows it's for it
   packetSent->owner              = COMPONENT_IEEE802154E_TO_SIXTOP;
   // post RES's sendDone task
   scheduler_push_task(task_sixtopNotifSendDone,TASKPRIO_SIXTOP_NOTIF_TXDONE);
   // wake up the scheduler
   SCHEDULER_WAKEUP();
}

void notif_receive(OpenQueueEntry_t* packetReceived) {
   // record the current ASN
   memcpy(&packetReceived->l2_asn, &ieee154e_vars.asn, sizeof(asn_t));
   if (ieee154e_vars.isUnscheduledEB == FALSE) {
      // indicate reception to the schedule, to keep statistics
      schedule_indicateRx(&packetReceived->l2_asn);
   }
   // associate this packet with the virtual component
   // COMPONENT_IEEE802154E_TO_SIXTOP so sixtop can knows it's for it
   packetReceived->owner          = COMPONENT_IEEE802154E_TO_SIXTOP;
#ifdef GOLDEN_IMAGE_ROOT
   openserial_printInfo(COMPONENT_IEEE802154E,ERR_PACKET_SYNC,
                   (errorparameter_t)packetReceived->l2_asn.bytes0and1,
                   (errorparameter_t)packetReceived->l2_timeCorrection);
#endif
   // post RES's Receive task
   scheduler_push_task(task_sixtopNotifReceive,TASKPRIO_SIXTOP_NOTIF_RX);
   // wake up the scheduler
   SCHEDULER_WAKEUP();
}

//======= stats

port_INLINE void resetStats() {
   ieee154e_stats.numSyncPkt      =    0;
   ieee154e_stats.numSyncAck      =    0;
   ieee154e_stats.minCorrection   =  127;
   ieee154e_stats.maxCorrection   = -127;
   ieee154e_stats.numTicsOn       =    0;
   ieee154e_stats.numTicsTotal    =    0;
   // do not reset the number of de-synchronizations
}

void updateStats(PORT_SIGNED_INT_WIDTH timeCorrection) {
   // update minCorrection
   if (timeCorrection<ieee154e_stats.minCorrection) {
     ieee154e_stats.minCorrection = timeCorrection;
   }
   // update maxConnection
   if(timeCorrection>ieee154e_stats.maxCorrection) {
     ieee154e_stats.maxCorrection = timeCorrection;
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
port_INLINE uint8_t calculateFrequency(uint8_t channelOffset) {
    if (ieee154e_vars.singleChannel >= 11 && ieee154e_vars.singleChannel <= 26 ) {
        return ieee154e_vars.singleChannel; // single channel
    } else {
        // channel hopping enabled, use the channel depending on hopping template
        return 11 + ieee154e_vars.chTemplate[(ieee154e_vars.asnOffset+channelOffset)%16];
    }
    //return 11+(ieee154e_vars.asnOffset+channelOffset)%16; //channel hopping
}

/**
\brief Changes the state of the IEEE802.15.4e FSM.

Besides simply updating the state global variable,
this function toggles the FSM debug pin.

\param[in] newstate The state the IEEE802.15.4e FSM is now in.
*/
void changeState(ieee154e_state_t newstate) {
   // update the state
   ieee154e_vars.state = newstate;
   // wiggle the FSM debug pin
   switch (ieee154e_vars.state) {
      case S_SYNCLISTEN:
      case S_TXDATAOFFSET:
         debugpins_fsm_set();
         break;
      case S_SLEEP:
      case S_RXDATAOFFSET:
         debugpins_fsm_clr();
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
         debugpins_fsm_toggle();
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
void endSlot() {
   PORT_RADIOTIMER_WIDTH currentPeriod;
   uint8_t i;
   
   // turn off the radio
   radio_rfOff();
   // compute the duty cycle if radio has been turned on
   if (ieee154e_vars.radioOnThisSlot==TRUE){  
      ieee154e_vars.radioOnTics+=(radio_getTimerValue()-ieee154e_vars.radioOnInit);
   }
   // clear any pending timer
   radiotimer_cancel();
   
   // reset capturedTimes
   ieee154e_vars.lastCapturedTime = 0;
   ieee154e_vars.syncCapturedTime = 0;
   
   //computing duty cycle.
   ieee154e_stats.numTicsOn+=ieee154e_vars.radioOnTics;//accumulate and tics the radio is on for that window
   ieee154e_stats.numTicsTotal+=radio_getTimerPeriod();//increment total tics by timer period.

   if (ieee154e_stats.numTicsTotal>DUTY_CYCLE_WINDOW_LIMIT){
      ieee154e_stats.numTicsTotal = ieee154e_stats.numTicsTotal>>1;
      ieee154e_stats.numTicsOn    = ieee154e_stats.numTicsOn>>1;
   }

   //clear vars for duty cycle on this slot   
   ieee154e_vars.radioOnTics=0;
   ieee154e_vars.radioOnThisSlot=FALSE;
   
   // clean up dataToSend
   if (ieee154e_vars.dataToSend!=NULL) {
      // if everything went well, dataToSend was set to NULL in ti9
      // getting here means transmit failed
      
      // indicate Tx fail to schedule to update stats
      schedule_indicateTx(&ieee154e_vars.asn,FALSE);
      
      //decrement transmits left counter
      ieee154e_vars.dataToSend->l2_retriesLeft--;
      
      if (ieee154e_vars.dataToSend->l2_retriesLeft==0) {
         // indicate tx fail if no more retries left
         notif_sendDone(ieee154e_vars.dataToSend,E_FAIL);
      } else {
         // return packet to the virtual COMPONENT_SIXTOP_TO_IEEE802154E component
         ieee154e_vars.dataToSend->owner = COMPONENT_SIXTOP_TO_IEEE802154E;
      }
      
      // reset local variable
      ieee154e_vars.dataToSend = NULL;
   }
   
   // clean up dataReceived
   if (ieee154e_vars.dataReceived!=NULL) {
      // assume something went wrong. If everything went well, dataReceived
      // would have been set to NULL in ri9.
      // indicate  "received packet" to upper layer since we don't want to loose packets
      notif_receive(ieee154e_vars.dataReceived);
      // reset local variable
      ieee154e_vars.dataReceived = NULL;
   }
   
   // clean up ackToSend
   if (ieee154e_vars.ackToSend!=NULL) {
      // free ackToSend so corresponding RAM memory can be recycled
      openqueue_freePacketBuffer(ieee154e_vars.ackToSend);
      // reset local variable
      ieee154e_vars.ackToSend = NULL;
   }
   
   // clean up ackReceived
   if (ieee154e_vars.ackReceived!=NULL) {
      // free ackReceived so corresponding RAM memory can be recycled
      openqueue_freePacketBuffer(ieee154e_vars.ackReceived);
      // reset local variable
      ieee154e_vars.ackReceived = NULL;
   }
   
   // Let the mote wake up after numSkippedSlots timeslots
   // get current timer period
   currentPeriod = radio_getTimerPeriod();
   // set timer period
   radio_setTimerPeriod(currentPeriod+TsSlotDuration*ieee154e_vars.numSkippedSlots);
   //increase ASN by numSkippedSlots timeslots
   for (i=0;i<ieee154e_vars.numSkippedSlots;i++){
      incrementAsnOffset();
   }
#ifdef ADAPTIVE_SYNC
   // deal with the case when schedule multi slots
   adaptive_sync_countCompensationTimeout_compoundSlots(ieee154e_vars.numSkippedSlots);
#endif
   // reset numSkippedSlots
   ieee154e_vars.numSkippedSlots = 0;
   
   // change state
   changeState(S_SLEEP);
   
   // start serial activity
   if (ieee154e_vars.serialInputOutput==TRUE) {
      openserial_startInput();
   } else {
      openserial_startOutput();
   }
}

bool ieee154e_isSynch(){
   return ieee154e_vars.isSync;
}

void ieee154e_sendEB() {
   sync_IE_ht  sync_IE;
   
   // change state
   changeState(S_TXDATAOFFSET);
   // change owner
   ieee154e_vars.dataToSend->owner = COMPONENT_IEEE802154E;
   //copy synch IE  -- should be Little endian???
   // fill in the ASN field of the EB
   ieee154e_getAsn(sync_IE.asn);
   sync_IE.join_priority = (neighbors_getMyDAGrank()/MINHOPRANKINCREASE)-1; //poipoi -- use dagrank(rank)-1
   memcpy(ieee154e_vars.dataToSend->l2_ASNpayload,&sync_IE,sizeof(sync_IE_ht));
   // record that I attempt to transmit this packet
   ieee154e_vars.dataToSend->l2_numTxAttempts++;
   // arm tt1
   radiotimer_schedule(DURATION_tt1);
}
