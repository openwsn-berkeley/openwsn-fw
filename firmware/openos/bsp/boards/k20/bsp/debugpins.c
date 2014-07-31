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

#ifdef TOWER_K20

#define FRAME_PIN 4 //PTD4 B40 
#define SLOT_PIN 20 //PTB20 B23
#define FSM_PIN 26  //PTE26 B25
#define TASK_PIN 0 //PTB0 B27
#define ISR_PIN 2  //PTB2 B29
#define RADIO_PIN 14 //PTA14 B35

#elif OPENMOTE_K20
//TODO
#define FRAME_PIN 0 //PTE0  
#define SLOT_PIN 1 //PTE1 
#define FSM_PIN 2  //PTE2 
#define TASK_PIN 3 //PTE3 
#define ISR_PIN 11  //PTC11 
#define RADIO_PIN 16 //PTC16 

#endif


//=========================== variables =======================================

//=========================== prototypes ======================================


//=========================== public ==========================================

void debugpins_init() {
#ifdef TOWER_K20
	// enable PORTC clock
	 SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK|SIM_SCGC5_PORTB_MASK|SIM_SCGC5_PORTD_MASK|SIM_SCGC5_PORTE_MASK;
	 
	 PORTD_PCR4 = PORT_PCR_MUX(1);
	 PORTB_PCR20 = PORT_PCR_MUX(1);
	 PORTE_PCR26 = PORT_PCR_MUX(1);
	 PORTB_PCR0 = PORT_PCR_MUX(1);
	 PORTB_PCR2  = PORT_PCR_MUX(1);
	 PORTA_PCR14  = PORT_PCR_MUX(1);
 	 
	 //set as output
	 GPIOD_PDDR |= 1<<FRAME_PIN;
	 GPIOB_PDDR |= 1<<SLOT_PIN;
	 GPIOE_PDDR |= 1<<FSM_PIN;
	 GPIOB_PDDR |= 1<<TASK_PIN;
	 GPIOB_PDDR |= 1<<ISR_PIN;
	 GPIOA_PDDR |= 1<<RADIO_PIN;
	 
#elif OPENMOTE_K20
	 SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK| SIM_SCGC5_PORTE_MASK | SIM_SCGC5_PORTB_MASK;
	 
	 PORTE_PCR0 = PORT_PCR_MUX(1);
	 PORTE_PCR1 = PORT_PCR_MUX(1);
	 PORTE_PCR2 = PORT_PCR_MUX(1);
	 PORTE_PCR3 = PORT_PCR_MUX(1);
	 PORTC_PCR11  = PORT_PCR_MUX(1);
	 PORTC_PCR16  = PORT_PCR_MUX(1);
 	 
	 //set as output
	 GPIOE_PDDR |= 1<<FRAME_PIN;
	 GPIOE_PDDR |= 1<<SLOT_PIN;
	 GPIOE_PDDR |= 1<<FSM_PIN;
	 GPIOE_PDDR |= 1<<TASK_PIN;
	 GPIOC_PDDR |= 1<<ISR_PIN;
	 GPIOC_PDDR |= 1<<RADIO_PIN;
	
#endif
}

void debugpins_frame_toggle() {
	//toggle
#ifdef TOWER_K20	
	GPIOD_PTOR |= 1<<FRAME_PIN;
#elif OPENMOTE_K20
	GPIOE_PTOR |= 1<<FRAME_PIN;
#endif
}
void debugpins_frame_clr() {
     //clear
#ifdef TOWER_K20	
	GPIOD_PCOR |= 1<<FRAME_PIN;
#elif OPENMOTE_K20
	GPIOE_PCOR |= 1<<FRAME_PIN;
#endif
}
void debugpins_frame_set() {
#ifdef TOWER_K20	
	GPIOD_PSOR |= 1<<FRAME_PIN;
#elif OPENMOTE_K20
	GPIOE_PSOR |= 1<<FRAME_PIN;
#endif
}

