/**
\brief Cross-platform declaration "radio" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#ifndef __RADIO_H
#define __RADIO_H

#include "radiotimer.h"

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
   RADIOSTATE_RFOFF               = 0x01,   ///< Listening for commands by RF chain is off.
   RADIOSTATE_SETTING_FREQUENCY   = 0x02,   ///< Configuring the frequency.
   RADIOSTATE_FREQUENCY_SET       = 0x03,   ///< Done configuring the frequency.
   RADIOSTATE_LOADING_PACKET      = 0x04,   ///< Loading packet to send over SPI.
   RADIOSTATE_PACKET_LOADED       = 0x05,   ///< Packet is loaded in the TX buffer.
   RADIOSTATE_ENABLING_TX         = 0x06,   ///< The RF Tx chaing is being enabled (includes locked the PLL).
   RADIOSTATE_TX_ENABLED          = 0x07,   ///< Radio completely ready to transmit.
   RADIOSTATE_TRANSMITTING        = 0x08,   ///< Busy transmitting bytes.
   RADIOSTATE_ENABLING_RX         = 0x09,   ///< The RF Rx chaing is being enabled (includes locked the PLL).
   RADIOSTATE_LISTENING           = 0x0a,   ///< RF chain is on, listening, but no packet received yet.
   RADIOSTATE_RECEIVING           = 0x0b,   ///< Busy receiving bytes.
   RADIOSTATE_TXRX_DONE           = 0x0c,   ///< Frame has been sent/received completely.
   RADIOSTATE_TURNING_OFF         = 0x0d,   ///< Turning the RF chain off.
} radio_state_t;

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

// admin
void     radio_init();
void     radio_setOverflowCb(radiotimer_compare_cbt cb);
void     radio_setCompareCb(radiotimer_compare_cbt cb);
void     radio_setStartFrameCb(radiotimer_capture_cbt cb);
void     radio_setEndFrameCb(radiotimer_capture_cbt cb);
// reset
void     radio_reset();
// timer
void     radio_startTimer(PORT_TIMER_WIDTH period);
PORT_TIMER_WIDTH radio_getTimerValue();
void     radio_setTimerPeriod(PORT_TIMER_WIDTH period);
PORT_TIMER_WIDTH radio_getTimerPeriod();
// RF admin
void     radio_setFrequency(uint8_t frequency);
void     radio_rfOn();
void     radio_rfOff();
// TX
void     radio_loadPacket(uint8_t* packet, uint8_t len);
void     radio_txEnable();
void     radio_txNow();
// RX
void     radio_rxEnable();
void     radio_rxNow();
void     radio_getReceivedFrame(uint8_t* bufRead,
                                uint8_t* lenRead,
                                uint8_t  maxBufLen,
                                 int8_t* rssi,
                                uint8_t* lqi,
                                uint8_t* crc);

uint8_t  radio_isr();

#endif
