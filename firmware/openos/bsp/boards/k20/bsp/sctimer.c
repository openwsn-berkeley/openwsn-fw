/**
 \brief A timer module with only a single compare value. Can be used to replace
 the "bsp_timer" and "radiotimer" modules with the help of abstimer.

 \author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, May 2012.
 */

#include "common.h"
#include "sctimer.h"
#include "arm_cm4.h"

//=========================== defines =========================================
#define LPTMR_USE_IRCLK 0 
#define LPTMR_USE_LPOCLK 1
#define LPTMR_USE_ERCLK32 2
#define LPTMR_USE_OSCERCLK 3
//=========================== variables =======================================

sctimer_cbt callback;//callback to call when the isr expires.
uint16_t unexpected_isr;// see isr function below.
volatile uint16_t lastval=0;
volatile uint16_t currval=0;

//=========================== prototypes ======================================

extern void lptmr_isr(void);
void sctimer_clear_registers();

//=========================== public ==========================================

void sctimer_init() {
	
	SIM_SCGC5 |= SIM_SCGC5_LPTIMER_MASK;//power the timer

	SIM_SCGC6 |= SIM_SCGC6_RTC_MASK; //Enable RTC registers
	RTC_CR |= RTC_CR_OSCE_MASK; //Turn on RTC oscillator
#ifdef TOWER_K20
	SIM_SOPT1 &= ~(3 << 18);//clear osc32ksel
	SIM_SOPT1 |= SIM_SOPT1_OSC32KSEL(2); //select rtc 32khz
#elif OPENMOTE_K20
	SIM_SOPT1 &= ~(1 << SIM_SOPT1_OSC32KSEL_SHIFT);//clear osc32ksel
	SIM_SOPT1 |= SIM_SOPT1_OSC32KSEL_MASK; //select rtc 32khz
#endif	
	sctimer_clear_registers();

	LPTMR0_PSR |= (LPTMR_PSR_PRESCALE(0) // 0000 is div 2
			| LPTMR_PSR_PBYP_MASK // external osc feeds directly to LPT
			| LPTMR_PSR_PCS(LPTMR_USE_ERCLK32)); // use the choice of clock          

	LPTMR0_CSR |= (LPTMR_CSR_TCF_MASK // Clear any pending interrupt
			//| LPTMR_CSR_TIE_MASK // LPT interrupt enabled
			| LPTMR_CSR_TPS(0) //TMR pin select
			| !LPTMR_CSR_TPP_MASK //TMR Pin polarity
			| LPTMR_CSR_TFC_MASK // Timer Free running counter is reset only on overflow ( not whenever TMR counter equals compare)
			| !LPTMR_CSR_TMS_MASK //LPTMR as Timer
	);
	//register interrupt
	//done hardcoded in the vector table in kinetis_sysinit.c

	unexpected_isr=0;
	enable_irq(85/*LPTMR_IRQ_NUM*/);
	LPTMR0_CSR |= LPTMR_CSR_TEN_MASK| LPTMR_CSR_TCF_MASK; //Turn on LPT and start counting
}

/**
 * When the LPTMR is enabled and the CNR equals the value in the CMR and increments, TCF is set and
 the hardware trigger asserts until the next time the CNR increments. If the CMR is 0, the hardware trigger 
 will remain asserted until the LPTMR is disabled. If the LPTMR is enabled, the CMR must be altered only
 when TCF is set. 
 * 
 */

void sctimer_schedule(uint16_t val) {
	volatile uint8_t cntrl,ten,aux;
	cntrl= ((LPTMR0_CSR & LPTMR_CSR_TCF_MASK) >> LPTMR_CSR_TCF_SHIFT);
	ten   = ((LPTMR0_CSR & LPTMR_CSR_TEN_MASK) >> LPTMR_CSR_TEN_SHIFT);
	
	if (val==0xffff){
		aux=10;//debug.
	}
    //timer is disabled or tcf is set
	if ((cntrl==1) || (ten==0)){
		LPTMR0_CMR = LPTMR_CMR_COMPARE(val); //Set compare value
		LPTMR0_CSR |= LPTMR_CSR_TIE_MASK| LPTMR_CSR_TCF_MASK;//enable interrupts
	}else{
		//this case happens when BSP timer sets a timer in user mode (i.e not in isr mode.).
		//according to the manual page 999 the CMR can only be changed if TCF is set or Timer is disabled. 
		//in this case the TCF is not set as we are not
		//inside the lptmr ISR function and the timer is Enabled as is running..
		//timer cannot be stop as it resets the counter.. :(
		//So I do not know the consequences of modifying compare here..
		//LPTMR0_CSR &= ~LPTMR_CSR_TEN_MASK;//disable timer cannot be done because this resets the counter.
		
		LPTMR0_CMR = LPTMR_CMR_COMPARE(val); //Set compare value
		LPTMR0_CSR |= LPTMR_CSR_TEN_MASK|LPTMR_CSR_TIE_MASK| LPTMR_CSR_TCF_MASK; //Turn on again
	}
}

uint16_t sctimer_getValue() {
	volatile uint16_t val, nextval;
	LPTMR0_CNR = 0x0;
	val = LPTMR0_CNR ;

	//manual says to read two times and check that the value is the same. However, 
	//freescale code examples only read this once.
	//	LPTMR0_CNR = 0x0;
	//	while ((nextval=(LPTMR0_CNR ))!=val) {
	//		LPTMR0_CNR = 0x0;
	//		val= LPTMR0_CNR ;
	//		LPTMR0_CNR = 0x0;
	//	}
	
	return val;
}

void sctimer_setCb(sctimer_cbt cb) {
	callback = cb;
}

void lptmr_isr(void) {
	 debugpins_isr_set();
	 callback();
	 debugpins_isr_clr();
}


void sctimer_clearISR(){
	volatile uint8_t csr;
	volatile uint8_t cntrl;
	//clear the isr flag on the csr register
	LPTMR0_CSR |= ( /*LPTMR_CSR_TEN_MASK |*/LPTMR_CSR_TIE_MASK| LPTMR_CSR_TCF_MASK);
	
	//read tcf bit in csr to check that it has turned to 0
	csr=LPTMR0_CSR;
	cntrl= ((LPTMR0_CSR & LPTMR_CSR_TCF_MASK) >> LPTMR_CSR_TCF_SHIFT);
		//timer is disabled or tcf is set
	if (cntrl==1){
		//this should never happen 
		while(1);
	}
	
}



void sctimer_reset(){
	//clear the isr flag on the csr register
	LPTMR0_CSR /=  ~LPTMR_CSR_TEN_MASK; //disable the timer, and enable again, this resets the counter
	sctimer_init();
}
//=========================== private =========================================

/*
 * Zero out all registers.
 *
 */
void sctimer_clear_registers() {
	LPTMR0_CSR = 0x00;
	LPTMR0_PSR = 0x00;
	LPTMR0_CMR = 0x00;
}


void sctimer_stop() {
 //do nothing by now.
}
