#ifndef __DEBUGPINS_H
#define __DEBUGPINS_H

#include "msp430x26x.h"

// debug pins
#define DEBUG_PIN_FRAME_INIT()    P4DIR |=  0x20 // P4.5
#define DEBUG_PIN_FRAME_TOGGLE()  P4OUT ^=  0x20
#define DEBUG_PIN_FRAME_CLR()     P4OUT &= ~0x20
#define DEBUG_PIN_FRAME_SET()     P4OUT |=  0x20

#define DEBUG_PIN_SLOT_INIT()     P4DIR |=  0x02 // P4.1
#define DEBUG_PIN_SLOT_TOGGLE()   P4OUT ^=  0x02
#define DEBUG_PIN_SLOT_CLR()      P4OUT &= ~0x02
#define DEBUG_PIN_SLOT_SET()      P4OUT |=  0x02

#define DEBUG_PIN_FSM_INIT()      P4DIR |=  0x04 // P4.2
#define DEBUG_PIN_FSM_TOGGLE()    P4OUT ^=  0x04
#define DEBUG_PIN_FSM_CLR()       P4OUT &= ~0x04
#define DEBUG_PIN_FSM_SET()       P4OUT |=  0x04

#define DEBUG_PIN_TASK_INIT()     P4DIR |=  0x08 // P4.3
#define DEBUG_PIN_TASK_TOGGLE()   P4OUT ^=  0x08
#define DEBUG_PIN_TASK_CLR()      P4OUT &= ~0x08
#define DEBUG_PIN_TASK_SET()      P4OUT |=  0x08

#define DEBUG_PIN_ISR_INIT()      P4DIR |=  0x10 // P4.4
#define DEBUG_PIN_ISR_TOGGLE()    P4OUT ^=  0x10
#define DEBUG_PIN_ISR_CLR()       P4OUT &= ~0x10
#define DEBUG_PIN_ISR_SET()       P4OUT |=  0x10

#define DEBUG_PIN_RADIO_INIT()    P1DIR |=  0x02 // P1.1
#define DEBUG_PIN_RADIO_TOGGLE()  P1OUT ^=  0x02
#define DEBUG_PIN_RADIO_CLR()     P1OUT &= ~0x02
#define DEBUG_PIN_RADIO_SET()     P1OUT |=  0x02

#endif
