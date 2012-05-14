/**
 \brief A timer module with only a single compare value. Can be used to replace
 the "bsp_timer" and "radiotimer" modules with the help of abstimer.

 \author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, May 2012.
 */

#include "derivative.h"
#include "sctimer.h"

//=========================== defines =========================================

//=========================== variables =======================================
sctimer_cbt callback;//callback to call when the isr expires.
//=========================== prototypes ======================================
extern void lptmr_isr(void);
//=========================== public ==========================================

void sctimer_init() {

	SIM_SCGC5 |= SIM_SCGC5_LPTIMER_MASK;//power the timer

	SIM_SCGC6	|=SIM_SCGC6_RTC_MASK; //Enable RTC registers
	RTC_CR|=RTC_CR_OSCE_MASK; //Turn on RTC oscillator
	SIM_SOPT1 &= ~(3<<18);//clear osc32ksel
	SIM_SOPT1 |=SIM_SOPT1_OSC32KSEL(2); //select rtc 32khz

	LPTMR0_PSR |= ( LPTMR_PSR_PRESCALE(0) // 0000 is div 2
			| LPTMR_PSR_PBYP_MASK // external osc feeds directly to LPT
			| LPTMR_PSR_PCS(clock_source)); // use the choice of clock          

	LPTMR0_CSR |=( LPTMR_CSR_TCF_MASK // C			learany pending interrupt
			//| LPTMR_CSR_TIE_MASK // LPT interrupt enabled
			| LPTMR_CSR_TPS(0) //TMR pin select
			|!LPTMR_CSR_TPP_MASK //TMR Pin polarity
			|LPTMR_CSR_TFC_MASK // Timer Free running counter is reset only on overflow ( not whenever TMR counter equals compare)
			|!LPTMR_CSR_TMS_MASK //LPTMR as Timer
	);
	//register interrupt
	//done hardcoded in the vector table in kinetis_sysinit.c

	enable_irq(LPTMR_IRQ_NUM);

	lptmr_vars.initiated=TRUE;

	LPTMR0_CSR |= LPTMR_CSR_TEN_MASK; //Turn on LPT and start counting
}

void sctimer_schedule(uint16_t val) {
	LPTMR0_CMR = LPTMR_CMR_COMPARE(val); //Set compare value
	LPTMR0_CSR |=LPTMR_CSR_TIE_MASK;//enable interrupts
}

uint16_t sctimer_getValue() {
	uint16_t val;
	LPTMR0_CNR = 0x0;
	val= LPTMR0_CNR & LPTMR_CNR_COUNTER_MASK;
	while (val!=LPTMR0_CNR) {
		LPTMR0_CNR = 0x0;
		val= LPTMR0_CNR & LPTMR_CNR_COUNTER_MASK;
	}
	return val;
}

void sctimer_setCb(sctimer_cbt cb){
	callback=cb;
}


void lptmr_isr(void)
{

	//clear flags.
	LPTMR0_CSR |= ( LPTMR_CSR_TEN_MASK | LPTMR_CSR_TIE_MASK | LPTMR_CSR_TCF_MASK  );
	
	//call the hook
	callback();

	
}
//=========================== private =========================================