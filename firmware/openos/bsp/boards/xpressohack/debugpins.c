/**
\brief LPCXpreeso1769-specific definition of the "debugpins" bsp module.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, March 2012.
 */

#include "debugpins.h"
#include "LPC17xx.h"
#include "board.h"

#define LPCXPRESSO1769

//=========================== defines =========================================
#ifdef OPENMOTE
#define FRAME_PIN 31
#define SLOT_PIN 30
#define FSM_PIN 26
#define TASK_PIN 25
#define ISR_PIN 1
#define RADIO_PIN 0
#endif

#ifdef LPCXPRESSO1769
#define FRAME_PIN 16
#define SLOT_PIN 24
#define FSM_PIN 25
#define TASK_PIN 26
#define ISR_PIN 30
#define RADIO_PIN 31
#endif

#define GPIO0 0
#define GPIO1 1

//=========================== variables =======================================

//=========================== prototypes ======================================

static void private_pin_on(uint8_t gpio,unsigned int pin);
static void private_pin_off(uint8_t gpio,unsigned int pin);
static void private_pin_toggle(uint8_t gpio,unsigned int pin);

//=========================== public ==========================================

// The following pins are aligned on the openmote board, which makes it
// easy to connect, e.g. a Logic8 or Logic16 login analyzer. We therefore
// use them as debug pins:
// - P1.31  (frame)
// - P1.30  (slot)
// - P0.26 (fsm)
// - P0.25 (task)
// - P0.1 (isr)
// - P0.0 (radio)

void debugpins_init() {

#ifdef OPENMOTE
	// configure as GPIO (see pinsel, page 108)
	LPC_PINCON->PINSEL3     &= ~0x3<<30;          // P1.31
	LPC_PINCON->PINSEL3     &= ~0x3<<28;          // P1.30
	LPC_PINCON->PINSEL1     &= ~0x3<<20;         // P0.26
	LPC_PINCON->PINSEL1     &= ~0x3<<18;         // P0.25
	LPC_PINCON->PINSEL0     &= ~0x3<<2;         // P0.1
	LPC_PINCON->PINSEL0     &= ~0x3<<0;         // P0.0

	LPC_GPIO1->FIODIR        |= 1<<FRAME_PIN;             // frame
	LPC_GPIO1->FIODIR        |= 1<<SLOT_PIN;             // slot
	LPC_GPIO0->FIODIR        |= 1<<ISR_PIN;            // isr
	LPC_GPIO0->FIODIR        |= 1<<RADIO_PIN;            // radio

#endif


#ifdef LPCXPRESSO1769
	// configure as GPIO (see pinsel, page 108)
	LPC_PINCON->PINSEL1     &= ~0x3<<0;          // P0.16
	LPC_PINCON->PINSEL1     &= ~0x3<<16;          // P0.24
	LPC_PINCON->PINSEL1     &= ~0x3<<18;         // P0.25
	LPC_PINCON->PINSEL1     &= ~0x3<<20;         // P0.26
	LPC_PINCON->PINSEL3     &= ~0x3<<28;         // P1.30
	LPC_PINCON->PINSEL3     &= ~0x3<<30;         // P1.31

	LPC_GPIO0->FIODIR        |= 1<<FRAME_PIN;             // frame
	LPC_GPIO0->FIODIR        |= 1<<SLOT_PIN;             // slot
	LPC_GPIO1->FIODIR        |= 1<<ISR_PIN;            // isr
	LPC_GPIO1->FIODIR        |= 1<<RADIO_PIN;            // radio

#endif
	//set as output
	LPC_GPIO0->FIODIR        |= 1<<FSM_PIN;            // fsm
	LPC_GPIO0->FIODIR        |= 1<<TASK_PIN;            // task

}

void debugpins_frame_toggle() {
#ifdef OPENMOTE
	private_pin_toggle(GPIO1,FRAME_PIN);
#else
	private_pin_toggle(GPIO0,FRAME_PIN);
#endif


}
void debugpins_frame_clr() {
#ifdef OPENMOTE
	private_pin_off(GPIO1,FRAME_PIN);
#else
	private_pin_off(GPIO0,FRAME_PIN);
#endif

}
void debugpins_frame_set() {
#ifdef OPENMOTE
	private_pin_on(GPIO1,FRAME_PIN);
#else
	private_pin_on(GPIO0,FRAME_PIN);
#endif
}

