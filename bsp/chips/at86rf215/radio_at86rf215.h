#ifndef __RADIO_AT86RF215_H
#define __RADIO_AT86RF215_H

/**
\addtogroup BSP
\{
\addtogroup radio
\{

\brief Cross-platform declaration "radio" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

/*
\modifications to implement the IEEE 802.15.4-SUN
\done by Jonathan Munoz <jonathan.munoz@inria.fr> 
*/

#include "radio.h"
#include "at86rf215.h"

//=========================== define ==========================================

#define LENGTH_CRC 2
// Number of modulations available on this radio chip. This is different from MAC_RADIOS which is for all the board and defined in board_info.h
#define MAX_MODULATIONS 3

//=========================== typedef =========================================

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


typedef struct {
    radio_capture_cbt           startFrame_cb;
    radio_capture_cbt           endFrame_cb;
    radio_state_t               state;
    uint8_t                     rf09_isr;
    uint8_t                     rf24_isr;
    uint8_t                     bb0_isr;
    uint8_t                     bb1_isr;
} radio_vars_t;




// open radio register mapping: an array of pointers to registersettings arrays

typedef struct {
    registerSetting_t * radios [MAX_MODULATIONS];
    size_t size [MAX_MODULATIONS];
}   radio_config_t;


//=========================== variables =======================================

//=========================== prototypes ======================================

// admin
void     radio_powerOn_at86rf215(void);
void     radio_init_at86rf215(void);
void     radio_setStartFrameCb_at86rf215(radio_capture_cbt cb);
void     radio_setEndFrameCb_at86rf215(radio_capture_cbt cb);
// reset
void     radio_reset_at86rf215(void);
// RF admin
void     radio_setFrequency_at86rf215(uint16_t channel, radio_freq_t tx_or_rx);
uint8_t  radio_getFrequencyOffset_at86rf215(void);
void     radio_rfOn_at86rf215(void);
void     radio_rfOff_at86rf215(void);
void     radio_set_modulation_at86rf215 (RADIO_TYPE selected_radio);
void     radio_change_modulation_at86rf215(registerSetting_t * mod);
void     radio_change_size_at86rf215(uint16_t* size);
// TX
void     radio_loadPacket_at86rf215(uint8_t* packet, uint16_t len);
radio_state_t    radio_getState_at86rf215(void);
void     radio_txEnable_at86rf215(void);
void     radio_txNow_at86rf215(void);
// RX
void     radio_rxEnable_at86rf215(void);
void     radio_rxNow_at86rf215(void);
void     radio_getReceivedFrame_at86rf215(uint8_t* bufRead,
                                uint16_t* lenRead,
                                uint16_t  maxBufLen,
                                 int8_t* rssi,
                                uint8_t* lqi,
                                   bool* crc);

// interrupt handlers
void    radio_isr_at86rf215(void);
// private
void     radio_local_bootstrap_at86rf215(void);
/**
\}
\}
*/

#endif
