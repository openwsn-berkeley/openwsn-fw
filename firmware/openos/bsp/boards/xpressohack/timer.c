/**
\brief LPC17XX-specific definition of the "timer" bsp module.

TIMER 0 and 1 are used by BSP timers
TIMER 2 is virtualized to offer 100 timers.
TIMER 3 is radiotimer
\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, February 2012.
 */

#include "stdint.h"
#include "timer.h"
#include "LPC17xx.h"
#include "leds.h"
#include "board.h"

//=========================== defines =========================================

extern void TIMER0_IRQHandler(void);
extern void TIMER1_IRQHandler(void);
extern void TIMER2_IRQHandler(void);
extern void TIMER3_IRQHandler(void);

//=========================== variables =======================================

low_timer_hook timer_compare_isr_hook_0;
low_timer_hook timer_compare_isr_hook_1;
low_timer_hook timer_compare_isr_hook_2;
low_timer_hook timer_compare_isr_hook_3;
low_timer_hook timer_capture_isr_hook_0;
low_timer_hook timer_capture_isr_hook_1;
low_timer_hook timer_capture_isr_hook_2;
low_timer_hook timer_capture_isr_hook_3;

//=========================== prototypes ======================================

static void private_prepareTimer0(void);
static void private_prepareTimer1(void);
static void private_prepareTimer2(void);
static void private_prepareTimer3(void);//radio timer.

//=========================== public ==========================================

void timer_set_isr_compare_hook(uint8_t timer_num, low_timer_hook cbt) {
	switch (timer_num){
	case TIMER_NUM0:
		timer_compare_isr_hook_0 = cbt;
		break;
	case TIMER_NUM1:
		timer_compare_isr_hook_1 = cbt;
		break;
	case TIMER_NUM2:
		timer_compare_isr_hook_2 = cbt;
		break;
	case TIMER_NUM3:
		timer_compare_isr_hook_3 = cbt;
		break;
	default:
		while(1); //this should never happen
		break;
	}
	return;
}

void timer_set_isr_capture_hook(uint8_t timer_num, low_timer_hook cbt) {
	switch (timer_num){
	case TIMER_NUM0:
		timer_capture_isr_hook_0 = cbt;
		break;
	case TIMER_NUM1:
		timer_capture_isr_hook_1 = cbt;
		break;
	case TIMER_NUM2:
		timer_capture_isr_hook_2 = cbt;
		break;
	case TIMER_NUM3:
		timer_capture_isr_hook_3 = cbt;
		break;
	default:
		while(1); //this should never happen
		break;
	}
	return;
}

/**
\brief Initialize a timer.
 */
void timer_init(uint8_t timer_num) {
	switch (timer_num){
	case TIMER_NUM0:
		private_prepareTimer0();
		break;
	case TIMER_NUM1:
		private_prepareTimer1();
		break;
	case TIMER_NUM2:
		private_prepareTimer2();
		break;
	case TIMER_NUM3:
		private_prepareTimer3();//radio timer
		break;
	default:
		while(1); //this should never happen
		break;
	}
	return;
}

/**
\brief Starts a timer.

\pre Call timer_init before calling this function.
 */
void timer_enable(uint8_t timer_num) {
	if        (timer_num == TIMER_NUM0) {
		LPC_TIM0->TCR     = 1;
	} else if (timer_num == TIMER_NUM1) {
		LPC_TIM1->TCR     = 1;
	} else if (timer_num == TIMER_NUM2) {
		LPC_TIM2->TCR     = 1;
	} else if (timer_num == TIMER_NUM3) {
		LPC_TIM3->TCR     = 1;
	}else {
		while(1); //this should never happen
	}
	return;
}



uint32_t timer_get_current_value(uint8_t timer_num) {
	uint32_t current=0;
	switch (timer_num){
	case TIMER_NUM0:
		current   =  LPC_TIM0->TC;
		break;
	case TIMER_NUM1:
		current   =  LPC_TIM1->TC;
		break;
	case TIMER_NUM2:
		current   =  LPC_TIM2->TC;
		break;
	case TIMER_NUM3:
		current   =  LPC_TIM3->TC;
		break;

	default:
		while(1); //this should never happen
		break;
	}
	return current;
}

/**
\brief Set the compare value in the timer.

Every timer has 3 compare registers. so choose one.
 */
