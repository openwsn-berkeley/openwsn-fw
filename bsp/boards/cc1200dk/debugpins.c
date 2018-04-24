/**
\brief cc1200dk-specific definition of the "debugpins" bsp module.

\author Jonathan Munoz <jonathan.munoz@inria.fr>, August 2016.
*/

#include "msp430f5438a.h"
#include "debugpins.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void debugpins_init(void) {
    P6DIR |=  0x20;      // frame [P6.5]
    P6DIR |=  0x80;      // slot  [P6.7]
    P6DIR |=  0x08;      // fsm   [P6.3]
    P6DIR |=  0x04;      // task  [P6.2]
    P6DIR |=  0x01;      // isr   [P6.0]
    P6DIR |=  0x02;      // radio [P6.1] 
}

// P6.5
void debugpins_frame_toggle(void) {
    P6OUT ^=  0x20;
}
void debugpins_frame_clr(void) {
    P6OUT &= ~0x20;
}
void debugpins_frame_set(void) {
    P6OUT |=  0x20;
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

// P6.3
void debugpins_fsm_toggle(void) {
    P6OUT ^=  0x08;
}
void debugpins_fsm_clr(void) {
    P6OUT &= ~0x08;
}
void debugpins_fsm_set(void) {
    P6OUT |=  0x08;
}

// P6.2
void debugpins_task_toggle(void) {
    P6OUT ^=  0x04;
}
void debugpins_task_clr(void) {
    P6OUT &= ~0x04;
}
void debugpins_task_set(void) {
    P6OUT |=  0x04;
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