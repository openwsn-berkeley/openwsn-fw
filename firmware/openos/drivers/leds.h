#ifndef __LEDS_H
#define __LEDS_H

/**
\addtogroup drivers
\{
\addtogroup Leds
\{
*/

#include "msp430x26x.h"
#include "stdint.h"
 
//=========================== define ==========================================

#define LED_RADIO_ON()       P2OUT |=  0x01; // yellow
#define LED_RADIO_OFF()      P2OUT &= ~0x01;
#define LED_RADIO_TOGGLE()   P2OUT ^=  0x01;

#define LED_SYNC_ON()        P2OUT |=  0x02; // green
#define LED_SYNC_OFF()       P2OUT &= ~0x02;
#define LED_SYNC_TOGGLE()    P2OUT ^=  0x02;

#define LED_D3_ON()          P2OUT |=  0x04; // orange
#define LED_D3_OFF()         P2OUT &= ~0x04;
#define LED_D3_TOGGLE()      P2OUT ^=  0x04;

#define LED_ERROR_ON()       P2OUT |=  0x08; // red
#define LED_ERROR_OFF()      P2OUT &= ~0x08;
#define LED_ERROR_TOGGLE()   P2OUT ^=  0x08;

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void leds_init();
void leds_circular_shift();
void leds_increment();

/**
\}
\}
*/

#endif