void timer_set_compare(uint8_t  timer_num,
		uint8_t  compareReg,
		uint32_t delayInTicks) {

	switch (timer_num) {

	case TIMER_NUM0:
		if (compareReg==TIMER_COMPARE_REG0) {
			// interrupt when MR0 matches the value in the TC
			LPC_TIM0->MCR   |=  1<<0;
			// set Match Register 0 value
			LPC_TIM0->MR0    =  delayInTicks;
		} else if (compareReg==TIMER_COMPARE_REG1) {
			//interrupt when MR1 matches the value in the TC
			LPC_TIM0->MCR   |=  1<<3;
			// set Match Register 1 value
			LPC_TIM0->MR1    =  delayInTicks;
		} else if (compareReg==TIMER_COMPARE_REG2) {
			//interrupt when MR2 matches the value in the TC
			LPC_TIM0->MCR   |=  1<<6;
			// set Match Register 2 value
			LPC_TIM0->MR2    =  delayInTicks;
		} else{
			while(1); //this should never happen
		}
		break;

	case TIMER_NUM1:
		if (compareReg==TIMER_COMPARE_REG0) {
			//interrupt when MR0 matches the value in the TC
			LPC_TIM1->MCR   |=  1<<0;
			// set Match Register 0 value
			LPC_TIM1->MR0    =  delayInTicks;
		} else if (compareReg==TIMER_COMPARE_REG1) {
			//interrupt when MR1 matches the value in the TC
			LPC_TIM1->MCR   |=  1<<3;
			// set Match Register 1 value
			LPC_TIM1->MR1    =  delayInTicks;
		} else if (compareReg==TIMER_COMPARE_REG2) {
			//interrupt when MR2 matches the value in the TC
			LPC_TIM1->MCR   |=  1<<6;
			// set Match Register 2 value
			LPC_TIM1->MR2    =  delayInTicks;
		} else {
			while(1); //this should never happen
		}
		break;
	case TIMER_NUM2:
		if (compareReg==TIMER_COMPARE_REG0) {
			//interrupt when MR0 matches the value in the TC
			LPC_TIM2->MCR   |=  1<<0;
			// set Match Register 0 value
			LPC_TIM2->MR0    =  delayInTicks;
		} else if (compareReg==TIMER_COMPARE_REG1) {
			//interrupt when MR1 matches the value in the TC
			LPC_TIM2->MCR   |=  1<<3;
			// set Match Register 1 value
			LPC_TIM2->MR1    =  delayInTicks;
		} else if (compareReg==TIMER_COMPARE_REG2) {
			//interrupt when MR2 matches the value in the TC
			LPC_TIM2->MCR   |=  1<<6;
			// set Match Register 2 value
			LPC_TIM2->MR2    =  delayInTicks;
		} else {
			while(1); //this should never happen
		}
		break;
	case TIMER_NUM3:
		if (compareReg==TIMER_COMPARE_REG0) {
			//interrupt when MR0 matches the value in the TC
			LPC_TIM3->MCR   |=  1<<0;
			// set Match Register 0 value
			LPC_TIM3->MR0    =  delayInTicks;

		} else if (compareReg==TIMER_COMPARE_REG1) {
			//interrupt when MR1 matches the value in the TC
			LPC_TIM3->MCR   |=  1<<3;
			// set Match Register 1 value
			LPC_TIM3->MR1    =  delayInTicks;
		} else if (compareReg==TIMER_COMPARE_REG2) {
			//interrupt when MR2 matches the value in the TC
			LPC_TIM3->MCR   |=  1<<6;
			// set Match Register 2 value
			LPC_TIM3->MR2    =  delayInTicks;
		} else {
			while(1); //this should never happen
		}
		break;

	default:
		while(1); //this should never happen
		break;
	}
	return;
}

