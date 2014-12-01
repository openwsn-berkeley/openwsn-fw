/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Date:   July 2013
 * Description: CC2538-specific definition of the "radiotimer" bsp module.
 */

#include <headers/hw_ints.h>
#include <headers/hw_rfcore_sfr.h>
#include <headers/hw_types.h>

#include "stdio.h"
#include "string.h"
#include "radiotimer.h"
#include "debugpins.h"
#include "interrupt.h"
#include "sys_ctrl.h"
#include "sys_ctrl.h"


#define RADIOTIMER_32MHZ_TICS_PER_32KHZ_TIC     ( 976 ) // 32 MHz to 32 kHz ratio

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

    //set period on the timer to 976 tics
    HWREG(RFCORE_SFR_MTMSEL) = (0x02 << RFCORE_SFR_MTMSEL_MTMSEL_S) & RFCORE_SFR_MTMSEL_MTMSEL_M;

    HWREG(RFCORE_SFR_MTM0)=(RADIOTIMER_32MHZ_TICS_PER_32KHZ_TIC << RFCORE_SFR_MTM0_MTM0_S) & RFCORE_SFR_MTM0_MTM0_M;
    HWREG(RFCORE_SFR_MTM1)=((RADIOTIMER_32MHZ_TICS_PER_32KHZ_TIC>> 8) << RFCORE_SFR_MTM1_MTM1_S)& RFCORE_SFR_MTM1_MTM1_M;

    //set counter on the timer to 0 tics
    HWREG(RFCORE_SFR_MTMSEL) = (0x00 << RFCORE_SFR_MTMSEL_MTMSEL_S) & RFCORE_SFR_MTMSEL_MTMSEL_M;

    HWREG(RFCORE_SFR_MTM0)=(0x00<< RFCORE_SFR_MTM0_MTM0_S) & RFCORE_SFR_MTM0_MTM0_M;
    HWREG(RFCORE_SFR_MTM1)=(0x00<< RFCORE_SFR_MTM1_MTM1_S) & RFCORE_SFR_MTM1_MTM1_M;

    //now overflow increments once every 1 32Khz tic.

    //select period register in the selector so it can be modified -- use OVF  so we have 24bit register
	HWREG(RFCORE_SFR_MTMSEL) = (0x02 << RFCORE_SFR_MTMSEL_MTMOVFSEL_S)& RFCORE_SFR_MTMSEL_MTMOVFSEL_M;
	//set the period now -- low 8bits
	HWREG(RFCORE_SFR_MTMOVF0) = (period << RFCORE_SFR_MTMOVF0_MTMOVF0_S)& RFCORE_SFR_MTMOVF0_MTMOVF0_M;
	//set the period now -- middle 8bits
	HWREG(RFCORE_SFR_MTMOVF1) = ((period >> 8) << RFCORE_SFR_MTMOVF1_MTMOVF1_S)& RFCORE_SFR_MTMOVF1_MTMOVF1_M;
	//set the period now -- high 8bits
	HWREG(RFCORE_SFR_MTMOVF2) = ((period >> 16) << RFCORE_SFR_MTMOVF2_MTMOVF2_S)& RFCORE_SFR_MTMOVF2_MTMOVF2_M;

	//select counter register in the selector so it can be modified -- use OVF version so we can have 24bit register
	HWREG(RFCORE_SFR_MTMSEL) = (0x00<< RFCORE_SFR_MTMSEL_MTMOVFSEL_S) & RFCORE_SFR_MTMSEL_MTMOVFSEL_M;
	//set the period now -- low 8bits
	HWREG(RFCORE_SFR_MTMOVF0) = (0x00 << RFCORE_SFR_MTMOVF0_MTMOVF0_S) & RFCORE_SFR_MTMOVF0_MTMOVF0_M;
	//set the period now -- middle 8bits
	HWREG(RFCORE_SFR_MTMOVF1) = (0x00 << RFCORE_SFR_MTMOVF1_MTMOVF1_S) & RFCORE_SFR_MTMOVF1_MTMOVF1_M;
	//set the period now -- high 8bits
	HWREG(RFCORE_SFR_MTMOVF2) = (0x00 << RFCORE_SFR_MTMOVF2_MTMOVF2_S) & RFCORE_SFR_MTMOVF2_MTMOVF2_M;

    //enable period interrupt - ovf
    HWREG(RFCORE_SFR_MTIRQM) = RFCORE_SFR_MTIRQM_MACTIMER_OVF_PERM;//RFCORE_SFR_MTIRQM_MACTIMER_OVF_PERM|RFCORE_SFR_MTIRQM_MACTIMER_PERM

    //register and enable the interrupt at the nvic
    IntRegister(INT_MACTIMR, radiotimer_isr_private);
    //clear all interrupts

    //active sync with 32khz clock and start the timer.
    HWREG(RFCORE_SFR_MTIRQF)=0x00;

    //enable,synch with 32khz and dont latch 3bytes on ovf counter so we have 24bit timer on the ovf.
    HWREG(RFCORE_SFR_MTCTRL)|=RFCORE_SFR_MTCTRL_RUN|RFCORE_SFR_MTCTRL_SYNC;

    while(!( HWREG(RFCORE_SFR_MTCTRL) & RFCORE_SFR_MTCTRL_STATE));//wait until stable.

    IntEnable(INT_MACTIMR);
}

