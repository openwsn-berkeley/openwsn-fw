/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Date:   Jan 2016
 * Description: EZR32WG-specific definition of the "debugpins" bsp module.
 */

#include "debugpins.h"
#include "board.h"

//=========================== defines =========================================
// Board dbPINS defines


//=========================== variables =======================================

//=========================== prototypes ======================================


//=========================== public ==========================================

void debugpins_init() {

}

// PA4
void debugpins_frame_toggle() {

}
void debugpins_frame_clr() {

}
void debugpins_frame_set() {

}

// PD3
void debugpins_slot_toggle() {
}
void debugpins_slot_clr() {
}
void debugpins_slot_set() {
}

// PD2
void debugpins_fsm_toggle() {
}
void debugpins_fsm_clr() {
}
void debugpins_fsm_set() {
}

// PD1
void debugpins_task_toggle() {
}
void debugpins_task_clr() {
}
void debugpins_task_set() {
}

// PA5
void debugpins_isr_toggle() {
}
void debugpins_isr_clr() {
}
void debugpins_isr_set() {
}

// PD0
void debugpins_radio_toggle() {
}
void debugpins_radio_clr() {
}
void debugpins_radio_set() {
}

//------------ private ------------//

