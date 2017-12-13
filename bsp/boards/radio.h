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

//=========================== typedef =========================================


//=========================== variables =======================================

//=========================== prototypes ======================================

// admin
void                radio_setFunctions(radio_functions_t * funcs);
void                radio_powerOn(void);
// RF admin
void                radio_init(void);
void                radio_setStartFrameCb(radio_capture_cbt cb);
void                radio_setEndFrameCb(radio_capture_cbt cb);
// RF admin
void                radio_rfOn(void);
void                radio_rfOff(void);
void                radio_setFrequency(uint16_t channel_spacing, uint32_t frequency_0, uint16_t channel);
void                radio_change_modulation(registerSetting_t * mod);
void                radio_change_size(uint16_t* size);
// reset
void                radio_reset(void);
// TX
void                radio_loadPacket_prepare(uint8_t* packet, uint8_t len);
void                radio_txEnable(void);
void                radio_txNow(void);
void                radio_loadPacket(uint8_t* packet, uint16_t len);
// RX
void                radio_rxPacket_prepare(void);
void                radio_rxEnable(void);
void                radio_rxEnable_scum(void);
void                radio_rxNow(void);
void                radio_getReceivedFrame(uint8_t* bufRead,
                                uint16_t* lenRead,
                                uint16_t  maxBufLen,
                                 int8_t*  rssi,
                                uint8_t*  lqi,
                                   bool*  crc,
                                uint8_t*  mcs);
uint8_t             radio_getCRCLen(void);
uint8_t             radio_calculateFrequency(uint8_t channelOffset, uint8_t asnOffset, uint8_t numChannels, uint8_t* hopSeq, bool singleChannel);



// interrupt handlers
kick_scheduler_t    radio_isr(void);

/**
\}
\}
*/

#endif
