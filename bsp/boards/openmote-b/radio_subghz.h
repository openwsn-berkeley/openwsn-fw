#ifndef __RADIO_H
#define __RADIO_H

/**
\addtogroup BSP
\{
\addtogroup radiosubghz
\{

\brief Cross-platform declaration "radiosubghz" bsp module.

\author Jonathan Munoz <jonathan.munoz@inria.fr>, July 2016.
\author Xavier Vilajosana (xvilajosana@eecs.berkeley.edu), Dec 2017

*/

#include "board.h"
#include "radio.h"
#include "at86rf215.h"
//=========================== define ==========================================

#define LENGTH_CRC 4

/**
\brief Current state of the radiosubghz.

\note This radiosubghz driver is very minimal in that it does not follow a state machine.
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
typedef void  (*radio_capture_cbt)(PORT_TIMER_WIDTH timestamp);

//=========================== variables =======================================
//=========================== prototypes ======================================

// admin
void     radiosubghz_powerOn(void);
void     radiosubghz_init(void);
void     radiosubghz_setStartFrameCb(radio_capture_cbt cb);
void     radiosubghz_setEndFrameCb(radio_capture_cbt cb);
// reset
void     radiosubghz_reset(void);
//===== reset

// RF admin
void     radiosubghz_setFrequency(uint16_t channel_spacing, uint32_t frequency_0, uint16_t channel);
void     radiosubghz_rfOn(void);
void     radiosubghz_rfOff(void);
void     radiosubghz_change_modulation(registerSetting_t * mod);
void     radiosubghz_change_size(uint16_t* size);
// TX
void     radiosubghz_loadPacket(uint8_t* packet, uint16_t len);
void     radiosubghz_txEnable(void);
void     radiosubghz_txNow(void);
// RX
void     radiosubghz_rxEnable(void);
void     radiosubghz_rxNow(void);
void     radiosubghz_getReceivedFrame(uint8_t* bufRead,
                                uint16_t* lenRead,
                                uint16_t  maxBufLen,
                                 int8_t*  rssi,
                                uint8_t*  lqi,
                                   bool*  crc,
                                uint8_t*  mcs);


// interrupt handlers
kick_scheduler_t   radiosubghz_isr(void);

/**
\}
\}
*/

#endif