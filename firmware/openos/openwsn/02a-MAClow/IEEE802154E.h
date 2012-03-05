#ifndef __IEEE802154E_H
#define __IEEE802154E_H

/**
\addtogroup MAClow
\{
\addtogroup IEEE802154E
\{
*/

#include "openwsn.h"

//=========================== define ==========================================

#define SYNCHRONIZING_CHANNEL       26 // channel the mote listens on to synchronize
#define TXRETRIES                    3 // number of retries before declaring failed
#define TX_POWER                    31 // 1=-25dBm, 31=0dBm (max value)
#define RESYNCHRONIZATIONGUARD       5 // in 32kHz ticks. min distance to the end of the slot to succesfully synchronize
#define US_PER_TICK                 30 // number of us per 32kHz clock tick
#define KATIMEOUT                  500 // in slots: @10ms per slot ->  5 second
#define DESYNCTIMEOUT             1500 // in slots: @10ms per slot -> 15 seconds

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
   TsTxOffset                =  66,    //  2000us
   TsLongGT                  =  43,    //  1300us
   TsTxAckDelay              =  66,    //  2000us
   TsShortGT                 =  16,    //   500us
   TsSlotDuration            = 327,    // 10000us (328 ticks, but counter counts one extra count, see datasheet)
   // execution speed related
   maxTxDataPrepare          =  33,    //  1000us (measured 584 us)
   maxRxAckPrepare           =  10,    //   305us (measured  64 us)
   maxRxDataPrepare          =  13,    //   400us (measured  82 us)
   maxTxAckPrepare           =  10,    //   305us (measured 260 us)
   // radio speed related
   delayTx                   =   6,    //   180us (measured 200 us)  // between GO signal and SFD
   delayRx                   =   0,    //     0us (can not measure!) // between GO signal and start listening
   // radio watchdog
   wdRadioTx                 =  33,    //  1000us (needs to be >delayTx)
   wdDataDuration            = 164,    //  5000us (measured 4280us with max payload)
   wdAckDuration             =  98,    //  3000us (measured 1000us)
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
   int16_t timeCorrection;
} IEEE802154E_ACK_ht;

#define ADV_PAYLOAD_LENGTH 5

//=========================== variables =======================================

//=========================== prototypes ======================================


// admin
          void     ieee154e_init();
// public
__monitor uint16_t ieee154e_asnDiff(asn_t* someASN);
// events
          void     ieee154e_startOfFrame(uint16_t capturedTime);
          void     ieee154e_endOfFrame(uint16_t capturedTime);
// misc
          bool     debugPrint_asn();
          bool     debugPrint_isSync();
          bool     debugPrint_macStats();

/**
\}
\}
*/

#endif