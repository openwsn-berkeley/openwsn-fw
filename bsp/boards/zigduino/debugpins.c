/**
\brief Zigduino-specific definition of the "debugpins" bsp module.

\author Sven Akkermans (sven.akkermans@cs.kuleuven.be), September 2015.
 */
#include "debugpins.h"
#include <avr/io.h>

//=========================== defines =========================================
#define DEBUG_PORT	PORTE
#define DEBUG_PORT_DIR DDRE

#define FRAME_PIN	PE6
#define SLOT_PIN	PE5
#define FSM_PIN		PE2
#define TASK_PIN	PE3
#define ISR_PIN		PE4
#define RADIO_PIN	PE7
//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void debugpins_init() {
	// all pins as output
	DEBUG_PORT_DIR |= (1 << FRAME_PIN);
	DEBUG_PORT_DIR |= (1 << SLOT_PIN);
	DEBUG_PORT_DIR |= (1 << FSM_PIN);
	DEBUG_PORT_DIR |= (1 << TASK_PIN);
	DEBUG_PORT_DIR |= (1 << ISR_PIN);
	DEBUG_PORT_DIR |= (1 << RADIO_PIN);
}

void debugpins_frame_toggle() {
	DEBUG_PORT ^= (1 << FRAME_PIN);
	DEBUG_PORT_DIR |= (1 << FRAME_PIN);
}

void debugpins_frame_clr() {
	DEBUG_PORT &= ~(1 << FRAME_PIN);
	DEBUG_PORT_DIR |= (1 << FRAME_PIN);
}
void debugpins_frame_set() {
	DEBUG_PORT |= (1 << FRAME_PIN);
	DEBUG_PORT_DIR |= (1 << FRAME_PIN);
}

void debugpins_slot_toggle() {
	DEBUG_PORT ^= (1 << SLOT_PIN);
	DEBUG_PORT_DIR |= (1 << SLOT_PIN);
}
void debugpins_slot_clr() {
	DEBUG_PORT &= ~(1 << SLOT_PIN);
	DEBUG_PORT_DIR |= (1 << SLOT_PIN);
}
void debugpins_slot_set() {
	DEBUG_PORT |= (1 << SLOT_PIN);
	DEBUG_PORT_DIR |= (1 << SLOT_PIN);
}

void debugpins_fsm_toggle() {
	DEBUG_PORT ^= (1 << FSM_PIN);
	DEBUG_PORT_DIR |= (1 << FSM_PIN);
}
void debugpins_fsm_clr() {
	DEBUG_PORT &= ~(1 << FSM_PIN);
	DEBUG_PORT_DIR |= (1 << FSM_PIN);
}
void debugpins_fsm_set() {
	DEBUG_PORT |= (1 << FSM_PIN);
	DEBUG_PORT_DIR |= (1 << FSM_PIN);
}

void debugpins_task_toggle() {
	DEBUG_PORT ^= (1 << TASK_PIN);
	DEBUG_PORT_DIR |= (1 << TASK_PIN);
}
void debugpins_task_clr() {
	DEBUG_PORT &= ~(1 << TASK_PIN);
	DEBUG_PORT_DIR |= (1 << TASK_PIN);
}
void debugpins_task_set() {
	DEBUG_PORT |= (1 << TASK_PIN);
	DEBUG_PORT_DIR |= (1 << TASK_PIN);
}

void debugpins_isr_toggle() {
	DEBUG_PORT ^= (1 << ISR_PIN);
	DEBUG_PORT_DIR |= (1 << ISR_PIN);
}
void debugpins_isr_clr() {
	DEBUG_PORT &= ~(1 << ISR_PIN);
	DEBUG_PORT_DIR |= (1 << ISR_PIN);
}
void debugpins_isr_set() {
	DEBUG_PORT |= (1 << ISR_PIN);
	DEBUG_PORT_DIR |= (1 << ISR_PIN);
}

void debugpins_radio_toggle() {
	DEBUG_PORT ^= (1 << RADIO_PIN);
	DEBUG_PORT_DIR |= (1 << RADIO_PIN);
}
void debugpins_radio_clr() {
	DEBUG_PORT &= ~(1 << RADIO_PIN);
	DEBUG_PORT_DIR |= (1 << RADIO_PIN);
}
void debugpins_radio_set() {
	DEBUG_PORT |= (1 << RADIO_PIN);
	DEBUG_PORT_DIR |= (1 << RADIO_PIN);
}
