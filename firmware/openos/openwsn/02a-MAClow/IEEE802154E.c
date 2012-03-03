#include "openwsn.h"
#include "IEEE802154E.h"
#include "radio.h"
#include "radiotimer.h"
#include "IEEE802154.h"
#include "openqueue.h"
#include "idmanager.h"
#include "openserial.h"
#include "schedule.h"
#include "packetfunctions.h"
#include "scheduler.h"
#include "leds.h"
#include "neighbors.h"
#include "debugpins.h"

//=========================== variables =======================================

typedef struct {
   // misc
   asn_t              asn;                  // current absolute slot number
   uint16_t           slotOffset;           // current slot offset (equal to slotOffset)
   uint16_t           deSyncTimeout;        // how many slots left before looses sync
   bool               isSync;               // TRUE iff mote is synchronized to network
   // as shown on the chronogram
   uint8_t            state;                // state of the FSM
   OpenQueueEntry_t*  dataToSend;           // pointer to the data to send
   OpenQueueEntry_t*  dataReceived;         // pointer to the data received
   OpenQueueEntry_t*  ackToSend;            // pointer to the ack to send
   OpenQueueEntry_t*  ackReceived;          // pointer to the ack received
   uint16_t           lastCapturedTime;     // last captured time
   uint16_t           syncCapturedTime;     // captured time used to sync
} ieee154e_vars_t;

ieee154e_vars_t ieee154e_vars;

// these statistics are reset every time they are reported
typedef struct {
   uint8_t            syncCounter;          // how many times we synchronized
   int16_t            minCorrection;        // minimum time correction
   int16_t            maxCorrection;        // maximum time correction
   uint8_t            numDeSync;            // number of times a desync happened
} ieee154e_stats_t;

ieee154e_stats_t ieee154e_stats;

//=========================== prototypes ======================================

// SYNCHRONIZING
void     activity_synchronize_newSlot();
void     activity_synchronize_startOfFrame(uint16_t capturedTime);
void     activity_synchronize_endOfFrame(uint16_t capturedTime);
// TX
void     activity_ti1ORri1();
void     activity_ti2();
void     activity_tie1();
void     activity_ti3();
void     activity_tie2();
void     activity_ti4(uint16_t capturedTime);
void     activity_tie3();
void     activity_ti5(uint16_t capturedTime);
void     activity_ti6();
void     activity_tie4();
void     activity_ti7();
void     activity_tie5();
void     activity_ti8(uint16_t capturedTime);
void     activity_tie6();
void     activity_ti9(uint16_t capturedTime);
// RX
void     activity_ri2();
void     activity_rie1();
void     activity_ri3();
void     activity_rie2();
void     activity_ri4(uint16_t capturedTime);
void     activity_rie3();
void     activity_ri5(uint16_t capturedTime);
void     activity_ri6();
void     activity_rie4();
void     activity_ri7();
void     activity_rie5();
void     activity_ri8(uint16_t capturedTime);
void     activity_rie6();
void     activity_ri9(uint16_t capturedTime);
// frame validity check
bool     isValidAdv(ieee802154_header_iht*     ieee802514_header);
bool     isValidRxFrame(ieee802154_header_iht* ieee802514_header);
bool     isValidAck(ieee802154_header_iht*     ieee802514_header,
                    OpenQueueEntry_t*          packetSent);
// ASN handling
void     incrementAsnOffset();
void     asnWriteToAdv(OpenQueueEntry_t* advFrame);
void     asnStoreFromAdv(OpenQueueEntry_t* advFrame);
// synchronization
void     synchronizePacket(uint16_t timeReceived);
void     synchronizeAck(int16_t timeCorrection);
void     changeIsSync(bool newIsSync);
// notifying upper layer
void     notif_sendDone(OpenQueueEntry_t* packetSent, error_t error);
void     notif_receive(OpenQueueEntry_t* packetReceived);
// statistics
void     resetStats();
void     updateStats(int16_t timeCorrection);
// misc
uint8_t  calculateFrequency(uint8_t channelOffset);
void     changeState(uint8_t newstate);
void     endSlot();
bool     debugPrint_asn();
bool     debugPrint_isSync();

//=========================== admin ===========================================

/**
\brief This function initializes this module.

Call this function once before any other function in this module, possibly
during boot-up.
*/
void ieee154e_init() {   
   // initialize debug pins
   DEBUG_PIN_FRAME_INIT();
   DEBUG_PIN_SLOT_INIT();
   DEBUG_PIN_FSM_INIT();
   
   // initialize variables
   ieee154e_vars.asn.byte4                  = 0;
   ieee154e_vars.asn.bytes2and3             = 0;
   ieee154e_vars.asn.bytes0and1             = 0;
   ieee154e_vars.slotOffset                 = 0;
   ieee154e_vars.deSyncTimeout              = 0;
   if (idmanager_getIsDAGroot()==TRUE) {
      changeIsSync(TRUE);
   } else {
      changeIsSync(FALSE);
   }
   ieee154e_vars.state                      = S_SLEEP;
   ieee154e_vars.dataToSend                 = NULL;
   ieee154e_vars.dataReceived               = NULL;
   ieee154e_vars.ackToSend                  = NULL;
   ieee154e_vars.ackReceived                = NULL;
   ieee154e_vars.lastCapturedTime           = 0;
   ieee154e_vars.syncCapturedTime           = 0;
   
   resetStats();
   ieee154e_stats.numDeSync                 = 0;
   
   // initialize (and start) IEEE802.15.4e timer
   radiotimer_init();
}

