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

// The following pins are aligned on the LPCXpresso1769 board, which makes it
// easy to connect, e.g. a Logic8 or Logic16 login analyzer. We therefore
// use them as debug pins:
// - P0.2  (frame)
// - P0.3  (slot)
// - P0.21 (fsm)
// - P0.22 (task)
// - P0.27 (isr)
// - P0.28 (radio)

void debugpins_init() {

   // configure as GPIO (see pinsel, page 108)
   LPC_PINCON->PINSEL0     &= ~0x3<<4;          // P0.2
   LPC_PINCON->PINSEL0     &= ~0x3<<6;          // P0.3
   LPC_PINCON->PINSEL1     &= ~0x3<<10;         // P0.21
   LPC_PINCON->PINSEL1     &= ~0x3<<12;         // P0.22
   LPC_PINCON->PINSEL1     &= ~0x3<<22;         // P0.27
   LPC_PINCON->PINSEL1     &= ~0x3<<24;         // P0.28

   //set as output
   LPC_GPIO0->FIODIR        |= 1<<2;             // frame
   LPC_GPIO0->FIODIR        |= 1<<3;             // slot
   LPC_GPIO0->FIODIR        |= 1<<21;            // fsm
   LPC_GPIO0->FIODIR        |= 1<<22;            // task
   LPC_GPIO0->FIODIR        |= 1<<27;            // isr
   LPC_GPIO0->FIODIR        |= 1<<28;            // radio
}

void debugpins_frame_toggle() {
   private_pin_toggle(FRAME_PIN);
}
void debugpins_frame_clr() {
   private_pin_off(FRAME_PIN);
}
void debugpins_frame_set() {
   private_pin_on(FRAME_PIN);
}

void debugpins_slot_toggle() {
   private_pin_toggle(SLOT_PIN);
}
void debugpins_slot_clr() {
   private_pin_off(SLOT_PIN);
}
void debugpins_slot_set() {
    private_pin_on(SLOT_PIN);
}

void debugpins_fsm_toggle() {
   private_pin_toggle(FSM_PIN);
}
void debugpins_fsm_clr() {
   private_pin_off(FSM_PIN);
}
void debugpins_fsm_set() {
    private_pin_on(FSM_PIN);
}

void debugpins_task_toggle() {
   private_pin_toggle(TASK_PIN);
}
void debugpins_task_clr() {
   private_pin_off(TASK_PIN);
}
void debugpins_task_set() {
    private_pin_on(TASK_PIN);
}

void debugpins_isr_toggle() {
   private_pin_toggle(ISR_PIN);
}
void debugpins_isr_clr() {
   private_pin_off(ISR_PIN);
}
void debugpins_isr_set() {
   private_pin_on(ISR_PIN);
}

void debugpins_radio_toggle() {
   private_pin_toggle(RADIO_PIN);
}
void debugpins_radio_clr() {
   private_pin_off(RADIO_PIN);
}
void debugpins_radio_set() {
   private_pin_on(RADIO_PIN);
}

//=========================== private =========================================

static void private_pin_on(unsigned int pin) {
   unsigned long pinstate;
   pinstate             = LPC_GPIO0->FIOPIN;     // get led state
   LPC_GPIO0->FIOSET    = ((~pinstate)&(1<<pin));// turn led on
}


static void private_pin_off(unsigned int pin){
   unsigned long pinstate;
   pinstate             = LPC_GPIO0->FIOPIN;     // get led state
   LPC_GPIO0->FIOCLR    = (( pinstate)&(1<<pin));// turn led off
}

// TODO: implement toggle rather than pulse?
static void private_pin_toggle(unsigned int pin){
   unsigned long pinstate;
   pinstate             = LPC_GPIO0->FIOPIN;     // get led state
   LPC_GPIO0->FIOCLR    = (( pinstate)&(1<<pin));// turn led off
   LPC_GPIO0->FIOSET    = ((~pinstate)&(1<<pin));// turn led on
}
