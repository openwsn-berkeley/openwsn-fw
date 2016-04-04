/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Date:   July 2013
 * Description: CC2538-specific definition of the "radiotimer" bsp module.
 */

#include "stdio.h"
#include "string.h"
#include "radiotimer.h"
#include "debugpins.h"


//=========================== variables =======================================

typedef struct {
   radiotimer_compare_cbt    overflow_cb;
   radiotimer_compare_cbt    compare_cb;
} radiotimer_vars_t;

radiotimer_vars_t radiotimer_vars;

//=========================== prototypes ======================================

void radiotimer_isr_private(void);
port_INLINE uint16_t get_real_counter(void);

//=========================== public ==========================================

//===== admin

void radiotimer_init() {
   // clear local variables
   memset(&radiotimer_vars,0,sizeof(radiotimer_vars_t));
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

void radiotimer_start(PORT_RADIOTIMER_WIDTH period) {


}

//===== direct access

PORT_RADIOTIMER_WIDTH radiotimer_getValue() {
	 PORT_RADIOTIMER_WIDTH value=0;
	 //select period register in the selector so it can be read

	 return value;
}

void radiotimer_setPeriod(PORT_RADIOTIMER_WIDTH period) {
	//select period register in the selector so it can be modified -- use OVF  so we have 24bit register

}

PORT_RADIOTIMER_WIDTH radiotimer_getPeriod() {
	PORT_RADIOTIMER_WIDTH period=0;


    return period;
}

//===== compare

void radiotimer_schedule(PORT_RADIOTIMER_WIDTH offset) {
	//select ovf cmp1 register in the selector so it can be modified

}

void radiotimer_cancel() {


}

//===== capture

port_INLINE PORT_RADIOTIMER_WIDTH radiotimer_getCapturedTime() {
	 PORT_RADIOTIMER_WIDTH value=0;

     return value;
}

//=========================== private =========================================

port_INLINE uint16_t get_real_counter(void){
	uint16_t value=0;

    return value;
}

//=========================== interrupt handlers ==============================

void radiotimer_isr_private(){
	debugpins_isr_set();
	radiotimer_isr();
	debugpins_isr_clr();
}


kick_scheduler_t radiotimer_isr() {

    return DO_NOT_KICK_SCHEDULER;
}

