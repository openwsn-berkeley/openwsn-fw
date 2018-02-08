/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Date:   July 2013
 * Description: CC2538-specific definition of the "radiotimer" bsp module.
 */

#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "radiotimer.h"
#include "debugpins.h"
#include "board.h"
#include "em_cmu.h"
#include "em_rtc.h"

//=========================== variables =======================================

typedef struct {
   radiotimer_compare_cbt    overflow_cb;
   radiotimer_compare_cbt    compare_cb;
   PORT_RADIOTIMER_WIDTH     currentSlotPeriod;
} radiotimer_vars_t;

radiotimer_vars_t radiotimer_vars;

//=========================== prototypes ======================================

void radiotimer_isr_private(void);
port_INLINE uint16_t get_real_counter(void);

//=========================== public ==========================================

//===== admin

void radiotimer_init(void) {
   // clear local variables
   memset(&radiotimer_vars,0,sizeof(radiotimer_vars_t));
   
   CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
   CMU_ClockEnable(cmuClock_CORELE, true);
   CMU_ClockEnable(cmuClock_RTC, true);
   
   RTC_Init_TypeDef rtcInit = RTC_INIT_DEFAULT;
  
   rtcInit.enable   = false;      /* Enable RTC after init has run */
   rtcInit.comp0Top = true;      /* Clear counter on compare match */
   rtcInit.debugRun = false;     /* Counter shall keep running during debug halt. */

   /* Initialize the RTC */
   RTC_Init(&rtcInit);
   /* Enable the RTC Interrupt */
   NVIC_EnableIRQ(RTC_IRQn);
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
   RTC_CompareSet(0, period);
   RTC_IntEnable(RTC_IEN_COMP0);
   radiotimer_vars.currentSlotPeriod = period;
   RTC_Enable(true);
   
}

//===== direct access

PORT_RADIOTIMER_WIDTH radiotimer_getValue(void) {
	 PORT_RADIOTIMER_WIDTH value=0;
	 //select period register in the selector so it can be read
         value = RTC->CNT;
	 return value;
}

void radiotimer_setPeriod(PORT_RADIOTIMER_WIDTH period) {
	//select period register in the selector so it can be modified -- use OVF  so we have 24bit register
        RTC_IntEnable(RTC_IEN_COMP0);
        RTC_CompareSet(0, period);
}

PORT_RADIOTIMER_WIDTH radiotimer_getPeriod(void) {
	PORT_RADIOTIMER_WIDTH period=0;
         

    return period;
}

//===== compare

void radiotimer_schedule(PORT_RADIOTIMER_WIDTH offset) {
	//select ovf cmp1 register in the selector so it can be modified
        RTC_IntDisable(RTC_IEN_COMP1);
        RTC_IntDisable(RTC_IEN_COMP0);
        RTC_CompareSet(1, offset);
        RTC_IntClear(RTC_IFC_COMP1 + RTC_IFC_COMP0);
        RTC_IntEnable(RTC_IEN_COMP1);
        RTC_IntEnable(RTC_IEN_COMP0);
        
}

void radiotimer_cancel(void) {
        RTC_IntDisable(RTC_IEN_COMP0);
        RTC_IntDisable(RTC_IEN_COMP1);
        //RTC_CompareSet(0, radiotimer_vars.currentSlotPeriod);
        RTC_IntClear(RTC_IFC_COMP0 + RTC_IFC_COMP1);
        RTC_IntEnable(RTC_IEN_COMP0);
        RTC_IntEnable(RTC_IEN_COMP1);
        

}

//===== capture

port_INLINE PORT_RADIOTIMER_WIDTH radiotimer_getCapturedTime(void) {
	 PORT_RADIOTIMER_WIDTH value=0;
         value = RTC->CNT;
     return (PORT_RADIOTIMER_WIDTH)value;
}

//=========================== private =========================================

port_INLINE uint16_t get_real_counter(void){
	uint16_t value=0;
        value = RTC->CNT; 
    return value;
}

//=========================== interrupt handlers ==============================

void radiotimer_isr_private(void) {
	debugpins_isr_set();
	radiotimer_isr();
	debugpins_isr_clr();
        
}


kick_scheduler_t radiotimer_isr(void) {
  /* Clear interrupt source */
    // RTC_IntClear(RTC_IFC_COMP0);
    // Compare
    if ((RTC->IF & 4) == 4){
      RTC_IntClear(RTC_IFC_COMP1);
    if (radiotimer_vars.compare_cb!=NULL) {
            
            // call the callback
            radiotimer_vars.compare_cb();
            // kick the OS
            return KICK_SCHEDULER;
    }
  }
  // Overflow
  if ((RTC->IF & 2) == 2){
      RTC_IntClear(RTC_IFC_COMP0);
  if (radiotimer_vars.overflow_cb!=NULL) {
                            
            //Set the RTC time counter to 0
            //RTC->CNT = 0x000000;
            // call the callback
            radiotimer_vars.overflow_cb();
            // kick the OS
            return KICK_SCHEDULER;
         }
  }
    return DO_NOT_KICK_SCHEDULER;
}

void RTC_IRQHandler(void)
{

  radiotimer_isr_private();
}
