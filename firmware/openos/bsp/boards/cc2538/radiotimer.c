/**
\brief cc2538-specific definition of the "radiotimer" bsp module.

\author Xavier Vilajosana <xvilajosana@eecs.berkeley.edu>, August 2013.
*/


#include "stdio.h"
#include "string.h"
#include "radiotimer.h"
#include "hw_rfcore_sfr.h"
#include "interrupt.h"
#include "sys_ctrl.h"
#include "hw_ints.h"
#include "hw_types.h"
//=========================== variables =======================================

typedef struct {
   radiotimer_compare_cbt    overflow_cb;
   radiotimer_compare_cbt    compare_cb;
} radiotimer_vars_t;

radiotimer_vars_t radiotimer_vars;

//=========================== prototypes ======================================

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

void radiotimer_start(uint16_t period) {
   
   //selects the event that triggers MacTimer isr. selecting compare 1 and overflow
   HWREG(RFCORE_SFR_MTCSPCFG)|=(0x01 && RFCORE_SFR_MTCSPCFG_MACTIMER_EVENT1_CFG_M)<<RFCORE_SFR_MTCSPCFG_MACTIMER_EVENT1_CFG_S;

   //clear all interrupts
   HWREG(RFCORE_SFR_MTIRQF)=0;

   //select period register in the selector so it can be modified
   HWREG(RFCORE_SFR_MTMSEL)= (0x02 && RFCORE_SFR_MTMSEL_MTMSEL_M) << RFCORE_SFR_MTMSEL_MTMSEL_S;
   //set the period now -- low 8bits
   HWREG(RFCORE_SFR_MTM0)=(period && RFCORE_SFR_MTM0_MTM0_M)<<RFCORE_SFR_MTM0_MTM0_S;
   //set the period now -- high 8bits
   HWREG(RFCORE_SFR_MTM1)=(period>>8 && RFCORE_SFR_MTM1_MTM1_M)<<RFCORE_SFR_MTM1_MTM1_S;

   //select counter register in the selector so it can be modified
   HWREG(RFCORE_SFR_MTMSEL)= (0x00 && RFCORE_SFR_MTMSEL_MTMSEL_M) << RFCORE_SFR_MTMSEL_MTMSEL_S;
   //set the counter to 0 now -- low 8bits
   HWREG(RFCORE_SFR_MTM0)=(0x00 && RFCORE_SFR_MTM0_MTM0_M)<<RFCORE_SFR_MTM0_MTM0_S;
   //set the counter to 0 now -- high 8bits
   HWREG(RFCORE_SFR_MTM1)=(0x00 && RFCORE_SFR_MTM1_MTM1_M)<<RFCORE_SFR_MTM1_MTM1_S;

   //enable interrupts
   HWREG(RFCORE_SFR_MTIRQM)|=RFCORE_SFR_MTIRQM_MACTIMER_PERM;


   // The rf peripheral must be enabled for use.
   SysCtrlPeripheralEnable(SYS_CTRL_PERIPH_RFC);

   //register and enable the interrupt at the nvic
   IntRegister(INT_MACTIMR, radiotimer_isr);
   IntEnable(INT_MACTIMR);

   //active sync with 32khz clock and start the timer.

   HWREG(RFCORE_SFR_MTCTRL)|=RFCORE_SFR_MTCTRL_SYNC|RFCORE_SFR_MTCTRL_RUN;
}

//===== direct access

PORT_TIMER_WIDTH radiotimer_getValue() {
	 uint16_t value=0;
	 //select period register in the selector so it can be read
	 HWREG(RFCORE_SFR_MTMSEL)= (0x00 && RFCORE_SFR_MTMSEL_MTMSEL_M) << RFCORE_SFR_MTMSEL_MTMSEL_S;
	 // compute value by adding m0 and m1 registers
	 value = HWREG(RFCORE_SFR_MTM0) + (HWREG(RFCORE_SFR_MTM1)<<8);
	 return value;
}

void radiotimer_setPeriod(uint16_t period) {
	   //select period register in the selector so it can be modified
	   HWREG(RFCORE_SFR_MTMSEL)= (0x02 && RFCORE_SFR_MTMSEL_MTMSEL_M) << RFCORE_SFR_MTMSEL_MTMSEL_S;
	   //set the period now -- low 8bits
	   HWREG(RFCORE_SFR_MTM0)=(period && RFCORE_SFR_MTM0_MTM0_M)<<RFCORE_SFR_MTM0_MTM0_S;
	   //set the period now -- high 8bits
	   HWREG(RFCORE_SFR_MTM1)=(period>>8 && RFCORE_SFR_MTM1_MTM1_M)<<RFCORE_SFR_MTM1_MTM1_S;
}

