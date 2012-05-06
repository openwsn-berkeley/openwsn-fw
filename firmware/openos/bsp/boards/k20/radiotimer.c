/**
\brief K20-specific definition of the "radiotimer" bsp module. Using opentimers.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, March 2012.
*/


#include "radiotimer.h"
#include "board.h"
#include "opentimers.h"
//pin 0.23 is cap0 for capture.

#define RADIO_TIMER_NOT_SET 0xFF

//=========================== variables =======================================

typedef struct {
   radiotimer_compare_cbt    overflow_cb;
   radiotimer_compare_cbt    compare_cb;
   uint32_t period;
   uint8_t period_timer_id;//id of the opentimer used for period 
   uint8_t offset_timer_id;//id of the opentimer used for offsets within the slot.
} radiotimer_vars_t;

radiotimer_vars_t radiotimer_vars;

//=========================== prototypes ======================================
void private_radiotimer_period_cb(void);
void private_radiotimer_offset_cb(void);
//=========================== public ==========================================

//===== admin

void radiotimer_init() {
   // clear local variables
   memset(&radiotimer_vars,0,sizeof(radiotimer_vars_t));
   radiotimer_vars.offset_timer_id=RADIO_TIMER_NOT_SET;
   radiotimer_vars.period_timer_id=RADIO_TIMER_NOT_SET;
}

void radiotimer_setOverflowCb(radiotimer_compare_cbt cb) {
   radiotimer_vars.overflow_cb    = cb;
}

void radiotimer_setCompareCb(radiotimer_compare_cbt cb) {
   radiotimer_vars.compare_cb     = cb;
}

void radiotimer_setStartFrameCb(radiotimer_capture_cbt cb) {
   while(1);
}

void radiotimer_setEndFrameCb(radiotimer_capture_cbt cb) {
   while(1);
}

void radiotimer_start(PORT_TIMER_WIDTH period) {
	if (radiotimer_vars.period_timer_id!=RADIO_TIMER_NOT_SET){
	 opentimers_stop(radiotimer_vars.period_timer_id);//stop it.	
	}
	radiotimer_vars.period=period;
	radiotimer_vars.period_timer_id=opentimers_start(period,TIMER_ONESHOT,TIME_TICS,private_radiotimer_period_cb);
}

//===== direct access

PORT_TIMER_WIDTH radiotimer_getValue() {
   return 0;
}
//period is in ms???

void radiotimer_setPeriod(PORT_TIMER_WIDTH period) {
	opentimers_stop(radiotimer_vars.period_timer_id);
	radiotimer_vars.period=period;
	radiotimer_vars.period_timer_id=opentimers_start(period,TIMER_ONESHOT,TIME_TICS,private_radiotimer_offset_cb);
}




PORT_TIMER_WIDTH radiotimer_getPeriod() {
   return (PORT_TIMER_WIDTH)radiotimer_vars.period;
}

//===== compare

void radiotimer_schedule(PORT_TIMER_WIDTH offset) {
   radiotimer_vars.offset_timer_id=opentimers_start(offset,TIMER_ONESHOT,TIME_TICS,private_radiotimer_period_cb);	
}

void radiotimer_cancel() {
   // reset the timer.
if (radiotimer_vars.offset_timer_id!=RADIO_TIMER_NOT_SET){
	opentimers_stop(radiotimer_vars.offset_timer_id);
	radiotimer_vars.offset_timer_id=RADIO_TIMER_NOT_SET;
}

if (radiotimer_vars.period_timer_id!=RADIO_TIMER_NOT_SET){
	opentimers_stop(radiotimer_vars.period_timer_id);
	radiotimer_vars.period_timer_id=RADIO_TIMER_NOT_SET;
}
}

//===== capture

inline PORT_TIMER_WIDTH radiotimer_getCapturedTime() {
	PORT_TIMER_WIDTH wi;
	PORT_TIMER_WIDTH wa;
	return wi;
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================


void private_radiotimer_period_cb(void){
	//call overflow c
	radiotimer_vars.overflow_cb();
	radiotimer_vars.period_timer_id=RADIO_TIMER_NOT_SET;//it is one shot. so declare it as not set
	
}
void private_radiotimer_offset_cb(void){
	radiotimer_vars.compare_cb();
	radiotimer_vars.offset_timer_id=RADIO_TIMER_NOT_SET;//it is one shot. so declare it as not set
	
}
