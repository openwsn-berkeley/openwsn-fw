#ifndef __RADIO_SUBGHZ_H
#define __RADIO_SUBGHZ_H

/**
\addtogroup BSP
\{
\addtogroup radio_subghz
\{

\brief Cross-platform declaration "radio_subghz" bsp module.

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
void     radio_subghz_setFunctions(radio_functions_t * funcs);
void     radio_subghz_powerOn(void);
void     radio_subghz_init(void);
void     radio_subghz_setStartFrameCb(radio_capture_cbt cb);
void     radio_subghz_setEndFrameCb(radio_capture_cbt cb);
// reset
void     radio_subghz_reset(void);
//===== reset

// RF admin
void     radio_subghz_setFrequency(uint16_t channel_spacing, uint32_t frequency_0, uint16_t channel);
void     radio_subghz_rfOn(void);
void     radio_subghz_rfOff(void);
void     radio_subghz_change_modulation(registerSetting_t * mod);
void     radio_subghz_change_size(uint16_t* size);
// TX
void     radio_subghz_loadPacket(uint8_t* packet, uint16_t len);
void     radio_subghz_txEnable(void);
void     radio_subghz_txNow(void);
// RX
void     radio_subghz_rxEnable(void);
void     radio_subghz_rxNow(void);
void     radio_subghz_getReceivedFrame(uint8_t* bufRead,
                                uint16_t* lenRead,
                                uint16_t  maxBufLen,
                                 int8_t*  rssi,
                                uint8_t*  lqi,
                                   bool*  crc,
                                uint8_t*  mcs);

void    radio_subghz_loadPacket_prepare(uint8_t* packet, uint8_t len);
void    radio_subghz_rxPacket_prepare(void);
void    radio_subghz_rxEnable_scum(void);

// interrupt handlers
void    radio_subghz_isr(void);

// some helpers for the MAC
uint8_t radio_subghz_getCRCLen(void);
uint8_t radio_subghz_calculateFrequency(uint8_t channelOffset, uint8_t asnOffset, uint8_t numChannels, uint8_t* hopSeq, bool singleChannel);
/**
\}
\}
*/

#endif