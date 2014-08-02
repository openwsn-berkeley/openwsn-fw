/**
\brief WSN430v14-specific definition of the "debugpins" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "msp430f1611.h"
#include "debugpins.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void debugpins_init() {
   P2DIR |=  0x02;      // frame [P2.1]
   P6DIR |=  0x02;      // slot  [P6.1]
   P2DIR |=  0x04;      // fsm   [P2.2]
   P2DIR |=  0x08;      // task  [P2.3]
   P2DIR |=  0x01;      // isr   [P2.0]
   P6DIR |=  0x01;      // radio [P6.0]
}

// frame [P2.1]
void debugpins_frame_toggle() {
   P2OUT ^=  0x02;
}
void debugpins_frame_clr() {
   P2OUT &= ~0x02;
}
void debugpins_frame_set() {
   P2OUT |=  0x02;
}

// slot [P6.1]
void debugpins_slot_toggle() {
   P6OUT ^=  0x02;
}
void debugpins_slot_clr() {
   P6OUT &= ~0x02;
}
void debugpins_slot_set() {
   P6OUT |=  0x02;
}

// fsm [P2.2]
void debugpins_fsm_toggle() {
   P2OUT ^=  0x04;
}
void debugpins_fsm_clr() {
   P2OUT &= ~0x04;
}
void debugpins_fsm_set() {
   P2OUT |=  0x04;
}

// task [P2.3]
void debugpins_task_toggle() {
   P2OUT ^=  0x08;
}
void debugpins_task_clr() {
   P2OUT &= ~0x08;
}
void debugpins_task_set() {
   P2OUT |=  0x08;
}

// isr [P2.0]
void debugpins_isr_toggle() {
   P2OUT ^=  0x01;
}
void debugpins_isr_clr() {
   P2OUT &= ~0x01;
}
void debugpins_isr_set() {
   P2OUT |=  0x01;
}

// radio [P6.0]
void debugpins_radio_toggle() {
   P6OUT ^=  0x01;
}
void debugpins_radio_clr() {
   P6OUT &= ~0x01;
}
void debugpins_radio_set() {
   P6OUT |=  0x01;
}

//=========================== private =========================================