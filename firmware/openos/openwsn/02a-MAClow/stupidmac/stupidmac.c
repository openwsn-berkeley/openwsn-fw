/**
\brief Implementation of stupidMAC

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2010
*/

#include "openwsn.h"
#include "stupidmac.h"
#include "IEEE802154.h"
#include "radio.h"
#include "packetfunctions.h"
#include "idmanager.h"
#include "openserial.h"
#include "openqueue.h"
#include "timers.h"
#include "packetfunctions.h"
#include "neighbors.h"
#include "nores.h"

//=========================== variables =======================================

OpenQueueEntry_t*  stupidmac_dataFrameToSend;     //  NULL at beginning and end of slot
OpenQueueEntry_t*  stupidmac_packetACK;           //  NULL at beginning and end, free at end of slot
OpenQueueEntry_t*  stupidmac_dataFrameReceived;   // !NULL between data received, and sent to upper layer
uint8_t            stupidmac_dsn;
uint8_t            stupidmac_state;
#ifndef SERIALINSCHEDULER
bool               stupidmac_serialInOutputMode;
#endif

//=========================== prototypes ======================================

#include "IEEE802154_common.c"
void packetReceived();
void armRandomBackoffTimer();
void change_state(uint8_t newstate);

//======= from upper layer

//in stupidMAC, the radio is always on, listening
void stupidmac_init() {
   radio_rxOn(openwsn_frequency_channel);
   change_state(S_IDLE_LISTENING);
   stupidmac_dataFrameToSend = NULL;
   timer_startPeriodic(TIMER_MAC_PERIODIC,PERIODICTIMERPERIOD);
}

//a packet sent from the upper layer is simply stored into the OpenQueue buffer.
//The timerBackoff is armed to service the packet later on.
error_t stupidmac_send(OpenQueueEntry_t* msg) {
   //metadata
   msg->owner = COMPONENT_MAC;
   if (packetfunctions_isBroadcastMulticast(&(msg->l2_nextORpreviousHop))==TRUE) {
      msg->l2_retriesLeft = 1;
   } else {
      msg->l2_retriesLeft = TXRETRIES;
   }
   msg->l1_txPower = TX_POWER;
   msg->l1_channel = openwsn_frequency_channel;
   //IEEE802.15.4 header
   prependIEEE802154header(msg,
         msg->l2_frameType,
         IEEE154_SEC_NO_SECURITY,
         stupidmac_dsn++,
         &(msg->l2_nextORpreviousHop)
         );
   // space for 2-byte CRC
   packetfunctions_reserveFooterSize(msg,2);
   //simulate timer backoff fires so that packet gets sent immediately
   timer_mac_backoff_fired();
   return E_SUCCESS;
}

//======= from lower layer

void stupidmac_sendDone(OpenQueueEntry_t* pkt, error_t error) {
   switch (stupidmac_state) {
      case S_TX_TXDATA:                                           //[sendNowDone] transmitter
         if (error!=E_SUCCESS) {
            nores_sendDone(pkt,E_FAIL);
            stupidmac_dataFrameToSend = NULL;
            armRandomBackoffTimer();//arm timer to retransmission (?)
            change_state(S_IDLE_LISTENING);
            openserial_printError(COMPONENT_MAC,ERR_SENDNOWDONE_FAILED,
                  (errorparameter_t)stupidmac_state,
                  (errorparameter_t)0);
            return;
         } else {
            timer_startOneShot(TIMER_MAC_WATCHDOG,ACK_WAIT_TIME);
            change_state(S_TX_RXACK);
         }
         break;
      case S_RX_TXACK:                                            //[sendNowDone] receiver
         //I'm a receiver, finished sending ACK (end of RX sequence)
         openqueue_freePacketBuffer(stupidmac_packetACK);
         packetReceived();
         change_state(S_IDLE_LISTENING);
         break;
      default:
         openserial_printError(COMPONENT_MAC,ERR_WRONG_STATE_IN_SUBSEND_SENDDONE,
               (errorparameter_t)stupidmac_state,
               (errorparameter_t)0);
         change_state(S_IDLE_LISTENING);
         break;
   }
}

