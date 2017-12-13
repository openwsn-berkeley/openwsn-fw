#ifndef __RADIO_SUBGHZ_H
#define __RADIO_SUBGHZ_H

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
//=========================== define ==========================================

#define LENGTH_CRC_SUBGHZ    4
#define NUM_CHANNELS_SUBGHZ  4 // number of channels to channel hop on




//=========================== typedef =========================================
//=========================== variables =======================================
//=========================== prototypes ======================================

// admin
void     radiosubghz_setFunctions(radio_functions_t * funcs);
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


void    radiosubghz_loadPacket_prepare(uint8_t* packet, uint8_t len);
void    radiosubghz_rxPacket_prepare(void);
void    radiosubghz_rxEnable_scum(void);

// interrupt handlers
void    radiosubghz_isr(void);

// some helpers for the MAC
uint8_t radiosubghz_getCRCLen(void);
uint8_t radiosubghz_calculateFrequency(uint8_t channelOffset, uint8_t asnOffset, uint8_t numChannels, uint8_t* hopSeq, bool singleChannel);


/**
\}
\}
*/

#endif