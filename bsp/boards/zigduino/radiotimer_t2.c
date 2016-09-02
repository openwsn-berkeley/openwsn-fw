/**
\brief Zigduino definition of the "radiotimer" bsp module.

\author Sven Akkermans <sven.akkermans@cs.kuleuven.be>, September 2015.
 */

#include "board_info.h"
#include "radiotimer.h"
#include "radio.h"

//=========================== variables =======================================

typedef struct {
	radiotimer_compare_cbt    overflow_cb;
	radiotimer_compare_cbt    compare_cb;
} radiotimer_vars_t;

radiotimer_vars_t radiotimer_vars;

//=========================== prototypes ======================================

kick_scheduler_t radiotimer_overflow_isr();
kick_scheduler_t radiotimer_compare_isr();
//=========================== public ==========================================

//===== admin

void radiotimer_init() {
	memset(&radiotimer_vars,0,sizeof(radiotimer_vars_t));    // clear local variables
}

void radiotimer_setOverflowCb(radiotimer_compare_cbt cb) {
	radiotimer_vars.overflow_cb     = cb;
}

void radiotimer_setCompareCb(radiotimer_compare_cbt cb) {
	radiotimer_vars.compare_cb      = cb;
}

void radiotimer_setStartFrameCb(radiotimer_capture_cbt cb) {
	while(1);
}

void radiotimer_setEndFrameCb(radiotimer_capture_cbt cb) {
	while(1);
}

void radiotimer_start(PORT_RADIOTIMER_WIDTH period) {
	TIMSK2 &= ~((1<<OCIE2A)|(1<<OCIE2B)|(1<<TOIE2));
	TIFR2 |=  (1 << OCF2A) | (1 << OCF2B) | (1 << TOV2);
	ASSR &= ~(1<<EXCLK);
	ASSR |= (1<<AS2);

	TCCR2A = 0;
	TCCR2B = 0;

	TCNT2 = 0;

	TCCR2A =  (0<<WGM21) | (0<<WGM20);
	TCCR2B |= (0<<WGM22) | //normal operation
			(1<<CS22) | (1<<CS21) | (1<CS20); //prescaler 128

    //wait for registers update
    while (ASSR & ((1<<TCN2UB)|(1<<TCR2BUB)));

	OCR2A = period;

	TIFR2 |= (1 << OCF2A) | (1 << OCF2B) | (1 << TOV2); //clears bits
	TIMSK2 |= (1 << OCIE2A); //enable ORCRA2 interrupt
}

//===== direct access

PORT_RADIOTIMER_WIDTH radiotimer_getValue() {
	return TCNT2;
}

void radiotimer_setPeriod(PORT_RADIOTIMER_WIDTH period) {
	OCR2A = period;
}

PORT_RADIOTIMER_WIDTH radiotimer_getPeriod() {
	return OCR2A;
}

//===== compare

void radiotimer_schedule(PORT_RADIOTIMER_WIDTH offset) {
	OCR2B = offset; //offset when to fire

	TIMSK2 |= (1 << OCIE2B); //enable OCCRB1 interrupt
}

void radiotimer_cancel() {
	OCR2B = 0; //offset when to fire
	TIMSK2 &= ~(1 << OCIE2B); //disable OCCRB1 interrupt
}

////===== capture

inline PORT_RADIOTIMER_WIDTH radiotimer_getCapturedTime() {
	return TCNT2;
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

/**
\brief TimerB CCR1-6 interrupt service routine
 */
kick_scheduler_t radiotimer_isr() {
	while(1);
}

kick_scheduler_t radiotimer_compare_isr() {
	if (radiotimer_vars.compare_cb!=NULL) {
		// call the callback
		radiotimer_vars.compare_cb();
		// kick the OS
		return KICK_SCHEDULER;
	}
	return DO_NOT_KICK_SCHEDULER;
}

kick_scheduler_t radiotimer_overflow_isr() {
	if (radiotimer_vars.overflow_cb!=NULL) {
		// call the callback
		radiotimer_vars.overflow_cb();
		// kick the OS
		return KICK_SCHEDULER;
	}
	return DO_NOT_KICK_SCHEDULER;
}