void debugpins_slot_toggle() {
#ifdef TOWER_K20	
	GPIOB_PTOR |= 1<<SLOT_PIN;
#elif OPENMOTE_K20
	GPIOE_PTOR |= 1<<SLOT_PIN;
#endif
}
void debugpins_slot_clr() {
#ifdef TOWER_K20	
	GPIOB_PCOR |= 1<<SLOT_PIN;
#elif OPENMOTE_K20
	GPIOE_PCOR |= 1<<SLOT_PIN;
#endif
}
void debugpins_slot_set() {
#ifdef TOWER_K20	
	GPIOB_PSOR |= 1<<SLOT_PIN;
#elif OPENMOTE_K20
	GPIOE_PSOR |= 1<<SLOT_PIN;
#endif
}

void debugpins_fsm_toggle() {
#ifdef TOWER_K20	
	GPIOE_PTOR |= 1<<FSM_PIN;
#elif OPENMOTE_K20
	GPIOE_PTOR |= 1<<FSM_PIN;
#endif
}
void debugpins_fsm_clr() {
#ifdef TOWER_K20	
	GPIOE_PCOR |= 1<<FSM_PIN;
#elif OPENMOTE_K20
	GPIOE_PCOR |= 1<<FSM_PIN;
#endif
}
void debugpins_fsm_set() {
#ifdef TOWER_K20	
	GPIOE_PSOR |= 1<<FSM_PIN;
#elif OPENMOTE_K20
	GPIOE_PSOR |= 1<<FSM_PIN;
#endif
}

void debugpins_task_toggle() {
#ifdef TOWER_K20	
	GPIOB_PTOR |= 1<<TASK_PIN;
#elif OPENMOTE_K20
	GPIOE_PTOR |= 1<<TASK_PIN;
#endif
}
void debugpins_task_clr() {
#ifdef TOWER_K20	
	GPIOB_PCOR |= 1<<TASK_PIN;
#elif OPENMOTE_K20
	GPIOE_PCOR |= 1<<TASK_PIN;
#endif
}
void debugpins_task_set() {
#ifdef TOWER_K20	
	GPIOB_PSOR |= 1<<TASK_PIN;
#elif OPENMOTE_K20
	GPIOE_PSOR |= 1<<TASK_PIN;
#endif
}

void debugpins_isr_toggle() {
#ifdef TOWER_K20	
	GPIOB_PTOR |= 1<<ISR_PIN;
#elif OPENMOTE_K20
	GPIOC_PTOR |= 1<<ISR_PIN;
#endif
	
}
void debugpins_isr_clr() {
#ifdef TOWER_K20	
	GPIOB_PCOR |= 1<<ISR_PIN;
#elif OPENMOTE_K20
	GPIOC_PCOR |= 1<<ISR_PIN;
#endif
}
void debugpins_isr_set() {
#ifdef TOWER_K20	
	GPIOB_PSOR |= 1<<ISR_PIN;
#elif OPENMOTE_K20
	GPIOC_PSOR |= 1<<ISR_PIN;
#endif
}

void debugpins_radio_toggle() {
#ifdef TOWER_K20	
	GPIOA_PTOR |= 1<<RADIO_PIN;
#elif OPENMOTE_K20
	GPIOC_PTOR |= 1<<RADIO_PIN;
#endif
}

void debugpins_radio_clr() {
#ifdef TOWER_K20	
	GPIOA_PCOR |= 1<<RADIO_PIN;
#elif OPENMOTE_K20
	GPIOC_PCOR |= 1<<RADIO_PIN;
#endif
}

void debugpins_radio_set() {
#ifdef TOWER_K20	
	GPIOA_PSOR |= 1<<RADIO_PIN;
#elif OPENMOTE_K20
	GPIOC_PSOR |= 1<<RADIO_PIN;
#endif
}

