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
	TIMSK1 &= ~((1<<ICIE1)|(1<<OCIE1A)|(1<<OCIE1B)|(1<<TOIE1));
	TIFR1 |= (1 << ICF1) | (1 << OCF1A) | (1 << OCF1B) | (1 << TOV1);

	TCCR1A = 0;
	TCCR1B = 0;

	TCNT1 = 0;

	TCCR1A =  (0<<WGM11) | (0<<WGM10);
	TCCR1B |= (0<<WGM13) | (1<<WGM12) | //CTC, TOP = OCR1A
			(1<<CS12) | (0<<CS11) | (1<CS10); //prescaler 1024

	OCR1A = period;

	TIFR1 |= (1 << ICF1) | (1 << OCF1A) | (1 << OCF1B) | (1 << TOV1); //clears bits
	TIMSK1 |= (1 << OCIE1A); //enable ORCRA1 interrupt
}

//===== direct access

PORT_RADIOTIMER_WIDTH radiotimer_getValue() {
	return TCNT1;
}

void radiotimer_setPeriod(PORT_RADIOTIMER_WIDTH period) {
	OCR1A = period;
}

PORT_RADIOTIMER_WIDTH radiotimer_getPeriod() {
	return OCR1A;
}

//===== compare

void radiotimer_schedule(PORT_RADIOTIMER_WIDTH offset) {
	OCR1B = offset; //offset when to fire

	TIMSK1 |= (1 << OCIE1B); //enable OCCRB1 interrupt
}

void radiotimer_cancel() {
	OCR1B = 0; //offset when to fire
	TIMSK1 &= ~(1 << OCIE1B); //disable OCCRB1 interrupt
}

////===== capture

inline PORT_RADIOTIMER_WIDTH radiotimer_getCapturedTime() {
	return TCNT1;
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
