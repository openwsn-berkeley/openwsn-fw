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

void debugpins_init(void);

void debugpins_frame_toggle(void);
void debugpins_frame_clr(void);
void debugpins_frame_set(void);

void debugpins_slot_toggle(void);
void debugpins_slot_clr(void);
void debugpins_slot_set(void);

void debugpins_fsm_toggle(void);
void debugpins_fsm_clr(void);
void debugpins_fsm_set(void);

void debugpins_task_toggle(void);
void debugpins_task_clr(void);
void debugpins_task_set(void);

void debugpins_isr_toggle(void);
void debugpins_isr_clr(void);
void debugpins_isr_set(void);

void debugpins_isruarttx_toggle(void);
void debugpins_isruarttx_clr(void);
void debugpins_isruarttx_set(void);

void debugpins_isruartrx_toggle(void);
void debugpins_isruartrx_clr(void);
void debugpins_isruartrx_set(void);

void debugpins_radio_toggle(void);
void debugpins_radio_clr(void);
void debugpins_radio_set(void);

void debugpins_intdisabled_toggle(void);
void debugpins_intdisabled_clr(void);
void debugpins_intdisabled_set(void);

void debugpins_debug_x_toggle(void);
void debugpins_debug_x_clr(void);
void debugpins_debug_x_set(void);

void debugpins_debug_y_toggle(void);
void debugpins_debug_y_clr(void);
void debugpins_debug_y_set(void);

void debugpins_debug_z_toggle(void);
void debugpins_debug_z_clr(void);
void debugpins_debug_z_set(void);

#ifdef OPENSIM
void debugpins_ka_clr(void);
void debugpins_ka_set(void);

void debugpins_syncPacket_clr(void);
void debugpins_syncPacket_set(void);

void debugpins_syncAck_clr(void);
void debugpins_syncAck_set(void);

void debugpins_debug_clr(void);
void debugpins_debug_set(void);
#endif

/**
\}
\}
*/

#endif
