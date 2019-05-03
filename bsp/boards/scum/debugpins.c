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
    // GPIO pin 0 to 7 are used as debugpins, 
    GPIO_REG__OUTPUT    &= ~0x00FF; // all PINS low at initial
}

void debugpins_frame_toggle(void) {
    GPIO_REG__OUTPUT    ^=  0x0001;
}

void debugpins_frame_clr(void) {
    GPIO_REG__OUTPUT    &= ~0x0001;
}

void debugpins_frame_set(void) {
    GPIO_REG__OUTPUT    |=  0x0001;
}

void debugpins_slot_toggle(void) {
    GPIO_REG__OUTPUT    ^=  0x0002;
}

void debugpins_slot_clr(void) {
    GPIO_REG__OUTPUT    &= ~0x0002;
}

void debugpins_slot_set(void) {
    GPIO_REG__OUTPUT    |=  0x0002;
}

void debugpins_fsm_toggle(void) {
    GPIO_REG__OUTPUT    ^=  0x0004;
}

void debugpins_fsm_clr(void) {
    GPIO_REG__OUTPUT    &= ~0x0004;
}

void debugpins_fsm_set(void) {
    GPIO_REG__OUTPUT    |=  0x0004;
}

void debugpins_task_toggle(void) {
    GPIO_REG__OUTPUT    ^=  0x0008;
}

void debugpins_task_clr(void) {
    GPIO_REG__OUTPUT    &= ~0x0008;
}

void debugpins_task_set(void) {
    GPIO_REG__OUTPUT    |=  0x0008;
}

void debugpins_isr_toggle(void) {
    GPIO_REG__OUTPUT    ^=  0x0010;
}

void debugpins_isr_clr(void) {
    GPIO_REG__OUTPUT    &= ~0x0010;
}

void debugpins_isr_set(void) {
    GPIO_REG__OUTPUT    |=  0x0010;
}

void debugpins_radio_toggle(void) {
    GPIO_REG__OUTPUT    ^=  0x0020;
}

void debugpins_radio_clr(void) {
    GPIO_REG__OUTPUT    &= ~0x0020;
}

void debugpins_radio_set(void) {
    GPIO_REG__OUTPUT    |=  0x0020;
}

void debugpins_debug_x_toggle(void) {
    GPIO_REG__OUTPUT    ^=  0x0040;
}

void debugpins_debug_x_clr(void) {
    GPIO_REG__OUTPUT    &= ~0x0040;
}

void debugpins_debug_x_set(void) {
    GPIO_REG__OUTPUT    |=  0x0040;
}

void debugpins_debug_y_toggle(void){
    GPIO_REG__OUTPUT    ^=  0x0080;
}

void debugpins_debug_y_clr(void){
    GPIO_REG__OUTPUT    &= ~0x0080;
}

void debugpins_debug_y_set(void){
    GPIO_REG__OUTPUT    |=  0x0080;
}
