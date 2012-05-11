
/*
 * File:        lptmr.c
 * Purpose:     Provide common low power timer functions
 *
 * Notes:       Always counting up 16 bits counter reset on overflow. 32khz crystal
 *
 */

#include "lptmr.h"
#include "derivative.h"
#include "arm_cm4.h"
#include "openwsn.h"


typedef struct{
	uint16_t total; //total number of tics for that timer
	lptmr_cbt callback;
	bool running;
	bool oneshot;
	bool hasexpired;
	uint16_t lastcompare; //is absolute value of the counter.
	uint16_t nextcompare;
}tmr_info_t;

typedef struct{
	tmr_info_t tmrs[LPTMR_BSP_MAX];
	bool initiated;
	uint16_t current_timeout; 
	bool isRunning;
}lptmr_vars_t;


lptmr_vars_t lptmr_vars;


inline static void reschedule_tmr();
inline static void lptmr_set_compare(PORT_TIMER_WIDTH count);
inline static void lptmr_reset_compare();

void lptmr_isr(void)
{
	uint8_t i=0;
	uint8_t id=0;
	bool isrunning=FALSE;//check is something is running
	uint16_t current_val;
	uint16_t min=0xFFFF;
	uint16_t cur_timeout;
	bool run;

	LPTMR0_CSR |= ( LPTMR_CSR_TEN_MASK | LPTMR_CSR_TIE_MASK | LPTMR_CSR_TCF_MASK  );
	// enable timer
	// enable interrupts
	// clear the flag
	cur_timeout=lptmr_vars.current_timeout;

	//identify the timers that have expired
	for (i=0;i<LPTMR_BSP_MAX;i++){
		if (lptmr_vars.tmrs[i].running==TRUE){
			if (lptmr_vars.tmrs[i].nextcompare<=cur_timeout){
				lptmr_vars.tmrs[i].hasexpired=TRUE;
			}
		}
	}

	//execute all callbacks
	for (i=0;i<LPTMR_BSP_MAX;i++){

		if (lptmr_vars.tmrs[i].running==TRUE){
			isrunning|=TRUE;
			if (lptmr_vars.tmrs[i].hasexpired==TRUE){
				lptmr_vars.tmrs[i].callback();//call the hook

				//has expired should be true, if not this means that in the callback has been set again
				if (lptmr_vars.tmrs[i].hasexpired==TRUE){
					//noone touch it
					if (lptmr_vars.tmrs[i].oneshot==FALSE){
										//is periodic - reschedule next
						lptmr_vars.tmrs[i].lastcompare=lptmr_vars.tmrs[i].nextcompare;//the last timeout is current
						lptmr_vars.tmrs[i].nextcompare+=lptmr_vars.tmrs[i].total; //future timeout is now + period
										
					}else{
						lptmr_vars.tmrs[i].running=FALSE; //set to not running. It can be rescheduled in the callback so set it here to not running to avoid race condition  
					}
				}
				lptmr_vars.tmrs[i].hasexpired=FALSE;
			}
		}else{
			isrunning|=FALSE;
			//if none of them is running we can clear the compare 
		}
	}
	if (isrunning==FALSE){
		lptmr_reset_compare();
		lptmr_vars.isRunning=FALSE;
	}else{
		//reschedule 
		run=FALSE;
		for (i=0;i<LPTMR_BSP_MAX;i++){
			if (lptmr_vars.tmrs[i].running==TRUE){//find the min running timer
				if (min>(uint16_t)(lptmr_vars.tmrs[i].nextcompare-cur_timeout)){//check the least remaining
					min=(uint16_t)(lptmr_vars.tmrs[i].nextcompare - cur_timeout);
					
					id=i;//keep what timer is the next to be scheduled
				}
				run=TRUE;
			}
		}
		if (run==TRUE){
			lptmr_vars.current_timeout=lptmr_vars.tmrs[id].nextcompare;//set next timeout
			lptmr_set_compare(lptmr_vars.current_timeout);//schedule the next absolute timeout that corresponds to the min reminding timer
			//printf("%d\n",lptmr_vars.current_timeout);
		}else{
			lptmr_vars.isRunning=FALSE;
			lptmr_reset_compare();
		}
	}
}



void lptmr_set_isr_callback (lptmr_source_t type,lptmr_cbt cb){
	lptmr_vars.tmrs[type].callback=cb;
}

/*******************************************************************************
 *
 *   PROCEDURE NAME:
 *       lptmr_init -
 *
 *******************************************************************************/

