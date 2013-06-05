#ifndef __IEEE802154E_H
#define __IEEE802154E_H

/**
\addtogroup MAClow
\{
\addtogroup IEEE802154E
\{
*/

#include "openwsn.h"
#include "board_info.h"
#include "schedule.h"

//=========================== debug define ====================================

//=========================== define ==========================================

#define SYNCHRONIZING_CHANNEL       20 // channel the mote listens on to synchronize
#define TXRETRIES                    3 // number of MAC retries before declaring failed
#define TX_POWER                    31 // 1=-25dBm, 31=0dBm (max value)
#define RESYNCHRONIZATIONGUARD       5 // in 32kHz ticks. min distance to the end of the slot to succesfully synchronize
#define US_PER_TICK                 30 // number of us per 32kHz clock tick
#define KATIMEOUT                   66 // in slots: @15ms per slot -> ~1 seconds
#define DESYNCTIMEOUT              333 // in slots: @15ms per slot -> ~5 seconds
#define LIMITLARGETIMECORRECTION     5 // threshold number of ticks to declare a timeCorrection "large"
#define LENGTH_IEEE154_MAX         128 // max length of a valid radio packet  

/**
When a packet is received, it is written inside the OpenQueueEntry_t->packet
buffer, starting at the byte defined below. When a packet is relayed, it
traverses the stack in which the MAC and IPHC headers are parsed and stripped
off, then put on again. During that process, the IPv6 hop limit field is
decremented. Depending on the new value of the hop limit, the IPHC header
compression algorithm might not be able to compress it, and hence has to carry
it inline, adding a byte to the header. To avoid having to move bytes around
inside OpenQueueEntry_t->packet buffer, we start writing the received packet a
bit after the start of the packet.
*/
#define FIRST_FRAME_BYTE             1

// the different states of the IEEE802.15.4e state machine
typedef enum {
   S_SLEEP                   = 0x00,   // ready for next slot
   // synchronizing
   S_SYNCLISTEN              = 0x01,   // listened for packet to synchronize to network
   S_SYNCRX                  = 0x02,   // receiving packet to synchronize to network
   S_SYNCPROC                = 0x03,   // processing packet just received
   // TX
   S_TXDATAOFFSET            = 0x04,   // waiting to prepare for Tx data
   S_TXDATAPREPARE           = 0x05,   // preparing for Tx data
   S_TXDATAREADY             = 0x06,   // ready to Tx data, waiting for 'go'
   S_TXDATADELAY             = 0x07,   // 'go' signal given, waiting for SFD Tx data
   S_TXDATA                  = 0x08,   // Tx data SFD received, sending bytes
   S_RXACKOFFSET             = 0x09,   // Tx data done, waiting to prepare for Rx ACK
   S_RXACKPREPARE            = 0x0a,   // preparing for Rx ACK
   S_RXACKREADY              = 0x0b,   // ready to Rx ACK, waiting for 'go'
   S_RXACKLISTEN             = 0x0c,   // idle listening for ACK
   S_RXACK                   = 0x0d,   // Rx ACK SFD received, receiving bytes
   S_TXPROC                  = 0x0e,   // processing sent data
   // RX
   S_RXDATAOFFSET            = 0x0f,   // waiting to prepare for Rx data
   S_RXDATAPREPARE           = 0x10,   // preparing for Rx data
   S_RXDATAREADY             = 0x11,   // ready to Rx data, waiting for 'go'
   S_RXDATALISTEN            = 0x12,   // idle listening for data
   S_RXDATA                  = 0x13,   // data SFD received, receiving more bytes
   S_TXACKOFFSET             = 0x14,   // waiting to prepare for Tx ACK
   S_TXACKPREPARE            = 0x15,   // preparing for Tx ACK
   S_TXACKREADY              = 0x16,   // Tx ACK ready, waiting for 'go'
   S_TXACKDELAY              = 0x17,   // 'go' signal given, waiting for SFD Tx ACK
   S_TXACK                   = 0x18,   // Tx ACK SFD received, sending bytes
   S_RXPROC                  = 0x19,   // processing received data
} ieee154e_state_t;

// Atomic durations
// expressed in 32kHz ticks:
//    - ticks = duration_in_seconds * 32768
//    - duration_in_seconds = ticks / 32768
enum ieee154e_atomicdurations_enum {
   // time-slot related
   TsTxOffset                =  131,                  //  4000us
   TsLongGT                  =   43,                  //  1300us
   TsTxAckDelay              =  151,                  //  4606us
   TsShortGT                 =   16,                  //   500us
   TsSlotDuration            =  PORT_TsSlotDuration,  // 15000us
   // execution speed related
   maxTxDataPrepare          =  PORT_maxTxDataPrepare,
   maxRxAckPrepare           =  PORT_maxRxAckPrepare,
   maxRxDataPrepare          =  PORT_maxRxDataPrepare,
   maxTxAckPrepare           =  PORT_maxTxAckPrepare,
   // radio speed related
   delayTx                   =  PORT_delayTx,         // between GO signal and SFD
   delayRx                   =  PORT_delayRx,         // between GO signal and start listening
   // radio watchdog
   wdRadioTx                 =   33,                  //  1000us (needs to be >delayTx)
   wdDataDuration            =  164,                  //  5000us (measured 4280us with max payload)
   wdAckDuration             =   98,                  //  3000us (measured 1000us)
};



