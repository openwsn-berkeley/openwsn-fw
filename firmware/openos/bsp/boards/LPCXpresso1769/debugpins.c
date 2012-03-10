/**
\brief LPCXpreeso1769-specific definition of the "debugpins" bsp module.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, March 2012.
*/


#include "debugpins.h"
#include "LPC17xx.h"
#include "board.h"

//=========================== defines =========================================

#define FRAME_PIN 2
#define SLOT_PIN 3
#define FSM_PIN 21
#define TASK_PIN 22
#define ISR_PIN 27
#define RADIO_PIN 28

//=========================== variables =======================================

//=========================== prototypes ======================================

static void private_pin_on(unsigned int pin);
static void private_pin_off(unsigned int pin);
static void private_pin_toggle(unsigned int pin);

//=========================== public ==========================================
//p0.2,0.3,0.21,0.22,0.27,0.28 -- this pins are all in line in the lpcxpresso 1769 board.

void debugpins_init() {


	LPC_PINCON->PINSEL0 &= ~0x3<<4; //set pinsel to GPIO pag 108 port 0.2
	LPC_PINCON->PINSEL0 &= ~0x3<<6; //port 0.3
	LPC_PINCON->PINSEL1 &= ~0x3<<10;//port 0.21
	LPC_PINCON->PINSEL1 &= ~0x3<<12;//port 0.22
	LPC_PINCON->PINSEL1 &= ~0x3<<22;//port 0.27
	LPC_PINCON->PINSEL1 &= ~0x3<<24;//port 0.28

	//set as output
	LPC_GPIO0->FIODIR 	|= 1<<2;//frame
	LPC_GPIO0->FIODIR 	|= 1<<3;//slot
	LPC_GPIO0->FIODIR 	|= 1<<21;//fsm
	LPC_GPIO0->FIODIR 	|= 1<<22;//task
	LPC_GPIO0->FIODIR 	|= 1<<27;//isr
	LPC_GPIO0->FIODIR 	|= 1<<28;//radio
}

// P0.2
void debugpins_frame_toggle() {
	private_pin_toggle(FRAME_PIN);
}
void debugpins_frame_clr() {
	private_pin_off(FRAME_PIN);
}
void debugpins_frame_set() {
   private_pin_on(FRAME_PIN);
}

// P0.3
void debugpins_slot_toggle() {
	private_pin_toggle(SLOT_PIN);
}
void debugpins_slot_clr() {
	private_pin_off(SLOT_PIN);
}
void debugpins_slot_set() {
	 private_pin_on(SLOT_PIN);
}

// P0.22
void debugpins_fsm_toggle() {
	private_pin_toggle(FSM_PIN);
}
void debugpins_fsm_clr() {
	private_pin_off(FSM_PIN);
}
void debugpins_fsm_set() {
	 private_pin_on(FSM_PIN);
}

// P0.23
void debugpins_task_toggle() {
	private_pin_toggle(TASK_PIN);
}
void debugpins_task_clr() {
	private_pin_off(TASK_PIN);
}
void debugpins_task_set() {
	 private_pin_on(TASK_PIN);
}

// P0.27
void debugpins_isr_toggle() {
	private_pin_toggle(ISR_PIN);
}
void debugpins_isr_clr() {
	private_pin_off(ISR_PIN);
}
void debugpins_isr_set() {
	private_pin_on(ISR_PIN);
}

// P0.28
void debugpins_radio_toggle() {
	private_pin_toggle(RADIO_PIN);
}
void debugpins_radio_clr() {
	private_pin_off(RADIO_PIN);
}
void debugpins_radio_set() {
	private_pin_on(RADIO_PIN);
}


static void private_pin_on(unsigned int pin){
	unsigned long pinstate;

	pinstate = LPC_GPIO0->FIOPIN;//get led state
	LPC_GPIO0->FIOSET = ( ( ~pinstate ) & ( 1 << pin ) );//turn it on! does nothing if it is already on
}


static void private_pin_off(unsigned int pin){
	unsigned long pinstate;

	pinstate = LPC_GPIO0->FIOPIN;	//get led state
	LPC_GPIO0->FIOCLR = pinstate & ( 1 << pin );//turn it off! does nothing if already off
}

static void private_pin_toggle(unsigned int pin){
	unsigned long pinstate;
	pinstate = LPC_GPIO0->FIOPIN;	//get led pin
	LPC_GPIO0->FIOCLR = pinstate & ( 1 << ledbit );//turn it off! does nothing if already off
	LPC_GPIO0->FIOSET = ( ( ~pinstate ) & ( 1 << pin ) );//turn it on! does nothing if it is already on
}
