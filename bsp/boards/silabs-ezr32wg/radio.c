/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Date:   Jan 2016
 * Description: EZR32WG-specific definition of the "radio" bsp module.
 */

#include "board.h"
#include "radio.h"
#include "leds.h"
#include "stdio.h"
#include "string.h"
#include "radiotimer.h"
#include "debugpins.h"

//=========================== defines =========================================

/* RSSI Offset */
#define RSSI_OFFSET 73
#define CHECKSUM_LEN 2

//=========================== variables =======================================

typedef struct {
	radiotimer_capture_cbt startFrame_cb;
	radiotimer_capture_cbt endFrame_cb;
	radio_state_t state;
} radio_vars_t;

radio_vars_t radio_vars;

//=========================== prototypes ======================================

void enable_radio_interrupts(void);
void disable_radio_interrupts(void);

void radio_on(void);
void radio_off(void);

void radio_error_isr(void);
void radio_isr_internal(void);

//=========================== public ==========================================

//===== admin

void radio_init(void) {

	// clear variables
	memset(&radio_vars, 0, sizeof(radio_vars_t));

	// change state
	radio_vars.state = RADIOSTATE_STOPPED;

	//flush fifos

	radio_off();

	//disable radio interrupts
	disable_radio_interrupts();

	//register interrupt

	/* Enable all RF Error interrupts */

	// change state
	radio_vars.state = RADIOSTATE_RFOFF;
}

void radio_setOverflowCb(radiotimer_compare_cbt cb) {
	radiotimer_setOverflowCb(cb);
}

void radio_setCompareCb(radiotimer_compare_cbt cb) {
	radiotimer_setCompareCb(cb);
}

void radio_setStartFrameCb(radiotimer_capture_cbt cb) {
	radio_vars.startFrame_cb = cb;
}

void radio_setEndFrameCb(radiotimer_capture_cbt cb) {
	radio_vars.endFrame_cb = cb;
}

//===== reset

void radio_reset(void) {

}

//===== timer

void radio_startTimer(PORT_TIMER_WIDTH period) {
	radiotimer_start(period);
}

PORT_TIMER_WIDTH radio_getTimerValue(void) {
	return radiotimer_getValue();
}

void radio_setTimerPeriod(PORT_TIMER_WIDTH period) {
	radiotimer_setPeriod(period);
}

PORT_TIMER_WIDTH radio_getTimerPeriod(void) {
	return radiotimer_getPeriod();
}

//===== RF admin

void radio_setConfig (radioSetting_t radioSetting){
    selected_radioSetting = radioSetting;
    //do nothing
}

void radio_setFrequency(uint8_t frequency) {

	// change state
	radio_vars.state = RADIOSTATE_SETTING_FREQUENCY;

	radio_vars.state = RADIOSTATE_FREQUENCY_SET;
}

void radio_rfOn(void) {

}

void radio_rfOff(void) {

	// change state
	radio_vars.state = RADIOSTATE_TURNING_OFF;
	radio_off();
	// wiggle debug pin
	debugpins_radio_clr();
	leds_radio_off();
	//enable radio interrupts
	disable_radio_interrupts();

	// change state
	radio_vars.state = RADIOSTATE_RFOFF;
}

//===== TX

void radio_loadPacket(uint8_t* packet, uint8_t len) {
	uint8_t i = 0;

	// change state
	radio_vars.state = RADIOSTATE_LOADING_PACKET;

	// load packet in

	// change state
	radio_vars.state = RADIOSTATE_PACKET_LOADED;
}

void radio_txEnable(void) {

	// change state
	radio_vars.state = RADIOSTATE_ENABLING_TX;

	// wiggle debug pin
	debugpins_radio_set();
	leds_radio_on();

	//do nothing -- radio is activated by the strobe on rx or tx
	//radio_rfOn();

	// change state
	radio_vars.state = RADIOSTATE_TX_ENABLED;
}

void radio_txNow(void) {
	PORT_TIMER_WIDTH count;

	// change state
	radio_vars.state = RADIOSTATE_TRANSMITTING;

	//enable radio interrupts
	enable_radio_interrupts();

	//make sure we are not transmitting already

	//sent packet

}

//===== RX

void radio_rxEnable(void) {

	// change state
	radio_vars.state = RADIOSTATE_ENABLING_RX;

	//enable radio interrupts

	// do nothing as we do not want to receive anything yet.
	// wiggle debug pin
	debugpins_radio_set();
	leds_radio_on();

	// change state
	radio_vars.state = RADIOSTATE_LISTENING;
}

void radio_rxNow(void) {
	//empty buffer before receiving

	//enable radio interrupts
	enable_radio_interrupts();

}

void radio_getReceivedFrame(uint8_t* pBufRead, uint8_t* pLenRead,
		uint8_t maxBufLen, int8_t* pRssi, uint8_t* pLqi,
		bool* pCrc) {

}

//=========================== private =========================================

void enable_radio_interrupts(void) {

}

void disable_radio_interrupts(void) {

}

void radio_on(void) {

}

void radio_off(void) {
}

//=========================== callbacks =======================================

//=========================== interrupt handlers ==============================

/**
 \brief Stub function for the EZR32WG.

 In MSP430 platforms the CPU status after servicing an interrupt can be managed
 toggling some bits in a special register, e.g. CPUOFF, LPM1, etc, within the
 interrupt context itself. By default, after servicing an interrupt the CPU will
 be off so it makes sense to return a value and enable it if something has
 happened that needs the scheduler to run (a packet has been received that needs
 to be processed). Otherwise, the CPU is kept in sleep mode without even
 reaching the main loop.
 */
kick_scheduler_t radio_isr(void) {
	return DO_NOT_KICK_SCHEDULER;
}

void radio_isr_internal(void) {
	volatile PORT_TIMER_WIDTH capturedTime;
	uint8_t irq_status0, irq_status1;

	// capture the time
	capturedTime = radiotimer_getCapturedTime();

	// reading IRQ_STATUS


	return;
}

void radio_error_isr(void) {
	uint8_t rferrm;
}