//=========================== public ==========================================

/**
/brief Difference between some older ASN and the current ASN.

\param someASN [in] some ASN to compare to the current

\returns The ASN difference, or 0xffff if more than 65535 different
*/
__monitor uint16_t ieee154e_asnDiff(asn_t* someASN) {
   uint16_t diff;
   
   if (ieee154e_vars.asn.byte4 != someASN->byte4) {
      return 0xffff;
   }
   
   diff = 0;
   if        (ieee154e_vars.asn.bytes2and3 == someASN->bytes2and3) {
      return ieee154e_vars.asn.bytes0and1-someASN->bytes0and1;
   } else if (ieee154e_vars.asn.bytes2and3-someASN->bytes2and3==1) {
      diff  = ieee154e_vars.asn.bytes0and1;
      diff += 0xffff-someASN->bytes0and1;
      diff += 1;
   } else {
      diff = 0xffff;
   }
   
   return diff;
}

//======= events

/**
\brief Indicates a new slot has just started.

This function executes in ISR mode, when the new slot timer fires.
*/
void isr_ieee154e_newSlot() {
   TACCR0 =  TsSlotDuration;
   if (ieee154e_vars.isSync==FALSE) {
      activity_synchronize_newSlot();
   } else {
      activity_ti1ORri1();
   }
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
}

