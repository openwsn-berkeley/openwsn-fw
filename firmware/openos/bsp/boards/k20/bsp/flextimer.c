/**
 \brief A timer module with. Not a low power timer.

 \author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, March 2013.
 */

#include "common.h"
#include "flextimer.h"
#include "arm_cm4.h"

//=========================== defines =========================================

//=========================== variables =======================================

flextimer_cbt ft_cb;//callback to call when the isr expires.

//=========================== prototypes ======================================

extern void ftm0_isr(void);

//=========================== public ==========================================

void flextimer_init() {
	//power the timer
	SIM_SCGC6 |= SIM_SCGC6_FTM0_MASK; 
	/* Dummy read of the FTM0_SC register to clear the interrupt flag */
	(void) (FTM0_SC == 0U); 
	
	FTM0_SC = 0; //make sure it is off
	/* Dummy read of the FTM0_C0SC register to clear the interrupt flag */
	(void) (FTM0_C0SC == 0U); 
	FTM0_C0SC = (uint32_t) 0x00UL; //set to 0 

	/* FTM0_SC: TOF=0,CPWMS=0 */
	FTM0_SC &= (uint32_t) ~FTM_SC_TOF_MASK;
	FTM0_SC &= (uint32_t) ~FTM_SC_CPWMS_MASK;

	FTM0_MODE |= FTM_MODE_WPDIS_MASK; /* Disable write protection */

	/* FTM0_MODE: FTMEN=0 */
	FTM0_MODE &= (uint32_t) ~FTM_MODE_FTMEN_MASK;

	/* FTM0_C0SC: ??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,CHF=0,CHIE=0,MSB=0,MSA=1,ELSB=0,ELSA=0,??=0,DMA=0 */
	FTM0_C0SC |= FTM_CnSC_MSA_MASK;

	/* FTM0_C0V: ??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,VAL=0 */
	FTM0_C0V = (uint32_t) 0x00UL;
	/* FTM0_MOD: MOD=0xFFFF */
	FTM0_MOD |= (uint32_t) 0xFFFFUL;

	/* FTM0_CNTIN: INIT=0 */
	FTM0_CNTIN &= (uint32_t) ~0xFFFFUL;

	/* FTM0_CNT: COUNT=0 */
	FTM0_CNT &= (uint32_t) ~0xFFFFUL;
	/* FTM0_MODE: ??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,FAULTIE=0,FAULTM=0,PWMSYNC=0,INIT=0,FTMEN=0 */
	FTM0_MODE &= (uint32_t) ~0xFFFFFFEBUL;

	/* FTM0_SC: TOF=0,TOIE=0,CPWMS=0,CLKS=1,PS=7 */
	FTM0_SC &= ~FTM_SC_TOF_MASK;
	FTM0_SC &= ~FTM_SC_TOIE_MASK;
	FTM0_SC &= ~FTM_SC_CPWMS_MASK;
    //use internal clock
	FTM0_SC |= FTM_SC_CLKS(1);
    //divide internal clock by 2^7
	FTM0_SC |= FTM_SC_PS(7);
	//enable IRQ	
	enable_irq(62/*FTM0_IRQ_NUM*/);
}

void flextimer_schedule(uint16_t val) {
	FTM0_C0V = FTM_CnV_VAL(val);//set the val of the compare.
	FTM0_C0SC |= FTM_CnSC_CHIE_MASK;//enable ch interrupt
}

uint16_t flextimer_getValue() {
	return FTM0_CNT;
}

void flextimer_setCb(flextimer_cbt cb) {
	ft_cb = cb;
}

void ftm0_isr(void) {
	//check if the Timer Offset raised.. should not happen.
	if (FTM0_SC & FTM_SC_TOF_MASK)
		FTM0_SC &= ~FTM_SC_TOF_MASK;//clear it

	//Clear the overflow mask if set	
	if (FTM0_STATUS & FTM_STATUS_CH0F_MASK) {
		//the event occurred
		//read csc as described in p.865
		(void) (FTM0_C0SC == 0U); /* Dummy read of the FTM0_C0SC register to clear the interrupt flag */
		FTM0_C0SC &= ~FTM_CnSC_CHF_MASK;//set Channel flag to zero.
		FTM0_C0SC |= FTM_CnSC_CHIE_MASK;//enable ch interrupt
		//call the callback
		ft_cb();
	}
}

void flextimer_cancel() {
	FTM0_C0SC &= ~FTM_CnSC_CHIE_MASK;//disable ch interrupt
	FTM0_C0V = FTM_CnV_VAL(0);//set the val of the compare   
}
//=========================== private =========================================


void flextimer_reset() {
	FTM0_C0SC &= ~FTM_CnSC_CHIE_MASK;//disable ch interrupt
	FTM0_C0V = FTM_CnV_VAL(0);//set the val of the compare   
	// reset timer
	FTM0_CNT = 0; //initializes the counter load initial value
}
