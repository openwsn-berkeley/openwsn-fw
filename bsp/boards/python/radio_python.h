#ifndef __RADIO_PYTHON_H
#define __RADIO_PYTHON_H

/**
\addtogroup BSP
\{
\addtogroup radio_python
\{

\brief Cross-platform declaration "radio" bsp module.

\author Tengfei Chang <tengfei.chang@inria.fr>, March 2018.
*/

//=========================== define ==========================================

/**
\brief Current state of the radio python.

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
void                radio_python_setFunctions(radio_functions_t * funcs);
void                radio_python_powerOn(void);
// RF admin
void                radio_python_init(void);
void                radio_python_setStartFrameCb(radio_capture_cbt cb);
void                radio_python_setEndFrameCb(radio_capture_cbt cb);
// RF admin
void                radio_python_rfOn(void);
void                radio_python_rfOff(void);
void                radio_python_setFrequency(uint16_t channel_spacing, uint32_t frequency_0, uint16_t channel);
void                radio_python_change_modulation(registerSetting_t * mod);
void                radio_python_change_size(uint16_t* size);
// reset
void                radio_python_reset(void);
// TX
void                radio_python_loadPacket_prepare(uint8_t* packet, uint8_t len);
void                radio_python_txEnable(void);
void                radio_python_txNow(void);
void                radio_python_loadPacket(uint8_t* packet, uint16_t len);
// RX
void                radio_python_rxPacket_prepare(void);
void                radio_python_rxEnable(void);
void                radio_python_rxEnable_scum(void);
void                radio_python_rxNow(void);
void                radio_python_getReceivedFrame(uint8_t* bufRead,
                                uint16_t* lenRead,
                                uint16_t  maxBufLen,
                                 int8_t*  rssi,
                                uint8_t*  lqi,
                                   bool*  crc,
                                uint8_t*  mcs);
uint8_t             radio_python_getCRCLen(void);
uint8_t             radio_python_calculateFrequency(uint8_t channelOffset, uint8_t asnOffset, uint8_t numChannels, uint8_t* hopSeq, bool singleChannel);
uint8_t             radio_python_getDelayTx(void);
uint8_t             radio_python_getDelayRx(void);

/**
\}
\}
*/

#endif