void radio_packet_received(OpenQueueEntry_t* msg) {
   ieee802154_header_iht   received_ieee154_header;
   ieee802154_header_iht   transmitted_ieee154_header;

   openserial_stop();
   //ensure debug fires only after packet fully received
   timer_startPeriodic(TIMER_MAC_PERIODIC,PERIODICTIMERPERIOD);

   msg->owner = COMPONENT_MAC;

   if (stupidmac_state!=S_TX_RXACK && stupidmac_state!=S_IDLE_LISTENING) {
      //not expecting this packet, throw away
      //do not go back to S_IDLE_LISTENING, just don't receive the packet and let the stupidmac_state machine be where it was
      openqueue_freePacketBuffer(msg);
      return;
   }

   received_ieee154_header = retrieveIEEE802154header(msg);
   packetfunctions_tossHeader(msg,received_ieee154_header.headerLength);
   packetfunctions_tossFooter(msg,2);

   msg->l2_frameType = received_ieee154_header.frameType;
   memcpy(&(msg->l2_nextORpreviousHop),&(received_ieee154_header.src),sizeof(open_addr_t));
   if (   received_ieee154_header.frameType==IEEE154_TYPE_DATA      &&
         !(idmanager_isMyAddress(&received_ieee154_header.panid))) {
      openserial_printError(COMPONENT_MAC,ERR_WRONG_PANID,
            (errorparameter_t)received_ieee154_header.panid.panid[0]*256+received_ieee154_header.panid.panid[1],
            (errorparameter_t)0);
      openqueue_freePacketBuffer(msg);
      return;
   }

   switch (stupidmac_state) {

      /*------------------- TX sequence ------------------------*/
      case S_TX_RXACK:                                            //[receive] transmitter
         transmitted_ieee154_header = retrieveIEEE802154header(stupidmac_dataFrameToSend);
         if (received_ieee154_header.dsn == transmitted_ieee154_header.dsn) {
            //I'm a transmitter, just received ACK (end of TX sequence)
            timer_stop(TIMER_MAC_WATCHDOG);
            neighbors_indicateTx(&(stupidmac_dataFrameToSend->l2_nextORpreviousHop),WAS_ACKED);
            nores_sendDone(stupidmac_dataFrameToSend,E_SUCCESS);
            stupidmac_dataFrameToSend = NULL;
            armRandomBackoffTimer();//arm timer for next transmission
            change_state(S_IDLE_LISTENING);
         }
         openqueue_freePacketBuffer(msg);//free packet I received         
         break;

         /*------------------- RX sequence ------------------------*/
      case S_IDLE_LISTENING:                                           //[receive] receiver
         //I'm a receiver, just received a packet
         if (received_ieee154_header.frameType==IEEE154_TYPE_DATA || received_ieee154_header.frameType==IEEE154_TYPE_CMD) {
            neighbors_indicateRx(&(msg->l2_nextORpreviousHop),msg->l1_rssi);
            if (idmanager_isMyAddress(&received_ieee154_header.dest)) {
               //this packet is unicast to me
               if (stupidmac_dataFrameReceived==NULL) {
                  stupidmac_dataFrameReceived = msg;
               } else {
                  openserial_printError(COMPONENT_MAC,ERR_BUSY_RECEIVING,
                        (errorparameter_t)0,
                        (errorparameter_t)0);
                  openqueue_freePacketBuffer(msg);
               }
               //the sender requests an ACK
               if (received_ieee154_header.ackRequested) {
                  change_state(S_RX_TXACKPREPARE);
                  stupidmac_packetACK = openqueue_getFreePacketBuffer();
                  if (stupidmac_packetACK!=NULL) {
                     //send ACK
                     stupidmac_packetACK->creator        = COMPONENT_MAC;
                     stupidmac_packetACK->owner          = COMPONENT_MAC;
                     stupidmac_packetACK->l1_txPower     = TX_POWER;
                     stupidmac_packetACK->l1_channel     = openwsn_frequency_channel;
                     stupidmac_packetACK->l2_retriesLeft = 1;
                     prependIEEE802154header(stupidmac_packetACK,
                           IEEE154_TYPE_ACK,
                           IEEE154_SEC_NO_SECURITY,
                           received_ieee154_header.dsn,
                           NULL);
                     packetfunctions_reserveFooterSize(stupidmac_packetACK,2);
                     change_state(S_RX_TXACKREADY);
                     change_state(S_RX_TXACK);
                     if (radio_send(stupidmac_packetACK)!=E_SUCCESS) {
                        //abort
                        openserial_printError(COMPONENT_MAC,ERR_PREPARESEND_FAILED,
                              (errorparameter_t)0,(errorparameter_t)2);
                        openqueue_freePacketBuffer(stupidmac_packetACK);
                        change_state(S_IDLE_LISTENING);
                     }
                  } else {
                     openserial_printError(COMPONENT_MAC,ERR_NO_FREE_PACKET_BUFFER,
                           (errorparameter_t)0,(errorparameter_t)0);
                     change_state(S_IDLE_LISTENING);
                     void packetReceived();
                     return;
                  }
               } else {
                  packetReceived();
               }
            } else if (packetfunctions_isBroadcastMulticast(&received_ieee154_header.dest)==TRUE) {
               //this packet is broadcast
               if (stupidmac_dataFrameReceived==NULL) {
                  stupidmac_dataFrameReceived = msg;
               } else {
                  openserial_printError(COMPONENT_MAC,ERR_BUSY_RECEIVING,
                        (errorparameter_t)1,
                        (errorparameter_t)0);
                  openqueue_freePacketBuffer(msg);
               }
               packetReceived();
            } else {
               openqueue_freePacketBuffer(msg);
            }
         } else {
            //not data. I could be an ACK but I'm not in S_TX_RXACK stupidmac_state, so I discard
            openqueue_freePacketBuffer(msg);
         }
         break;

      default:
         //this should never happen as error was caught above
         //do not go back to S_IDLE_LISTENING, just don't receive the packet and let the stupidmac_state machine be where it was
         openqueue_freePacketBuffer(msg);
         openserial_printError(COMPONENT_MAC,ERR_WRONG_STATE_IN_RECEIVE,
               (errorparameter_t)stupidmac_state,
               (errorparameter_t)0);
         break;
   }
}

