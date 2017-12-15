#ifndef __RADIO_2D4GHZ_H
#define __RADIO_2D4GHZ_H

/**
\addtogroup BSP
\{
\addtogroup radio_2d4ghz
\{

\brief Cross-platform declaration "radio" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "radio.h"

//=========================== define ==========================================

/**
\brief Current state of the radio 2.4ghz.

\note This radio driver is very minimal in that it does not follow a state machine.
      It is up to the MAC layer to ensure that the different radio operations 
      are called in the righr order. The radio keeps a state for debugging purposes only.
*/

#define LENGTH_CRC      2
#define delayTx_2D4GHZ 12
#define delayRx_2D4GHZ  0

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

// admin
void                radio_2d4ghz_setFunctions(radio_functions_t * funcs);
void                radio_2d4ghz_powerOn(void);
// RF admin
void                radio_2d4ghz_init(void);
void                radio_2d4ghz_setStartFrameCb(radio_capture_cbt cb);
void                radio_2d4ghz_setEndFrameCb(radio_capture_cbt cb);
// RF admin
void                radio_2d4ghz_rfOn(void);
void                radio_2d4ghz_rfOff(void);
void                radio_2d4ghz_setFrequency(uint16_t channel_spacing, uint32_t frequency_0, uint16_t channel);
void                radio_2d4ghz_change_modulation(registerSetting_t * mod);
void                radio_2d4ghz_change_size(uint16_t* size);
// reset
void                radio_2d4ghz_reset(void);
// TX
void                radio_2d4ghz_loadPacket_prepare(uint8_t* packet, uint8_t len);
void                radio_2d4ghz_txEnable(void);
void                radio_2d4ghz_txNow(void);
void                radio_2d4ghz_loadPacket(uint8_t* packet, uint16_t len);
// RX
void                radio_2d4ghz_rxPacket_prepare(void);
void                radio_2d4ghz_rxEnable(void);
void                radio_2d4ghz_rxEnable_scum(void);
void                radio_2d4ghz_rxNow(void);
void                radio_2d4ghz_getReceivedFrame(uint8_t* bufRead,
                                uint16_t* lenRead,
                                uint16_t  maxBufLen,
                                 int8_t*  rssi,
                                uint8_t*  lqi,
                                   bool*  crc,
                                uint8_t*  mcs);
uint8_t             radio_2d4ghz_getCRCLen(void);
uint8_t             radio_2d4ghz_calculateFrequency(uint8_t channelOffset, uint8_t asnOffset, uint8_t numChannels, uint8_t* hopSeq, bool singleChannel);
uint8_t             radio_2d4ghz_getDelayTx(void);
uint8_t             radio_2d4ghz_getDelayRx(void);
// interrupt handlers
kick_scheduler_t    radio_2d4ghz_isr(void);

/**
\}
\}
*/

#endif