void lptmr_init(uint8_t clock_source)
{ 
	if (lptmr_vars.initiated==TRUE) return; //initialization only one time.
	//clear struct	
	memset(&lptmr_vars,0,sizeof(lptmr_vars_t));	

	SIM_SCGC5 |= SIM_SCGC5_LPTIMER_MASK;//power the timer

	SIM_SCGC6|=SIM_SCGC6_RTC_MASK; //Enable RTC registers
	RTC_CR|=RTC_CR_OSCE_MASK;      //Turn on RTC oscillator
	SIM_SOPT1 &= ~(3<<18);//clear osc32ksel
	SIM_SOPT1 |=SIM_SOPT1_OSC32KSEL(2); //select rtc 32khz

	LPTMR0_PSR |= ( LPTMR_PSR_PRESCALE(0) // 0000 is div 2
			| LPTMR_PSR_PBYP_MASK  // external osc feeds directly to LPT
			| LPTMR_PSR_PCS(clock_source)) ; // use the choice of clock          

	LPTMR0_CSR |=(  LPTMR_CSR_TCF_MASK   // Clear any pending interrupt
			//| LPTMR_CSR_TIE_MASK   // LPT interrupt enabled
			| LPTMR_CSR_TPS(0)     //TMR pin select
			|!LPTMR_CSR_TPP_MASK   //TMR Pin polarity
			|LPTMR_CSR_TFC_MASK   // Timer Free running counter is reset only on overflow ( not whenever TMR counter equals compare)
			|!LPTMR_CSR_TMS_MASK   //LPTMR as Timer
	);
	//register interrupt
	//done hardcoded in the vector table in kinetis_sysinit.c

	enable_irq(LPTMR_IRQ_NUM);

	lptmr_vars.initiated=TRUE;

	LPTMR0_CSR |= LPTMR_CSR_TEN_MASK;   //Turn on LPT and start counting
}

void lptmr_enable(){
	LPTMR0_CSR |=LPTMR_CSR_TIE_MASK|LPTMR_CSR_TEN_MASK;//enable interrupts + timer enable
}



inline static void lptmr_set_compare(PORT_TIMER_WIDTH count){
	LPTMR0_CMR = LPTMR_CMR_COMPARE(count);  //Set compare value
	LPTMR0_CSR |=LPTMR_CSR_TIE_MASK;//enable interrupts
}

void lptmr_set_compare_bsp(PORT_TIMER_WIDTH count){
	//if it is not running reset the timer so it starts to 0 and then 
	if (lptmr_vars.isRunning==FALSE){
		lptmr_reset_counter();
	}

	lptmr_vars.tmrs[LPTMR_BSP_COMPARE].total=count;
	lptmr_vars.tmrs[LPTMR_BSP_COMPARE].running=TRUE;
	lptmr_vars.tmrs[LPTMR_BSP_COMPARE].oneshot=FALSE;
	lptmr_vars.tmrs[LPTMR_BSP_COMPARE].hasexpired=FALSE;

	lptmr_vars.tmrs[LPTMR_BSP_COMPARE].lastcompare=lptmr_get_current_value();//now -- 0 if none is running
	lptmr_vars.tmrs[LPTMR_BSP_COMPARE].nextcompare=lptmr_vars.tmrs[LPTMR_BSP_COMPARE].lastcompare+count;//absolute value 

	if (lptmr_vars.tmrs[LPTMR_BSP_COMPARE].nextcompare < lptmr_vars.current_timeout || lptmr_vars.isRunning==FALSE){
		lptmr_vars.current_timeout=lptmr_vars.tmrs[LPTMR_BSP_COMPARE].nextcompare;
		//set the compare value
		lptmr_vars.isRunning=TRUE;//set to running.
		lptmr_set_compare(lptmr_vars.current_timeout);
	}
}


void lptmr_set_compare_radio(PORT_TIMER_WIDTH count){
	if (lptmr_vars.isRunning==FALSE){
		lptmr_reset_counter();
	}

	lptmr_vars.tmrs[LPTMR_RADIO_COMPARE].total=count;
	lptmr_vars.tmrs[LPTMR_RADIO_COMPARE].running=TRUE;
	lptmr_vars.tmrs[LPTMR_RADIO_COMPARE].oneshot=TRUE;
	lptmr_vars.tmrs[LPTMR_RADIO_COMPARE].hasexpired=FALSE;

	lptmr_vars.tmrs[LPTMR_RADIO_COMPARE].lastcompare=lptmr_get_current_value();//now -- 0 if none is running
	lptmr_vars.tmrs[LPTMR_RADIO_COMPARE].nextcompare=lptmr_vars.tmrs[LPTMR_RADIO_COMPARE].lastcompare+count;//absolute value 

	if (lptmr_vars.tmrs[LPTMR_RADIO_COMPARE].nextcompare < lptmr_vars.current_timeout || lptmr_vars.isRunning==FALSE){
		lptmr_vars.current_timeout=lptmr_vars.tmrs[LPTMR_RADIO_COMPARE].nextcompare;
		//set the compare value
		lptmr_vars.isRunning=TRUE;//set to running.
		lptmr_set_compare(lptmr_vars.current_timeout);
	}
}