void timer_reset_compare(uint8_t timer_num, uint8_t compareReg) {

	switch (timer_num) {

	case TIMER_NUM0:
		if        (compareReg==TIMER_COMPARE_REG0) {
			// don't interrupt when MR0 matches the value in the TC
			LPC_TIM0->MCR   &= ~(1<<0);
			// reset Match Register 0 value
			LPC_TIM0->MR0    =  0;
		} else if (compareReg==TIMER_COMPARE_REG1) {
			// don't interrupt when MR1 matches the value in the TC
			LPC_TIM0->MCR   &= ~(1<<3);
			// reset Match Register 1 value
			LPC_TIM0->MR1    =  0;
		} else if (compareReg==TIMER_COMPARE_REG2) {
			// don't interrupt when MR2 matches the value in the TC
			LPC_TIM0->MCR   &= ~(1<<6);
			// reset Match Register 2 value
			LPC_TIM0->MR2    =  0;
		} else {
			while(1); //this should never happen
		}
		break;

	case TIMER_NUM1:
		if (compareReg==TIMER_COMPARE_REG0) {
			// don't interrupt when MR0 matches the value in the TC
			LPC_TIM1->MCR   &= ~(1<<0);
			// reset Match Register 0 value
			LPC_TIM1->MR0    =  0;
		}else if (compareReg==TIMER_COMPARE_REG1) {
			// don't interrupt when MR1 matches the value in the TC
			LPC_TIM1->MCR   &= ~(1<<3);
			// reset Match Register 1 value
			LPC_TIM1->MR1    =  0;
		}else if (compareReg==TIMER_COMPARE_REG2) {
			// don't interrupt when MR2 matches the value in the TC
			LPC_TIM1->MCR   &= ~(1<<6);
			// reset Match Register 2 value
			LPC_TIM1->MR2    =  0;
		} else {
			while(1); //this should never happen
		}
		break;
	case TIMER_NUM2:
		if (compareReg==TIMER_COMPARE_REG0) {
			// don't interrupt when MR0 matches the value in the TC
			LPC_TIM2->MCR   &= ~(1<<0);
			// reset Match Register 0 value
			LPC_TIM2->MR0    =  0;
		}else if (compareReg==TIMER_COMPARE_REG1) {
			// don't interrupt when MR1 matches the value in the TC
			LPC_TIM2->MCR   &= ~(1<<3);
			// reset Match Register 1 value
			LPC_TIM2->MR1    =  0;
		}else if (compareReg==TIMER_COMPARE_REG2) {
			// don't interrupt when MR2 matches the value in the TC
			LPC_TIM2->MCR   &= ~(1<<6);
			// reset Match Register 2 value
			LPC_TIM2->MR2    =  0;
		} else {
			while(1); //this should never happen
		}
		break;

	case TIMER_NUM3:
		if (compareReg==TIMER_COMPARE_REG0) {
			// don't interrupt when MR0 matches the value in the TC
			LPC_TIM3->MCR   &= ~(1<<0);
			// reset Match Register 0 value
			LPC_TIM3->MR0    =  0;
		}else if (compareReg==TIMER_COMPARE_REG1) {
			// don't interrupt when MR1 matches the value in the TC
			LPC_TIM3->MCR   &= ~(1<<3);
			// reset Match Register 1 value
			LPC_TIM3->MR1    =  0;
		}else if (compareReg==TIMER_COMPARE_REG2) {
			// don't interrupt when MR2 matches the value in the TC
			LPC_TIM3->MCR   &= ~(1<<6);
			// reset Match Register 2 value
			LPC_TIM3->MR2    =  0;
		} else {
			while(1); //this should never happen
		}
		break;

	default:
		while(1); //this should never happen
		break;
	}
	return;
}

/**
\brief Configure the capture register and set interrupts.

Every timer has 2 capture registers. so choose one.
 */
