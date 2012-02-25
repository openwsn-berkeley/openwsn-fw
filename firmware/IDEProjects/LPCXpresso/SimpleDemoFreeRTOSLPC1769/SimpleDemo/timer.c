/**
\brief LPC17XX-specific definition of the "timer" bsp module.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, February 2012.
 */

#include "stdint.h"
#include "timer.h"
#include "LPC17xx.h"
#include "leds.h"

extern void TIMER0_IRQHandler(void);
extern void TIMER1_IRQHandler(void);
static void private_prepareTimer0(void);
static void private_prepareTimer1(void);


/**
 * Init the timer
 */
void timer_init(uint8_t timer_num){

	switch (timer_num){
	case 0:
		private_prepareTimer0();

		break;
	case 1:
		private_prepareTimer0();
		break;
	}
	return;
}

/**
 * sets the compare value in the timer. every timer has 3 compare registers. so choose one.
 */
void timer_set_compare(uint8_t timer_num,uint8_t compareReg, uint32_t delayInMs){
	switch (timer_num){

	case 0:
		if (compareReg==TIMER_COMPARE_REG0){
			LPC_TIM0->MR0 = delayInMs * TIME_INTERVALmS;// N milliseconds x Number of tics a millisecond is.
		}else if (compareReg==TIMER_COMPARE_REG1){
			LPC_TIM0->MR1 = delayInMs * TIME_INTERVALmS;// N milliseconds x Number of tics a millisecond is.
		}else if (compareReg==TIMER_COMPARE_REG2){
			LPC_TIM0->MR2 = delayInMs * TIME_INTERVALmS;// N milliseconds x Number of tics a millisecond is.
		}else{
			//error.. do nothing??
		}
		break;

	case 1:
		if (compareReg==TIMER_COMPARE_REG0){
			LPC_TIM1->MR0 = delayInMs * TIME_INTERVALmS;// N milliseconds x Number of tics a millisecond is.
		}else if (compareReg==TIMER_COMPARE_REG1){
			LPC_TIM1->MR1 = delayInMs * TIME_INTERVALmS;// N milliseconds x Number of tics a millisecond is.
		}else if (compareReg==TIMER_COMPARE_REG2){
			LPC_TIM1->MR2 = delayInMs * TIME_INTERVALmS;// N milliseconds x Number of tics a millisecond is.
		}else{
			//error.. do nothing??
		}
		break;
	}
	return;
}


/**
 * TODO. check first how it works the capture...
 * sets the capture register. every timer has 2 capture registers. so choose one.
 */
void timer_set_capture(uint8_t timer_num,uint8_t captureReg){
	switch (timer_num){

	case 0:
		if (captureReg==TIMER_CAPTURE_REG0){

		}else if (captureReg==TIMER_CAPTURE_REG1){

		}else{
			//error.. do nothing??
		}
		break;

	case 1:
		if (captureReg==TIMER_CAPTURE_REG0){

		}else if (captureReg==TIMER_CAPTURE_REG1){

		}else{
			//error.. do nothing??
		}
		break;
	}
	return;
}

/**
 * Starts the timer. Call it after timer_init.
 */
void timer_enable( uint8_t timer_num )
{
	if ( timer_num == 0 )
	{
		LPC_TIM0->TCR = 1;
	}
	else if ( timer_num == 1 )
	{
		LPC_TIM1->TCR = 1;
	}
	else if ( timer_num == 2 )
	{
		LPC_TIM2->TCR = 1;
	}
	else if ( timer_num == 3 )
	{
		LPC_TIM3->TCR = 1;
	}
	return;
}

/**
 * resets the timer.
 */
void timer_reset( uint8_t timer_num )
{
	uint32_t regVal;

	if ( timer_num == 0 )
	{
		regVal = LPC_TIM0->TCR;
		regVal |= 0x02;
		LPC_TIM0->TCR = regVal;
	}
	else if ( timer_num == 1 )
	{
		regVal = LPC_TIM1->TCR;
		regVal |= 0x02;
		LPC_TIM1->TCR = regVal;
	}
	else if ( timer_num == 2 )
	{
		regVal = LPC_TIM2->TCR;
		regVal |= 0x02;
		LPC_TIM2->TCR = regVal;
	}
	else if ( timer_num == 3 )
	{
		regVal = LPC_TIM3->TCR;
		regVal |= 0x02;
		LPC_TIM3->TCR = regVal;
	}
	return;
}

/**
 * disables the timer.
 */
void timer_disable( uint8_t timer_num )
{
	if ( timer_num == 0 )
	{
		LPC_TIM0->TCR = 0;
	}
	else if ( timer_num == 1 )
	{
		LPC_TIM1->TCR = 0;
	}
	else if ( timer_num == 2 )
	{
		LPC_TIM2->TCR = 0;
	}
	else if ( timer_num == 3 )
	{
		LPC_TIM2->TCR = 0;
	}
	return;
}