/**
\brief Indicates the radio just received the first byte of a packet.

This function executes in ISR mode.
*/
void ieee154e_startOfFrame(uint16_t capturedTime) {
   if (ieee154e_vars.isSync==FALSE) {
      activity_synchronize_startOfFrame(capturedTime);
   } else {
      switch (ieee154e_vars.state) {
         case S_TXDATADELAY:
            activity_ti4(capturedTime);
            break;
         case S_RXACKLISTEN:
            activity_ti8(capturedTime);
            break;
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
}

/**
\brief Indicates the radio just received the last byte of a packet.

This function executes in ISR mode.
*/
void ieee154e_endOfFrame(uint16_t capturedTime) {
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
}

//======= misc

bool debugPrint_asn() {
   uint8_t output[ADV_PAYLOAD_LENGTH];
   
   output[0]  = (ieee154e_vars.asn.bytes0and1     & 0xff);
   output[1]  = (ieee154e_vars.asn.bytes0and1/256 & 0xff);
   output[2]  = (ieee154e_vars.asn.bytes2and3     & 0xff);
   output[3]  = (ieee154e_vars.asn.bytes2and3/256 & 0xff);
   output[4]  =  ieee154e_vars.asn.byte4;
   openserial_printStatus(STATUS_ASN,(uint8_t*)&output,sizeof(output));
   return TRUE;
}

bool debugPrint_isSync() {
   uint8_t output=0;
   output = ieee154e_vars.isSync;
   openserial_printStatus(STATUS_ISSYNC,(uint8_t*)&output,sizeof(uint8_t));
   return TRUE;
}

bool debugPrint_macStats() {
   // send current stats over serial
   openserial_printStatus(STATUS_MACSTATS,(uint8_t*)&ieee154e_stats,sizeof(ieee154e_stats_t));
   return TRUE;
}

//=========================== private =========================================

//======= SYNCHRONIZING

inline void activity_synchronize_newSlot() {
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
      
      // switch on the radio in Rx mode.
      radio_rxEnable();
      radio_rxNow();
   }

   // we want to be able to receive and transmist serial even when not synchronized
   // take turns every other slot to send or receive
   openserial_stop();
   if (ieee154e_vars.asn.byte4++%2==0) {
      openserial_startOutput();
   } else {
      openserial_startInput();
   }
}

inline void activity_synchronize_startOfFrame(uint16_t capturedTime) {
   
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

inline void activity_synchronize_endOfFrame(uint16_t capturedTime) {
   ieee802154_header_iht ieee802514_header;
   
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
   ieee154e_vars.dataReceived = openqueue_getFreePacketBuffer();
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
   
   // retrieve the received data frame from the radio's Rx buffer
   //poipoiradio_getReceivedFrame(ieee154e_vars.dataReceived);
   
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
      
      // break if invalid CRC
      if (ieee154e_vars.dataReceived->l1_crc==FALSE) {
         // break from the do-while loop and execute abort code below
         break;
      }
      
      // parse the IEEE802.15.4 header
      ieee802154_retrieveHeader(ieee154e_vars.dataReceived,&ieee802514_header);
      
      // store header details in packet buffer
      ieee154e_vars.dataReceived->l2_frameType = ieee802514_header.frameType;
      ieee154e_vars.dataReceived->l2_dsn       = ieee802514_header.dsn;
      memcpy(&(ieee154e_vars.dataReceived->l2_nextORpreviousHop),&(ieee802514_header.src),sizeof(open_addr_t));
      
      // toss the IEEE802.15.4 header
      packetfunctions_tossHeader(ieee154e_vars.dataReceived,ieee802514_header.headerLength);
      
      // if I just received a valid ADV, handle
      if (isValidAdv(&ieee802514_header)==TRUE) {
         
         // turn off the radio
         radio_rfOff();
         
         // record the ASN from the ADV payload
         asnStoreFromAdv(ieee154e_vars.dataReceived);
         
         // toss the ADV payload
         packetfunctions_tossHeader(ieee154e_vars.dataReceived,ADV_PAYLOAD_LENGTH);
         
         // synchronize (for the first time) to the sender's ADV
         synchronizePacket(ieee154e_vars.syncCapturedTime);
         
         // declare synchronized
         changeIsSync(TRUE);
         
         // log the "error"
         openserial_printError(COMPONENT_IEEE802154E,ERR_SYNCHRONIZED,
                               (errorparameter_t)ieee154e_vars.slotOffset,
                               (errorparameter_t)0);
         
         // send received ADV up the stack so RES can update statistics (synchronizing)
         notif_receive(ieee154e_vars.dataReceived);
         
         // clear local variable
         ieee154e_vars.dataReceived = NULL;
         
         // official end of synchronization
         endSlot();
         
         // everything went well, return here not to execute the error code below
         return;
      }
   } while (0);
   
   // free the (invalid) received data buffer so RAM memory can be recycled
   openqueue_freePacketBuffer(ieee154e_vars.dataReceived);
   
   // clear local variable
   ieee154e_vars.dataReceived = NULL;
}

//======= TX

inline void activity_ti1ORri1() {
   uint8_t cellType;
   open_addr_t neighbor;
   
   // increment ASN (do this first so debug pins are in sync)
   incrementAsnOffset();
   
   // wiggle debug pins
   DEBUG_PIN_SLOT_TOGGLE();
   if (ieee154e_vars.slotOffset==0) {
      DEBUG_PIN_FRAME_TOGGLE();
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

   // check the schedule to see what type of slot this is
   cellType = schedule_getType(ieee154e_vars.slotOffset);
   switch (cellType) {
      case CELLTYPE_OFF:
         // stop using serial
         openserial_stop();
         // abort the slot
         endSlot();
         //start outputing serial
         openserial_startOutput();
         break;
      case CELLTYPE_ADV:
         // stop using serial
         openserial_stop();
         // look for an ADV packet in the queue
         ieee154e_vars.dataToSend = openqueue_macGetAdvPacket();
         if (ieee154e_vars.dataToSend==NULL) {   // I will be listening for an ADV
            // change state
            changeState(S_RXDATAOFFSET);
            // arm rt1
            radiotimer_schedule(DURATION_rt1);
         } else {                                // I will be sending an ADV
            // change state
            changeState(S_TXDATAOFFSET);
            // change owner
            ieee154e_vars.dataToSend->owner = COMPONENT_IEEE802154E;
            // fill in the ASN field of the ADV
            asnWriteToAdv(ieee154e_vars.dataToSend);
            // record that I attempt to transmit this packet
            ieee154e_vars.dataToSend->l2_numTxAttempts++;
            // arm tt1
            radiotimer_schedule(DURATION_tt1);
         }
         break;
      case CELLTYPE_TXRX:
      case CELLTYPE_TX:
         // stop using serial
         openserial_stop();
         // check whether we can send
         if (schedule_getOkToSend(ieee154e_vars.slotOffset)) {
            schedule_getNeighbor(ieee154e_vars.slotOffset,&neighbor);
            ieee154e_vars.dataToSend = openqueue_macGetDataPacket(&neighbor);
         } else {
            ieee154e_vars.dataToSend = NULL;
         }
         if (ieee154e_vars.dataToSend!=NULL) {   // I have a packet to send
            // change state
            changeState(S_TXDATAOFFSET);
            // change owner
            ieee154e_vars.dataToSend->owner = COMPONENT_IEEE802154E;
            // record that I attempt to transmit this packet
            ieee154e_vars.dataToSend->l2_numTxAttempts++;
            // arm tt1
            radiotimer_schedule(DURATION_tt1);
         } else if (cellType==CELLTYPE_TX){
            // abort
            endSlot();
         }
         if (cellType==CELLTYPE_TX || 
             (cellType==CELLTYPE_TXRX && ieee154e_vars.dataToSend!=NULL)) {
            break;
         }
      case CELLTYPE_RX:
         // stop using serial
         openserial_stop();
         // change state
         changeState(S_RXDATAOFFSET);
         // arm rt1
         radiotimer_schedule(DURATION_rt1);
         break;
      case CELLTYPE_SERIALRX:
         // stop using serial
         openserial_stop();
         // abort the slot
         endSlot();
         //start inputting serial data
         openserial_startInput();
         break;
      case CELLTYPE_MORESERIALRX:
         // do nothing (not even endSlot())
         break;
      default:
         // stop using serial
         openserial_stop();
         // log the error
         openserial_printError(COMPONENT_IEEE802154E,ERR_WRONG_CELLTYPE,
                               (errorparameter_t)cellType,
                               (errorparameter_t)ieee154e_vars.slotOffset);
         // abort
         endSlot();
         break;
   }
}

inline void activity_ti2() {
   uint8_t frequency;
   
   // change state
   changeState(S_TXDATAPREPARE);

   // calculate the frequency to transmit on
   frequency = calculateFrequency(schedule_getChannelOffset(ieee154e_vars.slotOffset));

   // configure the radio for that frequency
   radio_setFrequency(frequency);

   // load the packet in the radio's Tx buffer
   //poipoiradio_loadPacket(ieee154e_vars.dataToSend);

   // enable the radio in Tx mode. This does not send the packet.
   radio_txEnable();

   // arm tt2
   radiotimer_schedule(DURATION_tt2);

   // change state
   changeState(S_TXDATAREADY);
}

inline void activity_tie1() {
   // log the error
   openserial_printError(COMPONENT_IEEE802154E,ERR_MAXTXDATAPREPARE_OVERFLOW,
                         (errorparameter_t)ieee154e_vars.state,
                         (errorparameter_t)ieee154e_vars.slotOffset);

   // abort
   endSlot();
}

inline void activity_ti3() {
   // change state
   changeState(S_TXDATADELAY);
   
   // arm tt3
   radiotimer_schedule(DURATION_tt3);
   
   // give the 'go' to transmit
   radio_txNow();
}

inline void activity_tie2() {
   // log the error
   openserial_printError(COMPONENT_IEEE802154E,ERR_WDRADIO_OVERFLOW,
                         (errorparameter_t)ieee154e_vars.state,
                         (errorparameter_t)ieee154e_vars.slotOffset);

   // abort
   endSlot();
}

inline void activity_ti4(uint16_t capturedTime) {
   // change state
   changeState(S_TXDATA);

   // cancel tt3
   radiotimer_cancel();

   // record the captured time
   ieee154e_vars.lastCapturedTime = capturedTime;
   
   // arm tt4
   radiotimer_schedule(DURATION_tt4);
}

inline void activity_tie3() {
   // log the error
   openserial_printError(COMPONENT_IEEE802154E,ERR_WDDATADURATION_OVERFLOWS,
                         (errorparameter_t)ieee154e_vars.state,
                         (errorparameter_t)ieee154e_vars.slotOffset);

   // abort
   endSlot();
}

inline void activity_ti5(uint16_t capturedTime) {
   bool listenForAck;
   
   // change state
   changeState(S_RXACKOFFSET);
   
   // cancel tt4
   radiotimer_cancel();
   
   // turn off the radio
   radio_rfOff();

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
      // indicate succesful Tx to schedule to keep statistics
      schedule_indicateTx(ieee154e_vars.slotOffset,&ieee154e_vars.asn,TRUE);
      // indicate to upper later the packet was sent successfully
      notif_sendDone(ieee154e_vars.dataToSend,E_SUCCESS);
      // reset local variable
      ieee154e_vars.dataToSend = NULL;
      // abort
      endSlot();
   }
}

inline void activity_ti6() {
   uint8_t frequency;
   
   // change state
   changeState(S_RXACKPREPARE);

   // calculate the frequency to transmit on
   frequency = calculateFrequency(schedule_getChannelOffset(ieee154e_vars.slotOffset));

   // configure the radio for that frequency
   radio_setFrequency(frequency);

   // enable the radio in Rx mode. The radio is not actively listening yet.
   radio_rxEnable();

   // arm tt6
   radiotimer_schedule(DURATION_tt6);
   
   // change state
   changeState(S_RXACKREADY);
}

inline void activity_tie4() {
   // log the error
   openserial_printError(COMPONENT_IEEE802154E,ERR_MAXRXACKPREPARE_OVERFLOWS,
                         (errorparameter_t)ieee154e_vars.state,
                         (errorparameter_t)ieee154e_vars.slotOffset);

   // abort
   endSlot();
}

inline void activity_ti7() {
   // change state
   changeState(S_RXACKLISTEN);

   // start listening
   radio_rxNow();

   // arm tt7
   radiotimer_schedule(DURATION_tt7);
}

inline void activity_tie5() {
   // indicate transmit failed to schedule to keep stats
   schedule_indicateTx(ieee154e_vars.slotOffset,&ieee154e_vars.asn,FALSE);
   
   // decrement transmits left counter
   ieee154e_vars.dataToSend->l2_retriesLeft--;

   if (ieee154e_vars.dataToSend->l2_retriesLeft==0) {
      // indicate tx fail if no more retries left
      notif_sendDone(ieee154e_vars.dataToSend,E_FAIL);
   } else {
      // return packet to the virtual COMPONENT_RES_TO_IEEE802154E component
      ieee154e_vars.dataToSend->owner = COMPONENT_RES_TO_IEEE802154E;
   }

   // reset local variable
   ieee154e_vars.dataToSend = NULL;

   // abort
   endSlot();
}

inline void activity_ti8(uint16_t capturedTime) {
   // change state
   changeState(S_RXACK);
   
   // cancel tt7
   radiotimer_cancel();

   // record the captured time
   ieee154e_vars.lastCapturedTime = capturedTime;

   // arm tt8
   radiotimer_schedule(DURATION_tt8);
}

inline void activity_tie6() {
   // abort
   endSlot();
}

inline void activity_ti9(uint16_t capturedTime) {
   ieee802154_header_iht ieee802514_header;
   volatile int16_t  timeCorrection;
   uint8_t byte0;
   uint8_t byte1;
   
   // change state
   changeState(S_TXPROC);
   
   // cancel tt8
   radiotimer_cancel();
   
   // turn off the radio
   radio_rfOff();

   // record the captured time
   ieee154e_vars.lastCapturedTime = capturedTime;
   
   // get a buffer to put the (received) ACK in
   ieee154e_vars.ackReceived = openqueue_getFreePacketBuffer();
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
   
   // retrieve the received ack frame from the radio's Rx buffer
   //poipoiradio_getReceivedFrame(ieee154e_vars.ackReceived);
   
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
      
      // break if invalid CRC
      if (ieee154e_vars.ackReceived->l1_crc==FALSE) {
         // break from the do-while loop and execute the clean-up code below
         break;
      }
      
      // parse the IEEE802.15.4 header
      ieee802154_retrieveHeader(ieee154e_vars.ackReceived,&ieee802514_header);
      
      // store header details in packet buffer
      ieee154e_vars.ackReceived->l2_frameType  = ieee802514_header.frameType;
      ieee154e_vars.ackReceived->l2_dsn        = ieee802514_header.dsn;
      memcpy(&(ieee154e_vars.ackReceived->l2_nextORpreviousHop),&(ieee802514_header.src),sizeof(open_addr_t));
      
      // toss the IEEE802.15.4 header
      packetfunctions_tossHeader(ieee154e_vars.ackReceived,ieee802514_header.headerLength);
      
      // if frame is a valid ACK, handle
      if (isValidAck(&ieee802514_header,ieee154e_vars.dataToSend)==TRUE) {
         
         // resynchronize if I'm not a DAGroot and ACK from preferred parent
         if (idmanager_getIsDAGroot()==FALSE &&
             neighbors_isPreferredParent(&(ieee154e_vars.ackReceived->l2_nextORpreviousHop)) ) {
            byte0 = ieee154e_vars.ackReceived->payload[0];
            byte1 = ieee154e_vars.ackReceived->payload[1];
            timeCorrection  = (int16_t)((uint16_t)byte1<<8 | (uint16_t)byte0);
            timeCorrection /=  US_PER_TICK;
            timeCorrection  = -timeCorrection;
            synchronizeAck(timeCorrection);
         }
         
         // inform schedule of successful transmission
         schedule_indicateTx(ieee154e_vars.slotOffset,&ieee154e_vars.asn,TRUE);
         
         // inform upper layer
         notif_sendDone(ieee154e_vars.dataToSend,E_SUCCESS);
         ieee154e_vars.dataToSend = NULL;
      }
      
      // in any case, execute the clean-up code below
   } while (0);
   
   // free the received ack so corresponding RAM memory can be recycled
   openqueue_freePacketBuffer(ieee154e_vars.ackReceived);
   
   // clear local variable
   ieee154e_vars.ackReceived = NULL;

   // official end of Tx slot
   endSlot();
}