void lptmr_set_overflow_radio(PORT_TIMER_WIDTH count){
	if (lptmr_vars.isRunning==FALSE){
		lptmr_reset_counter();
	}

	lptmr_vars.tmrs[LPTMR_RADIO_OVERFLOW].total=count;
	lptmr_vars.tmrs[LPTMR_RADIO_OVERFLOW].running=TRUE;
	lptmr_vars.tmrs[LPTMR_RADIO_OVERFLOW].oneshot=TRUE;
	lptmr_vars.tmrs[LPTMR_RADIO_OVERFLOW].hasexpired=FALSE;

	lptmr_vars.tmrs[LPTMR_RADIO_OVERFLOW].lastcompare=lptmr_get_current_value();//now -- 0 if none is running
	lptmr_vars.tmrs[LPTMR_RADIO_OVERFLOW].nextcompare=lptmr_vars.tmrs[LPTMR_RADIO_COMPARE].lastcompare+count;//absolute value 

	if (lptmr_vars.tmrs[LPTMR_RADIO_OVERFLOW].nextcompare < lptmr_vars.current_timeout || lptmr_vars.isRunning==FALSE){
		lptmr_vars.current_timeout=lptmr_vars.tmrs[LPTMR_RADIO_OVERFLOW].nextcompare;
		//set the compare value
		lptmr_vars.isRunning=TRUE;//set to running.
		lptmr_set_compare(lptmr_vars.current_timeout);
	}
}



inline static void reschedule_tmr(){
	uint8_t i=0;
	uint8_t id=0xFF;
	int32_t min=0xFFFF;
	bool run=FALSE;

	if (lptmr_vars.isRunning==FALSE){
		lptmr_reset_counter();
	}

	for (i=0;i<LPTMR_BSP_MAX;i++){
		if (lptmr_vars.tmrs[i].running==TRUE){//find the min running timer
//			if (min>(lptmr_vars.tmrs[i].reminder)){//check the least remaining
//				min=lptmr_vars.tmrs[i].reminder;
//				id=i;//keep what timer is the next to be scheduled
//			}
			run=TRUE;
		}
	}
	if (run==TRUE){
		lptmr_vars.current_timeout=lptmr_vars.tmrs[id].nextcompare;//set next timeout
		lptmr_set_compare(lptmr_vars.current_timeout);//schedule the next absolute timeout that corresponds to the min reminding timer
		//printf("%d\n",lptmr_vars.current_timeout);
	}else{
		lptmr_vars.isRunning=FALSE;
		lptmr_reset_compare();
	}
}


void lptmr_reset_overflow_radio(){
	lptmr_vars.tmrs[LPTMR_RADIO_OVERFLOW].running=FALSE;
	//reschedule_tmr();
}

void lptmr_reset_compare_radio(){
	lptmr_vars.tmrs[LPTMR_RADIO_COMPARE].running=FALSE;
	//reschedule_tmr();
}

void lptmr_reset_compare_bsp(){
	lptmr_vars.tmrs[LPTMR_BSP_COMPARE].running=FALSE;
	//reschedule_tmr();
}


void lptmr_reset_counter(){
	LPTMR0_CSR &=~LPTMR_CSR_TIE_MASK;
	LPTMR0_CSR &=~LPTMR_CSR_TEN_MASK;//disable interrupts + timer disable
	LPTMR0_CSR |=LPTMR_CSR_TIE_MASK;
	LPTMR0_CSR |=LPTMR_CSR_TEN_MASK;//enable interrupts + timer enable
}


/*
 * Get the current LPTMR Counter Value. 
 *
 * On each read of the LPTMR counter register, software must first write to the 
 * LPTMR counter register with any value. This will synchronize and register the
 * current value of the LPTMR counter register into a temporary register. The 
 * contents of the temporary register are returned on each read of the LPTMR 
 * counter register.
 */

PORT_TIMER_WIDTH lptmr_get_current_value(){
	uint16_t val;
	LPTMR0_CNR = 0x0;
	val= LPTMR0_CNR & LPTMR_CNR_COUNTER_MASK;
	while (val!=LPTMR0_CNR) {
		LPTMR0_CNR = 0x0;
		val= LPTMR0_CNR & LPTMR_CNR_COUNTER_MASK;
	}
	return val;
}

inline static void lptmr_reset_compare(){
	LPTMR0_CMR = LPTMR_CMR_COMPARE(0);  //Set compare value to 0
	LPTMR0_CSR &=~LPTMR_CSR_TIE_MASK;//disable interrupts..
}


void lptmr_disable(){
	uint8_t i;
	bool isrunning=FALSE;
	//clear all timers if nothing is running.
	for (i=0;i<LPTMR_BSP_MAX;i++){
		if (lptmr_vars.tmrs[i].running==TRUE){
			isrunning=TRUE;
		}
	}
	if (isrunning==FALSE){
		memset(&lptmr_vars,0,sizeof(lptmr_vars_t));	
		LPTMR0_CSR &=~LPTMR_CSR_TIE_MASK;
		LPTMR0_CSR &=~LPTMR_CSR_TEN_MASK;//disable interrupts + timer disable
	}
}

