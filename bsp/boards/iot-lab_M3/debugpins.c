/**
\brief iot-lab_M3 definition of the "debugpins" bsp module.

\author Alaeddine Weslati <alaeddine.weslati@inria.fr>,  January 2014.
*/
#include "stm32f10x_conf.h"
#include "debugpins.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void debugpins_init(void) {}

void debugpins_frame_toggle(void) {}
void debugpins_frame_clr(void) {}
void debugpins_frame_set(void) {}

void debugpins_slot_toggle(void) {}
void debugpins_slot_clr(void) {}
void debugpins_slot_set(void) {}

void debugpins_fsm_toggle(void) {}
void debugpins_fsm_clr(void) {}
void debugpins_fsm_set(void) {}

void debugpins_task_toggle(void) {}
void debugpins_task_clr(void) {}
void debugpins_task_set(void) {}

void debugpins_isr_toggle(void) {}
void debugpins_isr_clr(void) {}
void debugpins_isr_set(void) {}

void debugpins_radio_toggle(void) {}
void debugpins_radio_clr(void) {}
void debugpins_radio_set(void) {}