//======= RX

inline void activity_ri2() {
   uint8_t frequency;
   
   // change state
   changeState(S_RXDATAPREPARE);

   // calculate the frequency to transmit on
   frequency = calculateFrequency(schedule_getChannelOffset(ieee154e_vars.slotOffset) );

   // configure the radio for that frequency
   radio_setFrequency(frequency);

   // enable the radio in Rx mode. The radio does not actively listen yet.
   radio_rxEnable();

   // arm rt2
   radiotimer_schedule(DURATION_rt2);

   // change state
   changeState(S_RXDATAREADY);
}

inline void activity_rie1() {
   // log the error
   openserial_printError(COMPONENT_IEEE802154E,ERR_MAXRXDATAPREPARE_OVERFLOWS,
                         (errorparameter_t)ieee154e_vars.state,
                         (errorparameter_t)ieee154e_vars.slotOffset);

   // abort
   endSlot();
}

inline void activity_ri3() {
   // change state
   changeState(S_RXDATALISTEN);

   // give the 'go' to receive
   radio_rxNow();

   // arm rt3
   radiotimer_schedule(DURATION_rt3);
}

inline void activity_rie2() {
   // abort
   endSlot();
}

inline void activity_ri4(uint16_t capturedTime) {
   // change state
   changeState(S_RXDATA);
   
   // cancel rt3
   radiotimer_cancel();

   // record the captured time
   ieee154e_vars.lastCapturedTime = capturedTime;
   
   // record the captured time to sync
   ieee154e_vars.syncCapturedTime = capturedTime;

   // arm rt4
   radiotimer_schedule(DURATION_rt4);
}

