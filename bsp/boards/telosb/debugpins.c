/**
\brief TelosB-specific definition of the "debugpins" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/
#include <stdint.h>               // needed for uin8_t, uint16_t
#include "msp430f1611.h"
#include "debugpins.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void debugpins_init(void) {
   P6DIR |=  0x40;      // frame     [P6.6]
   P6DIR |=  0x80;      // slot      [P6.7]
   P2DIR |=  0x08;      // fsm       [P2.3]
   P2DIR |=  0x40;      // task      [P2.6]
   P6DIR |=  0x01;      // isr       [P6.0]
   P3DIR |=  0x20;      // isruarttx [P3.5]
   P3DIR |=  0x10;      // isruartrx [P3.4]
   P6DIR |=  0x02;      // radio     [P6.1]
}

// P6.6
void debugpins_frame_toggle(void) {
   P6OUT ^=  0x40;
}
void debugpins_frame_clr(void) {
   P6OUT &= ~0x40;
}
void debugpins_frame_set(void) {
   P6OUT |=  0x40;
}

// P6.7
void debugpins_slot_toggle(void) {
   P6OUT ^=  0x80;
}
void debugpins_slot_clr(void) {
   P6OUT &= ~0x80;
}
void debugpins_slot_set(void) {
   P6OUT |=  0x80;
}

// P2.3
void debugpins_fsm_toggle(void) {
   P2OUT ^=  0x08;
}
void debugpins_fsm_clr(void) {
   P2OUT &= ~0x08;
}
void debugpins_fsm_set(void) {
   P2OUT |=  0x08;
}

// P2.6
void debugpins_task_toggle(void) {
   P2OUT ^=  0x40;
}
void debugpins_task_clr(void) {
   P2OUT &= ~0x40;
}
void debugpins_task_set(void) {
   P2OUT |=  0x40;
}

// P6.0
void debugpins_isr_toggle(void) {
   P6OUT ^=  0x01;
}
void debugpins_isr_clr(void) {
   P6OUT &= ~0x01;
}
void debugpins_isr_set(void) {
   P6OUT |=  0x01;
}

// P3.5
void debugpins_isruarttx_toggle(void) {
   P3OUT ^=  0x20;
}
void debugpins_isruarttx_clr(void) {
   P3OUT &= ~0x20;
}
void debugpins_isruarttx_set(void) {
   P3OUT |=  0x20;
}

// P3.4
void debugpins_isruartrx_toggle(void) {
   P3OUT ^=  0x10;
}
void debugpins_isruartrx_clr(void) {
   P3OUT &= ~0x10;
}
void debugpins_isruartrx_set(void) {
   P3OUT |=  0x10;
}

// P6.1
void debugpins_radio_toggle(void) {
   P6OUT ^=  0x02;
}
void debugpins_radio_clr(void) {
   P6OUT &= ~0x02;
}
void debugpins_radio_set(void) {
   P6OUT |=  0x02;
}

//=========================== private =========================================
