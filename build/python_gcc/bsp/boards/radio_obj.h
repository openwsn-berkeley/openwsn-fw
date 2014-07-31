/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:02:32.274879.
*/
#ifndef __RADIO_H
#define __RADIO_H

/**
\addtogroup BSP
\{
\addtogroup radio
\{

\brief Cross-platform declaration "radio" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "radiotimer_obj.h"

//=========================== define ==========================================

#define LENGTH_CRC 2

/**
\brief Current state of the radio.

\note This radio driver is very minimal in that it does not follow a state machine.
      It is up to the MAC layer to ensure that the different radio operations 
      are called in the righr order. The radio keeps a state for debugging purposes only.
*/
typedef enum {
   RADIOSTATE_STOPPED             = 0x00,   ///< Completely stopped.
   RADIOSTATE_RFOFF               = 0x01,   ///< Listening for commands, but RF chain is off.
   RADIOSTATE_SETTING_FREQUENCY   = 0x02,   ///< Configuring the frequency.
   RADIOSTATE_FREQUENCY_SET       = 0x03,   ///< Done configuring the frequency.
   RADIOSTATE_LOADING_PACKET      = 0x04,   ///< Loading packet into the radio's TX buffer.
   RADIOSTATE_PACKET_LOADED       = 0x05,   ///< Packet is fully loaded in the radio's TX buffer.
   RADIOSTATE_ENABLING_TX         = 0x06,   ///< The RF TX chaing is being enabled (includes locking the PLL).
   RADIOSTATE_TX_ENABLED          = 0x07,   ///< Radio ready to transmit.
   RADIOSTATE_TRANSMITTING        = 0x08,   ///< Busy transmitting bytes.
   RADIOSTATE_ENABLING_RX         = 0x09,   ///< The RF RX chain is being enabled (includes locking the PLL).
   RADIOSTATE_LISTENING           = 0x0a,   ///< RF chain is on, listening, but no packet received yet.
   RADIOSTATE_RECEIVING           = 0x0b,   ///< Busy receiving bytes.
   RADIOSTATE_TXRX_DONE           = 0x0c,   ///< Frame has been sent/received completely.
   RADIOSTATE_TURNING_OFF         = 0x0d,   ///< Turning the RF chain off.
} radio_state_t;

//=========================== typedef =========================================

//=========================== variables =======================================

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

// admin
void radio_init(OpenMote* self);
void radio_setOverflowCb(OpenMote* self, radiotimer_compare_cbt cb);
void radio_setCompareCb(OpenMote* self, radiotimer_compare_cbt cb);
void radio_setStartFrameCb(OpenMote* self, radiotimer_capture_cbt cb);
void radio_setEndFrameCb(OpenMote* self, radiotimer_capture_cbt cb);
// reset
void radio_reset(OpenMote* self);
// timer
void radio_startTimer(OpenMote* self, PORT_TIMER_WIDTH period);
PORT_TIMER_WIDTH radio_getTimerValue(OpenMote* self);
void radio_setTimerPeriod(OpenMote* self, PORT_TIMER_WIDTH period);
PORT_TIMER_WIDTH radio_getTimerPeriod(OpenMote* self);
// RF admin
void radio_setFrequency(OpenMote* self, uint8_t frequency);
void radio_rfOn(OpenMote* self);
void radio_rfOff(OpenMote* self);
// TX
void radio_loadPacket(OpenMote* self, uint8_t* packet, uint8_t len);
void radio_txEnable(OpenMote* self);
void radio_txNow(OpenMote* self);
// RX
void radio_rxEnable(OpenMote* self);
void radio_rxNow(OpenMote* self);
void radio_getReceivedFrame(OpenMote* self, uint8_t* bufRead,
                                uint8_t* lenRead,
                                uint8_t  maxBufLen,
                                 int8_t* rssi,
                                uint8_t* lqi,
                                   bool* crc);

// interrupt handlers
kick_scheduler_t radio_isr(OpenMote* self);

/**
\}
\}
*/

#endif