inline void activity_rie3() {
   // log the error
   openserial_printError(COMPONENT_IEEE802154E,ERR_WDDATADURATION_OVERFLOWS,
                         (errorparameter_t)ieee154e_vars.state,
                         (errorparameter_t)ieee154e_vars.slotOffset);
   
   // abort
   endSlot();
}

inline void activity_ri5(uint16_t capturedTime) {
   ieee802154_header_iht ieee802514_header;
   
   // change state
   changeState(S_TXACKOFFSET);
   
   // cancel rt4
   radiotimer_cancel();
   
   // turn off the radio
   radio_rfOff();
   
   // get a buffer to put the (received) data in
   ieee154e_vars.dataReceived = openqueue_getFreePacketBuffer();
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
   
   // retrieve the received data frame from the radio's Rx buffer
   //poipoiradio_getReceivedFrame(ieee154e_vars.dataReceived);

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
      
      // if CRC doesn't check, stop
      if (ieee154e_vars.dataReceived->l1_crc==FALSE) {
         // jump to the error code below this do-while loop
         break;
      }
      
      // parse the IEEE802.15.4 header
      ieee802154_retrieveHeader(ieee154e_vars.dataReceived,&ieee802514_header);
      
      // store header details in packet buffer
      ieee154e_vars.dataReceived->l2_frameType = ieee802514_header.frameType;
      ieee154e_vars.dataReceived->l2_dsn       = ieee802514_header.dsn;
      memcpy(&(ieee154e_vars.dataReceived->l2_nextORpreviousHop),&(ieee802514_header.src),sizeof(open_addr_t));
      
      // toss the IEEE802.15.4 header
      packetfunctions_tossHeader(ieee154e_vars.dataReceived,ieee802514_header.headerLength);
      
      // if I just received a valid ADV, record the ASN and toss the payload
      if (isValidAdv(&ieee802514_header)==TRUE) {
         if (idmanager_getIsDAGroot()==FALSE) {
            asnStoreFromAdv(ieee154e_vars.dataReceived);
         }
         // toss the ADV payload
         packetfunctions_tossHeader(ieee154e_vars.dataReceived,ADV_PAYLOAD_LENGTH);
      }
      
      // record the captured time
      ieee154e_vars.lastCapturedTime = capturedTime;
      
      // if I just received an invalid frame, stop
      if (isValidRxFrame(&ieee802514_header)==FALSE) {
         // jump to the error code below this do-while loop
         break;
      }
      
      // check if ack requested
      if (ieee802514_header.ackRequested==1) {
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

inline void activity_ri6() {
   int16_t timeCorrection;
   uint8_t frequency;
   
   // change state
   changeState(S_TXACKPREPARE);
   
   // get a buffer to put the ack to send in
   ieee154e_vars.ackToSend = openqueue_getFreePacketBuffer();
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
   
   // calculate the time timeCorrection
   timeCorrection = (int16_t)((int16_t)ieee154e_vars.syncCapturedTime-(int16_t)TsTxOffset);
   
   // add the payload to the ACK (i.e. the timeCorrection)
   packetfunctions_reserveHeaderSize(ieee154e_vars.ackToSend,sizeof(IEEE802154E_ACK_ht));
   timeCorrection  = -timeCorrection;
   timeCorrection *= US_PER_TICK;
   ieee154e_vars.ackToSend->payload[0] = (uint8_t)((((uint16_t)timeCorrection)   ) & 0xff);
   ieee154e_vars.ackToSend->payload[1] = (uint8_t)((((uint16_t)timeCorrection)>>8) & 0xff);
   
   // prepend the IEEE802.15.4 header to the ACK
   ieee154e_vars.ackToSend->l2_frameType = IEEE154_TYPE_ACK;
   ieee154e_vars.ackToSend->l2_dsn       = ieee154e_vars.dataReceived->l2_dsn;
   ieee802154_prependHeader(ieee154e_vars.ackToSend,
                            ieee154e_vars.ackToSend->l2_frameType,
                            IEEE154_SEC_NO_SECURITY,
                            ieee154e_vars.dataReceived->l2_dsn,
                            &(ieee154e_vars.dataReceived->l2_nextORpreviousHop)
                            );
   
   // space for 2-byte CRC
   packetfunctions_reserveFooterSize(ieee154e_vars.ackToSend,2);
   
   // calculate the frequency to transmit on
   frequency = calculateFrequency(schedule_getChannelOffset(ieee154e_vars.slotOffset) );
   
   // configure the radio for that frequency
   radio_setFrequency(frequency);
   
   // load the packet in the radio's Tx buffer
   //poipoiradio_loadPacket(ieee154e_vars.ackToSend);
   
   // enable the radio in Tx mode. This does not send that packet.
   radio_txEnable();
   
   // arm rt6
   radiotimer_schedule(DURATION_rt6);
   
   // change state
   changeState(S_TXACKREADY);
}

inline void activity_rie4() {
   // log the error
   openserial_printError(COMPONENT_IEEE802154E,ERR_MAXTXACKPREPARE_OVERFLOWS,
                         (errorparameter_t)ieee154e_vars.state,
                         (errorparameter_t)ieee154e_vars.slotOffset);
   
   // abort
   endSlot();
}

inline void activity_ri7() {
   // change state
   changeState(S_TXACKDELAY);
   
   // arm rt7
   radiotimer_schedule(DURATION_rt7);
   
   // give the 'go' to transmit
   radio_txNow();
}

inline void activity_rie5() {
   // log the error
   openserial_printError(COMPONENT_IEEE802154E,ERR_WDRADIOTX_OVERFLOWS,
                         (errorparameter_t)ieee154e_vars.state,
                         (errorparameter_t)ieee154e_vars.slotOffset);
   
   // abort
   endSlot();
}

inline void activity_ri8(uint16_t capturedTime) {
   // change state
   changeState(S_TXACK);
   
   // cancel rt7
   radiotimer_cancel();
   
   // record the captured time
   ieee154e_vars.lastCapturedTime = capturedTime;
   
   // arm rt8
   radiotimer_schedule(DURATION_rt8);
}

inline void activity_rie6() {
   // log the error
   openserial_printError(COMPONENT_IEEE802154E,ERR_WDACKDURATION_OVERFLOWS,
                         (errorparameter_t)ieee154e_vars.state,
                         (errorparameter_t)ieee154e_vars.slotOffset);
   
   // abort
   endSlot();
}

inline void activity_ri9(uint16_t capturedTime) {
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
\brief Decides whether the packet I just received is a valid ADV

\param [in] ieee802514_header IEEE802.15.4 header of the packet I just received

\returns TRUE if packet is a valid ADV, FALSE otherwise
*/
inline bool isValidAdv(ieee802154_header_iht* ieee802514_header) {
   return ieee802514_header->valid==TRUE                                                              && \
          ieee802514_header->frameType==IEEE154_TYPE_BEACON                                           && \
          packetfunctions_sameAddress(&ieee802514_header->panid,idmanager_getMyID(ADDR_PANID))        && \
          ieee154e_vars.dataReceived->length==ADV_PAYLOAD_LENGTH;
}

/**
\brief Decides whether the packet I just received is valid received frame.

A valid Rx frame satisfies the following constraints:
- its IEEE802.15.4 header is well formatted
- its a DATA of BEACON frame (i.e. not ACK and not COMMAND)
- its sent on the same PANid as mine
- its for me (unicast or broadcast)
- 

\param [in] ieee802514_header IEEE802.15.4 header of the packet I just received

\returns TRUE if packet is valid received frame, FALSE otherwise
*/
inline bool isValidRxFrame(ieee802154_header_iht* ieee802514_header) {
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
\brief Decides whether the packet I just received is a valid ACK

A packet is a valid ACK if it satisfies the following conditions:
- the IEEE802.15.4 header is valid
- the frame type is 'ACK'
- the sequence number in the ACK matches the sequence number of the packet sent
- the ACK contains my PANid
- the packet is unicast to me
- the packet comes from the neighbor I sent the data to

\param [in] ieee802514_header IEEE802.15.4 header of the packet I just received
\param [in] packetSent points to the packet I just sent

\returns TRUE if packet is a valid ACK, FALSE otherwise
*/
inline bool isValidAck(ieee802154_header_iht* ieee802514_header,
                       OpenQueueEntry_t*      packetSent) {
   /*return ieee802514_header->valid==TRUE                                                           && \
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

inline void incrementAsnOffset() {
   // increment the asn
   ieee154e_vars.asn.bytes0and1++;
   if (ieee154e_vars.asn.bytes0and1==0) {
      ieee154e_vars.asn.bytes2and3++;
      if (ieee154e_vars.asn.bytes2and3==0) {
         ieee154e_vars.asn.byte4++;
      }
   }
   // increment the offset
   ieee154e_vars.slotOffset = (ieee154e_vars.slotOffset+1)%SCHEDULELENGTH;
}

inline void asnWriteToAdv(OpenQueueEntry_t* advFrame) {
   advFrame->l2_payload[0]        = (ieee154e_vars.asn.bytes0and1     & 0xff);
   advFrame->l2_payload[1]        = (ieee154e_vars.asn.bytes0and1/256 & 0xff);
   advFrame->l2_payload[2]        = (ieee154e_vars.asn.bytes2and3     & 0xff);
   advFrame->l2_payload[3]        = (ieee154e_vars.asn.bytes2and3/256 & 0xff);
   advFrame->l2_payload[4]        =  ieee154e_vars.asn.byte4;
}

inline void asnStoreFromAdv(OpenQueueEntry_t* advFrame) {
   // store the ASN
   ieee154e_vars.asn.bytes0and1   =     ieee154e_vars.dataReceived->payload[0]+
                                    256*ieee154e_vars.dataReceived->payload[1];
   ieee154e_vars.asn.bytes2and3   =     ieee154e_vars.dataReceived->payload[2]+
                                    256*ieee154e_vars.dataReceived->payload[3];
   ieee154e_vars.asn.byte4        =     ieee154e_vars.dataReceived->payload[4];
   // determine the current slotOffset
   /*
   Note: this is a bit of a hack. Normally, slotOffset=ASN%slotlength. But since
   the ADV is exchanged in slot 0, we know that we're currently at slotOffset==0
   */
   ieee154e_vars.slotOffset       = 0;
   
}

//======= synchronization

void synchronizePacket(uint16_t timeReceived) {
   int16_t  timeCorrection;
   uint16_t newTaccr0;
   uint16_t currentTar;
   uint16_t currentTaccr0;
   // record the current states of the TAR and TACCR0 registers
   currentTar           =  TAR;
   currentTaccr0        =  TACCR0;
   // calculate new value for TACCR0
   timeCorrection    =  (int16_t)((int16_t)timeReceived-(int16_t)TsTxOffset);
   newTaccr0         =  TsSlotDuration;
   // detect whether I'm too close to the edge of the slot, in that case,
   // skip a slot and increase the temporary slot length to be 2 slots long
   if (currentTar<timeReceived ||
       currentTaccr0-currentTar<RESYNCHRONIZATIONGUARD) {
      DEBUG_PIN_SLOT_TOGGLE();
      TACTL         &= ~TAIFG;
      newTaccr0     +=  TsSlotDuration;
      incrementAsnOffset();
      DEBUG_PIN_SLOT_TOGGLE();
   }
   newTaccr0         =  (uint16_t)((int16_t)newTaccr0+timeCorrection);
   TACCR0            =  newTaccr0;
   ieee154e_vars.deSyncTimeout = DESYNCTIMEOUT;
   // update statistics
   updateStats(timeCorrection);
}

void synchronizeAck(int16_t timeCorrection) {
   uint16_t newTaccr0;
   uint16_t currentTaccr0;
   // resynchronize
   currentTaccr0     =  TACCR0;
   newTaccr0         =  (uint16_t)((int16_t)currentTaccr0-timeCorrection);
   TACCR0            =  newTaccr0;
   ieee154e_vars.deSyncTimeout = DESYNCTIMEOUT;
   // update statistics
   updateStats(timeCorrection);
}

void changeIsSync(bool newIsSync) {
   ieee154e_vars.isSync = newIsSync;
   if (ieee154e_vars.isSync==TRUE) {
      leds_sync_on();
      resetStats();
   } else {
      leds_sync_off();
   }
}

//======= notifying upper layer

void notif_sendDone(OpenQueueEntry_t* packetSent, error_t error) {
   // record the outcome of the trasmission attempt
   packetSent->l2_sendDoneError   = error;
   // record the current ASN
   memcpy(&packetSent->l2_asn,&ieee154e_vars.asn,sizeof(asn_t));
   // associate this packet with the virtual component
   // COMPONENT_IEEE802154E_TO_RES so RES can knows it's for it
   packetSent->owner              = COMPONENT_IEEE802154E_TO_RES;
   // post RES's sendDone task
   scheduler_push_task(TASKID_RESNOTIF_TXDONE);
   // wake up the scheduler
   SCHEDULER_WAKEUP();
}

void notif_receive(OpenQueueEntry_t* packetReceived) {
   // record the current ASN
   memcpy(&packetReceived->l2_asn, &ieee154e_vars.asn, sizeof(asn_t));
   // indicate reception to the schedule, to keep statistics
   schedule_indicateRx(ieee154e_vars.slotOffset,&packetReceived->l2_asn);
   // associate this packet with the virtual component
   // COMPONENT_IEEE802154E_TO_RES so RES can knows it's for it
   packetReceived->owner          = COMPONENT_IEEE802154E_TO_RES;
   // post RES's Receive task
   scheduler_push_task(TASKID_RESNOTIF_RX);
   // wake up the scheduler
   SCHEDULER_WAKEUP();
}

//======= stats

inline void resetStats() {
   ieee154e_stats.syncCounter     =    0;
   ieee154e_stats.minCorrection   =  127;
   ieee154e_stats.maxCorrection   = -127;
   // do not reset the number of de-synchronizations
}

void updateStats(int16_t timeCorrection) {
   
   ieee154e_stats.syncCounter++;
   
   if (timeCorrection<ieee154e_stats.minCorrection) {
     ieee154e_stats.minCorrection = timeCorrection;
   }
   
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

\param [in] channelOffset channel offset for the current slot

\returns The calculated frequency channel, an integer between 11 and 26.
*/
inline uint8_t calculateFrequency(uint8_t channelOffset) {
   //return 11+(asn+channelOffset)%16;
   // poipoi: no channel hopping
   return 26;
}

/**
\brief Changes the state of the IEEE802.15.4e FSM.

Besides simply updating the state global variable,
this function toggles the FSM debug pin.

\param [in] newstate The state the IEEE802.15.4e FSM is now in.
*/
void changeState(uint8_t newstate) {
   // update the state
   ieee154e_vars.state = newstate;
   // wiggle the FSM debug pin
   switch (ieee154e_vars.state) {
      case S_SYNCLISTEN:
      case S_TXDATAOFFSET:
         DEBUG_PIN_FSM_SET();
         break;
      case S_SLEEP:
      case S_RXDATAOFFSET:
         DEBUG_PIN_FSM_CLR();
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
         DEBUG_PIN_FSM_TOGGLE();
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
   // turn off the radio
   radio_rfOff();
   
   // clear any pending timer
   radiotimer_cancel();
   
   // reset capturedTimes
   ieee154e_vars.lastCapturedTime = 0;
   ieee154e_vars.syncCapturedTime = 0;
   
   // clean up dataToSend
   if (ieee154e_vars.dataToSend!=NULL) {
      // if everything went well, dataToSend was set to NULL in ti9
      // getting here means transmit failed
      
      // indicate Tx fail to schedule to update stats
      schedule_indicateTx(ieee154e_vars.slotOffset,&ieee154e_vars.asn,FALSE);
      
      //decrement transmits left counter
      ieee154e_vars.dataToSend->l2_retriesLeft--;
      
      if (ieee154e_vars.dataToSend->l2_retriesLeft==0) {
         // indicate tx fail if no more retries left
         notif_sendDone(ieee154e_vars.dataToSend,E_FAIL);
      } else {
         // return packet to the virtual COMPONENT_RES_TO_IEEE802154E component
         ieee154e_vars.dataToSend->owner = COMPONENT_RES_TO_IEEE802154E;
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
   
   // change state
   changeState(S_SLEEP);
}