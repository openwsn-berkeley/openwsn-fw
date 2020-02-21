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
    GPIO_REG__OUTPUT    &= ~0x0FFF; // all PINS low at initial
}

// frame
void debugpins_frame_toggle(void) {
    GPIO_REG__OUTPUT    ^=  0x0002;
}

void debugpins_frame_clr(void) {
    GPIO_REG__OUTPUT    &= ~0x0002;
}

void debugpins_frame_set(void) {
    GPIO_REG__OUTPUT    |=  0x0002;
}

// slot
void debugpins_slot_toggle(void) {
    GPIO_REG__OUTPUT    ^=  0x0008;
}

void debugpins_slot_clr(void) {
    GPIO_REG__OUTPUT    &= ~0x0008;
}

void debugpins_slot_set(void) {
    GPIO_REG__OUTPUT    |=  0x0008;
}

// fsm
void debugpins_fsm_toggle(void) {
    GPIO_REG__OUTPUT    ^=  0x0010;
}

void debugpins_fsm_clr(void) {
    GPIO_REG__OUTPUT    &= ~0x0010;
}

void debugpins_fsm_set(void) {
    GPIO_REG__OUTPUT    |=  0x0010;
}

// task
void debugpins_task_toggle(void) {
    GPIO_REG__OUTPUT    ^=  0x0020;
}

void debugpins_task_clr(void) {
    GPIO_REG__OUTPUT    &= ~0x0020;
}

void debugpins_task_set(void) {
    GPIO_REG__OUTPUT    |=  0x0020;
}

// isr
void debugpins_isr_toggle(void) {
    GPIO_REG__OUTPUT    ^=  0x0004;
}

void debugpins_isr_clr(void) {
    GPIO_REG__OUTPUT    &= ~0x0004;
}

void debugpins_isr_set(void) {
    GPIO_REG__OUTPUT    |=  0x0004;
}

// radio
void debugpins_radio_toggle(void) {
    GPIO_REG__OUTPUT    ^=  0x0040;
}

void debugpins_radio_clr(void) {
    GPIO_REG__OUTPUT    &= ~0x0040;
}

void debugpins_radio_set(void) {
    GPIO_REG__OUTPUT    |=  0x0040;
}

// debug x
void debugpins_debug_x_toggle(void) {
    GPIO_REG__OUTPUT    ^=  0x0080;
}

void debugpins_debug_x_clr(void) {
    GPIO_REG__OUTPUT    &= ~0x0080;
}

void debugpins_debug_x_set(void) {
    GPIO_REG__OUTPUT    |=  0x0080;
}

// debug y
void debugpins_debug_y_toggle(void){
    GPIO_REG__OUTPUT    ^=  0x0100;
}

void debugpins_debug_y_clr(void){
    GPIO_REG__OUTPUT    &= ~0x0100;
}

void debugpins_debug_y_set(void){
    GPIO_REG__OUTPUT    |=  0x0100;
}

// debug z
void debugpins_debug_z_toggle(void){
    GPIO_REG__OUTPUT    ^=  0x0200;
}

void debugpins_debug_z_clr(void){
    GPIO_REG__OUTPUT    &= ~0x0200;
}

void debugpins_debug_z_set(void){
    GPIO_REG__OUTPUT    |=  0x0200;
}

// ISRs for external interrupts
void ext_gpio3_activehigh_debounced_isr(){
    printf("External Interrupt GPIO3 triggered\r\n");
}
void ext_gpio8_activehigh_isr(){
    printf("External Interrupt GPIO8 triggered\r\n");
}
void ext_gpio9_activelow_isr(){
    printf("External Interrupt GPIO9 triggered\r\n");
}
void ext_gpio10_activelow_isr(){
    printf("External Interrupt GPIO10 triggered\r\n");
}
