/*
 * servo.c
 *
 *  Created on: Dec 4, 2017
 *      Author: kilberg
 */

/******************************************************************************
* INCLUDES
*/
#include <stdio.h>
//#include "bsp.h"
//#include "bsp_led.h"
#include "gptimer.h"
#include "sys_ctrl.h"
#include "headers/hw_gptimer.h"
#include "headers/hw_ints.h"
#include "gpio.h"
#include "interrupt.h"
//#include "led.h"
#include "headers/hw_memmap.h"
#include "headers/hw_gpio.h"
#include "ioc.h"


//Global VAriables ***************************************************************
float servo_scale;

//refresh width and pulse widths are in ms
void servo_init(uint32_t timer,int refresh_rate,float center){

	 uint32_t pwmTimerClkEnable;
	 uint32_t pwmTimerBase;
	 uint32_t freqCnt=SysCtrlClockGet()*refresh_rate/1000; //number of ticks in one period is period * clock rate
	 uint8_t pre_cnt=(freqCnt>>16)&0xFF;	//prescaler count should have the upper bits 16 to 23 of the count
	 uint16_t timer_cnt=freqCnt & 0xFFFF; 	//lower 16 bits of count value
	 float neutral_pulse_width = center;	//pulse width for neutral pos

	 //calc pulse widths for servo neutral

	 uint32_t neutral_match_count = neutral_pulse_width / 1000 * SysCtrlClockGet(); 	//match set
	 uint16_t match_lower = neutral_match_count & 0xFFFF;
	 uint8_t match_upper = (neutral_match_count >> 16) & 0xFF;

	 switch(timer){

	  case 0:
	    pwmTimerClkEnable=SYS_CTRL_PERIPH_GPT0;
	    pwmTimerBase=GPTIMER0_BASE;
	   // timerIntA=INT_TIMER0A;
	    //timerIntB=INT_TIMER0B;
	    break;

	  case 1:
	    pwmTimerClkEnable=SYS_CTRL_PERIPH_GPT1;
	    pwmTimerBase=GPTIMER1_BASE;
	    //timerIntA=INT_TIMER1A;
	    //timerIntB=INT_TIMER1B;
	    break;

	  case 2:
	    pwmTimerClkEnable=SYS_CTRL_PERIPH_GPT2;
	    pwmTimerBase=GPTIMER2_BASE;
	   // timerIntA=INT_TIMER2A;
	   // timerIntB=INT_TIMER2B;
	    break;

	  case 3:
	    pwmTimerClkEnable=SYS_CTRL_PERIPH_GPT3;
	    pwmTimerBase=GPTIMER3_BASE;
	   // timerIntA=INT_TIMER3A;
	   // timerIntB=INT_TIMER3B;
	    break;


	  }


	  SysCtrlPeripheralEnable(pwmTimerClkEnable); //enables timer module

	  TimerConfigure(pwmTimerBase, GPTIMER_CFG_SPLIT_PAIR |GPTIMER_CFG_A_PWM | GPTIMER_CFG_B_PWM); //configures timers as pwm timers
	  TimerPrescaleSet(pwmTimerBase,GPTIMER_A,pre_cnt);		//load upper 8 bits of timer count
	  TimerLoadSet(pwmTimerBase,GPTIMER_A,timer_cnt); 	//load lower 16 bits of timer count

	  TimerControlLevel(pwmTimerBase,GPTIMER_A,true); //active high pwm

	  TimerPrescaleSet(pwmTimerBase,GPTIMER_B,pre_cnt);		//load upper 8 bits of timer count
	  TimerLoadSet(pwmTimerBase,GPTIMER_B,timer_cnt); 	//load lower 16 bits of timer count

	  TimerControlLevel(pwmTimerBase,GPTIMER_B,true); //active high pwm


	  IOCPinConfigPeriphOutput(GPIO_D_BASE,GPIO_PIN_1,IOC_MUX_OUT_SEL_GPT3_ICP1); //maps pwm1 output to pin1
	  IOCPinConfigPeriphOutput(GPIO_D_BASE,GPIO_PIN_2,IOC_MUX_OUT_SEL_GPT3_ICP2);

	  //set match registers to determine pulse width
	  TimerMatchSet(pwmTimerBase,GPTIMER_A,match_lower);
	  TimerPrescaleMatchSet(pwmTimerBase,GPTIMER_A,match_upper);

	  TimerEnable(pwmTimerBase,GPTIMER_A);

	  TimerMatchSet(pwmTimerBase,GPTIMER_B,match_lower);
	  TimerPrescaleMatchSet(pwmTimerBase,GPTIMER_B,match_upper);

	  TimerEnable(pwmTimerBase,GPTIMER_B);

	  GPIOPinTypeTimer(GPIO_D_BASE,GPIO_PIN_1); //enables hw muxing of pin outputs

	    IOCPadConfigSet(GPIO_D_BASE,GPIO_PIN_1,IOC_OVERRIDE_OE|IOC_OVERRIDE_PUE); // enables pins as outputs, necessary for this code to work correctly

		  GPIOPinTypeTimer(GPIO_D_BASE,GPIO_PIN_2); //enables hw muxing of pin outputs
		    IOCPadConfigSet(GPIO_D_BASE,GPIO_PIN_2,IOC_OVERRIDE_OE|IOC_OVERRIDE_PUE); // enables pins as outputs, necessary for this code to work correctly


}

//time based servo controller. input is 1-2 ms
void servo_rotate_time(float pulse_width,int servo){
	 //calc pulse widths for servo neutral

	 uint32_t neutral_match_count = pulse_width / 1000 * SysCtrlClockGet(); 	//match set
	 uint16_t match_lower = neutral_match_count & 0xFFFF;
	 uint8_t match_upper = (neutral_match_count >> 16) & 0xFF;
	 if (servo==0){
	  TimerMatchSet(GPTIMER3_BASE,GPTIMER_A,match_lower);
	  TimerPrescaleMatchSet(GPTIMER3_BASE,GPTIMER_A,match_upper);
	 }
	 else if(servo==1){
		  TimerMatchSet(GPTIMER3_BASE,GPTIMER_B,match_lower);
		  TimerPrescaleMatchSet(GPTIMER3_BASE,GPTIMER_B,match_upper);
	 }
}

