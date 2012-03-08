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



//hooks to isr's
low_timer_hook timer_compare_isr_hook_0;
low_timer_hook timer_compare_isr_hook_1;
low_timer_hook timer_capture_isr_hook_0;
low_timer_hook timer_capture_isr_hook_1;


void timer_set_isr_compare_hook(uint8_t timer_num, low_timer_hook cbt){
	switch (timer_num){
		case TIMER_NUM0:
			timer_compare_isr_hook_0=cbt;
			break;
		case TIMER_NUM1:
			timer_compare_isr_hook_1=cbt;
			break;
		}
		return;
}

void timer_set_isr_capture_hook(uint8_t timer_num, low_timer_hook cbt){
	switch (timer_num){
		case TIMER_NUM0:
			timer_capture_isr_hook_0=cbt;
			break;
		case TIMER_NUM1:
			timer_capture_isr_hook_1=cbt;
			break;
		}
		return;
}

/**
 * Init the timer
 */
void timer_init(uint8_t timer_num){

	switch (timer_num){
	case TIMER_NUM0:
		private_prepareTimer0();

		break;
	case TIMER_NUM1:
		private_prepareTimer0();
		break;
	}
	return;
}

uint32_t timer_get_current_value(uint8_t timer_num){
	uint32_t current=0;
	switch (timer_num){
	case TIMER_NUM0:
		break;
		current=LPC_TIM0->TC;
	case TIMER_NUM1:
		current=LPC_TIM1->TC;
		break;

	}
	return current;
}

/**
 * sets the compare value in the timer. every timer has 3 compare registers. so choose one.
 */
void timer_set_compare(uint8_t timer_num,uint8_t compareReg, uint32_t delayInMs){
	switch (timer_num){

	case TIMER_NUM0:
		if (compareReg==TIMER_COMPARE_REG0){
			//configure match register:

			LPC_TIM0->MCR|=1;//interrupt when mr0 matches the value in the TC
			LPC_TIM0->MR0 = delayInMs * TIME_INTERVALmS;// N milliseconds x Number of tics a millisecond is.
		}else if (compareReg==TIMER_COMPARE_REG1){
			LPC_TIM0->MCR|=1<<3;//interrupt when mr1 matches the value in the TC
			LPC_TIM0->MR1 = delayInMs * TIME_INTERVALmS;// N milliseconds x Number of tics a millisecond is.
		}else if (compareReg==TIMER_COMPARE_REG2){
			LPC_TIM0->MCR|=1<<6;//interrupt when mr2 matches the value in the TC
			LPC_TIM0->MR2 = delayInMs * TIME_INTERVALmS;// N milliseconds x Number of tics a millisecond is.
		}else{
			//error.. do nothing??
		}
		break;

	case TIMER_NUM1:
		if (compareReg==TIMER_COMPARE_REG0){
			LPC_TIM1->MCR|=1;//interrupt when mr0 matches the value in the TC
			LPC_TIM1->MR0 = delayInMs * TIME_INTERVALmS;// N milliseconds x Number of tics a millisecond is.
		}else if (compareReg==TIMER_COMPARE_REG1){
			LPC_TIM1->MCR|=1<<3;//interrupt when mr1 matches the value in the TC
			LPC_TIM1->MR1 = delayInMs * TIME_INTERVALmS;// N milliseconds x Number of tics a millisecond is.
		}else if (compareReg==TIMER_COMPARE_REG2){
			LPC_TIM1->MCR|=1<<6;//interrupt when mr1 matches the value in the TC
			LPC_TIM1->MR2 = delayInMs * TIME_INTERVALmS;// N milliseconds x Number of tics a millisecond is.
		}else{
			//error.. do nothing??
		}
		break;
	}
	return;
}

