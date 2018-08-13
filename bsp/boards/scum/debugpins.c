/**
\brief SCuM-specific definition of the "debugpins" bsp module.

To be implemented after issue: SCUM-25

\author Tengfei Chang <tengfei.chang@inria.fr>, August 2016.
*/

#include "memory_map.h"
#include "debugpins.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void debugpins_init(void) {
    // GPIO pin 8 to 13 are used as debugpins, 
    GPIO_REG__OUTPUT    &= ~0x3F00; // all PINS low at initial
}

void debugpins_frame_toggle(void) {
    GPIO_REG__OUTPUT    ^=  0x0100;
}

void debugpins_frame_clr(void) {
    GPIO_REG__OUTPUT    &= ~0x0100;
}

void debugpins_frame_set(void) {
    GPIO_REG__OUTPUT    |=  0x0100;
}

void debugpins_slot_toggle(void) {
    GPIO_REG__OUTPUT    ^=  0x0200;
}

void debugpins_slot_clr(void) {
    GPIO_REG__OUTPUT    &= ~0x0200;
}

void debugpins_slot_set(void) {
    GPIO_REG__OUTPUT    |=  0x0200;
}

void debugpins_fsm_toggle(void) {
    GPIO_REG__OUTPUT    ^=  0x0400;
}

void debugpins_fsm_clr(void) {
    GPIO_REG__OUTPUT    &= ~0x0400;
}

void debugpins_fsm_set(void) {
    GPIO_REG__OUTPUT    |=  0x0400;
}

void debugpins_task_toggle(void) {
    GPIO_REG__OUTPUT    ^=  0x0800;
}

void debugpins_task_clr(void) {
    GPIO_REG__OUTPUT    &= ~0x0800;
}

void debugpins_task_set(void) {
    GPIO_REG__OUTPUT    |=  0x0800;
}

void debugpins_isr_toggle(void) {
    GPIO_REG__OUTPUT    ^=  0x1000;
}

void debugpins_isr_clr(void) {
    GPIO_REG__OUTPUT    &= ~0x1000;
}

void debugpins_isr_set(void) {
    GPIO_REG__OUTPUT    |=  0x1000;
}

void debugpins_radio_toggle(void) {
    GPIO_REG__OUTPUT    ^=  0x2000;
}

void debugpins_radio_clr(void) {
    GPIO_REG__OUTPUT    &= ~0x2000;
}

void debugpins_radio_set(void) {
    GPIO_REG__OUTPUT    |=  0x2000;
}