PORT_TIMER_WIDTH radiotimer_getPeriod() {
	PORT_TIMER_WIDTH period=0;
	//select period register in the selector so it can be read
    HWREG(RFCORE_SFR_MTMSEL)= (0x02 && RFCORE_SFR_MTMSEL_MTMSEL_M) << RFCORE_SFR_MTMSEL_MTMSEL_S;
	// compute period by adding m0 and m1 registers
    period = HWREG(RFCORE_SFR_MTM0) + (HWREG(RFCORE_SFR_MTM1)<<8);
    return period;
}

//===== compare

void radiotimer_schedule(uint16_t offset) {
	//select cmp1 register in the selector so it can be modified
	HWREG(RFCORE_SFR_MTMSEL)= (0x03 && RFCORE_SFR_MTMSEL_MTMSEL_M) << RFCORE_SFR_MTMSEL_MTMSEL_S;
	//set the cmp1 now
	HWREG(RFCORE_SFR_MTM0)=(offset && RFCORE_SFR_MTM0_MTM0_M)<<RFCORE_SFR_MTM0_MTM0_S;
    //set cmp1 - high 8 bits
	HWREG(RFCORE_SFR_MTM1)=(offset>>8 && RFCORE_SFR_MTM1_MTM1_M)<<RFCORE_SFR_MTM1_MTM1_S;
   // enable cmp1 interrupt (this also cancels any pending interrupts)
	HWREG(RFCORE_SFR_MTIRQM)|=RFCORE_SFR_MTIRQM_MACTIMER_COMPARE1M;
}

void radiotimer_cancel() {
	//select cmp1 register in the selector so it can be modified
	HWREG(RFCORE_SFR_MTMSEL)= (0x03 && RFCORE_SFR_MTMSEL_MTMSEL_M) << RFCORE_SFR_MTMSEL_MTMSEL_S;
	//set the cmp1 now to 0
	HWREG(RFCORE_SFR_MTM0)=(0x00 && RFCORE_SFR_MTM0_MTM0_M)<<RFCORE_SFR_MTM0_MTM0_S;
	//set cmp1 - high 8 bits
	HWREG(RFCORE_SFR_MTM1)=(0x00 && RFCORE_SFR_MTM1_MTM1_M)<<RFCORE_SFR_MTM1_MTM1_S;
	// disable cmp1 interrupt
	HWREG(RFCORE_SFR_MTIRQM)&=~RFCORE_SFR_MTIRQM_MACTIMER_COMPARE1M;
}

//===== capture

port_INLINE PORT_TIMER_WIDTH radiotimer_getCapturedTime() {
	uint16_t value=0;
	//select period register in the selector so it can be read
	HWREG(RFCORE_SFR_MTMSEL)= (0x00 && RFCORE_SFR_MTMSEL_MTMSEL_M) << RFCORE_SFR_MTMSEL_MTMSEL_S;
	// compute value by adding m0 and m1 registers
	value = HWREG(RFCORE_SFR_MTM0) + (HWREG(RFCORE_SFR_MTM1)<<8);
	return value;
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================


/*
 * The interrupt flags are given in the RFCORE_SFR_MTIRQF registers. The interrupt flag bits are set only
by hardware and are cleared only by writing to the SFR register.
Each interrupt source can be masked by its corresponding mask bit in the RFCORE_SFR_MTIRQM
register. An interrupt is generated when the corresponding mask bit is set; otherwise, the interrupt is not
generated. The interrupt flag bit is set, however, regardless of the state of the interrupt mask bit.
 *
 */
kick_scheduler_t radiotimer_isr() {
   uint8_t source = RFCORE_SFR_MTIRQF && 0xFF;  // read interrupt source

   //clear flags
   HWREG(RFCORE_SFR_MTIRQF) &= ~0xFF;

   switch (source) {
      case 0x00: // timer overflows
         if (radiotimer_vars.overflow_cb!=NULL) {
               // call the callback
             radiotimer_vars.overflow_cb();
               // kick the OS
             return KICK_SCHEDULER;
          }
      break;
      case 0x01: // compare 1
         if (radiotimer_vars.compare_cb!=NULL) {
            // call the callback
            radiotimer_vars.compare_cb();
            // kick the OS
            return KICK_SCHEDULER;
         }
         break;
      default:
         while(1);                               // this should not happen
   }
   return DO_NOT_KICK_SCHEDULER;
}
