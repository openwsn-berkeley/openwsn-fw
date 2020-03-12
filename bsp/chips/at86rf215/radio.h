#ifndef __RADIO_H
#define __RADIO_H

#include "board.h"
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
//=========================== define ==========================================

#define LENGTH_CRC 2
//definitions for open radio


//=========================== typedef =========================================


typedef enum {
   FREQ_TX                        = 0x01,
   FREQ_RX                        = 0x02,
} radio_freq_t;

typedef enum 
{
    FSK_OPTION1_FEC,
    OFDM_OPTION_1_MCS0,
    OFDM_OPTION_1_MCS1
} RADIO_TYPE;

typedef void                (*radio_capture_cbt)(PORT_TIMER_WIDTH timestamp);

//=========================== variables =======================================

//=========================== prototypes ======================================

// admin
void     radio_bootstrap(void);
void     radio_powerOn(void);
void     radio_init(void);
void     radio_setStartFrameCb(radio_capture_cbt cb);
void     radio_setEndFrameCb(radio_capture_cbt cb);
// reset
void     radio_reset(void);
// RF admin
void     radio_setFrequency(uint16_t channel, radio_freq_t tx_or_rx);
void     radio_rfOn(void);
void     radio_rfOff(void);
//void     radio_change_modulation(registerSetting_t * mod);
void     radio_set_modulation(RADIO_TYPE radio);
void     radio_change_size(uint16_t* size);
// TX
void     radio_loadPacket(uint8_t* packet, uint16_t len);
void     radio_txEnable(void);
void     radio_txNow(void);
// RX
void     radio_rxEnable(void);
void     radio_rxNow(void);
void     radio_getReceivedFrame(uint8_t* bufRead,
                                uint16_t* lenRead,
                                uint16_t  maxBufLen,
                                 int8_t* rssi,
                                uint8_t* lqi,
                                   bool* crc);

// interrupt handlers
void    radio_isr(void);

/**
\}
\}
*/

#endif