//===== direct access

PORT_RADIOTIMER_WIDTH radiotimer_getValue() {
	 PORT_RADIOTIMER_WIDTH value=0;
	 //select period register in the selector so it can be read
	 HWREG(RFCORE_SFR_MTMSEL) = (0x00 << RFCORE_SFR_MTMSEL_MTMOVFSEL_S) & RFCORE_SFR_MTMSEL_MTMOVFSEL_M;
	 // compute value by adding m0 and m1 registers
	 value = HWREG(RFCORE_SFR_MTMOVF0);
	 value+=(HWREG(RFCORE_SFR_MTMOVF1)<<8);
     value+=(HWREG(RFCORE_SFR_MTMOVF2)<<16);

	 return value;
}

void radiotimer_setPeriod(PORT_RADIOTIMER_WIDTH period) {
	//select period register in the selector so it can be modified -- use OVF  so we have 24bit register
	HWREG(RFCORE_SFR_MTMSEL) = (0x02 << RFCORE_SFR_MTMSEL_MTMOVFSEL_S)& RFCORE_SFR_MTMSEL_MTMOVFSEL_M;
	//set the period now -- low 8bits
	HWREG(RFCORE_SFR_MTMOVF0) = (period << RFCORE_SFR_MTMOVF0_MTMOVF0_S)& RFCORE_SFR_MTMOVF0_MTMOVF0_M;
	//set the period now -- middle 8bits
	HWREG(RFCORE_SFR_MTMOVF1) = ((period >> 8) << RFCORE_SFR_MTMOVF1_MTMOVF1_S)& RFCORE_SFR_MTMOVF1_MTMOVF1_M;
	//set the period now -- high 8bits
	HWREG(RFCORE_SFR_MTMOVF2) = ((period >> 16) << RFCORE_SFR_MTMOVF2_MTMOVF2_S)& RFCORE_SFR_MTMOVF2_MTMOVF2_M;

    IntEnable(INT_MACTIMR);
}

PORT_RADIOTIMER_WIDTH radiotimer_getPeriod() {
	PORT_RADIOTIMER_WIDTH period=0;
	//select period register in the selector so it can be read
	HWREG(RFCORE_SFR_MTMSEL) = (0x02 << RFCORE_SFR_MTMSEL_MTMOVFSEL_S) & RFCORE_SFR_MTMSEL_MTMOVFSEL_M;
	// compute period by adding m0 and m1 registers
	period = HWREG(RFCORE_SFR_MTMOVF0);
	period+=(HWREG(RFCORE_SFR_MTMOVF1)<<8);
	period+=(HWREG(RFCORE_SFR_MTMOVF2)<<16);

    return period;
}

//===== compare

void radiotimer_schedule(PORT_RADIOTIMER_WIDTH offset) {
	//select ovf cmp1 register in the selector so it can be modified
	HWREG(RFCORE_SFR_MTMSEL) = (0x03 << RFCORE_SFR_MTMSEL_MTMOVFSEL_S) & RFCORE_SFR_MTMSEL_MTMOVFSEL_M;
	//set value
	HWREG(RFCORE_SFR_MTMOVF0) = (offset << RFCORE_SFR_MTMOVF0_MTMOVF0_S)& RFCORE_SFR_MTMOVF0_MTMOVF0_M;
	//set the period now -- middle 8bits
    HWREG(RFCORE_SFR_MTMOVF1) = ((offset >> 8) << RFCORE_SFR_MTMOVF1_MTMOVF1_S) & RFCORE_SFR_MTMOVF1_MTMOVF1_M;
	//set the period now -- high 8bits
	HWREG(RFCORE_SFR_MTMOVF2) = ((offset >> 16) << RFCORE_SFR_MTMOVF2_MTMOVF2_S)& RFCORE_SFR_MTMOVF2_MTMOVF2_M;
    // enable cmp1 ovf interrupt (this also cancels any pending interrupts)
	//clear interrupts
	HWREG(RFCORE_SFR_MTIRQF)=~RFCORE_SFR_MTIRQM_MACTIMER_OVF_COMPARE1M;
	//enable the timer compare interrupt
	HWREG(RFCORE_SFR_MTIRQM)|=RFCORE_SFR_MTIRQM_MACTIMER_OVF_COMPARE1M;
	IntEnable(INT_MACTIMR);
}