void debugpins_slot_toggle() {
#ifdef OPENMOTE
	private_pin_toggle(GPIO1,SLOT_PIN);
#else
	private_pin_toggle(GPIO0,SLOT_PIN);
#endif

}
void debugpins_slot_clr() {
#ifdef OPENMOTE
	private_pin_off(GPIO1,SLOT_PIN);
#else
	private_pin_off(GPIO0,SLOT_PIN);
#endif
}
void debugpins_slot_set() {
#ifdef OPENMOTE
	private_pin_on(GPIO1,SLOT_PIN);
#else
	private_pin_on(GPIO0,SLOT_PIN);
#endif
}

void debugpins_fsm_toggle() {
	private_pin_toggle(GPIO0,FSM_PIN);
}
void debugpins_fsm_clr() {
	private_pin_off(GPIO0,FSM_PIN);
}
void debugpins_fsm_set() {
	private_pin_on(GPIO0,FSM_PIN);
}

void debugpins_task_toggle() {
	private_pin_toggle(GPIO0,TASK_PIN);
}
void debugpins_task_clr() {
	private_pin_off(GPIO0,TASK_PIN);
}
void debugpins_task_set() {
	private_pin_on(GPIO0,TASK_PIN);
}

void debugpins_isr_toggle() {
#ifdef OPENMOTE
	private_pin_toggle(GPIO0,ISR_PIN);
#else
	private_pin_toggle(GPIO1,ISR_PIN);
#endif

}
void debugpins_isr_clr() {
#ifdef OPENMOTE
	private_pin_off(GPIO0,ISR_PIN);
#else
	private_pin_off(GPIO1,ISR_PIN);
#endif
}
void debugpins_isr_set() {
#ifdef OPENMOTE
	private_pin_on(GPIO0,ISR_PIN);
#else
	private_pin_on(GPIO1,ISR_PIN);
#endif
}

void debugpins_radio_toggle() {
#ifdef OPENMOTE
	private_pin_toggle(GPIO0,RADIO_PIN);
#else
	private_pin_toggle(GPIO1,RADIO_PIN);
#endif
}
void debugpins_radio_clr() {
#ifdef OPENMOTE
	private_pin_off(GPIO0,RADIO_PIN);
#else
	private_pin_off(GPIO1,RADIO_PIN);
#endif
}
void debugpins_radio_set() {
#ifdef OPENMOTE
	private_pin_on(GPIO0,RADIO_PIN);
#else
	private_pin_on(GPIO1,RADIO_PIN);
#endif
}

//=========================== private =========================================

static void private_pin_on(uint8_t gpio,unsigned int pin) {
	unsigned long pinstate;
	switch (gpio){
	case GPIO0:
		pinstate             = LPC_GPIO0->FIOPIN;     // get led state
		LPC_GPIO0->FIOSET    = ((~pinstate)&(1<<pin));// turn led on
		break;
	case GPIO1:
		pinstate             = LPC_GPIO1->FIOPIN;     // get led state
		LPC_GPIO1->FIOSET    = ((~pinstate)&(1<<pin));// turn led on
		break;
	}
}


static void private_pin_off(uint8_t gpio,unsigned int pin){
	unsigned long pinstate;
	switch (gpio){
	case GPIO0:
		pinstate             = LPC_GPIO0->FIOPIN;     // get led state
		LPC_GPIO0->FIOCLR    = (( pinstate)&(1<<pin));// turn led off
		break;
	case GPIO1:
		pinstate             = LPC_GPIO1->FIOPIN;     // get led state
		LPC_GPIO1->FIOCLR    = (( pinstate)&(1<<pin));// turn led off
		break;
	}
}

// TODO: implement toggle rather than pulse?
static void private_pin_toggle(uint8_t gpio,unsigned int pin){
	unsigned long pinstate;
	switch (gpio){
	case GPIO0:
		pinstate             = LPC_GPIO0->FIOPIN;     // get led state
		LPC_GPIO0->FIOCLR    = (( pinstate)&(1<<pin));// turn led off
		LPC_GPIO0->FIOSET    = ((~pinstate)&(1<<pin));// turn led on
		break;
	case GPIO1:
		pinstate             = LPC_GPIO1->FIOPIN;     // get led state
		LPC_GPIO1->FIOCLR    = (( pinstate)&(1<<pin));// turn led off
		LPC_GPIO1->FIOSET    = ((~pinstate)&(1<<pin));// turn led on
		break;
	}
}
