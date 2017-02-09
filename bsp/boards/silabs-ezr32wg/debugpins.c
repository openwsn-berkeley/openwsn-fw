/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Date:   Jan 2016
 * Description: EZR32WG-specific definition of the "debugpins" bsp module.
 */

#include "debugpins.h"
#include "stdint.h"
#include "board.h"
#include "board_info.h"
#include "em_device.h"
#include "em_cmu.h"
#include "em_gpio.h"
//=========================== defines =========================================

// Board PINS defines
#define GPIO_PORT_SLOT 6
#define GPIO_PORT_FRAME 7
#define GPIO_PORT_ISR 0
#define GPIO_PORT_TASK 1
#define GPIO_PORT_FSM 11
#define GPIO_PORT_RADIO 3

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void debugpins_init() {
	//enable clock for this peripheral
	CMU_ClockEnable(cmuClock_HFPER, true);
	CMU_ClockEnable(cmuClock_GPIO, true);

	//set the led pins to output.
	GPIO_PinModeSet(gpioPortC, GPIO_PORT_SLOT, gpioModePushPull, 0);
	GPIO_PinModeSet(gpioPortC, GPIO_PORT_FRAME, gpioModePushPull, 0);
	GPIO_PinModeSet(gpioPortE, GPIO_PORT_ISR, gpioModePushPull, 0);
	GPIO_PinModeSet(gpioPortE, GPIO_PORT_TASK, gpioModePushPull, 0);
	GPIO_PinModeSet(gpioPortB, GPIO_PORT_FSM, gpioModePushPull, 0);
	GPIO_PinModeSet(gpioPortF, GPIO_PORT_RADIO, gpioModePushPull, 0);
}

// PD1
void debugpins_frame_toggle() {
	GPIO_PinOutToggle(gpioPortC, GPIO_PORT_FRAME);
}

void debugpins_frame_clr() {
	GPIO_PinOutClear(gpioPortC, GPIO_PORT_FRAME);
}

void debugpins_frame_set() {
	GPIO_PinOutSet(gpioPortC, GPIO_PORT_FRAME);
}

// PD3
void debugpins_slot_toggle() {
	GPIO_PinOutToggle(gpioPortC, GPIO_PORT_SLOT);
}

void debugpins_slot_clr() {
	GPIO_PinOutClear(gpioPortC, GPIO_PORT_SLOT);
}

void debugpins_slot_set() {
	GPIO_PinOutSet(gpioPortC, GPIO_PORT_SLOT);
}

// PD2
void debugpins_fsm_toggle() {
	GPIO_PinOutToggle(gpioPortB, GPIO_PORT_FSM);
}

void debugpins_fsm_clr() {
	GPIO_PinOutClear(gpioPortB, GPIO_PORT_FSM);
}

void debugpins_fsm_set() {
	GPIO_PinOutSet(gpioPortB, GPIO_PORT_FSM);
}

// PD1
void debugpins_task_toggle() {
	GPIO_PinOutToggle(gpioPortE, GPIO_PORT_TASK);
}

void debugpins_task_clr() {
	GPIO_PinOutClear(gpioPortE, GPIO_PORT_TASK);
}

void debugpins_task_set() {
	GPIO_PinOutSet(gpioPortE, GPIO_PORT_TASK);
}

// PA5
void debugpins_isr_toggle() {
	GPIO_PinOutToggle(gpioPortE, GPIO_PORT_ISR);
}

void debugpins_isr_clr() {
	GPIO_PinOutClear(gpioPortE, GPIO_PORT_ISR);
}

void debugpins_isr_set() {
	GPIO_PinOutSet(gpioPortE, GPIO_PORT_ISR);
}

// PD0
void debugpins_radio_toggle() {
	GPIO_PinOutToggle(gpioPortF, GPIO_PORT_RADIO);

}

void debugpins_radio_clr() {
	GPIO_PinOutClear(gpioPortF, GPIO_PORT_RADIO);

}

void debugpins_radio_set() {
	GPIO_PinOutSet(gpioPortF, GPIO_PORT_RADIO);
}

//------------ private ------------//