void radiotimer_cancel() {
	//select ovf cmp1 register in the selector so it can be modified
	HWREG(RFCORE_SFR_MTMSEL) = (0x03 << RFCORE_SFR_MTMSEL_MTMOVFSEL_S) & RFCORE_SFR_MTMSEL_MTMOVFSEL_M;
	//set value
	HWREG(RFCORE_SFR_MTMOVF0) = (0x00 << RFCORE_SFR_MTMOVF0_MTMOVF0_S)& RFCORE_SFR_MTMOVF0_MTMOVF0_M;
	//set the period now -- middle 8bits
	HWREG(RFCORE_SFR_MTMOVF1) = (0x00 << RFCORE_SFR_MTMOVF1_MTMOVF1_S) & RFCORE_SFR_MTMOVF1_MTMOVF1_M;
	//set the period now -- high 8bits
	HWREG(RFCORE_SFR_MTMOVF2) = (0x00 << RFCORE_SFR_MTMOVF2_MTMOVF2_S)& RFCORE_SFR_MTMOVF2_MTMOVF2_M;
	// disable cmp1 interrupt
	HWREG(RFCORE_SFR_MTIRQM)&=~RFCORE_SFR_MTIRQM_MACTIMER_OVF_COMPARE1M;
	//clear interrupts
	HWREG(RFCORE_SFR_MTIRQF)=~RFCORE_SFR_MTIRQM_MACTIMER_OVF_COMPARE1M;

}

//===== capture

port_INLINE PORT_RADIOTIMER_WIDTH radiotimer_getCapturedTime() {
	 volatile PORT_RADIOTIMER_WIDTH value=0;

	 //select period register in the selector so it can be read
	 HWREG(RFCORE_SFR_MTMSEL) = (0x00 << RFCORE_SFR_MTMSEL_MTMOVFSEL_S) & RFCORE_SFR_MTMSEL_MTMOVFSEL_M;
	 // compute value by adding m0 and m1 registers
	 value = HWREG(RFCORE_SFR_MTMOVF0);
	 value+=(HWREG(RFCORE_SFR_MTMOVF1)<<8);
     value+=(HWREG(RFCORE_SFR_MTMOVF2)<<16);

     return value;
}

//=========================== private =========================================

port_INLINE uint16_t get_real_counter(void){
	uint16_t value=0;
	 //select period register in the selector so it can be read
	HWREG(RFCORE_SFR_MTMSEL) = (0x00 << RFCORE_SFR_MTMSEL_MTMSEL_S)& RFCORE_SFR_MTMSEL_MTMSEL_M;
		 // compute value by adding m0 and m1 registers
    value = HWREG(RFCORE_SFR_MTM0) + (HWREG(RFCORE_SFR_MTM1)<<8);
    return value;
}

//=========================== interrupt handlers ==============================

void radiotimer_isr_private(){
	debugpins_isr_set();
	radiotimer_isr();
	debugpins_isr_clr();
}

/*
 * The interrupt flags are given in the RFCORE_SFR_MTIRQF registers. The interrupt flag bits are set only
by hardware and are cleared only by writing to the SFR register.
Each interrupt source can be masked by its corresponding mask bit in the RFCORE_SFR_MTIRQM
register. An interrupt is generated when the corresponding mask bit is set; otherwise, the interrupt is not
generated. The interrupt flag bit is set, however, regardless of the state of the interrupt mask bit.
 *
 */
kick_scheduler_t radiotimer_isr() {

   uint8_t t2irqm;
   uint8_t t2irqf;

   t2irqm = HWREG(RFCORE_SFR_MTIRQM);
   t2irqf = HWREG(RFCORE_SFR_MTIRQF);

   IntPendClear(INT_MACTIMR);

   if ((t2irqf & RFCORE_SFR_MTIRQM_MACTIMER_OVF_COMPARE1M)& t2irqm){ // compare 1
	   debugpins_isr_toggle();
  	   //clear interrupt
  	   HWREG(RFCORE_SFR_MTIRQF)=~RFCORE_SFR_MTIRQM_MACTIMER_OVF_COMPARE1M;

  	   if (radiotimer_vars.compare_cb!=NULL) {
              // call the callback
              radiotimer_vars.compare_cb();
            // kick the OS
              return KICK_SCHEDULER;
           }
     } else if((t2irqf & RFCORE_SFR_MTIRQM_MACTIMER_OVF_PERM) & t2irqm){ // timer overflows
    	 debugpins_isr_toggle();
	     //clear interrupt
	     HWREG(RFCORE_SFR_MTIRQF)=~RFCORE_SFR_MTIRQM_MACTIMER_OVF_PERM;

	     if (radiotimer_vars.overflow_cb!=NULL) {
               // call the callback
             radiotimer_vars.overflow_cb();
               // kick the OS
             return KICK_SCHEDULER;
          }
   }
   return DO_NOT_KICK_SCHEDULER;
}

