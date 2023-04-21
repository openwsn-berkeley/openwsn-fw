#ifndef __RADIO_BLE_H
#define __RADIO_BLE_H

/**

\brief declaration "radio_ble" bsp module.

\author Tengfei Chang <tengfei.chang@inria.fr>, August 2020.
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

// admin
void                radio_ble_init(void);
void                radio_ble_setFrequency(uint8_t channel);
void                radio_ble_loadPacket(uint8_t* packet, uint16_t len);
void                radio_ble_getReceivedFrame(uint8_t* pBufRead,
                            uint8_t* pLenRead,
                            uint8_t  maxBufLen,
                             int8_t* pRssi,
                            uint8_t* pLqi,
                               bool* pCrc);

/**
\}
\}
*/

#endif
