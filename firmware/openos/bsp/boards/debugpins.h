#ifndef __DEBUGPINS_H
#define __DEBUGPINS_H

/**
\addtogroup BSP
\{
\addtogroup debugpins
\{

\brief Cross-platform declaration "leds" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void debugpins_init();

void debugpins_frame_toggle();
void debugpins_frame_clr();
void debugpins_frame_set();

void debugpins_slot_toggle();
void debugpins_slot_clr();
void debugpins_slot_set();

void debugpins_fsm_toggle();
void debugpins_fsm_clr();
void debugpins_fsm_set();

void debugpins_task_toggle();
void debugpins_task_clr();
void debugpins_task_set();

void debugpins_isr_toggle();
void debugpins_isr_clr();
void debugpins_isr_set();

void debugpins_radio_toggle();
void debugpins_radio_clr();
void debugpins_radio_set();

#ifdef OPENSIM
void debugpins_ka_clr();
void debugpins_ka_set();

void debugpins_syncPacket_clr();
void debugpins_syncPacket_set();

void debugpins_syncAck_clr();
void debugpins_syncAck_set();

void debugpins_debug_clr();
void debugpins_debug_set();
#endif

/**
\}
\}
*/

#endif