void timer_reset_compare(uint8_t timer_num,uint8_t compareReg){
	switch (timer_num){
		case TIMER_NUM0:
			if (compareReg==TIMER_COMPARE_REG0){
				//configure match register:

				LPC_TIM0->MCR&=~1;//interrupt when mr0 matches the value in the TC
				LPC_TIM0->MR0 = 0;
			}else if (compareReg==TIMER_COMPARE_REG1){
				LPC_TIM0->MCR&=~1<<3;//interrupt when mr1 matches the value in the TC
				LPC_TIM0->MR1 = 0;
			}else if (compareReg==TIMER_COMPARE_REG2){
				LPC_TIM0->MCR&=~1<<6;//interrupt when mr2 matches the value in the TC
				LPC_TIM0->MR2 = 0;
			}else{
				//error.. do nothing??
			}
			break;

		case TIMER_NUM1:
			if (compareReg==TIMER_COMPARE_REG0){
				LPC_TIM1->MCR&=~1;//interrupt when mr0 matches the value in the TC
				LPC_TIM1->MR0 = 0;
			}else if (compareReg==TIMER_COMPARE_REG1){
				LPC_TIM1->MCR&=~1<<3;//interrupt when mr1 matches the value in the TC
				LPC_TIM1->MR1 = 0;
			}else if (compareReg==TIMER_COMPARE_REG2){
				LPC_TIM1->MCR&=~1<<6;//interrupt when mr1 matches the value in the TC
				LPC_TIM1->MR2 = 0;
			}else{
				//error.. do nothing??
			}
			break;
		}
		return;
}
/**
 *
 * configures the capture register and sets its interruption. every timer has 2 capture registers. so choose one.
 */
