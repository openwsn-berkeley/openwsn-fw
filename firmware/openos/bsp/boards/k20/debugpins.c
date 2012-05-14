/**
\brief K20-specific definition of the "debugpins" bsp module.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, April 2012.
 */

#include "debugpins.h"
#include "board.h"

/*
 * Check pag 220 of the manual for available GPIOs
 */

//=========================== defines =========================================

#define FRAME_PIN 12
#define SLOT_PIN 13
#define FSM_PIN 14
#define TASK_PIN 15
#define ISR_PIN 5  //PTC5
#define RADIO_PIN 6 //PTC6

//PTC12,13,14,15
//GPIO14 = A50
//GPIO15= A51
//GPIO16= A52
//GPIO17 = A53
//TWRPI_GPIO4 PTD5
//TWRPI_GPIO1 PTC6

//=========================== variables =======================================

//=========================== prototypes ======================================


//=========================== public ==========================================

void debugpins_init() {
	// enable PORTC clock
	 SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK;
	 
	 // set pin as gpio, default to be input
	 PORTC_PCR12 = PORT_PCR_MUX(1);
	 PORTC_PCR13 = PORT_PCR_MUX(1);
	 PORTC_PCR14 = PORT_PCR_MUX(1);
	 PORTC_PCR15 = PORT_PCR_MUX(1);
	 PORTD_PCR5  = PORT_PCR_MUX(1);
	 PORTC_PCR6  = PORT_PCR_MUX(1);
		 
	 //set as output
	 GPIOC_PDDR |= 1<<FRAME_PIN;
	 GPIOC_PDDR |= 1<<SLOT_PIN;
	 GPIOC_PDDR |= 1<<FSM_PIN;
	 GPIOC_PDDR |= 1<<TASK_PIN;
	 GPIOD_PDDR |= 1<<ISR_PIN;
	 GPIOC_PDDR |= 1<<RADIO_PIN;
}

void debugpins_frame_toggle() {
	//toggle
	GPIOC_PTOR |= 1<<FRAME_PIN;
}
void debugpins_frame_clr() {
     //clear
	GPIOC_PCOR |= 1<<FRAME_PIN;
}
void debugpins_frame_set() {
	 //set
	GPIOC_PSOR |= 1<<FRAME_PIN;
}

void debugpins_slot_toggle() {
	GPIOC_PTOR |= 1<<SLOT_PIN;

}
void debugpins_slot_clr() {
	GPIOC_PCOR |= 1<<SLOT_PIN;
}
void debugpins_slot_set() {
	GPIOC_PSOR |= 1<<SLOT_PIN;
}

void debugpins_fsm_toggle() {
	GPIOC_PTOR |= 1<<FSM_PIN;
}
void debugpins_fsm_clr() {
	GPIOC_PCOR |= 1<<FSM_PIN;
}
void debugpins_fsm_set() {
	GPIOC_PSOR |= 1<<FSM_PIN;
}

void debugpins_task_toggle() {
	GPIOC_PTOR |= 1<<TASK_PIN;
}
void debugpins_task_clr() {
	GPIOC_PCOR |= 1<<TASK_PIN;
}
void debugpins_task_set() {
	GPIOC_PSOR |= 1<<TASK_PIN;
}

void debugpins_isr_toggle() {
	GPIOD_PTOR |= 1<<ISR_PIN;
	
}
void debugpins_isr_clr() {
	GPIOD_PCOR |= 1<<ISR_PIN;
}
void debugpins_isr_set() {
	GPIOD_PSOR |= 1<<ISR_PIN;
}

void debugpins_radio_toggle() {
	GPIOC_PTOR |= 1<<RADIO_PIN;
}
void debugpins_radio_clr() {
	GPIOC_PCOR |= 1<<RADIO_PIN;
}
void debugpins_radio_set() {
	GPIOC_PSOR |= 1<<RADIO_PIN;
}

