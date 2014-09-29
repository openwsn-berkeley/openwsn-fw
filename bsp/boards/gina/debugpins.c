/**
\brief TelosB-specific definition of the "debugpins" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "msp430x26x.h"
#include "debugpins.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void debugpins_init() {
   P4DIR |=  0x20;      // frame
   P4DIR |=  0x02 ;     // slot
   P4DIR |=  0x04;      // fsm
   P4DIR |=  0x08;      // task
   P4DIR |=  0x10;      // isr
   P1DIR |=  0x02;      // radio
   
   debugpins_frame_clr();
   debugpins_slot_clr();
   debugpins_fsm_clr();
   debugpins_task_clr();
   debugpins_isr_clr();
   debugpins_radio_clr();
}

// P4.5
void debugpins_frame_toggle() {
   P4OUT ^=  0x20;
}
void debugpins_frame_clr() {
   P4OUT &= ~0x20;
}
void debugpins_frame_set() {
   P4OUT |=  0x20;
}

// P4.1
void debugpins_slot_toggle() {
   P4OUT ^=  0x02;
}
void debugpins_slot_clr() {
   P4OUT &= ~0x02;
}
void debugpins_slot_set() {
   P4OUT |=  0x02;
}

// P4.2
void debugpins_fsm_toggle() {
   P4OUT ^=  0x04;
}
void debugpins_fsm_clr() {
   P4OUT &= ~0x04;
}
void debugpins_fsm_set() {
   P4OUT |=  0x04;
}

// P4.3
void debugpins_task_toggle() {
   P4OUT ^=  0x08;
}
void debugpins_task_clr() {
   P4OUT &= ~0x08;
}
void debugpins_task_set() {
   P4OUT |=  0x08;
}

// P4.4
void debugpins_isr_toggle() {
   P4OUT ^=  0x10;
}
void debugpins_isr_clr() {
   P4OUT &= ~0x10;
}
void debugpins_isr_set() {
   P4OUT |=  0x10;
}

// P1.1
void debugpins_radio_toggle() {
   P1OUT ^=  0x02;
}
void debugpins_radio_clr() {
   P1OUT &= ~0x02;
}
void debugpins_radio_set() {
   P1OUT |=  0x02;
}