//=========================== private =========================================

void packetReceived() {
   if (stupidmac_dataFrameReceived->length>0) {
      //packet contains payload destined to an upper layer
      nores_receive(stupidmac_dataFrameReceived);
   } else {
      //packet contains no payload (KA)
      openqueue_freePacketBuffer(stupidmac_dataFrameReceived);
   }
   stupidmac_dataFrameReceived = NULL;
}

void armRandomBackoffTimer() {
   timer_startOneShot(TIMER_MAC_BACKOFF,MINBACKOFF); //TODO randomize
}

void change_state(uint8_t newstate) {
   stupidmac_state = newstate;
   switch (newstate) {
      case S_TX_TXDATAPREPARE:
      case S_TX_TXDATA:
      case S_RX_TXACKPREPARE:
      case S_RX_TXACK:
         //atomic P3OUT |= 0x20;
         break;
      case S_TX_TXDATAREADY:
      case S_TX_RXACK:
      case S_RX_TXACKREADY:
      case S_IDLE_LISTENING:
         //atomic P3OUT &= ~0x20;
         break;
   }
}

bool stupidmac_debugPrint() {
   return FALSE;
}

//======= timers firing

//periodic timer used to transmit, and to trigger serial input/output
void timer_mac_periodic_fired() {
#ifndef SERIALINSCHEDULER
   openserial_stop();
#endif
   //trigger transmit
   armRandomBackoffTimer();
#ifndef SERIALINSCHEDULER
   //trigger serial input/output
   stupidmac_serialInOutputMode = !stupidmac_serialInOutputMode;
   if (stupidmac_serialInOutputMode) {
      openserial_startOutput();
   } else {
      openserial_startInput();
   }
#endif
}

//this function is the one which really initiates the transmission of a packet.
//It only does so if the MAC layer is in S_IDLE_LISTENING stupidmac_state, otherwise it defers
void timer_mac_backoff_fired() {
   if (stupidmac_state==S_IDLE_LISTENING) {
      if (stupidmac_dataFrameToSend!=NULL) {
         openserial_printError(COMPONENT_MAC,ERR_DATAFRAMETOSEND_ERROR,
               (errorparameter_t)0,
               (errorparameter_t)0);
      }
      stupidmac_dataFrameToSend = openqueue_inQueue(IS_NOT_ADV);
      if (stupidmac_dataFrameToSend==NULL) {
         stupidmac_dataFrameToSend = openqueue_inQueue(IS_ADV);
      }
      if (stupidmac_dataFrameToSend!=NULL) {
         change_state(S_TX_TXDATA);
         if (radio_send(stupidmac_dataFrameToSend)!=E_SUCCESS) {
            nores_sendDone(stupidmac_dataFrameToSend,E_FAIL);
            stupidmac_dataFrameToSend = NULL;
            armRandomBackoffTimer();//arm to retry later
            change_state(S_IDLE_LISTENING);
            openserial_printError(COMPONENT_MAC,ERR_PREPARESEND_FAILED,
                  (errorparameter_t)0,
                  (errorparameter_t)0);
         }
      }
   } else {
      //retry later on
      armRandomBackoffTimer();
   }
}

void timer_mac_watchdog_fired() {
   switch (stupidmac_state) {
      case S_TX_RXACK:
         //I'm a transmitter, didn't receive ACK (end of TX sequence).
         neighbors_indicateTx(&(stupidmac_dataFrameToSend->l2_nextORpreviousHop),WAS_NOT_ACKED);
         stupidmac_dataFrameToSend->l2_retriesLeft--;
         if (stupidmac_dataFrameToSend->l2_retriesLeft==0) {
            nores_sendDone(stupidmac_dataFrameToSend,E_FAIL);
            stupidmac_dataFrameToSend = NULL;
            armRandomBackoffTimer();
            change_state(S_IDLE_LISTENING);
            break;
         }
         //retransmit later on
         armRandomBackoffTimer();
         stupidmac_dataFrameToSend = NULL;
         change_state(S_IDLE_LISTENING);
         break;
      default:
         openserial_printError(COMPONENT_MAC,ERR_WRONG_STATE_IN_FASTTIMER_FIRED,
               (errorparameter_t)stupidmac_state,
               (errorparameter_t)0);
         change_state(S_IDLE_LISTENING);
         break;
   }
}