void timer_set_capture(uint8_t timer_num,uint8_t captureReg) {
	switch (timer_num) {

	case TIMER_NUM0:
		if        (captureReg==TIMER_CAPTURE_REG0) {

#if TIMER_MATCH // if external match pins (DMA)
			//set to 0 4 lowest bits.. ExternalMatchRegister. p.498
			LPC_TIM0->EMR   &= ~(0xFF<<4);
			//set bit 4-5 to 11 -- mean toggle external match pin
			LPC_TIM0->EMR   |=  ((0x3<<4);
#else
			//configure capture register when something happens.
			//bit 0 = CAP0RE=1; capture on raising edge; value of TC will be copied to CAP
			//bit 2 = CAP0I=1; generate interrupt on capture
			LPC_TIM0->CCR    =  (0x1<<0)|(0x1<<2);
#endif
		} else if (captureReg==TIMER_CAPTURE_REG1) {

#if TIMER_MATCH // if external match pins (DMA)
			//set to 0 4 lowest bits.. ExternalMatchRegister. p.498
			LPC_TIM0->EMR   &= ~(0xFF<<4);
			//set bit 6-7 to 11 -- mean toggle external match pin
			LPC_TIM0->EMR   |=  ((0x3<<6);
#else
			//configure capture register when something happens.
			//bit 3 = CAP1RE=1; capture on raising edge; value of TC will be copied to CAP
			//bit 5 = CAP1I=1; generate interrupt on capture
			LPC_TIM0->CCR    =  (0x1<<3)|(0x1<<5);
#endif
		} else {
			while(1); //this should never happen
		}
		break;

	case TIMER_NUM1:
		if (captureReg==TIMER_CAPTURE_REG0) {
			//configure capture register when something happens.
			//bit 0 = CAP0RE=1 -- capture on raising edge -- value of TC will be copied to CAP
			//bit 2 = CAP0I=1 -- generate interrupt on capture
			LPC_TIM1->CCR   |=  (0x1<<0)|(0x1<<2);
		} else if (captureReg==TIMER_CAPTURE_REG1){
			//bit 3 = CAP1RE=1 -- capture on raising edge -- value of TC will be copied to CAP
			//bit 5 = CAP1I=1 -- generate interrupt on capture
			LPC_TIM1->CCR   |=  (0x1<<3)|(0x1<<5);
		} else {
			while(1); //this should never happen
		}
		break;
	case TIMER_NUM2:
		if (captureReg==TIMER_CAPTURE_REG0) {
			//configure capture register when something happens.
			//bit 0 = CAP0RE=1 -- capture on raising edge -- value of TC will be copied to CAP
			//bit 2 = CAP0I=1 -- generate interrupt on capture
			LPC_TIM2->CCR   |=  (0x1<<0)|(0x1<<2);
		} else if (captureReg==TIMER_CAPTURE_REG1){
			//bit 3 = CAP1RE=1 -- capture on raising edge -- value of TC will be copied to CAP
			//bit 5 = CAP1I=1 -- generate interrupt on capture
			LPC_TIM2->CCR   |=  (0x1<<3)|(0x1<<5);
		} else {
			while(1); //this should never happen
		}
		break;

	case TIMER_NUM3:
		if (captureReg==TIMER_CAPTURE_REG0) {
			//configure capture register when something happens.
			//bit 0 = CAP0RE=1 -- capture on raising edge -- value of TC will be copied to CAP
			//bit 2 = CAP0I=1 -- generate interrupt on capture
			//LPC_TIM3->CCR   |=  (0x1<<0)|(0x1<<2);
			LPC_TIM3->CCR   |=  (0x1<<0);//capture on rising, not interrupt.
		} else if (captureReg==TIMER_CAPTURE_REG1){
			//bit 3 = CAP1RE=1 -- capture on raising edge -- value of TC will be copied to CAP
			//bit 5 = CAP1I=1 -- generate interrupt on capture
			LPC_TIM3->CCR   |=  (0x1<<3)|(0x1<<5);
		} else {
			while(1); //this should never happen
		}
		break;


	default:
		while(1); //this should never happen
		break;
	}
	return;
}

uint32_t timer_get_capture_value(uint8_t timer_num,uint8_t captureReg){

	uint32_t cap_time=0;

	switch (timer_num) {

	case TIMER_NUM0:
		if (captureReg==TIMER_CAPTURE_REG0) {
			cap_time=LPC_TIM0->CR0;
		} else if (captureReg==TIMER_CAPTURE_REG1) {
			cap_time=LPC_TIM0->CR1;
		} else {
			while(1); //this should never happen
		}
		break;
	case TIMER_NUM1:
		if (captureReg==TIMER_CAPTURE_REG0) {
			cap_time=LPC_TIM1->CR0;
		} else if (captureReg==TIMER_CAPTURE_REG1) {
			cap_time=LPC_TIM1->CR1;
		} else {
			while(1); //this should never happen
		}
		break;
	case TIMER_NUM2:
		if (captureReg==TIMER_CAPTURE_REG0) {
			cap_time=LPC_TIM2->CR0;
		} else if (captureReg==TIMER_CAPTURE_REG1) {
			cap_time=LPC_TIM2->CR1;
		} else {
			while(1); //this should never happen
		}
		break;
	case TIMER_NUM3:
		if (captureReg==TIMER_CAPTURE_REG0) {
			cap_time=LPC_TIM3->CR0;
		} else if (captureReg==TIMER_CAPTURE_REG1) {
			cap_time=LPC_TIM3->CR1;
		} else {
			while(1); //this should never happen
		}
		break;
	}

}
	/**
\brief Reset a timer.
	 */
	void timer_reset(uint8_t timer_num) {
		uint32_t regVal;

		if        (timer_num==TIMER_NUM0) {
			regVal            = LPC_TIM0->TCR;
			regVal           |= 0x02;
			LPC_TIM0->TCR     = regVal;
		} else if (timer_num==TIMER_NUM1) {
			regVal            = LPC_TIM1->TCR;
			regVal           |= 0x02;
			LPC_TIM1->TCR     = regVal;
		} else if (timer_num==TIMER_NUM2) {
			regVal            = LPC_TIM2->TCR;
			regVal           |= 0x02;
			LPC_TIM2->TCR     = regVal;
		}else if (timer_num==TIMER_NUM3) {
			regVal            = LPC_TIM3->TCR;
			regVal           |= 0x02;
			LPC_TIM3->TCR     = regVal;
		}else {
			while(1); //this should never happen
		}

		return;
	}

	/**
\brief Disable a timer.
	 */
	void timer_disable( uint8_t timer_num ) {
		if        (timer_num==TIMER_NUM0) {
			LPC_TIM0->TCR = 0;
		} else if (timer_num==TIMER_NUM1) {
			LPC_TIM1->TCR = 0;
		}else if (timer_num==TIMER_NUM2) {
			LPC_TIM2->TCR = 0;
		}else if (timer_num==TIMER_NUM3) {
			LPC_TIM3->TCR = 0;
		}
		else {
			while(1); //this should never happen
		}
		return;
	}

	/**
\brief irq handler for timer 0.

"implements" weak function in cr_startup_lpc17.c
	 */
	void TIMER0_IRQHandler (void) {
		if ( LPC_TIM0->IR & (0x1<<0)) {
			// clear interrupt flag
			LPC_TIM0->IR = 0x1<<0;
			// call the callback
			timer_compare_isr_hook_0(TIMER_COMPARE_REG0);
		}
		if ( LPC_TIM0->IR & (0x1<<1) ) {
			// clear interrupt flag
			LPC_TIM0->IR = 0x1<<1;
			// call the callback
			timer_compare_isr_hook_0(TIMER_COMPARE_REG1);
		}
		if ( LPC_TIM0->IR & (0x1<<2) ) {
			// clear interrupt flag
			LPC_TIM0->IR = 0x1<<2;
			// call the callback
			timer_compare_isr_hook_0(TIMER_COMPARE_REG2);
		}
		if ( LPC_TIM0->IR & (0x1<<4) ) {
			// clear interrupt flag
			LPC_TIM0->IR = 0x1<<4;
			// call the callback
			timer_capture_isr_hook_0(TIMER_CAPTURE_REG0);
		}
		if ( LPC_TIM0->IR & (0x1<<5) ) {
			// clear interrupt flag
			LPC_TIM0->IR = 0x1<<5;
			// call the callback
			timer_capture_isr_hook_0(TIMER_CAPTURE_REG1);
		}
		return;
	}

	/**
\brief irq handler for timer 1.

"implements" weak function in cr_startup_lpc17.c
	 */
	void TIMER1_IRQHandler (void) {
		if ( LPC_TIM1->IR & (0x1<<0) ) {
			// clear interrupt flag
			LPC_TIM1->IR = 0x1<<0;
			// call the callback
			timer_compare_isr_hook_1(TIMER_COMPARE_REG0);
		}
		if ( LPC_TIM1->IR & (0x1<<1) ) {
			// clear interrupt flag
			LPC_TIM1->IR = 0x1<<1;
			// call the callback
			timer_compare_isr_hook_1(TIMER_COMPARE_REG1);
		}
		if ( LPC_TIM1->IR & (0x1<<3) ) {
			// clear interrupt flag
			LPC_TIM1->IR = 0x1<<2;
			// call the callback
			timer_compare_isr_hook_1(TIMER_COMPARE_REG2);//call
		}
		if ( LPC_TIM1->IR & (0x1<<4) ) {
			// clear interrupt flag
			LPC_TIM1->IR = 0x1<<4;
			// call the callback
			timer_capture_isr_hook_1(TIMER_CAPTURE_REG0);
		}
		if ( LPC_TIM1->IR & (0x1<<5) ) {
			// clear interrupt flag
			LPC_TIM1->IR = 0x1<<5;
			// call the callback
			timer_capture_isr_hook_1(TIMER_CAPTURE_REG1);
		}
		return;
	}



	/**
\brief irq handler for timer 2.

"implements" weak function in cr_startup_lpc17.c
	 */
	void TIMER2_IRQHandler (void) {

		if ( LPC_TIM2->IR & (0x1<<0)) {
			// clear interrupt flag
			LPC_TIM2->IR = 0x1<<0;

			// call the callback
			timer_compare_isr_hook_2(TIMER_COMPARE_REG0);
		}
		if ( LPC_TIM2->IR & (0x1<<1) ) {
			// clear interrupt flag
			LPC_TIM2->IR = 0x1<<1;
			// call the callback
			timer_compare_isr_hook_2(TIMER_COMPARE_REG1);
		}
		if ( LPC_TIM2->IR & (0x1<<2) ) {
			// clear interrupt flag
			LPC_TIM2->IR = 0x1<<2;
			// call the callback
			timer_compare_isr_hook_2(TIMER_COMPARE_REG2);
		}
		if ( LPC_TIM2->IR & (0x1<<4) ) {
			// clear interrupt flag
			LPC_TIM2->IR = 0x1<<4;
			// call the callback
			timer_capture_isr_hook_2(TIMER_CAPTURE_REG0);
		}
		if ( LPC_TIM2->IR & (0x1<<5) ) {
			// clear interrupt flag
			LPC_TIM2->IR = 0x1<<5;
			// call the callback
			timer_capture_isr_hook_2(TIMER_CAPTURE_REG1);
		}
		return;
	}



	/**
\brief irq handler for timer 2.

"implements" weak function in cr_startup_lpc17.c
	 */
void TIMER3_IRQHandler (void) {
		if ( LPC_TIM3->IR & (0x1<<0)) {
			// clear interrupt flag
			CAPTURE_TIME();
			LPC_TIM3->IR = 0x1<<0;
			// call the callback
			timer_compare_isr_hook_3(TIMER_COMPARE_REG0);
		}
		if ( LPC_TIM3->IR & (0x1<<1) ) {
			// clear interrupt flag
			CAPTURE_TIME();

			LPC_TIM3->IR = 0x1<<1;
			// call the callback
			timer_compare_isr_hook_3(TIMER_COMPARE_REG1);
		}
		if ( LPC_TIM3->IR & (0x1<<2) ) {
			// clear interrupt flag
			LPC_TIM3->IR = 0x1<<2;
			// call the callback
			timer_compare_isr_hook_3(TIMER_COMPARE_REG2);
		}
		if ( LPC_TIM3->IR & (0x1<<4) ) {
			// clear interrupt flag
			LPC_TIM3->IR = 0x1<<4;
			// call the callback
			timer_capture_isr_hook_3(TIMER_CAPTURE_REG0);
		}
		if ( LPC_TIM3->IR & (0x1<<5) ) {
			// clear interrupt flag
			LPC_TIM2->IR = 0x1<<5;
			// call the callback
			timer_capture_isr_hook_3(TIMER_CAPTURE_REG1);
		}
		return;
	}



	/**
\brief Private method to initialize the timer 0.

Sets the clock preescaler and configures everything.
Does NOT set the compare nor capture registers.
	 */
static void private_prepareTimer0() {
		uint32_t pclkdiv;
		uint32_t pclk;

		// power Timer/Counter 0 module [bit 1]
		LPC_SC->PCONP       |=  (0<<1);

		// clock Timer/Counter 0 module [bit 1]
		//LPC_SC->PCLKSEL0  |=1;           //PCLKSEL1 register (Table 41), select PCLK_TIMER2/3.

		//1 means PCLK_peripheral = CCLK == cpu clock
		pclkdiv = (LPC_SC->PCLKSEL0 >> 2) & 0x03;

		switch (pclkdiv) {
		case 0x00:
		default:
			pclk = SystemCoreClock / 4; //global from system_LPC17xx.c
			break;
		case 0x01:
			pclk = SystemCoreClock;
			break;
		case 0x02:
			pclk = SystemCoreClock / 2;
			break;
		case 0x03:
			pclk = SystemCoreClock / 8;
			break;
		}

		// Timer Control Register: reset counter
		LPC_TIM0->TCR   = 0x02;

		// Prescale Register
		LPC_TIM0->PR    = pclk/TICS_PER_SECOND;

		//LPC_TIM0->MR0 = delayInMs * TIME_INTERVALmS;

		// It has 2 matches more MR1, MR2 so we can set
		// (we do that in set compare function)

		// Interrupt Register
		LPC_TIM0->IR    = 0xFF;

		// Match Control Register
		LPC_TIM0->MCR   = 0x01;

		//3 = Interrupt & reset timer0 on match
		//1 = Interrupt only, no reset of timer0

		//enable timer0 interrupt
		NVIC_EnableIRQ(TIMER0_IRQn);
	}

	/**
\brief Private method to initialize the timer 1.

Sets the clock preescaler and configures everything.
Does NOT set the compare nor capture registers.
	 */
  static void private_prepareTimer1()
	{
		uint32_t pclkdiv;
		uint32_t pclk;

		// power Timer/Counter 0 module [bit 2]
		LPC_SC->PCONP  |=  (0<<2);

		// On reset:
		// - Timers 0/1 are enabled  (PCTIM0/1 = 1)
		// - Timers 2/3 are disabled (PCTIM2/3 = 0)

		// Peripheral clock:
		// In the PCLKSEL0 register (Table 40), select PCLK_TIMER0/1
		//LPC_SC->PCLKSEL0  |=1;           //PCLKSEL1 register (Table 41), select PCLK_TIMER2/3.

		//1 means PCLK_peripheral = CCLK == cpu clock

		pclkdiv = (LPC_SC->PCLKSEL1 >> 2) & 0x03;

		switch (pclkdiv) {
		case 0x00:
		default:
			pclk = SystemCoreClock / 4; //global from system_LPC17xx.c
			break;
		case 0x01:
			pclk = SystemCoreClock;
			break;
		case 0x02:
			pclk = SystemCoreClock / 2;
			break;
		case 0x03:
			pclk = SystemCoreClock / 8;
			break;
		}

		// Timer Control Register: reset counter
		LPC_TIM1->TCR   = 0x02;

		// Prescale Register
		LPC_TIM1->PR    = pclk/TICS_PER_SECOND;
		//LPC_TIM1->MR0 = delayInMs * TIME_INTERVALmS;// N milliseconds x Number of tics a millisecond is.
		//if we want 32768 hz, we need to preescale the timer. If the freq of the mcu is 100Mhz, we need to
		//clock the timer at cclk/8=12,5Mhz and then preescale at a value around 382 (17E)
		// It has 2 matches more MR1, MR2 so we can set
		// (we do that in set compare function)

		LPC_TIM1->IR    = 0xFF;
		LPC_TIM1->MCR   = 0x01;

		//3 = Interrupt & reset timer0 on match
		//1 = Interrupt only, no reset of timer

		//enable timer1 interrupt
		NVIC_EnableIRQ(TIMER1_IRQn);
	}



	/**
\brief Private method to initialize the timer 2.

Sets the clock preescaler and configures everything.
Does NOT set the compare nor capture registers.
	 */
 static  void private_prepareTimer2()
	{
		uint32_t pclkdiv;
		uint32_t pclk;

		// power Timer/Counter 2 module [bit 22]
		LPC_SC->PCONP  |=  (1<<22);

		// On reset:
		// - Timers 0/1 are enabled  (PCTIM0/1 = 1)
		// - Timers 2/3 are disabled (PCTIM2/3 = 0) pins 22 & 23

		// Peripheral clock:
		// In the PCLKSEL0 register (Table 40), select PCLK_TIMER0/1
		LPC_SC->PCLKSEL1  |=0<<12;           //PCLKSEL1 register (Table 41), select PCLK_TIMER2/3. (set to 11 if we want cclk/8)

		//1 means PCLK_peripheral = CCLK == cpu clock

		pclkdiv = (LPC_SC->PCLKSEL1 >> 2) & 0x03;

		switch (pclkdiv) {
		case 0x00:
		default:
			pclk = SystemCoreClock / 4; //global from system_LPC17xx.c
			break;
		case 0x01:
			pclk = SystemCoreClock;
			break;
		case 0x02:
			pclk = SystemCoreClock / 2;
			break;
		case 0x03:
			pclk = SystemCoreClock / 8;
			break;
		}

		// Timer Control Register: reset counter
		LPC_TIM2->TCR   = 0x02;

		// Prescale Register we want 32768hz, pclk/8=12.5Mhz --> we need to set pclk to a multiple of 32768.
		//LPC_TIM2->PR    = pclk/TICS_PER_SECOND; //by now is 1M per second
		 LPC_TIM2->PR=0x2FC;//at 100Mhz --> 32768 tics per second.
		//LPC_TIM1->MR0 = delayInMs * TIME_INTERVALmS;// N milliseconds x Number of tics a millisecond is.
		//if we want 32768 hz, we need to preescale the timer. If the freq of the mcu is 100Mhz, we need to
		//clock the timer at cclk/8=12,5Mhz and then preescale at a value around 382 (17E)
		// It has 2 matches more MR1, MR2 so we can set
		// (we do that in set compare function)

		LPC_TIM2->IR    = 0xFF;
		LPC_TIM2->MCR   |= 0x3;//reset the timer on match.

		//3 = Interrupt & reset timer0 on match
		//1 = Interrupt only, no reset of timer

		//enable timer2 interrupt
		NVIC_EnableIRQ(TIMER2_IRQn);
	}




	/**
\brief Private method to initialize the timer 3.

Sets the clock preescaler and configures everything.
Does NOT set the compare nor capture registers.
	 */

static  void private_prepareTimer3()
	{
		uint32_t pclkdiv;
		uint32_t pclk;

		// power Timer/Counter 3 module [bit 23]
		LPC_SC->PCONP  |=  (1<<23);

		// On reset:
		// - Timers 0/1 are enabled  (PCTIM0/1 = 1)
		// - Timers 2/3 are disabled (PCTIM2/3 = 0) pins 22 & 23

		// Peripheral clock:
		// In the PCLKSEL0 register (Table 40), select PCLK_TIMER0/1
		LPC_SC->PCLKSEL1  |=0<<14;           //PCLKSEL1 register (Table 41), select PCLK_TIMER2/3. (set to 11 if we want cclk/8)

		//1 means PCLK_peripheral = CCLK == cpu clock

		pclkdiv = (LPC_SC->PCLKSEL1 >> 2) & 0x03;

		switch (pclkdiv) {
		case 0x00:
		default:
			pclk = SystemCoreClock / 4; //global from system_LPC17xx.c
			break;
		case 0x01:
			pclk = SystemCoreClock;
			break;
		case 0x02:
			pclk = SystemCoreClock / 2;
			break;
		case 0x03:
			pclk = SystemCoreClock / 8;
			break;
		}

		// Timer Control Register: reset counter
		LPC_TIM3->TCR   = 0x02;

		// Prescale Register we want 32768hz, pclk/8=12.5Mhz --> we need to set pclk to a multiple of 32768.
		//preescale by 549 as we want 327
		//LPC_TIM3->PR    = pclk/TICS_PER_SECOND; //by now is 1M per second
		 //LPC_TIM3->PR    =0x225; //preescale by 549, as we have 72Mhz clock, we want 72Mhz/(4*549)= 32786 tics per second
		 LPC_TIM3->PR=0x2FC;//at 100Mhz
		 //LPC_TIM3->PR=0x394;//at 120Mhz


		 //LPC_TIM1->MR0 = delayInMs * TIME_INTERVALmS;// N milliseconds x Number of tics a millisecond is.
		//if we want 32768 hz, we need to preescale the timer. If the freq of the mcu is 100Mhz, we need to
		//clock the timer at cclk/8=12,5Mhz and then preescale at a value around 382 (17E)
		// It has 2 matches more MR1, MR2 so we can set
		// (we do that in set compare function)

		LPC_TIM3->IR    = 0xFF;
		LPC_TIM3->MCR   = 0x01;

		//3 = Interrupt & reset timer0 on match
		//1 = Interrupt only, no reset of timer

		//enable timer2 interrupt
		NVIC_EnableIRQ(TIMER3_IRQn);
	}