// FSM timer durations (combinations of atomic durations)
// TX
#define DURATION_tt1 ieee154e_vars.lastCapturedTime+TsTxOffset-delayTx-maxTxDataPrepare
#define DURATION_tt2 ieee154e_vars.lastCapturedTime+TsTxOffset-delayTx
#define DURATION_tt3 ieee154e_vars.lastCapturedTime+TsTxOffset-delayTx+wdRadioTx
#define DURATION_tt4 ieee154e_vars.lastCapturedTime+wdDataDuration
#define DURATION_tt5 ieee154e_vars.lastCapturedTime+TsTxAckDelay-TsShortGT-delayRx-maxRxAckPrepare
#define DURATION_tt6 ieee154e_vars.lastCapturedTime+TsTxAckDelay-TsShortGT-delayRx
#define DURATION_tt7 ieee154e_vars.lastCapturedTime+TsTxAckDelay+TsShortGT
#define DURATION_tt8 ieee154e_vars.lastCapturedTime+wdAckDuration
// RX
#define DURATION_rt1 ieee154e_vars.lastCapturedTime+TsTxOffset-TsLongGT-delayRx-maxRxDataPrepare
#define DURATION_rt2 ieee154e_vars.lastCapturedTime+TsTxOffset-TsLongGT-delayRx
#define DURATION_rt3 ieee154e_vars.lastCapturedTime+TsTxOffset+TsLongGT
#define DURATION_rt4 ieee154e_vars.lastCapturedTime+wdDataDuration
#define DURATION_rt5 ieee154e_vars.lastCapturedTime+TsTxAckDelay-delayTx-maxTxAckPrepare
#define DURATION_rt6 ieee154e_vars.lastCapturedTime+TsTxAckDelay-delayTx
#define DURATION_rt7 ieee154e_vars.lastCapturedTime+TsTxAckDelay-delayTx+wdRadioTx
#define DURATION_rt8 ieee154e_vars.lastCapturedTime+wdAckDuration

//=========================== typedef =========================================

//IEEE802.15.4E acknowledgement (ACK)
typedef struct {
   PORT_SIGNED_INT_WIDTH timeCorrection;
} IEEE802154E_ACK_ht;

#define ADV_PAYLOAD_LENGTH 5

//=========================== module variables ================================

typedef struct {
   // misc
   asn_t              asn;                  // current absolute slot number
   slotOffset_t       slotOffset;           // current slot offset
   slotOffset_t       nextActiveSlotOffset; // next active slot offset
   PORT_TIMER_WIDTH   deSyncTimeout;        // how many slots left before looses sync
   bool               isSync;               // TRUE iff mote is synchronized to network
   // as shown on the chronogram
   ieee154e_state_t   state;                // state of the FSM
   OpenQueueEntry_t*  dataToSend;           // pointer to the data to send
   OpenQueueEntry_t*  dataReceived;         // pointer to the data received
   OpenQueueEntry_t*  ackToSend;            // pointer to the ack to send
   OpenQueueEntry_t*  ackReceived;          // pointer to the ack received
   PORT_TIMER_WIDTH   lastCapturedTime;     // last captured time
   PORT_TIMER_WIDTH   syncCapturedTime;     // captured time used to sync
   //channel hopping
   uint8_t            freq;                 // frequency of the current slot
   uint8_t            asnOffset;            // offset inside the frame
   
   PORT_TIMER_WIDTH radioOnInit;  //when within the slot the radio turns on
   PORT_TIMER_WIDTH radioOnTics;//how many tics within the slot the radio is on
   bool             radioOnThisSlot; //to control if the radio has been turned on in a slot.
} ieee154e_vars_t;

PRAGMA(pack(1));
typedef struct {
   uint8_t                   numSyncPkt;    // how many times synchronized on a non-ACK packet
   uint8_t                   numSyncAck;    // how many times synchronized on an ACK
   PORT_SIGNED_INT_WIDTH     minCorrection; // minimum time correction
   PORT_SIGNED_INT_WIDTH     maxCorrection; // maximum time correction
   uint8_t                   numDeSync;     // number of times a desync happened
   float                     dutyCycle;     // mac dutyCycle at each superframe
} ieee154e_stats_t;
PRAGMA(pack());

typedef struct {
   PORT_TIMER_WIDTH          num_newSlot;
   PORT_TIMER_WIDTH          num_timer;
   PORT_TIMER_WIDTH          num_startOfFrame;
   PORT_TIMER_WIDTH          num_endOfFrame;
} ieee154e_dbg_t;

//=========================== prototypes ======================================

// admin
void               ieee154e_init();
// public
PORT_TIMER_WIDTH   ieee154e_asnDiff(asn_t* someASN);
bool               ieee154e_isSynch();
void               ieee154e_getAsn(uint8_t* array);
// events
void               ieee154e_startOfFrame(PORT_TIMER_WIDTH capturedTime);
void               ieee154e_endOfFrame(PORT_TIMER_WIDTH capturedTime);
// misc
bool               debugPrint_asn();
bool               debugPrint_isSync();
bool               debugPrint_macStats();

/**
\}
\}
*/

#endif
