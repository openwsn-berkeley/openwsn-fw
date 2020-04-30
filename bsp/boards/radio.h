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

#include "board.h"

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

typedef enum {
   FREQ_TX                        = 0x01,
   FREQ_RX                        = 0x02,
} radio_freq_t;

// radio settings available for the MAC layer and supported by openmote-b
typedef enum 
{
    
    // different "modulations" (AT86RF215-specific)
    // At83rf215 settings start at index 0 because they are used directly in a sub-array in the atmel driver.
    RADIOSETTING_FSK_OPTION1_FEC,
    RADIOSETTING_OQPSK_RATE3,
    RADIOSETTING_OFDM_OPTION_1_MCS0,
    RADIOSETTING_OFDM_OPTION_1_MCS1,
    RADIOSETTING_OFDM_OPTION_1_MCS2,
    RADIOSETTING_OFDM_OPTION_1_MCS3,

    // default
    RADIOSETTING_24GHZ,
 
    // can be useful for switching receiver between OFDMx MCS modes.
    RADIOSETTING_NONE,
    
    RADIOSETTING_MAX 

} radioSetting_t;

//=========================== typedef =========================================

typedef void  (*radio_capture_cbt)(PORT_TIMER_WIDTH timestamp);

//=========================== variables =======================================

//=========================== prototypes ======================================

// admin
// used in MAC init
void                radio_init(void);
// referred to only inside projects. Never inside MAC or even drivers 
void                radio_powerOn(void);
// referred to in init and change of settings
void                radio_setStartFrameCb(radio_capture_cbt cb);
void                radio_setEndFrameCb(radio_capture_cbt cb);
// reset
// I don't see it referenced anywhere in cource code. there are some references in pbi and pbd files 
void                radio_reset(void);
// RF admin
// This function never sets frequency in fact. It sets a "channel". It shoud change accordingly. 
// Should be claculateChannel and it can take an 8 bit channel index.  
void                radio_setFrequency(uint16_t frequency, radio_freq_t tx_or_rx);

void                radio_setConfig(radioSetting_t radioSetting);

//referred to in MAC init and in some projects
void                radio_rfOn(void);
//referenced at end of each rf activity in the MAC
void                radio_rfOff(void);
// not referenced in MAC but referenced in some projects
int8_t              radio_getFrequencyOffset(void);
// TX
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
// why not put it under the scum implementation of loadpacket?
void                radio_loadPacket_prepare(uint8_t* packet, uint16_t len);
#endif


void                radio_loadPacket(uint8_t* packet, uint16_t len);
void                radio_txEnable(void);
void                radio_txNow(void);
// RX
//referenced in the MAC in scum context only.
// why not put it under the scum implementation of rxEnable?
void                radio_rxPacket_prepare(void);

void                radio_rxEnable(void);
//referenced in the MAC in scum context only: why not have an implementation of rxEnable
void                radio_rxEnable_scum(void);
void                radio_rxNow(void);
// some older implementations support only 8bit lenRead and maxBufLen. They need to be generalized to 16bit because of big packets and to keep a flexible radio interface for non-openwsn applications where lengh can be up to 16bit)
void                radio_getReceivedFrame(uint8_t* bufRead,
                                uint8_t* lenRead,
                                uint8_t  maxBufLen,
                                 int8_t* rssi,
                                uint8_t* lqi,
                                   bool* crc);

// interrupt handlers
// kick_scheduler_t: a generalized version of this will need to maintain void return.? 
kick_scheduler_t    radio_isr(void);

/**
\}
\}
*/

#endif
