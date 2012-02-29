#ifndef __DEBUGPINS_H
#define __DEBUGPINS_H

#include "msp430f1611.h"

// debug pins
#define DEBUG_PIN_FRAME_INIT()    P6DIR |=  0x40 // P6.6
#define DEBUG_PIN_FRAME_TOGGLE()  P6OUT ^=  0x40
#define DEBUG_PIN_FRAME_CLR()     P6OUT &= ~0x40
#define DEBUG_PIN_FRAME_SET()     P6OUT |=  0x40

#define DEBUG_PIN_SLOT_INIT()     P2DIR |=  0x40 // P2.6
#define DEBUG_PIN_SLOT_TOGGLE()   P2OUT ^=  0x40
#define DEBUG_PIN_SLOT_CLR()      P2OUT &= ~0x40
#define DEBUG_PIN_SLOT_SET()      P2OUT |=  0x40

#define DEBUG_PIN_FSM_INIT()      P6DIR |=  0x01 // P6.0
#define DEBUG_PIN_FSM_TOGGLE()    P6OUT ^=  0x01
#define DEBUG_PIN_FSM_CLR()       P6OUT &= ~0x01
#define DEBUG_PIN_FSM_SET()       P6OUT |=  0x01

#define DEBUG_PIN_TASK_INIT()     P3DIR |=  0x08 // P3.3
#define DEBUG_PIN_TASK_TOGGLE()   P3OUT ^=  0x08
#define DEBUG_PIN_TASK_CLR()      P3OUT &= ~0x08
#define DEBUG_PIN_TASK_SET()      P3OUT |=  0x08

#define DEBUG_PIN_ISR_INIT()      P6DIR |=  0x02 // P6.1
#define DEBUG_PIN_ISR_TOGGLE()    P6OUT ^=  0x02
#define DEBUG_PIN_ISR_CLR()       P6OUT &= ~0x02
#define DEBUG_PIN_ISR_SET()       P6OUT |=  0x02

#define DEBUG_PIN_RADIO_INIT()    P3DIR |=  0x02 // P3.1
#define DEBUG_PIN_RADIO_TOGGLE()  P3OUT ^=  0x02
#define DEBUG_PIN_RADIO_CLR()     P3OUT &= ~0x02
#define DEBUG_PIN_RADIO_SET()     P3OUT |=  0x02

#endif
