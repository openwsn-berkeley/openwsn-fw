/**
\brief Z1-specific definition of the "debugpins" bsp module.

\author Xavier Vilajosana <xvilajosana@eecs.berkeley.edu>, May 2013.
*/

#include "msp430x26x.h"
#include "debugpins.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void debugpins_init(void) {  //JP1C all in a line
   P1DIR |=  0x40;      // frame [P1.0]
   P6DIR |=  0x80;      // slot  [P1.6]
   P2DIR |=  0x08;      // fsm   [P1.7]
   P2DIR |=  0x40;      // task  [P2.1]
   P6DIR |=  0x01;      // isr   [P2.3]
   P6DIR |=  0x02;      // radio [P4.3] 
}

// P1.0
void debugpins_frame_toggle(void) {
   P1OUT ^=  0x01;
}
void debugpins_frame_clr(void) {
   P1OUT &= ~0x01;
}
void debugpins_frame_set(void) {
   P1OUT |=  0x01;
}

// P1.6
void debugpins_slot_toggle(void) {
   P1OUT ^=  0x40;
}
void debugpins_slot_clr(void) {
   P1OUT &= ~0x40;
}
void debugpins_slot_set(void) {
   P1OUT |=  0x40;
}

// P1.7
void debugpins_fsm_toggle(void) {
   P1OUT ^=  0x80;
}
void debugpins_fsm_clr(void) {
   P1OUT &= ~0x80;
}
void debugpins_fsm_set(void) {
   P1OUT |=  0x80;
}

// P2.1
void debugpins_task_toggle(void) {
   P2OUT ^=  0x02;
}
void debugpins_task_clr(void) {
   P2OUT &= ~0x02;
}
void debugpins_task_set(void) {
   P2OUT |=  0x02;
}

// P2.3
void debugpins_isr_toggle(void) {
   P2OUT ^=  0x08;
}
void debugpins_isr_clr(void) {
   P2OUT &= ~0x08;
}
void debugpins_isr_set(void) {
   P2OUT |=  0x08;
}

// P4.3
void debugpins_radio_toggle(void) {
   P4OUT ^=  0x08;
}
void debugpins_radio_clr(void) {
   P4OUT &= ~0x08;
}
void debugpins_radio_set(void) {
   P4OUT |=  0x08;
}

//=========================== private =========================================