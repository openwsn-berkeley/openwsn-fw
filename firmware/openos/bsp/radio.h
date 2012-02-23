/**
\brief Cross-platform declaration "radio" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#ifndef __RADIO_H
#define __RADIO_H

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void radio_init();
void radio_reset();
void radio_setFrequency(uint8_t frequency);
void radio_loadPacket(uint8_t* packet, uint8_t len);
void radio_rfOn();
void radio_txEnable();
void radio_txNow();
void radio_rxEnable();
void radio_rxNow();
void radio_getReceivedFrame();
void radio_rfOff();

#endif