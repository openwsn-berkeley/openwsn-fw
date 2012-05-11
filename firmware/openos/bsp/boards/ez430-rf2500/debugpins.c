/**
\brief eZ430-RF2500-specific definition of the "debugpins" bsp module.

\author Chuang Qian <cqian@berkeley.edu>, April 2012.

*/

#include "io430.h"
#include "debugpins.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void debugpins_init() {
   P2DIR |=  0x01;      // frame
   P2DIR |=  0x02;      // slot
   P2DIR |=  0x04;      // fsm
   P2DIR |=  0x08;      // task
   P2DIR |=  0x10;      // isr
   P4DIR |=  0x08;      // radio
}

// P2.0
void debugpins_frame_toggle() {
   P2OUT ^=  0x01;
}
void debugpins_frame_clr() {
   P2OUT &= ~0x01;
}
void debugpins_frame_set() {
   P2OUT |=  0x01;
}

// P2.1
void debugpins_slot_toggle() {
   P2OUT ^=  0x02;
}
void debugpins_slot_clr() {
   P2OUT &= ~0x02;
}
void debugpins_slot_set() {
   P2OUT |=  0x02;
}

// P2.2
void debugpins_fsm_toggle() {
   P2OUT ^=  0x04;
}
void debugpins_fsm_clr() {
   P2OUT &= ~0x04;
}
void debugpins_fsm_set() {
   P2OUT |=  0x04;
}

// P2.3
void debugpins_task_toggle() {
   P2OUT ^=  0x08;
}
void debugpins_task_clr() {
   P2OUT &= ~0x08;
}
void debugpins_task_set() {
   P2OUT |=  0x08;
}

// P2.4
void debugpins_isr_toggle() {
   P2OUT ^=  0x10;
}
void debugpins_isr_clr() {
   P2OUT &= ~0x10;
}
void debugpins_isr_set() {
   P2OUT |=  0x10;
}

// P4.3
void debugpins_radio_toggle() {
   P4OUT ^=  0x08;
}
void debugpins_radio_clr() {
   P4OUT &= ~0x08;
}
void debugpins_radio_set() {
   P4OUT |=  0x08;
}