/**
 * irq handler for timer 0. "implements" weak function in cr_startup_lpc17.c
 */
void TIMER0_IRQHandler (void)
{
	if ( LPC_TIM0->IR & (0x1<<0) )
	{
		LPC_TIM0->IR = 0x1<<0;		/* clear interrupt flag */
		led_all_toggle();
	}
	if ( LPC_TIM0->IR & (0x1<<1) )
	{
		LPC_TIM0->IR = 0x1<<1;		/* clear interrupt flag */
		led_all_toggle();
	}
	if ( LPC_TIM0->IR & (0x1<<4) )
	{
		LPC_TIM0->IR = 0x1<<4;		/* clear interrupt flag */
		led_all_toggle();
	}
	if ( LPC_TIM0->IR & (0x1<<5) )
	{
		LPC_TIM0->IR = 0x1<<5;		/* clear interrupt flag */
		led_all_toggle();
	}
	return;
}

/**
 * irq handler for timer 1. "implements" weak function in cr_startup_lpc17.c
 */
void TIMER1_IRQHandler (void)
{
	if ( LPC_TIM1->IR & (0x1<<0) )
	{
		LPC_TIM1->IR = 0x1<<0;		/* clear interrupt flag */
		led_all_toggle();
	}
	if ( LPC_TIM1->IR & (0x1<<1) )
	{
		LPC_TIM1->IR = 0x1<<1;		/* clear interrupt flag */
		led_all_toggle();
	}
	if ( LPC_TIM1->IR & (0x1<<4) )
	{
		LPC_TIM1->IR = 0x1<<4;		/* clear interrupt flag */
		led_all_toggle();
	}
	if ( LPC_TIM1->IR & (0x1<<5) )
	{
		LPC_TIM1->IR = 0x1<<5;		/* clear interrupt flag */
		led_all_toggle();
	}
	return;
}


/**
 * private method to initialize the timer 0. Sets the clock preescaler and configures everything. Does NOT set the compare nor capture registers.
 */
static void private_prepareTimer0()
{
	uint32_t pclkdiv, pclk;
	LPC_SC->PCONP |=0x01<1;
	//Remark: On reset, Timer0/1 are enabled (PCTIM0/1 = 1), and Timer2/3 are disabled
	//(PCTIM2/3 = 0).
	//Peripheral clock: In the PCLKSEL0 register (Table 40), select PCLK_TIMER0/1; in the
	//LPC_SC->PCLKSEL0  |=1; 		    //PCLKSEL1 register (Table 41), select PCLK_TIMER2/3.
	//1 means PCLK_peripheral = CCLK == cpu clock
	pclkdiv = (LPC_SC->PCLKSEL0 >> 2) & 0x03;
	switch (pclkdiv){
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
	LPC_TIM0->TCR = 0x02;
	LPC_TIM0->PR  = pclk/TICS_PER_SECOND;
	//LPC_TIM0->MR0 = delayInMs * TIME_INTERVALmS;// N milliseconds x Number of tics a millisecond is.
	//note: it has 2 matches more MR1,MR2 so we can set (we do that in set compare function)
	LPC_TIM0->IR  = 0xFF;
	LPC_TIM0->MCR = 0x03;
	//3 = Interrupt & reset timer0 on match
	//1 = Interrupt only, no reset of timer0
	NVIC_EnableIRQ(TIMER0_IRQn); //enable timer0 interrupt
}


/**
 * private method to initialize the timer 1. Sets the clock preescaler and configures everything. Does NOT set the compare nor capture registers.
 */
static void private_prepareTimer1()
{
	uint32_t pclkdiv, pclk;
	LPC_SC->PCONP |=0x01<1;
	//Remark: On reset, Timer0/1 are enabled (PCTIM0/1 = 1), and Timer2/3 are disabled
	//(PCTIM2/3 = 0).
	//Peripheral clock: In the PCLKSEL0 register (Table 40), select PCLK_TIMER0/1; in the
	//LPC_SC->PCLKSEL0  |=1; 		    //PCLKSEL1 register (Table 41), select PCLK_TIMER2/3.
	//1 means PCLK_peripheral = CCLK == cpu clock
	pclkdiv = (LPC_SC->PCLKSEL1 >> 2) & 0x03;
	switch (pclkdiv){
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
	LPC_TIM1->TCR = 0x02;
	LPC_TIM1->PR  = pclk/TICS_PER_SECOND;
	//LPC_TIM1->MR0 = delayInMs * TIME_INTERVALmS;// N milliseconds x Number of tics a millisecond is.
	//note: it has 2 matches more MR1,MR2 so we can set (we do that in set compare function)
	LPC_TIM1->IR  = 0xFF;
	LPC_TIM1->MCR = 0x03;
	//3 = Interrupt & reset timer0 on match
	//1 = Interrupt only, no reset of timer0
	NVIC_EnableIRQ(TIMER1_IRQn); //enable timer0 interrupt
}