void timer_set_capture(uint8_t timer_num,uint8_t captureReg){
	switch (timer_num){
	case TIMER_NUM0:
		if (captureReg==TIMER_CAPTURE_REG0){

#if TIMER_MATCH // if external match pins (DMA)
			LPC_TIM0->EMR &= ~(0xFF<<4);//set to 0 4 lowest bits.. ExternalMatchRegister. p.498
			LPC_TIM0->EMR |= ((0x3<<4);//set bit 4-5 to 11 -- mean toggle external match pin
#else
			//configure capture register when something happens.
			//bit 0 = CAP0RE=1 -- capture on raising edge -- value of TC will be copied to CAP
			//bit 2 = CAP0I=1 -- generate interrupt on capture
			LPC_TIM0->CCR = (0x1<<0)|(0x1<<2);
#endif
		}else if (captureReg==TIMER_CAPTURE_REG1){

#if TIMER_MATCH // if external match pins (DMA)
			LPC_TIM0->EMR &= ~(0xFF<<4);//set to 0 4 lowest bits.. ExternalMatchRegister. p.498
			LPC_TIM0->EMR |= ((0x3<<6);//set bit 6-7 to 11 -- mean toggle external match pin
#else
			//configure capture register when something happens.
			//bit 3 = CAP1RE=1 -- capture on raising edge -- value of TC will be copied to CAP
			//bit 5 = CAP1I=1 -- generate interrupt on capture
			LPC_TIM0->CCR = (0x1<<3)|(0x1<<5);
#endif
		}else{
			//error.. do nothing??
		}
		break;
	case TIMER_NUM1:
		if (captureReg==TIMER_CAPTURE_REG0){
			//configure capture register when something happens.
			//bit 0 = CAP0RE=1 -- capture on raising edge -- value of TC will be copied to CAP
			//bit 2 = CAP0I=1 -- generate interrupt on capture
			LPC_TIM1->CCR |= (0x1<<0)|(0x1<<2);
		}else if (captureReg==TIMER_CAPTURE_REG1){
			//bit 3 = CAP1RE=1 -- capture on raising edge -- value of TC will be copied to CAP
			//bit 5 = CAP1I=1 -- generate interrupt on capture
			LPC_TIM1->CCR |= (0x1<<3)|(0x1<<5);
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
	if ( timer_num == TIMER_NUM0 )
	{
		LPC_TIM0->TCR = 1;
	}
	else if ( timer_num == TIMER_NUM1 )
	{
		LPC_TIM1->TCR = 1;
	}

	return;
}

/**
 * resets the timer.
 */
void timer_reset( uint8_t timer_num )
{
	uint32_t regVal;

	if ( timer_num == TIMER_NUM0 )
	{
		regVal = LPC_TIM0->TCR;
		regVal |= 0x02;
		LPC_TIM0->TCR = regVal;
	}
	else if ( timer_num == TIMER_NUM1 )
	{
		regVal = LPC_TIM1->TCR;
		regVal |= 0x02;
		LPC_TIM1->TCR = regVal;
	}

	return;
}

/**
 * disables the timer.
 */
void timer_disable( uint8_t timer_num )
{
	if ( timer_num == TIMER_NUM0 )
	{
		LPC_TIM0->TCR = 0;
	}
	else if ( timer_num == TIMER_NUM1 )
	{
		LPC_TIM1->TCR = 0;
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
		printf(" interrupt of timer 0 compare 0 - time is %d , capture is %d \n" ,LPC_TIM0->TC,LPC_TIM0->CR0);
		LPC_TIM0->IR = 0x1<<0;		/* clear interrupt flag */
		leds_all_toggle();
		timer_compare_isr_hook_0(TIMER_COMPARE_REG0);
		//timer_compare_isr_hook_0();
	}
	if ( LPC_TIM0->IR & (0x1<<1) )
	{
		printf(" interrupt of timer 0 compare 1 - time is %d \n" ,LPC_TIM0->TC);
		LPC_TIM0->IR = 0x1<<1;		/* clear interrupt flag */
		leds_all_toggle();
		timer_compare_isr_hook_0(TIMER_COMPARE_REG1);
		//timer_compare_isr_hook_0();
	}
	if ( LPC_TIM0->IR & (0x1<<2) )
	{
		printf(" interrupt of timer 0 compare 2 - time is %d \n" ,LPC_TIM0->TC);
		LPC_TIM0->IR = 0x1<<1;		/* clear interrupt flag */
		leds_all_toggle();
		timer_compare_isr_hook_0(TIMER_COMPARE_REG2);
		//timer_compare_isr_hook_0();
	}
	if ( LPC_TIM0->IR & (0x1<<4) )
	{
		printf(" interrupt of timer 0 capture 0 - time is %d \n" ,LPC_TIM0->CR0);
		LPC_TIM0->IR = 0x1<<4;		/* clear interrupt flag */
		leds_all_toggle();
		timer_capture_isr_hook_0(TIMER_CAPTURE_REG0);
		//timer_capture_isr_hook_0();
	}
	if ( LPC_TIM0->IR & (0x1<<5) )
	{
		printf(" interrupt of timer 0 capture 1 - time is %d \n" ,LPC_TIM0->CR1);
		LPC_TIM0->IR = 0x1<<5;		/* clear interrupt flag */
		leds_all_toggle();
		timer_capture_isr_hook_0(TIMER_CAPTURE_REG1);
	//	timer_capture_isr_hook_0();
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
		printf(" interrupt of timer 1 compare 0 - time is %d \n" ,LPC_TIM1->TC);
		LPC_TIM1->IR = 0x1<<0;		/* clear interrupt flag */
		leds_all_toggle();
		timer_compare_isr_hook_1(TIMER_COMPARE_REG0);
		//timer_compare_isr_hook_1();
	}
	if ( LPC_TIM1->IR & (0x1<<1) )
	{
		printf(" interrupt of timer 1 compare 1 - time is %d \n" ,LPC_TIM1->TC);
		LPC_TIM1->IR = 0x1<<1;		/* clear interrupt flag */
		leds_all_toggle();
		timer_compare_isr_hook_1(TIMER_COMPARE_REG1);
	//	timer_compare_isr_hook_1();
	}
	if ( LPC_TIM1->IR & (0x1<<3) )
	{
		printf(" interrupt of timer 1 compare 2 - time is %d \n" ,LPC_TIM1->TC);
		LPC_TIM1->IR = 0x1<<2;		/* clear interrupt flag */
		leds_all_toggle();
		timer_compare_isr_hook_1(TIMER_COMPARE_REG2);//call
	//	timer_compare_isr_hook_1();
	}
	if ( LPC_TIM1->IR & (0x1<<4) )
	{
		printf(" interrupt of timer 1 capture 0 - time is %d \n" ,LPC_TIM1->CR0);
		LPC_TIM1->IR = 0x1<<4;		/* clear interrupt flag */
		leds_all_toggle();
		timer_capture_isr_hook_1(TIMER_CAPTURE_REG0);
	//	timer_capture_isr_hook_1();
	}
	if ( LPC_TIM1->IR & (0x1<<5) )
	{
		printf(" interrupt of timer 1 capture 1 - time is %d \n" ,LPC_TIM1->CR1);
		LPC_TIM1->IR = 0x1<<5;		/* clear interrupt flag */
		leds_all_toggle();
		timer_capture_isr_hook_1(TIMER_CAPTURE_REG1);
	//	timer_capture_isr_hook_1();
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
	//note: it has 2 matches more MR1,MR2 so we can set (we do that in set compare function
	LPC_TIM0->IR  = 0xFF;
	LPC_TIM0->MCR = 0x01;
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
	LPC_TIM1->MCR = 0x01;
	//3 = Interrupt & reset timer0 on match
	//1 = Interrupt only, no reset of timer0
	NVIC_EnableIRQ(TIMER1_IRQn); //enable timer0 interrupt
}
